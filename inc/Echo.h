#ifndef SHELLANG_ECHO_H
#define SHELLANG_ECHO_H
#include "ICmd.h"

struct Echo : ICmd {
    void Run(std::vector<std::string> args, Context& context, std::istream& is, std::ostream& os) override {
        for(const std::string& arg : args)
            os << arg << ' ';
    }
};

#endif