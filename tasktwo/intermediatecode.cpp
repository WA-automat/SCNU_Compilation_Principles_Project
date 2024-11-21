#include "intermediatecode.h"

/*!
    @name   IntermediateCode
    @brief  构造函数
    @param
    @return
    @attention  生成一个空的中间代码项，使得中间代码项从1开始
*/
IntermediateCode::IntermediateCode() {
    GEN();
}

/*!
    @name   GEN
    @brief  生成四元组
    @param  四元组的四个变量a,b,c,d
    @return
    @attention
*/
void IntermediateCode::GEN(QString a, QString b, QString c, QString d) {
    Tetrad* tetrad = new Tetrad();
    tetrad->s[0] = a;
    tetrad->s[1] = b;
    tetrad->s[2] = c;
    tetrad->s[3] = d;
    tetrads.push_back(tetrad);
}

/*!
    @name   toIntermediateCode
    @brief  转为中间代码
    @param
    @return 中间代码
    @attention
*/
QString IntermediateCode::toIntermediateCode() {
    QString ans = "";
    for (int i = 1; i < this->tetrads.size(); i++) {
        ans += QString::number(i) + ": (";
        ans += this->tetrads[i]->s[0];
        ans += ",";
        ans += this->tetrads[i]->s[1];
        ans += ",";
        ans += this->tetrads[i]->s[2];
        ans += ",";
        ans += this->tetrads[i]->s[3];
        ans += ")\n";
    }
    return ans;
}

/*!
    @name   BackPatch
    @brief  回填算法
    @param  真出口或假出口P，待填充数值t
    @return
    @attention
*/
void IntermediateCode::BackPatch(int P, int t) {
    int Q = P;
    while (Q != 0) {
        QString S = tetrads[Q]->s[3];
        tetrads[Q]->s[3] = QString::number(t);
        Q = S.toInt();
    }
}

/*!
    @name   Merge
    @brief  合并真假出口链
    @param  真假出口链 P1 P2
    @return 合并后的真假出口链首
    @attention  P2为空时，返回P1;否则返回P2
*/
int IntermediateCode::Merge(int P1, int P2) {
    if (P2 == 0) return P1;
    int P = P2;
    while (tetrads[P]->s[3].toInt() != 0) {
        P = tetrads[P]->s[3].toInt();
    }
    tetrads[P]->s[3] = QString::number(P1);
    return P2;
}
