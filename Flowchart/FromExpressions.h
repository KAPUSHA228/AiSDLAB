// Generate Mermaid flowchart from existing Expression objects (no AST required)

#ifndef FLOWCHART_FROM_EXPRESSIONS_H
#define FLOWCHART_FROM_EXPRESSIONS_H

#include <string>
#include <sstream>
#include <vector>
#include "../Expression/Expression.h"
#include "../Expression/StatementExpression.h"
#include "../Expression/ConditionExpression.h"
#include "../Expression/CaseOf.h"

static int id = 1;
static int tmpid = 0;

struct FlowchartFromExpressions {
    static std::string build(const std::vector<Expression *> &exprs) {
        std::ostringstream out; // Будет содержать итоговый Mermaid-код
        std::string last; // Текущая позиция в графе для соединения узлов
        last = "N" + std::to_string(id);
        size_t i = 0;
        std::cout << "EBEBE" << std::endl;
        while (i < exprs.size()) {
            Expression *e = exprs[i];
            e->print(0);
            if (auto st = dynamic_cast<StatementExpression *>(e)) {
                //std::cout<<"EX "<<st->getList()[0].getValue()<<std::endl;
                std::string node =
                        st->getList()[0].getValue() == "Writeln" || st->getList()[0].getValue() == "Write" ||
                        st->getList()[0].getValue() == "Readln" || st->getList()[0].getValue() == "Read"
                        ? newOut(out, ++id, tokensToLine(st->getList()))
                        : newProcess(out, ++id, tokensToLine(st->getList()));
                link(out, last, node);
                last = node;
                ++i;
                continue;
            }
            if (auto cx = dynamic_cast<ConditionExpression *>(e)) {
                bool isElseOrRepeat = (!cx->getCondition().empty()) &&
                                      (cx->getCondition().front().getValue() == "else" ||
                                       cx->getCondition().front().getValue() == "repeat");
                if (!isElseOrRepeat) {
                    std::string node = newDecision(out, ++id, tokensToLine(cx->getCondition()));
                    tmpid = id;
                    link(out, last, node);
                    last = node;
                    ++i;
                }
                const auto body = cx->getBody().second;
                std::string f = FlowchartFromExpressions::build(body);
                link(out, last, f);
                last = f;

                //std::cout << "BUBUBU" << f << std::endl;
                //std::string node = newProcess(out, ++id, "else");
                //link(out, last, node);
                //last = node;
                //++i;
                // decision node
//                std::string cond = tokensToLine(cx->getCondition());
//                std::string dec = newDecision(out, ++id, cond);
//                link(out, last, dec);
//                // yes branch
//                std::string yesTail = dec;

                ++i;
                continue;
            }
            if (auto sw = dynamic_cast<CaseOf *>(e)) {
                std::string node = newDecision(out, ++id, "case ... of");
                link(out, last, node);
                last = node;
                ++i;
                continue;
            }
            ++i;
        }
        out << "N" << (id) << "([End])\n";
        //link(out,"N" + std::to_string(--(--id)),"N" + std::to_string(--id));
        return out.str();
    }

private:
    static std::string tokensToLine(const std::vector<Token> &v) {
        std::string s;
        for (Token t: v) {
            if (!s.empty())
                s += " ";
            s += t.getValue();
        }
        return s;
    }

    static std::string newProcess(std::ostringstream &out, int id, const std::string &label) {
        std::string node = "N" + std::to_string(id);
        std::string lab = escape(label);
        out << node << "[\"" << lab << "\"]\n";

        return node;
    }

    static std::string newOut(std::ostringstream &out, int id, const std::string &label) {
        std::string node = "N" + std::to_string(id);
        std::string lab = escape(label);
        out << node << "[/\"" << lab << "\"/]\n";
        return node;
    }

    static std::string newDecision(std::ostringstream &out, int &id, const std::string &label) {
        std::string node = "N" + std::to_string(id);
        out << node << "{\"" << escape(label) << "\"}\n";
        return node;
    }

    static void link(std::ostringstream &out, const std::string &a, const std::string &b) {
        out << a << " --> " << b << "\n";
    }

    static std::string escape(const std::string &s) {
        std::string r;
        r.reserve(s.size());
        for (char c: s) {
            if (c == '"')
                r += "\\\"";
            else
                r += c;
        }
        return r;
    }
};

#endif // FLOWCHART_FROM_EXPRESSIONS_H


