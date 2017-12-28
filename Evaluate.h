//
// Created by denis on 12/27/17.
//

#ifndef MYSSHSERVER_EVALUATE_H
#define MYSSHSERVER_EVALUATE_H
#include <string>
#include "Parser.h"
#include <memory>
#include <stack>

#define MAX_ARGUMENTS 100

int Execute(std::string command, int _stdin, int _stdout, int _stderr);
int Evaluate(std::string command);
int Evaluate(std::shared_ptr<SyntaxTree> node);

#endif //MYSSHSERVER_EVALUATE_H
