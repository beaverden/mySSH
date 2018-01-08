#include "../include/Server.h"

Server* Server::instance = nullptr;

Server::Server()
{
    this->context = nullptr;
    this->listeningSocket = -1;
}

void Server::initializeSecurity()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ERR_load_BIO_strings();

    const SSL_METHOD* method = SSLv23_server_method();
    this->context = SSL_CTX_new(method);
    if (context == NULL)
    {
        ERR_print_errors_fp(stderr);
        throw SecurityException("Unable to create SSL context");
    }

    if (SSL_CTX_use_certificate_file(context, "/home/denis/cert/mycert.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        throw SecurityException("Invalid certificate");
    }
    
    if (SSL_CTX_use_PrivateKey_file(context, "/home/denis/cert/mycert.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        throw SecurityException("Invalid private key");
    }

    if (!SSL_CTX_check_private_key(context))
    {
        ERR_print_errors_fp(stderr);    
        throw SecurityException("Private key check failed");
    }
    Logger::log(LOG_EVENTS, "Initialized security");
}

void Server::initializeSockets(char* _port)
{
    int port = atoi(_port);
    if (port == 0) 
    {
        throw ServerException("Invalid port");
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = true;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in s_addr;
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    s_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr*)(&s_addr), sizeof(s_addr)) < 0)
    {
        throw ServerException("Listening socket bind failed");
    }

    if (listen(fd, 2) == FAILED)
    {
        throw ServerException("Listening socket listen failed");
    }
    this->listeningSocket = fd;
    Logger::log(LOG_EVENTS, "Listening socket is %d", fd);
}

void Server::outputDataStream(std::shared_ptr<ServerContext> ctx)
{
    while (true)
    {      
        if (ctx->shouldTerminate) return;
        int has_read = 0;
        int result = 0;
        ioctl(ctx->svServ, FIONREAD, &has_read);
        try
        {
            if (has_read > 0)
            {
                ctx->sslMutex.lock();
                char* buff = new char[has_read];
                result = read(ctx->svServ, buff, has_read);
                // TODO shutdown exceptions
                send_packet(ctx->ssl, PACKET_RESPONSE, buff, result);
                ctx->sslMutex.unlock();
                delete buff;
            }
        } catch (...) {}

    }
}

void Server::inputDataStream(std::shared_ptr<ServerContext> ctx)
{
    int fd = SSL_get_fd(ctx->ssl);
    SSH_Packet packet;
    while (true)
    {
        if (ctx->shouldTerminate) return;
        int has_read = 0;
        int result = 0;
        ioctl(fd, FIONREAD, &has_read);
        try
        {
            if (has_read > 0)
            {
                ctx->sslMutex.lock();
                recv_packet(ctx->ssl, &packet);
                result = write(ctx->svServ, packet.payload.content, packet.payload.content_length);
                ctx->sslMutex.unlock();
            }
        } catch (...) {}
    }  
}

void Server::sendError(std::shared_ptr<ServerContext> ctx, const char* str)
{
    Logger::log(LOG_ERRORS, str);
    ctx->sslMutex.lock();
    std::string err = str;
    err += "\n";
    try
    {
        send_packet(ctx->ssl, PACKET_ERROR, const_cast<char*>(err.c_str()), err.length());
    } catch (...) {}
    
    ctx->sslMutex.unlock();
}

void Server::spawnShell(std::shared_ptr<ServerContext> ctx)
{
    try
    {
        if (ctx == nullptr) throw ServerException("Invalid context");
        int shellPid; 
        int shellSocket = ctx->svShell;
        if ((shellPid = fork()) == FAILED)
        {
            throw ServerException("Could not fork");
        }
        if (shellPid == 0)
        {
            // Shell space
            close(STDIN_FILENO);
            dup(shellSocket);
            
            close(STDOUT_FILENO);
            dup(shellSocket);

            close(STDERR_FILENO);
            dup(shellSocket);

            execl("shell", "shell", NULL);
            throw ServerException("Could not exec");
        }
        else
        {
            ctx->shellRunning = true;
            // Server space
            ctx->shellPid = shellPid;
            int status;
            wait(&status);
            ctx->shellRunning = false;
            ctx->shouldTerminate = true;
            ctx->shellStatus = status;
        }
    }
    catch (...)
    {
        ctx->shellPid = -1;
        ctx->shellStatus = -1;
        ctx->shellRunning = false;
    }

}

int Server::serverRoutine(SSL* ssl)
{
    try
    {
        auto ctx = std::make_shared<ServerContext>();
        if (ctx == nullptr)
        {
            throw std::bad_alloc();
        }

        int sockets[2];
        if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sockets) == FAILED)
        {
            throw ServerException("Unable initialize parent-child sockets");
        }
        ctx->ssl = ssl;
        ctx->svShell = sockets[0];
        ctx->svServ = sockets[1];

        std::thread t3(spawnShell, ctx);
        std::thread t1(inputDataStream, ctx);
        std::thread t2(outputDataStream, ctx);
        t1.join();
        t2.join();  
        t3.join();
        char msg[] = "Terminate";
        send_packet(ssl, PACKET_TERMINATE, msg, strlen(msg));
    }
    catch (ServerException& ex)
    {
        Logger::log(LOG_ERRORS, ex.what());
    }
    catch (...)
    {
        Logger::log(LOG_ERRORS, "Unknown exception in server routine");
    }
}


void Server::connectionListen()
{
    int client = 0;
    while (client = accept(this->listeningSocket, NULL, 0))
    {
        SSL* ssl = SSL_new(context);
        int servletPid;
        int exitCode = 0;
        try
        {     
            ssl = SSL_new(context);
            if (ssl == nullptr) throw ServerException("Failed to create SSL object");
            if (SSL_set_fd(ssl, client) == 0) throw ServerException("Failed to set client socket in SSL object");
            if (SSL_accept(ssl) == FAILED) throw ServerException("SSL failed to accept client");
            if ((servletPid=fork()) == FAILED) throw ServerException("Unable to fork, aborting connection: [%d]", errno);
            else if (servletPid == 0)
            {
                Logger::log(LOG_CONNECTIONS, "Client connected");
                try
                {
                   exitCode = Server::getInstance()->serverRoutine(ssl);
                }
                catch (ServerException& ex)
                {
                    exitCode = -1;
                }
                exit(exitCode);          
            }
        }
        catch (ServerException& ex)
        {
            Logger::log(LOG_ERRORS, ex.what());
            if (ssl != nullptr)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(client);
            continue;
        }   
    }
    int status;
    int wpid;
    while ((wpid = wait(&status)) > 0);  
}

void Server::deleteInstance()
{
    if (instance != nullptr)
    {
        delete instance;
    }
}

void Server::destroy()
{
    if (this->context != nullptr)
    {
        SSL_CTX_free(this->context);
    }
    if (this->listeningSocket != -1)
    {
        close(this->listeningSocket);
    }
}

Server* Server::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Server();
    }
    return instance;
}


void Server::handleAuth(SSL* ssl)
{
    char* message = "Please login in.\n";
    send_packet(ssl, PACKET_AUTH_REQUEST, message, strlen(message));

    SSH_Packet response;
    recv_packet(ssl, &response);
    if (response.packet_type != PACKET_AUTH_RESPONSE)
    {
        throw PacketIOException("Requested packet is not of type auth");
    }
    Login_Payload* payload = (Login_Payload*)(response.payload.content);
    payload->login[payload->login_length] = 0;
    payload->password[payload->password_length] = 0;
    printf("Got credentials: %s:%s\n", payload->login, payload->password);
}
