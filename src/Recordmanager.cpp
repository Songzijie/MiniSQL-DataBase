/*
 * by Zijie Song
 */

#include "RecordManager.h"

RecordManager::RecordManager()//���캯��
{
}

RecordManager::~RecordManager()//��������
{
}

int RecordManager::getRecordlength(vector<Attr> &attrs) {
    //��¼�ܳ�Ϊ���һ�����Ե�λƫ�������ϸ����Եĳ���(������һλ���ַ���Ҫ����4λ���ַ����ȼ�¼λ)
    if (attrs[attrs.size() - 1].type == MINI_STRING)
        return attrs[attrs.size() - 1].offset + attrs[attrs.size() - 1].length + sizeof(int);
    else
        return attrs[attrs.size() - 1].offset + attrs[attrs.size() - 1].length;
}

bool RecordManager::Isempty(string fileName, vector<Attr> &attrs, int num, int offset) {
    BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, fileName, num);
    for (int i = 0; i < attrs.size(); i++) {//�����ü�¼������ֵ
        switch (attrs[i].type) {
            case MINI_FLOAT: {
                float f;
                thisblock.readBlock(f, sizeof(float), offset + attrs[i].offset);
                if (f != 0.0) return false;//�����ֵ��Ϊ0����ü�¼��Ϊ��
                break;
            }
            case MINI_INT: {
                int n;
                thisblock.readBlock(n, sizeof(int), offset + attrs[i].offset);
                if (n != 0) return false;//�����ֵ��Ϊ0����ü�¼��Ϊ��
                break;
            }
            case MINI_STRING: {
                int n;
                thisblock.readBlock(n, sizeof(int), offset + attrs[i].offset);
                if (n != 0) return false;//������ַ����ȼ�¼λ��Ϊ0����ü�¼��Ϊ��
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
    if (Isempty(fileName, attrs, num, offset)) return false;//����ü�¼Ϊ����ض�������ɸѡ����

    BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, fileName, num);
    for (int i = 0; i < conditions.size(); i++) {//��������ɸѡ����
        int column = conditions[i].id;
        int tmpoffset = offset + attrs[column].offset;//���е�λƫ����

        if (conditions[i].op == NO) continue;//��������������ڣ�����ת����һ������

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
                thisblock.readBlock(n, sizeof(int), tmpoffset);//�ȶ������ַ�ʵ�ʳ���
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
    return true;//������Ҫ��
}

void RecordManager::Output(string fileName, vector<Attr> &attrs, vector<int> &selectColumns, int num, int offset) {
    BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, fileName, num);
    for (int i = 0; i < selectColumns.size(); i++) {//��������ѡ�е���
        if (i > 0)
            cout << "  ";
        cout << "|";
        int thisoffset = offset + attrs[selectColumns[i]].offset;//�����ڶ�Ӧ�Ŀ��е���ƫ����
        switch (attrs[selectColumns[i]].type) {
            case MINI_FLOAT: {
                float f;
                thisblock.readBlock(f, sizeof(float), thisoffset);
                cout.width(15);//������������Ϊ15λ���Ա����
                cout << setprecision(8) << f;//����Ϊ8λ
                break;
            }
            case MINI_INT: {
                int n;
                thisblock.readBlock(n, sizeof(int), thisoffset);
                cout.width(15);//����������Ϊ15λ
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
                cout.width(strWidth);//�ַ�������Ϊ��λ���ȼ�5
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
    for (int i = 0; i < attrs.size(); i++) {//�����������ԣ���ÿһ�����ÿ�
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
                int n = 0;//�����ַ����Ե��У�ֱ�ӽ��ַ����ȼ�¼λ��Ϊ0������ɾ��
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

    bool IsFindposition = false;//��¼�Ƿ����ҵ�����λ��
    IndexInfo position;
    IndexInfo insertfail;//������Ч������info����ӦΪ���Ϊ0��ƫ����Ϊ0
    insertfail.blockNum = 0;
    insertfail.offset = 0;

    vector<int> column;
    column.clear();
    for (int k = 0; k < attrs.size(); k++)//�ҵ�������Ҫ�ж�Unique����
        if ((attrs[k].isPrimary) || (attrs[k].isUnique)) column.push_back(k);

    for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//�������еĿ�
        BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, fileName, Blockoffset);
        int recordnumber;
        int offset = 0;
        thisblock.readBlock(recordnumber, sizeof(int), offset);
        offset += sizeof(int);
        int tmpoffset = offset;

        for (int recnum = 0; recnum < recordnumber; recnum++) {//�����ÿ�����м�¼
            if ((Isempty(fileName, attrs, Blockoffset, tmpoffset)) && (IsFindposition = false)) {
                //����ü�¼Ϊ���һ�Ϊ�ҵ�����λ�ã����¼�¸�λΪ����λ��
                IsFindposition = true;
                position.blockNum = Blockoffset;
                position.offset = tmpoffset;
            } else {//����ü�¼��Ϊ�գ�����������¼��Unique�����Ƿ�����ظ�
                for (int k = 0; k < column.size(); k++) {
                    switch (attrs[column[k]].type) {
                        case MINI_FLOAT: {
                            float f;
                            thisblock.readBlock(f, sizeof(float), tmpoffset + attrs[column[k]].offset);
                            if (f == AttrValues[column[k]].value.f) return insertfail;//�����ظ��򷵻ز�����Ч������info
                            break;
                        }
                        case MINI_INT: {
                            int n;
                            thisblock.readBlock(n, sizeof(int), tmpoffset + attrs[column[k]].offset);
                            if (n == AttrValues[column[k]].value.n) return insertfail;//�����ظ��򷵻ز�����Ч������info
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
                                if (s == AttrValues[column[k]].value.s) return insertfail;//�����ظ��򷵻ز�����Ч������info
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
            //���������ÿ����м�¼��δ�ҵ�����λ�ã����ж��ÿ�ĩβ�Ƿ�����Ӽ�¼
            IsFindposition = true;
            position.blockNum = Blockoffset;
            position.offset = recordnumber * Recordlength + 4;
            recordnumber++;//���¸ÿ��¼����
            thisblock.writeBlock(recordnumber, sizeof(int), 0);
        }
    }
    if (IsFindposition == false) {//������������п���δ�ҵ�����λ�ã����¿�һ�������
        IsFindposition = true;
        Blocknum++;//�����ܿ���
        firstblock.writeBlock(Blocknum, sizeof(int), 0);
        BufferNode &newblock = bufferManager.getBlock(MINI_TABLE, fileName, Blocknum);
        int newrecordnumber = 1;//�����¿��¼��Ϊ1
        newblock.writeBlock(newrecordnumber, sizeof(int), 0);
        position.blockNum = Blocknum;
        position.offset = 4;
    }
    return position;//�������ղ���λ�õ�����info
}

vector<AttrWithValue> RecordManager::selectAttr(string tableName, vector<Attr> &attrs, int column) {
    string filename = tableName + ".table";
    BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);
    int Blocknum = 0;
    firstblock.readBlock(Blocknum, sizeof(int), 0);
    int Recordlength = 0;
    firstblock.readBlock(Recordlength, sizeof(int), 4);
    vector<AttrWithValue> finalattrwithvalue;
    finalattrwithvalue.clear();//��ʼ�����Ϊ��
    if (Blocknum == 0 || column > attrs.size()) //����ļ���û�п�����кų�����Χ��ֱ�ӷ��ؿ�
        return finalattrwithvalue;

    int columnoffset = attrs[column].offset;//���е�λƫ����
    for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//�������п�
        BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, Blockoffset);
        int offset = 0;
        int recordnumber = 0;
        thisblock.readBlock(recordnumber, sizeof(int), offset);
        offset += sizeof(int);

        for (int recnum = 0; recnum < recordnumber; recnum++) {//�����������м�¼
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
                    tmprecord->value.f = f;//��¼�¸ü�¼��Ӧ���Ե�ֵ
                    break;
                }
                case MINI_INT: {
                    int n;
                    thisblock.readBlock(n, sizeof(int), tmpoffset);
                    tmprecord->value.n = n;//��¼�¸ü�¼��Ӧ���Ե�ֵ
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
                    tmprecord->value.s = s;//��¼�¸ü�¼��Ӧ���Ե�ֵ
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

    if (indexes.size()) {//�����������
        for (int i = 0; i < indexes.size(); i++)
            if (Issatisfied(filename, conditions, attrs, indexes[i].blockNum, indexes[i].offset)) {
                //����ü�¼�����������������1ͬʱ����ü�¼��ѡ����
                count++;
                Output(filename, attrs, selectColumns, indexes[i].blockNum, indexes[i].offset);
            }
    } else {
        BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);//ȡ�����ļ��������
        int Blocknum = 0;
        int Recordlength = 0;
        firstblock.readBlock(Blocknum, sizeof(int), 0);//�ӵ�������ж������ļ����ܿ���
        firstblock.readBlock(Recordlength, sizeof(int), 4);//�ӵ�������ж������ļ�������¼�ĳ���
        for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//�������еĿ�
            BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, Blockoffset);
            int offset = 0;
            int recordnumber = 0;
            thisblock.readBlock(recordnumber, sizeof(int), offset);
            offset += sizeof(int);

            for (int recnum = 0; recnum < recordnumber; recnum++) {//�������еļ�¼
                int thisoffset = offset + recnum * Recordlength;
                if (Issatisfied(filename, conditions, attrs, Blockoffset, thisoffset)) {
                    //����ü�¼�����������������1ͬʱ����ü�¼��ѡ����
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
    BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);//ȡ�����ļ��������
    int Blocknum = 0;
    int Recordlength = 0;
    firstblock.readBlock(Blocknum, sizeof(int), 0);//�ӵ�������ж������ļ����ܿ���
    firstblock.readBlock(Recordlength, sizeof(int), 4);//�ӵ�������ж������ļ�������¼�ĳ���
    vector<IndexInfo> AllIndex;
    AllIndex.clear();

    for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//�������п�
        BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, Blockoffset);
        int recordnumber;
        int offset = 0;
        thisblock.readBlock(recordnumber, sizeof(int), offset);
        offset += sizeof(int);

        for (int recnum = 0; recnum < recordnumber; recnum++) {//�������м�¼
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
    BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);//ȡ�����ļ��������
    int Blocknum = 0;
    int Recordlength = 0;
    firstblock.readBlock(Blocknum, sizeof(int), 0);//�ӵ�������ж������ļ����ܿ���
    firstblock.readBlock(Recordlength, sizeof(int), 4);//�ӵ�������ж������ļ�������¼�ĳ���
    if (Blocknum == 0 && Recordlength == 0) {//������ü�¼����Ϊ��˵��δ��ʼ�������ʼ�����ļ��ĵ�����¼����
        Recordlength = getRecordlength(attrs);
        firstblock.writeBlock(Blocknum, sizeof(int), 0);
        firstblock.writeBlock(Recordlength, sizeof(int), 4);
    }

    IndexInfo position = getPosition(filename, attrs, AttrValues);//��õ�ǰ��¼���Բ����λ��
    if (position.blockNum) {//�����λ��Ϊ��Чλ�����ڸ�λ��ʵ�ֲ���
        BufferNode &insertblock = bufferManager.getBlock(MINI_TABLE, filename, position.blockNum);
        int insertoffset = position.offset;
        for (int i = 0; i < AttrValues.size(); i++) {
            switch (AttrValues[i].type) {
                case MINI_FLOAT: {
                    float f;
                    if (AttrValues[i].isNull)//�������ļ�¼������Ϊ�գ������0
                        f = 0.0;
                    else
                        f = AttrValues[i].value.f;
                    insertblock.writeBlock(f, sizeof(float), insertoffset);
                    insertoffset += sizeof(float);
                    break;
                }
                case MINI_INT: {
                    int n;
                    if (AttrValues[i].isNull)//�������ļ�¼������Ϊ�գ������0
                        n = 0;
                    else
                        n = AttrValues[i].value.n;
                    insertblock.writeBlock(n, sizeof(int), insertoffset);
                    insertoffset += sizeof(int);
                    break;
                }
                case MINI_STRING: {
                    string s;
                    if (AttrValues[i].isNull)//�������ļ�¼������Ϊ�գ��������ַ�
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
    return position;//���ظ�λ�õ�����info
}

vector<vector<AttrWithValue> >
RecordManager::deleteTuple(string tableName, vector<Condition> &conditions, vector<Attr> &attrs,
                           vector<IndexInfo> &indexes) {
    string filename = tableName + ".table";
    vector<vector<AttrWithValue> > Deleted;
    Deleted.clear();
    if (indexes.size()) {//�����������
        for (int i = 0; i < indexes.size(); i++)//�������и�����������
            if (Issatisfied(filename, conditions, attrs, indexes[i].blockNum, indexes[i].offset)) {
                //�����������ɸѡ����
                BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, indexes[i].blockNum);
                int thisoffset = indexes[i].offset;
                vector<AttrWithValue> thisrecord;
                thisrecord.clear();

                for (int j = 0; j < attrs.size(); j++) {//��������������
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
                            thisattrs->value.f = f;//��¼�¸����Ե�ֵ
                            thisoffset += sizeof(float);
                            break;
                        }
                        case MINI_INT: {
                            int n;
                            thisblock.readBlock(n, sizeof(int), thisoffset);
                            thisattrs->value.n = n;//��¼�¸����Ե�ֵ
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
                            thisattrs->value.s = s;//��¼�¸����Ե�ֵ
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
                DeleteRecord(filename, attrs, indexes[i].blockNum, indexes[i].offset);//ɾ��������
            }
    } else {
        BufferNode &firstblock = bufferManager.getBlock(MINI_TABLE, filename, 0);//ȡ�����ļ��������
        int Blocknum = 0;
        int Recordlength = 0;
        firstblock.readBlock(Blocknum, sizeof(int), 0);//�ӵ�������ж������ļ����ܿ���
        firstblock.readBlock(Recordlength, sizeof(int), 4);//�ӵ�������ж������ļ�������¼�ĳ���
        for (int Blockoffset = 1; Blockoffset <= Blocknum; Blockoffset++) {//�������еĿ�
            BufferNode &thisblock = bufferManager.getBlock(MINI_TABLE, filename, Blockoffset);
            int offset = 0;
            int recordnumber = 0;
            thisblock.readBlock(recordnumber, sizeof(int), offset);
            offset += sizeof(int);

            for (int recnum = 0; recnum < recordnumber; recnum++) {//�������еļ�¼
                int thisoffset = offset + recnum * Recordlength;
                if (Issatisfied(filename, conditions, attrs, Blockoffset, thisoffset)) {
                    //�����������ɸѡ����
                    vector<AttrWithValue> thisrecord;
                    thisrecord.clear();
                    int tmpoffset = thisoffset;

                    for (int j = 0; j < attrs.size(); j++) {//��������������
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
                                thisattrs->value.f = f;//��¼�¸����Ե�ֵ
                                thisoffset += sizeof(float);
                                break;
                            }
                            case MINI_INT: {
                                int n;
                                thisblock.readBlock(n, sizeof(int), thisoffset);
                                thisattrs->value.n = n;//��¼�¸����Ե�ֵ
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
                                thisattrs->value.s = s;//��¼�¸����Ե�ֵ
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
                    DeleteRecord(filename, attrs, Blockoffset, tmpoffset);//ɾ��������
                }
            }
        }
    }
    return Deleted;//����ɾ���ļ�¼��
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
