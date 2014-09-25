TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    tokenizer.cpp \
    donkey.cpp

HEADERS += \
    errors.hpp \
    tokenizer.hpp \
    donkey.hpp \
    expressions.hpp \
    variables.hpp \
    runtime_context.hpp \
    types.hpp \
    helpers.hpp

OTHER_FILES += \
    examples.txt \
    test.dky

