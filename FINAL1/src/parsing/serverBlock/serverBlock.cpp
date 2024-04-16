#include "serverBlock.hpp"
#include <cstring>

// Constructor
ServerBlock::ServerBlock() : listen(NULL), root(NULL), index(NULL), uploadFiles(NULL), errorPage(NULL), firstLocation(NULL), next(NULL)
{
    serverName = new char[10];
    strcpy(serverName, "localhost");
    this->maxBodySize = 2147483647;
}

// Destructor

ServerBlock::~ServerBlock()
{
    if (listen != NULL)
        delete[] listen;
    if (serverName != NULL)
        delete[] serverName;
    if (root != NULL)
        delete[] root;
    if (index != NULL)
        delete[] index;
    if (uploadFiles != NULL)
        delete[] uploadFiles;
    if (errorPage != NULL)
        delete[] errorPage;
    if (firstLocation != NULL)
        delete firstLocation;
    if (next != NULL)
        delete next;
}

// Setters
void ServerBlock::setListen(const char *newListen)
{
    if (listen != NULL)
        delete[] listen;
    listen = new char[strlen(newListen) + 1];
    strcpy(listen, newListen);
}

void ServerBlock::setServerName(const char *newServerName)
{
    if (serverName != NULL)
        delete[] serverName;
    serverName = new char[strlen(newServerName) + 1];
    strcpy(serverName, newServerName);
}

void ServerBlock::setRoot(const char *newRoot)
{
    if (root != NULL)
        delete[] root;
    root = new char[strlen(newRoot) + 1];
    strcpy(root, newRoot);
}

void ServerBlock::setIndex(const char *newIndex)
{
    int newIndexLength = strlen(newIndex);
    int tempLength;
    char *temp = index;

    if (temp != NULL)
    {
        tempLength = strlen(temp);
        index = new char[tempLength + newIndexLength + 2];
        strcpy(index, temp);
        strcat(index, " ");
        strcat(index, newIndex);
        delete[] temp;
    }
    else
    {
        index = new char[newIndexLength + 1];
        strcpy(index, newIndex);
    }
}

void    ServerBlock::setUploadFiles(const char *newUploadFiles)
{
    if (uploadFiles != NULL)
        delete[] uploadFiles;
    uploadFiles = new char[strlen(newUploadFiles) + 1];
    strcpy(uploadFiles, newUploadFiles);
}

void    ServerBlock::setErrorPage(const char *newErrorPage)
{
    int newErrorPageLength = strlen(newErrorPage);
    int tempLength;
    char *temp = errorPage;

    if (temp != NULL)
    {
        tempLength = strlen(temp);
        errorPage = new char[tempLength + newErrorPageLength + 2];
        strcpy(errorPage, temp);
        strcat(errorPage, " ");
        strcat(errorPage, newErrorPage);
        delete[] temp;
    }
    else
    {
        errorPage = new char[newErrorPageLength + 1];
        strcpy(errorPage, newErrorPage);
    }
}


void    ServerBlock::setMaxBodySize(int newBodySize)
{
    maxBodySize = newBodySize;
}

// Getters
const char *ServerBlock::getListen() const
{
    return (listen);
}

const char *ServerBlock::getServerName() const
{
    return (serverName);
}

const char *ServerBlock::getRoot() const
{
    return (root);
}

const char *ServerBlock::getIndex() const
{
    return (index);
}

const char    *ServerBlock::getUploadFiles(void ) const
{
    return (uploadFiles);
}

const char    *ServerBlock::getErrorPage(void ) const
{
    return (errorPage);
}

int ServerBlock::getMaxBodySize(void ) const
{
    return (maxBodySize);
}

// Linked list

LocationBlock   *ServerBlock::lastLocation(void )
{
    LocationBlock   *iterator;

    iterator = firstLocation;
    if (iterator == NULL)
        return (NULL);
    while (iterator->next != NULL)
        iterator = iterator->next;
    return (iterator);
}

const char* findNextWordAfterErrorCode(const char* bigString, int errorCode)
{
    if (bigString == NULL)
        return (NULL);
        
    char errorCodeStr[20];
    snprintf(errorCodeStr, sizeof(errorCodeStr), "%d", errorCode);
    size_t errorCodeLen = strlen(errorCodeStr);

    const char* errorCodePos = strstr(bigString, errorCodeStr);
    if (errorCodePos != NULL)
    {
        const char* nextWordStart = errorCodePos + errorCodeLen;
        while (*nextWordStart == ' ')
            nextWordStart++;

        const char* nextWordEnd = strchr(nextWordStart, ' ');
        if (nextWordEnd == NULL)
            return nextWordStart;
        else
        {
            size_t wordLength = nextWordEnd - nextWordStart;
            char* word = new char[wordLength + 1];
            strncpy(word, nextWordStart, wordLength);
            word[wordLength] = '\0';
            return word;
        }
    }
    return (NULL);
}

char            *ServerBlock::getErrorPagePath(int errorCode, const char *portNumber)
{
    ServerBlock *iterator = this;
    char        *result;
    const char        *fileName;

    while (iterator != NULL)
    {
        if (strcmp(portNumber, iterator->listen) == 0)
        {
            fileName = findNextWordAfterErrorCode(iterator->errorPage, errorCode);
            if (fileName == NULL)
                return (NULL);
            result = new char[strlen(iterator->getRoot()) + strlen(fileName) + 1];
            strcpy(result, iterator->getRoot());
            strcat(result, fileName);
            return (result);
        }
        iterator = iterator->next;
    }
    return (NULL);   
}