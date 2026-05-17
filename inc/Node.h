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

struct AlgebraicNode {
    std::vector<Token> tokens;
};

struct IfNode {
    AlgebraicNode condition;
    RootNode ifBody;
    RootNode elseBody;
};

struct WhileNode {
    AlgebraicNode condition;
    RootNode body;
};

using NodeValue = std::variant <
    CmdNode,
    VarNode,
    ArgNode,
    RedirectNode,
    AlgebraicNode,
    IfNode,
    WhileNode
>;

struct Node {
    NodeValue value;
    size_t x, y;
};

#endif