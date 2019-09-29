/*
 * by Zijie Song
 */

#include "RecordManager.h"

RecordManager::RecordManager()//构造函数
{
}

RecordManager::~RecordManager()//析构函数
{
}

int RecordManager::getRecordlength(vector<Attr> &attrs) {
    //记录总长为最后一个属性的位偏移量加上该属性的长度(如果最后一位是字符则还要加上4位的字符长度记录位)
    if (attrs[attrs.size() - 1].type == MINI_STRING)
        return attrs[attrs.size() - 1].offset + attrs[attrs.size() - 1].length + sizeof(int);
    else
        return attrs[attrs.size() - 1].offset + attrs[attrs.size() - 1].length;
}

bool RecordManager::Isempty(string fileName, vector<Attr> &attrs, int num, int offset) {
    BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, fileName, num);
    for (int i = 0; i < attrs.size(); i++) {//遍历该记录的所有值
        switch (attrs[i].type) {
            case MINI_FLOAT: {
                float f;
                thisblock.readBlock(f, sizeof(float), offset + attrs[i].offset);
                if (f != 0.0) return false;//如果有值不为0，则该记录不为空
                break;
            }
            case MINI_INT: {
                int n;
                thisblock.readBlock(n, sizeof(int), offset + attrs[i].offset);
                if (n != 0) return false;//如果有值不为0，则该记录不为空
                break;
            }
            case MINI_STRING: {
                int n;
                thisblock.readBlock(n, sizeof(int), offset + attrs[i].offset);
                if (n != 0) return false;//如果该字符长度记录位不为0，则该记录不为空
                break;
            }
            default:
                break;
        }
    }
    return true;
}

bool
RecordManager::Issatisfied(string fileName, vector<Condition> &conditions, vector<Attr> &attrs, int num, int offset) {
    if (Isempty(fileName, attrs, num, offset)) return false;//如果该记录为空则必定不满足筛选条件

    BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, fileName, num);
    for (int i = 0; i < conditions.size(); i++) {//遍历所有筛选条件
        int column = conditions[i].id;
        int tmpoffset = offset + attrs[column].offset;//该列的位偏移量

        if (conditions[i].op == NO) continue;//如果该条件不存在，则跳转至下一个条件

        switch (attrs[column].type) {
            case MINI_FLOAT: {
                float f;
                thisblock.readBlock(f, sizeof(float), tmpoffset);
                switch (conditions[i].op) {
                    case EQU:
                        if (!(f == conditions[i].value.f)) return false;
                        break;// =
                    case NE:
                        if (!(f != conditions[i].value.f)) return false;
                        break;// <>
                    case LT:
                        if (!(f < conditions[i].value.f)) return false;
                        break;// <
                    case GT:
                        if (!(f > conditions[i].value.f)) return false;
                        break;// >
                    case LE:
                        if (!(f <= conditions[i].value.f)) return false;
                        break;// <=
                    case GE:
                        if (!(f >= conditions[i].value.f)) return false;
                        break;// >=
                    default:
                        break;
                }
                break;
            }
            case MINI_INT: {
                int n;
                thisblock.readBlock(n, sizeof(int), tmpoffset);
                switch (conditions[i].op) {
                    case EQU:
                        if (!(n == conditions[i].value.n)) return false;
                        break;// =
                    case NE:
                        if (!(n != conditions[i].value.n)) return false;
                        break;// <>
                    case LT:
                        if (!(n < conditions[i].value.n)) return false;
                        break;// <
                    case GT:
                        if (!(n > conditions[i].value.n)) return false;
                        break;// >
                    case LE:
                        if (!(n <= conditions[i].value.n)) return false;
                        break;// <=
                    case GE:
                        if (!(n >= conditions[i].value.n)) return false;
                        break;// >=
                    default:
                        break;
                }
                break;
            }
            case MINI_STRING: {
                int n;
                thisblock.readBlock(n, sizeof(int), tmpoffset);//先读出该字符实际长度
                char *str = new char[n];
                thisblock.readBlock(*str, n, tmpoffset + sizeof(int));
                string s = "";
                for (int l = 0; l < n; l++)
                    s += str[l];
                switch (conditions[i].op) {
                    case EQU:
                        if (!(s == conditions[i].value.s)) return false;
                        break;// =
                    case NE:
                        if (!(s != conditions[i].value.s)) return false;
                        break;// <>
                    case LT:
                        if (!(s < conditions[i].value.s)) return false;
                        break;// <
                    case GT:
                        if (!(s > conditions[i].value.s)) return false;
                        break;// >
                    case LE:
                        if (!(s <= conditions[i].value.s)) return false;
                        break;// <=
                    case GE:
                        if (!(s >= conditions[i].value.s)) return false;
                        break;// >=
                    default:
                        break;
                }
                delete[] str;
                break;
            }
            default:
                break;
        }
    }
    return true;//均符合要求
}

void RecordManager::Output(string fileName, vector<Attr> &attrs, vector<int> &selectColumns, int num, int offset) {
    BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, fileName, num);
    for (int i = 0; i < selectColumns.size(); i++) {//遍历所有选中的列
        if (i > 0)
            cout << "  ";
        cout << "|";
        int thisoffset = offset + attrs[selectColumns[i]].offset;//该列在对应的块中的总偏移量
        switch (attrs[selectColumns[i]].type) {
            case MINI_FLOAT: {
                float f;
                thisblock.readBlock(f, sizeof(float), thisoffset);
                cout.width(15);//浮点型输出宽度为15位，以便对齐
                cout << setprecision(8) << f;//精度为8位
                break;
            }
            case MINI_INT: {
                int n;
                thisblock.readBlock(n, sizeof(int), thisoffset);
                cout.width(15);//整型输出宽度为15位
                cout << n;
                break;
            }
            case MINI_STRING: {
                int n;
                thisblock.readBlock(n, sizeof(int), thisoffset);
                char *str = new char[n];
                thisblock.readBlock(*str, n, thisoffset + sizeof(int));
                cout << "  ";
                string s = "";
                for (int l = 0; l < n; l++)
                    s += str[l];

                const int strWidth = 13;
                if (n > strWidth) {
                    s = s.substr(0, strWidth - 3);
                    s += "...";
                }
                cout.width(strWidth);//字符输出宽度为该位长度加5
                cout << s;
                delete[] str;
                break;
            }
            default:
                break;
        }
    }
    drawLine(selectColumns.size());
}

void RecordManager::DeleteRecord(string fileName, vector<Attr> &attrs, int num, int offset) {
    BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, fileName, num);
    for (int i = 0; i < attrs.size(); i++) {//遍历所有属性，将每一条均置空
        switch (attrs[i].type) {
            case MINI_FLOAT: {
                float f = 0.0;
                thisblock.writeBlock(f, sizeof(float), offset + attrs[i].offset);
                break;
            }
            case MINI_INT: {
                int n = 0;
                thisblock.writeBlock(n, sizeof(int), offset + attrs[i].offset);
                break;
            }
            case MINI_STRING: {
                int n = 0;//对于字符属性的列，直接将字符长度记录位置为0，即是删除
                thisblock.writeBlock(n, sizeof(int), offset + attrs[i].offset);
                break;
            }
            default:
                break;
        }
    }
}

IndexInfo RecordManager::getPosition(string fileName, vector<Attr> &attrs, vector<AttrWithValue> &AttrValues) {
    BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, fileName, 0);
    int Blocknum;
    firstblock.readBlock(Blocknum, sizeof(int), 0);
    int Recordlength;
    firstblock.readBlock(Recordlength, sizeof(int), 4);

    bool IsFindposition = false;//记录是否已找到插入位置
    IndexInfo position;
    IndexInfo insertfail;//插入无效的索引info，对应为块号为0，偏移量为0
    insertfail.blockNum = 0;
    insertfail.offset = 0;

    vector<int> column;
    column.clear();
    for (int k = 0; k < attrs.size(); k++)//找到所有需要判定Unique的列
        if ((attrs[k].isPrimary) || (attrs[k].isUnique)) column.push_back(k);

    for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//遍历所有的块
        BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, fileName, Blockoffset);
        int recordnumber;
        int offset = 0;
        thisblock.readBlock(recordnumber, sizeof(int), offset);
        offset += sizeof(int);
        int tmpoffset = offset;

        for (int recnum = 0; recnum < recordnumber; recnum++) {//遍历该块的所有记录
            if ((Isempty(fileName, attrs, Blockoffset, tmpoffset)) && (IsFindposition = false)) {
                //如果该记录为空且还为找到插入位置，则记录下该位为插入位置
                IsFindposition = true;
                position.blockNum = Blockoffset;
                position.offset = tmpoffset;
            } else {//如果该记录不为空，则检测与插入记录的Unique属性是否出现重复
                for (int k = 0; k < column.size(); k++) {
                    switch (attrs[column[k]].type) {
                        case MINI_FLOAT: {
                            float f;
                            thisblock.readBlock(f, sizeof(float), tmpoffset + attrs[column[k]].offset);
                            if (f == AttrValues[column[k]].value.f) return insertfail;//出现重复则返回插入无效的索引info
                            break;
                        }
                        case MINI_INT: {
                            int n;
                            thisblock.readBlock(n, sizeof(int), tmpoffset + attrs[column[k]].offset);
                            if (n == AttrValues[column[k]].value.n) return insertfail;//出现重复则返回插入无效的索引info
                            break;
                        }
                        case MINI_STRING: {
                            int n;
                            thisblock.readBlock(n, sizeof(int), tmpoffset + attrs[column[k]].offset);
                            if (n == AttrValues[column[k]].value.s.length()) {
                                char *str = new char[n];
                                thisblock.readBlock(*str, n, tmpoffset + attrs[column[k]].offset + sizeof(int));
                                string s = "";

                                for (int l = 0; l < n; l++)
                                    s += str[l];
                                if (s == AttrValues[column[k]].value.s) return insertfail;//出现重复则返回插入无效的索引info
                                delete[] str;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            tmpoffset += Recordlength;
        }
        if (((recordnumber + 1) * Recordlength + 4 <= BLOCK_SIZE) && (IsFindposition == false)) {
            //如果遍历完该块所有记录仍未找到插入位置，则判定该块末尾是否还能添加记录
            IsFindposition = true;
            position.blockNum = Blockoffset;
            position.offset = recordnumber * Recordlength + 4;
            recordnumber++;//更新该块记录总数
            thisblock.writeBlock(recordnumber, sizeof(int), 0);
        }
    }
    if (IsFindposition == false) {//如果遍历完所有块仍未找到插入位置，则新开一个块插入
        IsFindposition = true;
        Blocknum++;//更新总块数
        firstblock.writeBlock(Blocknum, sizeof(int), 0);
        BufferNode &newblock = bufferManager.getBlock(MINI_TABLE, fileName, Blocknum);
        int newrecordnumber = 1;//更新新块记录数为1
        newblock.writeBlock(newrecordnumber, sizeof(int), 0);
        position.blockNum = Blocknum;
        position.offset = 4;
    }
    return position;//返回最终插入位置的索引info
}

vector<AttrWithValue> RecordManager::selectAttr(string tableName, vector<Attr> &attrs, int column) {
    string filename = tableName + ".table";
    BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);
    int Blocknum = 0;
    firstblock.readBlock(Blocknum, sizeof(int), 0);
    int Recordlength = 0;
    firstblock.readBlock(Recordlength, sizeof(int), 4);
    vector<AttrWithValue> finalattrwithvalue;
    finalattrwithvalue.clear();//初始化结果为空
    if (Blocknum == 0 || column > attrs.size()) //如果文件中没有块或是列号超出范围，直接返回空
        return finalattrwithvalue;

    int columnoffset = attrs[column].offset;//该列的位偏移量
    for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//遍历所有块
        BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, Blockoffset);
        int offset = 0;
        int recordnumber = 0;
        thisblock.readBlock(recordnumber, sizeof(int), offset);
        offset += sizeof(int);

        for (int recnum = 0; recnum < recordnumber; recnum++) {//遍历块中所有记录
            int tmpoffset = offset + columnoffset;
            AttrWithValue *tmprecord = new AttrWithValue;
            tmprecord->type = attrs[column].type;
            tmprecord->length = attrs[column].length;
            tmprecord->isNull = 0;
            tmprecord->isPrimary = attrs[column].isUnique;
            tmprecord->isUnique = attrs[column].isPrimary;
            switch (attrs[column].type) {
                case MINI_FLOAT: {
                    float f;
                    thisblock.readBlock(f, sizeof(float), tmpoffset);
                    tmprecord->value.f = f;//记录下该记录对应属性的值
                    break;
                }
                case MINI_INT: {
                    int n;
                    thisblock.readBlock(n, sizeof(int), tmpoffset);
                    tmprecord->value.n = n;//记录下该记录对应属性的值
                    break;
                }
                case MINI_STRING: {
                    int n;
                    thisblock.readBlock(n, sizeof(int), tmpoffset);
                    char *str = new char[n];
                    thisblock.readBlock(*str, n, tmpoffset + sizeof(int));
                    string s = "";
                    for (int l = 0; l < n; l++)
                        s += str[l];
                    tmprecord->value.s = s;//记录下该记录对应属性的值
                    delete[] str;
                    break;
                }
                default:
                    break;
            }
            offset += Recordlength;
            finalattrwithvalue.push_back(*tmprecord);
            delete tmprecord;
        }
    }
    return finalattrwithvalue;
}


int
RecordManager::select(string tableName, vector<Condition> &conditions, vector<Attr> &attrs, vector<int> &selectColumns,
                      vector<IndexInfo> &indexes) {
    string filename = tableName + ".table";
    int count = 0;

    //output column names
    bool first = true;
    drawLine(selectColumns.size(), true);
    for (auto selectColumn:selectColumns) {
        if (!first)
            cout << "  ";
        else
            first = false;
        cout << "|";
        string name = attrs[selectColumn].attrName;
        const int strWidth = 13;
        if (name.length() > strWidth) {
            name[strWidth - 3] = '.';
            name[strWidth - 2] = '.';
            name[strWidth - 1] = '.';
            name[strWidth] = '\0';
        }
        cout << setw(15) << name;
    }
    drawLine(selectColumns.size());

    if (indexes.size()) {//如果有索引项
        for (int i = 0; i < indexes.size(); i++)
            if (Issatisfied(filename, conditions, attrs, indexes[i].blockNum, indexes[i].offset)) {
                //如果该记录满足条件，则计数加1同时输出该记录的选中列
                count++;
                Output(filename, attrs, selectColumns, indexes[i].blockNum, indexes[i].offset);
            }
    } else {
        BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);//取出该文件第零个块
        int Blocknum = 0;
        int Recordlength = 0;
        firstblock.readBlock(Blocknum, sizeof(int), 0);//从第零个块中读出该文件的总块数
        firstblock.readBlock(Recordlength, sizeof(int), 4);//从第零个块中读出该文件单条记录的长度
        for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//遍历所有的块
            BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, Blockoffset);
            int offset = 0;
            int recordnumber = 0;
            thisblock.readBlock(recordnumber, sizeof(int), offset);
            offset += sizeof(int);

            for (int recnum = 0; recnum < recordnumber; recnum++) {//遍历所有的记录
                int thisoffset = offset + recnum * Recordlength;
                if (Issatisfied(filename, conditions, attrs, Blockoffset, thisoffset)) {
                    //如果该记录满足条件，则计数加1同时输出该记录的选中列
                    count++;
                    Output(filename, attrs, selectColumns, Blockoffset, thisoffset);
                }
            }
        }
    }
    return count;
}

vector<IndexInfo> RecordManager::selectIndexInfo(string tableName) {
    string filename = tableName + ".table";
    BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);//取出该文件第零个块
    int Blocknum = 0;
    int Recordlength = 0;
    firstblock.readBlock(Blocknum, sizeof(int), 0);//从第零个块中读出该文件的总块数
    firstblock.readBlock(Recordlength, sizeof(int), 4);//从第零个块中读出该文件单条记录的长度
    vector<IndexInfo> AllIndex;
    AllIndex.clear();

    for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//遍历所有块
        BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, Blockoffset);
        int recordnumber;
        int offset = 0;
        thisblock.readBlock(recordnumber, sizeof(int), offset);
        offset += sizeof(int);

        for (int recnum = 0; recnum < recordnumber; recnum++) {//遍历所有记录
            IndexInfo *ThisIndex = new IndexInfo;
            ThisIndex->blockNum = Blockoffset;
            ThisIndex->offset = offset;
            AllIndex.push_back(*ThisIndex);
            delete ThisIndex;
            offset += Recordlength;
        }
    }
    return AllIndex;
}

IndexInfo RecordManager::insert(string tableName, vector<Attr> &attrs, vector<AttrWithValue> &AttrValues) {
    string filename = tableName + ".table";
    BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);//取出该文件第零个块
    int Blocknum = 0;
    int Recordlength = 0;
    firstblock.readBlock(Blocknum, sizeof(int), 0);//从第零个块中读出该文件的总块数
    firstblock.readBlock(Recordlength, sizeof(int), 4);//从第零个块中读出该文件单条记录的长度
    if (Blocknum == 0 && Recordlength == 0) {//如果读得记录长度为零说明未初始化，则初始化该文件的单条记录长度
        Recordlength = getRecordlength(attrs);
        firstblock.writeBlock(Blocknum, sizeof(int), 0);
        firstblock.writeBlock(Recordlength, sizeof(int), 4);
    }

    IndexInfo position = getPosition(filename, attrs, AttrValues);//获得当前记录可以插入的位置
    if (position.blockNum) {//如果该位置为有效位置则在该位置实现插入
        BufferNode &insertblock = bufferManager.getBlock(MINI_TABLE, filename, position.blockNum);
        int insertoffset = position.offset;
        for (int i = 0; i < AttrValues.size(); i++) {
            switch (AttrValues[i].type) {
                case MINI_FLOAT: {
                    float f;
                    if (AttrValues[i].isNull)//如果插入的记录该属性为空，则插入0
                        f = 0.0;
                    else
                        f = AttrValues[i].value.f;
                    insertblock.writeBlock(f, sizeof(float), insertoffset);
                    insertoffset += sizeof(float);
                    break;
                }
                case MINI_INT: {
                    int n;
                    if (AttrValues[i].isNull)//如果插入的记录该属性为空，则插入0
                        n = 0;
                    else
                        n = AttrValues[i].value.n;
                    insertblock.writeBlock(n, sizeof(int), insertoffset);
                    insertoffset += sizeof(int);
                    break;
                }
                case MINI_STRING: {
                    string s;
                    if (AttrValues[i].isNull)//如果插入的记录该属性为空，则插入空字符
                        s = "";
                    else
                        s = AttrValues[i].value.s;
                    int n = s.length();
                    insertblock.writeBlock(n, sizeof(int), insertoffset);
                    insertoffset += sizeof(int);
                    const char *str = s.c_str();
                    insertblock.writeBlock(*str, n, insertoffset);
                    insertoffset += AttrValues[i].length;
                    break;
                }
                default:
                    break;
            }
        }
    }
    return position;//返回该位置的索引info
}

vector<vector<AttrWithValue> >
RecordManager::deleteTuple(string tableName, vector<Condition> &conditions, vector<Attr> &attrs,
                           vector<IndexInfo> &indexes) {
    string filename = tableName + ".table";
    vector<vector<AttrWithValue> > Deleted;
    Deleted.clear();
    if (indexes.size()) {//如果有索引项
        for (int i = 0; i < indexes.size(); i++)//遍历所有给出的索引项
            if (Issatisfied(filename, conditions, attrs, indexes[i].blockNum, indexes[i].offset)) {
                //如果该项满足筛选条件
                BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, indexes[i].blockNum);
                int thisoffset = indexes[i].offset;
                vector<AttrWithValue> thisrecord;
                thisrecord.clear();

                for (int j = 0; j < attrs.size(); j++) {//遍历所有属性项
                    AttrWithValue *thisattrs = new AttrWithValue;
                    thisattrs->isNull = 0;
                    thisattrs->isPrimary = attrs[j].isPrimary;
                    thisattrs->isUnique = attrs[j].isUnique;
                    thisattrs->length = attrs[j].length;
                    thisattrs->type = attrs[j].type;
                    switch (attrs[j].type) {
                        case MINI_FLOAT: {
                            float f;
                            thisblock.readBlock(f, sizeof(float), thisoffset);
                            thisattrs->value.f = f;//记录下该属性的值
                            thisoffset += sizeof(float);
                            break;
                        }
                        case MINI_INT: {
                            int n;
                            thisblock.readBlock(n, sizeof(int), thisoffset);
                            thisattrs->value.n = n;//记录下该属性的值
                            thisoffset += sizeof(int);
                            break;
                        }
                        case MINI_STRING: {
                            int n;
                            thisblock.readBlock(n, sizeof(int), thisoffset);
                            thisoffset += sizeof(int);
                            char *str = new char[n];
                            thisblock.readBlock(*str, n, thisoffset);
                            string s = "";
                            for (int l = 0; l < n; l++)
                                s += str[l];
                            thisattrs->value.s = s;//记录下该属性的值
                            thisoffset += attrs[j].length;
                            delete[] str;
                            break;
                        }
                        default:
                            break;
                    }
                    thisrecord.push_back(*thisattrs);
                    delete thisattrs;
                }
                Deleted.push_back(thisrecord);
                DeleteRecord(filename, attrs, indexes[i].blockNum, indexes[i].offset);//删除该属性
            }
    } else {
        BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);//取出该文件第零个块
        int Blocknum = 0;
        int Recordlength = 0;
        firstblock.readBlock(Blocknum, sizeof(int), 0);//从第零个块中读出该文件的总块数
        firstblock.readBlock(Recordlength, sizeof(int), 4);//从第零个块中读出该文件单条记录的长度
        for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//遍历所有的块
            BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, Blockoffset);
            int offset = 0;
            int recordnumber = 0;
            thisblock.readBlock(recordnumber, sizeof(int), offset);
            offset += sizeof(int);

            for (int recnum = 0; recnum < recordnumber; recnum++) {//遍历所有的记录
                int thisoffset = offset + recnum * Recordlength;
                if (Issatisfied(filename, conditions, attrs, Blockoffset, thisoffset)) {
                    //如果该项满足筛选条件
                    vector<AttrWithValue> thisrecord;
                    thisrecord.clear();
                    int tmpoffset = thisoffset;

                    for (int j = 0; j < attrs.size(); j++) {//遍历所有属性项
                        AttrWithValue *thisattrs = new AttrWithValue;
                        thisattrs->isNull = 0;
                        thisattrs->isPrimary = attrs[j].isPrimary;
                        thisattrs->isUnique = attrs[j].isUnique;
                        thisattrs->length = attrs[j].length;
                        thisattrs->type = attrs[j].type;
                        switch (attrs[j].type) {
                            case MINI_FLOAT: {
                                float f;
                                thisblock.readBlock(f, sizeof(float), thisoffset);
                                thisattrs->value.f = f;//记录下该属性的值
                                thisoffset += sizeof(float);
                                break;
                            }
                            case MINI_INT: {
                                int n;
                                thisblock.readBlock(n, sizeof(int), thisoffset);
                                thisattrs->value.n = n;//记录下该属性的值
                                thisoffset += sizeof(int);
                                break;
                            }
                            case MINI_STRING: {
                                int n;
                                thisblock.readBlock(n, sizeof(int), thisoffset);
                                thisoffset += sizeof(int);
                                char *str = new char[n];
                                thisblock.readBlock(*str, n, thisoffset);
                                string s = "";
                                for (int l = 0; l < n; l++)
                                    s += str[l];
                                thisattrs->value.s = s;//记录下该属性的值
                                thisoffset += attrs[j].length;
                                delete[] str;
                                break;
                            }
                            default:
                                break;
                        }
                        thisrecord.push_back(*thisattrs);
                        delete thisattrs;
                    }
                    Deleted.push_back(thisrecord);
                    DeleteRecord(filename, attrs, Blockoffset, tmpoffset);//删除该属性
                }
            }
        }
    }
    return Deleted;//返回删除的记录集
}

void RecordManager::drawLine(int columnNum, bool firstLine) {
    if (!firstLine)
        cout << "  |" << endl;
    cout << "+";
    cout.fill('-');
    cout << setw(columnNum * 18) << "+";
    cout.fill(' ');
    cout << endl;
}
