#ifndef ERROR_HPP
#define ERROR_HPP

#include <cctype>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "../parsing/serverBlock/serverBlock.hpp"
#include "../parsing/serverBlock/locationBlock.hpp"

std::string RespondPage(int num, const std::string message, ServerBlock *server);

#endif