#include "ServerManager.hpp"

ServerManager::ServerManager() : max_fd(0) {
    FD_ZERO(&master_fds);
}

ServerManager::~ServerManager() {
    // Cleanup listening sockets
    for (std::map<int, int>::iterator it = listeningSockets.begin(); it != listeningSockets.end(); ++it) {
        close(it->first);
    }

    // Cleanup ServerBlock objects
    for (std::map<int, ServerBlock*>::iterator it = serverConfigs.begin(); it != serverConfigs.end(); ++it) {
        delete it->second;
    }
}

bool ServerManager::isRunning = true;

bool ServerManager::parseConfig(const char* configFile) {
    ServerBlock* result = parseConfigFile(configFile);
    if (result == NULL) {
        return false;
    }

    bool isFirstServer = true;
    bool hasPort80Server = false;

    for (ServerBlock* currentServer = result; currentServer != NULL; currentServer = currentServer->next) {
        int port = atoi(currentServer->getListen());
        serverConfigs[port] = currentServer;

        // Create listening socket for the specified port
        int server_fd = SocketManager::createListeningSocket(port);
        if (server_fd != -1) {
            listeningSockets[server_fd] = port;
            FD_SET(server_fd, &master_fds);
            if (server_fd > max_fd) max_fd = server_fd;
            std::cout << "Server listening on port " << port << std::endl;
        }

        if (port == 80) {
            hasPort80Server = true;
        }
    }

    if (!hasPort80Server && isFirstServer) {
        int server_fd_80 = SocketManager::createListeningSocket(80);
        if (server_fd_80 != -1) {
            ServerBlock* firstServer = result;
            listeningSockets[server_fd_80] = atoi(firstServer->getListen());
            FD_SET(server_fd_80, &master_fds);
            if (server_fd_80 > max_fd) max_fd = server_fd_80;
            std::cout << "First server listening on port 80" << std::endl;
        }
    }

    return true;
}

void ServerManager::run() {
    if (!isRunning) {
        std::cerr << "Server not running" << std::endl;
        return;
    }
    std::cout << "Server running" << std::endl;
    SocketManager socketManager;
    RequestHandler requestHandler;
    socketManager.watch(serverConfigs, listeningSockets, isRunning, master_fds, max_fd, requestHandler);


    std::cout << "Server stopped" << std::endl;
}