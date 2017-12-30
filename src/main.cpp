#include <iostream>
#include "../include/Parser.h"
#include "../include/Evaluate.h"

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
using namespace std;



int main(int argc, char* argv[]) {

    /* INIT OPENSSL */
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

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
