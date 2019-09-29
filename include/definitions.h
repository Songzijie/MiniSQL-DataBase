/*
 * by Zijie Song
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include<string>

using namespace std;

enum DataType {
    MINI_FLOAT, MINI_INT, MINI_STRING
};

enum ActionType {
    MINI_CREATE, MINI_DELETE, MINI_SELECT, MINI_DROP, MINI_INSERT,MINI_EXEC
};

enum ObjectType {
    MINI_TABLE, MINI_INDEX, MINI_CATALOG
};

enum OpType {
	EQU, NE, LT, GT, LE, GE, NO
};

struct IndexInfo {//某一条记录的信息
    int blockNum;//所处的块序号
    int offset; //所处块内的位置
};

struct UnionValue {
	bool isNull = false;
	string s = "";
	int n = 0;
	float f = 0.0;
};

struct AttrWithValue {
    bool isPrimary = 0, isUnique = 0, isNull = 0;
    int length;
    DataType type;
    UnionValue value;

    AttrWithValue() : isPrimary(0), isUnique(0), isNull(0), length(0), type(MINI_FLOAT), value() {}
};

struct Attr {
	bool isPrimary, isUnique, hasIndex;
    int length, offset;
    DataType type;
    string attrName;

	Attr() : isPrimary(0), isUnique(0), hasIndex(0), length(0), type(MINI_FLOAT), offset(0), attrName("") {}
};


struct Condition {
    int id = -1;
    DataType type = MINI_FLOAT;
    OpType op = NO; //0: =; 1: <>; 2: <; 3: >; 4: <=; 5: >=;6:不存在;
    UnionValue value;
};


#endif