// Lightweight translator facade: Tokens -> AST -> C++ / Mermaid

#ifndef TRANSLATOR_TRANSLATOR_H
#define TRANSLATOR_TRANSLATOR_H

#include <memory>
#include <string>
#include <vector>

#include "Token.h"
#include "Ast/Stmt.h"
#include "Ast/Expr.h"
#include "Ast/Visitor.h"
#include "CodeGen/CppCodeGenerator.h"
#include "Flowchart/FlowchartGenerator.h"

class Translator {
public:
    // Build a minimal AST from token stream: assigns and Write/Writeln
    static std::unique_ptr<Program> buildMinimalAst(const std::vector<Token> &tokenList){
        auto program = std::make_unique<Program>();
        program->name = "generated";
        program->body = std::make_unique<Block>();
        int p = 0;
        while(p<(int)tokenList.size() && tokenList[p].getType()!="BEGIN") ++p;
        if (p<(int)tokenList.size()) ++p;
        while(p<(int)tokenList.size() && tokenList[p].getType()!="ENDofPROGRAM"){
            if (tokenList[p].getType()=="WRITE" || tokenList[p].getType()=="WRITELN"){
                auto w = std::make_unique<WriteStmt>();
                w->newline = tokenList[p].getType()=="WRITELN"; ++p;
                if (p<(int)tokenList.size() && tokenList[p].getType()=="OPENPARENTHESES") ++p;
                std::string joined;
                while(p<(int)tokenList.size() && tokenList[p].getType()!="CLOSEPARENTHESES"){
                    joined += tokenList[p].getValue();
                    ++p;
                }
                if (p<(int)tokenList.size() && tokenList[p].getType()=="CLOSEPARENTHESES") ++p;
                if (p<(int)tokenList.size() && tokenList[p].getType()=="SEMICOLON") ++p;
                auto arg = std::make_unique<StringLiteral>(); arg->value = joined; w->args.push_back(std::move(arg));
                program->body->statements.push_back(std::move(w));
                continue;
            }
            if (p+1<(int)tokenList.size() && tokenList[p].getType()=="VARIABLE" && tokenList[p+1].getType()=="ASSIGN"){
                auto a = std::make_unique<AssignStmt>();
                a->target = tokenList[p].getValue(); p+=2;
                if (p<(int)tokenList.size() && tokenList[p].getType()=="VALUEINTEGER"){
                    auto lit = std::make_unique<IntLiteral>(); lit->value = std::stoll(tokenList[p].getValue()); a->value = std::move(lit); ++p;
                } else if (p<(int)tokenList.size() && tokenList[p].getType()=="VALUEREAL"){
                    auto lit = std::make_unique<RealLiteral>(); lit->value = std::stod(tokenList[p].getValue()); a->value = std::move(lit); ++p;
                } else if (p<(int)tokenList.size() && tokenList[p].getType()=="VALUESTRING"){
                    auto lit = std::make_unique<StringLiteral>(); lit->value = tokenList[p].getValue(); a->value = std::move(lit); ++p;
                } else {
                    auto id = std::make_unique<Identifier>(); id->name = tokenList[p].getValue(); a->value = std::move(id); ++p;
                }
                if (p<(int)tokenList.size() && tokenList[p].getType()=="SEMICOLON") ++p;
                program->body->statements.push_back(std::move(a));
                continue;
            }
            ++p;
        }
        return program;
    }

    static std::string toCpp(Program &program){
        CppCodeGenerator gen; return gen.generate(program);
    }
    static std::string toMermaid(Program &program){
        FlowchartGenerator gen; return gen.generate(program);
    }
};

#endif // TRANSLATOR_TRANSLATOR_H



