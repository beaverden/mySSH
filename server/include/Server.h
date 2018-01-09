#ifndef SERVER_H
#define SERVER_H

#include "../../common/include/Exceptions.h"
#include "../../common/include/Packet.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <pwd.h>
#include <shadow.h>


#include <unistd.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <new>
#include <mutex>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include <stdexcept>

#define FAILED -1

struct ServerContext
{
    int userId = -1;
    int svShell = -1;
    int svServ = -1;

    SSL* ssl;
    std::mutex sslMutex;

    bool shouldTerminate = false;

    int shellPid = -1;
    int shellStatus = 0;
    bool shellRunning = false;
};

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

        /**
         * \brief Prepares execution environment. Runs out and input data streams,
         * spawns a shell for the client
         * 
         * \param[in] SSL Pointer to initialized and ready SSL structure
         * \return Execution result
         */
        int serverRoutine(SSL* ssl);

        /**
         * \brief Constantly reads output from the shell and outputs it to the 
         * SSL socket
         * 
         * \param ctx A pointer to ServerContext structure, containing sockets
         */
        static void outputDataStream(std::shared_ptr<ServerContext> ctx);

        /**
         * \brief Constantly reads from SSL socket and redirects it to the shell
         * 
         * \param ctx A pointer to ServerContext structure, containing sockets
         */        
        static void inputDataStream(std::shared_ptr<ServerContext> ctx);

        /**
         * \brief Sends an error to the SSL socket
         * 
         * \param ctx A pointer to ServerContext structure, containing sockets
         * \param str Pointer to a null terminated C string 
         */
        void sendError(std::shared_ptr<ServerContext> ctx, const char* str);
       
        /**
         * \brief Spawns a shell with input/output/error redirected with data streams
         * 
         * \param ctx A pointer to ServerContext structure, containing sockets
         */
        static void spawnShell(std::shared_ptr<ServerContext> ctx);

        bool handleAuth(SSL* SSL, std::shared_ptr<ServerContext> ctx);

        static void sendSignal(std::shared_ptr<ServerContext> ctx, int sig);
};




#endif // SERVER_H