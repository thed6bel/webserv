#include "SocketManager.hpp"

int SocketManager::createListeningSocket(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "socket failed" << std::endl;
        return -1;
    }

    int on = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            std::cerr << "setsockopt failed" << std::endl;
            close(server_fd);
            return -1;
        }

    fcntl(server_fd, F_SETFL, O_NONBLOCK | FD_CLOEXEC);

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "bind failed" << std::endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        std::cerr << "listen failed" << std::endl;
        close(server_fd);
        return -1;
    }
    return server_fd;
}

void SocketManager::watch(std::map<int, ServerBlock*>& serverConfigs, std::map<int, int>& listeningSockets, bool& isRunning, fd_set& master_fds, int& max_fd, RequestHandler& requestHandler) {
    int activity, curr_fd, client_fd;
    fd_set read_fds, write_fds, except_fds;
    std::map<int, std::string> client_responses;
    struct timeval timeout;

    do {
        read_fds = master_fds;
        write_fds = master_fds;
        except_fds = master_fds;

        // Set timeout for select
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        // Wait for activity on any of the sockets
        activity = select(max_fd+1, &read_fds, &write_fds, &except_fds, &timeout);
        if (activity < 0) {
            if (!ServerManager::getIsRunning())
                break;
            std::cerr << "select error" << std::endl;
            break;
        }

        // Identify which socket has activity
        for (curr_fd = 0; curr_fd <= max_fd; curr_fd++) {
            if (FD_ISSET(curr_fd, &read_fds)) {
                // Server = new TCP connection
                if (listeningSockets.find(curr_fd) != listeningSockets.end()) {
                    try {
                        client_fd = acceptConnection(curr_fd);
                        FD_SET(client_fd, &master_fds);
                        if (client_fd > max_fd)
                            max_fd = client_fd;
                    } catch(std::exception& e) {
                        std::cerr << "Error accepting connection" << std::endl;
                    }
                } else {
                    try {
                        requestHandler.handleRequest(curr_fd, master_fds, serverConfigs, client_responses);
                    } catch(std::exception& e) {
                        close(curr_fd);
                        FD_CLR(curr_fd, &master_fds);
                    }
                }
            }

            if (FD_ISSET(curr_fd, &write_fds)) {
                // Handle writable sockets
                if (client_responses.find(curr_fd) != client_responses.end()) {
                    std::string response = client_responses[curr_fd];
                    ssize_t bytes_sent = send(curr_fd, response.c_str(), response.length(), 0);
                    
                    if (bytes_sent == -1) {
                        // Erreur lors de l'envoi des données
                        close(curr_fd);
                        FD_CLR(curr_fd, &master_fds);
                        client_responses.erase(curr_fd);
                    } else if (bytes_sent == 0) {
                        // La connexion a été fermée par le client
                        close(curr_fd);
                        FD_CLR(curr_fd, &master_fds);
                        client_responses.erase(curr_fd);
                    } else if (bytes_sent < static_cast<ssize_t>(response.length())) {
                        // Seules certaines données ont été envoyées 
                        client_responses[curr_fd] = response.substr(bytes_sent);
                    } else {
                        // Toutes les données ont été envoyées avec succès youpi
                        close(curr_fd);
                        FD_CLR(curr_fd, &master_fds);
                        client_responses.erase(curr_fd);
                    }
                }
            }

            if (FD_ISSET(curr_fd, &except_fds)) {
                // Handle exceptional conditions
                std::cerr << "Exceptional condition on socket " << curr_fd << std::endl;
            }
        }
    } while (isRunning);

    std::cout << "Server shutting down" << std::endl;
    for (curr_fd = 0; curr_fd <= max_fd; ++curr_fd) {
        if (FD_ISSET(curr_fd, &master_fds))
            close(curr_fd);
    }
    exit(1);
}

int SocketManager::acceptConnection(int serverSocket) {
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    int client_socket = accept(serverSocket, (struct sockaddr *)&addr, &addrLen);
    if (client_socket < 0) 
        throw std::runtime_error("Accept failed");
    return client_socket;
}