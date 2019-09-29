# MiniSQL

     __  __ _       _ ____   ___  _  
    |  \/  (_)_ __ (_) ___| / _ \| |  
    | |\/| | | '_ \| \___ \| | | | |  
    | |  | | | | | | |___) | |_| | |___  
    |_|  |_|_|_| |_|_|____/ \__\_\_____|  

## Description
This is a MiniSql engine powered by c++, DBMS homework.

## Authors
Guanzhou Liu,  Tao Chen, Yanfeng Zhao, Yi Ren, Zijie Song

## How to Use
## How to Use
1. g++ src/BufferManager.cpp src/CatalogManager.cpp src/RecordManager.cpp src/main.cpp src/Interpreter.cpp src/helpers.cpp src/Index.cpp -Iinclude -std=c++11 -o minisql
2. ./minisql

## Hint
Remember to configure the working directory of Clion project.

## How to Test
1. 把`/test/test.cpp.example`复制一份，重命名为test.cpp
1. 把`CmakeLists.txt`中你用不到的模块注释掉，防止别人模块的bug影响你的编译
1. 测试代码写在test.cpp中，测试完一个模块后更改下方的Unit Testing Status
1. 测试全部通过后，把`CmakeLists.txt`中的注释取消掉，push

## Unit Testing Status
只需要列出public函数的测试通过状态

### API & Interface

### CatalogManager
- Read and Write
    - [X] readCatalog
    - [X] writeCatalog
- Functions for Table
    - [X] createTable
    - [X] existTable
    - [X] deleteTable
    - [X] showTable
- Functions for Index
    - [X] createIndex
    - [X] existIndex
    - [X] deleteIndex
    - [X] getIndexNamesOfTable
    - [X] getIndexNameByAttrID
    - [X] getIndexFileByIndexName
    - [X] existIndexOnAttrID
    - [X] existIndexOnTable
    - [X] getAttrType
    - [X] AttrtoId
    - [X] IdtoAttr
    - [X] getRecordNum
    - [X] deleteRecord
    - [X] addRecord

### IndexManager

### RecordManager

### BufferManager
- BufferManager
    - [X] getBlock
- BufferNode
    - [X] setBufferNode
    - [X] flush
    - [X] isMatch
    - [X] isEmpty
    - [X] writeBlock
    - [X] readBlock
    
## Document


### BufferManager
- `BufferManager.getBlock` 
    - input: object类型（index或table），文件名filename和块偏移量blockOffset（注意这里的offset不同于下面两个函数中的offset）
    - return: 对应的`BufferNode`
- `BufferNode.writeBlock` 
    - 将数据写入该bufferNode，相当于写入数组
    - input: 数据，数据字节长度，偏移量（字节为单位）
- `BufferNode.readBlock`
    - 读该bufferNode中的数据，相当于读数组
    - input: 想要读入的变量地址，数据字节长度，偏移量（字节为单位）

```c++
    for (int i = 0; i < 10; i++) {
        auto &blockNode = bufferManager.getBlock(MINI_TABLE, "tableName.table", i);
        char a[10] = "12345";
        blockNode.writeBlock(a, sizeof(a), 0);
    }

    for (int i = 9; i >= 0; i--) {
        auto &blockNode = bufferManager.getBlock(MINI_TABLE, "tableName.table", i);
        char a[10];
        blockNode.readBlock(a, sizeof(a), 0);
        cout << a << endl;
    }
```
    

## Code Guide
1. 不要擅自修改类中的public方法（包括方法名、参数和返回值），如有必要更改，请先在群里询问其他人是否会用到该方法。
1. 如果需要添加模块暴露更多的接口，请联系该模块的负责人。
1. 写好自己的模块后，请务必做好单元测试，参考方法如下：
    1. 在主目录下新建一个test文件夹
    1. 在test文件夹中新建一个test.cpp
    1. 在test.cpp中调用你写的模块，编写单元测试代码
    1. cmd中进入test目录，run `g++ test.cpp xxxx.cpp -o test`, run `test.exe` ，(xxxx.cpp是你写的模块)，得到输出结果
1. 文件结构如下：
    - include
        - API.h
        - BufferManager.h
        - definitions.h
        - IndexManager.h
        - Interpreter.h
        - RecordManger.h
        - StmtAfterParse.h
    - db
        - ... (Some db storage files)
    - ...cpp (Some implementations)
    - .gitignore
    - CMakeLists.txt
    - main.cpp
    - test.cpp (ignored)
    - readme.md
1. 为了减少不必要的冲突，养成每次开始写代码前git pull，结束写代码前git push的好习惯

## Usage
1. `g++ main.cpp xxx.cpp xxxxx.cpp -o main`
2. `main.exe`
3. Input some sql statement and enjoy the MiniSql