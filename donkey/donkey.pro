TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11

QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg

SOURCES += main.cpp \
    tokenizer.cpp \
    donkey.cpp \
    expression_builder.cpp \
    expressions.cpp

HEADERS += \
    errors.hpp \
    tokenizer.hpp \
    donkey.hpp \
    expressions.hpp \
    variables.hpp \
    runtime_context.hpp \
    types.hpp \
    helpers.hpp \
    compiletime_context.hpp \
    identifiers.hpp \
    statements.hpp \
    donkey_function.h \
    expression_builder.hpp \
    unary_expressions.hpp \
    arithmetic_expressions.hpp \
    string_expressions.hpp \
    logical_expressions.hpp \
    relation_expressions.hpp \
    ternary_expressions.hpp \
    sequentional_expressions.hpp \
    assignment_expressions.hpp

OTHER_FILES += \
    examples.txt \
    test.dky

