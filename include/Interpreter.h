/*
 * by Zijie Song
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "definitions.h"
#include "API.h"
#include "CatalogManager.h"
#include "StmtAfterParse.h"
#include <math.h>
#include <time.h>
#include <typeinfo>
#include <fstream>

using namespace std;

class Interpreter {
public:
    Interpreter() : top(0), api(catalog), wordtop(0) {};

    /**
     * 读入指令，每次调用时输出 miniSQL-> 然后接受一行输入
     * @param isCmd true:命令行输入 false:文件输入
     */
    void readCmd(bool isCmd, ifstream &fin);

/**
 * 循环调用readCommand读入command，解析得到的interface，调用api执行语句
 */
    void pipeline(bool isCmd = true, string fileName = "");

/**
 * 将statement翻译成interface class数据类型
 * @return
 */
    StmtAfterParse parse();

private:
    CatalogManager &catalog = CatalogManager::getInstance();
    API api;
    string statement[100], word[100];
    int top, wordtop;

    void copy(string *array, int top);

    void Initialize(string *array);

    void InitializeData(UnionValue &v);

    void InitializeCondition(Condition &c);

    void InitializeAttribute(Attr &a);

    UnionValue StringtoValue(DataType type, string &x);

    DataType WordType(string &a);

    int StringtoInt(string &word);

    float StringtoFloat(string &word);

    bool TypeMatch(DataType type1, DataType type2);

};

#endif