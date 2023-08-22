CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -pthread

SRC_DIR = ./server ./slashFunctions ./user
BUILD_DIR = ./build

# List of source files
SRCS = $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.cpp))
OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(SRCS:.cpp=.o)))

# Executable name
TARGET = main

.PHONY: all clean

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET): $(OBJS) main.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) main.cpp

$(BUILD_DIR)/%.o: */%.cpp */%.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

