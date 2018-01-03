//
// Created by denis on 12/27/17.
//

#ifndef MYSSHSERVER_EVALUATE_H
#define MYSSHSERVER_EVALUATE_H
#include <string>
#include <memory>
#include <stack>
#include "Parser.h"
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#define MAX_ARGUMENTS 100

struct ExecutionContext
{
    std::stack<int> inputRedir;
    std::stack<int> outputRedir;
    std::stack<int> errorRedir;  
    SSL* ssl;
};

void redirect_output(
    int local_socket, 
    int client_socket, 
    std::shared_ptr<ExecutionContext> context
);
void redirect_input(
    int local_socket, 
    int client_socket, 
    std::shared_ptr<ExecutionContext> context
);


int Execute(
    std::string command, 
    int _stdin, 
    int _stdout, 
    int _stderr, 
    std::shared_ptr<ExecutionContext> context
);

void Evaluate(
    std::string command, SSL* ssl
);
int Evaluate(
    std::shared_ptr<SyntaxTree> node, 
    std::shared_ptr<ExecutionContext> context
);

#endif //MYSSHSERVER_EVALUATE_H
