#include "header.hpp"

static int whatDirective(char *line)
{
    if (strncmp(line, "allow_methods ", 14) == 0)
        return (14);
    if (strncmp(line, "autoindex ", 10) == 0)
        return (10);
    if (strncmp(line, "index ", 6) == 0)
        return (6);
    return (0);
}

static int validValue(char *value, int type)
{
    int i;
    char    *valueDup;

    i = 0;
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
    else if (strcmp(line, "}") == 0)
        return (2);
    return (directiveType(line + i));
}

int allowMethods(char *value) //GET POST DELETE
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
            iterator += result + 1;
        }
        else if (strncmp(iterator, "POST", 4) == 0)
        {
            result *= 3;
            iterator += result + 1;
        }
        else if (strncmp(iterator, "DELETE", 6) == 0)
        {
            result *= 5;
            iterator += result + 1;
        }
        else
            return (-1);
    }
    return (result);
}

static void    setValue(LocationBlock *block, int key, char *value)
{
    int resultMethods;
    
    if (key == -1)
        return ;
    else if (key == 14)
    {
        resultMethods = allowMethods(value);
        if (resultMethods == -1)
        {
            std::cout << "Error in location block allow_methods.." << std::endl;
            exit(1);
        }
        block->setLimitExcept(resultMethods);
    }
    else if (key == 10)
        block->setAutoIndex(value);
    else if (key == 6)
        block->setIndex(value);
}

LocationBlock   *parseSingleLocation(std::ifstream file, char   *line)
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
    if (URL == NULL)
    {
        delete result;
        exit(1);
    }
    while (std::isspace(*URL))
        URL++;
    *(strchr(URL, ' ')) = '\0';
    result->setURL(URL);

    while (std::getline(file, bufferLine))
    {
        type = locationTypeOfLine((char *)bufferLine.c_str());
        if (type == 2)
            return (result);
        else if (type == -1)
        {
            std::cout << "Error in file at line : " << bufferLine << std::endl;
            delete result;
            exit(1);
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
            setValue(result, type, value);
            free(toFree);
        }
    }   
}