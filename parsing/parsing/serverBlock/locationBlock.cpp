#include "locationBlock.hpp"
#include <cstring>

LocationBlock::LocationBlock() : URL(NULL), index(NULL), autoIndex(NULL), limitExcept(1), next(NULL) {}

LocationBlock::~LocationBlock()
{
    if (URL != NULL)
        delete[] URL;
    if (index != NULL)
        delete[] index;
    if (autoIndex != NULL)
        delete[] autoIndex;
}

LocationBlock &LocationBlock::operator=(const LocationBlock &other)
{
    if (this != &other)
    {
        setURL(other.URL);
        setIndex(other.index);
        setAutoIndex(other.autoIndex);
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

const char *LocationBlock::getURL() const
{
    return URL;
}

const char *LocationBlock::getIndex() const
{
    return index;
}

int LocationBlock::getLimitExcept() const
{
    return limitExcept;
}

const char  *LocationBlock::getAutoIndex() const
{
    return autoIndex;
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