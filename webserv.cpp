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

    // Lecture du contenu du fichier
std::ifstream file("." + requestedPath, std::ios::binary);
if (file.is_open()) {
    std::ostringstream oss;
    oss << file.rdbuf();

    // Envoi de la réponse au client
    std::string response = "HTTP/1.1 200 OK\r\n";

    // Détermination du type de contenu en fonction de l'extension du fichier
    if (requestedPath.find(".html") != std::string::npos) {
        response += "Content-Type: text/html\r\n";
    } else if (requestedPath.find(".jpg") != std::string::npos || requestedPath.find(".jpeg") != std::string::npos) {
        response += "Content-Type: image/jpeg\r\n";
    } else if (requestedPath.find(".png") != std::string::npos) {
        response += "Content-Type: image/png\r\n";
    } else if (requestedPath.find(".webp") != std::string::npos) {
        response += "Content-Type: image/webp\r\n";
    } else if (requestedPath.find(".ico") != std::string::npos) {
        response += "Content-Type: image/x-icon\r\n";
    } else if (requestedPath.find(".js") != std::string::npos) {
        response += "Content-Type: application/javascript\r\n";
    } else if (requestedPath.find(".css") != std::string::npos) {
        response += "Content-Type: text/css\r\n";
    } else {
        response += "Content-Type: application/octet-stream\r\n";
    }

    response += "Content-Length: " + std::to_string(oss.str().size()) + "\r\n\r\n" + oss.str();

    ssize_t bytesSent = send(new_socket, response.c_str(), response.size(), 0);
    if (bytesSent == -1) {
        reportError("send failed");
    } else {
        std::cout << "Réponse envoyée\n";
    }
} else {
    // Gestion du cas où le fichier n'est pas trouvé
    std::string notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\nFile not found!";
    send(new_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
}

    // Fermeture du socket client
    close(new_socket);
    }

    // Fermeture du socket serveur (cette ligne ne sera jamais atteinte dans la boucle infinie)
    close(server_fd);

    return 0;
}
