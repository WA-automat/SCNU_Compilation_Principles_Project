/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    syntaxtree.cpp
*  @brief   语法树相关操作实现
*
*  @author  林泽勋
*  @date    2024-11-10
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#include "syntaxtree.h"
#include <QDebug>
#include <QStringList>

/*!
    @name   showAnalysis
    @brief  展示分析树
    @param  item ui树结点, tr树结点
    @return
    @attention
*/
void SyntaxNode::showAnalysis(QTreeWidgetItem *item, SyntaxNode* tr) {
    if (tr == nullptr) return;
    QString s;
    if (tr->str != "") s = tr->str;
    if (tr->note != "") s = tr->note;
    QTreeWidgetItem *newItem = new QTreeWidgetItem(QStringList() << s);
    item->addChild(newItem);
    for (SyntaxNode* childTr : tr->children) {
        showAnalysis(newItem, childTr);
    }
}

/*!
    @name   showSyntax
    @brief  展示语法树
    @param  item ui树结点, tr树结点
    @return
    @attention
*/
void SyntaxNode::showSyntax(QTreeWidgetItem *item, SyntaxNode *tr) {
    if (tr == nullptr) return;
    QTreeWidgetItem *newItem = new QTreeWidgetItem(QStringList() << tr->note);
    if (tr->str != "empty") {
        item->addChild(newItem);
        for (SyntaxNode* childTr : tr->children) {
            showSyntax(newItem, childTr);
        }
    }
    for (SyntaxNode* brotherTr : tr->brother) {
        showSyntax(item, brotherTr);
    }
}

/*!
    @name   showAnalysis
    @brief  展示分析树
    @param  item ui树结点
    @return
    @attention
*/
void SyntaxTree::showAnalysis(QTreeWidgetItem *item) {
    root->showAnalysis(item, root);
}

/*!
    @name   showSyntax
    @brief  展示语法树
    @param  item ui树结点
    @return
    @attention
*/
void SyntaxTree::showSyntax(QTreeWidgetItem *item) {
    root->showSyntax(item, root);
}
