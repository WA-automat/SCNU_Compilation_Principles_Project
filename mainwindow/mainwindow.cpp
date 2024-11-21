/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    mainwindow.cpp
*  @brief   主窗口页面
*
*  @author  林泽勋
*  @date    2024-11-04
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 基本布局
    this->setWindowFlags(Qt::Window);
    this->showMaximized();
    this->setWindowTitle("编译原理课程设计：22级计科2班 林泽勋 20212821020");
    ui->tabWidget->setTabText(0, "项目任务一");
    ui->tabWidget->setTabText(1, "项目任务二");
    ui->tabWidget->setCurrentIndex(0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

