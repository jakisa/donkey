TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11

#QMAKE_CXXFLAGS_RELEASE += -g

#QMAKE_LFLAGS_RELEASE += -g

SOURCES += main.cpp \
    tokenizer.cpp \
    donkey.cpp \
    expression_builder.cpp \
    expressions.cpp \
    core_vtables.cpp \
    runtime_context.cpp \
    donkey_object.cpp

HEADERS += \
    errors.hpp \
    tokenizer.hpp \
    donkey.hpp \
    expressions.hpp \
    variables.hpp \
    runtime_context.hpp \
    helpers.hpp \
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
    assignment_expressions.hpp \
    functional_expressions.hpp \
    scope.hpp \
    module.hpp \
    function.hpp \
    string_functions.hpp \
    vtable.hpp \
    core_expressions.hpp \
    native_function.hpp \
    config.hpp \
    native_converter.hpp \
    donkey_callback.hpp \
    donkey_object.hpp \
    sequential_expressions.hpp

OTHER_FILES += \
    examples.txt \
    test.dky

