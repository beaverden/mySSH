#include "../include/Evaluate.h"

std::string get_cwd()
{
    char cwd[1024] = {};
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        return std::string(cwd);
    } 
    else
    {
        return ".";
    }
}

int Execute(
    std::string command, 
    std::shared_ptr<ExecutionContext> ctx
)
{
    std::vector<std::string> tokens = Parser::getInstance()->tokenizeExecute(command, " ");
    if (tokens.size() > MAX_ARGUMENTS)
    {
        throw EvaluationException("Too many arguments in command [%.30s...]", command.c_str());
    }
    if (tokens[0] == "login")
    {
        // TODO login
    }
/*
    if (ctx->username == "$_anonymous_$")
    {
        throw AuthException("Unauthorized. Please use command \"login [username] [password]\" to authenticate");
    } */
    if (tokens[0] == "exit")
    {
        throw ExitException("Exit called.");
    }
    else if (tokens[0] == "cd")
    {
        int result = chdir(tokens[1].c_str());
        ctx->currentDir = get_cwd();
        return result;
    }
    int pid;
    int input = ctx->inputRedir.top();
    int output = ctx->outputRedir.top();
    int error = ctx->errorRedir.top();

    if ((pid = fork()) != -1)
    {
        if (pid == 0)
        {
            // Child
            if (input != STDIN_FILENO)
            {
                close(STDIN_FILENO);
                dup(input);
            }
            
            if (output != STDOUT_FILENO)
            {
                close(STDOUT_FILENO);
                dup(output);
            }
            if (error != STDERR_FILENO)
            {
                close(STDERR_FILENO);
                dup(error);
            }

            char* strings[MAX_ARGUMENTS] = {0};
            for (size_t i = 0; i < tokens.size(); i++)
            {
                trim(tokens[i]);
                strings[i] = strdup(tokens[i].c_str());
            }
            execvp(tokens[0].c_str(), strings);
            printf("Unable to execute command [%.30s]\n", tokens[0].c_str());
            fflush(stdout);
            exit(1);
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
        throw EvaluationException("Cannot execute fork on command [%.30s]!", command.c_str());
    }
}

void Evaluate(std::string command, std::shared_ptr<ExecutionContext> ctx)
{
    while (!ctx->inputRedir.size() > 1) ctx->inputRedir.pop();
    while (!ctx->outputRedir.size() > 1) ctx->outputRedir.pop();
    while (!ctx->errorRedir.size() > 1) ctx->errorRedir.pop();

    trim(command, "\t\n\r ");
    if (command.length() == 0) return;
    std::shared_ptr<SyntaxTree> root;
    root = Parser::getInstance()->parseCommand(command);
    Evaluate_Tree(root, ctx);
}

int Evaluate_Tree(std::shared_ptr<SyntaxTree> node, std::shared_ptr<ExecutionContext> context)
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
        int d[2] = {0};
        if (pipe(d) == -1)
        {
            throw EvaluationException("Can't create a pipe");
        }
        context->outputRedir.push(d[1]);
        int result1 = Evaluate_Tree(node->left, context);
        close(d[1]);
        context->outputRedir.pop();

        if (result1 != 0) 
        {
            int read_result = 0;
            ioctl(d[0], FIONREAD, &read_result);
            if (read_result > 0)
            {
                char* buff = new char[read_result + 1]();
                read(d[0], buff, read_result);
                printf("%s", buff);
                fflush(stdout);
                delete buff;
            }
            close(d[0]);
            return result1;
        }
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
        return Execute(node->content, context);
    }
    else 
    {
        throw EvaluationException("Invalid operation type [%d]", node->type);
    }
}
