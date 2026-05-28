#ifndef SHELLANG_NODE_H
#define SHELLANG_NODE_H

#include "Token.h"

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <optional>

struct Node;

struct RootNode {
    std::vector<Node> nodes;

    bool operator==(const RootNode&) const;
};

struct CmdNode {
    std::string cmd;
    std::vector<Node> args;

    bool operator==(const CmdNode&) const;
};

struct StringNode {
    std::string str;

    bool operator==(const StringNode&) const;
};

struct VarNode {
    std::string var;

    bool operator==(const VarNode&) const;
};

struct ArgNode {
    std::string arg;

    bool operator==(const ArgNode&) const;
};

struct RedirectNode {
    std::shared_ptr<Node> SideLeft, SideRight;
    Sign sign;

    bool operator==(const RedirectNode&) const;
};

struct AlgebraicNode {
    std::vector<std::variant<Token,Node>> tns;

    bool operator==(const AlgebraicNode&) const;
};

struct ArrNode {
    std::string arr;
    AlgebraicNode idx;

    bool operator==(const ArrNode&) const;
};

struct IfNode {
    AlgebraicNode condition;
    RootNode ifBody;
    RootNode elseBody;

    bool operator==(const IfNode&) const;
};

struct WhileNode {
    AlgebraicNode condition;
    RootNode body;

    bool operator==(const WhileNode&) const;
};

struct ForNode {
    std::optional<RedirectNode> declaration, iteration;
    AlgebraicNode condition;
    RootNode body;

    bool operator==(const ForNode&) const;
};

using NodeValue = std::variant <
    CmdNode,
    StringNode,
    VarNode,
    ArrNode,
    ArgNode,
    RedirectNode,
    AlgebraicNode,
    IfNode,
    WhileNode,
    ForNode
>;

struct Node {
    NodeValue value;
    size_t x, y;

    bool operator==(const Node&) const;
};

inline bool RootNode::operator==(const RootNode&) const = default;
inline bool CmdNode::operator==(const CmdNode&) const = default;
inline bool StringNode::operator==(const StringNode&) const = default;
inline bool VarNode::operator==(const VarNode&) const = default;
inline bool ArgNode::operator==(const ArgNode&) const = default;
inline bool AlgebraicNode::operator==(const AlgebraicNode&) const = default;
inline bool ArrNode::operator==(const ArrNode&) const = default;
inline bool IfNode::operator==(const IfNode&) const = default;
inline bool WhileNode::operator==(const WhileNode&) const = default;
inline bool ForNode::operator==(const ForNode&) const = default;

inline bool Node::operator==(const Node& o) const { return value == o.value; }

inline bool RedirectNode::operator==(const RedirectNode& o) const {
    const auto eq = [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
        return (!a && !b) || (a && b && *a == *b);
    };

    return sign == o.sign && eq(SideLeft, o.SideLeft) && eq(SideRight, o.SideRight);
}

#endif