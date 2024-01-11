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

#define PORT 80
#define BUFFER_SIZE 2048

std::string readFromFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (file.is_open()) {
        std::ostringstream oss;
        oss << file.rdbuf();
        return oss.str();
    }
    return "File not found!";
}

void reportError(const char* message) {
    std::cerr << "Error: " << message << std::endl;
}

std::string getContentType(const std::string& filename) {
    if (filename.find(".html") != std::string::npos) {
        return "text/html";
    } else if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos) {
        return "image/jpeg";
    } else if (filename.find(".png") != std::string::npos) {
        return "image/png";
    } else if (filename.find(".webp") != std::string::npos) {
        return "image/webp";
    } else if (filename.find(".ico") != std::string::npos) {
        return "image/x-icon";
    } else if (filename.find(".js") != std::string::npos) {
        return "application/javascript";
    } else if (filename.find(".css") != std::string::npos) {
        return "text/css";
    } else {
        return "application/octet-stream";
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Création du socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        reportError("socket failed");
        std::exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Attachement du socket à l'adresse et au port spécifiés
    if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) == -1) {
        reportError("bind failed");
        std::exit(EXIT_FAILURE);
    }

    // Passage en mode d'écoute
    if (listen(server_fd, 3) == -1) {
        reportError("listen failed");
        std::exit(EXIT_FAILURE);
    }

    std::cout << "Serveur en écoute sur le port " << PORT << "...\n";

    while (true) {
        // Attente d'une connexion entrante
        if ((new_socket = accept(server_fd, reinterpret_cast<struct sockaddr*>(&address), &addrlen)) == -1) {
            reportError("accept failed");
            std::exit(EXIT_FAILURE);
        }

        // Lecture des données du client
        ssize_t bytesRead = read(new_socket, buffer, BUFFER_SIZE);
        if (bytesRead == -1) {
            reportError("read failed");
            close(new_socket);
            continue;
        }

        buffer[bytesRead] = '\0';
        std::cout << "Reçu : " << buffer << std::endl;
        std::cout << "---------------------" << std::endl;

        // Extrait le chemin demandé à partir de la requête HTTP
        std::string request(buffer);
        size_t start = request.find("GET ") + 4;
        size_t end = request.find(" HTTP/");
        std::string requestedPath = request.substr(start, end - start);

        // Si le chemin est vide, charge la page d'accueil (index.html, voir plus tard le php et autre...)
        if (requestedPath.empty() || requestedPath == "/") {
            requestedPath = "/index.html";
        }

        // Chemin absolu du fichier demandé
        std::string filePath = "." + requestedPath;

        // Vérification de l'existence du fichier
        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) != 0) {
            std::string notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\nFile not found!";
            send(new_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
            close(new_socket);
            continue;
        }

        // Lecture du contenu du fichier
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::string internalServerErrorResponse = "HTTP/1.1 500 InternalServer Error\r\n\r\nInternal Server Error!";
            send(new_socket, internalServerErrorResponse.c_str(), internalServerErrorResponse.size(), 0);
            close(new_socket);
            continue;
        }

        std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // Construction de la réponse HTTP
        std::string contentType = getContentType(filePath);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent;

        // Envoi de la réponse au client
        send(new_socket, response.c_str(), response.size(), 0);
        close(new_socket);
    }

    return 0;
}