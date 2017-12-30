//
// Created by denis on 12/27/17.
//

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <wait.h>
#include "../include/Evaluate.h"

std::stack<int> inputRedir;
std::stack<int> outputRedir;
std::stack<int> errorRedir;


int Execute(std::string command, int _stdin, int _stdout, int _stderr)
{
    std::vector<std::string> tokens = Parser::Get()->tokenizeExecute(command, " ");
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
            if (_stdout != STDOUT_FILENO)
            {
                close(STDOUT_FILENO);
                dup(_stdout);
            }
            if (_stdin != STDIN_FILENO)
            {
                close(STDIN_FILENO);
                dup(_stdin);
            }
            if (_stderr != STDERR_FILENO)
            {
                close(STDERR_FILENO);
                dup(_stderr);
            }

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

int Evaluate(std::string command)
{
    while (!inputRedir.empty()) inputRedir.pop();
    while (!outputRedir.empty()) outputRedir.pop();
    while (!errorRedir.empty()) errorRedir.pop();
    inputRedir.push(STDIN_FILENO);
    outputRedir.push(STDOUT_FILENO);
    errorRedir.push(STDERR_FILENO);

    std::shared_ptr<SyntaxTree> root;
    root = Parser::Get()->parse(command);

    if (root == nullptr)
    {
        return 0;
    }

    return Evaluate(root);
}

int Evaluate(std::shared_ptr<SyntaxTree> node)
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
        outputRedir.push(reg);
        int result = Evaluate(node->left);
        outputRedir.pop();
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
        inputRedir.push(reg);
        int result = Evaluate(node->left);
        inputRedir.pop();
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
        errorRedir.push(reg);
        int result = Evaluate(node->left);
        errorRedir.pop();
        close(reg);
        return result;
    }
    else if (node->type == OperationType::LOGICAL_AND)
    {
        int left_result = Evaluate(node->left);
        if (left_result == 0)
        {
            // Evaluation OK, proceed to right
            return Evaluate(node->right);
        }
        else
        {
            return left_result;
        }
    }
    else if (node->type == OperationType::LOGICAL_OR)
    {
        int left_result = Evaluate(node->left);
        if (left_result != 0)
        {
            // First failed, execute second
            return Evaluate(node->right);
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
        outputRedir.push(d[1]);
        Evaluate(node->left);
        close(d[1]);
        outputRedir.pop();

        inputRedir.push(d[0]);
        int result = Evaluate(node->right);
        close(d[0]);
        inputRedir.pop();
        return result;
    }
    else if (node->type == OperationType::FOLLOWUP)
    {
        Evaluate(node->left);
        return Evaluate(node->right);
    }
    else if (node->type == OperationType::EXECUTE)
    {
        return Execute(node->content, inputRedir.top(), outputRedir.top(), errorRedir.top());
    }
    else 
    {
        throw EvaluationException("Invalid operation type [%d]", node->type);
    }
}