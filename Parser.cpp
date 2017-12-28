//
// Created by denis on 12/6/17.
//

#include "Parser.h"
#include <queue>
#include <stack>
#include <algorithm>
#include <memory>
#include <iostream>

template <class T>
void freeStack(std::stack<T*> &st)
{
    while (!st.empty())
    {
        T* ptr = st.top();
        delete ptr;
    }
}

Parser* Parser::instance = nullptr;
Parser::Parser()
{
    operators.push_back({OperationType::ERROR_REDIRECT, "2>>", 1});
    operators.push_back({OperationType::LOGICAL_AND, "&&", 0});
    operators.push_back({OperationType::LOGICAL_OR, "||", 0});
    operators.push_back({OperationType::FOLLOWUP, ";", 0});
    operators.push_back({OperationType::PIPE, "|", 0});
    operators.push_back({OperationType::INPUT_REDIRECT, "<", 1});
    operators.push_back({OperationType::OUTPUT_REDIRECT, ">", 1});
}


Parser* Parser::Get()
{
    if (instance == nullptr)
    {
        instance = new Parser();
    }
    return instance;
}

bool Parser::isOperator(std::string& original, unsigned long position, Token* foundOp) {
    for (auto op : operators)
    {
        unsigned long sz = op.content.length();
        if (position + sz <= original.length())
        {
            std::string possible = original.substr(position, sz);
            if (op.content == possible)
            {
                if (foundOp != nullptr)
                {
                    foundOp->content = op.content;
                    foundOp->position = position;
                    foundOp->type = op.type;
                    foundOp->precedence = op.precedence;
                    foundOp->isOperation = true;
                }
                return true;
            }
        }
    }
    return false;
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
}


void Parser::tokenize(std::string command, std::vector<Token> &tokens)
{
    tokens.clear();
    trim(command);
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
                    comm.precedence = -1;

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
                t.isOperation = false;
                currentPosition++;
                tokens.push_back(t);
            }
            else if (c == '(')
            {
                t.content = "(";
                t.position = currentPosition;
                t.type = LEFT_BRACKET;
                t.isOperation = false;
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
        throw ParserException(0, "No tokens found");
    }
}

void Parser::verify(std::vector<Token> &tokens)
{
    for (unsigned long i = 0; i < tokens.size() - 1; i++)
    {
        if (tokens[i].isOperation && tokens[i+1].isOperation)
        {
            throw VerificationException(
                    tokens[i].position,
                    std::string("Two consecutive operators ")
                    + "(" + tokens[i].content + ", " + tokens[i+1].content + ")"
            );
        }
    }
}

std::shared_ptr<SyntaxTree> Parser::getSyntaxTree(std::vector<Token> &tokens) {
    std::queue <Token> shTokens;
    std::stack <Token> shOperators;

    for (Token t : tokens)
    {
        if (t.isOperation)
        {
            while (!shOperators.empty())
            {
                if (shOperators.top().precedence > t.precedence)
                {
                    Token p = shOperators.top();
                    shTokens.push(p);
                    shOperators.pop();
                } else if (shOperators.top().type != LEFT_BRACKET
                           && shOperators.top().precedence == t.precedence)
                {
                    Token p = shOperators.top();
                    shTokens.push(p);
                    shOperators.pop();
                } else break;
            }
            shOperators.push(t);
        }
        else if (t.type == LEFT_BRACKET)
        {
            shOperators.push(t);
        }
        else if (t.type == RIGHT_BRACKET)
        {
            while (!shOperators.empty() && shOperators.top().type != LEFT_BRACKET)
            {
                Token t = shOperators.top();
                shTokens.push(t);
                shOperators.pop();
            }

            if (shOperators.empty())
            {
                throw ParserException(0, "Brackets not matching");
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
            throw ParserException(0, "Brackets not matching");
        }
        else
        {
            Token t = shOperators.top();
            shTokens.push(t);
            shOperators.pop();
        }
    }

    std::stack<std::shared_ptr<SyntaxTree>> trees;


    while (!shTokens.empty())
    {
        Token t = shTokens.front();
        if (t.type == OperationType::EXECUTE)
        {
            std::shared_ptr<SyntaxTree> newTree = std::make_shared<SyntaxTree>();
            newTree->type = t.type;
            newTree->content = t.content;
            trees.push(std::move(newTree));
        }
        else
        {
            std::shared_ptr<SyntaxTree> opTree = std::make_shared<SyntaxTree>();
            opTree->type  = t.type;
            opTree->content = t.content;

            std::shared_ptr<SyntaxTree> t1 = nullptr;
            std::shared_ptr<SyntaxTree> t2 = nullptr;

            if (trees.empty())
            {
                throw ParserException(t.position, "Operator " + t.content + " doesn't have a operand");
            }
            t1 = trees.top();
            trees.pop();

            if (t.type != EXECUTE)
            {
                if (trees.empty())
                {
                    throw ParserException(t.position, "Operator " + t.content + " takes two operands");
                }
                t2 = trees.top();
                trees.pop();
            }
            opTree->left = t2;
            opTree->right = t1;
            if (t2 != nullptr)
            {
                t2->parent = opTree;
            }
            if (t1 != nullptr)
            {
                t1->parent = opTree;
            }
            trees.push(opTree);
        }

        //std::cout << t.type << " " << t.content << std::endl;

        shTokens.pop();
    }

    if (trees.size() != 1)
    {
        throw ParserException(0, "Too many operations");
    }
    return std::move(trees.top());
}

std::vector<std::string> Parser::tokenizeExecute(std::string s, std::string delim)
{
    std::vector<std::string> v;
    trim(s);
    while (s.length() != 0 && s.find(delim) != std::string::npos)
    {
        std::string token = s.substr(0, s.find(delim));
        v.push_back(token);
        s.erase(0, s.find(delim) + delim.length());
        trim(s);
    }
    if (s.length() != 0)
    {
        trim(s);
        v.push_back(s);
    }
    return v;
}


std::shared_ptr<SyntaxTree> Parser::parse(std::string command)
{
    std::vector<Token> tokens;
    tokenize(command, tokens);

    verify(tokens);

    std::shared_ptr<SyntaxTree> ptr = getSyntaxTree(tokens);

    return ptr;
}
