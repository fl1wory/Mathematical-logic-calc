import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 700
    height: 650
    title: "Logic Simplifier"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // Поле введення формули
        TextField {
            id: formulaInput
            Layout.fillWidth: true
            font.pixelSize: 18
            placeholderText: "Введіть формулу (напр. !(A | B) -> (!A & !B))"
            selectByMouse: true
        }

        // Панель математичних кнопок для швидкого введення
        RowLayout {
            spacing: 6
            Layout.alignment: Qt.AlignHCenter

            Button { text: "∧ (&&)";  onClicked: formulaInput.insert(formulaInput.cursorPosition, " & ") }
            Button { text: "∨ (|)";  onClicked: formulaInput.insert(formulaInput.cursorPosition, " | ") }
            Button { text: "¬ (!)";  onClicked: formulaInput.insert(formulaInput.cursorPosition, "!") }
            Button { text: "→ (->)"; onClicked: formulaInput.insert(formulaInput.cursorPosition, " -> ") }
            Button { text: "↔ (<->)";onClicked: formulaInput.insert(formulaInput.cursorPosition, " <-> ") }
            Button { text: "(";      onClicked: formulaInput.insert(formulaInput.cursorPosition, "(") }
            Button { text: ")";      onClicked: formulaInput.insert(formulaInput.cursorPosition, ")") }
            Button { text: "Очистити"; onClicked: formulaInput.clear() }
        }

        // Вкладки режимів
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            TabButton { text: "Покрокове спрощення" }
            TabButton { text: "Таблиця істинності" }
            TabButton { text: "Метод Куайна-Мак-Класкі" }
        }

        // Кнопка розрахунку
        Button {
            text: "Обчислити"
            Layout.fillWidth: true
            highlighted: true
            font.bold: true
            font.pixelSize: 16

            onClicked: {
                if (tabBar.currentIndex === 0) {
                    resultArea.text = logicController.solveStepByStep(formulaInput.text)
                } else if (tabBar.currentIndex === 1) {
                    resultArea.text = logicController.solveTruthTable(formulaInput.text)
                } else {
                    resultArea.text = logicController.solveQuine(formulaInput.text)
                }
            }
        }

        // Поле відображення результату зі скролом
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            TextArea {
                id: resultArea
                readOnly: true
                font.family: "Monospace"
                font.pixelSize: 14
                selectByMouse: true
                wrapMode: TextEdit.NoWrap
                background: Rectangle {
                    color: "#f5f5f5"
                    border.color: "#ccc"
                    radius: 4
                }
            }
        }
    }
}