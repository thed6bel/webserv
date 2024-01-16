#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

struct LocationConfig {
    std::string path;
    std::string allowMethods;
    std::string autoindex;
    std::string index;  // Déplacer l'index ici
};

struct ServerConfig {
    int listenPort;
    std::string serverName;
    std::string host;
    std::string root;
    std::string index;
    int errorPage;
    std::vector<LocationConfig> locations;
};

struct Config {
    std::vector<ServerConfig> servers;
};

void parseConfig(const std::string &configFile, Config &config) {
    std::ifstream file(configFile);
    std::string line;
    ServerConfig currentServer;
    LocationConfig currentLocation;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "server") {
            if (currentServer.listenPort != 0) {
                config.servers.push_back(currentServer);
            }
            currentServer = ServerConfig();
            iss >> keyword; // Pour consommer la "{"
        } else if (keyword == "listen") {
            iss >> currentServer.listenPort;
        } else if (keyword == "server_name") {
            iss >> currentServer.serverName;
        } else if (keyword == "host") {
            iss >> currentServer.host;
        } else if (keyword == "root") {
            iss >> currentServer.root;
        } else if (keyword == "index") {
            iss >> currentServer.index;
        } else if (keyword == "error_page") {
            iss >> currentServer.errorPage;
        } else if (keyword == "location") {
            currentLocation = LocationConfig(); // Réinitialisation de currentLocation
            iss >> currentLocation.path;
            iss >> keyword; // Pour consommer la "{"
        } else if (keyword == "allow_methods") {
            std::getline(iss >> std::ws, currentLocation.allowMethods);
        } else if (keyword == "autoindex") {
            iss >> currentLocation.autoindex;
        } else if (keyword == "index") {
            iss >> currentLocation.index;
        } else if (keyword == "}") {
            currentServer.locations.push_back(currentLocation);
        }
    }

    if (currentServer.listenPort != 0) {
        config.servers.push_back(currentServer);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string configFile = argv[1];
    Config servers;
    parseConfig(configFile, servers);

    // Affichage des résultats
    for (const auto &server : servers.servers) {
        std::cout << "Server: Listen Port - " << server.listenPort << std::endl;
        std::cout << "Server Name - " << server.serverName << std::endl;
        std::cout << "Host - " << server.host << std::endl;
        std::cout << "Root - " << server.root << std::endl;
        std::cout << "Index - " << server.index << std::endl;
        std::cout << "Error Page - " << server.errorPage << std::endl;

        for (const auto &location : server.locations) {
            std::cout << "  Location: Path - " << location.path << std::endl;
            std::cout << "  Allow Methods - " << location.allowMethods << std::endl;
            std::cout << "  Autoindex - " << location.autoindex << std::endl;
            std::cout << "  Index - " << location.index << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
