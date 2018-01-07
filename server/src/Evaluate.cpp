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


int Execute(
    std::string command, 
    std::shared_ptr<ExecutionContext> ctx
)
{
    std::vector<std::string> tokens = Parser::getInstance()->tokenizeExecute(command, " ");
    if (tokens.size() > MAX_ARGUMENTS)
    {
        throw EvaluationException("Too many arguments in command [%30s...]", command.c_str());
    }
    int pid;
    if ((pid = fork()) != -1)
    {
        if (pid == 0)
        {
            // child
            close(STDIN_FILENO);
            dup(ctx->inputRedir.top());

            close(STDOUT_FILENO);
            dup(ctx->outputRedir.top());

            close(STDERR_FILENO);
            dup(ctx->errorRedir.top());

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
            int sts = 0;
            wait(&sts);
            return sts;
        }
    }
    else
    {
        throw EvaluationException("Cannot execute fork on command [%30s...]!", command.c_str());
    }
}

void Evaluate(std::string command, std::shared_ptr<ExecutionContext> ctx)
{
    Logger::log(LOG_EVAL, "Parsing: %s", command.c_str());
    std::shared_ptr<SyntaxTree> root;
    root = Parser::getInstance()->parseCommand(command);
    Evaluate_Tree(root, ctx);
}

int Evaluate_Tree(std::shared_ptr<SyntaxTree> node, std::shared_ptr<ExecutionContext> context)
{
    if (node == nullptr) return 0;
    Logger::log(LOG_EVAL_VERBOSE, "Node: [Type: %d, Content: %s]", node->type, node->content.c_str());
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
        int result = Evaluate_Tree(node->left, context);
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
        int result = Evaluate_Tree(node->left, context);
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
        int result = Evaluate_Tree(node->left, context);
        close(reg);
        context->errorRedir.pop();
        return result;
    }
    else if (node->type == OperationType::LOGICAL_AND)
    {
        int left_result = Evaluate_Tree(node->left, context);
        if (left_result == 0)
        {
            // Evaluation OK, proceed to right
            return Evaluate_Tree(node->right, context);
        }
        else
        {
            return left_result;
        }
    }
    else if (node->type == OperationType::LOGICAL_OR)
    {
        int left_result = Evaluate_Tree(node->left, context);
        if (left_result != 0)
        {
            // First failed, execute second
            return Evaluate_Tree(node->right, context);
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
        Evaluate_Tree(node->left, context);
        close(d[1]);
        context->outputRedir.pop();

        context->inputRedir.push(d[0]);
        int result = Evaluate_Tree(node->right, context);
        close(d[0]);
        context->inputRedir.pop();
        return result;
    }
    else if (node->type == OperationType::FOLLOWUP)
    {
        Evaluate_Tree(node->left, context);
        return Evaluate_Tree(node->right, context);
    }
    else if (node->type == OperationType::EXECUTE)
    {
        Logger::log(LOG_EVAL, "Executing: %s\n", node->content.c_str());
        return Execute(node->content, context);
    }
    else 
    {
        throw EvaluationException("Invalid operation type [%d]", node->type);
    }
}
