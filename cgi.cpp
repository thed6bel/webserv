#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <vector>

#define PORT 80
#define BUFFER_SIZE 2048
#define MAX_CLIENTS 10
#define RETRY_INTERVAL 10
#define CGI_BIN_PATH "./cgi-bin/"

// Fonction pour le GET a mettre ici

// Fonction pour le parsing de la méthode de la requête HTTP
std::string parseMethod(const std::string& request) {
    std::istringstream iss(request);
    std::string method;
    iss >> method;
    return method;
}
// Fonction pour la lecture de la methode DELETE
std::string parseMethodDelete(const std::string& request) {
    std::string requ(request);
    size_t start = requ.find("DELETE ") + 7;
    size_t end = requ.find(" HTTP/");
    std::string requestedPath = requ.substr(start, end - start);
    return requestedPath;
}

std::string readFromFile(const std::string &filename)
{
    std::ifstream file(filename.c_str());
    if (file.is_open())
    {
        std::ostringstream oss;
        oss << file.rdbuf();
        return oss.str();
    }
    return "File not found!";
}

void reportError(const char *message)
{
    std::cerr << "Error: " << message << std::endl;
}

std::string getContentType(const std::string &filename)
{
    if (filename.find(".html") != std::string::npos)
        return "text/html";
    else if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos)
        return "image/jpeg";
    else if (filename.find(".png") != std::string::npos)
        return "image/png";
    else if (filename.find(".webp") != std::string::npos)
        return "image/webp";
    else if (filename.find(".ico") != std::string::npos)
        return "image/x-icon";
    else if (filename.find(".js") != std::string::npos)
        return "application/javascript";
    else if (filename.find(".css") != std::string::npos)
        return "text/css";
    else
        return "application/octet-stream";
}

// Exécute un script CGI et renvoie la sortie
std::string executeCGI(const std::string &scriptPath, const std::string &queryString)
{
    std::stringstream ss;
    ss << CGI_BIN_PATH << scriptPath << " \"" << queryString << "\"";
    std::string command = ss.str();

    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        return "Error executing CGI script.";
    }

    char buffer[128];
    std::string result;
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            result += buffer;
        }
    }

    pclose(pipe);
    return result;
}

int main(int argc, char *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    std::vector<int> clients(MAX_CLIENTS, 0);

    // Création du socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        reportError("socket failed");
        std::exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Attachement du socket à l'adresse et au port spécifiés
    while (bind(server_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1)
    {
        reportError("bind failed. Retrying in 10 seconds...");
        close(server_fd); // Fermer le socket précédent
        sleep(RETRY_INTERVAL); // Attendre pendant 10 secondes avant de réessayer
    }

    // Passage en mode d'écoute
    if (listen(server_fd, 3) == -1)
    {
        reportError("listen failed");
        std::exit(EXIT_FAILURE);
    }

    std::cout << "Serveur en écoute sur le port " << PORT << "...\n";

    while (true)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        int max_sd = server_fd;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = clients[i];
            if (sd > 0)
                FD_SET(sd, &read_fds);
            if (sd > max_sd)
                max_sd = sd;
        }

        if (select(max_sd + 1, &read_fds, nullptr, nullptr, nullptr) == -1)
        {
            reportError("select failed");
            std::exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &read_fds))
        {
            // Nouvelle connexion
            if ((new_socket = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), &addrlen)) == -1)
            {
                reportError("accept failed");
                std::exit(EXIT_FAILURE);
            }

            // Ajout du nouveau client à la liste des clients
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] == 0)
                {
                    clients[i] = new_socket;
                    char ipAddr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(address.sin_addr), ipAddr, INET_ADDRSTRLEN);
                    std::cout << "Nouvelle connexion, socket fd " << new_socket << " , ip " << ipAddr << " , port " << ntohs(address.sin_port) << std::endl;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = clients[i];
            if (FD_ISSET(sd, &read_fds))
            {
                // Lecture des données du client
                ssize_t bytesRead = read(sd, buffer, BUFFER_SIZE);
                if (bytesRead == -1)
                {
                    reportError("read failed");
                    close(sd);
                    clients[i] = 0;
                    continue;
                }
                if (bytesRead == 0)
                {
                    // Déconnexion du client
                    std::cout << "Client déconnecté, socket fd " << sd << std::endl;
                    close(sd);
                    clients[i] = 0;
                    continue;
                }

                buffer[bytesRead] = '\0';
                std::cout << "Reçu : " << buffer << std::endl;
                // printf("Buffer pour controler : %s\n", buffer);
                std::cout << "---------------------" << std::endl;

                // faire le parsing ici des methodes et des chemins
                std::string method = parseMethod(buffer);

                // Utilisation d'un switch pour gérer les différentes méthodes HTTP
                if (method == "GET") {
                    // Extrait le chemin demandé à partir de la requête HTTP
                    std::string request(buffer);
                    size_t start = request.find("GET ") + 4;
                    size_t end = request.find(" HTTP/");
                    std::string requestedPath = request.substr(start, end - start);

                    // Si le chemin est vide, charge la page d'accueil (index.html, voir plus tard le php et autre...)
                    if (requestedPath.empty() || requestedPath == "/")
                        requestedPath = "/index.html";

                    // Chemin absolu du fichier demandé
                    std::string filePath = "." + requestedPath;

                    // Vérification de l'existence du fichier et erreur 404 si le fichier n'existe pas + gestion des CGI
                    struct stat fileStat;
                    if (stat(filePath.c_str(), &fileStat) != 0)
                    {
                        // Vérifier si le fichier est un script CGI
                        std::string scriptPath = filePath.substr(9); // Supprimer le préfixe "./cgi-bin/"
                        std::string queryString;
                        size_t queryPos = scriptPath.find("?");
                        if (queryPos != std::string::npos)
                        {
                            queryString = scriptPath.substr(queryPos + 1);
                            scriptPath = scriptPath.substr(0, queryPos);
                        }
                        // si queryString est vide, ce n'est pas un script CGI-GET et donc erreur 404
                        if (scriptPath.find(".cgi") == std::string::npos || scriptPath.find(".cgi") != scriptPath.size() - 4)
                        {
                            std::string notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\nFile not found!";
                            send(sd, notFoundResponse.c_str(), notFoundResponse.size(), 0);
                            close(sd);
                            clients[i] = 0;
                            continue;
                        }
                        printf("Script CGI détecté : %s\n", scriptPath.c_str());
                        // Exécuter le script CGI et renvoyer la sortie au client
                        std::string cgiOutput = executeCGI(scriptPath, queryString);
                        std::string contentType = "text/html"; // Type de contenu par défaut pour la sortie CGI
                        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " +
                                            std::to_string(cgiOutput.size()) + "\r\n\r\n" + cgiOutput;

                        // Envoi de la réponse au client
                        send(sd, response.c_str(), response.size(), 0);
                        close(sd);
                        clients[i] = 0;
                        continue;
                    }

                    // Lecture du contenu du fichier
                    std::ifstream file(filePath.c_str(), std::ios::binary | std::ios::ate);
                    if (!file.is_open())
                    {
                        std::string internalServerErrorResponse = "HTTP/1.1 500 Internal Server Error\r\n\r\nInternal Server Error!";
                        send(sd, internalServerErrorResponse.c_str(), internalServerErrorResponse.size(), 0);
                        close(sd);
                        clients[i] = 0;
                        continue;
                    }

                    std::streamsize fileSize = file.tellg();
                    file.seekg(0, std::ios::beg);

                    std::string fileContent(fileSize, 0);
                    file.read(&fileContent[0], fileSize);
                    file.close();

                    // Construction de la réponse HTTP
                    std::string contentType = getContentType(filePath);
                    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " +
                                        std::to_string(fileContent.size()) + "\r\n\r\n" + fileContent;

                    // Envoi de la réponse au client
                    send(sd, response.c_str(), response.size(), 0);
                    close(sd);
                    clients[i] = 0;
                } else if (method == "POST") {
                    printf("POST request\n");
                } else if (method == "DELETE") {
                    printf("DELETE request\n");
                    std::string parseDel = parseMethodDelete(buffer);
                    std::string path = "." + parseDel;
                    
                    if ( std::remove(path.c_str()) == 0)
                    {
                        printf("File deleted successfully\n");
                        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
                        send(sd, response.c_str(), response.size(), 0);
                        close(sd);
                        clients[i] = 0;
                    } else {
                        printf("Error deleting file\n");
                        std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
                        send(sd, response.c_str(), response.size(), 0);
                        close(sd);
                        clients[i] = 0;}
            
                } else if (method == "OPTIONS") {
                    // Construction de la réponse HTTP pour OPTIONS
                    std::string optionsResponse = "HTTP/1.1 200 OK\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                        "Access-Control-Allow-Headers: Content-Type\r\n"
                        "\r\n";

                    // Envoi de la réponse OPTIONS au client
                    send(sd, optionsResponse.c_str(), optionsResponse.size(), 0);
                    close(sd);
                    clients[i] = 0;

                } else {
                    std::cerr << "Unsupported HTTP method: " << method << std::endl;
                }

                
            }
        }
    }
    return 0;
}