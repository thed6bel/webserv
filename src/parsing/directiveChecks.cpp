#include <iostream>
#include <cstring>
#include <cstdlib>

int isAlphaNumeric(const char* str)
{
    while (*str && *str != ' ')
    {
        if (!std::isalnum(*str) && *str != '/')
            return (0);
        str++;
    }
    return (1);
}

int isNumeric(const char* str)
{
    while (*str)
    {
        if (!std::isdigit(*str))
            return (0);
        str++;
    }
    return (1);
}

int validPath(const char *value)
{
    if (value == NULL)
        return (1);
    if (value[0] != '/')
        return (0);
    return (isAlphaNumeric(value + 1));
}

int validListen(const char *listenValue)
{
    if (listenValue == NULL)
        return (1);
    if (isNumeric(listenValue) == false || atoi(listenValue) > 65535 || atoi(listenValue) <= 0)
        return (0);
    return (1);
}

int checkAllPaths(const char *longPath)
{
    if (longPath == NULL)
        return (1);
    while (*longPath)
    {
        if (validPath(longPath) == 1)
        {
            longPath++;
            while (*longPath && *longPath != ' ')
                longPath++;
            if (*longPath == ' ')
                longPath++;
        }
        else
            return (0);
    }
    return (1);
}