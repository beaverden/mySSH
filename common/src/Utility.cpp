#include "../include/Utility.h"
#include <iostream>

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

void trim(std::string& str, std::string toRemove)
{
    while (!str.empty())
    {
        if (std::isspace(str[0]) ||  toRemove.find(str[0]) != std::string::npos)
        {
            str.erase(0, 1);
        } 
        else break;
    }
    
    while (!str.empty())
    {
        if (std::isspace(str.back()) || toRemove.find(str.back()) != std::string::npos) 
        {
            str.erase(str.length()-1, 1);
        }
        else break;
    }
}

unsigned int    Logger::logOptions = 0;
std::string     Logger::logPath = "log.txt";
std::mutex      Logger::logMutex;