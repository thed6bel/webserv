#ifndef HEADER_HPP
#define HEADER_HPP

#include <cctype>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "../serverBlock/serverBlock.hpp"

int isServerLine(char *line);
int isRegularLine(char *line);
int serverTypeOfLine(char *line);
int countWords(const char* str);
void trimEnd(char *str);

//----------------------Location Parsing------------------------------
LocationBlock   *parseSingleLocation(std::ifstream file, char   *line);
//--------------------------------------------------------------------

#endif