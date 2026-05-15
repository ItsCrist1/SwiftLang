#ifndef SHELLANG_ECHO_H
#define SHELLANG_ECHO_H
#include "ICmd.h"

struct Echo : ICmd {
    int Run(const std::vector<std::string>& args, Context& context, std::istream& is, std::ostream& os) override {
        for(size_t i=0; i < args.size(); ++i)
            os << args[i] << (i == args.size()-1 ? '\n' : ' ');

        return 0;
    }
};

#endif