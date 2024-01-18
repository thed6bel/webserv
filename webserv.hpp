#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "parsing.hpp"

class WebServer {
public:
    WebServer(const std::vector<ServerConfig>& serverConfigs);
    ~WebServer();

    void start();

private:
    int server_fd;
    std::vector<int> clients;
    std::vector<ServerConfig> serverConfigs;
    int port;

    static const int BUFFER_SIZE = 2048;
    static const int MAX_CLIENTS = 10;
    static const int RETRY_INTERVAL = 10;

    static std::string readFromFile(const std::string& filename);
    static void reportError(const char* message);
    static std::string getContentType(const std::string& filename);

    void initializeServer();
    void handleNewConnection();
    void handleClientData(int clientSocket);
    void sendResponse(int clientSocket, const std::string& response);
};

#endif // WEBSERV_HPP
