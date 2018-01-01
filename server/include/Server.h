#ifndef SERVER_H
#define SERVER_H

#include "Exceptions.h"
#include "Evaluate.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#define FAILED -1
#define LISTEN_PORT 2018
#define SHA_256_LENGTH 32

enum Packet_Type
{
    PACKET_QUERY,
    PACKET_RESPONSE,
    PACKET_REQUEST,
    PACKET_AUTH
};

struct SSH_Packet
{
    Packet_Type         packetType;
    unsigned int        packetLength;
    unsigned char       sha256Verification[SHA_256_LENGTH];
    struct SSH_Payload
    {
        unsigned int            contentLength;
        unsigned int            paddingLength;
        char*                   content;
        char*                   padding;
    } payload;
};

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

        int HandleInput(SSL* ssl);

};


#endif // SERVER_H