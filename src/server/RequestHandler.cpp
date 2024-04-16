#include "RequestHandler.hpp"

std::string readRequestData(int client_socket, fd_set& master_fds, ServerBlock*& serverConfigRequest, std::map<int, ServerBlock*>& serverConfigs) {
    std::string requestData;
    char buffer[2048];
    ssize_t bytes_read;
    bool headersComplete = false;
    size_t contentLength = 0;
    std::string::size_type headerEndPos = std::string::npos;

    while (true) {
        bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read == -1) {
            std::cerr << "Error reading from client socket" << std::endl;
            return "";
        } else if (bytes_read == 0) {
            std::cout << "Connection closed by client." << std::endl;
            close(client_socket);
            FD_CLR(client_socket, &master_fds);
            return requestData;
        } else {
            requestData.append(buffer, bytes_read);

            if (!headersComplete) {
                headerEndPos = requestData.find("\r\n\r\n");
                if (headerEndPos != std::string::npos) {
                    std::string requestHeaders = requestData.substr(0, headerEndPos + 4);
                    headersComplete = true;
                    std::string::size_type clPos = requestHeaders.find("Content-Length:");
                    if (clPos != std::string::npos) {
                        std::istringstream iss(requestHeaders.substr(clPos + 16));
                        iss >> contentLength;
                    }
                    serverConfigRequest = findServerBlockByHostPort(requestHeaders, serverConfigs);
                    if (serverConfigRequest == NULL) {
                        std::cerr << "No server block found for request" << std::endl;
                        return "";
                    }
                }
            }

            if (headersComplete && headerEndPos != std::string::npos && requestData.size() - (headerEndPos + 4) >= contentLength) {
                break;
            }
        }
    }

    return requestData;
}

void handleGetRequest(const std::string& requestHeaders, int client_socket, std::map<int, std::string>& client_responses, ServerBlock* serverConfigRequest) {
    size_t start = requestHeaders.find("GET ") + 4;
    size_t end = requestHeaders.find(" HTTP/");
    std::string requestedPath = requestHeaders.substr(start, end - start);

    LocationBlock* locationBlock = findMatchingLocationBlock(requestedPath, serverConfigRequest);
    if (locationBlock != NULL) {
        const char* returnPageCStr = locationBlock->getReturnPage();
        if (returnPageCStr && std::strlen(returnPageCStr) > 0) {
            std::string returnPage(returnPageCStr);
            if (returnPage.find("http://") == 0 || returnPage.find("https://") == 0) {
                std::string response = "HTTP/1.1 301 Moved Permanently\r\nLocation: " + returnPage + "\r\nContent-Length: 0\r\n\r\n";
                client_responses[client_socket] = response;
                return;
            } else {
                std::string response = "HTTP/1.1 302 Found\r\nLocation: " + returnPage + "\r\nContent-Length: 0\r\n\r\n";
                client_responses[client_socket] = response;
                return;
            }
        }

        if (!(isMethodAllowed(locationBlock->getLimitExcept(), 2))) {
            std::string response = RespondPage(403, "Forbidden", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        }

        std::string ctrlPath = ".";
        ctrlPath.append(serverConfigRequest->getRoot());
        ctrlPath.append(requestedPath);
        if (std::string(locationBlock->getAutoIndex()) == "on" && isDirectory(ctrlPath) == true ) {
            std::string fullPath = ".";
            // fullPath.append(serverConfigRequest->getRoot());
            fullPath.append(requestedPath);
            std::string directoryListing = generateDirectoryListing(fullPath, ctrlPath);
            std::stringstream ss;
            ss << directoryListing.length();
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + directoryListing;
            client_responses[client_socket] = response;
            return;
        }

        // Si le chemin est un répertoire, ajouter l'index SI existant
        if (serverConfigRequest->getRoot()) {
            std::string tmpPath = ".";
            tmpPath.append(serverConfigRequest->getRoot());
            tmpPath.append(requestedPath);
            if (isDirectory(tmpPath)) {
                if (locationBlock->getIndex() != NULL)
                    requestedPath += locationBlock->getIndex();
            }
        }
    }

    // Si le chemin est vide, charge la page d'accueil
    if (requestedPath.empty() || requestedPath == "/") {
        requestedPath += serverConfigRequest->getIndex();
    }
    std::string completPath = serverConfigRequest->getRoot();
    int i = completPath.length() + 2;
    completPath += requestedPath;

    // Chemin absolu du fichier demandé
    std::string filePath = "." + completPath;
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        // Vérifier si le fichier est un script CGI
        std::string scriptPath = filePath.substr(i);
        std::string queryString;
        size_t queryPos = scriptPath.find("?");
        if (queryPos != std::string::npos) {
            queryString = scriptPath.substr(queryPos + 1);
            scriptPath = scriptPath.substr(0, queryPos);
        }
        //verifier que le fichier existe
        std::string ctrlScripPath = CGI_BIN_PATH + scriptPath;
        if (stat(ctrlScripPath.c_str(), &fileStat) != 0) {
            std::string response = RespondPage(404, "File not found", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        }

        bool isPerlScript = scriptPath.find(".pl") != std::string::npos && scriptPath.find(".pl") == scriptPath.size() - 3;
        bool isPythonScript = scriptPath.find(".py") != std::string::npos && scriptPath.find(".py") == scriptPath.size() - 3;

        if (!isPerlScript && !isPythonScript) {
            // Si ce n'est ni un script Perl ni Python, renvoyer une erreur 404
            std::string response = RespondPage(403, "Forbidden, Only Perl and Python script", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        }

        // Exécuter le script CGI et renvoyer la sortie au client
        std::string cgiOutput = executeCGI_GET(scriptPath, queryString);
        if (cgiOutput == "504") {
            std::string response = RespondPage(504, "Gateway Timeout !", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        } else if (cgiOutput == "500") {
            std::string response = RespondPage(500, "Internal Server Error !", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        } else if (cgiOutput == "404") {
            std::string response = RespondPage(404, "File not found !", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        } else {
            std::string contentType = "text/html";
            std::stringstream ss;
            ss << cgiOutput.length();
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " +
                                ss.str() + "\r\n\r\n" + cgiOutput;
            client_responses[client_socket] = response;
            return;
        }
    }

    // Ouverture du fichier
    std::ifstream file(filePath.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open() || isDirectory(filePath)) { 
        std::cerr << "File not found: " << filePath << std::endl;
        std::string response = RespondPage(404, "Not Found", serverConfigRequest);
        client_responses[client_socket] = response;
        return;
    } else {
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string fileContent(fileSize, 0);
        file.read(&fileContent[0], fileSize);
        file.close();

        // Construction de la réponse HTTP
        std::string contentType = getContentType(filePath);
        std::stringstream ss;
        ss << fileContent.length();
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " +
                            ss.str() + "\r\n\r\n" + fileContent;
        client_responses[client_socket] = response;
        return;
    }
}

// handlePostRequest(requestData, requestHeaders, requestBody, client_socket, client_responses, serverConfigRequest)
void handlePostRequest(const std::string& requestData, const std::string& requestHeaders, const std::string& requestBody, int client_socket, std::map<int, std::string>& client_responses, ServerBlock* serverConfigRequest) {
    
    size_t start = requestHeaders.find("POST ") + 5;
    size_t end = requestHeaders.find(" HTTP/");
    std::string requestedPath = requestHeaders.substr(start, end - start);

    // Vérifier la méthode allow_methods
    LocationBlock* locationBlock = findMatchingLocationBlock(requestedPath, serverConfigRequest);
    if (locationBlock != NULL) {
        if (!(isMethodAllowed(locationBlock->getLimitExcept(), 3))) {
            std::string response = RespondPage(403, "Forbidden : Method not allowed", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        }
    }
    if (locationBlock == NULL)
        std::cerr << "--------------no location block------------------" << std::endl;
    
    // savoir quel content-type est utilisé (application/x-www-form-urlencoded ou multipart/form-data)
    std::string contentType = parseContentType(requestHeaders);
    // si le content-type est different de application/x-www-form-urlencoded ou multipart/form-data envoyer une erreur au serveur 415 Unsupported Media Type
    if (contentType != "application/x-www-form-urlencoded" && contentType != "multipart/form-data") {
        std::string response = "HTTP/1.1 415 Unsupported Media Type\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
        client_responses[client_socket] = response;
        return;
    } else {
        // si le content-type est application/x-www-form-urlencoded ou multipart/form-data
        if (contentType == "application/x-www-form-urlencoded") {
            std::string form = requestBody;
            // savoir si c'est un script cgi et avoir le chemin du script
            std::string scriptPath = parseScriptPath(requestHeaders);
            // convertir les données du formulaire et les stocker dans une structure de données MAP
            std::map<std::string, std::string> formMap = convertFormData(form);
            // convertir les %ascii en caractères
            formMap = convertAscii(formMap);
            if (scriptPath == "") {
                // verifier si le formulaire est vide
                if (form.empty()) {
                    std::string response = RespondPage(400, "BAD REQUEST", serverConfigRequest);
                    client_responses[client_socket] = response;
                    return;
                }
                // afficher les données du formulaire dans le terminale
                for (std::map<std::string, std::string>::iterator it = formMap.begin(); it != formMap.end(); ++it) {
                    std::cout << it->first << " => " << it->second << std::endl;
                }
                std::string response = RespondPage(200, "your form is send correctly", serverConfigRequest);
                client_responses[client_socket] = response;
                return;
            } if (scriptPath != "") {
                std::string scriptCgiPost = scriptPath.substr(9);
                std::string cgiOutput = executeCGI_POST(scriptCgiPost, formMap);
                if (cgiOutput == "504")
                {
                    std::string response = RespondPage(504, "Gateway Timeout", serverConfigRequest);
                    client_responses[client_socket] = response;
                    return;
                } else if (cgiOutput == "404") {
                    std::string response = RespondPage(404, "File Not Found", serverConfigRequest);
                    client_responses[client_socket] = response;
                    return;
                } else if (cgiOutput == "500") {
                    std::string response = RespondPage(500, "Internal Server Error", serverConfigRequest);
                    client_responses[client_socket] = response;
                    return;
                } else {
                    std::stringstream ss;
                    ss << cgiOutput.length();
                    std::string contentType = "text/html";
                    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " +
                                ss.str() + "\r\n\r\n" + cgiOutput;
                    client_responses[client_socket] = response;
                    return;
                }
            }
        } else {
            std::vector<FileData> files = parseMultipartFormData(requestData);
            std::string baseFileName = files[0].filename;
            if (containsAccentsOrSpaces(baseFileName)) {
                std::string response = RespondPage(403, "Mais putain retire tes accents et les espaces, petit con c'est pas complique !", serverConfigRequest);
                client_responses[client_socket] = response;
                return;
            }

            std::string uploadDir = ".";
            uploadDir.append(serverConfigRequest->getRoot());
            uploadDir.append(serverConfigRequest->getUploadFiles());
            std::string tmp = serverConfigRequest->getUploadFiles();
            locationBlock = findMatchingLocationBlock(tmp, serverConfigRequest);
            if (locationBlock != NULL) {
                if (!(isMethodAllowed(locationBlock->getLimitExcept(), 3))) {
                    std::string response = RespondPage(403, "Forbidden : Method not allowed", serverConfigRequest);
                    client_responses[client_socket] = response;
                    return;
                }
            }

            if (!uploadDir.empty()) {
                // Créez le chemin complet du fichier téléchargé
                std::string filePath = uploadDir + "/" + files[0].filename;
                // Vérifiez si un fichier du même nom existe déjà
                std::ifstream existingFile(filePath.c_str());
                if (existingFile.good()) {
                    int fileNumber = 1;
                    std::string baseFilePath = ".";
                    baseFilePath.append(serverConfigRequest->getRoot());
                    baseFilePath.append(serverConfigRequest->getUploadFiles());
                    while (existingFile.good()) {
                        std::stringstream ss;
                        ss << "(copy_" << fileNumber << ")" << baseFileName;
                        filePath = baseFilePath + "/" + ss.str();

                        existingFile.close();
                        existingFile.open(filePath.c_str());
                        fileNumber++;
                    }
                }
                existingFile.close();

                // Enregistrez le fichier téléchargé dans le répertoire spécifié
                std::ofstream output(filePath.c_str(), std::ios::binary);
                if (output.is_open()) {
                    output.write(files[0].content.c_str(), files[0].content.size());
                    output.close();
                    std::string response = RespondPage(200, "Your file is uploaded correctly", serverConfigRequest);
                    client_responses[client_socket] = response;
                    return;
                } else {
                    std::cerr << "Error: Failed to open file " << filePath << " for writing." << std::endl;
                }
            }

            // Si aucun répertoire d'upload n'est configuré ou si l'ouverture du fichier a échoué
            std::string response = RespondPage(500, "Error : Failed to save uploaded file", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        }
    }
}

void handleDeleteRequest(const std::string& requestHeaders, int client_socket, std::map<int, std::string>& client_responses, ServerBlock* serverConfigRequest){
    if (serverConfigRequest->getUploadFiles() == NULL) {
        std::string response = RespondPage(500, "Error: Upload directory don't existe", serverConfigRequest);
        client_responses[client_socket] = response;
        return;
    }
    std::string parseDel = parseMethodDelete(requestHeaders);
    std::string path = ".";
    path += serverConfigRequest->getRoot();
    path += serverConfigRequest->getUploadFiles() + parseDel;
    std::string requestedPath = serverConfigRequest->getUploadFiles();

    //verifier la methode autorisée
    LocationBlock* locationBlock = findMatchingLocationBlock(requestedPath, serverConfigRequest);
    if (locationBlock != NULL) {
        if (!(isMethodAllowed(locationBlock->getLimitExcept(), 5))) {
            std::string response = RespondPage(403, "Forbidden : Method not allowed", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        }
    }

    //suppression du fichier direct avec remove et erreur 404 si le fichier n'existe pas
    if ( std::remove(path.c_str()) == 0) {
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
        client_responses[client_socket] = response;
        return;
    } else {
        std::string response = RespondPage(404, "File cannot be deleted because it does not exist", serverConfigRequest);
        client_responses[client_socket] = response;
        return;
    }
}

void handleOptionsRequest(int client_socket, std::map<int, std::string>& client_responses, ServerBlock* serverConfigRequest) {
    // Construction de la réponse HTTP pour OPTIONS
    std::string requestedPath = serverConfigRequest->getUploadFiles();

    LocationBlock* locationBlock = findMatchingLocationBlock(requestedPath, serverConfigRequest);
    if (locationBlock != NULL) {
        if (!(isMethodAllowed(locationBlock->getLimitExcept(), 5))) { // 5 = delete
            std::string response = RespondPage(403, "Forbidden : Method not allowed", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        }
    }
    std::string response = "HTTP/1.1 200 OK\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "\r\n";
    client_responses[client_socket] = response;
    return;
}

void RequestHandler::handleRequest(int client_socket, fd_set& master_fds, std::map<int, ServerBlock*>& serverConfigs, std::map<int, std::string>& client_responses) {
    ServerBlock* serverConfigRequest = NULL;
    std::string requestData = readRequestData(client_socket, master_fds, serverConfigRequest, serverConfigs);

    if (requestData.empty()) {
        std::string response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 21\r\n\r\nInternal Server Error";
        client_responses[client_socket] = response;
        return;
    }

    std::string::size_type headerEndPos = requestData.find("\r\n\r\n");
    if (headerEndPos == std::string::npos)
        return;

    std::string requestHeaders = requestData.substr(0, headerEndPos + 4);
    std::string requestBody = requestData.substr(headerEndPos + 4);

    std::cout << "Request headers: " << std::endl << requestHeaders << std::endl;

    if (!requestBody.empty()) {
        if (requestBody.length() > static_cast<std::string::size_type>(serverConfigRequest->getMaxBodySize())) {
            std::string response = RespondPage(413, "Payload Too Large", serverConfigRequest);
            client_responses[client_socket] = response;
            return;
        }
    }
    
    std::string method = parseMethod(requestHeaders);

    if (method == "GET") {
        handleGetRequest(requestHeaders, client_socket, client_responses, serverConfigRequest);
    } else if (method == "POST") {
        handlePostRequest(requestData, requestHeaders, requestBody, client_socket, client_responses, serverConfigRequest);
    } else if (method == "DELETE") {
        handleDeleteRequest(requestHeaders, client_socket, client_responses, serverConfigRequest);
    } else if (method == "OPTIONS") {
        handleOptionsRequest(client_socket, client_responses, serverConfigRequest);
    } else {
        if (!requestHeaders.empty()) {
            std::string response = RespondPage(405, "Methode Not Allowed", serverConfigRequest);
            client_responses[client_socket] = response;
            std::cerr << "Method not allowed" << std::endl;
        }
    }
}