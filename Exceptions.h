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

class EvaluationException : std::exception
{
public:
    EvaluationException(std::string message);
    template<class ... Args>
    EvaluationException(const char* fmt, Args ... args)
    {
        char final[1000] = {0};
        snprintf(final, 1000, fmt, args...);
        this->message = std::string(final);
    }

    virtual ~EvaluationException() {};
    const char* what();
private:
    std::string message;
};


#endif //MYSSHSERVER_EXCEPTIONS_H
