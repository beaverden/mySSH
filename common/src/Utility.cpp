#include "../include/Utility.h"

void trim(std::string& str)
{
    while (!str.empty() && std::isspace(str[0]))
    {
        str.erase(0, 1);
    }
    while (!str.empty() &&std::isspace(str[str.length()-1]))
    {
        str.erase(str.length()-1, 1);
    }
}