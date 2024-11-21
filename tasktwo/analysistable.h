/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    analysistable.h
*  @brief   分析表数据结构头文件
*
*  @author  林泽勋
*  @date    2024-11-10
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#ifndef ANALYSISTABLE_H
#define ANALYSISTABLE_H

#include "lr.h"
#include <QVector>
#include <QString>
#include <QHash>

/*!
    @name   AnalysisTableItem
    @brief  分析表结点
*/
class AnalysisTableItem {
public:
    int kind;   // 1 表示移进、2 表示规约、3 表示非终结符移进、4 表示接受
    int idx;
};

/*!
    @name   AnalysisTable
    @brief  LALR1 分析表
*/
class AnalysisTable
{
public:
    AnalysisTable();

    // 规约规则存储
    QVector<Item> recursion;

    // LALR1 分析表
    QHash<int, QHash<QString, AnalysisTableItem>> tb;

    void clear();
};

/*!
    @name   StkItem
    @brief  分析栈结点
*/
class StkItem {
public:
    int kind;       // 0 表示符号，1表示状态
    int state;
    QString str;
    QString detail;
};

#endif // ANALYSISTABLE_H
