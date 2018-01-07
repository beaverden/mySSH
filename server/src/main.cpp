#include "../../common/include/Exceptions.h"
#include "../../common/include/Utility.h"
#include "../include/Server.h"

#include <iostream>
#include <exception>
#include <stdexcept>

int main(int argc, char* argv[]) {
    if (argc != 2)
    {
        printf("Usage: ./server [port]\n");
        return 0;
    }

    Logger::logOptions = LOG_EVENTS | LOG_PACKETS | LOG_CONNECTIONS | LOG_ERRORS;
    Logger::logPath = "../logs/log_server.txt";
    Logger::resetLog();
    try
    {
        Server::getInstance()->initializeSecurity();
        Server::getInstance()->initializeSockets(argv[1]);
        Server::getInstance()->connectionListen();
    }
    catch (SecurityException& ex)
    {
        Logger::log(LOG_ERRORS, ex.what());
        printf("A security exception occured. Please check the log file\n");
    }
    catch (ServerException& ex)
    {
        Logger::log(LOG_ERRORS, ex.what());
        printf("A server exception occured. Please check the log file\n");
    }
    catch (std::exception& ex)
    {
        Logger::log(LOG_ERRORS, "Unknown fatal exception: %s", ex.what());
        printf("A unknown exception occured. Please check the log file\n");
    }

    try
    {
        Server::getInstance()->destroy();
        Server::deleteInstance();
    } catch(...) 
    {
        // Ignore exceptions
    }
    
    return 0;
}
