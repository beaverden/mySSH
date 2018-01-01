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

int main(int argc, char* argv[]) {

    /* INIT OPENSSL */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    
    
    int sock = create_socket();
    const SSL_METHOD* method = SSLv23_client_method();//TLSv1_2_client_method();
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

    while (true)
    {
        std::string message;
        std::getline(std::cin, message);
        if (message == "exit")
        {
            SSL_shutdown(ssl);
            break;
        }
        else
        {
            char* chm = (char*)message.c_str();
            int len = strlen(chm);
            if (SSL_write(ssl, chm, len) != len)
            {
                printf("Failed to write all the data\n");
                continue;
            }
            char ans[1000] = {0};
            printf("Sent. Awating response\n");
            SSL_read(ssl, ans, 1000);
            printf("Got response!\n");
            ans[999] = 0;
            printf("%s", ans);
        }
    }
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(context);
    /*
    std::string comm = argv[1];

    try {
        Evaluate(comm);
    }
    catch (EvaluationException& ex)
    {
        std::cout << ex.what() << std::endl;
    }
    catch (ParserException& ex)
    {
        std::cout << ex.what() << std::endl;
        return -1;
    }
    catch (VerificationException& ex)
    {
        std::cout << ex.what() << std::endl;
        return -1;
    }
    */


    return 0;
}
