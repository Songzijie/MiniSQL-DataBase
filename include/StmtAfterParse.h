/*
 * by Zijie Song
 */

#ifndef STMT_AFTER_PARSE_H
#define STMT_AFTER_PARSE_H

#include "definitions.h"
#include <typeinfo>
#include <vector>

using namespace std;

class StmtAfterParse {
public:
    int getOperation() {
        return action;
    }

    int getObject() {
        return object;
    }

    int getPrimaryKeyIndex() {
        return pkey;
    }

    Attr getPrimaryKey() {
        return definitions[pkey];
    }

    Attr getAttrNode(int i) {
        return definitions[i];
    }

    vector<Attr> getDefinitions() {
        return definitions;
    }

    string indexOnWhichTable() {
        return tableName;
    }

    int indexOnWhichColumn() {
        return column;
    }

    string getIndexName() {
        return indexName;
    }

    string getTableName() {
        return tableName;
    }

    vector<Condition> getConditions() {
        return conditions;
    }

    Condition getConditionByIndex(int index) {
        return conditions[index];
    }

    vector<UnionValue> getData() {
        return data;
    }

    UnionValue getDatumByIndex(int index) {
        return data[index];
    }

    void setOperation(ActionType a) {
        action = a;
    }

    void setObject(ObjectType o) {
        object = o;
    }

    void setTableName(string name) {
        tableName = name;
    }

    void setIndexName(string name) {
        indexName = name;
    }

    void addCondition(Condition t) {
        conditions.push_back(t);
    }

    void addData(UnionValue u) {
        data.push_back(u);
    }

    void setColumn(int i) {
        column = i;
    }

    void addDefinition(Attr def) {
        definitions.push_back(def);
    }

	void setDefinition(vector<Attr> def) {
		definitions = def;
	}

    int findIndexOfDef(string name) {
        for (int i = 0; i < definitions.size(); ++i)
            if (definitions[i].attrName == name) {
                definitions[i].isPrimary = true;
                definitions[i].isUnique = true;
                return i;
            }
        return -1;
    }

    void setPrimaryKeyIndex(int index) {
        pkey = index;
    }

    const string &getFileName() const {
        return fileName;
    }

    void setFileName(const string &fileName) {
        StmtAfterParse::fileName = fileName;
    }
private:
    ActionType action;      //create, dropIndex, select, insertNode, delete
    ObjectType object;      //table or index
    string tableName;
    //delete from <tableName> where <conditions>
    //select * from <tableName> where <conditions>
    vector<Condition> conditions;
    int pkey;

    //create <object(table)> <definitions> primary key definitions[<pkey>]
    vector<Attr> definitions;

    //create <object(index)> on <tableName>(<column>)
    int column;

    //insertNode <data> into <tableName>
    vector<UnionValue> data;

    //dropIndex <object> <name>
    string indexName;

    string fileName;

};

#endif
