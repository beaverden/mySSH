//
// Created by denis on 12/27/17.
//

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <wait.h>
#include "../include/Evaluate.h"
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include <sys/socket.h>

void redirect(int local_socket, int client_socket, ExecutionContext* context)
{
    char buffer[1024] = {0};
    int sz = 0;
    while ((sz = read(local_socket, buffer, 1024)) > 0)
    {
        if (SSL_write(context->ssl, buffer, sz) <= 0)
        {
            printf("Didn't write\n");
            return;
        }
    }
}

int Execute(
    std::string command, 
    int _stdin,
    int _stdout,
    int _stderr,
    ExecutionContext* context
)
{
    std::vector<std::string> tokens = Parser::Get()->tokenizeExecute(command, " ");
    if (tokens.size() > MAX_ARGUMENTS)
    {
        throw EvaluationException("Too many arguments in command [%30s...]", command.c_str());
    }
    int pid;
    int in_sock[2];
    int out_sock[2];
    int err_sock[2];
    int client_sock = SSL_get_fd(context->ssl);
    if ((socketpair(AF_LOCAL, SOCK_STREAM, 0, in_sock)) == -1)
    {
        throw EvaluationException("Error creating input socket");
    }

    if ((socketpair(AF_LOCAL, SOCK_STREAM, 0, out_sock)) == -1)
    {
        throw EvaluationException("Error creating output socket");
    }

    if ((socketpair(AF_LOCAL, SOCK_STREAM, 0, err_sock)) == -1)
    {
        throw EvaluationException("Error creating error socket");
    }

    if ((pid = fork()) != -1)
    {
        if (pid == 0)
        {
            close(in_sock[1]);
            close(out_sock[1]);
            close(err_sock[1]);
            // child
            close(STDIN_FILENO);
            if (_stdin != client_sock) dup(_stdin);
            else dup(in_sock[0]);

            close(STDOUT_FILENO);
            if (_stdout != client_sock) dup(_stdout);
            else { dup(out_sock[0]); }

            close(STDERR_FILENO);
            if (_stderr != client_sock) dup(_stderr);
            else dup(err_sock[0]);

            char* strings[MAX_ARGUMENTS] = {0};
            for (size_t i = 0; i < tokens.size(); i++)
            {
                Parser::Get()->trim(tokens[i]);
                strings[i] = strdup(tokens[i].c_str());
            }
            execvp(tokens[0].c_str(), strings);
            throw EvaluationException("Unable to run execvp on command [%s]", tokens[0].c_str());
        }
        else
        {
            close(in_sock[0]);
            close(out_sock[0]);
            close(err_sock[0]);
            int sts = 0;
            wait(&sts);
            if (_stdin == client_sock)
            {
                redirect(in_sock[1], client_sock, context);
                
            }
            if (_stdout == client_sock)
            {
                redirect(out_sock[1], client_sock, context);
                
            }
            if (_stderr == client_sock)
            {
                redirect(err_sock[1], client_sock, context);
                
            }
            close(in_sock[1]);
            close(out_sock[1]);
            close(err_sock[1]);
            return sts;
        }
    }
    else
    {
        throw EvaluationException("Cannot execute fork on command [%30s...]!", command.c_str());
    }
}

int Evaluate(std::string command, SSL* ssl)
{
    ExecutionContext* context = new ExecutionContext;
    context->ssl = ssl;
    int actualFD = SSL_get_fd(ssl);
    context->inputRedir.push(actualFD);
    context->outputRedir.push(actualFD);
    context->errorRedir.push(actualFD);

    std::shared_ptr<SyntaxTree> root;
    try
    {
        root = Parser::Get()->parse(command);
    }
    catch (ParserException& ex)
    {
        printf("Parser exception: %s\n", ex.what());
        return -1;
    }
    catch (VerificationException& ex)
    {
        printf("Verification exception: %s\n", ex.what());
        return -1;
    }

    int result = -1;
    try
    {
        result = Evaluate(root, context);
    }
    catch (EvaluationException& ex)
    {
        printf("Evaluation exception: %s\n", ex.what());
        return -1;
    }
    delete context;
    return result;
}

int Evaluate(std::shared_ptr<SyntaxTree> node, ExecutionContext* context)
{
    if (node == nullptr) return 0;
    if (node->type == OperationType::OUTPUT_REDIRECT)
    {
        std::shared_ptr<SyntaxTree> file = node->right;
        if (file == nullptr)
        {
            throw EvaluationException("Invalid redirect of [%s]", node->content.c_str());
        }
        int reg = open(file->content.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (reg == -1)
        {
            throw EvaluationException("Can't open redirect file [%s]", file->content.c_str());
        }
        context->outputRedir.push(reg);
        int result = Evaluate(node->left, context);
        context->outputRedir.pop();
        close(reg);
        return result;
    }
    else if (node->type == OperationType::INPUT_REDIRECT)
    {
        std::shared_ptr<SyntaxTree> file = node->right;
        if (file == nullptr)
        {
            throw EvaluationException("Invalid redirect of [%s]", node->content.c_str());
        }
        int reg = open(file->content.c_str(), O_RDONLY);
        if (reg == -1)
        {
            throw EvaluationException("Can't open redirect file [%s]", file->content.c_str());
        }
        context->inputRedir.push(reg);
        int result = Evaluate(node->left, context);
        context->inputRedir.pop();
        close(reg);
        return result;
    }
    else if (node->type == OperationType::ERROR_REDIRECT)
    {
        std::shared_ptr<SyntaxTree> file = node->right;
        if (file == nullptr)
        {
            throw EvaluationException("Invalid redirect of [%s]", node->content.c_str());
        }
        int reg = open(file->content.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (reg == -1)
        {
            throw EvaluationException("Can't open redirect file [%s]", file->content.c_str());
        }
        context->errorRedir.push(reg);
        int result = Evaluate(node->left, context);
        context->errorRedir.pop();
        close(reg);
        return result;
    }
    else if (node->type == OperationType::LOGICAL_AND)
    {
        int left_result = Evaluate(node->left, context);
        if (left_result == 0)
        {
            // Evaluation OK, proceed to right
            return Evaluate(node->right, context);
        }
        else
        {
            return left_result;
        }
    }
    else if (node->type == OperationType::LOGICAL_OR)
    {
        int left_result = Evaluate(node->left, context);
        if (left_result != 0)
        {
            // First failed, execute second
            return Evaluate(node->right, context);
        }
        else
        {
            return left_result;
        }
    }
    else if (node->type == OperationType::PIPE)
    {
        int d[2];
        if (pipe(d) == -1)
        {
            throw EvaluationException("Can't create a pipe");
        }
        context->outputRedir.push(d[1]);
        Evaluate(node->left, context);
        close(d[1]);
        context->outputRedir.pop();

        context->inputRedir.push(d[0]);
        int result = Evaluate(node->right, context);
        close(d[0]);
        context->inputRedir.pop();
        return result;
    }
    else if (node->type == OperationType::FOLLOWUP)
    {
        Evaluate(node->left, context);
        return Evaluate(node->right, context);
    }
    else if (node->type == OperationType::EXECUTE)
    {
        return Execute(
            node->content, 
            context->inputRedir.top(), 
            context->outputRedir.top(), 
            context->errorRedir.top(), 
            context
        );
    }
    else 
    {
        throw EvaluationException("Invalid operation type [%d]", node->type);
    }
}
