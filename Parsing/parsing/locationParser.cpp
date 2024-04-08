#include "header.hpp"

static int whatDirective(char *line)
{
    if (strncmp(line, "allow_methods ", 14) == 0)
        return (14);
    if (strncmp(line, "autoindex ", 10) == 0)
        return (10);
    if (strncmp(line, "index ", 6) == 0)
        return (6);
    if (strncmp(line, "return ", 7) == 0)
        return (7);
    return (0);
}

static int validValue(char *value, int type)
{
    char    *valueDup;

    if (value == NULL)
        return (-1);
    valueDup = strdup(value);
    if (valueDup == NULL)
        return (-1);
    *(strchr(valueDup, ';')) = '\0';
    if ((type == 14 && countWords(valueDup) >= 1) || (type != 14 && countWords(valueDup) == 1))
    {
        free(valueDup);
        return (type);
    }
    free(valueDup);
    return (-1);
}

int directiveType(char *line)
{
    int i;
    char *semiColon;

    if (line == NULL)
        return (-1);
    semiColon = strchr(line, ';');
    if (strchr(line, ' ') == NULL || semiColon == NULL || *(semiColon + 1) != '\0')
        return (-1);
    else if ((i = whatDirective(line)) == 0)
        return (-1);
    return (validValue(line + i, i));
}

int locationTypeOfLine(char *line)
{
    int i;

    if (line == NULL)
        return (-1);
    i = 0;
    while (line[i] != '\0' && std::isspace(line[i]))
        i++;
    trimEnd(line);
    if (line[i] == '\0')
        return (0);
    else if (strcmp(line + i, "}") == 0)
        return (2);
    return (directiveType(line + i));
}

int allowMethods(char *value) //GET POST DELETE OPTIONS
{
    int result = 1;
    char    *iterator = value;

    if (value == NULL)
        return (result);
    while (*iterator != '\0')
    {
        while (*iterator != '\0' && std::isspace(*iterator))
            iterator++;
        if (strncmp(iterator, "GET", 3) == 0)
        {
            result *= 2;
            iterator += 4;
        }
        else if (strncmp(iterator, "POST", 4) == 0)
        {
            result *= 3;
            iterator += 5;
        }
        else if (strncmp(iterator, "DELETE", 6) == 0)
        {
            result *= 5;
            iterator += 7;
        }
        else if (strncmp(iterator, "OPTIONS", 7) == 0)
        {
            result *= 7;
            iterator += 8;
        }
        else
            return (-1);
    }
    return (result);
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

static int    setValue(LocationBlock *block, int key, char *value)
{
    int resultMethods;
    
    if (key == -1)
        return 0;
    else if (key == 14)
    {
        resultMethods = allowMethods(value);
        if (resultMethods == -1)
        {
            std::cout << "Error in location block allow_methods.." << std::endl;
            return (-1);
        }
        block->setLimitExcept(resultMethods);
    }
    else if (key == 10)
        block->setAutoIndex(value);
    else if (key == 6)
    {
        if (badPath(value) == 1)
            return (-1);
        block->setIndex(value);
    }
    else if (key == 7)
        block->setReturnPage(value);
    return (1);
}

int validLocationBlock(LocationBlock *block)
{
    if (block == NULL)
        return (0);
    if (block->getAutoIndex() == NULL)
        block->setAutoIndex(strdup("off"));
    return (1);
}

LocationBlock   *parseSingleLocation(std::ifstream &file, char   *line)
{
    LocationBlock   *result = new LocationBlock();
    std::string bufferLine;
    int type;
    char    *value;
    char    *toFree;
    char    *URL;

    if (result == NULL)
        return (NULL);
    URL = strdup(strchr(line, ' '));
    toFree = URL;
    if (URL == NULL)
    {
        delete result;
        return (NULL);
    }
    while (std::isspace(*URL))
        URL++;
    *(strchr(URL, ' ')) = '\0';
    result->setURL(URL);
    free(toFree);

    while (std::getline(file, bufferLine))
    {
        type = locationTypeOfLine((char *)bufferLine.c_str());
        if (type == 2)
        {
            if (validLocationBlock(result) == 0)
            {
                delete result;
                return (NULL);
            }
            else
                return (result);
        }
        else if (type == -1)
        {
            std::cout << "Error in file at line : " << bufferLine << std::endl;
            delete result;
            return (NULL);
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
                free(toFree);
                delete result;
                std::cout << "Error in configfile !" << std::endl;
                return (NULL);
            }
            free(toFree);
        }
    }  
    return (NULL);
}