#ifndef SERVER_H
#define SERVER_H

#include "../../common/include/Exceptions.h"
#include "../../common/include/Packet.h"
#include "Evaluate.h"


#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <string>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#define FAILED -1
#define LISTEN_PORT 2018

class Server
{
    private:
        Server();
    
        static Server* instance;
        SSL_CTX* context;
        int listening_socket;

    public:
        static Server* Get();
        static void Delete();
        void Destroy();

        void InitializeListeningSocket();
        void InitializeSecurity();
        void Listen();

        int HandleInput(SSL* ssl, SSH_Packet* packet);
        void HandleAuth(SSL* ssl);

};


#endif // SERVER_H