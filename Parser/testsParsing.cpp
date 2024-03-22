#include "parsing/header.hpp"
//Compile with : c++ -Wall -Wextra -Werror -g3 -fsanitize=address testsParsing.cpp parsing/*.cpp parsing/serverBlock/*.cpp



void    printLocationList(LocationBlock *first)
{
    LocationBlock *iterator = first;

    while (iterator != NULL)
    {
        std::cout << "URL equals " << iterator->getURL() << std::endl;
        std::cout << "Index equals " << iterator->getIndex() << std::endl;
        std::cout << "Autoindex equals " << iterator->getAutoIndex() << std::endl;
        std::cout << "Limitexcept equals "<< iterator->getLimitExcept() << std::endl;
        if (iterator->getReturnPage() != NULL)
            std::cout << "Return equals "<< iterator->getReturnPage() << std::endl << std::endl;
        iterator = iterator->next;
    }
}

void    printServerList(ServerBlock *first)
{
    ServerBlock *iterator = first;

    while (iterator != NULL)
    {
        std::cout << "-----------  Entering Server Block  ---------" << std::endl << std::endl;
        std::cout << "Listen equals " << iterator->getListen() << std::endl;
        std::cout << "Server name equals " << iterator->getServerName() << std::endl;
        std::cout << "Root equals " << iterator->getRoot() << std::endl;
        std::cout << "Index equals "<< iterator->getIndex() << std::endl;
        if (iterator->getErrorPage() != NULL)
            std::cout << "Errorpage equals "<< iterator->getErrorPage() << std::endl;
        std::cout << "Maximum body size equals "<< iterator->getMaxBodySize() << std::endl;
        if (iterator->getUploadFiles() != NULL)
            std::cout << "Uploadfiles equals "<< iterator->getUploadFiles() << std::endl;

        std::cout << "-----------  Entering Location Block  ---------" << std::endl;
        printLocationList(iterator->firstLocation);
        iterator = iterator->next;
    }
}

//Test for SERVER pointer
// int main()
// {
//     ServerBlock *result = NULL;
//     std::ifstream file("configFiles/singleServer");
//     std::string bufferLine;

//     if (!(file.is_open()) || result != NULL)
//     {
//         std::cout << "Error opening file: config2" << std::endl;
//         exit(1);
//     }
//     if (std::getline(file, bufferLine))
//         result = parseSingleServer(file);
    
//     //Print results here.

//     if (result != NULL)
//     {
//         std::cout << "Listen equals " << result->getListen() << std::endl;
//         std::cout << "Server name equals " << result->getServerName() << std::endl;
//         std::cout << "Root equals " << result->getRoot() << std::endl;
//         std::cout << "Index equals "<< result->getIndex() << std::endl;
//         std::cout << "Errorcode equals "<< result->getErrorPage().errorCode << std::endl;
//         std::cout << "Index errorpage equals "<< result->getErrorPage().indexErrorPage << std::endl;
//         std::cout << "Maximum body size equals "<< result->getMaxBodySize() << std::endl;
//     }
//     file.close();
// }



//Test for Location pointer
// int main()
// {
//     LocationBlock *result;
//     std::ifstream file("configFiles/singleLocation");
//     std::string bufferLine;

//     if (!(file.is_open()))
//     {
//         std::cout << "Error opening file: config2" << std::endl;
//         exit(1);
//     }
//     if (std::getline(file, bufferLine))
//         result = parseSingleLocation(file, (char *)bufferLine.c_str());
    
//     //Print results here.

//     if (result != NULL)
//     {
//         std::cout << "URL equals " << result->getURL() << std::endl;
//         std::cout << "Index equals " << result->getIndex() << std::endl;
//         std::cout << "Autoindex equals " << result->getAutoIndex() << std::endl;
//         std::cout << "Limitexcept equals "<< result->getLimitExcept() << std::endl;
//     }

//     file.close();
// }

//Test for Server with a Location pointer
// int main()
// {
//     ServerBlock *result;
//     std::ifstream file("configFiles/serverWithLocation");
//     std::string bufferLine;

//     if (!(file.is_open()))
//     {
//         std::cout << "Error opening file: config2" << std::endl;
//         exit(1);
//     }
//     if (std::getline(file, bufferLine))
//         result = parseSingleServer(file);
    
//     //Print results here.    
//     printServerList(result);

//     file.close();
// }

//Test for Complete Server with or without a Locations
int main()
{
    ServerBlock *result;
    result = parseConfigFile("configFiles/multipleServers");
    
    //Print results here.    
    printServerList(result);
    delete result;
}