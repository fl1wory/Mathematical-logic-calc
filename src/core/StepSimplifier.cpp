#include "StepSimplifier.h"

bool isNegationOf(const ASTNode* a, const ASTNode* b) {
    if (!a || !b) return false;
    if (a->getType() == NodeType::NOT) {
        return static_cast<const NotNode*>(a)->operand->equals(b);
    }
    if (b->getType() == NodeType::NOT) {
        return static_cast<const NotNode*>(b)->operand->equals(a);
    }
    return false;
}

// Збирає всі множники всередині ланцюжка AND
void collectAndFactors(const ASTNode* node, std::vector<const ASTNode*>& factors) {
    if (!node) return;
    if (node->getType() == NodeType::AND) {
        const auto* a = static_cast<const AndNode*>(node);
        collectAndFactors(a->left.get(), factors);
        collectAndFactors(a->right.get(), factors);
    } else {
        factors.push_back(node);
    }
}

// Збирає всі доданки всередині ланцюжка OR
void collectOrTerms(const ASTNode* node, std::vector<const ASTNode*>& terms) {
    if (!node) return;
    if (node->getType() == NodeType::OR) {
        const auto* o = static_cast<const OrNode*>(node);
        collectOrTerms(o->left.get(), terms);
        collectOrTerms(o->right.get(), terms);
    } else {
        terms.push_back(node);
    }
}

// Перевіряє суперечність у добутку: (A & !A & ...) = 0
bool hasNestedContradiction(const ASTNode* node) {
    if (!node || node->getType() != NodeType::AND) return false;
    std::vector<const ASTNode*> factors;
    collectAndFactors(node, factors);
    for (size_t i = 0; i < factors.size(); ++i) {
        for (size_t j = i + 1; j < factors.size(); ++j) {
            if (isNegationOf(factors[i], factors[j])) return true;
        }
    }
    return false;
}

// Перевіряє закон виключеного третього в сумі: (A | !A | ...) = 1
bool hasNestedExcludedMiddle(const ASTNode* node) {
    if (!node || node->getType() != NodeType::OR) return false;
    std::vector<const ASTNode*> terms;
    collectOrTerms(node, terms);
    for (size_t i = 0; i < terms.size(); ++i) {
        for (size_t j = i + 1; j < terms.size(); ++j) {
            if (isNegationOf(terms[i], terms[j])) return true;
        }
    }
    return false;
}

std::unique_ptr<ASTNode> tryAbsorbWithNegation(const ASTNode* simple, const ASTNode* andNode) {
    if (!simple || !andNode || andNode->getType() != NodeType::AND) return nullptr;
    const auto* a = static_cast<const AndNode*>(andNode);
    if (isNegationOf(simple, a->left.get())) return a->right->clone();
    if (isNegationOf(simple, a->right.get())) return a->left->clone();
    return nullptr;
}

std::unique_ptr<ASTNode> tryGlue(const ASTNode* n1, const ASTNode* n2) {
    if (!n1 || !n2 || n1->getType() != NodeType::AND || n2->getType() != NodeType::AND) return nullptr;
    const auto* a = static_cast<const AndNode*>(n1);
    const auto* b = static_cast<const AndNode*>(n2);

    if (a->left->equals(b->left.get()) && isNegationOf(a->right.get(), b->right.get())) return a->left->clone();
    if (a->left->equals(b->right.get()) && isNegationOf(a->right.get(), b->left.get())) return a->left->clone();
    if (a->right->equals(b->left.get()) && isNegationOf(a->left.get(), b->right.get())) return a->right->clone();
    if (a->right->equals(b->right.get()) && isNegationOf(a->left.get(), b->left.get())) return a->right->clone();
    return nullptr;
}

bool StepSimplifier::applyOneRule(std::unique_ptr<ASTNode>& node, TransformStep& step) {
    if (!node) return false;

    // --- 1. Усунення еквівалентності ---
    if (node->getType() == NodeType::EQUIV) {
        auto* eq = static_cast<EquivNode*>(node.get());
        step.ruleName = "Усунення еквівалентності: (A <-> B) = (!A | B) & (!B | A)";
        step.formulaBefore = node->toString();

        auto notL = std::make_unique<NotNode>(eq->left->clone());
        auto leftPart = std::make_unique<OrNode>(std::move(notL), eq->right->clone());

        auto notR = std::make_unique<NotNode>(eq->right->clone());
        auto rightPart = std::make_unique<OrNode>(std::move(notR), eq->left->clone());

        node = std::make_unique<AndNode>(std::move(leftPart), std::move(rightPart));
        step.formulaAfter = node->toString();
        return true;
    }

    // --- 2. Усунення імплікації ---
    if (node->getType() == NodeType::IMPLIES) {
        auto* imp = static_cast<ImpliesNode*>(node.get());
        step.ruleName = "Усунення імплікації: (A -> B) = (!A | B)";
        step.formulaBefore = node->toString();

        auto notLeft = std::make_unique<NotNode>(imp->left->clone());
        node = std::make_unique<OrNode>(std::move(notLeft), imp->right->clone());
        step.formulaAfter = node->toString();
        return true;
    }

    // --- 3. Подвійне заперечення та де Морган ---
    if (node->getType() == NodeType::NOT) {
        auto* notNode = static_cast<NotNode*>(node.get());
        if (notNode->operand->getType() == NodeType::NOT) {
            auto* innerNot = static_cast<NotNode*>(notNode->operand.get());
            step.ruleName = "Закон подвійного заперечення: !(!A) = A";
            step.formulaBefore = node->toString();
            node = innerNot->operand->clone();
            step.formulaAfter = node->toString();
            return true;
        }

        if (notNode->operand->getType() == NodeType::AND) {
            auto* andNode = static_cast<AndNode*>(notNode->operand.get());
            step.ruleName = "Закон де Моргана: !(A & B) = !A | !B";
            step.formulaBefore = node->toString();
            auto n1 = std::make_unique<NotNode>(andNode->left->clone());
            auto n2 = std::make_unique<NotNode>(andNode->right->clone());
            node = std::make_unique<OrNode>(std::move(n1), std::move(n2));
            step.formulaAfter = node->toString();
            return true;
        }

        if (notNode->operand->getType() == NodeType::OR) {
            auto* orNode = static_cast<OrNode*>(notNode->operand.get());
            step.ruleName = "Закон де Моргана: !(A | B) = !A & !B";
            step.formulaBefore = node->toString();
            auto n1 = std::make_unique<NotNode>(orNode->left->clone());
            auto n2 = std::make_unique<NotNode>(orNode->right->clone());
            node = std::make_unique<AndNode>(std::move(n1), std::move(n2));
            step.formulaAfter = node->toString();
            return true;
        }
    }

    // --- 4. Закони для кон'юнкції (AND) ---
    if (node->getType() == NodeType::AND) {
        auto* aNode = static_cast<AndNode*>(node.get());

        // Суперечність у добутку: (A & !A & ...) = 0
        if (hasNestedContradiction(node.get())) {
            step.ruleName = "Закон суперечності: (A & !A & ...) = 0";
            step.formulaBefore = node->toString();
            node = std::make_unique<ConstNode>(false);
            step.formulaAfter = node->toString();
            return true;
        }

        // A & A === A
        if (aNode->left->equals(aNode->right.get())) {
            step.ruleName = "Закон ідемпотентності: A & A = A";
            step.formulaBefore = node->toString();
            node = aNode->left->clone();
            step.formulaAfter = node->toString();
            return true;
        }

        // Константи в AND
        if (aNode->left->getType() == NodeType::CONST) {
            bool v = static_cast<ConstNode*>(aNode->left.get())->getValue();
            step.ruleName = v ? "Властивість константи: 1 & A = A" : "Властивість константи: 0 & A = 0";
            step.formulaBefore = node->toString();
            node = v ? aNode->right->clone() : std::make_unique<ConstNode>(false);
            step.formulaAfter = node->toString();
            return true;
        }
        if (aNode->right->getType() == NodeType::CONST) {
            bool v = static_cast<ConstNode*>(aNode->right.get())->getValue();
            step.ruleName = v ? "Властивість константи: A & 1 = A" : "Властивість константи: A & 0 = 0";
            step.formulaBefore = node->toString();
            node = v ? aNode->left->clone() : std::make_unique<ConstNode>(false);
            step.formulaAfter = node->toString();
            return true;
        }

        // Поглинання: A & (A | B) === A
        if (aNode->right->getType() == NodeType::OR) {
            auto* rOr = static_cast<OrNode*>(aNode->right.get());
            if (aNode->left->equals(rOr->left.get()) || aNode->left->equals(rOr->right.get())) {
                step.ruleName = "Закон поглинання: A & (A | B) = A";
                step.formulaBefore = node->toString();
                node = aNode->left->clone();
                step.formulaAfter = node->toString();
                return true;
            }
        }

        // Дистрибутивне розкриття протилежних: (A | B) & (!A | C) === (A & C) | (!A & B)
        if (aNode->left->getType() == NodeType::OR && aNode->right->getType() == NodeType::OR) {
            auto* lOr = static_cast<OrNode*>(aNode->left.get());
            auto* rOr = static_cast<OrNode*>(aNode->right.get());

            struct Match { const ASTNode* A; const ASTNode* notA; const ASTNode* B; const ASTNode* C; };
            std::vector<Match> checks = {
                {lOr->left.get(), rOr->left.get(), lOr->right.get(), rOr->right.get()},
                {lOr->left.get(), rOr->right.get(), lOr->right.get(), rOr->left.get()},
                {lOr->right.get(), rOr->left.get(), lOr->left.get(), rOr->right.get()},
                {lOr->right.get(), rOr->right.get(), lOr->left.get(), rOr->left.get()}
            };

            for (const auto& m : checks) {
                if (isNegationOf(m.A, m.notA)) {
                    step.ruleName = "Дистрибутивне розкриття протилежних: (A | B) & (!A | C) = (A & C) | (!A & B)";
                    step.formulaBefore = node->toString();
                    auto p1 = std::make_unique<AndNode>(m.A->clone(), m.C->clone());
                    auto p2 = std::make_unique<AndNode>(m.notA->clone(), m.B->clone());
                    node = std::make_unique<OrNode>(std::move(p1), std::move(p2));
                    step.formulaAfter = node->toString();
                    return true;
                }
            }
        }
    }

    // --- 5. Закони для диз'юнкції (OR) ---
    if (node->getType() == NodeType::OR) {
        auto* oNode = static_cast<OrNode*>(node.get());

        // Закон виключеного третього у вкладених дужках: (... | R | ... | !R | ...) === 1
        if (hasNestedExcludedMiddle(node.get())) {
            step.ruleName = "Закон виключеного третього: (A | !A | ...) = 1";
            step.formulaBefore = node->toString();
            node = std::make_unique<ConstNode>(true);
            step.formulaAfter = node->toString();
            return true;
        }

        // A | A === A
        if (oNode->left->equals(oNode->right.get())) {
            step.ruleName = "Закон ідемпотентності: A | A = A";
            step.formulaBefore = node->toString();
            node = oNode->left->clone();
            step.formulaAfter = node->toString();
            return true;
        }

        // Константи в OR
        if (oNode->left->getType() == NodeType::CONST) {
            bool v = static_cast<ConstNode*>(oNode->left.get())->getValue();
            step.ruleName = v ? "Властивість константи: 1 | A = 1" : "Властивість константи: 0 | A = A";
            step.formulaBefore = node->toString();
            node = v ? std::make_unique<ConstNode>(true) : oNode->right->clone();
            step.formulaAfter = node->toString();
            return true;
        }
        if (oNode->right->getType() == NodeType::CONST) {
            bool v = static_cast<ConstNode*>(oNode->right.get())->getValue();
            step.ruleName = v ? "Властивість константи: A | 1 = 1" : "Властивість константи: A | 0 = A";
            step.formulaBefore = node->toString();
            node = v ? std::make_unique<ConstNode>(true) : oNode->left->clone();
            step.formulaAfter = node->toString();
            return true;
        }

        // Просте поглинання з запереченням
        if (auto res = tryAbsorbWithNegation(oNode->left.get(), oNode->right.get())) {
            step.ruleName = "Поглинання з запереченням: A | (!A & B) = A | B";
            step.formulaBefore = node->toString();
            node = std::make_unique<OrNode>(oNode->left->clone(), std::move(res));
            step.formulaAfter = node->toString();
            return true;
        }
        if (auto res = tryAbsorbWithNegation(oNode->right.get(), oNode->left.get())) {
            step.ruleName = "Поглинання з запереченням: (!A & B) | A = B | A";
            step.formulaBefore = node->toString();
            node = std::make_unique<OrNode>(oNode->right->clone(), std::move(res));
            step.formulaAfter = node->toString();
            return true;
        }

        // Вкладене поглинання праворуч: A | ((!A & B) | C) => (A | B) | C
        if (oNode->right->getType() == NodeType::OR) {
            auto* rOr = static_cast<OrNode*>(oNode->right.get());
            if (auto res = tryAbsorbWithNegation(oNode->left.get(), rOr->left.get())) {
                step.ruleName = "Поглинання з запереченням: A | ((!A & B) | C) = (A | B) | C";
                step.formulaBefore = node->toString();
                auto inner = std::make_unique<OrNode>(oNode->left->clone(), std::move(res));
                node = std::make_unique<OrNode>(std::move(inner), rOr->right->clone());
                step.formulaAfter = node->toString();
                return true;
            }
            if (auto res = tryAbsorbWithNegation(oNode->left.get(), rOr->right.get())) {
                step.ruleName = "Поглинання з запереченням: A | (C | (!A & B)) = (A | B) | C";
                step.formulaBefore = node->toString();
                auto inner = std::make_unique<OrNode>(oNode->left->clone(), std::move(res));
                node = std::make_unique<OrNode>(std::move(inner), rOr->left->clone());
                step.formulaAfter = node->toString();
                return true;
            }
        }

        // Вкладене поглинання ліворуч: (A | B) | (!A & C) => B | (A | C)
        if (oNode->left->getType() == NodeType::OR && oNode->right->getType() == NodeType::AND) {
            auto* lOr = static_cast<OrNode*>(oNode->left.get());
            if (auto res = tryAbsorbWithNegation(lOr->left.get(), oNode->right.get())) {
                step.ruleName = "Поглинання з запереченням: (A | B) | (!A & C) = B | (A | C)";
                step.formulaBefore = node->toString();
                auto inner = std::make_unique<OrNode>(lOr->left->clone(), std::move(res));
                node = std::make_unique<OrNode>(lOr->right->clone(), std::move(inner));
                step.formulaAfter = node->toString();
                return true;
            }
            if (auto res = tryAbsorbWithNegation(lOr->right.get(), oNode->right.get())) {
                step.ruleName = "Поглинання з запереченням: (A | B) | (!B & C) = A | (B | C)";
                step.formulaBefore = node->toString();
                auto inner = std::make_unique<OrNode>(lOr->right->clone(), std::move(res));
                node = std::make_unique<OrNode>(lOr->left->clone(), std::move(inner));
                step.formulaAfter = node->toString();
                return true;
            }
        }

        // Поглинання між AND та OR
        if (oNode->left->getType() == NodeType::AND && oNode->right->getType() == NodeType::OR) {
            auto* lAnd = static_cast<AndNode*>(oNode->left.get());
            auto* rOr = static_cast<OrNode*>(oNode->right.get());
            const auto* a = lAnd->left.get();
            const auto* b = lAnd->right.get();
            const auto* c = rOr->left.get();
            const auto* d = rOr->right.get();
            if (a->equals(c) || a->equals(d) || b->equals(c) || b->equals(d)) {
                step.ruleName = "Закон поглинання: (A & B) | (A | C) = A | C";
                step.formulaBefore = node->toString();
                node = oNode->right->clone();
                step.formulaAfter = node->toString();
                return true;
            }
        }
        if (oNode->left->getType() == NodeType::OR && oNode->right->getType() == NodeType::AND) {
            auto* lOr = static_cast<OrNode*>(oNode->left.get());
            auto* rAnd = static_cast<AndNode*>(oNode->right.get());
            const auto* c = lOr->left.get();
            const auto* d = lOr->right.get();
            const auto* a = rAnd->left.get();
            const auto* b = rAnd->right.get();
            if (a->equals(c) || a->equals(d) || b->equals(c) || b->equals(d)) {
                step.ruleName = "Закон поглинання: (A | C) | (A & B) = A | C";
                step.formulaBefore = node->toString();
                node = oNode->left->clone();
                step.formulaAfter = node->toString();
                return true;
            }
        }

        // Пряме склеювання
        if (oNode->left->getType() == NodeType::AND && oNode->right->getType() == NodeType::AND) {
            if (auto g = tryGlue(oNode->left.get(), oNode->right.get())) {
                step.ruleName = "Закон склеювання: (A & B) | (A & !B) = A";
                step.formulaBefore = node->toString();
                node = std::move(g);
                step.formulaAfter = node->toString();
                return true;
            }
        }

        // Асоціативне склеювання
        if (oNode->left->getType() == NodeType::OR && oNode->right->getType() == NodeType::OR) {
            auto* lOr = static_cast<OrNode*>(oNode->left.get());
            auto* rOr = static_cast<OrNode*>(oNode->right.get());

            const std::pair<const ASTNode*, const ASTNode*> candidates[] = {
                {lOr->left.get(), rOr->left.get()},
                {lOr->left.get(), rOr->right.get()},
                {lOr->right.get(), rOr->left.get()},
                {lOr->right.get(), rOr->right.get()}
            };

            for (const auto& pair : candidates) {
                if (auto g = tryGlue(pair.first, pair.second)) {
                    step.ruleName = "Асоціативне склеювання: (A & B) | (A & !B) = A";
                    step.formulaBefore = node->toString();

                    auto rem1 = (pair.first == lOr->left.get()) ? lOr->right->clone() : lOr->left->clone();
                    auto rem2 = (pair.second == rOr->left.get()) ? rOr->right->clone() : rOr->left->clone();

                    auto innerOr = std::make_unique<OrNode>(std::move(rem1), std::move(rem2));
                    node = std::make_unique<OrNode>(std::move(g), std::move(innerOr));
                    step.formulaAfter = node->toString();
                    return true;
                }
            }
        }
    }

    // Рекурсія
    if (node->getType() == NodeType::NOT) {
        return applyOneRule(static_cast<NotNode*>(node.get())->operand, step);
    }
    if (auto* bin = dynamic_cast<BinaryNode*>(node.get())) {
        if (applyOneRule(bin->left, step)) return true;
        return applyOneRule(bin->right, step);
    }

    return false;
}

std::vector<TransformStep> StepSimplifier::simplify(std::unique_ptr<ASTNode>& root) {
    std::vector<TransformStep> history;
    TransformStep step;

    while (applyOneRule(root, step)) {
        step.fullExpression = root->toString();
        history.push_back(step);
    }

    return history;
}