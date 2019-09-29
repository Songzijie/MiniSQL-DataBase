/*
 * by Zijie Song
 */

#ifndef RECORD_MANAGER_H
#define RECORD_MANAGER_H

#include <iostream>
#include <iomanip>
#include <map>
#include "definitions.h"
#include "StmtAfterParse.h"
#include "BufferManager.h"

using namespace std;

class RecordManager {
private:
	//调用bufferManager实现对文件和block的操作
	BufferManager &bufferManager = BufferManager::GetInstance();
	
	//获得该属性集对应的每条记录的定长
	int getRecordlength(vector<Attr> &attrs);
	//判断指定文件指定位置对应的记录是否为空
	bool Isempty(string fileName, vector<Attr> &attrs, int num, int offset);
	//判断指定文件指定位置对应的记录是否符合筛选条件
	bool Issatisfied(string fileName, vector<Condition> &conditions, vector<Attr> &attrs, int num, int offset);
	//命令行输出指定文件指定位置对应的记录的对应列
	void Output(string fileName, vector<Attr> &attrs, vector<int> &selectColumns, int num, int offset);
	//删除指定文件指定位置对应的记录
	void DeleteRecord(string fileName, vector<Attr> &attrs, int num, int offset);
	//获得当前可插入记录的位置
	IndexInfo getPosition(string fileName, vector<Attr> &attrs, vector<AttrWithValue> &AttrValues);

	void drawLine(int columnNum,bool firstLine=false);

public:
    RecordManager();//构造函数
	~RecordManager();//析构函数

    /**
     * select返回指定列的所有记录
     * @param tableName 表名
     * @param attrs 表的全部属性名
     * @param column 需要返回的列号
     * @return column列的所有记录
     */
    vector<AttrWithValue> selectAttr(string tableName, vector<Attr> &attrs, int column);

	/**
     * 打印select结果并且返回select到的记录数
     * @param tableName 表名
     * @param conditions 所有筛选情况
     * @param attrs 表的全部属性名
     * @param selectColumns 需要打印的所有列序号
     * @param indexes 表的全部索引info
     * @return 记录数
     */
    int select(string tableName, vector<Condition> &conditions, vector<Attr> &attrs, vector<int> &selectColumns, vector<IndexInfo> &indexes);

	/**
	 * 根据表名取得indexinfos
	 * @param tableName 表名
	 * @return 全部记录（indexinfo形式）
	 */
	vector<IndexInfo> selectIndexInfo(string tableName);

	/**
	* 插入指定记录并返回插入位置的索引info
	* @param tableName 表名
	* @param attrs 表的全部属性名
	* @param AttrValues 待插入记录的各条属性值
	* @return 插入位置的索引info
	*/
    IndexInfo insert(string tableName, vector<Attr> &attrs, vector<AttrWithValue> &AttrValues);

	/**
	* 删除符合条件记录并返回这些记录
	* @param tableName 表名
	* @param conditions 所有筛选情况
	* @param attrs 表的全部属性名
	* @param indexes 表的全部索引info
	* @return 删除的记录
	*/
    vector<vector<AttrWithValue> >
    deleteTuple(string tableName, vector<Condition> &conditions, vector<Attr> &attrs, vector<IndexInfo> &indexes);

};

#endif
