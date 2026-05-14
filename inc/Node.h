#ifndef SHELLANG_NODE_H
#define SHELLANG_NODE_H

#include "Token.h"

#include <variant>
#include <string>
#include <vector>
#include <memory>

struct Node;

struct RootNode {
    std::vector<Node> nodes;
};

struct CmdNode {
    std::string cmd;
    std::vector<Node> args;
};

struct VarNode {
    std::string var;
};

struct ArgNode {
    std::string arg;
};

struct RedirectNode {
    std::shared_ptr<Node> SideLeft, SideRight;
    Sign sign;
};

using NodeValue = std::variant <
    CmdNode,
    VarNode,
    ArgNode,
    RedirectNode
>;

struct Node {
    NodeValue value;
    size_t x, y;
};

#endif