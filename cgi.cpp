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
#include <map>

#define PORT 80
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define RETRY_INTERVAL 10
#define CGI_BIN_PATH "./cgi-bin/"

struct FileData {
    std::string filename;
    std::string content;
};

// Fonction pour extraire les données multipart/form-data
std::vector<FileData> parseMultipartFormData(const std::string& formData) {
    std::vector<FileData> files;
    std::string delimiter = "\r\n-----------------------------";

    size_t pos = formData.find(delimiter);
    if (pos == std::string::npos) {
        return files;
    }

    while (pos != std::string::npos) {
        size_t filenameStart = formData.find("filename=\"", pos);
        if (filenameStart == std::string::npos) {
            break;
        }
        filenameStart += 10;

        size_t filenameEnd = formData.find("\"", filenameStart);
        if (filenameEnd == std::string::npos) {
            break;
        }
        std::string filename = formData.substr(filenameStart, filenameEnd - filenameStart);

        size_t contentStart = formData.find("\r\n\r\n", filenameEnd);
        if (contentStart == std::string::npos)
            break;
        contentStart += 4;

        size_t contentEnd = formData.find(delimiter, contentStart);
        if (contentEnd == std::string::npos)
            break;
        std::string content = formData.substr(contentStart, contentEnd - contentStart);

        files.push_back(FileData());
        files.back().filename = filename;
        files.back().content = content;
        
        pos = formData.find(delimiter, contentEnd);
    }
    return files;
}

std::string parseScriptPath(std::string requestData) {
    std::string path;
    std::istringstream iss(requestData);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("POST") != std::string::npos) {
            size_t start = line.find("POST") + 5;
            size_t end = line.find("HTTP/") - 1;
            path = line.substr(start, end - start);
            break;
        }
    }
    //si .cgi existe return path
    if (path.find(".cgi") != std::string::npos) {
        return path;
    } else
        return "";
}

// savoir quel content-type est demandé et retirer le caractère cr(ascii 013) à la fin de la ligne
std::string parseContentType(const std::string&  buffer) 
{
    std::string contentType;
    std::string line;
    std::istringstream iss(buffer);
    std::string contentTrim;
    while (std::getline(iss, line)) 
    {
        if (line.find("Content-Type") != std::string::npos)
        {
            contentType = line.substr(14);
            for (size_t i = 0; i < contentType.length(); i++)
            {
                if (contentType[i] != 13)
                    contentTrim += contentType[i];
                else
                    break;
            }
            // std::cout << "Content-Type dans la fonction: " << contentTrim << std::endl;
            if (contentTrim.find("form-data") != std::string::npos)
                contentTrim = "multipart/form-data";
            break;
        }
    }
    // std::cout << "Content-Type a la fin de la fonction: " << contentTrim << std::endl;
    return contentTrim;
}

// recuperer les donnees du formulaire envoyees par le client qui sont apres la ligne vide du header
std::string parseFormData(const std::string&  buffer) {
    std::string formData;
    std::string line;
    std::istringstream iss(buffer);
    bool isFormData = false;
    while (std::getline(iss, line))
    {
        if (isFormData)
        {
            formData += line;
        }
        if (line == "\r")
        {
            isFormData = true;
        }
    }
    return formData;
}

// convertir les données du formulaire et les stocker dans une structure de données MAP
std::map<std::string, std::string> convertFormData(std::string form) {
    std::map<std::string, std::string> formData;
    std::string line;
    std::istringstream iss(form);
    while (std::getline(iss, line, '&'))
    {
        size_t pos = line.find("=");
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        formData[key] = value;
    }
    return formData;
}

// convertir les %ascii en caractères et convertir les + en espace
std::map<std::string, std::string> convertAscii(std::map<std::string, std::string> formMap) {
    std::map<std::string, std::string> formData;
    for (std::map<std::string, std::string>::iterator it = formMap.begin(); it != formMap.end(); ++it)
    {
        std::string key = it->first;
        std::string value = it->second;
        std::string newValue;
        for (size_t i = 0; i < value.length(); i++)
        {
            if (value[i] == '%')
            {
                std::string hex = value.substr(i + 1, 2);
                char c = strtol(hex.c_str(), 0, 16);
                newValue += c;
                i += 2;
            }
            else if (value[i] == '+')
            {
                newValue += ' ';
            }
            else
            {
                newValue += value[i];
            }
        }
        formData[key] = newValue;
    }
    return formData;
}

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

// Exécute un script CGI et renvoie la sortie !!! pas bon fusionner avec la fonction executeCGI_POST pour avoir une seule fonction
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

//Executer le script CGI via la methode POST
std::string executeCGI_POST(const std::string &scriptPath, const std::map<std::string, std::string> &formData) {
    std::stringstream ss;
    ss << CGI_BIN_PATH << scriptPath;
    std::string command = ss.str();

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "Error creating pipe.";
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "Error forking process.";
    } else if (pid == 0) { // Processus enfant
        close(pipefd[0]);

        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);

        std::string formDataString;
        for (std::map<std::string, std::string>::const_iterator it = formData.begin(); it != formData.end(); ++it) {
            formDataString += it->first + "=" + it->second + "&";
        }

        char* args[] = { const_cast<char*>(command.c_str()), const_cast<char*>(formDataString.c_str()), nullptr };
        char* envp[] = { nullptr };
        execve(command.c_str(), args, envp);
        perror("execve");
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);

        std::string result;
        char buffer[128];
        ssize_t bytesRead;

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(pipefd[0], &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        int status;
        int select_result = select(pipefd[0] + 1, &read_fds, NULL, NULL, &timeout);
        if (select_result == -1) {
            perror("select");
            close(pipefd[0]);
            return "Error waiting for CGI output.";
        } else if (select_result == 0) {
            close(pipefd[0]);
            return "Error_script";
        } else {
            bytesRead = read(pipefd[0], buffer, sizeof(buffer));
            while (bytesRead > 0) {
                result.append(buffer, bytesRead);
                bytesRead = read(pipefd[0], buffer, sizeof(buffer));
            }
        }

        waitpid(pid, &status, 0);

        close(pipefd[0]);

        return result;
    }
}

int main(int argc, char *argv[])
{
    std::map<std::string, int> urlToPort;

    // Ajoutez chaque URL avec son port correspondant à la map
    urlToPort["http://127.0.0.1/site/1/index.html"] = 8001; // site perso avec images et css redirection...
    urlToPort["http://127.0.0.1/site/2/index.html"] = 8002;
    urlToPort["http://127.0.0.1/site/3/index.html"] = 8003; // site avec JS 
    urlToPort["http://127.0.0.1/site/4/index.html"] = 8004; // cgi en methode GET
    urlToPort["http://127.0.0.1/site/5/index.html"] = 8005; // methode POST avec upload de fichier et suppression de fichier et formulaire
    urlToPort["http://127.0.0.1/site/6/index.html"] = 8006; // CGI en methode POST avec formulaire et simple script pour afficher l'heure

    int server_fd, new_socket;
    struct sockaddr_in address;
    ssize_t bytesRead;
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
                // Lecture des données du client faire une boucle pour lire tout le fd 
                std::string requestData;

                // Boucle pour lire les données du socket jusqu'à ce qu'il n'y ait plus de données disponibles
                while (true) {
                    char tempBuffer[BUFFER_SIZE];
                    bytesRead = read(sd, tempBuffer, BUFFER_SIZE);

                    if (bytesRead == -1) {
                        reportError("read failed");
                        close(sd);
                        clients[i] = 0;
                        break;
                    }

                    if (bytesRead == 0) {
                        std::cout << "Client déconnecté, socket fd " << sd << std::endl;
                        close(sd);
                        clients[i] = 0;
                        break;
                    }
                    requestData.append(tempBuffer, bytesRead);
                    if (bytesRead < BUFFER_SIZE) {
                        break;
                    }
                }

                // Après la boucle, requestData contient toutes les données de la requête


                // buffer[bytesRead] = '\0';
                std::cout << "Reçu : " << requestData << std::endl;
                // printf("Buffer pour controler : %s\n", buffer);
                std::cout << "---------------------" << std::endl;

                // faire le parsing ici des methodes et des chemins
                std::string method = parseMethod(requestData);

                // Utilisation d'un switch pour gérer les différentes méthodes HTTP
                if (method == "GET") {
                    // Extrait le chemin demandé à partir de la requête HTTP
                    std::cout << "GET request" << std::endl;
                    std::string request(buffer);
                    size_t start = requestData.find("GET ") + 4;
                    size_t end = requestData.find(" HTTP/");
                    std::string requestedPath = requestData.substr(start, end - start);

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
                    
                    // savoir quel content-type est utilisé (application/x-www-form-urlencoded ou multipart/form-data)
                    std::string contentType = parseContentType(requestData);
                    // si le content-type est different de application/x-www-form-urlencoded ou multipart/form-data envoyer une erreur au serveur 415 Unsupported Media Type
                    if (contentType != "application/x-www-form-urlencoded" && contentType != "multipart/form-data") {
                        std::string response = "HTTP/1.1 415 Unsupported Media Type\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
                        send(sd, response.c_str(), response.size(), 0);
                        close(sd);
                        clients[i] = 0;
                    } else {
                        // si le content-type est application/x-www-form-urlencoded ou multipart/form-data

                        // std::cout << contentType << std::endl;
                        if (contentType == "application/x-www-form-urlencoded") {
                            std::string form = parseFormData(requestData);
                            // savoir si c'est un script cgi et avoir le chemin du script
                            std::string scriptPath = parseScriptPath(requestData);
                            // convertir les données du formulaire et les stocker dans une structure de données MAP
                            std::map<std::string, std::string> formMap = convertFormData(form);
                            // convertir les %ascii en caractères
                            formMap = convertAscii(formMap);
                            if (scriptPath == "") {
                                // verifier si le formulaire est vide
                                if (form.empty()) {
                                    std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
                                    send(sd, response.c_str(), response.size(), 0);
                                    close(sd);
                                    clients[i] = 0;
                                }
                                // afficher les données du formulaire dans le terminale
                                for (std::map<std::string, std::string>::iterator it = formMap.begin(); it != formMap.end(); ++it) {
                                    std::cout << it->first << " => " << it->second << std::endl;
                                }
                                std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 37\r\n\r\n <h1>your form is send correctly </h1>";
                                send(sd, response.c_str(), response.size(), 0);
                                close(sd);
                                clients[i] = 0;
                            } if (scriptPath != "") {
                                std::string scriptCgiPost = scriptPath.substr(9);
                                std::cout << "Script CGI détecté en POST : " << scriptCgiPost << std::endl;
                                std::string cgiOutput = executeCGI_POST(scriptCgiPost, formMap);
                                if (cgiOutput == "Error_script")
                                {
                                    std::string response = "HTTP/1.1 504 Gateway Timeout\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
                                    send(sd, response.c_str(), response.size(), 0);
                                    close(sd);
                                    clients[i] = 0;
                                } else {
                                    std::string contentType = "text/html";
                                    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " +
                                                std::to_string(cgiOutput.size()) + "\r\n\r\n" + cgiOutput;
                                    send(sd, response.c_str(), response.size(), 0);
                                    close(sd);
                                    clients[i] = 0;
                                }
                            }
                        } else {
                            std::cout << "multipart/form-data" << std::endl;
                            std::vector<FileData> files = parseMultipartFormData(requestData);
                            
                            std::ofstream output(files[0].filename.c_str(), std::ios::binary);
                            output.write(files[0].content.c_str(), files[0].content.size());

                            //a refaire!!
                            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 37\r\n\r\n <h1>your multipart OK </h1>";
                            send(sd, response.c_str(), response.size(), 0);
                            close(sd);
                            clients[i] = 0;
                        }
                    }

                } else if (method == "DELETE") {
                    printf("DELETE request\n");
                    std::string parseDel = parseMethodDelete(requestData);
                    std::string path = "." + parseDel;

                    //suppression du fichier direct avec remove et erreur 404 si le fichier n'existe pas
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
                    // std::cerr << "Unsupported HTTP method: " << method << std::endl;
                }                
            }
        }
    }
    return 0;
}

/*
---------------------------41359768119475513041354677090
-----------------------------41359768119475513041354677090
-----------------------------407609466427608914477298209--
*/
