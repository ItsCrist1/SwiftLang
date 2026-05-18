#ifndef SHELLANG_ICMD_H
#define SHELLANG_ICMD_H

#include "Context.h"

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

struct Context;

struct ICmd {
    static constexpr int EXIT_CODE = INT32_MAX;

    virtual int Run(const std::vector<std::string>&, Context&, std::istream&, std::ostream&) = 0;
    virtual ~ICmd() = default;

    std::string getFlags(const std::vector<std::string>&);
    std::vector<std::string> getDataArgs(const std::vector<std::string>&);

    void changePath(const std::vector<std::string>&, Context&);
    std::string resolvePath(std::string);

    std::string getReadableSize(double);
    double getFileSize(const std::filesystem::path&);
    double getDirectorySize(const std::filesystem::path&);
};

#endif
