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
#include <sys/file.h>
#include <sys/ioctl.h>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <mutex>


struct ExecutionContext
{
    SSL* ssl;
    std::mutex ssl_mutex;

    bool shouldTerminate = false;
};

int createSocket(char* _ip, char* _port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int port = atoi(_port);
    if (port == 0)
    {
        printf("Invalid port conversion\n");
        return -1;
    } 

    sockaddr_in addr_in;

    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);
    inet_pton(AF_INET, _ip, &addr_in.sin_addr);

    int resultfd = connect(fd, (struct sockaddr*)&addr_in, sizeof(addr_in));
    if (resultfd < 0)
    {
        printf("Failed to connect\n");
        return -1;
    }
    printf("Connected!\n");
    return fd;
}

std::string handleAuth(SSL* ssl, std::string login)
{
    printf("Please provide password\n");
    std::string password;
    std::getline(std::cin, password);
    Login_Payload payload;
    memcpy(payload.login, login.c_str(), login.length()); 
    memcpy(payload.password, password.c_str(), password.length());                    
    payload.login_length = login.length();
    payload.password_length = password.length();
    send_packet(ssl, PACKET_AUTH_REQUEST, &payload, sizeof(payload));

    SSH_Packet packet;
    recv_packet(ssl, &packet);
    char* content = (char*)(packet.payload.content);
    return std::string(content);
}

void writingRoutine(std::shared_ptr<ExecutionContext> ctx)
{
    while (true)
    {      
        if (ctx->shouldTerminate) return;
        int has_read = 0;
        int result = 0;
        ioctl(STDIN_FILENO, FIONREAD, &has_read);
        try
        {
            if (has_read > 0)
            {
                ctx->ssl_mutex.lock();
                char* buff = new char[has_read];
                result = read(STDIN_FILENO, buff, has_read);
                send_packet(ctx->ssl, PACKET_RESPONSE, buff, result);
                ctx->ssl_mutex.unlock();
                delete buff;
            }
        }
        catch (ShutdownException& ex)
        {
            ctx->shouldTerminate = true;
            continue;
        }
        catch (PacketIOException& ex)
        {
            printf("[Client] There was an error writing the packet\n");
            fflush(stdout);
        }
        catch (...)
        {
            printf("[Client] Unknown error occured\n");
            fflush(stdout);
        }

    }
}

void readingRoutine(std::shared_ptr<ExecutionContext> ctx)
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
                ctx->ssl_mutex.lock();
                recv_packet(ctx->ssl, &packet);
                if (packet.packet_type == PACKET_TERMINATE)
                {
                    ctx->shouldTerminate = true;
                    return;
                }
                result = write(STDOUT_FILENO, packet.payload.content, packet.payload.content_length);
                ctx->ssl_mutex.unlock();
            }
        }
        catch (ShutdownException& ex)
        {
            ctx->shouldTerminate = true;
            continue;
        }
        catch (PacketIOException& ex)
        {
            printf("[Client] There was an error reading the packet\n");
            fflush(stdout);
        }
        catch (...)
        {
            printf("[Client] Unknown error occured\n");
            fflush(stdout);
        }

    }  
}


int main(int argc, char* argv[]) {

    if (argc != 4)
    {
        printf("Usage: ./client ip port username\n");
        return 0;
    }
    /* INIT OPENSSL */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    
    
    int sock = createSocket(argv[1], argv[2]);
    if (sock == FAILED)
    {
        printf("Unable to create socket\n");
        return -1;
    }
    const SSL_METHOD* method = SSLv23_client_method();
    SSL_CTX* context = SSL_CTX_new(method);
    if (context == NULL)
    {
        printf("Couldn't create context\n");
        return -1;
    }
    
    SSL* ssl = SSL_new(context);
    if (ssl == NULL)
    {
        printf("Unable to create new SSL object\n");
        SSL_CTX_free(context);
        close(sock);
        return -1;
    }

    SSL_set_fd(ssl, sock);
    if (SSL_connect(ssl) == -1)
    {
        printf("Failed to connect to server\n");
        SSL_free(ssl);
        close(sock);
        SSL_CTX_free(context);  
        return -1;
    }

    std::string result = handleAuth(ssl, argv[3]);
    if (result != "OK")
    {
        printf("Auth error: %s\n", result.c_str());
    }
    else
    {
        std::shared_ptr<ExecutionContext> ctx = std::make_shared<ExecutionContext>();
        ctx->ssl = ssl;
        
        std::thread t1(readingRoutine, ctx);
        std::thread t2(writingRoutine, ctx);

        t1.join();
        t2.join();
    }


    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(context);  
    return 0;
}
