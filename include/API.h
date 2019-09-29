/*
 * by Zijie Song
 */

#ifndef API_H
#define API_H

#include <iterator>

#include"definitions.h"
#include"StmtAfterParse.h"
#include"CatalogManager.h"
#include"IndexManager.h"
#include"RecordManager.h"
#include"string"
#include"iostream"

using namespace std;

class API {
private:
    CatalogManager &catalogManager = CatalogManager::getInstance();
    IndexManager indexManager;
    RecordManager recordManager;

    /*CREATE TABLE语句*/
    int createTable(string tableName, vector<Attr> attributes) {
        list <Attr> attrilist;
        std::copy(attributes.begin(), attributes.end(), std::back_inserter(attrilist));
        catalogManager.createTable(tableName, attrilist);
        for (int i = 0; i < attributes.size(); i++) {
            if (attributes[i].isPrimary) {
                catalogManager.createIndex(attributes[i].attrName + "_primary", tableName, i);
                createIndex(tableName,attributes[i].attrName + "_primary",i);
                break;
            }
        }
        return 1;
    };

    /*CREATE INDEX语句*/
    int createIndex(string tableName, string indexName, int column) {
        string fileName = indexName + "_" + tableName + ".index";
        Attr attr = catalogManager.IdtoAttr(tableName, column);
        vector<AttrWithValue> record;//该表的所有记录
        vector<Attr> attrvec = catalogManager.getAttrList(tableName);//该表的所有列
        vector<IndexInfo> tempii = recordManager.selectIndexInfo(tableName);
        catalogManager.createIndex(indexName, tableName, column);
        if (attr.type == MINI_STRING) {
            IndexInput<string> indexi(fileName);//语法不知道有没有问题
            indexi.setType(NO);
			indexManager.createIndex(indexi);
            record = recordManager.selectAttr(tableName, attrvec, column);
            for (int i = 0; i < record.size(); i++) {
                indexi.setKey(record[i].value.s);
                indexi.setIndex_info(tempii[i]);
                indexManager.insertNode(indexi);
            }
        } else if (attr.type == MINI_FLOAT) {
            IndexInput<float> indexi(fileName);
            indexi.setType(NO);
			indexManager.createIndex(indexi);
            record = recordManager.selectAttr(tableName, attrvec, column);
            for (int i = 0; i < record.size(); i++) {
                indexi.setKey(record[i].value.f);
                indexi.setIndex_info(tempii[i]);
                indexManager.insertNode(indexi);
            }
        } else {
            IndexInput<int> indexi(fileName);
            indexi.setType(NO);
			indexManager.createIndex(indexi);
            record = recordManager.selectAttr(tableName, attrvec, column);
            for (int i = 0; i < record.size(); i++) {
                indexi.setKey(record[i].value.n);
                indexi.setIndex_info(tempii[i]);
                indexManager.insertNode(indexi);
            }
        }
        return 1;
    };

    /*INSERT语句*/
    int Insert(string tableName, vector<UnionValue> valueList) {
        vector<Attr> attrs = catalogManager.getAttrList(tableName);
        vector<AttrWithValue> AttrValues, record;
        AttrWithValue tempa;
        string fileName, indexName, error;
        for (int i = 0; i < valueList.size(); i++) {
            tempa.isPrimary = attrs[i].isPrimary;
            tempa.isUnique = attrs[i].isUnique;
            tempa.type = attrs[i].type;
            tempa.length = attrs[i].length;
            tempa.value = valueList[i];
            tempa.isNull = valueList[i].isNull;
            if (attrs[i].isPrimary) {
                if (tempa.isNull)//primary key不能为空，异常
                {
                    error = "primary key can't be null";
                    throw error;
                }
                record = recordManager.selectAttr(tableName, attrs, i);
                for (int j = 0; j < record.size(); j++) {
                    if (record[j].value.s == tempa.value.s && record[j].value.f == tempa.value.f &&
                        record[j].value.n == tempa.value.n) {
                        error = "primary key can't be repeated";
                        throw error;
                    }//primary key不能重复，异常
                }
            }
            if (attrs[i].isUnique)
                if (!isUnique(tableName, attrs, i, tempa.value))//不符合unique，异常
                {
                    error = "value isn't unique";
                    throw error;
                }
            AttrValues.push_back(tempa);
        }
        IndexInfo tempii = recordManager.insert(tableName, attrs, AttrValues);
        for (int i = 0; i < valueList.size(); i++) {
            if (catalogManager.existIndexOnAttrID(tableName, i)) {
                indexName = catalogManager.getIndexNameByAttrID(tableName, i);
                fileName = indexName + "_" + tableName + ".index";
                if (attrs[i].type == MINI_STRING) {
                    IndexInput<string> indexi(fileName);
                    indexi.setKey(valueList[i].s);
                    indexi.setIndex_info(tempii);
                    indexManager.insertNode(indexi);
                } else if (attrs[i].type == MINI_FLOAT) {
                    IndexInput<float> indexi(fileName);
                    indexi.setKey(valueList[i].f);
                    indexi.setIndex_info(tempii);
                    indexManager.insertNode(indexi);
                } else {
                    IndexInput<int> indexi(fileName);
                    indexi.setKey(valueList[i].n);
                    indexi.setIndex_info(tempii);
                    indexManager.insertNode(indexi);
                }
            }
        }

        catalogManager.addRecord(tableName, 1);
        return 1;
    }

    /*DELETE语句*/
    int Delete(string tableName, vector<Condition> conditions) {
        string indexName, fileName;
        vector<string> indexFileName;
        vector<IndexInfo> indexInfo;
        vector<Condition>::iterator v;
        vector<Attr> attrList;
        Attr tempa;
        int flag = 1;
        attrList = catalogManager.getAttrList(tableName);
        for (int i = 0; i < conditions.size(); i++) {
            tempa = catalogManager.IdtoAttr(tableName, conditions[i].id);
            if (catalogManager.existIndexOnAttrID(tableName, conditions[i].id)) {
                indexName = catalogManager.getIndexNameByAttrID(tableName, conditions[i].id);
                fileName = indexName + "_" + tableName + ".index";
                indexFileName.push_back(fileName);
                if (conditions[i].type == MINI_STRING) {
                    IndexInput<string> indexi(fileName);
                    indexi.setType(conditions[i].op);
                    indexi.setKey(conditions[i].value.s);
                    if (!flag)
                        indexInfo = intersect(indexInfo, indexManager.search(indexi));
                    else {
                        indexInfo = indexManager.search(indexi);
                        flag = 0;
                    }
                } else if (conditions[i].type == MINI_FLOAT) {
                    IndexInput<float> indexi(fileName);
                    indexi.setType(conditions[i].op);
                    indexi.setKey(conditions[i].value.f);
                    if (!flag)
                        indexInfo = intersect(indexInfo, indexManager.search(indexi));
                    else {
                        indexInfo = indexManager.search(indexi);
                        flag = 0;
                    }
                } else {
                    IndexInput<int> indexi(fileName);
                    indexi.setType(conditions[i].op);
                    indexi.setKey(conditions[i].value.n);
                    if (!flag)
                        indexInfo = intersect(indexInfo, indexManager.search(indexi));
                    else {
                        indexInfo = indexManager.search(indexi);
                        flag = 0;
                    }
                }
                v = conditions.begin() + i;
                conditions.erase(v);
            }
        }
        for (int i = 0; i < indexInfo.size(); i++) {
            for (int j = 0; j < indexFileName.size(); j++) {
                IndexInput<float> indexi(indexFileName[j]);
                indexi.setIndex_info(indexInfo[i]);
                indexManager.deleteNode(indexi);
            }
        }
        int num = recordManager.deleteTuple(tableName, conditions, attrList, indexInfo).size();
        catalogManager.deleteRecord(tableName, num);
        cout << num << " records deleted." << endl;
        return 1;
    };

    /*DROPTABLE语句*/
    int dropTable(string tableName) {
        //想要删除所有的record，首先需要对每个index生成indexinput，然后把它传给search，获得indexinfo，最后传给deletetuple
        vector<Condition> conditions;
        vector<Attr> attrList = catalogManager.getAttrList(tableName);
        vector<IndexInfo> indexInfo;
        list <string> indexList = catalogManager.getIndexNamesOfTable(tableName);
        list<string>::iterator v = indexList.begin();
        IndexNode tempi;
        Attr tempa;
        string fileName;
        for (int i = 0; i < indexList.size(); i++, v++) {
            tempi = catalogManager.findIndex(*v, tableName);
            tempa = catalogManager.getAttrInfo(tableName, tempi.attribute);
            fileName = *v + "_" + tableName + ".index";
            if (tempa.type == MINI_STRING) {
                IndexInput<string> indexi(fileName);
                indexi.setType(NO);
                indexInfo = indexManager.search(indexi);
            } else if (tempa.type == MINI_FLOAT) {
                IndexInput<float> indexi(fileName);
                indexi.setType(NO);
                indexInfo = indexManager.search(indexi);
            } else {
                IndexInput<int> indexi(fileName);
                indexi.setType(NO);
                indexInfo = indexManager.search(indexi);
            }
            dropIndex(tableName, *v);
        }
        catalogManager.deleteRecord(tableName, catalogManager.getRecordNum(tableName));
        recordManager.deleteTuple(tableName, conditions, attrList, indexInfo);
        catalogManager.deleteTable(tableName);
        return 1;
    };

    /*DROPINDEX语句*/
    int dropIndex(string tableName, string indexName) {
        string fileName = indexName + "_" + tableName + ".index";
        IndexInput<float> indexi(fileName);
        indexi.setType(NO);
        catalogManager.deleteIndex(indexName);
        indexManager.dropIndex(indexi);
        return 1;
    };

    /*SELECT语句*/
    int Select(string tableName, vector<Condition> conditions, vector<Attr> definitions) {
        string indexName, fileName;
        vector<IndexInfo> indexInfo;
        vector<Condition>::iterator v;
        vector<Attr> attrList = catalogManager.getAttrList(tableName);
        vector<int> attrColumn;
        Attr tempa;
        int flag = 1;
        for (int i = 0; i < definitions.size(); i++)
            attrColumn.push_back(catalogManager.AttrtoId(tableName, definitions[i].attrName));
        for (int i = conditions.size() - 1; i >= 0; i--) {
            tempa = catalogManager.IdtoAttr(tableName, conditions[i].id);
            if (catalogManager.existIndexOnAttrID(tableName, conditions[i].id)) {
                indexName = catalogManager.getIndexNameByAttrID(tableName, conditions[i].id);
                fileName = indexName + "_" + tableName + ".index";
                if (conditions[i].type == MINI_STRING) {
                    IndexInput<string> indexi(fileName);
                    indexi.setType(conditions[i].op);
                    indexi.setKey(conditions[i].value.s);
                    if (!flag)
                        indexInfo = intersect(indexInfo, indexManager.search(indexi));
                    else {
                        indexInfo = indexManager.search(indexi);
                        flag = 0;
                    }
                } else if (conditions[i].type == MINI_FLOAT) {
                    IndexInput<float> indexi(fileName);
                    indexi.setType(conditions[i].op);
                    indexi.setKey(conditions[i].value.f);
                    if (!flag)
                        indexInfo = intersect(indexInfo, indexManager.search(indexi));
                    else {
                        indexInfo = indexManager.search(indexi);
                        flag = 0;
                    }
                } else {
                    IndexInput<int> indexi(fileName);
                    indexi.setType(conditions[i].op);
                    indexi.setKey(conditions[i].value.n);
                    if (!flag)
                        indexInfo = intersect(indexInfo, indexManager.search(indexi));
                    else {
                        indexInfo = indexManager.search(indexi);
                        flag = 0;
                    }
                }
                v = conditions.begin() + i;
                conditions.erase(v);
            }
        }
        cout << recordManager.select(tableName, conditions, attrList, attrColumn, indexInfo) << " records found."
             << endl;
        return 1;
    };

    Attr getAttName(string tableName, int column) {
        vector<Attr> attrList = catalogManager.getAttrList(tableName);
        return attrList[column];
    };

    bool isUnique(string tableName, vector<Attr> attributes, int column, UnionValue attrvalue) {
        vector<AttrWithValue> recordList = recordManager.selectAttr(tableName, attributes, column);
        for (int i = 0; i < recordList.size(); i++) {
            if ((recordList[i].value.isNull == attrvalue.isNull) && (recordList[i].value.s == attrvalue.s) &&
                (recordList[i].value.f == attrvalue.f) && (recordList[i].value.n == attrvalue.n))
                return false;
        }
        return true;
    };

    vector<IndexInfo> intersect(vector<IndexInfo> info1, vector<IndexInfo> info2) {
        vector<IndexInfo> result;
        for (int i = 0; i < info1.size(); i++) {
            for (int j = 0; j < info2.size(); j++) {
                if (info1[i].blockNum == info2[j].blockNum && info1[i].offset == info2[j].offset) {
                    result.push_back(info1[i]);
                    break;
                }
            }
        }
        return result;
    };

public:
    API(CatalogManager &_catalogManager) : catalogManager(_catalogManager) {}

    ~API() {}

    int exec(StmtAfterParse result) {
        switch (result.getOperation()) {
            case MINI_CREATE:
                if (result.getObject() == MINI_TABLE)
                    createTable(result.getTableName(), result.getDefinitions());
                else if (result.getObject() == MINI_INDEX)
                    createIndex(result.getTableName(), result.getIndexName(), result.indexOnWhichColumn());
                break;
            case MINI_INSERT:
                Insert(result.getTableName(), result.getData());
                break;
            case MINI_DELETE:
                Delete(result.getTableName(), result.getConditions());
                break;
            case MINI_SELECT:
                Select(result.getTableName(), result.getConditions(), result.getDefinitions());
                break;
            case MINI_DROP:
                if (result.getObject() == MINI_TABLE)
                    dropTable(result.getTableName());
                else if (result.getObject() == MINI_INDEX)
                    dropIndex(result.getTableName(), result.getIndexName());
                break;
            default:
                break;
        }
        return 1;
    };

};

#endif
