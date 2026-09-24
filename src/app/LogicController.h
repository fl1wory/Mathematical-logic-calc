#pragma once
#include <QObject>
#include <QString>

class LogicController : public QObject {
    Q_OBJECT

public:
    explicit LogicController(QObject *parent = nullptr);

    // Функції, які можна буде викликати прямо з кнопок в інтерфейсі (QML)
    Q_INVOKABLE QString solveStepByStep(const QString& formula);
    Q_INVOKABLE QString solveTruthTable(const QString& formula);
    Q_INVOKABLE QString solveQuine(const QString& formula);
};