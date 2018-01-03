
#ifndef PARSER_H
#define PARSER_H

#include "../../common/include/Exceptions.h"
#include "../../common/include/Utility.h"

#include <string>
#include <map>
#include <iterator>
#include <queue>
#include <memory>
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <iostream>

/**
 * \brief Type of the operation of a Token or SyntaxTree node
 * 
 * Used for both parsing and execution logic
 */
enum OperationType
{
    LOGICAL_AND         = 0x0,  ///< &&
    LOGICAL_OR          = 0x1,  ///< ||
    INPUT_REDIRECT      = 0x2,  ///< <
    OUTPUT_REDIRECT     = 0x3,  ///< >
    ERROR_REDIRECT      = 0x4,  ///< 2>>
    FOLLOWUP            = 0x5,  ///< ;
    PIPE                = 0x6,  ///< |

    EXECUTE             = 0x7,  ///< Executable with arguments
    RIGHT_BRACKET       = 0x8,  ///< )
    LEFT_BRACKET        = 0x9   ///< (
};

/**
 * \brief Structure for holding a parsed token for later use
 * in Shunting Yard algorithm
 */
struct Token
{
    OperationType   type;               ///< Token type, each token must have the correct type
    std::string     content;            ///< The actual string content of the token
    unsigned long   position;           ///< The position in the input string where it was found
    int             precedence = -1;    ///< "Mathematical" precedence for Shunting Yard algorithm
    bool            is_operation = false;
};

/**
 * \brief Structure for holding a node of the AST
 */
struct SyntaxTree
{
    SyntaxTree() {};
    OperationType   type;
    std::string     content;
    std::shared_ptr<SyntaxTree> left = nullptr;
    std::shared_ptr<SyntaxTree> right = nullptr;
};

/**
 * \brief Genetic operator structure for holding operators like &&, ||, 2>> and others
 */
struct Operator
{
    OperationType   type;
    std::string     content;
    int             precedence;
};


/**
 * \brief A dedicated class for parsing a complex command and returning an AST of it
 */
class Parser {
public:

    /**
     * \brief Tokenizes and verifies the command. Returns an AST ready for execution
     * 
     * The function will tokenize the input, do a minimal verification of tokens
     * and will return a AST in the form of SyntaxTree root
     * Might throw:
     *      ParserException
     *      VerficationException
     * 
     * \param[in] command The complex query given by the user
     * \return A pointer to the AST root node of the respective command
     */
    std::shared_ptr<SyntaxTree> Parse(
        std::string command
    );


    /**
     * \brief Takes a single command with arguments and splits it in a vector
     * 
     * Example: "ls -la" with be tokenized into {"ls", "-la"}
     * 
     * \param command A executable with arguments
     * \param delim Characters that will be used as argument delimiters
     */
    std::vector<std::string> TokenizeExecute(
        std::string command, 
        std::string delim
    );

    /**
     * \brief Singleton get instance method
     * 
     * Allocates a instance if it was not yet created and returns a pointer to it
     * \return A pointer to a singleton instance of the class
     */
    static Parser* Get();

private:

    /**
     * Private constructor for singleton
     */
    Parser();

    /** 
     * Obtains the AST out of a token queue obtained after
     * The application of Shunting Yard algorithm
     * Might throw:
     *      ParserException
     */
    std::shared_ptr<SyntaxTree> GetSyntaxTree(
        std::vector<Token> &tokens
    );

    /**
     * Returns true if the string at the given position is an operator
     * &&, ||, (, ), |, ;, >, <, 2>>
     */
    bool IsOperator(
        std::string& original, 
        unsigned long position, 
        Token* foundOp = nullptr
    );

    /**
     * Divides the raw string into operable elements of type Token
     * It is the main parsing routine that uses the given user input
     * Might throw:
     *      ParserException
     */
    void Tokenize(
        std::string command, 
        std::vector<Token>& tokens
    );

    /**
     * Verifies the list of tokens for some obvious errors like
     * 1. Two consecutive operators
     * 2. Two consecutive execute tokens
     * Might throw:
     *      VerificationException
     */
    void Verify(
        std::vector<Token> &tokens
    );


    std::vector<Operator> operators;
    static Parser* instance;
};


#endif //PARSER_H
