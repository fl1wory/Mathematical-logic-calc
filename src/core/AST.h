#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <stdexcept>
#include <algorithm>

enum class NodeType { CONST, VAR, NOT, AND, OR, IMPLIES, EQUIV };

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual NodeType getType() const = 0;
    virtual bool evaluate(const std::map<std::string, bool>& vars) const = 0;
    virtual std::string toString() const = 0;
    virtual std::unique_ptr<ASTNode> clone() const = 0;
    virtual bool equals(const ASTNode* other) const = 0;
    virtual void getHeaders(std::vector<std::string>& headers) const = 0;
    virtual bool evaluateSteps(const std::map<std::string, bool>& vars, std::map<std::string, bool>& steps) const = 0;
};

// Вузол константи (0 або 1)
class ConstNode : public ASTNode {
    bool val;
public:
    ConstNode(bool v) : val(v) {}
    NodeType getType() const override { return NodeType::CONST; }
    bool evaluate(const std::map<std::string, bool>&) const override { return val; }
    std::string toString() const override { return val ? "1" : "0"; }
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<ConstNode>(val); }
    bool equals(const ASTNode* other) const override {
        if (!other || other->getType() != NodeType::CONST) return false;
        return val == static_cast<const ConstNode*>(other)->val;
    }
    void getHeaders(std::vector<std::string>&) const override {}
    bool evaluateSteps(const std::map<std::string, bool>&, std::map<std::string, bool>&) const override { return val; }
    bool getValue() const { return val; }
};

// Вузол змінної (A, B...)
class VarNode : public ASTNode {
    std::string name;
public:
    VarNode(std::string n) : name(std::move(n)) {}
    NodeType getType() const override { return NodeType::VAR; }
    bool evaluate(const std::map<std::string, bool>& vars) const override { return vars.at(name); }
    std::string toString() const override { return name; }
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<VarNode>(name); }
    bool equals(const ASTNode* other) const override {
        if (!other || other->getType() != NodeType::VAR) return false;
        return name == static_cast<const VarNode*>(other)->name;
    }
    void getHeaders(std::vector<std::string>&) const override {}
    bool evaluateSteps(const std::map<std::string, bool>& vars, std::map<std::string, bool>&) const override { return evaluate(vars); }
    std::string getName() const { return name; }
};

// Вузол заперечення (!)
class NotNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> operand;
    NotNode(std::unique_ptr<ASTNode> op) : operand(std::move(op)) {}
    NodeType getType() const override { return NodeType::NOT; }
    bool evaluate(const std::map<std::string, bool>& vars) const override { return !operand->evaluate(vars); }
    std::string toString() const override { return "(!" + operand->toString() + ")"; }
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<NotNode>(operand->clone()); }
    bool equals(const ASTNode* other) const override {
        if (!other || other->getType() != NodeType::NOT) return false;
        return operand->equals(static_cast<const NotNode*>(other)->operand.get());
    }
    void getHeaders(std::vector<std::string>& headers) const override {
        operand->getHeaders(headers);
        std::string myStr = toString();
        if (std::find(headers.begin(), headers.end(), myStr) == headers.end()) headers.push_back(myStr);
    }
    bool evaluateSteps(const std::map<std::string, bool>& vars, std::map<std::string, bool>& steps) const override {
        bool res = !operand->evaluateSteps(vars, steps);
        steps[toString()] = res;
        return res;
    }
};

// Базовий клас бінарних операцій
class BinaryNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    BinaryNode(std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r) : left(std::move(l)), right(std::move(r)) {}
    void getHeaders(std::vector<std::string>& headers) const override {
        left->getHeaders(headers);
        right->getHeaders(headers);
        std::string myStr = toString();
        if (std::find(headers.begin(), headers.end(), myStr) == headers.end()) headers.push_back(myStr);
    }
    bool equals(const ASTNode* other) const override {
        if (!other || other->getType() != getType()) return false;
        const auto* b = static_cast<const BinaryNode*>(other);
        return left->equals(b->left.get()) && right->equals(b->right.get());
    }
};

class AndNode : public BinaryNode {
public:
    using BinaryNode::BinaryNode;
    NodeType getType() const override { return NodeType::AND; }
    bool evaluate(const std::map<std::string, bool>& vars) const override { return left->evaluate(vars) && right->evaluate(vars); }
    std::string toString() const override { return "(" + left->toString() + " & " + right->toString() + ")"; }
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<AndNode>(left->clone(), right->clone()); }
    bool evaluateSteps(const std::map<std::string, bool>& vars, std::map<std::string, bool>& steps) const override {
        // ОБОВ'ЯЗКОВО обчислюємо обидві сторони перед оператором &&, щоб уникнути short-circuit
        bool l = left->evaluateSteps(vars, steps);
        bool r = right->evaluateSteps(vars, steps);
        bool res = l && r;
        steps[toString()] = res;
        return res;
    }
};

class OrNode : public BinaryNode {
public:
    using BinaryNode::BinaryNode;
    NodeType getType() const override { return NodeType::OR; }
    bool evaluate(const std::map<std::string, bool>& vars) const override { return left->evaluate(vars) || right->evaluate(vars); }
    std::string toString() const override { return "(" + left->toString() + " | " + right->toString() + ")"; }
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<OrNode>(left->clone(), right->clone()); }
    bool evaluateSteps(const std::map<std::string, bool>& vars, std::map<std::string, bool>& steps) const override {
        // ОБОВ'ЯЗКОВО обчислюємо обидві сторони перед оператором ||
        bool l = left->evaluateSteps(vars, steps);
        bool r = right->evaluateSteps(vars, steps);
        bool res = l || r;
        steps[toString()] = res;
        return res;
    }
};

class ImpliesNode : public BinaryNode {
public:
    using BinaryNode::BinaryNode;
    NodeType getType() const override { return NodeType::IMPLIES; }
    bool evaluate(const std::map<std::string, bool>& vars) const override { return !left->evaluate(vars) || right->evaluate(vars); }
    std::string toString() const override { return "(" + left->toString() + " -> " + right->toString() + ")"; }
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<ImpliesNode>(left->clone(), right->clone()); }
    bool evaluateSteps(const std::map<std::string, bool>& vars, std::map<std::string, bool>& steps) const override {
        // ОБОВ'ЯЗКОВО обчислюємо обидві сторони
        bool l = left->evaluateSteps(vars, steps);
        bool r = right->evaluateSteps(vars, steps);
        bool res = (!l) || r;
        steps[toString()] = res;
        return res;
    }
};

class EquivNode : public BinaryNode {
public:
    using BinaryNode::BinaryNode;
    NodeType getType() const override { return NodeType::EQUIV; }
    bool evaluate(const std::map<std::string, bool>& vars) const override { return left->evaluate(vars) == right->evaluate(vars); }
    std::string toString() const override { return "(" + left->toString() + " <-> " + right->toString() + ")"; }
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<EquivNode>(left->clone(), right->clone()); }
    bool evaluateSteps(const std::map<std::string, bool>& vars, std::map<std::string, bool>& steps) const override {
        bool l = left->evaluateSteps(vars, steps);
        bool r = right->evaluateSteps(vars, steps);
        bool res = (l == r);
        steps[toString()] = res;
        return res;
    }
};