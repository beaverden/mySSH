//
// Created by denis on 12/27/17.
//

#ifndef MYSSHSERVER_EVALUATE_H
#define MYSSHSERVER_EVALUATE_H
#include <string>
#include "Parser.h"
#include <memory>
#include <stack>

int Evaluate(std::string command);
int Evaluate(std::shared_ptr<SyntaxTree> node);

#endif //MYSSHSERVER_EVALUATE_H
