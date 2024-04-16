#include "src/parsing/header.hpp"
#include "src/parsing/serverBlock/serverBlock.hpp"
#include "src/server/RequestHandler.hpp"
#include "src/server/SocketManager.hpp"
#include "src/server/ServerManager.hpp"
#include "src/parsing/header.hpp"
#include "src/tools.hpp"
#include <csignal>

int main(int argc, char* argv[]) {
    const char* configFile = argc > 1 ? argv[1] : "configFiles/defaultConf";
    ServerManager serverManager;

    signal(SIGINT, sighandler);

    if (!serverManager.parseConfig(configFile)) {
        std::cerr << "Error parsing config file" << std::endl;
        return 1;
    }
    serverManager.run();
    return EXIT_SUCCESS;
}
