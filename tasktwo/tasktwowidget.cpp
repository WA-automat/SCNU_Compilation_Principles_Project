/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    tasktwowidget.cpp
*  @brief   项目任务二窗口文件
*
*  @author  林泽勋
*  @date    2024-11-05
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#include "tasktwowidget.h"
#include "ui_tasktwowidget.h"

TaskTwoWidget::TaskTwoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskTwoWidget)
{
    ui->setupUi(this);
}

TaskTwoWidget::~TaskTwoWidget()
{
    delete ui;
}
