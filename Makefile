# MiniDB Makefile

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2

SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

TARGET = minidb

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

.PHONY: all clean

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/$(TARGET)

run: all
	$(BIN_DIR)/$(TARGET)

test: all
	@echo "Running tests..."
	$(BIN_DIR)/$(TARGET) < test/test.sql 