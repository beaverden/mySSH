#include "../include/Server.h"


Server* Server::instance = nullptr;

Server::Server()
{
    this->context = nullptr;
    this->listening_socket = -1;
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

void Server::initializeSockets()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = true;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in s_addr;
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(LISTEN_PORT);
    s_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr*)(&s_addr), sizeof(s_addr)) < 0)
    {
        throw ServerException("Listening socket bind failed");
    }

    if (listen(fd, 2) == FAILED)
    {
        throw ServerException("Listening socket listen failed");
    }
    this->listening_socket = fd;
    Logger::log(LOG_EVENTS, "Listening socket is %d", fd);
}

void writing_routine(std::shared_ptr<ExecutionContext> ctx)
{
    while (true)
    {      
        int has_read = 0;
        int result = 0;
        ioctl(ctx->sv_serv, FIONREAD, &has_read);
        if (has_read > 0)
        {
            ctx->sslMutex.lock();
            char* buff = new char[has_read];
            result = read(ctx->sv_serv, buff, has_read);
            // TODO shutdown exceptions
            send_packet(ctx->ssl, PACKET_RESPONSE, buff, result);
            ctx->sslMutex.unlock();
            delete buff;
        }
    }
}

void reading_routine(std::shared_ptr<ExecutionContext> ctx)
{
    // TODO max frame size
    int fd = SSL_get_fd(ctx->ssl);
    SSH_Packet packet;
    while (true)
    {
        int has_read = 0;
        int result = 0;
        ioctl(fd, FIONREAD, &has_read);
        if (has_read > 0)
        {
            ctx->sslMutex.lock();
            recv_packet(ctx->ssl, &packet);
            result = write(ctx->sv_serv, packet.payload.content, packet.payload.content_length);
            ctx->sslMutex.unlock();
        }
    }  
}

void shell_routine(std::shared_ptr<ExecutionContext> ctx)
{
    bool newMessage = true;
    
    while (true)
    {
        if (newMessage)
        {
            write(ctx->sv_prog, "denis@ubuntu$ ", strlen("denis@ubuntu$ "));
            newMessage = false;
        }
        int has_read;
        ioctl(ctx->sv_prog, FIONREAD, &has_read);
        if (has_read > 0)
        {
            char* buff = new char[has_read + 1]();
            has_read = read(ctx->sv_prog, buff, has_read);
            std::string comm = buff;
            delete buff;
            Evaluate(comm, ctx);
            newMessage = true;
        }
    }           
}

int server_routine(SSL* ssl)
{
    try
    {
        auto ctx = std::make_shared<ExecutionContext>();
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
        ctx->sv_prog = sockets[0];
        ctx->sv_serv = sockets[1];
        ctx->inputRedir.push(ctx->sv_prog);
        ctx->outputRedir.push(ctx->sv_prog);
        ctx->errorRedir.push(ctx->sv_prog);

        std::thread t1(reading_routine, ctx);
        std::thread t2(writing_routine, ctx);
        std::thread shell(shell_routine, ctx);
        t1.join();
        t2.join();  
        shell.join();
    }
    catch (...)
    {

    }
}



void Server::connectionListen()
{
    int client = 0;
    while (client = accept(this->listening_socket, NULL, 0))
    {
        SSL* ssl = SSL_new(context);
        SSL_set_fd(ssl, client);
        if (SSL_accept(ssl) == FAILED)
        {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(client);
            continue;
        }

        Logger::log(LOG_CONNECTIONS, "Client connected");
        // Have to use fork
        int servlet_pid;
        if ((servlet_pid=fork()) == FAILED) 
        {
            perror("Fork error!");
        }
        else 
        {
            if (servlet_pid == 0)
            {
                int code = server_routine(ssl);
            }
            else 
            {
                int status;
                wait(&status);
                Logger::log(LOG_EVENTS, "Servlet finished");
            }
        }
        SSL_free(ssl);
        close(client);
        Logger::log(LOG_EVENTS, "Client disconnected");
    }    
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
    if (this->listening_socket != -1)
    {
        close(this->listening_socket);
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

std::string getPathString(std::shared_ptr<ExecutionContext> ctx)
{
    return ctx->username + ":" + ctx->currentDir + "$ ";
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
