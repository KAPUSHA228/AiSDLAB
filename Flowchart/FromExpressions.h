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

struct FlowchartFromExpressions {
    static std::string build(const std::vector<Expression *> &exprs) {
        std::ostringstream out; // Будет содержать итоговый Mermaid-код
        std::string last; // Текущая позиция в графе для соединения узлов
        int id = 0;
        out << "graph TD\n";
        // out << "N" << (++id) << "([" + title + "])\n";
        out << "N" << (++id) << "([program qq])\n";
        last = "N" + std::to_string(id);
        size_t i = 0;
        while (i < exprs.size()) {
            Expression *e = exprs[i];
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
                bool isElse = !cx->getCondition().empty() && cx->getCondition().front().getValue() == "else";
                if (!isElse) {
                    std::string node = newDecision(out, ++id, tokensToLine(cx->getCondition()));
                    link(out, last, node);
                    last = node;
                    ++i;
                    continue;
                }
                {std::string node = newProcess(out, ++id, "else");
                link(out, last, node);
                last = node;
                ++i;}


                // decision node
                std::string cond = tokensToLine(cx->getCondition());
                std::string dec = newDecision(out, ++id, cond);
                link(out, last, dec);

                // yes branch
                std::string yesTail = dec;
                const auto body = cx->getBody().second;
                std::string f = FlowchartFromExpressions::build(body);


                // no branch
                std::string noTail = dec;
                if (i + 1 < exprs.size()) {
                    if (auto nextCx = dynamic_cast<ConditionExpression *>(exprs[i + 1])) {
                        bool isElse2 = !nextCx->getCondition().empty() && nextCx->getCondition().front().getValue() == "else";
                        if (isElse2) {
                            const auto body2 = nextCx->getBody().second;
                            for (auto *inner: body2) {
                                if (auto st3 = dynamic_cast<StatementExpression *>(inner)) {
                                    std::string n3 = newProcess(out, ++id, tokensToLine(st3->getList()));
                                    out << dec << " -- No --> " << n3 << "\n";
                                    noTail = n3;
                                } else {
                                    std::string n3 = newProcess(out, ++id, "nested");
                                    out << dec << " -- No --> " << n3 << "\n";
                                    noTail = n3;
                                }
                            }
                            ++i; // consume else
                        } else {
                            std::string skip = newProcess(out, ++id, "skip");
                            out << dec << " -- No --> " << skip << "\n";
                            noTail = skip;
                        }
                    } else {
                        std::string skip = newProcess(out, ++id, "skip");
                        out << dec << " -- No --> " << skip << "\n";
                        noTail = skip;
                    }
                } else {
                    std::string skip = newProcess(out, ++id, "skip");
                    out << dec << " -- No --> " << skip << "\n";
                    noTail = skip;
                }

                // merge point
                std::string merge = newProcess(out, ++id, "merge");
                out << yesTail << " --> " << merge << "\n";
                out << noTail << " --> " << merge << "\n";
                last = merge;
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
        out << "N" << (++id) << "([End])\n";
        out << last << " --> N" << id << "\n";
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
        out << node << "[" << lab << "]\n";

        return node;
    }

    static std::string newOut(std::ostringstream &out, int id, const std::string &label) {
        std::string node = "N" + std::to_string(id);
        std::string lab = escape(label);
        out << node << "[/" << lab << "/]\n";
        return node;
    }

    static std::string newDecision(std::ostringstream &out, int& id, const std::string &label) {
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


