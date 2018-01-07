#ifndef SERVER_H
#define SERVER_H

#include "../../common/include/Exceptions.h"
#include "../../common/include/Packet.h"
#include "Evaluate.h"


#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <unistd.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <new>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include <stdexcept>

#define FAILED -1
#define LISTEN_PORT 2018

struct ExecutionContext;

class Server
{
    private:
        Server();
    
        static Server* instance;
        SSL_CTX* context;
        int listeningSocket;

    public:
        static Server* getInstance();
        static void deleteInstance();
        void destroy();

        /**
         * \brief Prepare a server socket for listening on the given port.
         * Sets Server::listeningSocket on success
         * 
         * Might throw:
         *      ServerException
         * 
         * \param[in] _port The port on which the server will listen to connections
         */ 
        void initializeSockets(char* port);

        /**
         * \brief Prepare private key, certificates and create SSL context
         * Might throw:
         *      SecurityException
         */
        void initializeSecurity();


        /**
         * \brief Listens to connections, fork()'s a child running a shell, 
         * reading and writing sessions for each connection
         */
        void connectionListen();

        void handleAuth(SSL* ssl);
};




#endif // SERVER_H