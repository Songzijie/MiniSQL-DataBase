/*
 * by Zijie Song
 */

#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#define  BLOCK_SIZE 4096
#define  MAX_BLOCK 1024

#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include "definitions.h"

using namespace std;

class BufferNode;

class BufferManager {
private:
    BufferNode *bufferPool;
    int accNum;

    BufferManager();

    ~BufferManager();

    //私有化拷贝构造函数和赋值构造函数实现单例模式
    BufferManager(const BufferManager &) {};

    BufferManager &operator=(const BufferManager &) {};

public:
    static BufferManager &GetInstance();

    BufferNode &getBlock(ObjectType type, string filename, int blockOffset);

};

class BufferNode {
private:
    char *data;     //数据
    string fileName;//文件名
    int offset;     //Block 偏移量
    ObjectType type;//目标类型
    bool dirty;     //脏标记

    //私有化构造函数、拷贝构造函数和赋值构造函数实现单例模式
    BufferNode() : age(0), pin(0), data(NULL), fileName(""), offset(0), type(MINI_TABLE), dirty(false) {}

    BufferNode(const BufferNode &bn) {};

    void operator=(const BufferNode &bn) {};

    void setBufferNode(ObjectType type, string filename, int blockOffset);

    // 将内存中的buffer转存到硬盘filename文件中
    void flush();

    bool isMatch(int type, string filename, int blockOffset);

    bool isEmpty();

    ~BufferNode();

    int age;       //最近访问时间

    bool pin;      //是否钉住
public:
    friend class BufferManager;

    template<class T>
    void writeBlock(T &data, int length, int offset);

    template<class T>
    void readBlock(T &dest, int length, int offset);

};

template<class T>
void BufferNode::readBlock(T &dest, int length, int offset) {
    memcpy((char *) &dest, this->data + offset, length);
}

template<class T>
void BufferNode::writeBlock(T &data, int length, int offset) {
    this->dirty = true;
    memcpy(this->data + offset, (char *) &data, length);
}


#endif