/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    syntaxtree.h
*  @brief   语法树相关定义头文件
*
*  @author  林泽勋
*  @date    2024-11-10
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H

#include <QString>
#include <QTreeWidget>
#include <QVector>

/*!
    @name   SyntaxNode
    @brief  语法树结点
*/
class SyntaxNode {
public:
    QString note;
    QString str;
    QVector<SyntaxNode *> children;
    QVector<SyntaxNode *> brother;

    SyntaxNode(QString no, QString s): note(no), str(s), children(QVector<SyntaxNode *>()), brother(QVector<SyntaxNode *>()) {}
    SyntaxNode(QString s): SyntaxNode("", s) {}
    SyntaxNode(): SyntaxNode("") {}

    void showAnalysis(QTreeWidgetItem *item, SyntaxNode *tr);
    void showSyntax(QTreeWidgetItem *item, SyntaxNode *tr);
};

/*!
    @name   SyntaxTree
    @brief  语法树
*/
class SyntaxTree {
public:
    SyntaxTree(SyntaxNode* r = nullptr): root(r) {}

    // 语法树
    SyntaxNode* root;

    void showAnalysis(QTreeWidgetItem *item);   // 展示分析树
    void showSyntax(QTreeWidgetItem *item);     // 展示语法树
};

#endif // SYNTAXTREE_H
