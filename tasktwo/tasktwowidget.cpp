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
#include "lr.h"
#include "syntaxtree.h"
#include "tasktwowidget.h"
#include "ui_tasktwowidget.h"

#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QQueue>
#include <QStack>
#include <QDateTime>

#include "intermediatecode.h"

TaskTwoWidget::TaskTwoWidget(QWidget *parent) :
    QWidget(parent), canAnalysis(false),
    ui(new Ui::TaskTwoWidget)
{
    ui->setupUi(this);

    // 基本布局
    ui->tabWidget->setTabText(0, "First与Follow集合");
    ui->tabWidget->setTabText(1, "LR(1) DFA");
    ui->tabWidget->setTabText(2, "LALR(1) DFA");
    ui->tabWidget->setTabText(3, "LALR(1) 分析表");
    ui->tabWidget->setTabText(4, "单词编码分析结果");
    ui->tabWidget->setCurrentIndex(0);

    ui->firstTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->followTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lr1TableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lalr1TableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lalrAnalysisTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->recursionTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->analysisLexTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->sentenceWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->recursionTableWidget->verticalHeader()->hide();
    ui->sentenceWidget->verticalHeader()->hide();

    ui->analysisTabWidget->setTabText(0, "单词编码语法分析结果");
    ui->analysisTabWidget->setTabText(1, "语法树生成结果");
    ui->analysisTabWidget->setTabText(2, "中间代码生成结果");
    ui->analysisTabWidget->setCurrentIndex(0);

    ui->innerTabWidget->setTabText(0, "单词编码");
    ui->innerTabWidget->setTabText(1, "语法树语义动作");
    ui->innerTabWidget->setTabText(2, "中间代码语义动作");
    ui->innerTabWidget->setCurrentIndex(0);

    ui->treeWidget->setHeaderLabels(QStringList() << "语法分析树");
    ui->syntaxTreeWidget->setHeaderLabels(QStringList() << "抽象语法树AST");

    ui->textEdit->setPlaceholderText(QString("请在此处填充文法规则，首个文法规则左侧为起始符号，符号之间需要以空格隔开。如：\nA -> ( A ) | a"));

    // 打开文法文件并展示
    connect(ui->uploadButton, &QPushButton::clicked, this, [&]() {
        QString fileName = QFileDialog::getOpenFileName(this, "选择文法文件");
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

    // 文法分析
    connect(ui->analysisButton, &QPushButton::clicked, this, [&]() {

        // 清空各类数据结构
        nonFinalizers.clear();
        grammars.clear();
        firstSet.clear();
        followSet.clear();
        lalr1AnalysisTable.clear();

        // 获取正文
        QString content = ui->textEdit->toPlainText();
        QStringList grammarList = content.split('\n');

        // 存储文法规则
        // 获取非终结符
        for (const auto &grammar: qAsConst(grammarList)) {
            QStringList wordList = grammar.split(" ");
            for (int i = 0; i < wordList.size(); i++) {
                if (wordList[i] == "") {
                    wordList.removeAt(i);
                }
            }
            if (wordList.size() < 3 || wordList[1] != "->") {
                QMessageBox::warning(this, "提示", "文法分析失败，左侧非终结符只能出现单个词，非终结符后需要有空格以及 -> 符号", QMessageBox::Yes);
                return;
            } else {
                nonFinalizers.push_back(wordList[0]);
            }
        }

        // 获取起始符
        QStringList firstGrammar = grammarList[0].split(" ");
        if (!firstGrammar.empty()) startString = firstGrammar[0];

        // 分词
        for (const auto &grammar: qAsConst(grammarList)) {
            QStringList wordList = grammar.split(" ");
            for (int i = 0; i < wordList.size(); i++) {
                if (wordList[i] == "") {
                    wordList.removeAt(i);
                } else {
                    if (!nonFinalizers.contains(wordList[i]) && wordList[i] != "->") {  // 提前存储：终结符的first集合是终结符本身
                        firstSet[wordList[i]].insert(wordList[i]);
                    }
                }
            }
            // 删除非终结符与->
            QString nonfinal = wordList[0];
            wordList.removeAt(0);
            wordList.removeAt(0);
            int lastIdx = 0;
            for (int i = 1; i < wordList.size() - 1; i++) {
                if (wordList[i] == "|") {
                    grammars[nonfinal].insert(wordList.mid(lastIdx, i - lastIdx));
                    lastIdx = i + 1;
                }
            }
            grammars[nonfinal].insert(wordList.mid(lastIdx, wordList.size() - lastIdx));
        }

        getFirst();     // 获取first集合
        getFollow();    // 获取follow集合

        // 判断是否需要扩充文法
        firstSet["$"] = QSet<QString>({"$"});
        if (grammars[startString].size() > 1) {
            grammars[startString + "_new"].insert(QStringList() << startString);
            firstSet[startString + "_new"].insert(startString);
            followSet[startString + "_new"].insert("$");
            startString = startString + "_new";
        }

        // 构造第一条文法规则
        Item firstItem;
        firstItem.name = startString;
        firstItem.rule = *grammars[startString].begin();
        firstItem.next = QSet<QString>({"$"});
        firstItem.pos = 0;

        State firstState;
        firstState.st.insert(firstItem);
        firstState = State::closure(firstState, grammars, nonFinalizers, firstSet);

        LR lr1, lalr1;
        lr1.stateHash[firstState] = lr1.size++;

        lr1.buildLR1(firstState, grammars, nonFinalizers, firstSet, followSet);

        // 判断 LR1 文法
        for (auto state: lr1.stateHash.keys()) {
            // 找到所有的归约项，判断前进字符是否有重复
            QSet<QString> callback;
            for (auto item: state.st) {
                // 判断规约项
                if (item.pos == item.rule.size()) {
                    for (auto next: item.next) {
                        if (callback.contains(next)) {
                            QMessageBox::warning(this, "警告", "该文法出现了规约-规约冲突，出现了向前看符号有相同的规约项，不是 LR(1) 文法", QMessageBox::Yes);
                            return;
                        }
                    }
                    callback.unite(item.next);
                }
            }
        }

        lalr1.buildLALR1(lr1);

        // 判断 LALR1 文法：移进-规约冲突 与 规约-规约冲突
        for (auto state: lalr1.stateHash.keys()) {
            // 找到所有的归约项，判断前进字符是否有重复
            QSet<QString> callback;
            for (auto item: state.st) {
                // 判断规约项
                if (item.pos == item.rule.size()) {
                    for (auto next: item.next) {
                        if (callback.contains(next)) {
                            QMessageBox::warning(this, "警告", "该文法在 LALR(1) 中出现了规约-规约冲突，出现了向前看符号有相同的规约项，不是 LALR(1) 文法", QMessageBox::Yes);
                            return;
                        }
                    }
                    callback.unite(item.next);
                }
            }
            // 找到所有移进项
            QSet<QString> footIn;
            for (auto item: state.st) {
                // 判断移进项
                if (item.pos != item.rule.size()) {
                    footIn.insert(item.rule[item.pos]);
                }
            }
            // 判断是否有移进-规约冲突
            if (!footIn.intersect(callback).empty()) {
                QMessageBox::warning(this, "警告", "该文法在 LALR(1) 中出现了移进-规约冲突，出现了规约项的向前看符号与移进项的下一个字符相同的情况，不是 LALR(1) 文法，但后续会使用移进项进行分析，解决二义性", QMessageBox::Yes);
            }
        }

        // 渲染 LR1 和 LALR1
        showLR1(lr1, firstState);
        showLALR1(lalr1, firstState);

        // 构造分析表
        ui->lalrAnalysisTableWidget->clear();
        ui->lalrAnalysisTableWidget->setHorizontalHeaderLabels(QStringList());
        ui->lalrAnalysisTableWidget->setRowCount(0);
        ui->lalrAnalysisTableWidget->setColumnCount(0);

        ui->recursionTableWidget->clear();
        ui->recursionTableWidget->setHorizontalHeaderLabels(QStringList());
        ui->recursionTableWidget->setRowCount(0);
        ui->recursionTableWidget->setColumnCount(0);

        QStringList header;
        QSet<QString> finalString;
        for (int i = 0; i < lalr1.size; i++) {
            for (auto changeMethod: lalr1.changeHash[i].keys()) {
                if (!nonFinalizers.contains(changeMethod)) {
                    finalString.insert(changeMethod);
                }
            }
        }
        QList<QString> finalStringList = finalString.toList();
        QList<QString> nonFinalStringList = nonFinalizers.toList();
        qSort(finalStringList.begin(), finalStringList.end());
        finalStringList.append("$");
        qSort(nonFinalStringList.begin(), nonFinalStringList.end());
        header.append(finalStringList);
        header.append(nonFinalStringList);
        ui->lalrAnalysisTableWidget->setColumnCount(header.size());
        ui->lalrAnalysisTableWidget->setHorizontalHeaderLabels(header);
        ui->lalrAnalysisTableWidget->setRowCount(lalr1.size);

        // 可视化 LALR(1) 分析表
        QHash<int, State> revHash;
        for (auto state: lalr1.stateHash.keys()) {
            revHash[lalr1.stateHash[state]] = state;
        }
        QHash<QString, int> headerHash;
        int headerIdx = 0;
        for (QString head: header) {
            headerHash[head] = headerIdx++;
        }
        for (int i = 0; i < lalr1.size; i++) {

            // 渲染行表头
            auto headerItem = new QTableWidgetItem(QString::number(i));
            if (i == 0) headerItem->setTextColor(QColor(255, 0, 0));
            ui->lalrAnalysisTableWidget->setVerticalHeaderItem(i, headerItem);

            State state = revHash[i];
            for (Item item: state.st) {
                if (item.pos == item.rule.size()) {
                    // 规约项
                    if (item.name == startString) {
                        // 接受状态
                        for (auto next: item.next) {
                            if (next == "$") {
                                AnalysisTableItem lalrItem;
                                lalrItem.kind = 4;
                                lalr1AnalysisTable.tb[i].insert(next, lalrItem);
                                ui->lalrAnalysisTableWidget->setItem(i, headerHash[next], new QTableWidgetItem("接受"));
                            } else if (lalr1AnalysisTable.recursion.contains(item)) {
                                AnalysisTableItem lalrItem;
                                lalrItem.idx = lalr1AnalysisTable.recursion.indexOf(item);
                                lalrItem.kind = 2;
                                lalr1AnalysisTable.tb[i].insert(next, lalrItem);
                                ui->lalrAnalysisTableWidget->setItem(i, headerHash[next], new QTableWidgetItem("r" + QString::number(lalr1AnalysisTable.recursion.indexOf(item))));
                            } else {
                                lalr1AnalysisTable.recursion.push_back(item);
                                AnalysisTableItem lalrItem;
                                lalrItem.idx = lalr1AnalysisTable.recursion.size() - 1;
                                lalrItem.kind = 2;
                                lalr1AnalysisTable.tb[i].insert(next, lalrItem);
                                ui->lalrAnalysisTableWidget->setItem(i, headerHash[next], new QTableWidgetItem("r" + QString::number(lalr1AnalysisTable.recursion.size() - 1)));
                            }
                        }
                    } else {
                        for (auto next: item.next) {
                            // 判断该规约规则是否已经出现过
                            if (lalr1AnalysisTable.recursion.contains(item)) {
                                AnalysisTableItem lalrItem;
                                lalrItem.idx = lalr1AnalysisTable.recursion.indexOf(item);
                                lalrItem.kind = 2;
                                lalr1AnalysisTable.tb[i].insert(next, lalrItem);
                                ui->lalrAnalysisTableWidget->setItem(i, headerHash[next], new QTableWidgetItem("r" + QString::number(lalr1AnalysisTable.recursion.indexOf(item))));
                            } else {
                                lalr1AnalysisTable.recursion.push_back(item);
                                AnalysisTableItem lalrItem;
                                lalrItem.idx = lalr1AnalysisTable.recursion.size() - 1;
                                lalrItem.kind = 2;
                                lalr1AnalysisTable.tb[i].insert(next, lalrItem);
                                ui->lalrAnalysisTableWidget->setItem(i, headerHash[next], new QTableWidgetItem("r" + QString::number(lalr1AnalysisTable.recursion.size() - 1)));
                            }
                        }
                    }
                }
            }
        }
        for (int i = 0; i < lalr1.size; i++) {
            // 移进项
            for (int j = 0; j < finalStringList.size(); j++) {
                auto changeMethod = finalStringList[j];
                if (lalr1.changeHash[i].contains(changeMethod)) {
                    AnalysisTableItem lalrItem;
                    lalrItem.idx = lalr1.changeHash[i][changeMethod];
                    lalrItem.kind = 1;
                    lalr1AnalysisTable.tb[i].insert(changeMethod, lalrItem);
                    ui->lalrAnalysisTableWidget->setItem(i, j, new QTableWidgetItem("s" + QString::number(lalr1.changeHash[i][changeMethod])));
                }
            }
            // 规约后的回溯
            for (int j = 0; j < nonFinalStringList.size(); j++) {
                auto changeMethod = nonFinalStringList[j];
                if (lalr1.changeHash[i].contains(changeMethod)) {
                    AnalysisTableItem lalrItem;
                    lalrItem.idx = lalr1.changeHash[i][changeMethod];
                    lalrItem.kind = 3;
                    lalr1AnalysisTable.tb[i].insert(changeMethod, lalrItem);
                    ui->lalrAnalysisTableWidget->setItem(i, j + finalStringList.size(), new QTableWidgetItem(QString::number(lalr1.changeHash[i][changeMethod])));
                }
            }
        }

        ui->recursionTableWidget->setRowCount(lalr1AnalysisTable.recursion.size());
        ui->recursionTableWidget->setColumnCount(2);

        ui->recursionTableWidget->setHorizontalHeaderLabels(QStringList() << "规约编号" << "规约规则");
        for (int i = 0; i < lalr1AnalysisTable.recursion.size(); i++) {
            ui->recursionTableWidget->setItem(i, 0, new QTableWidgetItem("r" + QString::number(i)));
            QString recursionStr = lalr1AnalysisTable.recursion[i].name;
            recursionStr += " -> ";
            for (int j = 0; j < lalr1AnalysisTable.recursion[i].rule.size(); j++) {
                recursionStr += lalr1AnalysisTable.recursion[i].rule[j];
                recursionStr += " ";
            }
            ui->recursionTableWidget->setItem(i, 1, new QTableWidgetItem(recursionStr));
        }

        ui->lalrAnalysisTableWidget->resizeColumnsToContents();
        ui->lalrAnalysisTableWidget->resizeRowsToContents();
        ui->recursionTableWidget->resizeColumnsToContents();
        ui->recursionTableWidget->resizeRowsToContents();

        canAnalysis = true;
    });

    // 上传单词编码文件
    connect(ui->lexButton, &QPushButton::clicked, this, [&]() {

        ui->analysisLexTableWidget->clear();
        ui->analysisLexTableWidget->setHorizontalHeaderLabels(QStringList());
        ui->analysisLexTableWidget->setRowCount(0);
        ui->analysisLexTableWidget->setColumnCount(0);

        ui->analysisLexTableWidget->setColumnCount(2);

        ui->analysisLexTableWidget->setHorizontalHeaderLabels(QStringList() << "单词（token）" << "类型（type）");

        QString fileName = QFileDialog::getOpenFileName(this, "选择单词编码文件");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                in.setCodec("UTF-8"); // 设置编码格式为 UTF-8
                QString allText = in.readAll();
                QStringList lines = allText.split('\n');
                int len = 0;
                for (QString line: lines) {
                    if (line == "") continue;
                    QStringList words = line.split(' ');
                    if (words.size() != 2) continue;
                    len++;
                }
                ui->analysisLexTableWidget->setRowCount(len);
                int cnt = 0;
                for (QString line: lines) {
                    if (line == "") continue;
                    QStringList words = line.split(' ');
                    if (words.size() != 2) continue;
//                    qDebug() << cnt << words;
                    ui->analysisLexTableWidget->setItem(cnt, 0, new QTableWidgetItem(words[0]));
                    ui->analysisLexTableWidget->setItem(cnt, 1, new QTableWidgetItem(words[1]));
                    cnt++;
                }
                file.close();
            }
        }

        ui->analysisLexTableWidget->resizeColumnsToContents();
        ui->analysisLexTableWidget->resizeRowsToContents();
    });

    // 单词编码分析
    connect(ui->grammarButton, &QPushButton::clicked, this, [&]() {
        if (!canAnalysis) {
            QMessageBox::warning(this, "警告", "还未进行文法分析！");
            return;
        }

        if (syntaxAction.empty()) {
            QMessageBox::warning(this, "警告", "还未上传生成语法树的语义动作！");
        } else if (interAction.empty()) {
            QMessageBox::warning(this, "警告", "还未上传生成中间代码的语义动作！");
        }

        ui->sentenceWidget->clear();
        ui->sentenceWidget->setHorizontalHeaderLabels(QStringList());
        ui->sentenceWidget->setRowCount(0);
        ui->sentenceWidget->setColumnCount(0);

        ui->sentenceWidget->setColumnCount(2);

        ui->sentenceWidget->setHorizontalHeaderLabels(QStringList() << "分析栈" << "单词编码输入");

        QQueue<QPair<QString, QString>> sentence;   // 输入队列(first表示token，second表示token类型)
        QStack<StkItem> analysisStack;                // 分析栈

        for (int i = 0; i < ui->analysisLexTableWidget->rowCount(); i++) {
            QTableWidgetItem* item1 = ui->analysisLexTableWidget->item(i, 0);
            QTableWidgetItem* item2 = ui->analysisLexTableWidget->item(i, 1);
            if (item2->text() == "annotation") continue;
            if (item1 && item2) {
                sentence.push_back({item1->text(), item2->text()});
            }
        }
        sentence.push_back({"$", "end"});

        StkItem firstItem, secondItem;
        firstItem.kind = 0;
        firstItem.str = "$";
        firstItem.detail = "";
        secondItem.kind = 1;
        secondItem.state = 0;
        analysisStack.push(firstItem);
        analysisStack.push(secondItem);
        int cnt = 0;    // 分析栈计数器

        /*!
            @name   printAnalysis
            @brief  打印分析过程表
            @param
            @return
            @attention
        */
        std::function<void(void)> printAnalysis = [&]() {
            QString analysisString;
            for (auto item: analysisStack) {
                if (item.kind == 1) {
                    analysisString += QString::number(item.state) + " ";
                } else {
                    analysisString += item.str + " ";
                }
            }
            ui->sentenceWidget->setRowCount(cnt + 1);
            ui->sentenceWidget->setItem(cnt, 0, new QTableWidgetItem(analysisString));

            QString str = "";
            for (auto pair: sentence) {
                str += pair.first;
                str += " ";
            }
            ui->sentenceWidget->setItem(cnt, 1, new QTableWidgetItem(str));
            cnt++;
        };

        // 语法树准备
        SyntaxTree* tree,* syntaxTree;
        QQueue<SyntaxNode*> analysisTreeStack;    // 分析树结点栈
        QQueue<SyntaxNode*> syntaxTreeStack;      // 语法树结点栈

        // 中间代码准备
        IntermediateCode intermediateCode;  // 中间代码生成器
        QQueue<CodeCache*> interStack;      // 中间代码生成栈
        int tmpCnt = 0;

        // 语法分析
        printAnalysis();
        while (true) {
            StkItem stkItem = analysisStack.top();
            if (stkItem.kind == 0) {    // 栈顶取出的必须是状态，而不能是符号
                QMessageBox::warning(this, "警告", "待分析的句子与文法不匹配", QMessageBox::Yes);
                return;
            }

            AnalysisTableItem analysisTableItem;
            if (!this->lalr1AnalysisTable.tb[stkItem.state].contains(sentence.front().first) &&         // token和类型都匹配不到
                    !this->lalr1AnalysisTable.tb[stkItem.state].contains(sentence.front().second)) {
                sentence.push_front({"@", "empty"});
                analysisTableItem = lalr1AnalysisTable.tb[stkItem.state][sentence.front().first];
            } else if (!this->lalr1AnalysisTable.tb[stkItem.state].contains(sentence.front().first)) {  // 匹配不到字符，只能匹配类型（如，identify类型和number类型等
                analysisTableItem = lalr1AnalysisTable.tb[stkItem.state][sentence.front().second];
            } else {
                analysisTableItem = lalr1AnalysisTable.tb[stkItem.state][sentence.front().first];
            }

            if (analysisTableItem.kind == 1) {      // 移进

                StkItem chStkItem;
                chStkItem.kind = 0;
                if (!this->lalr1AnalysisTable.tb[stkItem.state].contains(sentence.front().first)) {  // 匹配不到字符，只能匹配类型（如，identify类型和number类型等
                    chStkItem.str = sentence.front().second;
                    chStkItem.detail = sentence.front().first;
                } else {
                    chStkItem.str = sentence.front().first;
                    chStkItem.detail = sentence.front().second;
                }

                SyntaxNode* node = new SyntaxNode(sentence.front().first, sentence.front().second);
                analysisTreeStack.push_back(node);

                // 进行语法树分析
                if (!syntaxAction.empty()) {
                    SyntaxNode* syntaxNode = new SyntaxNode(sentence.front().first, sentence.front().second);
                    syntaxTreeStack.push_back(syntaxNode);
                }

                // 进行中间代码分析
                if (!interAction.empty()) {
                    CodeCache* codeCache = new CodeCache();
                    codeCache->val = sentence.front().first;
                    interStack.push_back(codeCache);
//                    qDebug() << codeCache->val;
                }

                sentence.pop_front();

                StkItem stStkItem;
                stStkItem.kind = 1;
                stStkItem.state = analysisTableItem.idx;

                analysisStack.push(chStkItem);
                analysisStack.push(stStkItem);
                printAnalysis();

            } else if (analysisTableItem.kind == 2) {   // 规约

                // 获取规约规则
                Item recursionItem = lalr1AnalysisTable.recursion[analysisTableItem.idx];

                // 删除规约右侧元素
                int popCnt = recursionItem.pos * 2;
                while (!analysisStack.empty() && popCnt) {
                    analysisStack.pop();
                    popCnt--;
                }
                if (popCnt || analysisStack.empty()) {
                    QMessageBox::warning(this, "警告", "待分析的句子与文法不匹配", QMessageBox::Yes);
                    return;
                }

                // 添加规约左侧元素
                StkItem nextStkItem = analysisStack.top();
                if (nextStkItem.kind != 1) {
                    QMessageBox::warning(this, "警告", "待分析的句子与文法不匹配", QMessageBox::Yes);
                    return;
                }

                if (!this->lalr1AnalysisTable.tb[nextStkItem.state].contains(recursionItem.name)) {
                    sentence.push_front({"@", "empty"});
                }


                QStack<SyntaxNode*> tmpNodeStk; // 分析树结点栈
                QQueue<SyntaxNode*> tmpNodeQ;   // 语法树结点队列
                QQueue<CodeCache*> tmpCacheQ;   // 中间代码缓存队列
                QString recursionString = recursionItem.name + " ->";

                for (auto item: recursionItem.rule) {
                    recursionString += " " + item;
                }

                QVector<int> actions;
                if (!syntaxAction.empty()) actions = this->syntaxAction[recursionString];
                QVector<QStringList> interActions;
                if (!interAction.empty()) interActions = this->interAction[recursionString];

                for (int i = 0; i < recursionItem.pos; i++) {
                    tmpNodeStk.push(analysisTreeStack.back());
                    if (!syntaxAction.empty()) tmpNodeQ.push_front(syntaxTreeStack.back());
                    if (!interAction.empty()) tmpCacheQ.push_front(interStack.back());
                    analysisTreeStack.pop_back();
                    if (!syntaxAction.empty()) syntaxTreeStack.pop_back();
                    if (!interAction.empty()) interStack.pop_back();
                }

                // 分析树结点
                SyntaxNode* node = new SyntaxNode(recursionItem.name);
                while (!tmpNodeStk.empty()) {
                    node->children.push_back(tmpNodeStk.top());
                    tmpNodeStk.pop();
                }
                analysisTreeStack.push_back(node);

                // 语法树结点
                if (!syntaxAction.empty()) {
                    SyntaxNode* syntaxNode;
                    for (int i = 0; i < actions.size(); i++) {
                        if (actions[i] == 1) {
                            syntaxNode = tmpNodeQ[i];
                            break;
                        }
                    }
                    if (syntaxNode) {
                        for (int i = 0; i < actions.size(); i++) {
                            if (actions[i] == 2) {
                                syntaxNode->children.push_back(tmpNodeQ[i]);
                            }
                        }
                        for (int i = 0; i < actions.size(); i++) {
                            if (actions[i] == 3) {
                                for (auto bro: tmpNodeQ[i]->brother) {
                                    syntaxNode->brother.push_back(bro);
                                }
                                tmpNodeQ[i]->brother.clear();
                                syntaxNode->brother.push_back(tmpNodeQ[i]);
                            }
                        }
                        syntaxTreeStack.push_back(syntaxNode);
                    } else {
                        QMessageBox::warning(this, "警告", "语法树寄了", QMessageBox::Yes);
                    }
                }

                // 中间代码构造
//                qDebug() << recursionString;
//                for (auto codeCache: tmpCacheQ) {
//                    qDebug() << codeCache->val << " " << codeCache->TC << " " << codeCache->FC << " " << codeCache->Chain << " " << codeCache->Head;
//                }
                if (!interAction.empty()) {
                    bool tag = false;
                    CodeCache* interCache = new CodeCache();
                    for (int i = 0; i < interActions.size(); i++) {
                        QStringList action = interActions[i];
//                        qDebug() << action;
                        if (action[0] == "0") { // NOTE: 赋值
                            QString tarObj = action[1], srcObj = action[2];
                            // 获取源内容
                            int idx = srcObj[0].toLatin1() - '0';
                            QString src = "";
                            switch (srcObj[1].toLatin1()) {
                            case 'N':
                                src += QString::number(intermediateCode.NextStat());
                                break;
                            case 'P':
                                src += "T" + QString::number(tmpCnt);
                                tag = true;
                                break;
                            case 'C':
                                if (idx == 0) src += QString::number(interCache->Chain);
                                else src += QString::number(tmpCacheQ[idx - 1]->Chain);
                                break;
                            case 'H':
                                if (idx == 0) src += QString::number(interCache->Head);
                                else src += QString::number(tmpCacheQ[idx - 1]->Head);
                                break;
                            case 'T':
                                if (idx == 0) src += QString::number(interCache->TC);
                                else src += QString::number(tmpCacheQ[idx - 1]->TC);
                                break;
                            case 'F':
                                if (idx == 0) src += QString::number(interCache->FC);
                                else src += QString::number(tmpCacheQ[idx - 1]->FC);
                                break;
                            case 'V':
                                if (idx == 0) src += interCache->val;
                                else src += tmpCacheQ[idx - 1]->val;
                                break;
                            }
                            // 获取目标位置内容
                            idx = tarObj[0].toLatin1() - '0';
                            switch (tarObj[1].toLatin1()) {
                            case 'C':
                                if (idx == 0) interCache->Chain = src.toInt();
                                else tmpCacheQ[idx - 1]->Chain = src.toInt();
                                break;
                            case 'H':
                                if (idx == 0) interCache->Head = src.toInt();
                                else tmpCacheQ[idx - 1]->Head = src.toInt();
                                break;
                            case 'T':
                                if (idx == 0) interCache->TC = src.toInt();
                                else tmpCacheQ[idx - 1]->TC = src.toInt();
                                break;
                            case 'F':
                                if (idx == 0) interCache->FC = src.toInt();
                                else tmpCacheQ[idx - 1]->FC = src.toInt();
                                break;
                            case 'V':
                                if (idx == 0) interCache->val = src;
                                else tmpCacheQ[idx - 1]->val = src;
                                break;
                            }
                        } else if (action[0] == "1") {      // NOTE: GEN
                            QVector<QString> genV;
                            for (int i = 1; i <= 4; i++) {
                                QString obj = action[i];
                                QString s = "";
                                if (obj[0] == "J") {
                                    s += "J";
                                    if (obj.size() <= 1) {
                                        genV.push_back(s);
                                        continue;
                                    }
                                    obj.remove(0, 1);
                                }
                                if (obj.size() <= 1) {
                                    genV.push_back("_");
                                    continue;
                                }
                                int idx = obj[0].toLatin1() - '0';
                                char type = obj[1].toLatin1();
                                switch (type) {
                                case 'N':
                                    s += QString::number(intermediateCode.NextStat());
                                    break;
                                case 'P':
                                    s += "T" + QString::number(tmpCnt);
                                    tag = true;
                                    break;
                                case 'C':
                                    if (idx == 0) s += QString::number(interCache->Chain);
                                    else s += QString::number(tmpCacheQ[idx - 1]->Chain);
                                    break;
                                case 'H':
                                    if (idx == 0) s += QString::number(interCache->Head);
                                    else s += QString::number(tmpCacheQ[idx - 1]->Head);
                                    break;
                                case 'T':
                                    if (idx == 0) s += QString::number(interCache->TC);
                                    else s += QString::number(tmpCacheQ[idx - 1]->TC);
                                    break;
                                case 'F':
                                    if (idx == 0) s += QString::number(interCache->FC);
                                    else s += QString::number(tmpCacheQ[idx - 1]->FC);
                                    break;
                                case 'V':
                                    if (idx == 0) s += interCache->val;
                                    else s += tmpCacheQ[idx - 1]->val;
                                    break;
                                }
                                genV.push_back(s);
                            }
                            intermediateCode.GEN(genV[0], genV[1], genV[2], genV[3]);
                        } else if (action[0] == "2") {  // NOTE: Merge
                            QString aObj = action[1], bObj = action[2], cObj = action[3];
                            int b, c;
                            int idx = bObj[0].toLatin1() - '0';
                            switch (bObj[1].toLatin1()) {
                            case 'N':
                                b = intermediateCode.NextStat();
                                break;
                            case 'C':
                                if (idx == 0) b = interCache->Chain;
                                else b = tmpCacheQ[idx - 1]->Chain;
                                break;
                            case 'H':
                                if (idx == 0) b = interCache->Head;
                                else b = tmpCacheQ[idx - 1]->Head;
                                break;
                            case 'T':
                                if (idx == 0) b = interCache->TC;
                                else b = tmpCacheQ[idx - 1]->TC;
                                break;
                            case 'F':
                                if (idx == 0) b = interCache->FC;
                                else b = tmpCacheQ[idx - 1]->FC;
                                break;
                            case 'V':
                                if (idx == 0) b = interCache->val.toInt();
                                else b = tmpCacheQ[idx - 1]->val.toInt();
                                break;
                            }
                            idx = cObj[0].toLatin1() - '0';
                            switch (cObj[1].toLatin1()) {
                            case 'N':
                                c = intermediateCode.NextStat();
                                break;
                            case 'C':
                                if (idx == 0) c = interCache->Chain;
                                else c = tmpCacheQ[idx - 1]->Chain;
                                break;
                            case 'H':
                                if (idx == 0) c = interCache->Head;
                                else c = tmpCacheQ[idx - 1]->Head;
                                break;
                            case 'T':
                                if (idx == 0) c = interCache->TC;
                                else c = tmpCacheQ[idx - 1]->TC;
                                break;
                            case 'F':
                                if (idx == 0) c = interCache->FC;
                                else c = tmpCacheQ[idx - 1]->FC;
                                break;
                            case 'V':
                                if (idx == 0) c = interCache->val.toInt();
                                else c = tmpCacheQ[idx - 1]->val.toInt();
                                break;
                            }
                            idx = aObj[0].toLatin1() - '0';
                            switch (aObj[1].toLatin1()) {
                            case 'C':
                                if (idx == 0) interCache->Chain = intermediateCode.Merge(b, c);
                                else tmpCacheQ[idx - 1]->Chain = intermediateCode.Merge(b, c);
                                break;
                            case 'H':
                                if (idx == 0) interCache->Head = intermediateCode.Merge(b, c);
                                else tmpCacheQ[idx - 1]->Head = intermediateCode.Merge(b, c);
                                break;
                            case 'T':
                                if (idx == 0) interCache->TC = intermediateCode.Merge(b, c);
                                else tmpCacheQ[idx - 1]->TC = intermediateCode.Merge(b, c);
                                break;
                            case 'F':
                                if (idx == 0) interCache->FC = intermediateCode.Merge(b, c);
                                else tmpCacheQ[idx - 1]->FC = intermediateCode.Merge(b, c);
                                break;
                            case 'V':
                                if (idx == 0) interCache->val = QString::number(intermediateCode.Merge(b, c));
                                else tmpCacheQ[idx - 1]->val = QString::number(intermediateCode.Merge(b, c));
                                break;
                            }
                        } else if (action[0] == "3") {  // NOTE: BackPatch
                            QString srcObj = action[1], tarObj = action[2];
                            // 获取源内容
                            int idx = srcObj[0].toLatin1() - '0';
                            int src;
                            switch (srcObj[1].toLatin1()) {
                            case 'N':
                                src = intermediateCode.NextStat();
                                break;
                            case 'C':
                                if (idx == 0) src = interCache->Chain;
                                else src = tmpCacheQ[idx - 1]->Chain;
                                break;
                            case 'H':
                                if (idx == 0) src = interCache->Head;
                                else src = tmpCacheQ[idx - 1]->Head;
                                break;
                            case 'T':
                                if (idx == 0) src = interCache->TC;
                                else src = tmpCacheQ[idx - 1]->TC;
                                break;
                            case 'F':
                                if (idx == 0) src = interCache->FC;
                                else src = tmpCacheQ[idx - 1]->FC;
                                break;
                            case 'V':
                                if (idx == 0) src = interCache->val.toInt();
                                else src = tmpCacheQ[idx - 1]->val.toInt();
                                break;
                            }
                            idx = tarObj[0].toLatin1() - '0';
                            int tar;
                            switch (tarObj[1].toLatin1()) {
                            case 'N':
                                tar = intermediateCode.NextStat();
                                break;
                            case 'C':
                                if (idx == 0) tar = interCache->Chain;
                                else tar = tmpCacheQ[idx - 1]->Chain;
                                break;
                            case 'H':
                                if (idx == 0) tar = interCache->Head;
                                else tar = tmpCacheQ[idx - 1]->Head;
                                break;
                            case 'T':
                                if (idx == 0) tar = interCache->TC;
                                else tar = tmpCacheQ[idx - 1]->TC;
                                break;
                            case 'F':
                                if (idx == 0) tar = interCache->FC;
                                else tar = tmpCacheQ[idx - 1]->FC;
                                break;
                            case 'V':
                                if (idx == 0) tar = interCache->val.toInt();
                                else tar = tmpCacheQ[idx - 1]->val.toInt();
                                break;
                            }
                            intermediateCode.BackPatch(src, tar);
                        }
                    }
//                    qDebug() << interCache->val;
                    interStack.push_back(interCache);
                    if (tag) tmpCnt++;
                }

                StkItem chStkItem, stStkItem;
                chStkItem.kind = 0;
                chStkItem.str = recursionItem.name;
                chStkItem.detail = "";
                stStkItem.kind = 1;
                stStkItem.state = this->lalr1AnalysisTable.tb[nextStkItem.state][recursionItem.name].idx;
                analysisStack.push(chStkItem);
                analysisStack.push(stStkItem);
                printAnalysis();

            } else if (analysisTableItem.kind == 3) {   // 非终结符移进
                QMessageBox::warning(this, "警告", "待分析的句子与文法不匹配", QMessageBox::Yes);
                return;
            } else if (analysisTableItem.kind == 4) {   // 接受态
                if (sentence.front().first == "$") {
                    QMessageBox::information(this, "提醒", "分析完毕，该单词编码文件属于该文法的句子", QMessageBox::Yes);
                    break;
                } else {
                    QMessageBox::warning(this, "警告", "待分析的句子与文法不匹配", QMessageBox::Yes);
                    return;
                }
            } else {
                QMessageBox::warning(this, "警告", "待分析的句子与文法不匹配", QMessageBox::Yes);
                return;
            }
        }

        // 构建分析树
        ui->treeWidget->clear();
        ui->treeWidget->setColumnCount(1);
        tree = new SyntaxTree(new SyntaxNode("program"));
        for (auto node: analysisTreeStack) {
            tree->root->children.push_back(node);
        }
        QTreeWidgetItem* topItem = new QTreeWidgetItem(QStringList() << "start");
        ui->treeWidget->addTopLevelItem(topItem);
        tree->showAnalysis(topItem);

        ui->treeWidget->expandAll();

        // 构建语法树
        if (!syntaxAction.empty()) {
            ui->syntaxTreeWidget->clear();
            ui->syntaxTreeWidget->setColumnCount(1);
            syntaxTree = new SyntaxTree(new SyntaxNode("program"));
            for (auto node: syntaxTreeStack) {
                syntaxTree->root->children.push_back(node);
            }
            QTreeWidgetItem* topItem2 = new QTreeWidgetItem(QStringList() << "start");
            ui->syntaxTreeWidget->addTopLevelItem(topItem2);
            syntaxTree->showSyntax(topItem2);

            ui->syntaxTreeWidget->expandAll();
        }

        // 构造中间代码
        if (!interAction.empty()) {
            intermediateCode.BackPatch(interStack.back()->Chain, intermediateCode.NextStat());
            QString code = intermediateCode.toIntermediateCode();
            ui->interCodeBrowser->setText(code);
        }

    });

    // 保存中间代码
    connect(ui->saveButton, &QPushButton::clicked, this, [&]() {
        QString fileName = QString("intermediate_code") + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()) + QString(".txt");
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << ui->interCodeBrowser->toPlainText();
            file.close();
            QMessageBox::information(this, "提示", "文件保存为：" + fileName + "成功！", QMessageBox::Yes);
        } else {
            QMessageBox::warning(this, "提示", "文件保存失败！", QMessageBox::Yes);
        }
    });

    // 上传语法树语义动作
    connect(ui->uploadSyntaxButton, &QPushButton::clicked, this, [&]() {
       QString fileName = QFileDialog::getOpenFileName(this, "选择语法树语义动作文件");
       syntaxAction.clear();
       if (!fileName.isEmpty()) {
           QFile file(fileName);
           if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
               QTextStream in(&file);
               in.setCodec("UTF-8"); // 设置编码格式为 UTF-8
               QString all = in.readAll();
               ui->syntaxTextBrowser->setText(all);
               QStringList allText = all.split('\n');
               for (int i = 0; i < allText.size(); i++) {
                   QString key, value;
                   while (i < allText.size() && allText[i] == "") i++;
                   if (i < allText.size()) {
                       key = allText[i];
                   } else break;
                   i++;
                   while (i < allText.size() && allText[i] == "") i++;
                   if (i < allText.size()) {
                       value = allText[i];
                       QVector<int> nums;
                       QStringList numStrs = value.split(' ');
                       for (auto numStr: numStrs) {
                           bool isNum;
                           int num = numStr.toInt(&isNum);
                           if (isNum) nums.push_back(num);
                       }
                       syntaxAction.insert(key, nums);
                   } else break;
               }
               file.close();
           }
       }
    });

    // 上传中间代码语义动作
    connect(ui->uploadInterButton, &QPushButton::clicked, this, [&]() {
        QString fileName = QFileDialog::getOpenFileName(this, "选择中间代码语义动作文件");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                in.setCodec("UTF-8"); // 设置编码格式为 UTF-8
                QString allText = in.readAll();
                ui->innerTextBrowser->setText(allText);
                // 解析语义动作
                QStringList textActions = allText.split("---\n");
                for (auto& textAction: textActions) {
                    if (textAction[textAction.size() - 1] == '\n') textAction.remove(textAction.size() - 1, 1);
                }
                for (auto textAction: textActions) {
                    QStringList lines = textAction.split('\n');
                    QString key = lines[0];
                    for (int i = 1; i < lines.size(); i++) {
                        QStringList word = lines[i].split(' ');
                        this->interAction[key].push_back(word);
                    }
                }
                file.close();
            }
        }
    });
}

TaskTwoWidget::~TaskTwoWidget() {
    delete ui;
}

/*!
    @name   getFirst
    @brief  求 first 集合
    @param
    @return
    @attention
*/
void TaskTwoWidget::getFirst() {
    ui->firstTableWidget->clear();
    ui->firstTableWidget->setHorizontalHeaderLabels(QStringList());
    ui->firstTableWidget->setRowCount(0);
    ui->firstTableWidget->setColumnCount(0);

    ui->firstTableWidget->setRowCount(this->nonFinalizers.size());
    ui->firstTableWidget->setColumnCount(2);

    ui->firstTableWidget->setHorizontalHeaderLabels(QStringList() << "非终结符" << "First 集合");

    // 计算 first 集合
    QHash<QString, QSet<QString>> tmpSet;
    QSet<QString> epsilonSet = {"@"};
    while (tmpSet.empty() || tmpSet != this->firstSet) {
        tmpSet = this->firstSet;
        for (int i = 0; i < this->nonFinalizers.size(); i++) {
            QString s = this->nonFinalizers[i];
            for (auto grammar: this->grammars[s]) {
                int k = 0;
                while (k < grammar.size()) {
                    if (!this->nonFinalizers.contains(grammar[k])) this->firstSet[grammar[k]].insert(grammar[k]);
                    bool mark = false;
                    if (this->firstSet[grammar[k]].contains("@")) mark = true;
                    QSet<QString> diff = this->firstSet[grammar[k]].subtract(epsilonSet);
                    this->firstSet[s] = this->firstSet[s].unite(diff);
                    if (!mark) break;
                    else this->firstSet[grammar[k]].insert("@");
                    k++;
                }
                if (k == grammar.size()) this->firstSet[s].insert("@");
            }
        }
    }

    // 渲染 first 集合
    for (int i = 0; i < nonFinalizers.size(); i++) {
        ui->firstTableWidget->setItem(i, 0, new QTableWidgetItem(nonFinalizers[i]));
        QString setStr = "{";
        for (auto item: firstSet[nonFinalizers[i]]) {
            setStr += item;
            setStr += ",";
        }
        setStr = setStr.left(setStr.size() - 1);
        setStr += "}";
        ui->firstTableWidget->setItem(i, 1, new QTableWidgetItem(setStr));
    }

    ui->firstTableWidget->resizeColumnsToContents();
    ui->firstTableWidget->resizeRowsToContents();
}

/*!
    @name   getFollow
    @brief  求 follow 集合
    @param
    @return
    @attention
*/
void TaskTwoWidget::getFollow() {
    ui->followTableWidget->clear();
    ui->followTableWidget->setHorizontalHeaderLabels(QStringList());
    ui->followTableWidget->setRowCount(0);
    ui->followTableWidget->setColumnCount(0);

    ui->followTableWidget->setRowCount(this->nonFinalizers.size());
    ui->followTableWidget->setColumnCount(2);

    ui->followTableWidget->setHorizontalHeaderLabels(QStringList() << "非终结符" << "Follow 集合");

    // 计算 follow 集合
    bool flag = true;
    QSet<QString> epsilonSet = {"@"};
    followSet[startString].insert("$");
    while (flag) {
        flag = false;
        for (auto key: grammars.keys()) {    // 获取每个非终结符
            for (auto grammar: grammars[key]) {    // 获取每个文法规则
                for (int i = 0; i < grammar.size(); i++) {    // 遍历每一个字符
                    if (!nonFinalizers.contains(grammar[i])) continue;    // 终结符直接跳过
                    int k = i + 1;
                    QSet<QString> tmpSet;    // 求first(X_{i+1}…X_{n})
                    while (k < grammar.size()) {
                        bool mark = false;
                        if (this->firstSet[grammar[k]].contains("@")) mark = true;
                        QSet<QString> diff = this->firstSet[grammar[k]].subtract(epsilonSet);
                        tmpSet = tmpSet.unite(diff);
                        if (!mark) break;
                        else this->firstSet[grammar[k]].insert("@");
                        k++;
                    }
                    for (QString tmp: tmpSet) {    // 将first(X_{i+1}…X_{n}) - {epsilon}添加到follow(X_{i})
                        if (!followSet[grammar[i]].contains(tmp)) {
                            followSet[grammar[i]].insert(tmp);
                            flag = true;
                        }
                    }
                    if (k == grammar.size()) {    // first(X_{i+1}…X_{n})包含@，要将follow(A)加入follow(X_{i})
                        for (QString tmp: followSet[key]) {
                            if (!followSet[grammar[i]].contains(tmp)) {
                                followSet[grammar[i]].insert(tmp);
                                flag = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // 渲染 follow 集合
    for (int i = 0; i < nonFinalizers.size(); i++) {
        ui->followTableWidget->setItem(i, 0, new QTableWidgetItem(nonFinalizers[i]));
        QString setStr = "{";
        for (auto item: followSet[nonFinalizers[i]]) {
            setStr += item;
            setStr += ",";
        }
        setStr = setStr.left(setStr.size() - 1);
        setStr += "}";
        ui->followTableWidget->setItem(i, 1, new QTableWidgetItem(setStr));
    }

    ui->followTableWidget->resizeColumnsToContents();
    ui->followTableWidget->resizeRowsToContents();
}

/*!
    @name   showLR1
    @brief  展示 LR1
    @param  lr1, 初始状态
    @return
    @attention
*/
void TaskTwoWidget::showLR1(LR lr1, State firstState) {
    ui->lr1TableWidget->clear();
    ui->lr1TableWidget->setHorizontalHeaderLabels(QStringList());
    ui->lr1TableWidget->setRowCount(0);
    ui->lr1TableWidget->setColumnCount(0);

    ui->lr1TableWidget->setRowCount(lr1.size);
    QSet<QString> lrChangeState;
    for (auto state: lr1.stateHash.keys()) {
        for (auto changeMethod: lr1.changeHash[lr1.stateHash[state]].keys()) {
            lrChangeState.insert(changeMethod);
        }
    }
    ui->lr1TableWidget->setColumnCount(lrChangeState.size());
    auto lrChangeList = lrChangeState.toList();

    ui->lr1TableWidget->setHorizontalHeaderLabels(lrChangeList);
    for (auto state: lr1.stateHash.keys()) {
        int lrIdx = lr1.stateHash[state];
        QString itemString = QString::number(lrIdx) + ":\n";
        for (auto item: state.st) {
            itemString += item.name;
            itemString += " -> ";
            for (int i = 0; i < item.rule.size(); i++) {
                if (i == item.pos) itemString += ". ";
                itemString += item.rule[i] + " ";
            }
            if (item.pos == item.rule.size()) itemString += ". ";
            itemString += ", {";
            int i = 0;
            for (auto nextCh: item.next) {
                itemString += nextCh;
                if (i != item.next.size() - 1) itemString += "、";
                i++;
            }
            itemString += "}\n";
        }
        QTableWidgetItem *item = new QTableWidgetItem(itemString);
        if (state == firstState) item->setTextColor(QColor(255, 0, 0));
        ui->lr1TableWidget->setVerticalHeaderItem(lrIdx, item);
        for (int i = 0; i < lrChangeList.size(); i++) {
            if (lr1.changeHash[lr1.stateHash[state]].contains(lrChangeList[i])) {
                ui->lr1TableWidget->setItem(lrIdx, i, new QTableWidgetItem(QString::number(lr1.changeHash[lr1.stateHash[state]][lrChangeList[i]])));
            }
        }
    }
    ui->lr1TableWidget->resizeRowsToContents();
    ui->lr1TableWidget->resizeColumnsToContents();
}

/*!
    @name   showLALR1
    @brief  展示LALR1
    @param  lalr1, 初始状态
    @return
    @attention
*/
void TaskTwoWidget::showLALR1(LR lalr1, State firstState) {
    ui->lalr1TableWidget->clear();
    ui->lalr1TableWidget->setHorizontalHeaderLabels(QStringList());
    ui->lalr1TableWidget->setRowCount(0);
    ui->lalr1TableWidget->setColumnCount(0);

    ui->lalr1TableWidget->setRowCount(lalr1.size);
    QSet<QString> lalrChangeState;
    for (auto state: lalr1.stateHash.keys()) {
       for (auto changeMethod: lalr1.changeHash[lalr1.stateHash[state]].keys()) {
           lalrChangeState.insert(changeMethod);
       }
    }
    ui->lalr1TableWidget->setColumnCount(lalrChangeState.size());
    auto lalrChangeList = lalrChangeState.toList();

    ui->lalr1TableWidget->setHorizontalHeaderLabels(lalrChangeList);
    for (auto state: lalr1.stateHash.keys()) {
       int lalrIdx = lalr1.stateHash[state];
       QString itemString = QString::number(lalrIdx) + ":\n";
       for (auto item: state.st) {
           itemString += item.name;
           itemString += " -> ";
           for (int i = 0; i < item.rule.size(); i++) {
               if (i == item.pos) itemString += ". ";
               itemString += item.rule[i] + " ";
           }
           if (item.pos == item.rule.size()) itemString += ". ";
           itemString += ", {";
           int i = 0;
           for (auto nextCh: item.next) {
               itemString += nextCh;
               if (i != item.next.size() - 1) itemString += "、";
               i++;
           }
           itemString += "}\n";
       }
       QTableWidgetItem *item = new QTableWidgetItem(itemString);
       if (State::haveSameCore(state, firstState)) item->setTextColor(QColor(255, 0, 0));
       ui->lalr1TableWidget->setVerticalHeaderItem(lalrIdx, item);
       for (int i = 0; i < lalrChangeList.size(); i++) {
           if (lalr1.changeHash[lalr1.stateHash[state]].contains(lalrChangeList[i])) {
               ui->lalr1TableWidget->setItem(lalrIdx, i, new QTableWidgetItem(QString::number(lalr1.changeHash[lalr1.stateHash[state]][lalrChangeList[i]])));
           }
       }
    }

    ui->lalr1TableWidget->resizeRowsToContents();
    ui->lalr1TableWidget->resizeColumnsToContents();
}
