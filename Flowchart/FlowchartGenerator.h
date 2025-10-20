#ifndef FLOWCHART_GENERATOR_H
#define FLOWCHART_GENERATOR_H

#include <string>
#include <sstream>
#include "../Ast/Visitor.h"
#include "../Ast/Expr.h"
#include "../Ast/Stmt.h"
#include "../Ast/ExprAcceptImpl.h"

// Simple Mermaid flowchart generator for linear blocks with IO/Process
class FlowchartGenerator : public Visitor {
public:
    std::string generate(Program &program){
        id=0; out.str(""); out.clear();
        out << "flowchart TD\n";
        out << "A"<< (++id) <<"([Start])\n"; lastNode = "A"+std::to_string(id);
        program.accept(*this);
        out << "A"<< (++id) <<"([End])\n";
        out << lastNode << " --> A"<< id <<"\n";
        return out.str();
    }

    void visit(Identifier&) override {}
    void visit(StringLiteral&) override {}
    void visit(IntLiteral&) override {}
    void visit(RealLiteral&) override {}
    void visit(BinaryOp&) override {}

    void visit(AssignStmt &s) override {
        std::string node = newNode("[" + s.target + " := expr]");
        link(node);
    }
    void visit(WriteStmt &s) override {
        std::string node = newNode("[/Write/]");
        link(node);
    }
    void visit(Block &b) override {
        for (auto &st: b.statements) st->accept(*this);
    }
    void visit(Program &p) override { if (p.body) p.body->accept(*this); }

private:
    std::ostringstream out; int id{0}; std::string lastNode;
    std::string newNode(const std::string &label){ out<<"A"<<(++id)<<""<<label<<"\n"; return "A"+std::to_string(id);}    
    void link(const std::string &to){ out<< lastNode << " --> " << to <<"\n"; lastNode = to; }
};

#endif // FLOWCHART_GENERATOR_H




