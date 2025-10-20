# Makefile for AiSDLAB project
# Compatible with MinGW, GCC, and other Unix-like compilers

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -I.
TARGET = AiSDLAB
SOURCES = main.cpp
HEADERS = Lexer.h Token.h Parser.h HierarchyList.h Postfix.h Stack.h SearchTreeTable.h \
          Expression/Expression.h Expression/StatementExpression.h Expression/ConditionExpression.h \
          Expression/Function.h Expression/FunctionExecutor.h Expression/Procedure.h Expression/CaseOf.h

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(TARGET).exe

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Release build
release: CXXFLAGS += -O2 -DNDEBUG
release: $(TARGET)

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the project (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  run     - Build and run the project"
	@echo "  debug   - Build with debug symbols"
	@echo "  release - Build optimized release version"
	@echo "  help    - Show this help message"

.PHONY: all clean run debug release help


