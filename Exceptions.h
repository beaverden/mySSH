//
// Created by denis on 12/15/17.
//

#ifndef MYSSHSERVER_EXCEPTIONS_H
#define MYSSHSERVER_EXCEPTIONS_H
#include <stdexcept>

class ParserException : std::exception
{
public:
    ParserException(size_t position, std::string message);
    virtual ~ParserException() {};
    const char* what();
private:
    std::string message;
    size_t position;
};

class VerificationException : std::exception
{
public:
    VerificationException(size_t position, std::string message);
    virtual ~VerificationException() {};
    const char* what();
private:
    std::string message;
    size_t position;
};


#endif //MYSSHSERVER_EXCEPTIONS_H
