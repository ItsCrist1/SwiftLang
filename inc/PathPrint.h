#ifndef SHELLANG_PATHPRINT_H
#define SHELLANG_PATHPRINT_H

#include "ICmd.h"

struct PathPrint : ICmd {
    void Run(std::vector<std::string> args, Context& context, std::istream& is, std::ostream& os) override {
        os << context.CurrentPath;
    }
};

#endif