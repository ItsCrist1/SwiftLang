#ifndef SHELLANG_ICMD_H
#define SHELLANG_ICMD_H

#include "Context.h"

#include <string>
#include <vector>

struct Context;

struct ICmd {
    virtual void Run(std::vector<std::string> args, Context& context, std::istream&, std::ostream&) = 0;
    virtual ~ICmd() = default;
};

#endif