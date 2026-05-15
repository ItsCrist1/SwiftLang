#ifndef SHELLANG_SHELL_H
#define SHELLANG_SHELL_H

#include "Lexer.h"
#include "Parser.h"
#include "Evaluator.h"

struct Shell {
    Shell(Context&);
    int Evaluate(const std::string&, std::optional<Context> context=std::nullopt);
    int StartREPL(std::optional<Context> context=std::nullopt);

private:
    std::unique_ptr<Context> context;

    Lexer lexer;
    Parser parser;
    Evaluator evaluator;
};

#endif
