#include "../include/Server.h"
#include "../include/Exceptions.h"
#include <iostream>

using namespace std;


int main(int argc, char* argv[]) {
    try
    {
        Server::Get()->InitializeSecurity();
        Server::Get()->InitializeListeningSocket();
        Server::Get()->Listen();
    }
    catch (ServerException &ex)
    {
        perror(ex.what());
    }
    catch (SecurityException &ex)
    {
        perror(ex.what());
    }

    Server::Get()->Destroy();
    Server::Delete();
    return 0;
}
