#include "../include/Exceptions.h"


ArgumentsException::ArgumentsException(std::string message) :
        message(message)
{
    this->message = message;
}


const char* ArgumentsException::what()
{
    return message.c_str();
}


