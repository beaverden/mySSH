//
// Created by denis on 12/6/17.
//

#include "Parser.h"
#include <queue>
#include <stack>
#include <algorithm>
#include <stdexcept>
#include <iostream>

Parser::Parser() { init(); }

void Parser::init()
{
    percedence["&&"] = 0;
    percedence["||"] = 0;
    percedence[";"] = 0;
    percedence[">"] = 1;
    percedence["<"] = 1;
    percedence["2>>"] = 1;

}

const static std::vector<std::pair<std::string, OperationType>> operators =
{
        {"2>>", ERROR_REDIRECT  },
        {"&&",  LOGICAL_AND     },
        {"||",  LOGICAL_OR      },
        {"<",   INPUT_REDIRECT  },
        {">",   OUTPUT_REDIRECT },
        {"|",   PIPE            },
        {";",   FOLLOWUP        }
};

bool Parser::isOperator(std::string& original, unsigned long position, Token* foundOp) {
    for (auto op : operators)
    {
        unsigned long sz = op.first.length();
        if (position + sz <= original.length())
        {
            std::string possible = original.substr(position, sz);
            if (op.first == possible)
            {
                if (foundOp != nullptr)
                {
                    foundOp->content = op.first;
                    foundOp->position = position;
                    foundOp->type = op.second;
                }
                return true;
            }
        }
    }
    return false;
}

bool Parser::isOperator(OperationType type)
{
    return  (    type == LOGICAL_AND
              || type == LOGICAL_OR
              || type == INPUT_REDIRECT
              || type == OUTPUT_REDIRECT
              || type == ERROR_REDIRECT
              || type == FOLLOWUP
              || type == PIPE);
}


void Parser::trim(std::string &command)
{
    while (!command.empty() && std::isspace(command[0]))
    {
        command.erase(0, 1);
    }
    while (!command.empty() &&std::isspace(command[command.length()-1]))
    {
        command.erase(command.length()-1, 1);
    }
    while (!command.empty() && command[0] == '(' && command[command.length()-1] == ')')
    {
        command.erase(0, 1);
        command.erase(command.length()-1, 1);
    }
}


SyntaxTree* Parser::GetTree(std::queue<Token> &tokens)
{
    std::queue<SyntaxTree*> nodes;
    while (!tokens.empty())
    {
        Token t = tokens.front();
        tokens.pop();
        SyntaxTree* node = new SyntaxTree;
        node->content = t.content;
        node->type = t.type;
        nodes.push(node);
    }
    /*
    std::stack<SyntaxTree*> commStack;
    while (!nodes.empty())
    {
            Token top = tokens.front();
            tokens.pop();
            if (top.type == OperationType::EXECUTE)
            {
                tkStack.push(top);
            }
            else
            {
                SyntaxTree* node = new SyntaxTree;
                node->type = top.type;
                node->content = top.content;

                Token lnode = tkStack.top();
                tkStack.pop();
                Token rnode = tkStack.top();
                tkStack.pop();
                node->left = new SyntaxTree;
                node->left->content = lnode.content;
                node->left->
            }
        }
    }
    */
    return nullptr;
}


RET_CODE Parser::tokenize(std::string command, std::vector<Token> &tokens)
{
    tokens.clear();
    this->trim(command);
    std::string currentToken = "";
    bool inCommand = true;
    bool inString = false;
    char stringStart;

    unsigned long currentPosition = 0;
    while (currentPosition < command.length())
    {
        char c = command[currentPosition];
        Token t;
        if (inString) {
            if (c == stringStart) {
                inString = false;
                stringStart = 0;
            }
            currentToken += c;
            currentPosition++;
        }
        else // Not in string
        {
            if (c == '"' || c == '\'') {
                inString = true;
                stringStart = c;
                currentToken += c;
                currentPosition++;
                continue;
            }
            /* Found a operator next, save the current token */
            if (isOperator(command, currentPosition, nullptr) || c == ')' || c == '(')
            {
                trim(currentToken);
                if (!currentToken.empty())
                {
                    Token comm;
                    comm.content = currentToken;
                    comm.type = EXECUTE;
                    comm.position = currentPosition - currentToken.length();
                    currentToken.clear();
                    tokens.push_back(comm);
                }
            }

            /* Process other tokens */
            if (isOperator(command, currentPosition, &t))
            {
                currentPosition += t.content.length();
                tokens.push_back(t);
            }
            else if (c == ')')
            {
                t.content = ")";
                t.position = currentPosition;
                t.type = RIGHT_BRACKET;
                currentPosition++;
                tokens.push_back(t);
            }
            else if (c == '(')
            {
                t.content = "(";
                t.position = currentPosition;
                t.type = LEFT_BRACKET;
                currentPosition++;
                tokens.push_back(t);
            }
            else {
                currentToken += c;
                currentPosition++;
            }
        }
    }
    if (!currentToken.empty())
    {
        Token comm;
        comm.content = currentToken;
        comm.type = EXECUTE;
        comm.position = currentPosition - currentToken.length();
        currentToken.clear();
        tokens.push_back(comm);
    }

    if (tokens.size() == 0)
    {
        return CODE_ERROR_EMTPY_TOKEN_LIST;
    }
    return CODE_OK;
}

RET_CODE Parser::verify(std::vector<Token> &tokens, std::string& error)
{
    for (unsigned long i = 0; i < tokens.size() - 1; i++)
    {
        if (isOperator(tokens[i].type) && isOperator(tokens[i+1].type))
        {
            error = "Two tokens one after another at " + std::to_string(i);
            return CODE_ERROR_UNEXPECTED_TOKEN;
        }
    }
}

SyntaxTree* Parser::parse(std::string command)
{
    std::vector<Token> tokens;
    RET_CODE code = tokenize(command, tokens);

    std::queue <Token> shTokens;
    std::stack <Token> shOperators;

    for (Token t : tokens)
    {
        if (isOperator(t.type))
        {
            while (!shOperators.empty()) {
                if (percedence.find(t.content) == percedence.end()) {
                    throw std::invalid_argument("Operator content not found");
                }
                if (percedence[shOperators.top().content] > percedence[t.content]) {
                    Token p = shOperators.top();
                    shTokens.push(p);
                    shOperators.pop();
                } else if (percedence[shOperators.top().content] == percedence[t.content]
                           && shOperators.top().content != "(") {
                    Token p = shOperators.top();
                    shTokens.push(p);
                    shOperators.pop();
                } else break;
            }
            shOperators.push(t);
        }
        else if (t.type == LEFT_BRACKET) {
            shOperators.push(t);
        }
        else if (t.type == RIGHT_BRACKET) {
            while (!shOperators.empty() && shOperators.top().content != "(") {
                Token t = shOperators.top();
                shTokens.push(t);
                shOperators.pop();
            }
            if (shOperators.empty()) {
                this->errorCode = CODE_ERROR_BRACKETS_NOT_MATCHING;
                return nullptr;
            }
            else
            {
                shOperators.pop();
            }

        }
        else if (t.type == EXECUTE)
        {
            shTokens.push(t);
        }

    }

    while (!shOperators.empty())
    {
        if (shOperators.top().type == OperationType::LEFT_BRACKET ||
            shOperators.top().type == OperationType::RIGHT_BRACKET)
        {
            this->errorCode = CODE_ERROR_BRACKETS_NOT_MATCHING;
            return nullptr;
        }
        else
        {
            Token t = shOperators.top();
            shTokens.push(t);
            shOperators.pop();
        }
    }

    while (!shTokens.empty())
    {
        Token t = shTokens.front();
        std::cout << t.type << " " << t.content << std::endl;
        shTokens.pop();
    }

    return nullptr;
}
