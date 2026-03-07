# Compiler
CXX = g++

# Compiler Flags
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic

# Zielprogramm
TARGET = fling

# Quellcode-Dateien
SRCS = main.cpp src/core.cpp src/frontend/ast.cpp src/frontend/lexer.cpp src/frontend/parser.cpp src/runtime/values.cpp src/runtime/interpreter.cpp

# Objektdateien (automatisch von SRCS abgeleitet)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET) run

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run:
	./fling

.PHONY: all clean
