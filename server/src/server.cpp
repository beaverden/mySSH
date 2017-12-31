#include "../include/Parser.h"
#include "../include/Evaluate.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
using namespace std;

#define LISTEN_PORT 2018

int create_socket()
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
        perror("Cound't bind socket to address\n");
        return -1;
    }

    if (listen(fd, 2) == -1)
    {
        perror("Couldn't listen");
        return -1;
    }

    return fd;
}

int main(int argc, char* argv[]) {

    /* INIT OPENSSL */
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    int sock = create_socket();
    if (sock == -1) 
    {
        perror("Couldn't create socket");
        return -1;
    }
    BIO* bio = BIO_new_accept("2019");
    //BIO_do_accept(bio);
    int client = 0;
    char data[1000] = {0};
    BIO_set_nbio_accept(bio, 0);
    BIO_set_bind_mode(bio, BIO_BIND_REUSEADDR);
    if (BIO_do_accept(bio) < 0)
    {
        perror("Accept failed");
        return -1;
    }
    while (client = BIO_do_accept(bio))
    {
        BIO_read(bio, data, 6);
        printf("Data: %s\n", data);
        if (strncmp(data, "hello", 5) == 0)
        {
            
            printf("Got a connection!\n");
            break;
        }
    }
    BIO_free_all(bio);
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
