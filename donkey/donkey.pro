TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11


QMAKE_CXXFLAGS_RELEASE += -g

QMAKE_LFLAGS_RELEASE += -g

SOURCES += main.cpp \
    tokenizer.cpp \
    donkey.cpp \
    expression_builder.cpp \
    expressions.cpp \
    core_vtables.cpp \
    runtime_context.cpp \
    donkey_object.cpp \
    variables.cpp \
    module_bundle.cpp \
    module.cpp \
    compilers/branch_compilers.cpp \
    compilers/class_compiler.cpp \
    compilers/expression_compiler.cpp \
    compilers/function_compiler.cpp \
    compilers/jump_compilers.cpp \
    compilers/loop_compilers.cpp \
    compilers/scope_compiler.cpp \
    compilers/statement_compiler.cpp \
    compilers/using_compiler.cpp \
    compilers/variable_compiler.cpp \
    modules/io/io_module.cpp \
    string_vtable.cpp \
    array_vtable.cpp \
    modules/containers/containers_module.cpp \
    modules/containers/container.cpp \
    expressions/operators.cpp \
    vtable.cpp \
    errors.cpp

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
    scope.hpp \
    module.hpp \
    function.hpp \
    string_functions.hpp \
    vtable.hpp \
    config.hpp \
    donkey_callback.hpp \
    donkey_object.hpp \
    stack.hpp \
    compiler.hpp \
    compiler_helpers.hpp \
    module_bundle.hpp \
    donkey_function.hpp \
    expressions/arithmetic_expressions.hpp \
    compilers/branch_compilers.hpp \
    compilers/class_compiler.hpp \
    compilers/expression_compiler.hpp \
    compilers/function_compiler.hpp \
    compilers/jump_compilers.hpp \
    compilers/loop_compilers.hpp \
    compilers/scope_compiler.hpp \
    compilers/statement_compiler.hpp \
    compilers/using_compiler.hpp \
    compilers/variable_compiler.hpp \
    expressions/assignment_expressions.hpp \
    expressions/core_expressions.hpp \
    expressions/functional_expressions.hpp \
    expressions/item_expressions.hpp \
    expressions/logical_expressions.hpp \
    expressions/null_check_expressions.hpp \
    expressions/relation_expressions.hpp \
    expressions/sequential_expressions.hpp \
    expressions/string_expressions.hpp \
    expressions/ternary_expressions.hpp \
    expressions/unary_expressions.hpp \
    cpp/donkey_callback.hpp \
    cpp/native_converter.hpp \
    cpp/native_function.hpp \
    cpp/native_module.hpp \
    cpp/native_object.hpp \
    modules/io/io_module.hpp \
    modules/containers/containers_module.hpp \
    modules/containers/container.hpp \
    expressions/operators.hpp

OTHER_FILES += \
    examples.txt \
    dky/test.dky \
    dky/other.dky

