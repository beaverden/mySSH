#include <iostream>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
using namespace std;



int main(int argc, char* argv[]) {

    /* INIT OPENSSL */
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    BIO* bio;
    bio = BIO_new_connect("127.0.0.1:2019");
    if(bio == NULL)
    {
        return -1;
    }
    
    if(BIO_do_connect(bio) <= 0)
    {
        return -1;
        /* Handle failed connection */
    }
    char arr[] = "hello";
    BIO_write(bio, arr, 6);
    printf("OK\n");
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
