/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    dfa.cpp
*  @brief   DFA相关函数实现
*
*  @author  林泽勋
*  @date    2024-11-05
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#include "dfa.h"

#include <QQueue>

DFA::DFA() {}

/*!
    @name   fromNFA
    @brief  NFA 转换为 DFA
    @param  NFA
    @return
    @attention
*/
void DFA::fromNFA(NFA nfa) {
    QHash<QSet<int>, int> revMapping;
    // 获取始态
    QSet<int> startSet;
    startSet.insert(nfa.startState);
    startSet = nfa.epsilonClosure(startSet);
    mapping[startState] = startSet;
    revMapping[startSet] = startState;
    if (startSet.contains(nfa.endState)) {
        endStates.insert(startState);
    }
    stateNum++;

    QSet<QSet<int>> vis;        // 判断某个状态是否被找到
    QQueue<int> q;              // 循环队列
    q.push_back(startState);
    while (!q.empty()) {

        // 取出队头
        int stateItem = q.front();
        q.pop_front();
        QSet<int> nfaStateSet = mapping[stateItem];

        // 未曾出现过的状态集合才需要查找
        if (!vis.contains(nfaStateSet)) {

            // 查询当前状态集合的转移
            for (QString changeItem: nfa.stateSet) {
                if (changeItem != "epsilon") {

                    // 获取对应转移到的epsilon闭包
                    QSet<int> changeClosure = nfa.valueClosure(nfaStateSet, changeItem);
                    QSet<int> changeEpsilon = nfa.epsilonClosure(changeClosure);

                    // 找不到集合
                    if (changeEpsilon.empty()) continue;

                    // 之前找到过这个状态
                    if (revMapping.contains(changeEpsilon)) {
                        int nextItem = revMapping[changeEpsilon];

                        G[stateItem][changeItem] = nextItem;
                        if (changeEpsilon.contains(nfa.endState)) {
                            endStates.insert(nextItem);
                        }

                        if (!vis.contains(changeEpsilon)) {
                            q.push_back(nextItem);
                        }
                    } else {
                        int nextItem = stateNum;
                        mapping[nextItem] = changeEpsilon;
                        revMapping[changeEpsilon] = nextItem;

                        G[stateItem][changeItem] = nextItem;
                        if (changeEpsilon.contains(nfa.endState)) {
                            endStates.insert(nextItem);
                        }
                        stateNum++;

                        if (!vis.contains(changeEpsilon)) {
                            q.push_back(nextItem);
                        }
                    }
                }
            }
            vis.insert(nfaStateSet);
        }
    }
}
