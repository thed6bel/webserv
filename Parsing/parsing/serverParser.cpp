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
    file.close();
    return (completeBlock);
}