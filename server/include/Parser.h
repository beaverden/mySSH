//
// Created by denis on 12/6/17.
//

#ifndef MYSSHSERVER_PARSER_H
#define MYSSHSERVER_PARSER_H

#include <string>
#include <map>
#include <iterator>
#include <queue>
#include <memory>
#include <stack>
#include <unordered_map>
#include "../../common/include/Exceptions.h"


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

    int precedence = -1;
    bool isOperation = false;
};

struct SyntaxTree
{
    SyntaxTree() {};
    OperationType type;
    std::string content;
    std::shared_ptr<SyntaxTree> left = nullptr;
    std::shared_ptr<SyntaxTree> right = nullptr;
};

struct Operator
{
    OperationType type;
    std::string content;
    int precedence;
};



class Parser {
public:

    /*
     * Tokenizes and verifies the command. Returns an AST ready for execution
     */
    std::shared_ptr<SyntaxTree> parse(std::string command);


    /*
     * Takes a single command with arguments and splits it in a vector
     */
    std::vector<std::string> tokenizeExecute(std::string command, std::string delim);

    /*
     * Singleton get instance method
     */
    static Parser* Get();

    /*
     * Trims a string of spaces and tabs
     */
    void trim(std::string &command);

private:

    /*
     * Private constructor for singleton
     */
    Parser();

    /* Obtains the AST out of a token queue obtained after
     * The application of Shunting Yard algorithm
     * Throws ParserException
     */
    std::shared_ptr<SyntaxTree> getSyntaxTree(std::vector<Token> &tokens);

    /*
     * Returns true if the string at the given position is a operator
     * &&, ||, (, ), |, ;, >, <, 2>>
     */
    bool isOperator(std::string& original, unsigned long position, Token* foundOp = nullptr);

    /*
     * Divides the raw string into operable elements of type Token
     * The main parsing routine that uses the given user input
     * Throws ParserException
     */
    void tokenize(std::string command, std::vector<Token>& tokens);

    /*
     * Verifies the list of tokens for some obvious errors like
     * 1. Two consecutive operators
     * 2. Two consecutive execute tokens
     * Throws VerificationException
     */
    void verify(std::vector<Token> &tokens);


    std::vector<Operator> operators;
    static Parser* instance;
};


#endif //MYSSHSERVER_PARSER_H
