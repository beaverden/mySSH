//
// Created by denis on 12/15/17.
//

#include "../include/Exceptions.h"

ParserException::ParserException(size_t position, std::string message) :
    message(message), position(position)
{
    this->message = "Parser error: " + message + " at position " + std::to_string(position);
}

const char* ParserException::what()
{
    return message.c_str();
}

VerificationException::VerificationException(size_t position, std::string message) :
        message(message), position(position)
{
    this->message = "Verification error: " + message + " at position " + std::to_string(position);
}

const char* VerificationException::what()
{
    return message.c_str();
}


ArgumentsException::ArgumentsException(std::string message) :
        message(message)
{
    this->message = message;
}


const char* ArgumentsException::what()
{
    return message.c_str();
}


