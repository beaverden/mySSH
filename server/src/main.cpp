#define DEBUG_MODE

#include "../include/Server.h"
#include "../../common/include/Exceptions.h"
#include "../../common/include/Utility.h"
#include <iostream>
#include <exception>
#include <stdexcept>
using namespace std;

int main(int argc, char* argv[]) {
    Logger::logOptions =   
        LOG_EVENTS | LOG_PACKETS | LOG_CONNECTIONS | LOG_ERRORS;
    Logger::logPath = "../logs/log_server.txt";
    Logger::resetLog();

    try
    {
        Server::getInstance()->initializeSecurity();
        Server::getInstance()->initializeSockets();
        Server::getInstance()->connectionListen();
    }
    catch (...)
    {
        // TODO code
        //perror(eptr->what());
    }

    Server::getInstance()->destroy();
    Server::deleteInstance();
    return 0;
}
