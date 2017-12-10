//
// Created by denis on 12/6/17.
//

#ifndef MYSSHSERVER_PARSER_H
#define MYSSHSERVER_PARSER_H

#include <string>
#include <map>
#include <iterator>
#include <queue>

#define CODE_OK                              0x00000000
#define CODE_ERROR_BRACKETS_NOT_MATCHING     0x00000001
#define CODE_ERROR_TOO_MANY_OPERATORS        0x00000002
#define CODE_ERROR_EMTPY_TOKEN_LIST          0x00000003
#define CODE_ERROR_UNEXPECTED_TOKEN          0x00000004


typedef unsigned int RET_CODE;


enum OperationType
{
    LOGICAL_AND         = 0x0,
    LOGICAL_OR          = 0x1,
    INPUT_REDIRECT      = 0x2,
    OUTPUT_REDIRECT     = 0x3,
    ERROR_REDIRECT      = 0x4,
    FOLLOWUP            = 0x5,
    PIPE                = 0x6,

    EXECUTE             = 0x7,
    RIGHT_BRACKET       = 0x8,
    LEFT_BRACKET        = 0x9
};

struct Token
{
    OperationType type;
    std::string content;
    unsigned long position;
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
    bool isOperator(std::string& original, unsigned long position, Token* foundOp = nullptr);
    bool isOperator(OperationType type);

    void trim(std::string &command);

    RET_CODE tokenize(std::string command, std::vector<Token>& tokens);
    RET_CODE verify(std::vector<Token> &tokens, std::string& error);

    SyntaxTree* parse(std::string command);
    SyntaxTree* GetTree(std::queue<Token> &tokens);
private:
    std::map<std::string, unsigned int> percedence;
    unsigned int errorCode;
};


#endif //MYSSHSERVER_PARSER_H
