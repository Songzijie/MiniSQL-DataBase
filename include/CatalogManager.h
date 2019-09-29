/*
 * by Zijie Song
 */

#ifndef CATALOG_MANAGER_H
#define CATALOG_MANAGER_H

#include <string>
#include <list>
#include <iostream>
#include "definitions.h"
#include <vector>
#include "BufferManager.h"

#define CATALOG_FILE_NAME "catalog.catalog"
#define MaxCharLength 300
#define TOINT *(int*)
#define TOBOOL *(bool*)
#define TODATATYPE *(DataType*)
#define SIZE_INT 4
#define SIZE_BOOL 1
#define SIZE_DATATYPE 4

class IndexNode {
public:
    IndexNode() {}

    IndexNode(std::string tableName, std::string indexName, std::string attrName, int column) {
        this->tableName = tableName;
        this->indexName = indexName;
        this->attribute = attrName;
        this->column = column;
    }

    std::string tableName;
    std::string attribute;
    std::string indexName;
    int column;
};

class TableNode {
public:
    TableNode() : recordNum(0) {}

    TableNode(std::string tableName, list <Attr> attrList, int recordNum) {
        this->tableName = tableName;
        this->attrList = attrList;
        this->recordNum = recordNum;
    }

    string tableName;
    list <Attr> attrList;
    int recordNum;
};

class CatalogManager {
private:
    list <TableNode> tableList;
    list <IndexNode> indexList;

    TableNode &findTableNodeByName(string tableName, bool &ifExist);

    IndexNode &findIndexNodeByName(string indexName, bool &ifExist);

    bool findAttrByName(TableNode &table, string attrName, Attr &resultAttr);

    //BufferManager &buffer = BufferManager::GetInstance();

    //构造函数将硬盘中的catalog文件读入内存
    CatalogManager();

    //私有化拷贝构造函数和赋值构造函数实现单例模式
    CatalogManager(const CatalogManager &) {};

    CatalogManager &operator=(const CatalogManager &) {};

    //析构函数将内存中的catalog存入硬盘
    ~CatalogManager();

public:

    void readCatalog();

    void writeCatalog();

    // 1/true 成功, 0/false失败
    // -1 不存在

    //table
    bool createTable(string tableName, list <Attr> attrList);

    bool existTable(string tableName);

    bool deleteTable(string tableName);

    void showTables();//打印tables
    void showIndexes();//打印indexes
    TableNode findTable(string tableName);


    //index
    bool createIndex(string indexName, string tableName, int column);

    bool existIndex(string indexName);

    bool deleteIndex(string indexName);

    list <string> getIndexNamesOfTable(string tableName);

    string getIndexNameByAttrID(string tableName, int column);

    string getIndexFileByIndexName(string indexName);

    bool existIndexOnTable(string indexName, string tableName);

    bool existIndexOnAttrID(string tableName, int column);

    IndexNode findIndex(string indexName, string tableName);
    //attribute

    bool existAttr(string tableName, string attrName);

    bool isAttrUnique(string tableName, string attrName);

    DataType getAttrType(string tableName, string attrName);

    int getAttrOffset(string tableName, string attrName);

    int getAttrLength(string tableName, string attrName);

    int AttrtoId(string &tablename, string &attrname);

    Attr IdtoAttr(string &tablename, int id);

    vector<Attr> getAttrList(string tableName);

    Attr getAttrInfo(string tableName, string attrName);

    //record
    int getRecordNum(string tableName);//返回条目数量
    bool deleteRecord(string tableName, int num);//删除记录数量
    bool addRecord(string tableName, int num);//添加记录数量
    static CatalogManager &getInstance() {
        static CatalogManager instance;
        return instance;

    };

};


#endif