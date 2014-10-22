TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11 -I ../donkey/


QMAKE_CXXFLAGS_RELEASE += -g

QMAKE_LFLAGS_RELEASE += -g

SOURCES += ../donkey/main.cpp \
    ../donkey/tokenizer.cpp \
    ../donkey/donkey.cpp \
    ../donkey/expression_builder.cpp \
    ../donkey/expressions.cpp \
    ../donkey/core_vtables.cpp \
    ../donkey/runtime_context.cpp \
    ../donkey/donkey_object.cpp \
    ../donkey/variables.cpp \
    ../donkey/module_bundle.cpp \
    ../donkey/module.cpp \
    ../donkey/compilers/branch_compilers.cpp \
    ../donkey/compilers/class_compiler.cpp \
    ../donkey/compilers/expression_compiler.cpp \
    ../donkey/compilers/function_compiler.cpp \
    ../donkey/compilers/jump_compilers.cpp \
    ../donkey/compilers/loop_compilers.cpp \
    ../donkey/compilers/scope_compiler.cpp \
    ../donkey/compilers/statement_compiler.cpp \
    ../donkey/compilers/using_compiler.cpp \
    ../donkey/compilers/variable_compiler.cpp \
    ../donkey/modules/io/io_module.cpp \
    ../donkey/string_vtable.cpp \
    ../donkey/array_vtable.cpp \
    ../donkey/modules/containers/containers_module.cpp \
    ../donkey/modules/containers/container.cpp \
    ../donkey/expressions/operators.cpp \
    ../donkey/vtable.cpp \
    ../donkey/errors.cpp

HEADERS += \
    ../donkey/errors.hpp \
    ../donkey/tokenizer.hpp \
    ../donkey/donkey.hpp \
    ../donkey/expressions.hpp \
    ../donkey/variables.hpp \
    ../donkey/runtime_context.hpp \
    ../donkey/helpers.hpp \
    ../donkey/identifiers.hpp \
    ../donkey/statements.hpp \
    ../donkey/expression_builder.hpp \
    ../donkey/scope.hpp \
    ../donkey/module.hpp \
    ../donkey/function.hpp \
    ../donkey/string_functions.hpp \
    ../donkey/vtable.hpp \
    ../donkey/config.hpp \
    ../donkey/donkey_callback.hpp \
    ../donkey/donkey_object.hpp \
    ../donkey/stack.hpp \
    ../donkey/compiler.hpp \
    ../donkey/compiler_helpers.hpp \
    ../donkey/module_bundle.hpp \
    ../donkey/donkey_function.hpp \
    ../donkey/expressions/arithmetic_expressions.hpp \
    ../donkey/compilers/branch_compilers.hpp \
    ../donkey/compilers/class_compiler.hpp \
    ../donkey/compilers/expression_compiler.hpp \
    ../donkey/compilers/function_compiler.hpp \
    ../donkey/compilers/jump_compilers.hpp \
    ../donkey/compilers/loop_compilers.hpp \
    ../donkey/compilers/scope_compiler.hpp \
    ../donkey/compilers/statement_compiler.hpp \
    ../donkey/compilers/using_compiler.hpp \
    ../donkey/compilers/variable_compiler.hpp \
    ../donkey/expressions/assignment_expressions.hpp \
    ../donkey/expressions/core_expressions.hpp \
    ../donkey/expressions/functional_expressions.hpp \
    ../donkey/expressions/item_expressions.hpp \
    ../donkey/expressions/logical_expressions.hpp \
    ../donkey/expressions/null_check_expressions.hpp \
    ../donkey/expressions/relation_expressions.hpp \
    ../donkey/expressions/sequential_expressions.hpp \
    ../donkey/expressions/string_expressions.hpp \
    ../donkey/expressions/ternary_expressions.hpp \
    ../donkey/expressions/unary_expressions.hpp \
    ../donkey/cpp/donkey_callback.hpp \
    ../donkey/cpp/native_converter.hpp \
    ../donkey/cpp/native_function.hpp \
    ../donkey/cpp/native_module.hpp \
    ../donkey/cpp/native_object.hpp \
    ../donkey/modules/io/io_module.hpp \
    ../donkey/modules/containers/containers_module.hpp \
    ../donkey/modules/containers/container.hpp \
    ../donkey/expressions/operators.hpp

OTHER_FILES += \
    ../donkey/examples.txt \
    ../donkey/dky/test.dky \
    ../donkey/dky/other.dky

