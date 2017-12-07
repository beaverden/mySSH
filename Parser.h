//
// Created by denis on 12/6/17.
//

#ifndef MYSSHSERVER_PARSER_H
#define MYSSHSERVER_PARSER_H

#include <string>
#include <map>
#include <iterator>
#include <queue>

#define ERROR_BRACKETS_NOT_MATCHING     0x00000001
#define ERROR_TOO_MANY_OPERATORS        0x00000002


enum OperationType
{
    EXECUTE,
    LOGICAL_AND,
    LOGICAL_OR,
    INPUT_REDIRECT,
    OUTPUT_REDIRECT,
    ERROR_REDIRECT,
    FOLLOWUP,
    PIPE,
    BRACKET
};

struct Token
{
    OperationType type;
    std::string content;
    long position;
};

struct SyntaxTree
{
    OperationType type;
    std::string content;
    SyntaxTree* left = nullptr;
    SyntaxTree* right = nullptr;
};

class Parser {
public:
    Parser();
    void init();
    bool isOperator(std::string::iterator it, std::string &str);
    bool getOperator(std::string::iterator it, std::string &str, Token* tk);
    bool isBracket(std::string::iterator it, std::string &str);
    void trim(std::string &command);

    SyntaxTree* parse(std::string command);
    SyntaxTree* GetTree(std::queue<Token> &tokens);
private:
    std::map<std::string, unsigned int> percedence;
    int errorCode;
};


#endif //MYSSHSERVER_PARSER_H
