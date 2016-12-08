.PHONY=all
SOURCES_DIR=donkey
BUILD_DIR=bld
CXX_FLAGS=--std=c++14 -I $(SOURCES_DIR)
EXE=$(BUILD_DIR)/dky

SOURCES=$(filter-out $(SOURCES_DIR)/modules/gui/window_X11.cpp, $(shell find $(SOURCES_DIR) -name *.cpp))

OBJS=$(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

DIRS=$(sort $(dir $(OBJS)))

all: $(EXE)

$(DIRS):
	mkdir -p $@

$(OBJS): $(BUILD_DIR)/%.o: %.cpp
	g++ $(CXX_FLAGS) -c $^ -o $@

$(EXE): $(DIRS) $(OBJS)
	g++ $(LD_FLAGS) $(OBJS) -o $@
