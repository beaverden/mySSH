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

void Server::InitializeListeningSocket()
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
        printf("Got connection!\n");
        char input[2048] = {0};
        char message[] = "Hello!";
        if (SSL_read(ssl, input, 2048) <= 0)
        {
            printf("SSL read error\n");
            ERR_print_errors_fp(stderr);
        }
        std::string comm = input;
        std::cout << "The command is: " << comm << std::endl;
        SSL_free(ssl);
        close(client);
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