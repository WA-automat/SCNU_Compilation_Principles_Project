/**
*****************************************************************************
*  Copyright (C), 2024, 林泽勋 20212821020
*  All right reserved. See COPYRIGHT for detailed Information.
*
*  @file    utils.cpp
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

#include "utils.h"

#include <QStringList>
#include <QStack>
#include <QDebug>

/*!
    @name   regexListPreprocessing
    @brief  正则表达式数组预处理：包含去除空格，重构[]等
    @param  regexList 正则表达式数组
    @return regexList 预处理后的正则表达式数组
    @attention
*/
QStringList regexListPreprocessing(QStringList regexList) {
    int lineSize = regexList.size();

    // 去除所有空格
    for (int i = 0; i < lineSize; i++) {
        regexList[i].replace(" ", "");
    }

    // 替换[]
    for (int i = 0; i < lineSize; i++) {
        int leftBracket = -1;    // 左右括号位置
        for (int j = 0; j < regexList[i].size(); j++) {
            if (regexList[i][j] == '\\') {             // 转义字符
                j++;
            } else if (regexList[i][j] == '[') {       // 匹配左括号
                leftBracket = j;
            } else if (regexList[i][j] == ']') {
                if (leftBracket == -1) {            // 出现了右括号，但没有出现左括号
                    continue;
                } else {
                    if (leftBracket == j - 1) continue; // 特判一下
                    QString bracketString = regexList[i].mid(leftBracket + 1, j - leftBracket - 1);
                    QString newBracketString = "";
                    int bracketLen = bracketString.size();
                    int groupNum = bracketLen / 3;
                    for (int k = 0; k < groupNum; k++) {
                        int beginIndex = 3 * k;
                        int endIndex = 3 * k + 2;
                        for (char l = bracketString[beginIndex].unicode(); l <= bracketString[endIndex].unicode(); l++) {
                            newBracketString.append(l);
                            newBracketString.append('|');
                        }
                    }
                    if (groupNum != 0) {
                        // 替换字符串
                        int newBracketStringLen = newBracketString.size();
                        newBracketString = newBracketString.left(newBracketStringLen - 1);
                        regexList[i] = regexList[i].left(leftBracket) + "(" + newBracketString + ")" + regexList[i].right(regexList[i].size() - j - 1);
                        j = leftBracket + newBracketString.size() + 2 - 1;
                    }
                    leftBracket = -1;               // 后续需要重新匹配中括号
                }
            }
        }
    }

    return regexList;
}

/*!
    @name   buildReHash
    @brief  正则表达式数组转换为hash表：key为正则表达式等号左侧，value为右侧
    @param  regexList 正则表达式数组
    @return 正则表达式哈希
    @attention
*/
QHash<QString, QString> buildReHash(QStringList regexList) {
    QHash<QString, QString> reHash;
    int lineSize = regexList.size();

    for (int i = 0; i < lineSize; i++) {
        QString regex = regexList[i];
        int regexLen = regex.size();
        for (int j = 0; j < regexLen; j++) {
            if (regex[j] == '=') {
                QString identifier = regex.left(j);
                QString regexBody = regex.right(regexLen - j - 1);
                reHash.insert(identifier, regexBody);
                break;
            }
        }
    }

    return reHash;
}

/*!
    @name   combineRegex
    @brief  合并正则表达式，仅保留需要展示的
    @param  reHash 原本的正则表达式哈希
    @return 合并后的正则表达式哈希
    @attention
*/
QHash<QString, QString> combineRegex(QHash<QString, QString> reHash) {
    for (QString key: reHash.keys()) {      // 逐个正则表达式进行替换
        QString replaceString = reHash[key];
        if (replaceString[0] != '(' || replaceString[replaceString.size() - 1] != ')') {
            replaceString = "(" + replaceString + ")";
        }
//        qDebug() << key << ' ' << replaceString;
        for (QString newkey: reHash.keys()) {
            reHash[newkey].replace(key, replaceString); // 替换
//            qDebug() << reHash[newkey];
        }
    }
    QHash<QString, QString> newReHash;  // 生成新的哈希
    for (QString key: reHash.keys()) {
        if (key[0] == '_') {
            newReHash.insert(key.right(key.size() - 1), reHash[key]);
        }
    }
    return newReHash;
}

/*!
    @name   addConnectOp
    @brief  为正则表达式添加连接符
    @param  re 正则表达式
    @return 添加连接符的正则表达式
    @attention
*/
QString addConnectOp(QString re) {
    for (int i = 0; i < re.size() - 1; i++) {
        if (re[i] == '\\') {
            i++;
            if (i < re.size() - 1) {
                if (re[i + 1] == '(' || !isOperator(re[i + 1].unicode())) {
                    re.insert(i + 1, '.');
                    i++;
                }
            }
        } else if (isOperator(re[i].unicode())) {
            if (re[i] == ')' || re[i] == '*' || re[i] == '+' || re[i] == '?') {
                if (!isOperator(re[i + 1].unicode()) || re[i + 1] == '(') {
                    re.insert(i + 1, '.');
                    i++;
                }
            }
        } else {
            if (!isOperator(re[i + 1].unicode()) || re[i + 1] == '(') {
                re.insert(i + 1, '.');
                i++;
            }
        }
    }
    return re;
}

/*!
    @name   regexToPostFix
    @brief  正则表达式转后缀表达式
    @param  re 正则表达式
    @return 后缀表达式
    @attention
*/
QString regexToPostFix(QString re) {
    QStack<QChar> s1;
    QStack<QString> s2;
    for (int i = 0; i < re.size(); i++) {
        if (re[i] == '\\') {
            if (i < re.size() - 1) {
                s2.push(re.mid(i, 2));
            }
            i++;
        }
        else if (isOperator(re[i].unicode())) {
            if (re[i] == '(') {
                s1.push(re[i]);                     // 左括号直接放入符号栈
            } else if (re[i] == ')') {
                while (!s1.empty() && s1.top() != '(') {                          // 右括号不断拿出符号栈内容，放入结果栈，直到遇到结果栈
                    s2.push(s1.top());
                    s1.pop();
                }
                if (!s1.empty() && s1.top() == '(') s1.pop();
            } else {
                int nowPriority = getPriority(re[i].unicode());
                while (!s1.empty() && getPriority(s1.top().unicode()) >= nowPriority) {
                    s2.push(s1.top());
                    s1.pop();
                }
                if (s1.empty() || getPriority(s1.top().unicode()) < nowPriority) {
                    s1.push(re[i]);
                }
            }
        } else {
            s2.push(re.mid(i, 1));
        }
    }
    while (!s1.empty()) {
        s2.push(s1.top());
        s1.pop();
    }
    QStack<QString> s3;
    QString postFixRegex = "";
    while (!s2.empty()) {
        s3.push(s2.top());
        s2.pop();
    }
    while (!s3.empty()) {
        postFixRegex += s3.top();
        s3.pop();
    }
    return postFixRegex;
}

/*!
    @name   getPriority
    @brief  获取运算符优先级
    @param  op 预算符
    @return 获取运算符优先级
    @attention
*/
int getPriority(const char &op) {
    switch (op) {
    case '(':
    case ')':
        return 0;
    case '*':
    case '+':
    case '?':
        return 4;
    case '.':
        return 2;
    case '|':
        return 1;
    default:
        return -1;
    }
}

/*!
    @name   isOperator
    @brief  判断是否为运算符
    @param  ch
    @return 是否为运算符
    @attention
*/
bool isOperator(const char &ch) {
    switch (ch) {
    case '(':
    case ')':
    case '*':
    case '+':
    case '?':
    case '|':
    case '.':
        return true;        // 运算符返回true
    default:
        return false;       // 非运算符返回false
    }
}

