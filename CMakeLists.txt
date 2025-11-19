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
        Scripts/Lexer.h
        Scripts/Token.h
    Expression/Expression.h
    Expression/StatementExpression.h
        Parser/PascalParserToExpression.h
        Scripts/HierarchyList.h
    Expression/ConditionExpression.h
        Scripts/Postfix.h
        Scripts/Stack.h
        Scripts/SearchTreeTable.h
    Expression/Function.h
    Expression/FunctionExecutor.h
    Expression/Procedure.h
    Expression/CaseOf.h
        CodeGen/PascalCodeGenerator.h
        Parser/CParserToAST.h
        Parser/AstVisualizer.h
        Parser/ASTParserToJSON.h
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
