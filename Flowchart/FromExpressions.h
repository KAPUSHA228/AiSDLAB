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
static int tmpCondId[3] = {-1, -1, -1};
static int layer = 0;

class FlowchartFromExpressions {
private:
    std::vector<Expression *> exprs;
public:
    FlowchartFromExpressions(const std::vector<Expression *> &exprs) {
        this->exprs = exprs;
    }

    std::string build(const std::vector<Expression *> &exprs) {
        layer++;
        std::ostringstream out; // Будет содержать итоговый Mermaid-код
        std::string last; // Текущая позиция в графе для соединения узлов
        //std::string copylast;
        last = "N" + std::to_string(id);
        size_t i = 0;
        std::cout << "BEBEBE" << std::endl;

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
                if (tmpCondId[2] != -1) {
                    link(out, "N"+to_string(tmpCondId[1]), node);
                    link(out, "N"+to_string(tmpCondId[2]), node);
                    tmpCondId[0]=-1;
                    tmpCondId[1]=-1;
                    tmpCondId[2]=-1;
                } else {
                    if ((layer == 1 && i != 0) || i != 0)// || (id - tmpCondId[0] != 1))
                    {
                        link(out, last, node);
                    }
                }
                last = node;
                ++i;
                continue;
            }
            if (auto cx = dynamic_cast<ConditionExpression *>(e)) {
                bool isElseOrRepeat = (cx->getCondition().front().getValue() == "else" ||
                                       cx->getCondition().front().getValue() == "repeat");
                if (!isElseOrRepeat) {
                    std::string node = newDecision(out, ++id, tokensToLine(cx->getCondition()));
                    tmpCondId[0] = id;
                    if (tmpCondId[2] != -1) {
                        link(out, "N"+to_string(tmpCondId[1]), node);
                        link(out, "N"+to_string(tmpCondId[2]), node);
                        tmpCondId[0]=-1;
                        tmpCondId[1]=-1;
                        tmpCondId[2]=-1;
                    } else {
                        link(out, last, node);
                    }

                    last = node;
                    //  copylast=last;
                }
                const auto body = cx->getBody().second;
                std::string f = build(body);
                // last=copylast;
                if (!isElseOrRepeat) {
                    tmpCondId[1] = id;
                    link(out, last, f, "true");
                } else {
                    tmpCondId[2] = id;
                    link(out, last, f, "false");
                }
                //last = f;
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
        if (layer == 1) {
            out << "N" << (++id) << "([End])\n";
            link(out, "N" + std::to_string(--id), "N" + std::to_string(id));
        }
        layer--;
        return out.str();
    }

private:
    std::string tokensToLine(const std::vector<Token> &v) {
        std::string s;
        for (Token t: v) {
            if (!s.empty())
                s += " ";
            s += t.getValue();
        }
        return s;
    }

    std::string newProcess(std::ostringstream &out, int id, const std::string &label) {
        std::string node = "N" + std::to_string(id);
        std::string lab = escape(label);
        out << node << "[\"" << lab << "\"]\n";

        return node;
    }

    std::string newOut(std::ostringstream &out, int id, const std::string &label) {
        std::string node = "N" + std::to_string(id);
        std::string lab = escape(label);
        out << node << "[/\"" << lab << "\"/]\n";
        return node;
    }

    std::string newDecision(std::ostringstream &out, int &id, const std::string &label) {
        std::string node = "N" + std::to_string(id);
        out << node << "{\"" << escape(label) << "\"}\n";
        return node;
    }

    void link(std::ostringstream &out, const std::string &a, const std::string &b) {
        out << a << " --> " << b << "\n";
    }

    void link(std::ostringstream &out, const std::string &a, const std::string &b, const std::string direction) {
        out << a << " -->|" << direction << "| " << b << "\n";
    }

    std::string escape(const std::string &s) {
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


