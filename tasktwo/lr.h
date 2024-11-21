/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    lr.h
*  @brief   LR 结构的头文件
*
*  @author  林泽勋
*  @date    2024-11-07
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#ifndef LR_H
#define LR_H

#include <QString>
#include <QStringList>
#include <QSet>
#include <QVector>
#include <QHash>

/*!
    @name   Item
    @brief  单条规则
*/
class Item {
public:
    QString name;
    QStringList rule;
    QSet<QString> next;
    int pos;

    static bool haveSameCore(Item i, Item j);

    friend bool operator==(const Item a, const Item b) {
        return a.name == b.name && a.rule == b.rule && a.next == b.next && a.pos == b.pos;
    }

    friend bool operator<(const Item a, const Item b) {
        if (a.name == b.name) {
            if (a.rule == b.rule) {
                if (a.pos == b.pos) {
                    return a.pos < b.pos;
                }
                return a.pos < b.pos;
            }
            return a.rule < b.rule;
        }
        return a.name < b.name;
    }
};

uint qHash(const Item& key);

/*!
    @name  State
    @brief DFA 中的单个状态
*/
class State {
public:
    QSet<Item> st;

    static State closure(State I, QHash<QString, QSet<QStringList>> grammars, QVector<QString> nonFinalizers, QHash<QString, QSet<QString>> firstSet);
    static State change(State I, QString X, QHash<QString, QSet<QStringList>> grammars, QVector<QString> nonFinalizers, QHash<QString, QSet<QString>> firstSet);

    static bool haveSameCore(State i, State j);

    friend bool operator==(const State a, const State b) {
        return a.st == b.st;
    }
};

uint qHash(const State& key);

/*!
    @name  LR
    @brief LR(1) 与 LALR(1) 的数据结构
*/
class LR
{
public:
    LR();

    // 成员变量
    int size;
    QHash<State, int> stateHash;
    QHash<int, QHash<QString, int>> changeHash;

    // 构建LR1
    void buildLR1(State faState, QHash<QString, QSet<QStringList>> grammars, QVector<QString> nonFinalizers,
                  QHash<QString, QSet<QString>> firstSet, QHash<QString, QSet<QString>> followSet);

    // 构建LALR1
    void buildLALR1(LR lr1);
};

#endif // LR_H
