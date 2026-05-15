#ifndef SHELLANG_SHELL_H
#define SHELLANG_SHELL_H

#include "Lexer.h"
#include "Parser.h"
#include "Evaluator.h"

struct Shell {
    Shell(Context&);
    int Evaluate(Context&);
    int StarrREPL(Context&);

private:
    Context& context;

    Lexer lexer;
    Parser parser;
    Evaluator evaluator;
};

#endif
