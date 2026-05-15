#ifndef SHELLANG_CLEAR_H
#define SHELLANG_CLEAR_H
#include "ICmd.h"

struct Clear : ICmd {
    static constexpr std::string CLEAR_ANSI = "\033[2J\033[H";

    int Run(const std::vector<std::string>& args, Context& context, std::istream& is, std::ostream& os) override {
        os << CLEAR_ANSI;
        return 0;
    }
};

#endif