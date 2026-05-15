#ifndef SHELLANG_EXIT_H
#define SHELLANG_EXIT_H
#include "ICmd.h"

struct Exit : ICmd {
    int Run(const std::vector<std::string>& args, Context& context, std::istream& is, std::ostream& os) override {
        return EXIT_CODE;
    }
};

#endif