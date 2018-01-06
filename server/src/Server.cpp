#include "../include/Server.h"


Server* Server::instance = nullptr;

Server::Server()
{
    this->context = nullptr;
    this->listening_socket = -1;
}

void Server::InitializeSecurity()
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
}

void Server::InitializeSockets()
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
            ctx->ssl_mutex.lock();
            char* buff = new char[has_read];
            result = read(ctx->sv_serv, buff, has_read);
            // TODO shutdown exceptions
            send_packet(ctx->ssl, PACKET_RESPONSE, buff, result);
            ctx->ssl_mutex.unlock();
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
            ctx->ssl_mutex.lock();
            recv_packet(ctx->ssl, &packet);
            result = write(ctx->sv_serv, packet.payload.content, packet.payload.content_length);
            ctx->ssl_mutex.unlock();
        }
    }  
}

void shell_routine(std::shared_ptr<ExecutionContext> ctx)
{
    while (true)
    {
        int has_read;
        ioctl(ctx->sv_prog, FIONREAD, &has_read);
        if (has_read > 0)
        {
            char* buff = new char[has_read + 1]();
            has_read = read(ctx->sv_prog, buff, has_read);
            std::string comm = buff;
            delete buff;
            Evaluate(comm, ctx);
            
        }
    }           
}

void server_routine(SSL* ssl)
{
    try
    {
        auto execContext = std::make_shared<ExecutionContext>();
        if (execContext == nullptr)
        {
            throw std::bad_alloc();
        }

        int sockets[2];
        if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sockets) == FAILED)
        {
            throw ServerException("Unable initialize parent-child sockets");
        }
        execContext->ssl = ssl;
        execContext->sv_prog = sockets[0];
        execContext->sv_serv = sockets[1];
        execContext->input_redir.push(execContext->sv_prog);
        execContext->output_redir.push(execContext->sv_prog);
        execContext->error_redir.push(execContext->sv_prog);

        
        std::thread t1(reading_routine, execContext);
        std::thread t2(writing_routine, execContext);
        std::thread shell(shell_routine, execContext);
        t1.join();
        t2.join();  
        shell.join();
    }
    catch (...)
    {

    }
}



void Server::Listen()
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
        std::thread srv_thread(server_routine, ssl);
        srv_thread.join();
        SSL_free(ssl);
        close(client);
        Logger::log(LOG_EVENTS, "Client disconnected");
    }    
}

void Server::Delete()
{
    if (instance != nullptr)
    {
        delete instance;
    }
}

void Server::Destroy()
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

Server* Server::Get()
{
    if (instance == nullptr)
    {
        instance = new Server();
    }
    return instance;
}

void Server::HandleAuth(SSL* ssl)
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
