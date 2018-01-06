#define DEBUG_MODE

#include "../include/Server.h"
#include "../../common/include/Exceptions.h"
#include "../../common/include/Utility.h"
#include <iostream>
#include <exception>
#include <stdexcept>
using namespace std;

int main(int argc, char* argv[]) {
    Logger::log_options = LOG_PACKETS | LOG_CONNECTIONS | LOG_ERRORS;
    Logger::log_path = "../logs/log_server.txt";
    Logger::resetLog();

    try
    {
        Server::Get()->InitializeSecurity();
        Server::Get()->InitializeSockets();
        Server::Get()->Listen();
    }
    catch (...)
    {
        // TODO code
        //perror(eptr->what());
    }

    Server::Get()->Destroy();
    Server::Delete();
    return 0;
}
