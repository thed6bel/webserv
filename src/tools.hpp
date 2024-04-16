#ifndef TOOLS_HPP
#define TOOLS_HPP
#include "parsing/header.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <map>
#include <vector>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <signal.h>

#define MAX_CLIENTS 1024
#define CGI_BIN_PATH "./cgi-bin/"

struct FileData {
    std::string filename;
    std::string content;
    std::string contentType;
};

std::vector<FileData> parseMultipartFormData(const std::string& formData);
std::string parseScriptPath(std::string requestData);
std::string parseContentType(const std::string&  buffer);
std::string parseFormData(const std::string&  buffer);
std::map<std::string, std::string> convertFormData(std::string form);
std::map<std::string, std::string> convertAscii(std::map<std::string, std::string> formMap);
std::string parseMethod(const std::string& request);
std::string parseMethodDelete(const std::string& request);
std::string readFromFile(const std::string &filename);
std::string executeCGI_GET(const std::string &scriptPath, const std::string &queryString);
std::string executeCGI_POST(const std::string &scriptPath, const std::map<std::string, std::string> &formData);
std::string trim(const std::string& str);
bool iequals(const std::string &a, const std::string &b);
ServerBlock* findServerBlockByHostPort(const std::string& headers, std::map<int, ServerBlock*>& serverConfigs);
std::string getContentType(const std::string &filename);
LocationBlock* findMatchingLocationBlock(const std::string& requestedPath, ServerBlock* serverConfigRequest);
bool isMethodAllowed(int result, int methodNumber);
bool isDirectory(const std::string& path);
std::string urlEncode(const std::string& str);
std::string generateDirectoryListing(const std::string& directoryPath, const std::string& requestedPath);
bool containsAccentsOrSpaces(const std::string& input);
void sighandler(int signum);

#endif