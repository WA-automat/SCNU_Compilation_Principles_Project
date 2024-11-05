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

#include <QWidget>

namespace Ui {
class TaskTwoWidget;
}

class TaskTwoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskTwoWidget(QWidget *parent = nullptr);
    ~TaskTwoWidget();

private:
    Ui::TaskTwoWidget *ui;
};

#endif // TASKTWOWIDGET_H
