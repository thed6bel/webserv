#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP

#include "../parsing/serverBlock/serverBlock.hpp"
#include "../parsing/header.hpp"
#include "../tools.hpp"
#include "RequestHandler.hpp"
#include "ServerManager.hpp"


class SocketManager {
public:
    static int createListeningSocket(int port);
    void watch(std::map<int, ServerBlock*>& serverConfigs, std::map<int, int>& listeningSockets, bool& isRunning, fd_set& master_fds, int& max_fd, RequestHandler& requestHandler);

private:
    static int acceptConnection(int serverSocket);
};

#endif