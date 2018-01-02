#define DEBUG_MODE

#include "../../common/include/Packet.h"
#include "../../common/include/Exceptions.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <memory.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#include <iostream>
#include <string>
#include <string.h>
using namespace std;

#define PORT 2018
#define LOCAL_IP "127.0.0.1"

int create_socket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr_in;

    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(PORT);
    inet_pton(AF_INET,LOCAL_IP, &addr_in.sin_addr);

    int resultfd = connect(fd, (struct sockaddr*)&addr_in, sizeof(addr_in));
    if (resultfd < 0)
    {
        perror("Failed to connect\n");
        return -1;
    }
    printf("Connected!\n");
    return fd;
}

void HandleAuth(SSL* ssl)
{
    printf("Please log in\n");
    std::string login;
    std::string password;
    std::getline(std::cin, login);
    std::getline(std::cin, password);
    Login_Payload payload;
    memcpy(payload.login, login.c_str(), login.length()); 
    memcpy(payload.password, password.c_str(), password.length());                    
    payload.login_length = login.length();
    payload.password_length = password.length();
    send_packet(ssl, PACKET_AUTH_RESPONSE, &payload, sizeof(payload));
}

void HandleResponse(SSL* ssl, SSH_Packet* packet)
{
    printf("%s", packet->payload.content);
}

void HandleCommand(SSL* ssl)
{
    printf("Input\n");
    std::string command;
    std::getline(std::cin, command);
    send_packet(ssl, PACKET_QUERY, (void*)command.c_str(), command.length());
}

int main(int argc, char* argv[]) {

    /* INIT OPENSSL */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    
    
    int sock = create_socket();
    const SSL_METHOD* method = SSLv23_client_method();
    SSL_CTX* context = SSL_CTX_new(method);
    if (context == NULL)
    {
        printf("Couldn't create context\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    SSL* ssl = SSL_new(context);
    SSL_set_fd(ssl, sock);
    if (SSL_connect(ssl) == -1)
    {
        printf("Failed to connect to server\n");
        return -1;
    }

   
    SSH_Packet packet;
    try
    {
        recv_packet(ssl, &packet);
        HandleAuth(ssl);
    }
    catch (PacketIOException& ex)
    {
        printf("Packer reading error: %s\nRetrying...\n", ex.what());
        return -1;
    }
    catch (ShutdownException& ex)
    {
        printf("Shutdown exception: %s\n", ex.what());
    }

    while (true)
    {
        recv_packet(ssl, &packet);
        switch (packet.packet_type)
        {
            case PACKET_AUTH_REQUEST:
            {
                HandleAuth(ssl);
                break;
            }
            case PACKET_RESPONSE:
            {
                HandleResponse(ssl, &packet);
                break;
            }
            case PACKET_READY:
            {
                HandleCommand(ssl);
                break;
            }
        }
    }

    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(context);  

    return 0;
}
