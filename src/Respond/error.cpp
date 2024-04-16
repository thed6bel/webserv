#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include "error.hpp"
#include "../parsing/serverBlock/serverBlock.hpp"
#include "../parsing/header.hpp"
#include "../parsing/serverBlock/locationBlock.hpp"


void replaceString(std::string& buffer, const std::string& searchString, const std::string& replaceString) {
    size_t pos = 0;

    while ((pos = buffer.find(searchString, pos)) != std::string::npos) {
        buffer.replace(pos, searchString.length(), replaceString);
        pos += replaceString.length();
    }
}

std::string RespondPage(int num, const std::string message, ServerBlock *server) {
    std::string ErrorNumber;
    std::string buffer;
    const char *port = server->getListen();
    char *path = server->getErrorPagePath(num, port);

    switch (num) {
		case 400:
			ErrorNumber = "400 Bad Request";
			break;
		case 403:
			ErrorNumber = "403 Forbidden";
			break;
		case 404:
			ErrorNumber = "404 Not Found ";
			break;
		case 405:
			ErrorNumber = "405 Method Not Allowed";
			break;
		case 408:
			ErrorNumber = "408 Request Timeout";
			break;
		case 413:
			ErrorNumber = "413 Payload Too Large";
			break;
		case 415:
			ErrorNumber = "415 Unsupported Media Type";
			break;
		case 500:
			ErrorNumber = "500 Internal Server Error";
			break;
		case 502:
			ErrorNumber = "502 Bad Gateway";
			break;
		case 504:
			ErrorNumber = "504 Gateway Timeout";
			break;
		case 200:
			ErrorNumber = "200 OK";
			break;
	}

    if (path != NULL) {
        std::string PathPunt = ".";
        PathPunt += path;
        std::ifstream inputFile(PathPunt.c_str());
        if (inputFile.is_open()) {
            std::string line;
            while (std::getline(inputFile, line)) {
                buffer += line + "\n";
            }
            inputFile.close();
        } else {
            std::cerr << "Erreur lors de l'ouverture du fichier d'erreur personnalisé." << std::endl;
        }
    }

    if (buffer.empty()) {
        std::ifstream DefaultErrorFile("src/Respond/error.html");
        if (DefaultErrorFile.is_open()) {
            std::string line;
            while (std::getline(DefaultErrorFile, line)) {
                buffer += line + "\n";
            }
            DefaultErrorFile.close();
        } else {
            std::cerr << "Erreur lors de l'ouverture du fichier d'erreur par défaut." << std::endl;
        }
        replaceString(buffer, "XXXXX", ErrorNumber);
        replaceString(buffer, "YYYYY", message);
    }

    std::string contentType = "text/html";
    std::ostringstream oss;
    oss << buffer.size();

    std::string response = "HTTP/1.1 " + ErrorNumber + "\r\nContent-Type: " + contentType + "\r\nContent-Length: " + oss.str() + "\r\n\r\n" + buffer;
	return response;
}
