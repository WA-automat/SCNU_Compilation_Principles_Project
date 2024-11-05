/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    utils.h
*  @brief   项目任务一工具函数头文件
*
*  @author  林泽勋
*  @date    2024-11-05
*  @version V1.0.0
*----------------------------------------------------------------------------
*  @note 历史版本  修改人员    修改内容
*  @note V1.0.0   林泽勋     创建文件
*****************************************************************************
*/
#ifndef UTILS_H
#define UTILS_H

#include <QStringList>
#include <QHash>

/*!
    @name   regexListPreprocessing
    @brief  正则表达式数组预处理：包含去除空格，重构[]等
    @param  正则表达式数组
    @return 预处理后的正则表达式数组
    @attention
*/
QStringList regexListPreprocessing(QStringList regexList);

/*!
    @name   buildReHash
    @brief  正则表达式数组转换为hash表：key为正则表达式等号左侧，value为右侧
    @param  regexList 正则表达式数组
    @return 正则表达式哈希
    @attention
*/
QHash<QString, QString> buildReHash(QStringList regexList);

/*!
    @name   combineRegex
    @brief  合并正则表达式，仅保留需要展示的
    @param  reHash 原本的正则表达式哈希
    @return 合并后的正则表达式哈希
    @attention
*/
QHash<QString, QString> combineRegex(QHash<QString, QString> reHash);

/*!
    @name   addConnectOp
    @brief  为正则表达式添加连接符
    @param  re
    @return 添加连接符的正则表达式
    @attention
*/
QString addConnectOp(QString re);

/*!
    @name   regexToPostFix
    @brief  正则表达式转后缀表达式
    @param  re 正则表达式
    @return 后缀表达式
    @attention
*/
QString regexToPostFix(QString re);

/*!
    @name   getPriority
    @brief  获取运算符优先级
    @param  op 预算符
    @return 获取运算符优先级
    @attention
*/
int getPriority(const char &op);

/*!
    @name   isOperator
    @brief  判断是否为运算符
    @param  ch
    @return 是否为运算符
    @attention
*/
bool isOperator(const char &ch);

#endif // UTILS_H
