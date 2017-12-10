#include <iostream>
#include "Parser.h"

int main() {
    Parser p;
    p.parse("./a.out > file  || ./a2.out");
    printf("\n");
    p.parse("(./a.out  || ./a2.out) > file");
    printf("\n");
    p.parse("./a.out \"param && ||\" &&  || ./a2.out > file)()()(");
    p.parse("./a.out \"param && ||\" &&  || ./a2.out > file)()()(&&");
    return 0;
}