#include "../../common/include/Exceptions.h"
#include "../../common/include/Logger.h"
#include "../include/Server.h"

#include <iostream>
#include <exception>
#include <stdexcept>

/*! \mainpage mySSH
 * <a style="font-size:18px;" href="../mySSH.pdf">PDF paper of the project</a>
 * 
 * \section intro_sec Introduction
 *  mySSH is a minimalistic cryptographic network protocol for remote 
 * control of a system over an unsecured network. A mySSH server will 
 * listen on a machine waiting for a number of clients to connect over 
 * a netwrok and securely trasmit commands. The commands (also called 
 * command lines) will have a specific format and the server will ensure 
 * the correct execution and forwading of the output to the client.
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: In the main directory, run make -B server
 * \subsection step2 Step 2: In the main directory, run make -B client
 * \subsection step3 Step 3: In the main directory, run make -B shell
 * \subsection step4 Step 4: Make bin folder the current directory and run server with the port to run
 * \subsection step5 Step 5: Run the client using ip, port and username
 */

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
