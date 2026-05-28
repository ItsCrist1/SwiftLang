#ifndef SHELLANG_CONTEXT_H
#define SHELLANG_CONTEXT_H

#include "ICmd.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>
#include <iostream>

#include "ScopeUnit.h"

struct ICmd;

struct Context {
    std::shared_ptr<std::unordered_map<std::string,std::string>> GlobalVariables;
    std::shared_ptr<std::unordered_map<std::string,std::vector<std::string>>> GlobalArrays;

    std::unordered_map<std::string,std::shared_ptr<ICmd>> Commands;
    ScopeUnit<std::string> Variables;
    ScopeUnit<std::vector<std::string>> Arrays;
    std::istream& InputStream;
    std::ostream& OutputStream;
    std::string CurrentPath;

    explicit Context(
        std::unordered_map<std::string,std::shared_ptr<ICmd>> Commands = {},
        std::unordered_map<std::string,std::string> Variables = {},
        std::unordered_map<std::string,std::vector<std::string>> Arrays = {},
        std::istream& InputStream = std::cin,
        std::ostream& OutputStream = std::cout,
        std::string CurrentPath = std::filesystem::current_path().string()
    ) : GlobalVariables(std::make_shared<std::unordered_map<std::string,std::string>>(std::move(Variables))),
        GlobalArrays(std::make_shared<std::unordered_map<std::string,std::vector<std::string>>>(std::move(Arrays))),
        Commands(std::move(Commands)),
        Variables(GlobalVariables),
        Arrays(GlobalArrays),
        InputStream(InputStream),
        OutputStream(OutputStream),
        CurrentPath(CurrentPath.empty()
                        ? std::filesystem::current_path().string()
                        : std::move(CurrentPath)) {}
};

#endif
