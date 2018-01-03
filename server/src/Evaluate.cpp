//
// Created by denis on 12/27/17.
//

#include "../include/Evaluate.h"
#include "../../common/include/Packet.h"
#include "../../common/include/Utility.h"

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <wait.h>

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include <sys/socket.h>
#include <sys/ioctl.h>

void redirect_output(int local_socket, int client_socket, std::shared_ptr<ExecutionContext> context)
{
    int socket_size = 0;
    ioctl(local_socket, FIONREAD, &socket_size);

    char* buffer = new char[socket_size];
    int sz = 0;
    sz = read(local_socket, buffer, socket_size);
    send_packet(context->ssl, PACKET_RESPONSE, buffer, socket_size);
}

void redirect_input(int local_socket, int client_socket, std::shared_ptr<ExecutionContext> context)
{
    char buffer[1024] = {0};
    int sz = 0;
    while ((sz = SSL_read(context->ssl, buffer, 1024)) > 0)
    {
        if (write(local_socket, buffer, sz) <= 0)
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
    std::shared_ptr<ExecutionContext> context
)
{
    std::vector<std::string> tokens = Parser::Get()->TokenizeExecute(command, " ");
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
            else dup(out_sock[0]);

            close(STDERR_FILENO);
            if (_stderr != client_sock) dup(_stderr);
            else dup(err_sock[0]);

            char* strings[MAX_ARGUMENTS] = {0};
            for (size_t i = 0; i < tokens.size(); i++)
            {
                trim(tokens[i]);
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

            printf("Program child finished\n");
            if (_stdin == client_sock)
            {
                // TODO stdin
                //redirect_output(in_sock[1], client_sock, context);
            }
            if (_stdout == client_sock)
            {
                printf("Output to socket\n");
                redirect_output(out_sock[1], client_sock, context);   
            } 
            else printf("Output to other\n");

            if (_stderr == client_sock)
            {
                printf("Error to socket\n");
                redirect_output(err_sock[1], client_sock, context);     
            } else printf("Error to other\n");

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

void Evaluate(std::string command, SSL* ssl)
{
    std::shared_ptr<ExecutionContext> context = std::make_shared<ExecutionContext>();
    context->ssl = ssl;
    int actualFD = SSL_get_fd(ssl);
    context->inputRedir.push(actualFD);
    context->outputRedir.push(actualFD);
    context->errorRedir.push(actualFD);

    std::shared_ptr<SyntaxTree> root;
    root = Parser::Get()->Parse(command);
    Evaluate(root, context);
}

int Evaluate(std::shared_ptr<SyntaxTree> node, std::shared_ptr<ExecutionContext> context)
{
    if (node == nullptr) return 0;
    if (node->type == OperationType::OUTPUT_REDIRECT)
    {
        std::shared_ptr<SyntaxTree> file = node->right;
        if (file == nullptr)
        {
            throw EvaluationException("Invalid redirect of [%s]", node->content.c_str());
        }
        if (file->type != OperationType::EXECUTE)
        {
            throw EvaluationException("Operations disallowed in redirections. The name of the file is required");
        }
        int reg = open(file->content.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (reg == -1)
        {
            throw EvaluationException("Can't open redirect file [%s]", file->content.c_str());
        }
        context->outputRedir.push(reg);
        int result = Evaluate(node->left, context);
        close(reg);
        context->outputRedir.pop();
        return result;
    }
    else if (node->type == OperationType::INPUT_REDIRECT)
    {
        std::shared_ptr<SyntaxTree> file = node->right;
        if (file == nullptr)
        {
            throw EvaluationException("Invalid redirect of [%s]", node->content.c_str());
        }
        if (file->type != OperationType::EXECUTE)
        {
            throw EvaluationException("Operations disallowed in redirections. The name of the file is required");
        }
        int reg = open(file->content.c_str(), O_RDONLY);
        if (reg == -1)
        {
            throw EvaluationException("Can't open redirect file [%s]", file->content.c_str());
        }
        context->inputRedir.push(reg);
        int result = Evaluate(node->left, context);
        close(reg);
        context->inputRedir.pop();
        return result;
    }
    else if (node->type == OperationType::ERROR_REDIRECT)
    {
        std::shared_ptr<SyntaxTree> file = node->right;
        if (file == nullptr)
        {
            throw EvaluationException("Invalid redirect of [%s]", node->content.c_str());
        }
        if (file->type != OperationType::EXECUTE)
        {
            throw EvaluationException("Operations disallowed in redirections. The name of the file is required");
        }
        int reg = open(file->content.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (reg == -1)
        {
            throw EvaluationException("Can't open redirect file [%s]", file->content.c_str());
        }
        context->errorRedir.push(reg);
        int result = Evaluate(node->left, context);
        close(reg);
        context->errorRedir.pop();
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
        printf("Executing: %s\n", node->content.c_str());
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
