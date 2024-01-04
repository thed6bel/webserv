#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

std::string readFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::ostringstream oss;
        oss << file.rdbuf();
        return oss.str();
    }
    return "File not found!";
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Création du socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Attachement du socket à l'adresse et au port spécifiés
    if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Passage en mode d'écoute
    if (listen(server_fd, 3) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Serveur en écoute sur le port " << PORT << "...\n";

    while (true) {
        // Attente d'une connexion entrante
        if ((new_socket = accept(server_fd, reinterpret_cast<struct sockaddr*>(&address), reinterpret_cast<socklen_t*>(&addrlen))) == -1) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Lecture des données du client
        read(new_socket, buffer, BUFFER_SIZE);
        std::cout << "Reçu : " << buffer << std::endl;

        // Extrait le chemin demandé à partir de la requête HTTP
        std::string request(buffer);
        size_t start = request.find("GET ") + 4;
        size_t end = request.find(" HTTP/");
        std::string requestedPath = request.substr(start, end - start);

        // Si le chemin est vide, charge la page d'accueil (index.html)
        if (requestedPath.empty() || requestedPath == "/") {
            requestedPath = "/index.html";
        }

        // Lecture du contenu du fichier HTML
        std::string htmlContent = readFromFile("." + requestedPath);

        // Envoi de la réponse au client
        std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " + std::to_string(htmlContent.size()) + "\n\n" + htmlContent;
        send(new_socket, response.c_str(), response.size(), 0);
        std::cout << "Réponse envoyée\n";

        // Fermeture du socket client
        close(new_socket);
    }

    // Fermeture du socket serveur (cette ligne ne sera jamais atteinte dans la boucle infinie)
    close(server_fd);

    return 0;
}
