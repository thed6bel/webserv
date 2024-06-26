#include "header.hpp"
#include <cstdlib>
#include <cstring>
#include <cctype>


int countWords(const char* str)
{
   bool inSpaces = true;
   int numWords = 0;
   int  i = 0;

   if (str == NULL)
      return (-1);

   while (str[i] != '\0')
   {
      if (std::isspace(str[i]))
        inSpaces = true;
      else if (inSpaces)
      {
        numWords++;
        inSpaces = false;
      }
      i++;
   }
   return (numWords);
}

void trimEnd(char *str)
{
    int length;
    int lastCharIndex;

    if (str == NULL || *str == '\0')
        return;
    
    length = strlen(str);
    lastCharIndex = length - 1;
    
    while (lastCharIndex >= 0 && (str[lastCharIndex] == ' ' || str[lastCharIndex] == '\t' ||
                                   str[lastCharIndex] == '\n' || str[lastCharIndex] == '\r'))
        lastCharIndex--;
    str[lastCharIndex + 1] = '\0';
}

static int    badErrorPage(char *value)
{
    int i = 0;

    if (value == NULL)
        return (1);
    while (value[i] != '\0' && (value[i] == ' ' || (value[i] >= '0' && value[i] <= '9')))
        i++;
    if (value[i] != '/')
        return (1);
    return (0);
}

static int  badPath(char *value)
{
    int i = 0;

    if (value == NULL)
        return (1);
    while (value[i] != '\0' && value[i] == ' ')
        i++;
    while (value[i] != '\0')
    {
        while (value[i] == ' ')
            i++;
        if (value[i] != '/' && value[i] != '\0')
            return (1);
        while (value[i] != '\0' && value[i] != ' ')
            i++;
    }
    return (0);
}

static int    setValue(ServerBlock *block, int key, char *value)
{
    if (key == -1)
        return 0;
    else if (key == 7)
    {
        if (block->getListen() != NULL)
            return (-1);
        block->setListen(value);
    }
    else if (key == 12)
        block->setServerName(value);
    else if (key == 5)
    {
        if (badPath(value) == 1)
            return (-1);
        block->setRoot(value);
    }
    else if (key == 6)
    {
        if (badPath(value) == 1)
            return (-1);
        block->setIndex(value);
    }
    else if (key == 11)
    {
        if (badErrorPage(value) == 1)
            return (-1);
        block->setErrorPage(value);
    }
    else if (key == 21)
        block->setMaxBodySize(atoi(value));
    else if (key == 13)
    {
        if (badPath(value) == 1)
            return (-1);
        block->setUploadFiles(value);
    }
    return (1);
}

int validServerBlock(ServerBlock *block)
{
    if (block == NULL)
        return (0);
    if (block->getListen() == NULL)
    {
        std::cout << "ERROR: Listen directive not specified in configfile." << std::endl;
        return (0);
    }
    if (block->getRoot() == NULL)
    {
        std::cout << "ERROR: Root directive not specified in configfile." << std::endl;
        return (0);
    }
    if (block->getIndex() == NULL)
    {
        std::cout << "ERROR: Index directive not specified in configfile." << std::endl;
        return (0);
    }
    if (validListen(block->getListen()) == 0)
    {
        std::cout << "ERROR: Invalid listen in serverblock." << std::endl;
        return (0);
    }
    return (1);
}

ServerBlock   *parseSingleServer(std::ifstream &file)
{   
    ServerBlock   *result = new ServerBlock();
    LocationBlock   *buffer = NULL;
    std::string bufferLine;
    int type;
    char    *value;
    char    *toFree;

    if (result == NULL)
    {
        std::cout << "New call failed." << std::endl;
        return (NULL);
    }

    while (std::getline(file, bufferLine))
    {
        type = serverTypeOfLine((char *)bufferLine.c_str());
        if (type == 2)
        {
            if (validServerBlock(result) == 0)
            {
                delete result;
                std::cout << "Invalid Serverblock, check configfile !" << std::endl;
                return (NULL);
            }
            else
                return (result);
        }
        else if (type == -1)
        {
            std::cout << "Error in file at line : " << bufferLine << std::endl;
            delete result;
            return (NULL); // Or return NULL ?
        }
        else if (type == 3)
        {
            //CHECK if location fails ! EXIT or NULL
            buffer = parseSingleLocation(file, (char *)strchr(bufferLine.c_str(), 'n') + 1);
            if (buffer == NULL)
            {
                delete  result;
                return (NULL);
            }
            if (result->firstLocation == NULL)
                result->firstLocation = buffer;
            else
                result->lastLocation()->next = buffer;
        }
        else if (type > 2)
        {
            value = strdup(bufferLine.c_str());
            trimEnd(value);
            toFree = value;
            while (*value != '\0' && std::isspace(*value))
                value++;
            value += type;
            while (*value != '\0' && std::isspace(*value))
                value++;
            *(strchr(value, ';')) = '\0';
            if (setValue(result, type, value) == -1)
            {
                std::cout << "Error in configfile " << value << " !" << std::endl;
                free(toFree);
                delete result;
                return (NULL);
            }
            free(toFree);
        }
    }
    return (NULL);
}

ServerBlock *lastServerBlock(ServerBlock *first)
{
    ServerBlock *iterator = first;

    if (first == NULL)
        return (NULL);
    while (iterator->next != NULL)
        iterator = iterator->next;
    return (iterator);
}

int checkServerDuplicate(const char *serverName, ServerBlock *startBlock)
{
    ServerBlock *iterator = startBlock;

    while (iterator != NULL)
    {
        if (strcmp(serverName, iterator->getServerName()) == 0 && strcmp(serverName, "localhost") != 0)
            return (1);    
        iterator = iterator->next;
    }
    return (0);
}

int checkListenDuplicate(const char *listenName, ServerBlock *startBlock)
{
    ServerBlock *iterator = startBlock;

    while (iterator != NULL)
    {
        if (strcmp(listenName, iterator->getListen()) == 0)
            return (1);
        iterator = iterator->next;
    }
    return (0);
}


int checkDuplicateServerListen(ServerBlock *firstBlock)
{
    ServerBlock *iterator = firstBlock;

    while (iterator != NULL && iterator->next != NULL)
    {
        if (checkServerDuplicate(iterator->getServerName(), iterator->next) == 1)
        {
            std::cout << "Duplicate servername in configfile !!!" << std::endl;
            return (1);
        }
        if (checkListenDuplicate(iterator->getListen(), iterator->next) == 1)
        {
            std::cout << "Duplicate listen in configfile !!!" << std::endl;
            return (1);
        }
        iterator = iterator->next;
    }
    return (0);
}

int    createUploadDirectory(ServerBlock *first)
{
    ServerBlock *iterator = first;
    struct stat info;
    const char *directoryPath;
    char    *temp;

    while (iterator != NULL)
    {
        if ((directoryPath = iterator->getUploadFiles()) != NULL)
        {
            temp = new char[strlen(directoryPath) + strlen(iterator->getRoot()) + 2];
            strcpy(temp, ".");
            strcat(temp, iterator->getRoot());
            strcat(temp, directoryPath);
            if (stat(temp, &info) != 0)
            {
                if (mkdir(directoryPath + 1, S_IRWXU) != 0)
                {
                    std::cout << "Error, your upload directory don't exist, create one" << directoryPath << std::endl;
                    delete(temp);
                    return (-1);
                }
            }
            delete(temp);
        }
        iterator = iterator->next;
    }
    return (1);
}

ServerBlock *parseConfigFile(std::string fileToParse)
{   
    ServerBlock *completeBlock = NULL;
    ServerBlock *buffer;
    
    std::string bufferLine;
    std::ifstream file(fileToParse.c_str());
    int lineType;

    if (!(file.is_open()))
    {
        std::cout << "Error opening file: " << fileToParse << std::endl;
        return (NULL);
    }
    
    while (std::getline(file, bufferLine))
    {
        lineType = serverTypeOfLine((char *)bufferLine.c_str());
        if (lineType == 1)
        {
            buffer = parseSingleServer(file);
            if (buffer == NULL)
            {
                delete completeBlock;
                return (NULL);
            }
            if (completeBlock == NULL)
                completeBlock = buffer;
            else
                lastServerBlock(completeBlock)->next = buffer;
        }
        else if (lineType == 0)
            continue;
        else
        {
            std::cout << "Error in file at line : " << bufferLine << std::endl;
            delete completeBlock;
            return (NULL);
        }
    }
    if (checkDuplicateServerListen(completeBlock) == 1)
    {
        delete completeBlock;
        return (NULL);
    }
    if (createUploadDirectory(completeBlock) == -1)
    {
        delete completeBlock;
        return (NULL);
    }
    file.close();
    return (completeBlock);
}