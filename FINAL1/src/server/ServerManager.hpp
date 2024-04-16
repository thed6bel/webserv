#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../parsing/serverBlock/serverBlock.hpp"
#include "../parsing/header.hpp"
#include "../tools.hpp"
#include "RequestHandler.hpp"
#include "SocketManager.hpp"


class ServerManager {
public:
    ServerManager();
    ~ServerManager();
    bool parseConfig(const char* configFile);
    void run();
    std::map<int, ServerBlock*>& getServerConfigs() { return serverConfigs; }
    std::map<int, int>& getListeningSockets() { return listeningSockets; }
    fd_set& getMasterFds() { return master_fds; }
    int& getMaxFd() { return max_fd; }
    static bool& getIsRunning() { return isRunning; }
    void setIsRunning(bool value) { isRunning = value; }
    static bool isRunning;

private:
    std::map<int, ServerBlock*> serverConfigs;
    std::map<int, int> listeningSockets;
    fd_set master_fds;
    int max_fd;
    // RequestHandler requestHandler;
};

#endif