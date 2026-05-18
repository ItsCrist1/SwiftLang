#ifndef SHELLANG_PATHCHANGE_H
#define SHELLANG_PATHCHANGE_H

#include "ICmd.h"
#include "unistd.h"

struct PathChange : ICmd {
    int Run(const std::vector<std::string>& args, Context& context, std::istream& is, std::ostream& os) override {
        context.CurrentPath = resolvePath(args.size() > 0 ? args[0] : "");
        chdir(context.CurrentPath.c_str());
        return 0;
    }
};

#endif
