/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    taskonewidget.cpp
*  @brief   项目任务一窗口界面
*
*  @author  林泽勋
*  @date    2024-11-05
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#include "taskonewidget.h"
#include "ui_taskonewidget.h"

#include <QFileDialog>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QDebug>
#include <QAction>
#include <QMessageBox>
#include <QException>

#include "../taskone/utils/utils.h"

TaskOneWidget::TaskOneWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskOneWidget)
{
    ui->setupUi(this);

    // 设置基本布局
    ui->tabWidget->setTabText(0, "NFA状态转换表");
    ui->tabWidget->setTabText(1, "DFA状态转换表");
    ui->tabWidget->setTabText(2, "最小化DFA状态转换表");
    ui->tabWidget->setTabText(3, "词法分析源程序");
    ui->tabWidget->setTabText(4, "词法分析结果");
    ui->tabWidget->setCurrentIndex(0);

    // 打开正则表达式文件并展示
    connect(ui->openButton, &QPushButton::clicked, this, [&]() {
        QString fileName = QFileDialog::getOpenFileName(this, "选择正则表达式文件");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                in.setCodec("UTF-8"); // 设置编码格式为 UTF-8
                ui->textEdit->setText(in.readAll());
                file.close();
            }
        }
    });

    // 正则表达式分析
    connect(ui->ReButton, &QPushButton::clicked, this, [&](){
        // 清空内容
        id2str.clear();
        id2nfa.clear();
        ui->comboBox->clear();

        QStringList lines = ui->textEdit->toPlainText().split('\n', QString::SkipEmptyParts);
        lines = regexListPreprocessing(lines);  // 预处理正则表达式

        // 构造键值对：键为等号左侧，值为等号右侧
        QHash<QString, QString> reHash = buildReHash(lines);

        // 合并正则表达式
        reHash = combineRegex(reHash);

        for (QString key: reHash.keys()) {
            reHash[key] = addConnectOp(reHash[key]);    // 正则添加连接符
            reHash[key] = regexToPostFix(reHash[key]);  // 转换为后缀表达式并存储
        }

        // 保存正则表达式映射并修改combobox样式
        id2str = reHash;
        for (QString key: id2str.keys()) {
            ui->comboBox->addItem(key);
        }

        // 正则表达式转NFA
        try {
            for (QString key: id2str.keys()) {
                NFA nfa;
                nfa.fromRegex(id2str[key]);
                id2nfa[key] = nfa;
            }
        } catch (QString e) {
            QMessageBox::warning(this, "警告", e);
            return;
        } catch (QException e) {
            QMessageBox::warning(this, "警告", "未知错误");
            return;
        }

        // NFA 转 DFA


        // DFA 最小化


        // 生成词法分析程序

        showNFA(id2nfa[ui->comboBox->currentText()]);
        QMessageBox::information(this, "提示", "正则表达式分析完成");
    });

    // 切换正则表达式
    connect(ui->comboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
            this, [&](const QString& text) {
        if (text.size() == 0 || text == "") return;

        // 获取对应的nfa、dfa、最小化dfa
        NFA nfa = id2nfa[text];

        // 渲染
        this->showNFA(nfa);
    });

}

TaskOneWidget::~TaskOneWidget() {
    delete ui;
}

void TaskOneWidget::showNFA(NFA nfa) {
    ui->nfaTableWidget->clear();
    ui->nfaTableWidget->setHorizontalHeaderLabels(QStringList());
    ui->nfaTableWidget->setRowCount(0);
    ui->nfaTableWidget->setColumnCount(0);
    ui->nfaTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->nfaTableWidget->setRowCount(nfa.stateNum);
    ui->nfaTableWidget->setColumnCount(nfa.stateSet.size());
    QStringList strListColumnHander;
    for (QString item : nfa.stateSet) {
        strListColumnHander << tr(item.toStdString().c_str());
    }
    ui->nfaTableWidget->setHorizontalHeaderLabels(strListColumnHander);
    QStringList strListRowHander;
    for (int i = 0; i < nfa.stateNum; i++) {
        strListRowHander << tr(QString::number(i).toStdString().c_str());
    }
    ui->nfaTableWidget->setVerticalHeaderLabels(strListRowHander);

    for (int i = 0; i < nfa.stateNum; i++) {
        int j = 0;
        for (QString stateChange: nfa.stateSet) {
            QString itemString = "";
            for (int k = 0; k < nfa.stateNum; k++) {
                if (nfa.G[i][k] == stateChange) {
                    itemString += QString::number(k);
                    itemString += ",";
                }
            }
            ui->nfaTableWidget->setItem(i, j, new QTableWidgetItem(itemString.left(itemString.size() - 1)));
            j++;
        }
    }

    // 添加始态、终态颜色
    QTableWidgetItem *beginItem = ui->nfaTableWidget->verticalHeaderItem(nfa.startState);
    QTableWidgetItem *endItem = ui->nfaTableWidget->verticalHeaderItem(nfa.endState);
    if (beginItem) beginItem->setTextColor(QColor(0, 255, 0));
    if (endItem) endItem->setTextColor(QColor(255, 0, 0));

    ui->nfaTableWidget->resizeColumnsToContents();
    ui->nfaTableWidget->resizeRowsToContents();
}
