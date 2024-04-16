#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../parsing/serverBlock/serverBlock.hpp"
#include "../parsing/header.hpp"
#include "../Respond/error.hpp"
#include "../tools.hpp"
#include "../Respond/error.hpp"


class RequestHandler {
public:
    void handleRequest(int client_socket, fd_set& master_fds, std::map<int, ServerBlock*>& serverConfigs, std::map<int, std::string>& client_responses);

private:

};

#endif