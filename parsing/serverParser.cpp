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

void    setErrorPage(ServerBlock *block, char *value)
{
    char *separatorPointer;

    separatorPointer = strchr(value, ' ');
    *(separatorPointer++) = '\0';
    while (*separatorPointer != '\0' && std::isspace(*separatorPointer))
        separatorPointer++;    
    block->setErrorPage(separatorPointer, value);
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
        setErrorPage(block, value);
    else if (key == 21)
        block->setMaxBodySize(atoi(value));
}



void parseConfigFile(ServerBlock *block, std::string fileToParse)
{
    std::string bufferLine;
    std::ifstream file(fileToParse.c_str());
    int type;
    int inServer;
    char    *value;
    char    *toFree;

    if (!(file.is_open()))
    {
        std::cout << "Error opening file: " << fileToParse << std::endl;
        exit(1);
    }

    inServer = 0;
    while (std::getline(file, bufferLine))
    {
        type = serverTypeOfLine((char *)bufferLine.c_str());
        if ((type == -1) || (type == 1 && inServer == 1) || (type == 2 && inServer == 0))
        {
            std::cout << "Error in file at line : " << bufferLine << std::endl;
            exit(1);
        }
        else if (type == 1 || type == 2)
            inServer = (type % 2);
        else if (type == 0)
            ;
        else if (type == 3)
            block->lastLocation() = parseSingleLocation(file, bufferLine);
        else
        {
            if (inServer == 0)
            {
                std::cout << "Error in file at line : " << bufferLine << " with type " << type << std::endl;
                exit(1);
            }
            value = strdup(bufferLine.c_str());
            toFree = value;
            while (*value != '\0' && std::isspace(*value))
                value++;
            value += type;
            while (*value != '\0' && std::isspace(*value))
                value++;
            *(strchr(value, ';')) = '\0';
            setValue(block, type, value);
            free(toFree);
        }
    }
    file.close();
}