/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    nfa.h
*  @brief   NFA相关函数头文件
*
*  @author  林泽勋
*  @date    2024-11-05
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#ifndef NFA_H
#define NFA_H

#include <QString>
#include <QSet>
#include <QVector>
#include <QStack>
#include <QPair>

/*!
    @name  NFA
    @brief 邻接矩阵存储NFA数据
*/
class NFA
{
public:
    NFA(int begin = -1, int end = -1);
    void allocateMemory(int num);   // 检查插入 num 状态数量后是否超过空间
    void clear();                   // 清空NFA
    void fromRegex(QString re);     // 利用后缀正则表达式构造NFA

    // 闭包函数
    QSet<int> epsilonClosure(QSet<int> state);  // 计算epsilon闭包
    QSet<int> valueClosure(QSet<int> state, QString value); // 计算某个集合状态能通过value转移到的状态集合

    // 构造NFA的中间辅助函数
    void nfaChange(QString str);
    void nfaOr();
    void nfaAnd();
    void nfaClosure();
    void nfaPositiveClosure();
    void nfaOption();


    // 用于正则表达式构造NFA的栈
    QStack<QPair<int, int>> stk;

    // 成员变量
    QVector<QVector<QString>> G;        // 邻接矩阵
    QSet<QString> stateSet;             // 存储所有状态类型
    int startState;                     // 表示起始状态
    int endState;                       // 表示终止状态
    int stateNum;                       // 表示状态数量
    int maxStateNum;                    // 表示当前最多存储的状态数量
};

#endif // NFA_H
