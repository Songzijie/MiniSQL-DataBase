/*
 * by Zijie Song
 */

#ifndef MINISQL_INDEX_H
#define MINISQL_INDEX_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "definitions.h"
#include "BufferManager.h"
#include "helpers.h"
#include <string>

using namespace std;
#define NINFO 24
#define PINFO 20


enum nodetype{ leaf, nonleaf };

class Index {
private:

    int datawidth[3];  //different width of int, float and string
    string index_name;
    BufferManager &buffer = BufferManager::GetInstance();
public:
    int curn; //current number of child
    int maxn; //most number of child
    int datatype; //int float or string
public:
    Index(){};
    Index(string filename);
    ~Index(){};
    template <class T>
    void init(T, IndexInfo);
    template <class T>
    IndexInfo search(T key);
    template <class T>
    int insert(T, IndexInfo);
    template <class T>
    int* split(char*, T*, T, IndexInfo, int, int);
    template <class T>
    void internal_insert(char*, T*, int, int);
    template <class T>
    void leaf_split(char*, char*, char*, T, IndexInfo);
    template <class T>
    void nonleaf_split(char*, char*, char*, T*, int, int);
    int remove(IndexInfo);
    template <class T>
    vector<IndexInfo> range(T, T, int, int, int, int);

};


template <class T>
void Index::init(T key, IndexInfo bo) {
    char *root = new char[BLOCK_SIZE];
    *(int*)(root) = nonleaf;
    *(int*)(root + 4) = 0;  //the pos of root: 0
    *(int*)(root + 8) = -1; //the pos of root' parent: -1
    *(int*)(root + 12) = 1; //the num of nodes
    *(int*)(root + 16) = 0; //delete or not
    *(int*)(root + 20) = rtype(key);
    datatype = rtype(key);

    *(int*)(root + NINFO + datawidth[datatype]) = 1;
    trans((root + NINFO + datawidth[datatype] + PINFO), key);

    *(int*)(root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype]) = -1;
    *(int*)(root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 4) = -1;
    *(int*)(root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 8) = 1;
    *(int*)(root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 12) = 1;
    *(int*)(root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 16) = 0;

    BufferNode &t1 = buffer.getBlock(MINI_INDEX, index_name, 0);
    t1.writeBlock(*root, BLOCK_SIZE, 0);

    char *node = new char[BLOCK_SIZE];
    *(int*)(node) = leaf;
    *(int*)(node + 4) = 1;  //leaf node pos: 1
    *(int*)(node + 8) = 0;  //parent node pos: 0
    *(int*)(node + 12) = 1; //n nodes
    *(int*)(node + 16) = 0; //not delete
    *(int*)(node + 20) = datatype;

    *(int*)(node + NINFO + datawidth[datatype]) = 1;
    trans((node + NINFO + datawidth[datatype] + PINFO), key);


    *(int*)(node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype]) = -1;
    *(int*)(node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 4) = bo.blockNum;
    *(int*)(node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 8) = bo.offset;
    *(int*)(node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 12) = 1;
    *(int*)(node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 16) = 0;

    BufferNode& t2 = buffer.getBlock(MINI_INDEX, index_name, 1);
    t2.writeBlock(*node, BLOCK_SIZE, 0);

    delete[] root;
    delete[] node;
}

template <class T>
IndexInfo Index::search(T key) {
    char *curblock = new char[BLOCK_SIZE];
    BufferNode &t1 = buffer.getBlock(MINI_INDEX, index_name, 0);
    t1.readBlock(*curblock, BLOCK_SIZE, 0);

    int i, curpos, lastsibling;
    int blocktype = *(int *) (curblock);
    int sibling = *(int *) (curblock + NINFO + datawidth[datatype]);
    IndexInfo ret;
    ret.blockNum = -1;
    ret.offset = 0;
    T curkey = key;
    while (blocktype == nonleaf) {
        int nkeys = *(int *) (curblock + 12);
        for (i = 0; i < nkeys; i++) {
            lastsibling = sibling;
            curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
            curpos = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4);
            sibling = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype]);
            if (key < curkey)
                break;
        }
        //most great number
        if (i == nkeys)
            curpos = *(int *) (curblock + NINFO + lastsibling * (datawidth[datatype] + PINFO) + datawidth[datatype] +
                               12);
        if (curpos == -1)
            return ret;
        BufferNode &t2 = buffer.getBlock(MINI_INDEX, index_name, curpos);
        t2.readBlock(*curblock, BLOCK_SIZE, 0);
        blocktype = *(int *) (curblock);
        sibling = *(int *) (curblock + NINFO + datawidth[datatype]);
    }
    //when this node is leaf node
    int nkeys = *(int *) (curblock + 12);
    sibling = *(int *) (curblock + NINFO + datawidth[datatype]);
    for (i = 0; i < nkeys; i++) {
        curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
        curpos = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4);
        int offset = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8);
        if (key == curkey &&
            *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) != 1) {
            delete[]curblock;
            ret.blockNum = curpos;
            ret.offset = offset;
            return ret;
        }
        sibling = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype]);
    }
    delete[] curblock;
    return ret;
}

template <class T>
int Index::insert(T key, IndexInfo bo) {
    //if already exists, break
    if (search(key).blockNum != -1)
        return -1;

    int i, nextpos, lastsibling;

    //read the root block
    char *curblock = new char[BLOCK_SIZE];
    BufferNode &t1 = buffer.getBlock(MINI_INDEX, index_name, 0);
    t1.readBlock(*curblock, BLOCK_SIZE, 0);
    int blocktype = *(int *) (curblock);
    int sibling = *(int *) (curblock + NINFO + datawidth[datatype]);
    T curkey = key;
    while (blocktype == nonleaf) {
        int nkeys = *(int *) (curblock + 12);
        for (i = 0; i < nkeys; i++) {
            lastsibling = sibling;
            curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
            //if <, go to left position
            nextpos = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4);
            sibling = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype]);
            if (key < curkey)
                break;
        }
        if (i == nkeys)
            nextpos = *(int *) (curblock + NINFO + lastsibling * (datawidth[datatype] + PINFO) + datawidth[datatype] +
                                12);
        if (nextpos == -1) {
            char *node = new char[BLOCK_SIZE];
            *(int *) (node) = leaf;
            *(int *) (node + 4) = curn++;   //2, pos of node:2, 3
            *(int *) (node + 8) = *(int *) (curblock + 4);
            *(int *) (node + 12) = 1;
            *(int *) (node + 16) = 0;
            *(int *) (node + 20) = datatype;
            *(int *) (node + NINFO + datawidth[datatype]) = 1;

            trans((node + NINFO + datawidth[datatype] + PINFO), key);

            *(int *) (node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype]) = -1;
            *(int *) (node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 4) = bo.blockNum;
            *(int *) (node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 8) = bo.offset;
            *(int *) (node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 12) = 1;
            *(int *) (node + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 16) = 0;

            nextpos = curn - 1;
            BufferNode &t2 = buffer.getBlock(MINI_INDEX, index_name, nextpos);
            t2.writeBlock(*node, BLOCK_SIZE, 0);
            *(int *) (curblock + NINFO + lastsibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = nextpos;

            BufferNode &t3 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (curblock + 4));
            t3.writeBlock(*curblock, BLOCK_SIZE, 0);
            return 1;
        }
        BufferNode& t4 = buffer.getBlock(MINI_INDEX, index_name, nextpos);
        t4.readBlock(*curblock, BLOCK_SIZE, 0);
        blocktype = *(int *) (curblock);
        sibling = *(int *) (curblock + NINFO + datawidth[datatype]);
    }
    //leaf node
    int nkeys = *(int *) (curblock + 12);
    sibling = *(int *) (curblock + NINFO + datawidth[datatype]);
    if (nkeys >= maxn) {
        *(int*)(curblock + 12) = nkeys + 1;
        T *mid = NULL;
        split(curblock, mid, key, bo, 0, 0);
        return 2;
    } else {
        nkeys++;
        lastsibling = 0;
        *(int *) (curblock + 12) = nkeys;
        for (i = 0; i < nkeys; i++) {
            if (i == nkeys - 1 || sibling == -1)
                break;
            curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
            if (key < curkey)
                break;
            lastsibling = sibling;
            sibling = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype]);
        }

        trans(curblock + NINFO + nkeys * (datawidth[datatype] + PINFO), key);

        *(int *) (curblock + NINFO + nkeys * (datawidth[datatype] + PINFO) + datawidth[datatype]) = (sibling == -1)
                                                                                                    ? -1 : *(int *) (
                        curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12);
        //perivous node point to new node
        *(int *) (curblock + NINFO + lastsibling * (datawidth[datatype] + PINFO) + datawidth[datatype]) = nkeys;
        *(int *) (curblock + NINFO + nkeys * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = bo.blockNum;
        *(int *) (curblock + NINFO + nkeys * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = bo.offset;
        //
        *(int *) (curblock + NINFO + nkeys * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = nkeys;
        BufferNode &t5 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (curblock + 4));
        t5.writeBlock(*curblock, BLOCK_SIZE, 0);
    }
    delete[] curblock;
    return 0;
}

template <class T>
int* Index::split(char *curblock, T *mid, T key, IndexInfo bo, int left, int right) {
    int nkeys = *(int *) (curblock + 12);
    int blocktype = *(int *) (curblock);

    int* father = new int[2];
    father[0] = father[1] = 0;
    //root node is full
    if ((*(int *) (curblock + 8) == -1) && nkeys >= maxn - 1) {
        char *root = new char[BLOCK_SIZE];
        char *block1 = new char[BLOCK_SIZE];
        char *block2 = new char[BLOCK_SIZE];

        nonleaf_split(block1, block2, curblock, mid, left, right);
        *(int *) (block1 + 8) = *(int *) (block2 + 8) = 0;
        int tmpleft = *(int*)(block1+4);
        int tmpright = *(int*)(block2+4);
        mid = new T(get(block2 + NINFO + datawidth[datatype] + PINFO, key));
        BufferNode &t1 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (block1 + 4));
        t1.writeBlock(*block1, BLOCK_SIZE, 0);
        BufferNode &t2 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (block2 + 4));
        t2.writeBlock(*block2, BLOCK_SIZE, 0);

        *(int *) (root) = nonleaf;
        *(int *) (root + 4) = 0;
        *(int *) (root + 8) = -1;
        *(int *) (root + 12) = 1;
        *(int *) (root + 16) = 0;
        *(int *) (root + 20) = datatype;

        *(int *) (root + NINFO + datawidth[datatype]) = 1;
        T tmpkey = *mid;
        trans(root + NINFO + (datawidth[datatype] + PINFO), tmpkey);

        *(int *) (root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype]) = -1;
        *(int *) (root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 4) = tmpleft;
        *(int *) (root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 8) = 1;
        *(int *) (root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 12) = tmpright;
        *(int *) (root + NINFO + datawidth[datatype] + PINFO + datawidth[datatype] + 16) = 0;

        BufferNode &t3 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (root + 4));
        t3.writeBlock(*root, BLOCK_SIZE, 0);

        father[0] = *(int *) (block1 + 4);
        father[1] = *(int *) (block2 + 4);

        delete[]root;
        delete[]block1;
        delete[]block2;
        return father;
    } else {
        if (blocktype == leaf && nkeys >= maxn) {
            char *root = new char[BLOCK_SIZE];
            char *block1 = new char[BLOCK_SIZE];
            char *block2 = new char[BLOCK_SIZE];
            int *tmp = NULL;

            leaf_split(block1, block2, curblock, key, bo);

            *(int *) (curblock + 16) = 1;
            int tmpleft = *(int *) (block1 + 4);
            int tmpright = *(int *) (block2 + 4);

            mid = new T(get(block2 + NINFO + datawidth[datatype] + PINFO, key));

            BufferNode &t1 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (curblock + 8));
            t1.readBlock(*root, BLOCK_SIZE, 0);

            tmp = split(root, mid, key, bo, tmpleft, tmpright);

            *(int *) (block1 + 8) = tmp[0];
            *(int *) (block2 + 8) = tmp[1];


            BufferNode &t2 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (curblock + 4));
            t2.writeBlock(*curblock, BLOCK_SIZE, 0);
            BufferNode &t3 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (block1 + 4));
            t3.writeBlock(*block1, BLOCK_SIZE, 0);
            BufferNode &t4 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (block2 + 4));
            t4.writeBlock(*block2, BLOCK_SIZE, 0);

            father[0] = *(int *) (block1 + 4);
            father[1] = *(int *) (block2 + 4);

            delete[] block1;
            delete[] block2;
            delete[] root;
            return father;
        } else if (blocktype == nonleaf && nkeys >= maxn - 1) {
            char *root = new char[BLOCK_SIZE];
            char *block1 = new char[BLOCK_SIZE];
            char *block2 = new char[BLOCK_SIZE];
            nonleaf_split(block1, block2, curblock, mid, left, right);
            *(int *) (curblock + 16) = 1;
            int tmpleft = *(int *) (block1 + 4);
            int tmpright = *(int *) (block2 + 4);
            int *tmp = NULL;

            mid = new T(get(block2 + NINFO + datawidth[datatype] + PINFO, key));

            BufferNode &t1 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (curblock + 8));
            t1.readBlock(*root, BLOCK_SIZE, 0);

            tmp = split(root, mid, key, bo, tmpleft, tmpright);

            *(int *) (block1 + 8) = tmp[0];
            *(int *) (block2 + 8) = tmp[1];


            BufferNode &t2 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (curblock + 4));
            t2.writeBlock(*curblock, BLOCK_SIZE, 0);
            BufferNode &t3 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (block1 + 4));
            t3.writeBlock(*block1, BLOCK_SIZE, 0);
            BufferNode &t4 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (block2 + 4));
            t4.writeBlock(*block2, BLOCK_SIZE, 0);

            father[0] = *(int *) (block1 + 4);
            father[1] = *(int *) (block2 + 4);

            delete[] block1;
            delete[] block2;
            delete[] root;
            return father;
        } else if (blocktype == nonleaf && nkeys < maxn - 1) {
            *(int *) (curblock + 12) += 1;
            int nkeys = *(int *) (curblock + 12);
            int sibling = *(int *) (curblock + NINFO + datawidth[datatype]);
            int lastsibling;
            int i = 0;
            T curkey = key;
            for (i = 0; i < nkeys; i++) {
                if (sibling == -1 && i == nkeys - 1)
                    break;
                curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
                if (*mid < curkey)
                    break;
                lastsibling = sibling;
                sibling = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype]);
            }
            T tmpkey = *mid;
            trans(curblock + NINFO + nkeys * (datawidth[datatype] + PINFO), tmpkey);

            *(int *) (curblock + NINFO + nkeys * (datawidth[datatype] + PINFO) + datawidth[datatype]) = (i == nkeys - 1)
                                                                                                        ? -1
                                                                                                        : sibling;
            *(int *) (curblock + NINFO + nkeys * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = left;
            *(int *) (curblock + NINFO + nkeys * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = nkeys;
            *(int *) (curblock + NINFO + nkeys * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = right;
            *(int *) (curblock + NINFO + lastsibling * (datawidth[datatype] + PINFO) + datawidth[datatype]) = nkeys;
            *(int *) (curblock + NINFO + lastsibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = left;
            if (i != nkeys - 1)
                *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = right;
            BufferNode &t1 = buffer.getBlock(MINI_INDEX, index_name, *(int *) (curblock + 4));
            t1.writeBlock(*curblock, BLOCK_SIZE, 0);

            father[0] = *(int *) (curblock + 4);
            father[1] = *(int *) (curblock + 4);
            return father;
        }
    }
}


//split and add one mid node
template <class T>
void Index::nonleaf_split(char *block1, char *block2, char *curblock, T *mid, int left, int right) {
    *(int*)(curblock + 12) += 1;
    int nkeys = *(int*)(curblock + 12);
    //first position
    int sibling = *(int*)(curblock + NINFO + datawidth[datatype]);

    *(int*)(block1) = *(int*)(block2) = nonleaf;
    *(int*)(block1 + 4) = curn++;
    *(int*)(block2 + 4) = curn++;
    *(int*)(block1 + 8) = *(int*)(block2 + 8) = 0;    //0 is root
    *(int*)(block1 + 12) = nkeys / 2;
    *(int*)(block2 + 12) = nkeys - nkeys / 2;
    *(int*)(block1 + 16) = *(int*)(block2 + 16) = 0;
    *(int*)(block1 + 20) = *(int*)(block2 + 20) = datatype;

    //first position of the two new blocks are both 1
    *(int*)(block1 + NINFO + datawidth[datatype]) = *(int*)(block2 + NINFO + datawidth[datatype]) = 1;

    int i;
    int pos = 1;
    int flag = 1;
    T curkey = *mid;
    for (i = 0; i < nkeys / 2; i++) {
        curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
        int tmpleft = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4);
        int tmpright = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12);
        if (flag && (*mid) <= curkey) {
            T tmpkey = *mid;
            trans(block1 + NINFO + pos * (datawidth[datatype] + PINFO), tmpkey);
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = pos + 1;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = left;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = pos;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = right;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
            *(int*)(block1 + NINFO + (pos - 1) * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = left;
            //if not change the value of curblock, the change will be canceled
            *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = right;
            pos++;
            flag = 0;
        } else {
            trans(block1 + NINFO + pos * (datawidth[datatype] + PINFO), curkey);
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = pos + 1;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = tmpleft;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = pos;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = tmpright;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
            pos++;
            sibling = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype]);
        }
    }
    pos = 1;
    int j = i;
    for (; i < nkeys; i++) {
        if (sibling == -1 && i == nkeys - 1)
            break;
        curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
        int tmpleft = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4);
        int tmpright = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12);
        if (flag && (*mid) <= curkey) {
            T tmpkey = *mid;
            trans(block2 + NINFO + pos * (datawidth[datatype] + PINFO), tmpkey);
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = pos + 1;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = left;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = pos;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = right;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
            *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = right;
            if (j == i)
                *(int*)(block1 + NINFO + (*(int*)(block1 + 12)) * (datawidth[datatype] + PINFO) + datawidth[datatype] +
                       12) = left;
            else
                *(int*)(block2 + NINFO + (pos - 1) * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = left;
            pos++;
            flag = 0;
        } else {
            trans(block2 + NINFO + pos * (datawidth[datatype] + PINFO), curkey);
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = pos + 1;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = tmpleft;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = pos;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = tmpright;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
            pos++;
            sibling = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO)+datawidth[datatype]);
        }
    }
    if (flag) {
        T tmpkey = *mid;
        trans(block2 + NINFO + pos * (datawidth[datatype] + PINFO), tmpkey);
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = -1;
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = left;
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = pos;
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = right;
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
        *(int*)(block2 + NINFO + (pos - 1) * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = left;
        pos++;
    }
}

template <class T>
void Index::leaf_split(char *block1, char *block2, char *curblock, T mid, IndexInfo bo) {
    int nkeys = *(int*)(curblock + 12);

    *(int*)(block1) = *(int*)(block2) = leaf;
    *(int*)(block1 + 4) = curn++;
    *(int*)(block2 + 4) = curn++;
    *(int*)(block1 + 8) = *(int*)(block2 + 8) = 0;    //0 is root
    *(int*)(block1 + 12) = nkeys / 2;
    *(int*)(block2 + 12) = nkeys - (nkeys / 2);
    *(int*)(block1 + 16) = *(int*)(block2 + 16) = 0;
    *(int*)(block1 + 20) = *(int*)(block2 + 20) = datatype;

    *(int*)(block1 + NINFO + datawidth[datatype]) = *(int*)(block2 + NINFO + datawidth[datatype]) = 1;
    int i;
    int pos = 1;
    int flag = 1;
    int blocknum;
    int offset;
    int sibling = *(int*)(curblock + NINFO + datawidth[datatype]);
    T curkey = mid;
    for (i = 0; i < nkeys / 2; i++) {
        curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
        blocknum = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4);
        offset = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8);
        if (flag && mid <= curkey) {
            trans(block1 + NINFO + pos * (datawidth[datatype] + PINFO), mid);
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = pos + 1;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = bo.blockNum;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = bo.offset;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = pos;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
            pos++;
            flag = 0;
        } else {
            trans(block1 + NINFO + pos * (datawidth[datatype] + PINFO), curkey);
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = pos + 1;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = blocknum;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = offset;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = pos;
            *(int*)(block1 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
            pos++;
            sibling = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO)+datawidth[datatype]);
        }
    }
    *(int*)(block1 + NINFO + (pos - 1) * (datawidth[datatype] + PINFO) + datawidth[datatype]) = -1;
    pos = 1;
    for (; i < nkeys; i++) {
        if (sibling == -1 && i == nkeys - 1)
            break;
        curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
        blocknum = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4);
        offset = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8);
        if (flag && mid <= curkey) {
            trans(block2 + NINFO + pos * (datawidth[datatype] + PINFO), mid);
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = pos + 1;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = bo.blockNum;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = bo.offset;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = pos;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
            pos++;
            flag = 0;
        } else {
            trans(block2 + NINFO + pos * (datawidth[datatype] + PINFO), curkey);
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = pos + 1;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = blocknum;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = offset;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = pos;
            *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
            pos++;
            sibling = *(int*)(curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype]);
        }
    }
    if (flag) {
        trans(block2 + NINFO + pos * (datawidth[datatype] + PINFO), mid);
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype]) = -1;
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 4) = bo.blockNum;
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 8) = bo.offset;
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 12) = pos;
        *(int*)(block2 + NINFO + pos * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 0;
        pos++;
    }
    *(int*)(block2 + NINFO + (pos - 1) * (datawidth[datatype] + PINFO) + datawidth[datatype]) = -1;
}


template <class T>
vector<IndexInfo> Index::range(T key1, T key2, int flag1, int flag2, int flag3, int flag4) {
    char *curblock = new char[BLOCK_SIZE];
    vector<IndexInfo> ret;
    IndexInfo zero, tmp;
    zero.blockNum = -1;
    zero.offset = 0;
    int size = curn;
    T curkey = key1;
    for (int i = 0; i < size; i++) {
        BufferNode &t1 = buffer.getBlock(MINI_INDEX, index_name, i);
        t1.readBlock(*curblock, BLOCK_SIZE, 0);
        int blocktype = *(int *) (curblock);
        int flag = *(int *) (curblock + 16);
        if (blocktype == nonleaf || flag)
            continue;
        int nkeys = *(int *) (curblock + 12);
        int sibling = *(int *) (curblock + NINFO + datawidth[datatype]);
        for (int j = 0; j < nkeys; j++) {
            if (sibling == -1)
                break;
            curkey = get(curblock + NINFO + sibling * (datawidth[datatype] + PINFO), curkey);
            int blocknum = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] +
                                     4);
            int offset = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] +
                                   8);
            if ((curkey > key1 || flag1) && (curkey < key2 || flag2) && (curkey >= key1 || flag3) &&
                (curkey <= key2 || flag4) &&
                *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) != 1) {
                tmp.blockNum = blocknum;
                tmp.offset = offset;
                ret.push_back(tmp);
            }
            sibling = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype]);
        }
    }
    if (ret.size() == 0)
        ret.push_back(zero);
    return ret;
}

#endif //MINISQL_INDEX_H
