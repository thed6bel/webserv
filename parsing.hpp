#ifndef PARSING_HPP
#define PARSING_HPP

#include <iostream>
#include <vector>
#include <map>

struct LocationConfig {
    std::vector<std::string> allow_methods;
    std::string autoindex;
    std::string index;
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

void printServerConfig(const ServerConfig& config);
bool parseConfigFile(const std::string& filename, std::vector<ServerConfig>& serverConfigs);

#endif
