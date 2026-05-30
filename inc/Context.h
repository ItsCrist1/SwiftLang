#ifndef SHELLANG_CONTEXT_H
#define SHELLANG_CONTEXT_H

#include "ICmd.h"
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>
#include <iostream>

#include "ScopeUnit.h"
#include "Node.h"

struct ICmd;

struct Context {
    std::shared_ptr<std::unordered_map<std::string,std::string>> GlobalVariables;
    std::shared_ptr<std::unordered_map<std::string,std::vector<std::string>>> GlobalArrays;

    std::unordered_map<std::string,std::shared_ptr<ICmd>> Commands;
    std::unordered_map<std::string, FuncDeclarationNode> Funcs;
    ScopeUnit<std::string> Variables;
    ScopeUnit<std::vector<std::string>> Arrays;
    std::istream& InputStream;
    std::ostream& OutputStream;
    std::string CurrentPath;

    explicit Context(
        std::unordered_map<std::string,std::shared_ptr<ICmd>> Commands = {},
        std::unordered_map<std::string,std::string> Variables = {},
        std::unordered_map<std::string,std::vector<std::string>> Arrays = {},
        std::unordered_map<std::string, FuncDeclarationNode> Funcs = {
            { "test", FuncDeclarationNode("test", {VarNode("i"), VarNode("j")}, RootNode({
                Node{CmdNode("cp", {
                    Node{VarNode("i")},
                    Node{StringNode(" ")},
                    Node{VarNode("j")}})}}))
            }
        },
        std::istream& InputStream = std::cin,
        std::ostream& OutputStream = std::cout,
        std::string CurrentPath = std::filesystem::current_path().string()
    ) : GlobalVariables(std::make_shared<std::unordered_map<std::string,std::string>>(std::move(Variables))),
        GlobalArrays(std::make_shared<std::unordered_map<std::string,std::vector<std::string>>>(std::move(Arrays))),
        Commands(std::move(Commands)),
        Funcs(std::move(Funcs)),
        Variables(GlobalVariables),
        Arrays(GlobalArrays),
        InputStream(InputStream),
        OutputStream(OutputStream),
        CurrentPath(CurrentPath.empty()
                        ? std::filesystem::current_path().string()
                        : std::move(CurrentPath)) {}
};

#endif
