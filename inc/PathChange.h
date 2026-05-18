#ifndef SHELLANG_PATHCHANGE_H
#define SHELLANG_PATHCHANGE_H

#include "ICmd.h"

struct PathChange : ICmd {
    int Run(const std::vector<std::string>& args, Context& context, std::istream& is, std::ostream& os) override {
        changePath(args, context);
        return 0;
    }
};

#endif
