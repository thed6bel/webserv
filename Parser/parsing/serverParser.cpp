#include "header.hpp"

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

static void    setValue(ServerBlock *block, int key, char *value)
{
    if (key == -1)
        return ;
    else if (key == 7)
        block->setListen(value);
    else if (key == 12)
        block->setServerName(value);
    else if (key == 5)
        block->setRoot(value);
    else if (key == 6)
        block->setIndex(value);
    else if (key == 11)
        block->setErrorPage(value);
    else if (key == 21)
        block->setMaxBodySize(atoi(value));
    else if (key == 13)
        block->setUploadFiles(value);
}

ServerBlock   *parseSingleServer(std::ifstream &file)
{   
    ServerBlock   *result = new ServerBlock();
    std::string bufferLine;
    int type;
    char    *value;
    char    *toFree;

    if (result == NULL)
        return (NULL);

    while (std::getline(file, bufferLine))
    {
        type = serverTypeOfLine((char *)bufferLine.c_str());
        if (type == 2)
            return (result);
        else if (type == -1)
        {
            std::cout << "Error in file at line : " << bufferLine << std::endl;
            delete result;
            exit(1);
        }
        else if (type == 3)
        {
            if (result->firstLocation == NULL)
                result->firstLocation = parseSingleLocation(file, (char *)strchr(bufferLine.c_str(), 'n') + 1);
            else
                result->lastLocation()->next = parseSingleLocation(file, (char *)strchr(bufferLine.c_str(), 'n') + 1);
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

ServerBlock *parseConfigFile(std::string fileToParse)
{   
    ServerBlock *completeBlock = NULL;
    
    std::string bufferLine;
    std::ifstream file(fileToParse.c_str());
    int lineType;

    if (!(file.is_open()))
    {
        std::cout << "Error opening file: " << fileToParse << std::endl;
        exit(1);
    }
    
    while (std::getline(file, bufferLine))
    {
        lineType = serverTypeOfLine((char *)bufferLine.c_str());
        if (lineType == 1)
        {
            if (completeBlock == NULL)
                completeBlock = parseSingleServer(file);
            else
                lastServerBlock(completeBlock)->next = parseSingleServer(file);
        }
        else if (lineType == 0)
            continue;
        else
        {
            std::cout << "Error in file at line : " << bufferLine << std::endl;
            exit(1);
        }
    }
    file.close();
    return (completeBlock);
}