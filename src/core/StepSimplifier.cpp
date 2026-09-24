#include "StepSimplifier.h"

// Перевірка, чи є один вузол запереченням іншого: A та !A
bool isNegationOf(const ASTNode* a, const ASTNode* b) {
    if (a->getType() == NodeType::NOT) {
        return static_cast<const NotNode*>(a)->operand->equals(b);
    }
    if (b->getType() == NodeType::NOT) {
        return static_cast<const NotNode*>(b)->operand->equals(a);
    }
    return false;
}

bool StepSimplifier::applyOneRule(std::unique_ptr<ASTNode>& node, TransformStep& step) {
    if (!node) return false;

    // --- 1. Усунення еквівалентності: A <-> B === (!A | B) & (!B | A) ---
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

    // --- 2. Усунення імплікації: A -> B === !A | B ---
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

        // A & A === A
        if (aNode->left->equals(aNode->right.get())) {
            step.ruleName = "Закон ідемпотентності: A & A = A";
            step.formulaBefore = node->toString();
            node = aNode->left->clone();
            step.formulaAfter = node->toString();
            return true;
        }

        // A & !A === 0
        if (isNegationOf(aNode->left.get(), aNode->right.get())) {
            step.ruleName = "Закон суперечності: A & !A = 0";
            step.formulaBefore = node->toString();
            node = std::make_unique<ConstNode>(false);
            step.formulaAfter = node->toString();
            return true;
        }

        // A & 1 === A; A & 0 === 0
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
            // Поглинання з запереченням: A & (!A | B) === A & B
            if (isNegationOf(aNode->left.get(), rOr->left.get())) {
                step.ruleName = "Поглинання з запереченням: A & (!A | B) = A & B";
                step.formulaBefore = node->toString();
                node = std::make_unique<AndNode>(aNode->left->clone(), rOr->right->clone());
                step.formulaAfter = node->toString();
                return true;
            }
            if (isNegationOf(aNode->left.get(), rOr->right.get())) {
                step.ruleName = "Поглинання з запереченням: A & (B | !A) = A & B";
                step.formulaBefore = node->toString();
                node = std::make_unique<AndNode>(aNode->left->clone(), rOr->left->clone());
                step.formulaAfter = node->toString();
                return true;
            }
        }

        // Двоїстий дистрибутивний закон: (X | Y) & (X | Z) === X | (Y & Z)
        if (aNode->left->getType() == NodeType::OR && aNode->right->getType() == NodeType::OR) {
            auto* lOr = static_cast<OrNode*>(aNode->left.get());
            auto* rOr = static_cast<OrNode*>(aNode->right.get());

            const ASTNode* common = nullptr;
            const ASTNode* diffL = nullptr;
            const ASTNode* diffR = nullptr;

            if (lOr->left->equals(rOr->left.get())) {
                common = lOr->left.get(); diffL = lOr->right.get(); diffR = rOr->right.get();
            } else if (lOr->left->equals(rOr->right.get())) {
                common = lOr->left.get(); diffL = lOr->right.get(); diffR = rOr->left.get();
            } else if (lOr->right->equals(rOr->left.get())) {
                common = lOr->right.get(); diffL = lOr->left.get(); diffR = rOr->right.get();
            } else if (lOr->right->equals(rOr->right.get())) {
                common = lOr->right.get(); diffL = lOr->left.get(); diffR = rOr->left.get();
            }

            if (common) {
                step.ruleName = "Дистрибутивний закон: (X | Y) & (X | Z) = X | (Y & Z)";
                step.formulaBefore = node->toString();
                auto andDiff = std::make_unique<AndNode>(diffL->clone(), diffR->clone());
                node = std::make_unique<OrNode>(common->clone(), std::move(andDiff));
                step.formulaAfter = node->toString();
                return true;
            }
        }
    }

    // --- 5. Закони для диз'юнкції (OR) ---
    if (node->getType() == NodeType::OR) {
        auto* oNode = static_cast<OrNode*>(node.get());

        // A | A === A
        if (oNode->left->equals(oNode->right.get())) {
            step.ruleName = "Закон ідемпотентності: A | A = A";
            step.formulaBefore = node->toString();
            node = oNode->left->clone();
            step.formulaAfter = node->toString();
            return true;
        }

        // A | !A === 1
        if (isNegationOf(oNode->left.get(), oNode->right.get())) {
            step.ruleName = "Закон виключеного третього: A | !A = 1";
            step.formulaBefore = node->toString();
            node = std::make_unique<ConstNode>(true);
            step.formulaAfter = node->toString();
            return true;
        }

        // A | 0 === A; A | 1 === 1
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

        // Поглинання: A | (A & B) === A
        if (oNode->right->getType() == NodeType::AND) {
            auto* rAnd = static_cast<AndNode*>(oNode->right.get());
            if (oNode->left->equals(rAnd->left.get()) || oNode->left->equals(rAnd->right.get())) {
                step.ruleName = "Закон поглинання: A | (A & B) = A";
                step.formulaBefore = node->toString();
                node = oNode->left->clone();
                step.formulaAfter = node->toString();
                return true;
            }
            // Поглинання з запереченням: A | (!A & B) === A | B
            if (isNegationOf(oNode->left.get(), rAnd->left.get())) {
                step.ruleName = "Поглинання з запереченням: A | (!A & B) = A | B";
                step.formulaBefore = node->toString();
                node = std::make_unique<OrNode>(oNode->left->clone(), rAnd->right->clone());
                step.formulaAfter = node->toString();
                return true;
            }
            if (isNegationOf(oNode->left.get(), rAnd->right.get())) {
                step.ruleName = "Поглинання з запереченням: A | (B & !A) = A | B";
                step.formulaBefore = node->toString();
                node = std::make_unique<OrNode>(oNode->left->clone(), rAnd->left->clone());
                step.formulaAfter = node->toString();
                return true;
            }
        }

        // ДИСТРИБУТИВНИЙ ЗАКОН (Винесення спільного за дужки): (X & Y) | (X & Z) === X & (Y | Z)
        if (oNode->left->getType() == NodeType::AND && oNode->right->getType() == NodeType::AND) {
            auto* lAnd = static_cast<AndNode*>(oNode->left.get());
            auto* rAnd = static_cast<AndNode*>(oNode->right.get());

            const ASTNode* common = nullptr;
            const ASTNode* diffL = nullptr;
            const ASTNode* diffR = nullptr;

            if (lAnd->left->equals(rAnd->left.get())) {
                common = lAnd->left.get(); diffL = lAnd->right.get(); diffR = rAnd->right.get();
            } else if (lAnd->left->equals(rAnd->right.get())) {
                common = lAnd->left.get(); diffL = lAnd->right.get(); diffR = rAnd->left.get();
            } else if (lAnd->right->equals(rAnd->left.get())) {
                common = lAnd->right.get(); diffL = lAnd->left.get(); diffR = rAnd->right.get();
            } else if (lAnd->right->equals(rAnd->right.get())) {
                common = lAnd->right.get(); diffL = lAnd->left.get(); diffR = rAnd->left.get();
            }

            if (common) {
                step.ruleName = "Дистрибутивний закон (винесення за дужки): (X & Y) | (X & Z) = X & (Y | Z)";
                step.formulaBefore = node->toString();
                auto orDiff = std::make_unique<OrNode>(diffL->clone(), diffR->clone());
                node = std::make_unique<AndNode>(common->clone(), std::move(orDiff));
                step.formulaAfter = node->toString();
                return true;
            }
        }
    }

    // Рекурсивно перевіряємо дочірні вузли
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