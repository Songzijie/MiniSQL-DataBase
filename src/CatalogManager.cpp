/*
 * by Zijie Song
 */

#include "CatalogManager.h"

CatalogManager::CatalogManager() {
//    cout << "#Debug Construct CatalogManager" << endl;
    readCatalog();
}

CatalogManager::~CatalogManager() {
//    cout << "#Debug Destroy CatalogManager" << endl;
    writeCatalog();
}

//从磁盘中读入，由构造函数调用
void CatalogManager::readCatalog() {
    int cursor = 0;     //指向当前读入地址
    int blockNum = 0;   //指向下一个操作的block
    int tableSize;      //total table number
    int indexSize;      //total index number
    int numOnB;         //number of table or index on the current block
    int stringLength;   //length of the string
    int attrNum;        //attribute number in a table
    char temp[MaxCharLength];//temp for string
    BufferManager &buffer = BufferManager::GetInstance();

    BufferNode &tableBlock = buffer.getBlock(MINI_CATALOG,CATALOG_FILE_NAME,blockNum);//the first block

    //char forTest[4096];
    //tableBlock.readBlock(forTest,4096,0);
    tableBlock.readBlock(tableSize,SIZE_INT,cursor);
    cursor += SIZE_INT;                             //读入一共有多少个tablenode

    tableBlock.readBlock(numOnB,SIZE_INT,cursor);
    cursor += SIZE_INT;                             //读入该block存了多少个tablenode

    //遍历每个block
    for(int i=0;i < tableSize;i+=numOnB){

        {
            BufferNode &tableBlock = buffer.getBlock(MINI_CATALOG, CATALOG_FILE_NAME, blockNum++);
            if (i != 0) {
                cursor = 0;                             //重置cursor
                tableBlock.readBlock(numOnB, SIZE_INT, cursor);
                cursor += SIZE_INT;                     //读入该block上有多少table
            }
            //遍历这个block，读入table
            for (int j = 0; j < numOnB; j++) {
                TableNode tableToPush;//暂时存储table

                tableBlock.readBlock(stringLength, SIZE_INT, cursor);
                cursor += SIZE_INT;                     //读入tablename的长度

                tableBlock.readBlock(temp, stringLength, cursor);
                cursor += stringLength;
                temp[stringLength] = 0;
                tableToPush.tableName.assign(temp);     //读入tablename

                tableBlock.readBlock(tableToPush.recordNum, SIZE_INT, cursor);
                cursor += SIZE_INT;                     //读入recordNum

                tableBlock.readBlock(attrNum, SIZE_INT, cursor);
                cursor += SIZE_INT;                     //读入attribute number

                //遍历读入attribute
                for (int k = 0; k < attrNum; k++) {
                    Attr attrToPush;//暂时存储attribute

                    tableBlock.readBlock(stringLength, SIZE_INT, cursor);
                    cursor += SIZE_INT;
                    tableBlock.readBlock(temp, stringLength, cursor);
                    cursor += stringLength;
                    temp[stringLength] = 0;
                    attrToPush.attrName.assign(temp);   //读入attrname

                    tableBlock.readBlock(attrToPush.isPrimary, SIZE_BOOL, cursor);
                    cursor += SIZE_BOOL;                //读入isPrimary

                    tableBlock.readBlock(attrToPush.isUnique, SIZE_BOOL, cursor);
                    cursor += SIZE_BOOL;                //读入isUnique

                    tableBlock.readBlock(attrToPush.hasIndex, SIZE_BOOL, cursor);
                    cursor += SIZE_BOOL;                //读入hasIndex

                    tableBlock.readBlock(attrToPush.length, SIZE_INT, cursor);
                    cursor += SIZE_INT;                 //读入length

                    tableBlock.readBlock(attrToPush.offset, SIZE_INT, cursor);
                    cursor += SIZE_INT;                 //读入offset

                    tableBlock.readBlock(attrToPush.type, SIZE_DATATYPE, cursor);
                    cursor += SIZE_DATATYPE;            //读入type

                    tableToPush.attrList.push_back(attrToPush);
                    //index读取完毕，保存进attrlist
                }
                tableList.push_back(tableToPush);       //table读取完毕，保存进tableList
            }

        }
    }

    //read index
    {
        BufferNode &tableBlock = buffer.getBlock(MINI_CATALOG,CATALOG_FILE_NAME,blockNum);//下一个block
        //tableBlock.readBlock(forTest,4096,0);

        cursor = 0;                                     //重置cursor
        tableBlock.readBlock(indexSize,SIZE_INT,cursor);
        cursor += SIZE_INT;                             //How many indexes totally
        tableBlock.readBlock(numOnB,SIZE_INT,cursor);
        cursor += SIZE_INT;                             //How many indexes on this block
    }

    //每次读入一个block上的index
    for(int i=0;i < indexSize;i += numOnB) {
        {
            BufferNode &tableBlock = buffer.getBlock(MINI_CATALOG, CATALOG_FILE_NAME, blockNum++);


            if (i != 0) {
                //下一个block
                cursor = 0;
                tableBlock.readBlock(numOnB, SIZE_INT, cursor);
                cursor += SIZE_INT;                     //get index number on this block

            }
            //读入block上的indexnode
            for (int j = 0; j < numOnB; j++) {

                IndexNode indexToPush;//暂存index

                tableBlock.readBlock(stringLength, SIZE_INT, cursor);
                cursor += SIZE_INT;
                tableBlock.readBlock(temp, stringLength, cursor);
                cursor += stringLength;
                temp[stringLength] = 0;
                indexToPush.tableName.assign(temp);     //read table name

                tableBlock.readBlock(stringLength, SIZE_INT, cursor);
                cursor += SIZE_INT;
                tableBlock.readBlock(temp, stringLength, cursor);
                cursor += stringLength;
                temp[stringLength] = 0;
                indexToPush.indexName.assign(temp);     //read index name

                tableBlock.readBlock(stringLength, SIZE_INT, cursor);
                cursor += SIZE_INT;
                tableBlock.readBlock(temp, stringLength, cursor);
                cursor += stringLength;
                temp[stringLength] = 0;
                indexToPush.attribute.assign(temp);     //read attribute name

                tableBlock.readBlock(indexToPush.column, SIZE_INT, cursor);
                cursor += SIZE_INT;                     //read column

                indexList.push_back(indexToPush);       //push it to the list
            }
        }

        //finish
        //delete[] temp;
    }
}

//写入磁盘，由析构函数调用
void CatalogManager::writeCatalog() {
    int cursor = 0;     //写入位置的指针
    int blockNum = 0;   //操作block的记录
    int numOnB = 0;     //block上写了多少个table或者index
    int numOffset;      //写入numOnB的位置
    int beginOffset;    //上一个table或index的开始位置
    int tableSize = 0;
    BufferManager &buffer = BufferManager::GetInstance();

    BufferNode &tableBlock = buffer.getBlock(MINI_CATALOG,CATALOG_FILE_NAME,blockNum);
    char tempBlock[BLOCK_SIZE + 600];   //暂存

    tableSize = tableList.size();
    TOINT tempBlock = tableSize;
    cursor += SIZE_INT;                             //有多少个tablenode

    numOffset = cursor;
    cursor += SIZE_INT;                             //等待写入该block存了多少个tablenode


    list <TableNode>::iterator tableIterator;

    //遍历tableList，逐个写入
    for(tableIterator = tableList.begin();tableIterator != tableList.end();tableIterator++) {




            beginOffset = cursor;//更新biginOffset
            numOnB++;//更新numOnB

            TOINT (tempBlock + cursor) = tableIterator->tableName.length();
            cursor += SIZE_INT;                         //tablename的长度

            memcpy(tempBlock + cursor, tableIterator->tableName.c_str(), tableIterator->tableName.length());
            cursor += tableIterator->tableName.length();//将string转成char[]存储

            TOINT (tempBlock + cursor) = tableIterator->recordNum;
            cursor += SIZE_INT;                         //存入recordNum

            TOINT (tempBlock + cursor) = tableIterator->attrList.size();
            cursor += SIZE_INT;                         //存入attrNum

            //遍历保存attribute
            list<Attr>::iterator attrIterator;
            for (attrIterator = tableIterator->attrList.begin();
                 attrIterator != tableIterator->attrList.end(); attrIterator++) {

                TOINT (tempBlock + cursor) = attrIterator->attrName.length();
                cursor += SIZE_INT;                     //attribute name 的长度

                memcpy(tempBlock + cursor, attrIterator->attrName.c_str(), attrIterator->attrName.length());
                cursor += attrIterator->attrName.length();
                //attribute name

                TOBOOL (tempBlock + cursor) = attrIterator->isPrimary;
                cursor += SIZE_BOOL;                    //isPrimary
                TOBOOL (tempBlock + cursor) = attrIterator->isUnique;
                cursor += SIZE_BOOL;                    //isUnique
                TOBOOL (tempBlock + cursor) = attrIterator->hasIndex;
                cursor += SIZE_BOOL;                    //hasIndex
                TOINT (tempBlock + cursor) = attrIterator->length;
                cursor += SIZE_INT;                     //length
                TOINT (tempBlock + cursor) = attrIterator->offset;
                cursor += SIZE_INT;                     //offset
                TODATATYPE (tempBlock + cursor) = attrIterator->type;
                cursor += SIZE_DATATYPE;                //type
            }

            if (cursor >= BLOCK_SIZE) {
                //超过一个block的大小，将之前的table存入block，新建block存储最后一个table

                TOINT (tempBlock + numOffset) = numOnB - 1; //保存了多少个table在这个block上


                {
                    BufferNode & tableBlock = buffer.getBlock(MINI_CATALOG, CATALOG_FILE_NAME, blockNum++);
                    tableBlock.writeBlock(tempBlock, BLOCK_SIZE, 0);
                }

                //写入

                //tableBlock = buffer.getBlock(MINI_CATALOG, CATALOG_FILE_NAME, blockNum++);//新的block


                memcpy(tempBlock + SIZE_INT, tempBlock + beginOffset, cursor - beginOffset);//将table转存
                cursor = cursor + SIZE_INT - beginOffset;//更新cursor
                numOnB = 1;
                numOffset = 0;
            }


    }
    TOINT (tempBlock+numOffset) = numOnB;

    {
        BufferNode & tableBlock = buffer.getBlock(MINI_CATALOG, CATALOG_FILE_NAME, blockNum++);
        tableBlock.writeBlock(tempBlock, BLOCK_SIZE, 0);
    }




    //write indexNode

    BufferNode &indexBlock = buffer.getBlock(MINI_CATALOG,CATALOG_FILE_NAME,blockNum);
    //tableBlock = buffer.getBlock(MINI_CATALOG,CATALOG_FILE_NAME,blockNum++);
    cursor = 0;
    numOnB = 0;
    int indexSize = indexList.size();
    TOINT tempBlock = indexSize;
    cursor += SIZE_INT;                             //how many indexNode

    numOffset = cursor;
    cursor += SIZE_INT;                             //how many indexnode on this block

    list<IndexNode>::iterator indexIterator = indexList.begin();
    //遍历写入indexNode
    for(indexIterator;indexIterator != indexList.end();indexIterator++){

        beginOffset = cursor;
        numOnB++;

        TOINT (tempBlock+cursor) = indexIterator->tableName.length();
        cursor += SIZE_INT;
        memcpy(tempBlock+cursor,indexIterator->tableName.c_str(),indexIterator->tableName.length());
        cursor += indexIterator->tableName.length();//tablename

        TOINT (tempBlock+cursor) = indexIterator->indexName.length();
        cursor += SIZE_INT;
        memcpy(tempBlock+cursor,indexIterator->indexName.c_str(),indexIterator->indexName.length());
        cursor += indexIterator->indexName.length();//indexname

        TOINT (tempBlock+cursor) = indexIterator->attribute.length();
        cursor += SIZE_INT;
        memcpy(tempBlock+cursor,indexIterator->attribute.c_str(),indexIterator->attribute.length());
        cursor += indexIterator->attribute.length();//attribute name

        TOINT (tempBlock+cursor) = indexIterator->column;
        cursor += SIZE_INT;                         //column

        if (cursor >= BLOCK_SIZE) {
            TOINT (tempBlock+numOffset) = numOnB-1;
            {
                BufferNode & tableBlock = buffer.getBlock(MINI_CATALOG, CATALOG_FILE_NAME, blockNum++);
                tableBlock.writeBlock(tempBlock, BLOCK_SIZE, 0);
            }

            memcpy(tempBlock+SIZE_INT,tempBlock + beginOffset,cursor - beginOffset);
            cursor = cursor + SIZE_INT - beginOffset;
            numOnB = 1;
            numOffset = 0;
        }
    }

    TOINT (tempBlock+numOffset) = numOnB;
    {
        BufferNode & tableBlock = buffer.getBlock(MINI_CATALOG, CATALOG_FILE_NAME, blockNum++);
        tableBlock.writeBlock(tempBlock, BLOCK_SIZE, 0);
    }  //写入

    //finish
    //delete[] tempBlock;
}

// 1/true 成功, 0/false失败
// -1 不存在

//table
bool CatalogManager::createTable(string tableName, list <Attr> attrList){
    TableNode temp(tableName,attrList,0);//创建
    bool ifExist = existTable(tableName);//是否已存在相同名字的table
    if(ifExist){
        //存在同名table
    } else {
        tableList.push_back(temp);//不存在同名table，push list
        writeCatalog();
    }
    return !ifExist;//返回
}

bool CatalogManager::existTable(string tableName){
    bool ifExist = false;//默认不存在

    findTableNodeByName(tableName,ifExist);//ifFind
    return ifExist;

}

bool CatalogManager::deleteTable(string tableName){
    bool      ifExist = false;

    list<TableNode>::iterator tableIte;
    for(tableIte = tableList.begin();tableIte != tableList.end();tableIte++){
        if(tableIte->tableName == tableName){
            tableList.erase(tableIte);
            ifExist = true;
            writeCatalog();
            break;
        }
    }
    return ifExist;
}

TableNode CatalogManager::findTable(string tableName) {

    list<TableNode>::iterator v = tableList.begin();

    for (int i = 0; i < tableList.size(); i++, v++) {

        if (v->tableName == tableName)

            return *v;

    }

    //异常

}

//打印tables
void CatalogManager::showTables(){
    list<TableNode>::iterator tableIterator;
    bool ifEmpty = true;

    for(tableIterator = tableList.begin();tableIterator != tableList.end();tableIterator++){
        ifEmpty = false;
        cout << tableIterator->tableName << endl;

    }
    if(ifEmpty){
        cout << "There are no tables." << endl;
    }
}

//打印indexes
void CatalogManager::showIndexes(){
    list<IndexNode>::iterator indexIterator;
    bool ifEmpty = true;

    for(indexIterator = indexList.begin();indexIterator != indexList.end();indexIterator++){
        ifEmpty = false;
        cout << indexIterator->indexName << endl;

    }
    if(ifEmpty){
        cout << "There are no indexes!" << endl;
    }
}

//index
bool CatalogManager::createIndex(string indexName, string tableName, int column){
    Attr attrTemp = IdtoAttr(tableName,column);
    IndexNode temp(tableName,indexName,attrTemp.attrName,column);
    bool ifExist = existIndex(indexName);
    if(ifExist){
        //存在同名index，不创建
    } else {
        indexList.push_back(temp);//不存在同名，则push进indexList中
        writeCatalog();
    }
    return !ifExist;//返回成功与否

}

bool CatalogManager::existIndex(string indexName){
    list <IndexNode>::iterator indexIterator;
    //遍历indexList，indexName是否相同
    for(indexIterator = indexList.begin();indexIterator != indexList.end();indexIterator++){
        if(indexIterator->indexName == indexName){
            return true;//存在返回true
        }
    }
    return false;//没找到，返回false
}

bool CatalogManager::deleteIndex(string indexName){
    bool ifExist = false;

    list<IndexNode>::iterator indexIte;
    for(indexIte = indexList.begin();indexIte != indexList.end();indexIte++){
        if(indexIte->indexName == indexName){
            ifExist = true;
            indexList.erase(indexIte);
            writeCatalog();
            break;
        }
    }

    return ifExist;//返回是否找到
}

list <string> CatalogManager::getIndexNamesOfTable(string tableName){
    list <string> temp;//返回的list
    list <IndexNode>::iterator indexIterator;//list指针
    //遍历indexList，将table的所有index push进temp容器中
    for(indexIterator = indexList.begin();indexIterator != indexList.end();indexIterator++){
        if(indexIterator->tableName == tableName){
            temp.push_back(indexIterator->indexName);
        }
    }
    return temp;//返回
}
//通过table中的attribute ID找到对应indexName
string CatalogManager::getIndexNameByAttrID(string tableName, int column){
    list <IndexNode>::iterator indexIterator;//list 指针
    //遍历indexList
    for(indexIterator = indexList.begin();indexIterator != indexList.end();indexIterator++){
        if(indexIterator->tableName == tableName && indexIterator->column == column){
            return indexIterator->indexName;//找到了，返回indexName
        }
    }
    return "No such index";//遍历没找到，返回空字符串
}

//通过indexName，返回存储此index的文件名
string CatalogManager::getIndexFileByIndexName(string indexName) {
    string fileName = indexName;
    bool ifExist = false;
    IndexNode &temp = findIndexNodeByName(indexName,ifExist);
    if(ifExist){
        fileName +="_";
        fileName +=temp.tableName;
        fileName +=".index";
        //indexName_tableName.index
    }
    return fileName;
}

IndexNode CatalogManager::findIndex(string indexName, string tableName){
    list <IndexNode>::iterator indexIterator;
    //遍历indexList，找到indexNode
    for(indexIterator = indexList.begin();indexIterator != indexList.end();indexIterator++){

        if(indexIterator->tableName == tableName && indexIterator->indexName == indexName){
            //find it
            return *indexIterator;
        }
    }
}

bool CatalogManager::existIndexOnTable(string indexName, string tableName){
    bool ifExist = false;
    IndexNode &temp = findIndexNodeByName(indexName,ifExist);
    if(ifExist){
        //存在index
        if(temp.tableName == tableName){
            //在表上
            return true;
        }
    }
    //default
    return false;
}

bool CatalogManager::existIndexOnAttrID(string tableName, int column){

    list <IndexNode>::iterator indexIterator = indexList.begin();
    //遍历
    for(indexIterator;indexIterator != indexList.end();indexIterator++){

        if(indexIterator->tableName == tableName && indexIterator->column == column){
            //exactly the index
            return true;
        }
    }
    //default
    return false;
}





//attribute
bool CatalogManager::existAttr(string tableName, string attrName){
    bool ifExist = false;
    TableNode &temp = findTableNodeByName(tableName,ifExist);//找到这个table
    //没找到这个table，return false
    if(ifExist){
        //找到了这个table，
        Attr tempAttr;
        Attr &attrPtr = tempAttr;

        return findAttrByName(temp,attrName,attrPtr);//是否找到table中的attribute
    }
    return false;
}

bool CatalogManager::isAttrUnique(string tableName, string attrName){

    bool ifTableExist = false;
    TableNode &temp = findTableNodeByName(tableName,ifTableExist);

    if(ifTableExist){
        //存在table
        Attr emptyNode;
        Attr &tempAttr = emptyNode;

        if(findAttrByName(temp,attrName,tempAttr)){
            //找到attr
            return tempAttr.isUnique;
        }
    }
    return false;
}

DataType CatalogManager::getAttrType(string tableName, string attrName){

    bool ifTableExist = false;
    TableNode &temp = findTableNodeByName(tableName,ifTableExist);//find table

    if(ifTableExist){
        //存在table
        Attr emptyNode;
        Attr &tempAttr = emptyNode;//initialize

        if(findAttrByName(temp,attrName,tempAttr)){
            //存在attribute
            return tempAttr.type;
        }
    }
}

int CatalogManager::getAttrOffset(string tableName, string attrName){

    bool ifTableExist = false;
    TableNode &temp = findTableNodeByName(tableName,ifTableExist);//find table

    if(ifTableExist){
        //存在table
        Attr emptyAttr;
        Attr &attrTemp = emptyAttr;

        if(findAttrByName(temp,attrName,attrTemp)){
            //存在attribute
            return attrTemp.offset;
        }
    }
    return -1;//不存在
}

int CatalogManager::getAttrLength(string tableName, string attrName){

    bool ifTableExist = false;
    TableNode &temp = findTableNodeByName(tableName,ifTableExist);//find table

    if(ifTableExist){
        //存在table
        Attr emptyAttr;
        Attr &attrTemp = emptyAttr;

        if(findAttrByName(temp,attrName,attrTemp)){
            //存在attribute
            return attrTemp.length;
        }
    }
    return -1;//不存在
}

vector<Attr> CatalogManager::getAttrList(string tableName){

    vector <Attr> attrVector;
    bool ifExist = false;
    TableNode &temp = findTableNodeByName(tableName,ifExist);//find table

    if(ifExist){
        //存在table
        list<Attr>::iterator attrIterator;
        //遍历写入Vector
        for(attrIterator = temp.attrList.begin();attrIterator != temp.attrList.end();attrIterator++){
            attrVector.push_back(*attrIterator);
        }
    }
    //返回
    return attrVector;
}

Attr CatalogManager::getAttrInfo(string tableName, string attrName){
    bool ifTableExist = false;
    TableNode &temp = findTableNodeByName(tableName,ifTableExist);

    if(ifTableExist){

        Attr emptyAttr;
        Attr &attrTemp = emptyAttr;

        if(findAttrByName(temp,attrName,attrTemp)){
            return attrTemp;
        }
    }
    //不存在

}

int CatalogManager::AttrtoId(string &tablename, string &attrname){
    TableNode table = findTable(tablename);
    int i=-1;
    list<Attr>::iterator attr = table.attrList.begin();

    for(attr;attr != table.attrList.end();attr++){
        i++;
        if(attr->attrName == attrname){
            break;
        }
    }

    return i;
}

Attr CatalogManager::IdtoAttr(string &tablename, int id) {

    TableNode table = findTable(tablename);

    list<Attr>::iterator attr = table.attrList.begin();
    //遍历
    for (id; id > 0; id--) {

        attr++;

    }

    return *attr;

}

//record
//返回条目数量
int CatalogManager::getRecordNum(string tableName){
    bool      ifExist = false;
    TableNode &temp   = findTableNodeByName(tableName,ifExist);

    if(ifExist){
        //存在table，return
        return temp.recordNum;
    } else {
        return -1;//不存在，返回-1
    }

}
//删除记录数量
bool CatalogManager::deleteRecord(string tableName, int num){
    bool      ifExist = false;
    TableNode &temp   = findTableNodeByName(tableName,ifExist);
    if(ifExist){
        //table 存在
        if(temp.recordNum - num <0){
            //recordNum为负数，置0
            temp.recordNum = 0;
        }else{
            //正常删除
            temp.recordNum -= num;
        }
        writeCatalog();
    }
    return ifExist;
}
//添加记录数量
bool CatalogManager::addRecord(string tableName, int num){
    bool      ifExist = false;
    TableNode &temp   = findTableNodeByName(tableName,ifExist);
    if(ifExist){
        //存在这样的table
        temp.recordNum += num;
        writeCatalog();
    }
    return ifExist;
}

//private functions
//find the table according to its name,if no such a table ,return ifExist = false
TableNode &CatalogManager::findTableNodeByName(string tableName,bool &ifExist) {
    list <TableNode>::iterator tableIterator;
    for(tableIterator = tableList.begin();tableIterator != tableList.end();++tableIterator){
        if(tableIterator->tableName == tableName){
            ifExist = true;
            return *tableIterator;
        }
    }
    ifExist = false;

}
//find index according to its name,if no such an index, return ifExist = false
IndexNode &CatalogManager::findIndexNodeByName(string indexName,bool &ifExist){
    list <IndexNode>::iterator indexIterator;
    for(indexIterator = indexList.begin();indexIterator != indexList.end();indexIterator++){
        if(indexIterator->indexName == indexName){
            ifExist = true;
            return *indexIterator;
        }
    }
    ifExist = false;
}
//if exist the attr,return true.And resultAttr = it.OtherWise, return false.
bool CatalogManager::findAttrByName(TableNode &table,string attrName,Attr &resultAttr){
    list <Attr>::iterator attrIterator;
    for(attrIterator = table.attrList.begin();attrIterator != table.attrList.end();attrIterator++){
        if(attrIterator->attrName == attrName){
            resultAttr = *attrIterator;
            return true;
        }
    }
    return false;
}