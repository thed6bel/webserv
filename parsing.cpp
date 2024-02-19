#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

struct LocationConfig {
    std::vector<std::string> allow_methods;
    std::string autoindex;
    std::string index;
    std::string root_rep;
    std::string ret;
};

struct ServerConfig {
    int listen_port;
    std::string server_name;
    std::string host;
    std::string root;
    std::string global_index;
    std::map<int, std::string> error_pages;
    std::map<std::string, LocationConfig> location;
};

// Fonction pour afficher la configuration du serveur
void printServerConfig(const ServerConfig& config) {
    std::cout << "Listen Port: " << config.listen_port << std::endl;
    std::cout << "Server Name: " << config.server_name << std::endl;
    std::cout << "Host: " << config.host << std::endl;
    std::cout << "Root: " << config.root << std::endl;
    std::cout << "Index: " << (config.global_index.empty() ? "Not defined" : config.global_index) << std::endl;

    std::cout << "Error Pages:" << std::endl;
    for (const auto& errorPage : config.error_pages) {
        std::cout << "  Error Page: " << errorPage.first << " : " << errorPage.second << std::endl;
    }

    std::cout << "Location Configurations:" << std::endl;
    for (const auto& loc : config.location) {
        std::cout << "  " << loc.first << ": ";
        const LocationConfig& locConfig = loc.second;
        std::cout << "allow_methods: ";
        for (const auto& method : locConfig.allow_methods) {
            std::cout << method << " ";
        }
        std::cout << "autoindex " << (locConfig.autoindex.empty() ? "Not defined" : locConfig.autoindex) << " ";
        std::cout << "index " << (locConfig.index.empty() ? "Not defined" : locConfig.index) << " ";
        std::cout << "root " << (locConfig.root_rep.empty() ? "Not defined" : locConfig.root_rep) << " ";
        std::cout << "return " << (locConfig.ret.empty() ? "Not defined" : locConfig.ret) << std::endl;
    }
}

// Fonction pour effectuer le parsing du fichier de configuration
bool parseConfigFile(const std::string& filename, std::vector<ServerConfig>& serverConfigs) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier de configuration." << std::endl;
        return false;
    }

    std::string line;
    std::string currentLocation;
    bool globalIndexSet = false;

    while (std::getline(file, line)) {
        // Supprimer les espaces en début et fin de la ligne
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));

        // Supprimer le point-virgule en fin de ligne, s'il y en a un
        if (!line.empty() && line.back() == ';') {
            line.pop_back();
        }

        std::istringstream iss(line);
        std::string key, value;
        iss >> key;

        if (key == "server") {
            // Nouvelle configuration de serveur détectée
            serverConfigs.push_back(ServerConfig());
            globalIndexSet = false;  // Réinitialiser l'indicateur d'index global
            continue;
        } else if (serverConfigs.empty()) {
            // Ignorer les lignes en dehors des blocs "server"
            continue;
        }

        ServerConfig& currentServer = serverConfigs.back();

        if (key == "listen") {
            iss >> currentServer.listen_port;
        } else if (key == "server_name") {
            iss >> currentServer.server_name;
        } else if (key == "host") {
            iss >> currentServer.host;
            if (!currentServer.host.empty() && currentServer.host.back() == '.') {
                currentServer.host.pop_back();
            }
        } else if (key == "root") {
            iss >> currentServer.root;
        } else if (key == "index" && !globalIndexSet) {
            iss >> currentServer.global_index;
            globalIndexSet = true;
        } else if (key == "error_page") {
            int error_code;
            iss >> error_code >> value;
            currentServer.error_pages[error_code] = value;
        } else if (key == "location") {
            std::getline(iss, currentLocation, '{');
            currentLocation = currentLocation.substr(0, currentLocation.find_last_not_of(" \t\n\r\f\v") + 1);
            currentLocation.erase(0, currentLocation.find_first_not_of(" \t\n\r\f\v"));
            continue;
        } else if (key == "root" && !currentLocation.empty()) {
            iss >> currentServer.location[currentLocation].root_rep;
        } else if (key == "allow_methods" && !currentLocation.empty()) {
            while (iss >> value) {
                currentServer.location[currentLocation].allow_methods.push_back(value);
            }
        } else if (key == "autoindex" && !currentLocation.empty()) {
            iss >> currentServer.location[currentLocation].autoindex;
        } else if (key == "index" && !currentLocation.empty()) {
            iss >> currentServer.location[currentLocation].index;
        } else if (key == "return" && !currentLocation.empty()) {
            iss >> currentServer.location[currentLocation].ret;
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    std::vector<ServerConfig> serverConfigs;
    if (parseConfigFile(argv[1], serverConfigs)) {
        for (const auto& config : serverConfigs) {
            std::cout << "==== Server Configuration ====" << std::endl;
            printServerConfig(config);
            std::cout << std::endl;
        }
    }

    return 0;
}
