/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    tasktwowidget.h
*  @brief   项目任务二窗口头文件
*
*  @author  林泽勋
*  @date    2024-11-05
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#ifndef TASKTWOWIDGET_H
#define TASKTWOWIDGET_H

#include "analysistable.h"
#include "lr.h"

#include <QWidget>

#include <QString>
#include <QVector>
#include <QHash>
#include <QSet>
#include <QStringList>

namespace Ui {
class TaskTwoWidget;
}

class TaskTwoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskTwoWidget(QWidget *parent = nullptr);
    ~TaskTwoWidget();

    // 成员变量
    QString startString;                        // 起始符
    QVector<QString> nonFinalizers;             // 存储非终结符
    QHash<QString, QSet<QStringList>> grammars; // 存储文法规则
    QHash<QString, QSet<QString>> firstSet;     // first集合
    QHash<QString, QSet<QString>> followSet;    // follow集合
    AnalysisTable lalr1AnalysisTable;           // LALR分析表
    QHash<QString, QVector<int>> syntaxAction;  // 语法树语义动作
    QHash<QString, QVector<QStringList>> interAction;    // 中间代码语义动作

    // 辅助函数
    void getFirst();  // 获取 first 集合
    void getFollow();  // 获取 follow 集合

    // 展示LR1
    void showLR1(LR lr1, State firstState);

    // 展示LALR1
    void showLALR1(LR lalr1, State firstState);

    // 判断是否已经构建LALR1
    bool canAnalysis;
private:
    Ui::TaskTwoWidget *ui;
};

#endif // TASKTWOWIDGET_H
