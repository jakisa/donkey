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
    donkey_object.cpp \
    variables.cpp \
    loop_compilers.cpp \
    statement_compiler.cpp \
    expression_compiler.cpp \
    variable_compiler.cpp \
    jump_compilers.cpp \
    branch_compilers.cpp \
    class_compiler.cpp \
    function_compiler.cpp \
    scope_compiler.cpp

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
    sequential_expressions.hpp \
    stack.hpp \
    null_check_expressions.hpp \
    item_expressions.hpp \
    compiler.hpp \
    loop_compilers.hpp \
    statement_compiler.hpp \
    function_compiler.hpp \
    variable_compiler.hpp \
    jump_compilers.hpp \
    branch_compilers.hpp \
    scope_compiler.hpp \
    expression_compiler.hpp \
    class_compiler.hpp \
    compiler_helpers.hpp \
    module_bundle.hpp \
    donkey_function.hpp

OTHER_FILES += \
    examples.txt \
    test.dky \
    other.dky

