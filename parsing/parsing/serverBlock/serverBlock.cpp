#include "serverBlock.hpp"
#include <cstring>

// Constructor
ServerBlock::ServerBlock() : listen(NULL), serverName(NULL), root(NULL), index(NULL), next(NULL)
{
    (this->errorPage).errorCode = 0;
    (this->errorPage).indexErrorPage = NULL;
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
    if (errorPage.indexErrorPage != NULL)
        delete[] errorPage.indexErrorPage;
    if (errorPage.errorCode != NULL)
        delete[] errorPage.errorCode;
    if (next != NULL)
        delete[] next;
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
    if (index != NULL)
        delete[] index;
    index = new char[strlen(newIndex) + 1];
    strcpy(index, newIndex);
}

void    ServerBlock::setErrorPage(const char *newErrorPage, const char *newErrorCode)
{
    if (errorPage.indexErrorPage != NULL)
        delete[] (errorPage.indexErrorPage);
    if (errorPage.errorCode != NULL)
        delete[] (errorPage.errorCode);
    errorPage.errorCode = new char[strlen(newErrorCode) + 1];;
    errorPage.indexErrorPage = new char[strlen(newErrorPage) + 1];
    strcpy(errorPage.errorCode, newErrorCode);
    strcpy(errorPage.indexErrorPage, newErrorPage);
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

ServerBlock::s_errorPage ServerBlock::getErrorPage(void ) const
{
    return (errorPage);
}

int ServerBlock::getMaxBodySize(void ) const
{
    return (maxBodySize);
}

// Linked List

LocationBlock   *ServerBlock::lastLocation(void )
{
    LocationBlock   *iterator;

    iterator = firstLocation;
    if (iterator == NULL)
        return (firstLocation);
    while (iterator->next != NULL)
        iterator = iterator->next;
    return (iterator);
}