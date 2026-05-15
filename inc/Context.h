#ifndef SHELLANG_CONTEXT_H
#define SHELLANG_CONTEXT_H

#include "ICmd.h"
#include <unordered_map>
#include <string>
#include <filesystem>
#include <memory>
#include <iostream>

struct Context {
    std::unordered_map<std::string,std::shared_ptr<ICmd>> Commands;
    std::unordered_map<std::string,std::string> Variables;
    std::istream& InputStream;
    std::ostream& OutputStream;
    std::string CurrentPath;

    explicit Context(
        std::unordered_map<std::string,std::shared_ptr<ICmd>> Commands = {},
        std::unordered_map<std::string,std::string> Variables = {
        {"PI", "3.14159"}
        },
        std::istream& InputStream = std::cin,
        std::ostream& OutputStream = std::cout,
        std::string CurrentPath = std::filesystem::current_path().string()
    ) : Commands(std::move(Commands)),
        Variables(std::move(Variables)),
        InputStream(InputStream),
        OutputStream(OutputStream),
        CurrentPath(CurrentPath.empty()
            ? std::filesystem::current_path().string()
            : std::move(CurrentPath))
    {}
};

#endif