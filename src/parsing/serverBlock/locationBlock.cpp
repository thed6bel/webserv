#include "locationBlock.hpp"
#include <cstring>

LocationBlock::LocationBlock() : URL(NULL), index(NULL), returnPage(NULL), limitExcept(1), next(NULL) 
{
    autoIndex = new char[4];
    strcpy(autoIndex, "off");
}

LocationBlock::~LocationBlock()
{
    if (URL != NULL)
        delete[] URL;
    if (index != NULL)
        delete[] index;
    if (autoIndex != NULL)
        delete[] autoIndex;
    if (returnPage != NULL)
        delete[] returnPage;
    if (next != NULL)
        delete next;
}

LocationBlock &LocationBlock::operator=(const LocationBlock &other)
{
    if (this != &other)
    {
        setURL(other.URL);
        setIndex(other.index);
        setAutoIndex(other.autoIndex);
        setReturnPage(other.returnPage);
        setLimitExcept(other.limitExcept);
        next = other.next;
    }
    return *this;
}

//------------Getters & Setters---------------
void LocationBlock::setURL(const char *newURL)
{
    if (URL != NULL)
        delete[] URL;
    URL = new char[strlen(newURL) + 1];
    strcpy(URL, newURL);
}

void LocationBlock::setIndex(const char *newIndex)
{
    if (index != NULL)
        delete[] index;
    index = new char[strlen(newIndex) + 1];
    strcpy(index, newIndex);
}

void LocationBlock::setLimitExcept(int newLimitExcept)
{
    limitExcept = newLimitExcept;
}

void LocationBlock::setAutoIndex(const char *newAutoIndex)
{
    if (autoIndex != NULL)
        delete[] autoIndex;
    autoIndex = new char[strlen(newAutoIndex) + 1];
    strcpy(autoIndex, newAutoIndex);
}

void    LocationBlock::setReturnPage(const char *newReturnPage)
{
    if (returnPage != NULL)
        delete[] returnPage;
    returnPage = new char[strlen(newReturnPage) + 1];
    strcpy(returnPage, newReturnPage);
}

const char *LocationBlock::getURL() const
{
    return (URL);
}

const char *LocationBlock::getIndex() const
{
    return (index);
}

int LocationBlock::getLimitExcept() const
{
    return (limitExcept);
}

const char  *LocationBlock::getAutoIndex() const
{
    return (autoIndex);
}

const char    *LocationBlock::getReturnPage(void ) const
{
    return (returnPage);
}
//--------------------------------------------
//------------ Method Permissions ------------
    int LocationBlock::getAllow(void )
    {
        return (limitExcept % 2 == 0);
    }

    int LocationBlock::postAllow(void )
    {
        return (limitExcept % 3 == 0);
    }
    
    int LocationBlock::deleteAllow(void )
    {
        return (limitExcept % 5 == 0);
    }

    int LocationBlock::optionAllow(void )
    {
        return (limitExcept % 7 == 0);
    }
//--------------------------------------------