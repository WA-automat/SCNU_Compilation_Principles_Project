/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    nfa.cpp
*  @brief   NFA相关函数实现
*
*  @author  林泽勋
*  @date    2024-11-05
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#include "nfa.h"

#include <QDebug>

NFA::NFA(int begin, int end): startState(begin), endState(end), stateNum(0), maxStateNum(100) {
    G.resize(maxStateNum);
    for (int i = 0; i < maxStateNum; i++) {
        G[i].resize(maxStateNum);
    }
}

/*!
    @name   clear
    @brief  清空NFA
    @param
    @return
    @attention
*/
void NFA::clear() {
    // 清空nfa
    for (int i = 0; i < maxStateNum; i++) {
        G[i].clear();
    }
    G.clear();
    maxStateNum = 100;
    stateNum = 0;
    startState = -1;
    endState = -1;
    G.resize(maxStateNum);
    for (int i = 0; i < maxStateNum; i++) {
        G[i].resize(maxStateNum);
    }
    stateSet.clear();
    stk.clear();
}

/*!
    @name   allocateMemory
    @brief  分配足够的内存空间
    @param  即将插入的状态数
    @return
    @attention
*/
void NFA::allocateMemory(int num) {
    // 当状态数即将超过最大状态时，分配更大的空间
    while (stateNum + num >= maxStateNum) {
        QVector<QVector<QString>> newG(2 * maxStateNum);
        for (int i = 0; i < 2 * maxStateNum; i++) {
            newG[i].resize(2 * maxStateNum);
        }
        for (int i = 0; i < maxStateNum; i++) {
            for (int j = 0; j < maxStateNum; j++) {
                newG[i][j] = G[i][j];
            }
        }
        maxStateNum *= 2;
        G = newG;
    }
}

/*!
    @name   fromRegex
    @brief  将后缀正则表达式转换为NFA
    @param  re 正则表达式
    @return
    @attention  re 是正则表达式的后缀形式
*/
void NFA::fromRegex(QString re) {
    stk.clear();
    for (int i = 0; i < re.size(); i++) {
        switch (re[i].unicode()) {
        case '\\':
            i++;
            if (i < re.size()) nfaChange(QString(re[i]));
            break;
        case '|':
            nfaOr();
            break;
        case '.':
            nfaAnd();
            break;
        case '*':
            nfaClosure();
            break;
        case '+':
            nfaPositiveClosure();
            break;
        case '?':
            nfaOption();
            break;
        case '#':
            nfaChange("epsilon");
            break;
        default:
            nfaChange(QString(re[i]));
            break;
        }
    }
    if (stk.empty()) {
        throw QString("NFA build ERROR!!!");
    } else {
        QPair<int, int> statePair = stk.top();
        stk.pop();
        startState = statePair.first;
        endState = statePair.second;
    }
    if (!stk.empty()) {
        throw QString("NFA build ERROR!!!");
    }
    buildTb();
}

/*!
    @name   epsilonClosure
    @brief  求某状态集合的epsilon闭包
    @param  state 状态集合
    @return 状态集合 state 的转移闭包
    @attention
*/
QSet<int> NFA::epsilonClosure(QSet<int> state) {
    QSet<int> rangeSet = state;
    while (true) {
        QSet<int> tmpSet;
        for (int item: rangeSet) {
            // 邻接矩阵构造
//            for (int i = 0; i < maxStateNum; i++) {
//                if (G[item][i] == "epsilon") {
//                    if (!state.contains(i)) {
//                        tmpSet.insert(i);
//                    }
//                }
//            }
            // 邻接表构造
            if (tb[item].contains("epsilon")) {
                tmpSet = tmpSet.unite(tb[item]["epsilon"]);
            }
        }
        if (tmpSet.empty() || state.contains(tmpSet)) break;
        state = state.unite(tmpSet);
        rangeSet = tmpSet;
    }
    return state;
}

/*!
    @name   valueClosure
    @brief  求状态集合经过某一转移的闭包
    @param  state 状态集合
    @param  value 转移类型
    @return 状态集合 state 经过 value 转移的闭包
    @attention
*/
QSet<int> NFA::valueClosure(QSet<int> state, QString value) {
    QSet<int> result;
    for (int item: state) {
        // 邻接矩阵构造
//        for (int i = 0; i < maxStateNum; i++) {
//            if (G[item][i] == value) {
//                result.insert(i);
//            }
//        }
        // 邻接表构造
        if (tb[item].contains(value)) {
            result = result.unite(tb[item][value]);
        }
    }
    return epsilonClosure(result);
}

/*!
    @name   nfaChange
    @brief  创建转移状态
    @param  str 转移类型
    @return
    @attention
*/
void NFA::nfaChange(QString str) {
    allocateMemory(2);
    G[stateNum][stateNum + 1] = str;  // 转义符不再出现
    stk.append({stateNum, stateNum + 1}); // 入栈
    stateNum += 2;                      // 状态数量 + 2
    stateSet.insert(str);
}

/*!
    @name   nfaOr
    @brief  构造取或的状态
    @param
    @return
    @attention
*/
void NFA::nfaOr() {
    allocateMemory(2);
    if (stk.size() >= 2) {                // 取出两个元素相连重新入栈
        QPair<int, int> left = stk.top();
        stk.pop();
        QPair<int, int> right = stk.top();
        stk.pop();
        G[stateNum][left.first] = "epsilon";
        G[stateNum][right.first] = "epsilon";
        G[left.second][stateNum + 1] = "epsilon";
        G[right.second][stateNum + 1] = "epsilon";
        stk.append({stateNum, stateNum + 1}); // 入栈
        stateNum += 2;                      // 状态数量 + 2
        stateSet.insert("epsilon");
    } else {
        throw QString("NFA build ERROR!!!");
    }
}

/*!
    @name   nfaAnd
    @brief  构造取连接的状态
    @param
    @return
    @attention
*/
void NFA::nfaAnd() {
    if (stk.size() >= 2) {                // 取出两个元素相连重新入栈
        QPair<int, int> tail = stk.top();
        stk.pop();
        QPair<int, int> head = stk.top();
        stk.pop();
        G[head.second][tail.first] = "epsilon";     // 连接操作
        stk.append({head.first, tail.second});        // 重新入栈
        stateSet.insert("epsilon");
    } else {
        throw QString("NFA build ERROR!!!");
    }
}

/*!
    @name   nfaClosure
    @brief  构造求闭包的状态
    @param
    @return
    @attention
*/
void NFA::nfaClosure() {
    allocateMemory(2);
    if (!stk.empty()) {                   // 闭包为单元运算符，因此只需要取出一个元素
        QPair<int, int> item = stk.top();
        stk.pop();
        G[stateNum][stateNum + 1] = "epsilon";
        G[stateNum][item.first] = "epsilon";
        G[item.second][stateNum + 1] = "epsilon";
        G[item.second][item.first] = "epsilon";
        stk.append({stateNum, stateNum + 1}); // 入栈
        stateNum += 2;                      // 状态数量 + 2
        stateSet.insert("epsilon");
    } else {
        throw QString("NFA build ERROR!!!");
    }
}

/*!
    @name   nfaPositiveClosure
    @brief  构造求正闭包的状态
    @param
    @return
    @attention
*/
void NFA::nfaPositiveClosure() {
    allocateMemory(2);
    if (!stk.empty()) {                   // 正闭包为单元运算符，因此只需要取出一个元素
        QPair<int, int> item = stk.top();
        stk.pop();
//        G[stateNum][stateNum + 1] = "epsilon";    // 闭包需要这个操作，而正闭包不需要
        G[stateNum][item.first] = "epsilon";
        G[item.second][stateNum + 1] = "epsilon";
        G[item.second][item.first] = "epsilon";
        stk.append({stateNum, stateNum + 1}); // 入栈
        stateNum += 2;                      // 状态数量 + 2
        stateSet.insert("epsilon");
    } else {
        throw QString("NFA build ERROR!!!");
    }
}

/*!
    @name   nfaOption
    @brief  构造可选的状态
    @param
    @return
    @attention
*/
void NFA::nfaOption() {
    allocateMemory(2);
    if (!stk.empty()) {                   // 正闭包为单元运算符，因此只需要取出一个元素
        QPair<int, int> item = stk.top();
        stk.pop();
        G[stateNum][item.first] = "epsilon";
        G[item.second][stateNum + 1] = "epsilon";
        G[item.first][item.second] = "epsilon";
        stk.append({stateNum, stateNum + 1}); // 入栈
        stateNum += 2;                      // 状态数量 + 2
        stateSet.insert("epsilon");
    } else {
        throw QString("NFA build ERROR!!!");
    }
}

/*!
    @name   buildTb
    @brief  构建邻接表
    @param
    @return
    @attention
*/
void NFA::buildTb() {
    for (int i = 0; i < stateNum; i++) {
        if (!tb.contains(i)) tb[i] = QHash<QString, QSet<int>>();
        for (int j = 0; j < stateNum; j++) {
            if (!tb[i].contains(G[i][j])) {
                tb[i][G[i][j]] = QSet<int>();
            }
            tb[i][G[i][j]] = tb[i][G[i][j]].unite(QSet<int>({j}));
        }
    }
}
