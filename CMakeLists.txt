cmake_minimum_required(VERSION 3.16)
project(AiSDLAB)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Add executable
add_executable(${PROJECT_NAME}
    main.cpp
    Lexer.h
    Token.h
    Expression/Expression.h
    Expression/StatementExpression.h
    Parser.h
    HierarchyList.h
    Expression/ConditionExpression.h
    Postfix.h
    Stack.h
    SearchTreeTable.h
    Expression/Function.h
    Expression/FunctionExecutor.h
    Expression/Procedure.h
    Expression/CaseOf.h
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
