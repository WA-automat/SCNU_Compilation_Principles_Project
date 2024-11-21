#ifndef DFA_H
#define DFA_H

#include <QHash>
#include <QSet>
#include <QString>

#include "nfa.h"

class DFA
{
public:
    DFA();
    void clear();                       // 清空 DFA

    void fromNFA(NFA nfa);              // NFA 转 DFA
    void fromDFA(DFA dfa);              // DFA 最小化为 miniDFA

    QHash<int, QSet<int>> mapping;      // dfa状态到nfa或dfa状态的映射
    QHash<int, QHash<QString, int>> G;  // 邻接表
    int startState;                     // 始态
    QSet<int> endStates;                // 终态集合
    int stateNum;                       // 表示状态数量
    QSet<QString> changeSet;            // 转移集合
};

#endif // DFA_H
