/*
 * by Zijie Song
 */

#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H

#include <iostream>
#include <String>
#include <vector>
#include "BufferManager.h"
#include "definitions.h"
#include "Index.h"
#include <map>

using namespace std;


template<class T>
class IndexInput {
public:
    string indexFileName;
    T key;
    IndexInfo index;
    int type; //op type

public:
    IndexInput(string t) { indexFileName = t; }

    void setKey(T x) { key = x; }

    void setType(int x) { type = x; }

    void setIndex_info(IndexInfo i) { index = i; }

    string getIndexFileName() { return indexFileName; }

    T getKey() { return key; }
};


class IndexManager {
private:
    map<string, int> num;
    map<string, int> type;
    map<string, int> maxnum;
public:

    IndexManager() {};

    virtual ~IndexManager() {};

    template<class T>
    int createIndex(IndexInput<T>);

    template<class T>
    void dropIndex(IndexInput<T>);

    template<class T>
    vector<IndexInfo> search(IndexInput<T>);

    template<class T>
    void insertNode(IndexInput<T>);

    template<class T>
    void deleteNode(IndexInput<T>);

};


template<class T>
int IndexManager::createIndex(IndexInput<T> x) {
    Index t(x.indexFileName);
    num.insert(pair<string, int>(x.indexFileName, 0));
    maxnum.insert(pair<string, int>(x.indexFileName, 0));
    type.insert(pair<string, int>(x.indexFileName, 0));
    return 1;
}

template<class T>
void IndexManager::insertNode(IndexInput<T> x) {
//    cout<<"insert Node: "<<x.key<<endl;
    Index t(x.indexFileName);
    if (t.curn > num[x.indexFileName]) {
        num[x.indexFileName] = t.curn;
        type[x.indexFileName] = t.datatype;
        maxnum[x.indexFileName] = t.maxn;
    }
    if (num[x.indexFileName] == 0) {
        t.init(x.key, x.index);
        type[x.indexFileName] = rtype(x.key);
        num[x.indexFileName] = 2;
        int length;
        if (type[x.indexFileName] == 0) length = 4;
        else if (type[x.indexFileName] == 1) length = 4;
        else if (type[x.indexFileName] == 2) length = 20;
        maxnum[x.indexFileName] = (BLOCK_SIZE - NINFO) / (length + PINFO) - 1;
    } else {
        t.datatype = (type[x.indexFileName]);
        t.curn = (num[x.indexFileName]);
        t.maxn = (maxnum[x.indexFileName]);
        int flag = t.insert(x.key, x.index);
        if (flag != -1) {
            num[x.indexFileName] += flag;
        }
    }
}

template<class T>
void IndexManager::deleteNode(IndexInput<T> x) {
    Index t(x.indexFileName);
    if (t.curn > num[x.indexFileName]) {
        num[x.indexFileName] = t.curn;
        type[x.indexFileName] = t.datatype;
        maxnum[x.indexFileName] = t.maxn;
    }
    t.datatype = (type[x.indexFileName]);
    t.curn = (num[x.indexFileName]);
    t.maxn = (maxnum[x.indexFileName]);
    if (num[x.indexFileName] == 0)
        return;
    else
        t.remove(x.index);
}

template<class T>
void IndexManager::dropIndex(IndexInput<T> x) {
    num.erase(x.indexFileName);
    maxnum.erase(x.indexFileName);
    type.erase(x.indexFileName);
    remove(("db_files/" + x.indexFileName).c_str());
}

template<class T>
vector<IndexInfo> IndexManager::search(IndexInput<T> x) {
    Index t(x.indexFileName);
    if (t.curn > num[x.indexFileName]) {
        num[x.indexFileName] = t.curn;
        type[x.indexFileName] = t.datatype;
        maxnum[x.indexFileName] = t.maxn;
    }
    t.datatype = (type[x.indexFileName]);
    t.curn = (num[x.indexFileName]);
    t.maxn = (maxnum[x.indexFileName]);
    vector<IndexInfo> ret;
    IndexInfo tmp;
    tmp.blockNum = -1;
    tmp.offset = 0;
    if (num[x.indexFileName] == 0) {
        ret.push_back(tmp);
        return ret;
    } else {
        if (x.type == EQU) {
            tmp = t.search(x.key);
            ret.push_back(tmp);
            return ret;
        } else if (x.type == NE) {
            vector<IndexInfo> ret1;
            vector<IndexInfo> ret2;
            ret1 = t.range(x.key, x.key, 0, 1, 1, 1);
            ret2 = t.range(x.key, x.key, 1, 0, 1, 1);
            ret.insert(ret.end(), ret1.begin(), ret1.end());
            ret.insert(ret.end(), ret2.begin(), ret2.end());
            return ret;
        } else if (x.type == LT) {
            ret = t.range(x.key, x.key, 1, 0, 1, 1);
            return ret;
        } else if (x.type == GT) {
            ret = t.range(x.key, x.key, 0, 1, 1, 1);
            return ret;
        } else if (x.type == LE) {
            ret = t.range(x.key, x.key, 1, 1, 1, 0);
            return ret;
        } else if (x.type == GE) {
            ret = t.range(x.key, x.key, 1, 1, 0, 1);
            return ret;
        } else if (x.type == NO) {
            ret.push_back(tmp);
            return ret;
        }
    }
}


#endif













