//
// Created by denis on 12/27/17.
//

#ifndef EVALUATE_H
#define EVALUATE_H

#include "Parser.h"
#include "Exception.h"
#include <string>
#include <memory>
#include <stack>
#include <mutex>

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <wait.h>


#define MAX_ARGUMENTS 100

struct ExecutionContext
{
    std::stack<int> inputRedir;
    std::stack<int> outputRedir;
    std::stack<int> errorRedir;  

    std::string username;
    std::string currentDir;

    bool shouldTerminate = false;
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

std::string get_cwd();

#endif //EVALUATE_H
