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
    percedence["&&"] = 3;
    percedence["||"] = 3;
    percedence[";"] = 3;
    percedence[">"] = 2;
    percedence["<"] = 2;
    percedence["2>>"] = 2;

}

bool Parser::isOperator(std::string::iterator it, std::string &str) {
    if (it != str.end() && std::next(it) != str.end() && std::next(std::next(it)) != str.end())
    {
        if (std::string(it, it+3) == "2>>")
        {
            return true;
        }
    }
    if (it != str.end() && std::next(it) != str.end())
    {
        if (std::string(it, it+2) == "&&" || std::string(it, it+2) == "||")
        {
            return true;
        }
    }

    if (it != str.end())
    {
        if ((*it) == '>' || (*it) == '<' || (*it) == ';')
        {
            return true;
        }
    }
    return false;
}

bool Parser::isBracket(std::string::iterator it, std::string &str)
{
    return ((*it) == '(' || (*it) == ')');
}

bool Parser::getOperator(std::string::iterator it, std::string &str, Token* tk)
{
    if (tk == nullptr)
    {
        throw std::invalid_argument("Token is a null pointer!");
    }
    if (it == str.end())
    {
        throw std::out_of_range("Iterator out of range");
    }

    if (it != str.end() && std::next(it) != str.end() && std::next(std::next(it)) != str.end())
    {
        if (std::string(it, it+3) == "2>>")
        {
            tk->content = "2>>";
            tk->type = ERROR_REDIRECT;
            tk->position = std::distance(str.begin(), it);
            return true;
        }
    }
    if (it != str.end() && std::next(it) != str.end())
    {
        if (std::string(it, it+2) == "&&")
        {
            tk->content = "&&";
            tk->type = LOGICAL_AND;
            tk->position = std::distance(str.begin(), it);
            return true;
        }
        if (std::string(it, it+2) == "||")
        {
            tk->content = "||";
            tk->type = LOGICAL_OR;
            tk->position = std::distance(str.begin(), it);
            return true;
        }
    }

    if (it != str.end())
    {
        if ((*it) == '>')
        {
            tk->content = ">";
            tk->type = OUTPUT_REDIRECT;
            tk->position = std::distance(str.begin(), it);
            return true;
        }
        if ((*it) == '<')
        {
            tk->content = "<";
            tk->type = INPUT_REDIRECT;
            tk->position = std::distance(str.begin(), it);
            return true;
        }
        if ((*it) == ';')
        {
            tk->content = ";";
            tk->type = FOLLOWUP;
            tk->position = std::distance(str.begin(), it);
            return true;
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

SyntaxTree* Parser::parse(std::string command)
{
    std::queue <Token> tokens;
    std::stack <Token> operators;
    this->trim(command);
    auto iter = command.begin();
    std::string currentToken = "";
    bool inCommand = true;
    bool inString = false;
    char stringStart;

    while (iter != command.end())
    {
        char c = *iter;
        Token t;
        if (inString) {
            if (c == stringStart) {
                inString = false;
                stringStart = 0;
            }
            currentToken += c;
            iter++;
        }
        else // Not in string
        {
            if (c == '"' || c == '\'') {
                inString = true;
                stringStart = c;
                currentToken += c;
                iter++;
                continue;
            }
            if (isOperator(iter, command) || isBracket(iter, command))
            {
                trim(currentToken);
                if (!currentToken.empty())
                {

                    Token comm;
                    comm.content = currentToken;
                    comm.type = EXECUTE;
                    comm.position = std::distance(command.begin(), iter) - currentToken.length();
                    currentToken.clear();
                    tokens.push(comm);
                }
            }
            if (getOperator(iter, command, &t)) {
                while (!operators.empty()) {
                    if (percedence.find(t.content) == percedence.end()) {
                        throw std::invalid_argument("Operator content not found");
                    }
                    if (percedence[operators.top().content] > percedence[t.content]) {
                        Token p = operators.top();
                        tokens.push(p);
                        operators.pop();
                    } else if (percedence[operators.top().content] == percedence[t.content]
                               && operators.top().content != "(") {
                        Token p = operators.top();
                        tokens.push(p);
                        operators.pop();
                    } else break;
                }
                operators.push(t);
            } else if (c == '(') {
                t.position = std::distance(command.begin(), iter);
                t.type = BRACKET;
                t.content = "(";
                operators.push(t);
            }
                //Right bracket
            else if (c == ')') {
                t.position = std::distance(command.begin(), iter);
                t.type = BRACKET;
                t.content = ")";

                while (!operators.empty() && operators.top().content != "(") {
                    Token t = operators.top();
                    tokens.push(t);
                    operators.pop();
                }
                if (operators.empty()) {
                    this->errorCode = ERROR_BRACKETS_NOT_MATCHING;
                    return nullptr;
                }
                operators.pop();
            } else {
                currentToken += c;
                iter++;
                continue;
            }
            iter += t.content.length();

        }
    }
    if (!currentToken.empty())
    {
        Token comm;
        comm.content = currentToken;
        comm.type = EXECUTE;
        comm.position = std::distance(command.begin(), iter) - currentToken.length();
        currentToken.clear();
        tokens.push(comm);
    }
    while (!operators.empty())
    {
        if (operators.top().type == OperationType::BRACKET)
        {
            this->errorCode = ERROR_BRACKETS_NOT_MATCHING;
            return nullptr;
        }
        else
        {
            Token t = operators.top();
            tokens.push(t);
            operators.pop();
        }
    }

    while (!tokens.empty())
    {
        Token t = tokens.front();
        std::cout << t.type << " " << t.content << std::endl;
        tokens.pop();
    }

    return GetTree(tokens);
}
