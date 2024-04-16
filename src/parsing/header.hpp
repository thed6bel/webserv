#ifndef HEADER_HPP
#define HEADER_HPP

#include <cctype>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include "serverBlock/serverBlock.hpp"

//------------------------Parsing Utils-------------------------------
int isServerLine(char *line);
int isRegularLine(char *line);
int serverTypeOfLine(char *line);
int countWords(const char* str);
void trimEnd(char *str);
//--------------------------------------------------------------------

//----------------------Location Parsing------------------------------
LocationBlock   *parseSingleLocation(std::ifstream &file, char   *line);
//--------------------------------------------------------------------

//-----------------------Server Parsing-------------------------------
ServerBlock   *parseSingleServer(std::ifstream &file);
//--------------------------------------------------------------------

//-----------------------File Parsing---------------------------------
ServerBlock *parseConfigFile(std::string fileToParse);
//--------------------------------------------------------------------

//---------------------Valid Directives-------------------------------
int checkAllPaths(const char *value);
int validListen(const char *listenValue);
//--------------------------------------------------------------------

#endif