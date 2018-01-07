//
// Created by denis on 12/27/17.
//

#ifndef EVALUATE_H
#define EVALUATE_H
#include <string>
#include <memory>
#include <stack>
#include <mutex>

#include "Parser.h"
#include "Server.h"
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#define MAX_ARGUMENTS 100

struct ExecutionContext
{
    std::stack<int> inputRedir;
    std::stack<int> outputRedir;
    std::stack<int> errorRedir;  

    std::mutex sslMutex;
    SSL* ssl;

    int sv_serv = -1;
    int sv_prog = -1;

    std::string username;
    std::string currentDir;
    // TODO closing sockets properly?
};

int Execute(
    std::string command, 
    std::shared_ptr<ExecutionContext> context
);

void Evaluate(
    std::string command, 
    std::shared_ptr<ExecutionContext> context
);

int Evaluate_Tree(
    std::shared_ptr<SyntaxTree> node, 
    std::shared_ptr<ExecutionContext> context
);

#endif //EVALUATE_H
