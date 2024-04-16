#include "tools.hpp"
#include "server/ServerManager.hpp"

// Fonction pour extraire les données multipart/form-data
std::vector<FileData> parseMultipartFormData(const std::string& formData) {
    std::vector<FileData> files;
    std::string delimiter = "\r\n-----------------------------";

    size_t pos = formData.find(delimiter);
    if (pos == std::string::npos)
        return files;
    while (pos != std::string::npos) {
        size_t filenameStart = formData.find("filename=\"", pos);
        if (filenameStart == std::string::npos) {
            break;
        }
        filenameStart += 10;

        size_t filenameEnd = formData.find("\"", filenameStart);
        if (filenameEnd == std::string::npos) {
            break;
        }
        std::string filename = formData.substr(filenameStart, filenameEnd - filenameStart);

        size_t contentStart = formData.find("\r\n\r\n", filenameEnd);
        if (contentStart == std::string::npos)
            break;
        contentStart += 4;

        size_t contentEnd = formData.find(delimiter, contentStart);
        if (contentEnd == std::string::npos)
            break;
        std::string content = formData.substr(contentStart, contentEnd - contentStart);

        files.push_back(FileData());
        files.back().filename = filename;
        files.back().content = content;
        
        pos = formData.find(delimiter, contentEnd);
    }
    return files;
}

std::string parseScriptPath(std::string requestData) {
    std::string path;
    std::istringstream iss(requestData);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("POST") != std::string::npos) {
            size_t start = line.find("POST") + 5;
            size_t end = line.find("HTTP/") - 1;
            path = line.substr(start, end - start);
            break;
        }
    }
    //si .cgi existe return path
    if (path.find(".pl") != std::string::npos || path.find(".py") != std::string::npos) {
        return path;
    } else
        return "";
}

// savoir quel content-type est demandé et retirer le caractère cr(ascii 013) à la fin de la ligne
std::string parseContentType(const std::string&  buffer) {
    std::string contentType;
    std::string line;
    std::istringstream iss(buffer);
    std::string contentTrim;
    while (std::getline(iss, line)) {
        if (line.find("Content-Type") != std::string::npos)
        {
            contentType = line.substr(14);
            for (size_t i = 0; i < contentType.length(); i++) {
                if (contentType[i] != 13)
                    contentTrim += contentType[i];
                else
                    break;
            }
            if (contentTrim.find("form-data") != std::string::npos)
                contentTrim = "multipart/form-data";
            break;
        }
    }
    return contentTrim;
}

// convertir les données du formulaire et les stocker dans une structure de données MAP
std::map<std::string, std::string> convertFormData(std::string form) {
    std::map<std::string, std::string> formData;
    std::string line;
    std::istringstream iss(form);
    while (std::getline(iss, line, '&')) {
        size_t pos = line.find("=");
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        formData[key] = value;
    }
    return formData;
}

// convertir les %ascii en caractères et convertir les + en espace
std::map<std::string, std::string> convertAscii(std::map<std::string, std::string> formMap) {
    std::map<std::string, std::string> formData;
    for (std::map<std::string, std::string>::iterator it = formMap.begin(); it != formMap.end(); ++it) {
        std::string key = it->first;
        std::string value = it->second;
        std::string newValue;
        for (size_t i = 0; i < value.length(); i++) {
            if (value[i] == '%') {
                std::string hex = value.substr(i + 1, 2);
                char c = strtol(hex.c_str(), 0, 16);
                newValue += c;
                i += 2;
            } else if (value[i] == '+') {
                newValue += ' ';
            } else {
                newValue += value[i];
            }
        }
        formData[key] = newValue;
    }
    return formData;
}

// Fonction pour le parsing de la méthode de la requête HTTP
std::string parseMethod(const std::string& request) {
    std::istringstream iss(request);
    std::string method;
    iss >> method;
    return method;
}

// Fonction pour la lecture de la methode DELETE
std::string parseMethodDelete(const std::string& request) {
    std::string requ(request);
    size_t start = requ.find("DELETE ") + 7;
    size_t end = requ.find(" HTTP/");
    std::string requestedPath = requ.substr(start, end - start);
    return requestedPath;
}

std::string readFromFile(const std::string &filename) {
    std::ifstream file(filename.c_str());
    if (file.is_open()) {
        std::ostringstream oss;
        oss << file.rdbuf();
        return oss.str();
    }
    return "File not found!";
}

std::string executeCGI_GET(const std::string &scriptPath, const std::string &queryString) {
    std::stringstream ss;
    ss << CGI_BIN_PATH << scriptPath;
    std::string command = ss.str();

    struct stat st;
    if (stat(command.c_str(), &st) == -1) {
        return "404";
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "500";
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "500";
    } else if (pid == 0) { // Processus enfant
        close(pipefd[0]);

        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);

        setenv("QUERY_STRING", queryString.c_str(), 1);

        char* args[] = { const_cast<char*>(command.c_str()), const_cast<char*>(queryString.c_str()), NULL };
        char* envp[] = { NULL };
        execve(command.c_str(), args, envp);
        perror("execve");
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);

        std::string result;
        char buffer[128];
        ssize_t bytesRead;

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(pipefd[0], &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        int status;
        int select_result = select(pipefd[0] + 1, &read_fds, NULL, NULL, &timeout);
        if (select_result == -1) {
            perror("select");
            close(pipefd[0]);
            return "500";
        } else if (select_result == 0) {
            close(pipefd[0]);
            return "504";
        } else {
            bytesRead = read(pipefd[0], buffer, sizeof(buffer));
            while (bytesRead > 0) {
                result.append(buffer, bytesRead);
                bytesRead = read(pipefd[0], buffer, sizeof(buffer));
            }
        }

        waitpid(pid, &status, 0);
        close(pipefd[0]);

        if (result.find("<!DOCTYPE") == std::string::npos) {
            return "500";
        }
        return result;
    }
}

std::string executeCGI_POST(const std::string &scriptPath, const std::map<std::string, std::string> &formData) {
    std::stringstream ss;
    ss << CGI_BIN_PATH << scriptPath;
    std::string command = ss.str();

    struct stat st;
    if (stat(command.c_str(), &st) == -1) {
        return "404";
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "500";
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "500";
    } else if (pid == 0) { // Processus enfant
        close(pipefd[0]);

        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);

        std::string formDataString;
        for (std::map<std::string, std::string>::const_iterator it = formData.begin(); it != formData.end(); ++it) {
            formDataString += it->first + "=" + it->second + "&";
        }

        char* args[] = { const_cast<char*>(command.c_str()), const_cast<char*>(formDataString.c_str()), NULL };
        char* envp[] = { NULL };
        execve(command.c_str(), args, envp);
        perror("execve");
        exit(EXIT_FAILURE);
    } else {
        close(pipefd[1]);

        std::string result;
        char buffer[128];
        ssize_t bytesRead;

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(pipefd[0], &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        int status;
        int select_result = select(pipefd[0] + 1, &read_fds, NULL, NULL, &timeout);
        if (select_result == -1) {
            perror("select");
            close(pipefd[0]);
            return "500";
        } else if (select_result == 0) {
            close(pipefd[0]);
            return "504";
        } else {
            bytesRead = read(pipefd[0], buffer, sizeof(buffer));
            while (bytesRead > 0) {
                result.append(buffer, bytesRead);
                bytesRead = read(pipefd[0], buffer, sizeof(buffer));
            }
        }

        waitpid(pid, &status, 0);

        close(pipefd[0]);

        if (result.find("<!DOCTYPE") == std::string::npos)
            return "500";

        return result;
    }
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

bool iequals(const std::string &a, const std::string &b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (tolower(a[i]) != tolower(b[i])) return false;
    }
    return true;
}

ServerBlock* findServerBlockByHostPort(const std::string& headers, std::map<int, ServerBlock*>& serverConfigs) {
    std::istringstream headerStream(headers);
    std::string line;
    std::string hostValue;
    int port = 0;

    // Extraire l'en-tete Host
    while (std::getline(headerStream, line)) {
        if (line.find("Host:") != std::string::npos) {
            hostValue = trim(line.substr(line.find(":") + 1));
            size_t colonPos = hostValue.find(":");
            if (colonPos != std::string::npos) {
                std::string portStr = hostValue.substr(colonPos + 1);
                port = std::atoi(portStr.c_str());
                break;
            }
        } 
    }
    if (port == 0)
        port = 80;

    // Recherche du ServerBlock correspondant par port ou au port 80
    if (serverConfigs.find(port) != serverConfigs.end()) {
        return serverConfigs[port];
    } else if (port == 80) { 
        for (std::map<int, ServerBlock*>::const_iterator it = serverConfigs.begin(); it != serverConfigs.end(); ++it) {
            std::string serverName = trim(it->second->getServerName());
            if (iequals(serverName, hostValue)) {
                return it->second;
            }
        }
        return serverConfigs.begin()->second;
    }
    return NULL;
}

std::string getContentType(const std::string &filename) {
    if (filename.find(".html") != std::string::npos)
        return "text/html";
    else if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos)
        return "image/jpeg";
    else if (filename.find(".png") != std::string::npos)
        return "image/png";
    else if (filename.find(".webp") != std::string::npos)
        return "image/webp";
    else if (filename.find(".ico") != std::string::npos)
        return "image/x-icon";
    else if (filename.find(".js") != std::string::npos)
        return "application/javascript";
    else if (filename.find(".css") != std::string::npos)
        return "text/css";
    else
        return "application/octet-stream";
}

LocationBlock* findMatchingLocationBlock(const std::string& requestedPath, ServerBlock* serverConfigRequest) {
    LocationBlock* bestMatch = NULL;
    std::string bestMatchPath;

    if (serverConfigRequest != NULL) {
        for (LocationBlock* location = serverConfigRequest->firstLocation; location != NULL; location = location->next) {
            std::string locationPath = location->getURL();

            if (requestedPath.compare(0, locationPath.length(), locationPath) == 0) {
                std::string remainingPath = requestedPath.substr(locationPath.length());

                if (remainingPath.empty() || remainingPath[0] == '/') {
                    if (locationPath.length() > bestMatchPath.length()) {
                        bestMatch = location;
                        bestMatchPath = locationPath;
                    }
                }
            }
        }
    }
    return bestMatch;
}

bool isMethodAllowed(int result, int methodNumber) {
    return result % methodNumber == 0;
}

bool isDirectory(const std::string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

std::string urlEncode(const std::string& str) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = str.begin(), n = str.end(); i != n; ++i) {
        std::string::value_type c = (*i);

        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }
    return escaped.str();
}

std::string generateDirectoryListing(const std::string& directoryPath, const std::string& requestedPath) {
    DIR* dir = opendir(requestedPath.c_str());
    struct dirent* ent;

    std::string directoryPathWithoutDot = directoryPath.substr(1, directoryPath.size() - 1);
    std::size_t webRootPos = directoryPath.find(directoryPathWithoutDot);
    std::string webPath;
    if (webRootPos != std::string::npos) {
        webPath = directoryPath.substr(webRootPos);
    }

    std::string htmlContent = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head><body><h2>Index of " + directoryPathWithoutDot + "</h2><ul>";

    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                std::string fileName = std::string(ent->d_name);
                std::string hrefPath = webPath;
                if (!hrefPath.empty() && hrefPath[hrefPath.size() - 1] != '/') {
                    hrefPath += '/';
                }
                hrefPath += fileName;
                htmlContent += "<li><a href=\"" + hrefPath + "\" download=\"" + fileName + "\">" + fileName + "</a></li>";
            }
        }
        closedir(dir);
    } else {
        htmlContent += "<p>Cannot open directory.</p>";
    }

    htmlContent += "</ul></body></html>";
    return htmlContent;
}

bool containsAccentsOrSpaces(const std::string& input) {
    const std::string accents = "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞß"
                                "àáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ";

    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == ' ' || accents.find(input[i]) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void sighandler(int signum)
{
	if (signum == SIGINT) {
        ServerManager::isRunning = false;
        std::cout << "\rShuting down.." << std::endl;
	}
}