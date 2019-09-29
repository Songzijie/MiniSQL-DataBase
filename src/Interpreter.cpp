/*
 * by Zijie Song
 */

#include <iostream>
#include "../include/Interpreter.h"

using namespace std;

void Interpreter::readCmd(bool isCmd, ifstream &fin) {
    if (isCmd) {
        cout << "MiniSQL << ";
        getline(cin, statement[top++]);
    } else {
        if (getline(fin, statement[top])) {
            top++;
        } else {
            statement[top++] = "quit;";
        }
    }
}

void Interpreter::pipeline(bool isCmd, string fileName) {
    int i, j;
    string line;
    StmtAfterParse result;
    string tempword[100];
    int tempwordtop = 0;
    clock_t start, finish;
    double duration;
    /*将输入中的单词分开并存入word数组*/
    ifstream fin(fileName);
    int fileSize = 0;
    string progressBar = "  |                                         | 100%";
    int lastProgress = 0;
    bool stringFlag = 0;
    if (!isCmd) {
        if (!fin) {
            throw string("Failed when open file.");
        }
        auto begin = fin.tellg();
        fin.seekg(0, ios_base::end);
        fileSize = (int) (fin.tellg() - begin);
        fin.seekg(0, ios_base::beg);
    }
    while (1) {
        try {
            tempwordtop = 0;
            wordtop = 0;
            top = 0;
            Initialize(statement);
            Initialize(tempword);
            Initialize(word);
            do {
                readCmd(isCmd, fin);
                if (statement[top - 1].find(";") != std::string::npos)
                    break;
            } while (statement[top - 1].length() != 0);//循环直到读到一个分号
            for (i = 0; i < top; i++) {
                line = statement[i];
                for (j = 0; j < line.length(); j++) {
                    if (line[j] == ' ' || line[j] == '(' || line[j] == ')' || line[j] == '\t') {
                        if (!stringFlag) {
                            tempwordtop++;
                            while (line[j] == ' ' || line[j] == '\t')
                                j++;
                            if (line[j] == '(' || line[j] == ')')
                                continue;
                        }
                    }
                    if (line[j] == ';' || j >= line.length()) {
                        tempwordtop++;
                        break;
                    }
                    if (line[j] == ',' || line[j] == '=') {
                        tempword[++tempwordtop] = line[j];
                        tempwordtop++;
                        continue;
                    } else if (line[j] == '<') {
                        if (line[j + 1] == '>' || line[j + 1] == '=') {
                            tempword[++tempwordtop] = line[j];
                            tempword[tempwordtop] += line[++j];
                        } else {
                            tempword[++tempwordtop] = line[j];
                        }
                        tempwordtop++;
                        continue;
                    } else if (line[j] == '>') {
                        if (line[j + 1] == '=') {
                            tempword[++tempwordtop] = line[j];
                            tempword[tempwordtop] += line[++j];
                        } else {
                            tempword[++tempwordtop] = line[j];
                        }
                        tempwordtop++;
                        continue;
                    } else if (line[j] == '\'') {
                        stringFlag = !stringFlag;
                    }
                    tempword[tempwordtop] = tempword[tempwordtop] + line[j];
                    /*else {
                        wordtop++;
                    }*/
                }
            }
            copy(tempword, tempwordtop);
            if (word[0] == "quit" && wordtop == 1) {
                lastProgress = 40;
                break;
            }
            if (wordtop == 0)
                continue;

            result = parse();
            if (result.getOperation() == MINI_SELECT) {
                cout << endl;
            }
            start = clock();
            if (result.getOperation() == MINI_EXEC) {
                Interpreter In;
                In.pipeline(false, result.getFileName());
            } else {
                api.exec(result);
            }
            finish = clock();
            duration = (double) (finish - start) / CLOCKS_PER_SEC;
            if (!isCmd) {
                cout << "\r                                                  \r";
            } else {
                cout << "Success. The duration is " << duration * 1000 << " ms" << endl;
            }
        }
        catch (string &error) {
            cout << "\nStatement \"" << statement[top - 1] << "\" Error: " << error << endl;
        }
        if (!isCmd) {
            if ((double) fin.tellg() / fileSize * 40 > lastProgress) {
                lastProgress = (int) ((double) fin.tellg() / fileSize * 40);
                for (int i = 0; i < lastProgress; i++) {
                    progressBar[i + 3] = '=';
                }
                progressBar[lastProgress + 3] = '>';
            }
            cout << progressBar << "  ";
        }
    }
    if (!isCmd) {
        cout << endl << "Finished " << int(lastProgress * 2.5 + 0.5) << "%.\n";
    }
}

StmtAfterParse Interpreter::parse() {
    StmtAfterParse result;
    Attr tempd;
    UnionValue tempv;
    Condition tempc;
    TableNode tempt;
    IndexNode tempi;
    string error;
    int i, type, wordi, priflag, existflag;
    /*CREATE语句*/
    if (word[0] == "create") {
        vector<Attr> definitions;
        priflag = 0, existflag = 0;
        result.setOperation(MINI_CREATE);
        if (word[1] == "table") {
            result.setObject(MINI_TABLE);
            if (!catalog.existTable(word[2]))
                result.setTableName(word[2]);
            else {
                error = "Table already exists";
                throw error;
            }//已存在表，异常
            for (i = 3; i < wordtop;) {
                wordi = 0;
                InitializeAttribute(tempd);
                if (word[i] == "primary" && word[i + 1] == "key") {
                    vector<Attr> tempdefi = result.getDefinitions();
                    for (int j = 0; j < tempdefi.size(); j++) {
                        if (tempdefi[j].attrName == word[i + 2]) {
                            existflag = 1;
                            tempdefi[j].isPrimary = 1;
                            break;
                        }
                    }
                    if (existflag == 0) {
                        error = "This attr doesn't exist(primary key)";
                        throw error;
                    }
                    result.setDefinition(tempdefi);
                    i += 3;
                    continue;
                } else
                    while (word[wordi + i] != ",") {
                        if ((wordi + i) >= wordtop)
                            break;
                        if (wordi == 0)tempd.attrName = word[wordi + i];
                        else if (wordi == 1) {
                            if (word[wordi + i] == "int") {
                                tempd.type = MINI_INT;
                                tempd.length = 4;
                            } else if (word[wordi + i] == "float") {
                                tempd.type = MINI_FLOAT;
                                tempd.length = 4;
                            } else if (word[wordi + i] == "char") {
                                tempd.type = MINI_STRING;
                                if (StringtoInt(word[(++wordi) + i]) != 0)
                                    tempd.length = StringtoInt(word[wordi + i]);
                                else tempd.length = 1;
                            } else {
                                error = "Not char,int or float";
                                throw error;
                            }//不属于char,int,float任何一种属性
                        } else {
                            if (word[wordi + i] == "unique") {
                                tempd.isUnique = 1;
                            } else if (word[wordi + i] == "primary" && word[(++wordi) + i] == "key") {
                                if (priflag == 0)
                                    priflag = 1;
                                else {
                                    error = "Repeated primary key";
                                    throw error;
                                }//第二个primary key，异常
                                tempd.isPrimary = 1;
                            }
                        }
                        wordi++;
                    }
                if (wordi >= 2) {
                    i += wordi + 1;
                    result.addDefinition(tempd);
                } else {
                    error = "Create input wrong";
                    throw error;
                }//必须要有create table table_name这三个单词，否则异常
            }
            definitions = result.getDefinitions();
            definitions[0].offset = 0;
            for (i = 1; i < definitions.size(); i++) {
                definitions[i].offset = definitions[i - 1].offset + definitions[i - 1].length;
                if (definitions[i - 1].type == MINI_STRING)
                    definitions[i].offset += 4;
            }
            result.setDefinition(definitions);
        } else if (word[1] == "index" && word[3] == "on") {
            result.setObject(MINI_INDEX);
            if (catalog.existTable(word[4])) {
                result.setTableName(word[4]);
                if (!catalog.existIndexOnTable(word[2], word[4]))
                    result.setIndexName(word[2]);
                else {
                    error = "Index already exists";
                    throw error;
                }
            } else {
                error = "This table doesn't exist";
                throw error;
            }
            if (wordtop == 6) {
                if (catalog.existAttr(word[4], word[5]))
                    result.setColumn(catalog.AttrtoId(word[4], word[5]));
                else {
                    error = "This column doesn't exist";
                    throw error;
                }
            } else {
                error = "The number of column is less or more than one";
                throw error;
            }
        } else {
            error = "Create input wrong";
            throw error;
        }
    }
        /*INSERT语句*/
    else if (word[0] == "insert") {
        result.setOperation(MINI_INSERT);
        if (word[1] == "into" && catalog.existTable(word[2]) && word[3] == "values") {
            result.setObject(MINI_TABLE);
            result.setTableName(word[2]);
            vector<Attr> v = catalog.getAttrList(word[2]);
            wordi = 4;
            for (i = 0; i < v.size(); i++) {
                if (word[wordi] == ",")
                    wordi++;
                InitializeData(tempv);
                if (wordi >= wordtop) {
                    for (i; i < v.size(); i++) {
                        result.addData(tempv);
                    }
                    break;
                }
                switch (v[i].type) {
                    case MINI_STRING:
                        if (TypeMatch(MINI_STRING, WordType(word[wordi]))) {
                            if (word[wordi].length() > (v[i].length + 2)) {
                                error = "String length overflows";
                                throw error;
                            }//字符串长度超过定义，异常
                            tempv.isNull = false;
                            word[wordi].erase(word[wordi].begin());
                            word[wordi].erase(word[wordi].begin() + word[wordi].length() - 1);
                            tempv.s = word[wordi];
                        } else {
                            error = "Condition value wrong ";
                            throw error;
                        }
                        result.addData(tempv);
                        wordi++;
                        break;
                    case MINI_FLOAT:
                        if (TypeMatch(MINI_FLOAT, WordType(word[wordi]))) {
                            tempv.isNull = false;
                            tempv.f = StringtoFloat(word[wordi]);
                        } else {
                            error = "Condition value wrong ";
                            throw error;
                        }
                        result.addData(tempv);
                        wordi++;
                        break;
                    case MINI_INT:
                        if (TypeMatch(MINI_INT, WordType(word[wordi]))) {
                            tempv.isNull = false;
                            tempv.n = StringtoInt(word[wordi]);
                        } else {
                            error = "Condition value wrong ";
                            throw error;
                        }
                        result.addData(tempv);
                        wordi++;
                        break;
                    default:
                        break;
                }
            }
        } else {
            error = "Insert input wrong or this table doesn't exist";
            throw error;
        }
    }
        /*DELETE语句*/
    else if (word[0] == "delete") {
        result.setOperation(MINI_DELETE);
        if (word[1] == "from" && catalog.existTable(word[2])) {
            result.setTableName(word[2]);
            result.setObject(MINI_TABLE);
        } else {
            error = "Delete input wrong or this record doesn't exist";
            throw error;
        }

        if (word[3] == "where")
            for (i = 4; i < wordtop;) {
                InitializeCondition(tempc);
                if (word[i + 1] == "=") {
                    tempc.op = EQU;
                } else if (word[i + 1] == "<>") {
                    tempc.op = NE;
                } else if (word[i + 1] == "<") {
                    tempc.op = LT;
                } else if (word[i + 1] == ">") {
                    tempc.op = GT;
                } else if (word[i + 1] == "<=") {
                    tempc.op = LE;
                } else if (word[i + 1] == ">=") {
                    tempc.op = GE;
                } else {
                    error = "Operator input wrong";
                    throw error;
                }
                tempc.id = catalog.AttrtoId(word[2], word[i]);
                tempc.type = catalog.IdtoAttr(word[2], tempc.id).type;
                if (TypeMatch(tempc.type, WordType(word[i + 2])) == 1) {
                    tempc.value = StringtoValue(tempc.type, word[i + 2]);
                    result.addCondition(tempc);
                } else {
                    error = "Condition value wrong ";
                    throw error;
                }
                if (i + 3 >= wordtop || word[i + 3] == "and")
                    i += 4;
                else {
                    error = "Delete input(and) wrong";
                    throw error;
                }
            }

    }
        /*SELECT语句*/
    else if (word[0] == "select") {
        int connum = 1;//condition number
        result.setOperation(MINI_SELECT);
        while (word[connum] != "from" && connum < wordtop)
            connum++;
        if (connum + 1 >= wordtop)//找不到from或者表名，即select格式异常
        {
            error = "Select input wrong";
            throw error;
        }

        if (catalog.existTable(word[connum + 1])) {
            result.setObject(MINI_TABLE);
            result.setTableName(word[connum + 1]);
            if (connum == 2 && word[1] == "*") {
                vector<Attr> v = catalog.getAttrList(word[connum + 1]);
                result.setDefinition(v);
            } else
                for (int i = 1; i < connum; i++) {
                    if (word[i] == ",")
                        continue;
                    if (catalog.existAttr(word[connum + 1], word[i])) {
                        tempd = catalog.getAttrInfo(word[connum + 1], word[i]);
                        result.addDefinition(tempd);
                    } else {
                        error = "This column doesn't exist";
                        throw error;
                    }
                }
        } else {
            error = "This table doesn't exist";
            throw error;
        }
        if (word[connum + 2] == "where")
            for (i = connum + 3; i < wordtop;) {
                InitializeCondition(tempc);
                if (word[i + 1] == "=") {
                    tempc.op = EQU;
                } else if (word[i + 1] == "<>") {
                    tempc.op = NE;
                } else if (word[i + 1] == "<") {
                    tempc.op = LT;
                } else if (word[i + 1] == ">") {
                    tempc.op = GT;
                } else if (word[i + 1] == "<=") {
                    tempc.op = LE;
                } else if (word[i + 1] == ">=") {
                    tempc.op = GE;
                } else {
                    error = "Operator input wrong";
                    throw error;
                }
                tempc.id = catalog.AttrtoId(word[connum + 1], word[i]);
                tempc.type = catalog.IdtoAttr(word[connum + 1], tempc.id).type;
                if (TypeMatch(tempc.type, WordType(word[i + 2])) == 1) {
                    tempc.value = StringtoValue(tempc.type, word[i + 2]);
                    result.addCondition(tempc);
                } else {
                    error = "condition value error";
                    throw error;
                }
                if (i + 3 >= wordtop || word[i + 3] == "and")
                    i += 4;
                else {
                    error = "Select input(and) wrong";
                    throw error;
                }
            }
    }
        /*DROP语句*/
    else if (word[0] == "drop") {
        result.setOperation(MINI_DROP);
        if (word[1] == "table") {
            if (catalog.existTable(word[2])) {
                result.setObject(MINI_TABLE);
                result.setTableName(word[2]);
            } else {
//                error = "This table doesn't exist";
//                throw error;
            }
        } else if (word[1] == "index" && word[3] == "on") {
            if (catalog.existIndexOnTable(word[2], word[4])) {
                if (wordtop < 5)//drop语句不完整，异常
                {
                    error = "Drop input wrong";
                    throw error;
                }
                result.setObject(MINI_INDEX);
                result.setIndexName(word[2]);
                result.setTableName(word[4]);
            } else {
                error = "This index doesn't exist";
                throw error;
            }
        } else {
            error = "Drop input wrong";
            throw error;
        }
    } else if (word[0] == "exec") {
        result.setOperation(MINI_EXEC);
        result.setFileName(word[1]);
    } else {
        error = "input error";
        throw error;
    }
    return result;
}

void Interpreter::copy(string *array, int top) {
    for (int i = 0; i < top; i++)
        if (array[i] != "")
            word[wordtop++] = array[i];
}

void Interpreter::Initialize(string *array) {
    for (int i = 0; i < 100; i++) {
        array[i] = "";
    }
}

void Interpreter::InitializeData(UnionValue &v) {
    v.f = 0;
    v.n = 0;
    v.s = "";
    v.isNull = false;
}

void Interpreter::InitializeCondition(Condition &c) {
    c.id = -1;
    c.op = EQU;
    c.type = MINI_FLOAT;
    InitializeData(c.value);
}

void Interpreter::InitializeAttribute(Attr &a) {
    a.attrName = "";
    a.hasIndex = 0;
    a.isPrimary = 0;
    a.isUnique = 0;
    a.length = 0;
    a.offset = 0;
    a.type = MINI_FLOAT;
}

UnionValue Interpreter::StringtoValue(DataType type, string &x) {
    UnionValue v;
    v.isNull = false;
    switch (type) {
        case MINI_STRING:
            for (int i = 1; i < x.length() - 1; i++)
                v.s += x[i];
            break;
        case MINI_INT:
            v.n = StringtoInt(x);
            break;
        case MINI_FLOAT:
            v.f = StringtoFloat(x);
            break;
        default:
            break;
    }
    return v;
}

DataType Interpreter::WordType(string &a) {
    DataType type = MINI_INT;
    int i, flag = 0;
    if (a[0] == '\'' && a[a.length() - 1] == '\'')
        return MINI_STRING;
    for (i = 0; i < a.length(); i++) {
        if (a[i] >= '9' || a[i] <= '0')
            if (a[i] == '.' && flag == 0) {
                type = MINI_FLOAT;
                flag = 1;
            } else //不属于string,float,int三种格式的任何一个
                ;
    }
    return type;
}

int Interpreter::StringtoInt(string &word) {
    int i, sum = 0;
    for (i = 0; i < word.length(); i++) {
        if (word[i] >= '0' && word[i] <= '9')
            sum = sum * 10 + (word[i] - 48);
        else return 0;
    }
    return sum;
}

float Interpreter::StringtoFloat(string &word) {
    int i = 0, wordi = 1;
    float sum = 0, sumi = 0;
    while (word[i] != '.' && i < word.length()) {
        if (word[i] >= '0' && word[i] <= '9')
            sum = sum * 10 + (word[i] - '0');
        else return 0;
        i++;
    }
    i++;
    while (i < word.length()) {
        if (word[i] >= '0' && word[i] <= '9') {
            sumi += (word[i] - '0') * pow(0.1, wordi);
            wordi++;
        } else return 0;
        i++;
    }
    sum += sumi;
    return sum;
}

bool Interpreter::TypeMatch(DataType type1, DataType type2) {
    if (type1 == type2)
        return 1;
    else if (type1 == MINI_FLOAT && type2 == MINI_INT)
        return 1;
    else
        return 0;
}
