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

class ArgumentsException : std::exception
{
public:
    ArgumentsException(std::string message);
    template<class ... Args>
    ArgumentsException(const char* fmt, Args ... args)
    {
        char final[1000] = {0};
        snprintf(final, 1000, fmt, args...);
        this->message = std::string(final);
    }

    virtual ~ArgumentsException() {};
    const char* what();
private:
    std::string message;
};

class EvaluationException : public ArgumentsException
{
public:
    EvaluationException(std::string message) 
    : ArgumentsException(message) {}
    template<class ... Args>
    EvaluationException(const char* fmt, Args ... args) 
    : ArgumentsException(fmt, args...) {};
};

class SecurityException : public ArgumentsException
{
public:
    SecurityException(std::string message) 
    : ArgumentsException(message) {}
    template<class ... Args>
    SecurityException(const char* fmt, Args ... args) 
    : ArgumentsException(fmt, args...) {};
};

class ServerException : public ArgumentsException
{
public:
    ServerException(std::string message) 
    : ArgumentsException(message) {}
    template<class ... Args>
    ServerException(const char* fmt, Args ... args) 
    : ArgumentsException(fmt, args...) {};
};



#endif //MYSSHSERVER_EXCEPTIONS_H
