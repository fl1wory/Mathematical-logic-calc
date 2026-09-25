import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 440
    height: 780
    minimumWidth: 360
    minimumHeight: 600
    title: "Калькулятор логіки"

    // Режим клавіатури: false = "Часті", true = "Повний алфавіт A-Z"
    property bool fullAlphabetMode: false

    // Масиви змінних
    readonly property var frequentVars: ["A", "B", "C", "D", "P", "Q", "R", "N", "M"]
    readonly property var allVars: [
        "A", "B", "C", "D", "E", "F", "G",
        "H", "I", "J", "K", "L", "M", "N",
        "O", "P", "Q", "R", "S", "T", "U",
        "V", "W", "X", "Y", "Z"
    ]

    // Допоміжні функції для роботи з полем вводу
    function insertSymbol(sym) {
        var pos = formulaInput.cursorPosition;
        formulaInput.insert(pos, sym);
        formulaInput.cursorPosition = pos + sym.length;
        formulaInput.forceActiveFocus();
    }

    function calculate() {
        if (formulaInput.text.trim() === "") return;
        if (tabBar.currentIndex === 0) {
            resultArea.text = logicController.solveStepByStep(formulaInput.text);
        } else if (tabBar.currentIndex === 1) {
            resultArea.text = logicController.solveTruthTable(formulaInput.text);
        } else {
            resultArea.text = logicController.solveQuine(formulaInput.text);
        }
    }

    function deleteChar() {
        var pos = formulaInput.cursorPosition;
        if (pos > 0) {
            formulaInput.remove(pos - 1, pos);
            formulaInput.cursorPosition = pos - 1;
        }
        formulaInput.forceActiveFocus();
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        // --- ВЕРХНЯ ЧАСТИНА: Поле введення формули ---
        TextField {
            id: formulaInput
            Layout.fillWidth: true
            Layout.preferredHeight: 52
            font.pixelSize: 20
            font.bold: true
            placeholderText: "Введіть вираз..."
            selectByMouse: true
        }

        // --- СЕРЕДНЯ ЧАСТИНА: Калькуляторна клавіатура ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: keyboardLayout.implicitHeight + 16
            color: "#eef2f5"
            radius: 8
            border.color: "#d0d7de"

            ColumnLayout {
                id: keyboardLayout
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                // 1. Панель операторів (ЗАВЖДИ ВИДИМА)
                GridLayout {
                    columns: 5
                    Layout.fillWidth: true
                    rowSpacing: 4
                    columnSpacing: 4

                    Button {
                        text: "¬ (!)"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        font.bold: true
                        onClicked: insertSymbol("!")
                    }
                    Button {
                        text: "∧ (&&)"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        font.bold: true
                        onClicked: insertSymbol(" & ")
                    }
                    Button {
                        text: "∨ (|)"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        font.bold: true
                        onClicked: insertSymbol(" | ")
                    }
                    Button {
                        text: "→ (->)"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        font.bold: true
                        onClicked: insertSymbol(" -> ")
                    }
                    Button {
                        text: "↔ (<->)"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        font.bold: true
                        onClicked: insertSymbol(" <-> ")
                    }

                    Button {
                        text: "("
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        font.bold: true
                        onClicked: insertSymbol("(")
                    }
                    Button {
                        text: ")"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        font.bold: true
                        onClicked: insertSymbol(")")
                    }
                    Button {
                        text: "⌫"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        font.bold: true
                        onClicked: deleteChar()
                    }
                    Button {
                        text: "C"
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        font.bold: true
                        palette.buttonText: "#d32f2f"
                        onClicked: formulaInput.clear()
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#d0d7de"
                }

                // 2. Панель літер (Змінних)
                // РЕЖИМ 1: Часті змінні (A, B, C, D, P, Q, R, N, M)
                GridLayout {
                    columns: 5
                    Layout.fillWidth: true
                    rowSpacing: 4
                    columnSpacing: 4
                    visible: !root.fullAlphabetMode

                    Repeater {
                        model: root.frequentVars
                        Button {
                            text: modelData
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            font.pixelSize: 17
                            font.bold: true
                            onClicked: insertSymbol(modelData)
                        }
                    }

                    // Кнопка перемикання на повний алфавіт
                    Button {
                        text: "A-Z ▾"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 44
                        highlighted: true
                        font.bold: true
                        onClicked: root.fullAlphabetMode = true
                    }
                }

                // РЕЖИМ 2: Повний алфавіт (A - Z)
                GridLayout {
                    columns: 7
                    Layout.fillWidth: true
                    rowSpacing: 4
                    columnSpacing: 3
                    visible: root.fullAlphabetMode

                    Repeater {
                        model: root.allVars
                        Button {
                            text: modelData
                            Layout.fillWidth: true
                            Layout.preferredHeight: 38
                            font.pixelSize: 15
                            font.bold: true
                            onClicked: insertSymbol(modelData)
                        }
                    }

                    // Кнопка повернення до частих змінних
                    Button {
                        text: "Часті ▴"
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        highlighted: true
                        font.bold: true
                        onClicked: root.fullAlphabetMode = false
                    }
                }
            }
        }

        // --- Кнопка дії та вкладки ---
        Button {
            id: computeBtn
            text: "Обчислити"
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            highlighted: true
            font.bold: true
            font.pixelSize: 16

            onClicked: calculate()
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            TabButton { text: "Покроково" }
            TabButton { text: "Таблиця" }
            TabButton { text: "Куайн-М." }

            // Автоматично перераховувати при зміні вкладки:
            onCurrentIndexChanged: calculate()
        }

        // --- НИЖНЯ ЧАСТИНА: Результати зі скролом ---
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            TextArea {
                id: resultArea
                readOnly: true
                font.family: "Monospace"
                font.pixelSize: 13
                selectByMouse: true
                wrapMode: TextEdit.NoWrap
                background: Rectangle {
                    color: "#ffffff"
                    border.color: "#d0d7de"
                    radius: 6
                }
            }
        }
    }
}