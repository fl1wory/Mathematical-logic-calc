import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
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

    function insertSymbol(sym) {
        var pos = formulaInput.cursorPosition;
        formulaInput.insert(pos, sym);
        formulaInput.cursorPosition = pos + sym.length;
        formulaInput.forceActiveFocus();
    }

    function deleteChar() {
        var pos = formulaInput.cursorPosition;
        if (pos > 0) {
            formulaInput.remove(pos - 1, pos);
            formulaInput.cursorPosition = pos - 1;
        }
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

    ScrollView {
        anchors.fill: parent
        contentWidth: parent.width
        clip: true

        ColumnLayout {
            width: parent.width
            anchors.margins: 10
            spacing: 8

            // --- ВЕРХНЯ ЧАСТИНА: Поле введення формули ---
            TextField {
                id: formulaInput
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                Layout.margins: 10
                font.pixelSize: 20
                font.bold: true
                placeholderText: "Введіть вираз..."
                
                // === МАГІЯ ВІДКЛЮЧЕННЯ СИСТЕМНОЇ КЛАВІАТУРИ ===
                // Робимо поле "Read Only", щоб Android не відкривав клавіатуру
                readOnly: true
                // Але примусово показуємо курсор, коли поле у фокусі
                cursorVisible: activeFocus
                // Дозволяємо тикати пальцем, щоб перемістити курсор
                selectByMouse: true
                onPressed: forceActiveFocus()
            }

            // --- СЕРЕДНЯ ЧАСТИНА: Калькуляторна клавіатура ---
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: keyboardLayout.implicitHeight + 16
                Layout.leftMargin: 10
                Layout.rightMargin: 10
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
                        // Тепер у нас 8 кнопок, робимо ідеально 4 колонки
                        columns: root.width > 500 ? 8 : 4
                        Layout.fillWidth: true
                        rowSpacing: 4
                        columnSpacing: 4

                        Button { text: "¬"; Layout.fillWidth: true; Layout.preferredHeight: 42; font.pixelSize: 18; font.bold: true; onClicked: insertSymbol("!") }
                        Button { text: "∧"; Layout.fillWidth: true; Layout.preferredHeight: 42; font.pixelSize: 18; font.bold: true; onClicked: insertSymbol(" & ") }
                        Button { text: "∨"; Layout.fillWidth: true; Layout.preferredHeight: 42; font.pixelSize: 18; font.bold: true; onClicked: insertSymbol(" | ") }
                        Button { text: "→"; Layout.fillWidth: true; Layout.preferredHeight: 42; font.pixelSize: 18; font.bold: true; onClicked: insertSymbol(" -> ") }
                        
                        Button { text: "↔"; Layout.fillWidth: true; Layout.preferredHeight: 42; font.pixelSize: 18; font.bold: true; onClicked: insertSymbol(" <-> ") }
                        Button { text: "("; Layout.fillWidth: true; Layout.preferredHeight: 42; font.pixelSize: 18; font.bold: true; onClicked: insertSymbol("(") }
                        Button { text: ")"; Layout.fillWidth: true; Layout.preferredHeight: 42; font.pixelSize: 18; font.bold: true; onClicked: insertSymbol(")") }
                        Button { text: "⌫"; Layout.fillWidth: true; Layout.preferredHeight: 42; font.pixelSize: 16; font.bold: true; onClicked: deleteChar() }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#d0d7de"
                    }

                    // 2. Панель літер (Змінних)
                    GridLayout {
                        columns: root.width > 500 ? 9 : 5
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

                        Button {
                            text: "A-Z ▾"
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            highlighted: true
                            font.bold: true
                            onClicked: root.fullAlphabetMode = true
                        }
                    }

                    // Повний алфавіт (A - Z)
                    GridLayout {
                        columns: root.width > 500 ? 10 : 6
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
                Layout.margins: 10
                highlighted: true
                font.bold: true
                font.pixelSize: 16
                onClicked: calculate()
            }

            TabBar {
                id: tabBar
                Layout.fillWidth: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                TabButton { text: "Покроково" }
                TabButton { text: "Таблиця" }
                TabButton { text: "Куайн-М." }
                onCurrentIndexChanged: calculate()
            }

            // --- НИЖНЯ ЧАСТИНА: Результати ---
            TextArea {
                id: resultArea
                readOnly: true
                Layout.fillWidth: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.minimumHeight: 300
                font.family: "Monospace"
                font.pixelSize: 13
                selectByMouse: true
                wrapMode: TextEdit.NoWrap
                background: Rectangle {
                    color: "#ffffff"
                    border.color: "#d0d7de"