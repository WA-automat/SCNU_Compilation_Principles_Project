/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    taskonewidget.h
*  @brief   项目任务一窗口头文件
*
*  @author  林泽勋
*  @date    2024-11-05
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#ifndef TASKONEWIDGET_H
#define TASKONEWIDGET_H

#include <QWidget>
#include "nfa.h"
#include "dfa.h"

namespace Ui {
class TaskOneWidget;
}

class TaskOneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskOneWidget(QWidget *parent = nullptr);
    ~TaskOneWidget();

    QHash<QString, QString> id2str; // 正则表达式名称到正则表达式的映射
    QHash<QString, NFA> id2nfa;     // 正则表达式名称到NFA的映射
    QHash<QString, DFA> id2dfa;     // 正则表达式名称到DFA的映射
    QHash<QString, DFA> id2minidfa; // 正则表达式名称到最小化DFA的映射

private:
    Ui::TaskOneWidget *ui;

    void showNFA(NFA& nfa);          // 展示 NFA
    void showDFA(DFA& dfa);          // 展示 DFA
    void showMiniDFA(DFA& minidfa);  // 展示 MiniDFA
    QString toCode();   // 生成词法分析程序
};

#endif // TASKONEWIDGET_H
