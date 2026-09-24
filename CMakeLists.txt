cmake_minimum_required(VERSION 3.16)
project(LogicSimplifier VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Вмикаємо обробку Qt (MOC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

# Пригнічуємо попередження Qt про префікси
if(COMMAND qt_policy)
    qt_policy(SET QTP0001 NEW)
endif()

# 1. Створюємо виконуваний файл LogicSimplifier
qt_add_executable(LogicSimplifier
    src/app/main.cpp
    src/app/LogicController.h
    src/app/LogicController.cpp
    src/core/Lexer.h
    src/core/Lexer.cpp
    src/core/Parser.h
    src/core/Parser.cpp
    src/core/Evaluator.h
    src/core/Evaluator.cpp
    src/core/Simplifier.h
    src/core/Simplifier.cpp
    src/core/StepSimplifier.h
    src/core/StepSimplifier.cpp
)

# 2. Підключаємо QML під іменем модуля LogicUI (це прибере конфлікт імен)
qt_add_qml_module(LogicSimplifier
    URI LogicUI
    VERSION 1.0
    QML_FILES
        src/ui/main.qml
)

# 3. Лінкуємо бібліотеки
target_link_libraries(LogicSimplifier PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)