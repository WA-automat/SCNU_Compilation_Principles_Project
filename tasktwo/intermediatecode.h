#ifndef INTERMEDIATECODE_H
#define INTERMEDIATECODE_H

#include <QString>
#include <QVector>

/*!
    @name   Tetrad
    @brief  四元组
*/
class Tetrad {
public:
    Tetrad(): s(new QString[4]) {}

    QString* s;
};

/*!
    @name   CodeCache
    @brief  中间代码生成临时变量
*/
class CodeCache {
public:
    int TC, FC, Chain, Head;
    QString val;
};

/*!
    @name   IntermediateCode
    @brief  中间代码存储类
*/
class IntermediateCode
{
public:
    IntermediateCode();

    // 成员变量
    QVector<Tetrad*> tetrads;

    // 成员函数
    void GEN(QString a = "_", QString b = "_", QString c = "_", QString d = "_");
    void BackPatch(int P, int t);
    int Merge(int P1, int P2);

    inline int NextStat() { return tetrads.size(); }

    QString toIntermediateCode(void);
};


#endif // INTERMEDIATECODE_H
