#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// Structure pour représenter une location
struct Location {
    std::string path;
    std::string allow_methods;
    std::string autoindex;
};

// Structure pour représenter un serveur
struct Server {
    int listen_port;
    std::string server_name;
    std::string host;
    std::string root;
    std::string index;
    std::string error_page;
    std::vector<Location> locations;
};

// Fonction de parsing
void parseConfig(const std::string &filename, std::vector<Server> &servers) {
    std::ifstream file(filename);
    std::string line;

    Server currentServer;
    Location currentLocation;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "server") {
            if (!currentServer.server_name.empty()) {
                servers.push_back(currentServer);
            }
            currentServer = Server();
            currentLocation = Location();
        } else if (keyword == "listen") {
            iss >> currentServer.listen_port;
        } else if (keyword == "server_name") {
            iss >> currentServer.server_name;
        } else if (keyword == "host") {
            iss >> currentServer.host;
        } else if (keyword == "root") {
            iss >> currentServer.root;
        } else if (keyword == "index") {
            iss >> currentServer.index;
        } else if (keyword == "error_page") {
            iss >> currentServer.error_page;
        } else if (keyword == "location") {
            iss >> currentLocation.path;
        } else if (keyword == "allow_methods") {
            std::string methods;
            std::getline(iss >> std::ws, methods);
            currentLocation.allow_methods = methods;
        } else if (keyword == "autoindex") {
            iss >> currentLocation.autoindex;
            currentServer.locations.push_back(currentLocation);
}
    }

    if (!currentServer.server_name.empty()) {
        servers.push_back(currentServer);
    }
}

int main(int argc, char *argv[]) {
    std::vector<Server> servers;
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return EXIT_FAILURE;
    }

    // std::string configFile = argv[1];
    parseConfig(argv[1], servers);

    for (const auto &server : servers) {
        std::cout << "Server: Listen Port - " << server.listen_port << std::endl
                  << "Server Name - " << server.server_name << std::endl
                  << "Host - " << server.host << std::endl
                  << "Root - " << server.root << std::endl
                  << "Index - " << server.index << std::endl
                  << "Error Page - " << server.error_page
                  << std::endl;

        for (const auto &location : server.locations) {
            std::cout << "  Location: Path - " << location.path << std::endl
                      << "  Allow Methods - " << location.allow_methods << std::endl
                      << "  Autoindex - " << location.autoindex
                      << std::endl;
        }
    }

    return 0;
}
