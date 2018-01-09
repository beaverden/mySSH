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
#include <sys/ioctl.h>
#include <pwd.h>


#define MAX_ARGUMENTS 100

/**
 * \brief Context for tree execution and redirection. Gets passed
 * between functions of Evaluation and Execution
 */
struct ExecutionContext
{
    std::stack<int>     inputRedir;  ///< Stack of input redirection descriptors
    std::stack<int>     outputRedir; ///< Stack of output redirection descriptors
    std::stack<int>     errorRedir;  ///< Stakc of error redirection descriptors

    std::string         username;    ///< Current username
    std::string         currentDir;  ///< Current working directory

    bool                shouldTerminate = false; ///< Indicates that the program should stop from execution
};

/**
 * \brief Executes a given command, taking redirect descriptors from the
 * top of the stacks
 *
 * \param[in] command Simple command with parameters
 * \param[in] context Execution context pointer
 * \return Execution status of the command
 */
int Execute(
    std::string command, 
    std::shared_ptr<ExecutionContext> context
);

/**
 * \brief Parses a complex command and calls Evaluate_Tree on 
 * the AST obtained from parsing
 * 
 * \param[in] command Complex command
 * \param[in] context Execution context pointer
 */
void Evaluate(
    std::string command, 
    std::shared_ptr<ExecutionContext> context
);

/**
 * \brief Opens redirects, pushes them to the stack, creates pipes
 * between programs and executes simple commands
 * 
 * \param[in] command Complex command
 * \param[in] context Execution context pointer
 * \return Execution result of a complex command
 */
int Evaluate_Tree(
    std::shared_ptr<SyntaxTree> node, 
    std::shared_ptr<ExecutionContext> context
);

std::string getCwd();
std::string getUsername();

#endif //EVALUATE_H
