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
    std::stack<int> input_redir;
    std::stack<int> output_redir;
    std::stack<int> error_redir;  

    std::mutex ssl_mutex;
    SSL* ssl;

    int sv_serv = -1;
    int sv_prog = -1;
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
