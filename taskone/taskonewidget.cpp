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
#include <QDateTime>
#include <QProcess>

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

    ui->nfaTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->dfaTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->miniDfaTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->resultTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->textEdit->setPlaceholderText(QString("请在此处填充正则表达式，以下划线_开头的标识符会展示NFA、DFA、最小化DFA以及词法分析程序函数，如：\n_identifier=letter(letter|digit)*\ndigit=[0-9]\nletter=[a-zA-Z]"));

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
        id2dfa.clear();
        id2minidfa.clear();
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
        try {
            for (QString key: id2nfa.keys()) {
                DFA dfa;
                dfa.fromNFA(id2nfa[key]);
                id2dfa[key] = dfa;
            }
        } catch (QException e) {
            QMessageBox::warning(this, "警告", "未知错误");
            return;
        }

        // DFA 最小化
        try {
            for (QString key: id2dfa.keys()) {
                DFA minidfa;
                minidfa.fromDFA(id2dfa[key]);
                id2minidfa[key] = minidfa;
            }
        } catch (QException e) {
            QMessageBox::warning(this, "警告", "未知错误");
            return;
        }


        // 生成词法分析程序
        showNFA(id2nfa[ui->comboBox->currentText()]);
        showDFA(id2dfa[ui->comboBox->currentText()]);
        showMiniDFA(id2minidfa[ui->comboBox->currentText()]);

        QString analysisCode = this->toCode();
        ui->codeView->setText(analysisCode);

        QMessageBox::information(this, "提示", "正则表达式分析完成");
    });

    // 切换正则表达式
    connect(ui->comboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
            this, [&](const QString& text) {
        if (text.size() == 0 || text == "") return;

        // 获取对应的nfa、dfa、最小化dfa
        NFA nfa = id2nfa[text];
        DFA dfa = id2dfa[text];
        DFA miniDfa = id2minidfa[text];

        // 渲染
        this->showNFA(nfa);
        this->showDFA(dfa);
        this->showMiniDFA(miniDfa);
    });

    // 保存正则表达式文件
    connect(ui->saveRegexButton, &QPushButton::clicked, this, [&]() {
        QString fileName = QString("regex") + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()) + QString(".txt");
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << ui->textEdit->toPlainText();
            file.close();
            QMessageBox::information(this, "提示", "文件保存为：" + fileName + "成功！", QMessageBox::Yes);
        } else {
            QMessageBox::warning(this, "提示", "文件保存失败！", QMessageBox::Yes);
        }
    });

    // 上传源程序
    connect(ui->uploadCodeButton, &QPushButton::clicked, this, [&]() {
        QString fileName = QFileDialog::getOpenFileName(this, "上传源程序");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                in.setCodec("UTF-8"); // 设置编码格式为 UTF-8
                QString src_code = in.readAll();
                ui->srcEdit->setText(src_code);
                file.close();
            }
        }
    });

    // 源代码分析
    connect(ui->lexButton, &QPushButton::clicked, this, [&]() {
        ui->resultTableWidget->clear();
        ui->resultTableWidget->setHorizontalHeaderLabels(QStringList());
        ui->resultTableWidget->setRowCount(0);
        ui->resultTableWidget->setColumnCount(0);

        QString srcFileName = QString("src.txt");
        QFile srcFile(srcFileName);
        if (srcFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&srcFile);
            out << ui->srcEdit->toPlainText();
            out << flush;
            srcFile.close();
        } else {
            QMessageBox::warning(this, "提示", "待分词源文件保存失败！", QMessageBox::Yes);
        }

        QString anaFileName = QString("anslysis.cpp");
        QFile anaFile(anaFileName);
        if (anaFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&anaFile);
            out << ui->codeView->toPlainText();
            out << flush;
            anaFile.close();
        } else {
            QMessageBox::warning(this, "提示", "词法分析程序保存失败！", QMessageBox::Yes);
        }

//        QString compileStr = "g++ -std=c++11 " + anaFileName + " -o analysis";
        QProcess process;
        process.start("g++", QStringList() << anaFileName << "-o" << "analysis");
        if (!process.waitForFinished()) {
            QMessageBox::warning(this, "提示", "编译失败!", QMessageBox::Yes);
        }
        process.start("./analysis");
        if (!process.waitForFinished()) {
            QMessageBox::warning(this, "提示", "运行失败!", QMessageBox::Yes);
        }

        // 获取单词编码结果并渲染
        QFile sample("sample.lex");
        if (sample.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&sample);
            in.setCodec("UTF-8"); // 设置编码格式为 UTF-8
            QString lex = in.readAll();
            QStringList tokenList = lex.split('\n');
//            qDebug() << tokenList;
            ui->resultTableWidget->setRowCount(tokenList.size() - 1);
            ui->resultTableWidget->setColumnCount(2);
            ui->resultTableWidget->setHorizontalHeaderLabels(QStringList() << "单词（token）" << "类型（type）");
            int cnt = 0;
            for (QString tokenPair: tokenList) {
                if (cnt == tokenList.size() - 1) break;
                QStringList token = tokenPair.split(' ');
//                qDebug() << token;
                ui->resultTableWidget->setItem(cnt, 0, new QTableWidgetItem(token[0]));
                ui->resultTableWidget->setItem(cnt, 1, new QTableWidgetItem(token[1]));
                cnt++;
            }
            sample.close();
        } else {
            QMessageBox::warning(this, "提示", "文件打开失败！", QMessageBox::Yes);
        }

        ui->resultTableWidget->resizeColumnsToContents();
        ui->resultTableWidget->resizeRowsToContents();

    });
}

TaskOneWidget::~TaskOneWidget() {
    delete ui;
}

/*!
    @name   showNFA
    @brief  展示 NFA
    @param  nfa
    @return
    @attention
*/
void TaskOneWidget::showNFA(NFA& nfa) {
    ui->nfaTableWidget->clear();
    ui->nfaTableWidget->setHorizontalHeaderLabels(QStringList());
    ui->nfaTableWidget->setRowCount(0);
    ui->nfaTableWidget->setColumnCount(0);

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

/*!
    @name   showDFA
    @brief  展示 DFA
    @param  dfa
    @return
    @attention
*/
void TaskOneWidget::showDFA(DFA& dfa) {
    ui->dfaTableWidget->clear();
    ui->dfaTableWidget->setHorizontalHeaderLabels(QStringList());
    ui->dfaTableWidget->setRowCount(0);
    ui->dfaTableWidget->setColumnCount(0);

    ui->dfaTableWidget->setRowCount(dfa.stateNum);
    ui->dfaTableWidget->setColumnCount(dfa.changeSet.size() - (dfa.changeSet.contains("epsilon") ? 1 : 0));

    QStringList strListColumnHander;
    for (QString item : dfa.changeSet) {
        if (item == "epsilon") continue;
        strListColumnHander << tr(item.toStdString().c_str());
    }
    ui->dfaTableWidget->setHorizontalHeaderLabels(strListColumnHander);

    QStringList strListRowHander;
    for (int i = 0; i < dfa.stateNum; i++) {
        strListRowHander << tr(QString::number(i).toStdString().c_str());
    }
    ui->dfaTableWidget->setVerticalHeaderLabels(strListRowHander);

    for (int i = 0; i < dfa.stateNum; i++) {
        if (!dfa.G.contains(i)) continue;
        int j = 0;
        for (QString stateChange: dfa.changeSet) {
            if (stateChange == "epsilon") continue;
            if (dfa.G[i].contains(stateChange)) {

                int k = dfa.G[i][stateChange];
                QString titleTr = QString::number(k) + ":{";
                for (int item: dfa.mapping[k]) {
                    titleTr += QString::number(item);
                    titleTr += ",";
                }
                titleTr = titleTr.left(titleTr.size() - 1);
                titleTr += "}";

                ui->dfaTableWidget->setItem(i, j, new QTableWidgetItem(titleTr));
            }
            j++;
        }
    }

    // 添加始态、终态颜色
    QTableWidgetItem *beginItem = ui->dfaTableWidget->verticalHeaderItem(dfa.startState);
    if (beginItem) beginItem->setTextColor(QColor(0, 255, 0));
    for (int endState: dfa.endStates) {
        QTableWidgetItem *endItem = ui->dfaTableWidget->verticalHeaderItem(endState);
        if (endItem) endItem->setTextColor(QColor(255, 0, 0));
    }

//    ui->dfaTableWidget->resizeColumnsToContents();
    ui->dfaTableWidget->resizeRowsToContents();
}

/*!
    @name   showMiniDFA
    @brief  展示最小化的 DFA
    @param  minidfa
    @return
    @attention
*/
void TaskOneWidget::showMiniDFA(DFA &minidfa) {
//    qDebug() << minidfa.stateNum;
    ui->miniDfaTableWidget->clear();
    ui->miniDfaTableWidget->setHorizontalHeaderLabels(QStringList());
    ui->miniDfaTableWidget->setRowCount(0);
    ui->miniDfaTableWidget->setColumnCount(0);

    ui->miniDfaTableWidget->setRowCount(minidfa.stateNum);
    ui->miniDfaTableWidget->setColumnCount(minidfa.changeSet.size() - (minidfa.changeSet.contains("epsilon") ? 1 : 0));

    QStringList strListColumnHander;
    for (QString item : minidfa.changeSet) {
        if (item == "epsilon") continue;
        strListColumnHander << tr(item.toStdString().c_str());
    }
    ui->miniDfaTableWidget->setHorizontalHeaderLabels(strListColumnHander);

    QStringList strListRowHander;
    for (int i = 0; i < minidfa.stateNum; i++) {
        strListRowHander << tr(QString::number(i).toStdString().c_str());
    }
    ui->miniDfaTableWidget->setVerticalHeaderLabels(strListRowHander);

    for (int i = 0; i < minidfa.stateNum; i++) {
        if (!minidfa.G.contains(i)) continue;
        int j = 0;
        for (QString stateChange: minidfa.changeSet) {
            if (stateChange == "epsilon") continue;
            if (minidfa.G[i].contains(stateChange)) {

                int k = minidfa.G[i][stateChange];
                QString titleTr = QString::number(k) + ":{";
                for (int item: minidfa.mapping[k]) {
                    titleTr += QString::number(item);
                    titleTr += ",";
                }
                titleTr = titleTr.left(titleTr.size() - 1);
                titleTr += "}";

                ui->miniDfaTableWidget->setItem(i, j, new QTableWidgetItem(titleTr));
            }
            j++;
        }
    }

    // 添加始态、终态颜色
    QTableWidgetItem *beginItem = ui->miniDfaTableWidget->verticalHeaderItem(minidfa.startState);
    if (beginItem) beginItem->setTextColor(QColor(0, 255, 0));
    for (int endState: minidfa.endStates) {
        QTableWidgetItem *endItem = ui->miniDfaTableWidget->verticalHeaderItem(endState);
        if (endItem) endItem->setTextColor(QColor(255, 0, 0));
    }

//    ui->dfaTableWidget->resizeColumnsToContents();
    ui->miniDfaTableWidget->resizeRowsToContents();
}

/*!
    @name   toCode
    @brief  生成词法分析程序
    @param
    @return
    @attention
*/
QString TaskOneWidget::toCode() {
    QString code = "";
    code += "#include <iostream>\n";
    code += "#include <fstream>\n";
    code += "#include <string>\n";
    code += "#include <cctype>\n";
    code += "using namespace std;\n\n";

    code += "ifstream in(\"src.txt\", ios::in);\n";         // TODO: 源代码存储的位置
    code += "ofstream out(\"sample.lex\", ios::out | ios::trunc);\n";      // TODO: 单词编码保存的位置
    code += "string buf, buf_err, buf_suc;\n";              // 存储单词字符串
    code += "string token, token_suc;\n";                   // 存储单词类型
    code += "int read_cnt;\n\n";                            // 当前读到文件的位置

    // 跳过空白字符
    code += "void skipBlank() {\n";
    code += "\tchar c;\n";
    code += "\twhile (in.get(c)) {\n";
    code += "\t\tread_cnt++;\n";
    code += "\t\tif (c == \'\\n\') read_cnt++;\n";
    code += "\t\tif (!isspace(c)) {\n";
    code += "\t\t\tread_cnt--;\n";
    code += "\t\t\tin.unget();\n";
    code += "\t\t\tbreak;\n";
    code += "\t\t}\n";
    code += "\t}\n";
    code += "}\n\n";

    for (auto dfaKey: id2minidfa.keys()) {
        DFA minidfa = id2minidfa[dfaKey];
        // 生成各个DFA
        code += "bool check_" + dfaKey + "() {\n";
        code += "\tint state = " + QString::number(minidfa.startState) + ";\n";
        code += "\tchar c;\n";
        code += "\twhile ((c = in.peek()) != EOF) {\n";
        code += "\t\tswitch(state) {\n";
        for (int i = 0; i < minidfa.stateNum; i++) {        // 遍历状态，每个状态需要一个case
            code += "\t\tcase " + QString::number(i) + ":\n";
            code += "\t\t\tswitch (c) {\n";

            for (QString changeItem: minidfa.G[i].keys()) {    // 遍历转移，每个转移需要一个case
                code += "\t\t\tcase \'" + changeItem + "\':\n";
                code += "\t\t\t\tstate = " + QString::number(minidfa.G[i][changeItem]) + ";\n"; // 状态转移
                code += "\t\t\t\tbuf += c;\n"; // buf附加字符
                code += "\t\t\t\tin.get(c);\n"; // buf附加字符
                code += "\t\t\t\tbreak;\n";
            }

            code += "\t\t\tdefault:\n";
            // 判断当前是否为终态
            if (minidfa.endStates.contains(i)) {
                code += "\t\t\t\ttoken = \"" + dfaKey + "\";\n";
                code += "\t\t\t\treturn true;\n";
            } else {
                code += "\t\t\t\treturn false;\n";
            }

            code += "\t\t\t}\n";  // switch c 结束
            code += "\t\t\tbreak;\n"; // case state 结束
        }
        code += "\t\t}\n";  // switch state 结束
        code += "\t}\n";    // while 结束

        // 判断state是否为终态
        code += "\tif (";
        int cnt = 0;
        for (int i: minidfa.endStates) {
            code += "state == " + QString::number(i);
            cnt++;
            if (cnt != minidfa.endStates.size()) code += "||";
        }
        code += ") {\n";
        code += "\t\ttoken = \"" + dfaKey + "\";\n";
        code += "\t\treturn true;\n";
        code += "\t}\n";

        code += "\telse return false;\n";

        code += "}\n\n";
    }

    code += "int main(void) {\n";
    code += "\tbool flag;\n";
    code += "\tchar c;\n";
    code += "\tskipBlank();\n";
    code += "\twhile ((c = in.peek()) != EOF) {\n";

//    code += "cout << \"c:\" << c <<endl;";

    code += "\t\ttoken_suc.clear();\n";
    code += "\t\tbuf_suc.clear();\n";

    // keyword要在标识符之前
    if (id2minidfa.contains("keyword")) {
        code += "\t\tif (!check_keyword()) buf_err = buf;\n";
        code += "\t\telse if (buf.size() > buf_suc.size()) {\n";
        code += "\t\t\tbuf_suc = buf;\n";
        code += "\t\t\ttoken_suc = token;\n";
        code += "\t\t}\n";

//        code += "cout << \"buf:\" << buf <<endl;";
//        code += "cout << \"buf_suc:\" << buf_suc <<endl;";

        code += "\t\tbuf.clear();\n";
        code += "\t\tin.seekg(read_cnt, ios::beg);\n";
    }

    for (auto dfaKey: id2minidfa.keys()) {
        if (dfaKey == "keyword") continue;  // keyword 已经在前面完成
        code += "\t\tif (!check_" + dfaKey + "()) buf_err = buf;\n";
        code += "\t\telse if (buf.size() > buf_suc.size()) {\n";
        code += "\t\t\tbuf_suc = buf;\n";
        code += "\t\t\ttoken_suc = token;\n";
        code += "\t\t}\n";

//        code += "cout << \"buf:\" << buf <<endl;";
//        code += "cout << \"buf_suc:\" << buf_suc <<endl;";

        code += "\t\tbuf.clear();\n";
        code += "\t\tin.seekg(read_cnt, ios::beg);\n";
    }

    // 判断成功或失败
    code += "\t\tif (buf_suc.empty()) {\n";
    code += "\t\t\tout << buf_err << \" UNKNOWN\" << endl;\n";
    code += "\t\t\texit(1);\n";
    code += "\t\t}\n";
    code += "\t\tout << buf_suc << \" \" << token_suc << endl;\n";
    code += "\t\tread_cnt += buf_suc.size();\n";
    code += "\t\tin.seekg(read_cnt, ios::beg);\n";
    code += "\t\tskipBlank();\n";

    code += "\t}\n";    // while 结束
    code += "\tin.close();\n";
    code += "\tout.close();\n";
    code += "\treturn 0;\n";
    code += "}\n";
    return code;
}
