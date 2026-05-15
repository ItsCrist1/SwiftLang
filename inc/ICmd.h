#ifndef SHELLANG_ICMD_H
#define SHELLANG_ICMD_H

#include "Context.h"

#include <string>
#include <vector>

struct Context;

struct ICmd {
    static constexpr int EXIT_CODE = INT32_MAX;

    virtual int Run(const std::vector<std::string>&, Context&, std::istream&, std::ostream&) = 0;
    virtual ~ICmd() = default;
};

#endif