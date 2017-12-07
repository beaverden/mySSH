#include <iostream>
#include "Parser.h"

int main() {
    Parser p;
    p.parse("hello;mata;&& hello");
    return 0;
}