/*
 * by Zijie Song
 */

#include <BufferManager.h>
#include <io.h>

/*
pin为true的block在buffer满时不会被替换
getBlock中的offset指第几个block
其余offset(block内的)全部指字节
length也是字节
type:table=0,index=1
*/

//Buffer Node

void BufferNode::setBufferNode(ObjectType type, string filename, int blockOffset) {
    flush();
    this->type = type;
    this->fileName = filename;
    this->offset = blockOffset;
    this->dirty = false;
    this->pin = false;
    data = new char[BLOCK_SIZE];
    memset(data, 0, BLOCK_SIZE);
    FILE *fp;
    string dir = "./db_files";
    if (access(dir.c_str(), 0) == -1) {
        cout << dir << " is not existing." << endl;
        cout << " - Now make it." << endl;
        int flag=-1;
        flag = mkdir(dir.c_str());
#ifdef linux
        flag=mkdir(dir.c_str(), 0777);
#endif
        if (flag == 0) {
            cout << " - Make successfully." << endl;
        } else if (flag == 1) {
            cout << " - Make errorly" << endl;
        } else{
            cout << " - Error: Can't identify OS" << endl;
        }
    }

    fp = fopen(("db_files/" + fileName).c_str(), "r+b");
    if (!fp) {
        //create a new file
        fp = fopen(("db_files/" + fileName).c_str(), "a+b");
    } else {
        //exist the file
        fseek(fp, blockOffset * BLOCK_SIZE, SEEK_SET);
        fread(data, sizeof(char), BLOCK_SIZE, fp);
    }
    fclose(fp);
}

void BufferNode::flush() {
    if (dirty) {
        FILE *fp;
        fp = fopen(("db_files/" + fileName).c_str(), "r+b");
        if (fp) {
            fseek(fp, BLOCK_SIZE * offset, SEEK_SET);
            fwrite(data, sizeof(char), BLOCK_SIZE, fp);
            fclose(fp);
        }
    }
}

bool BufferNode::isMatch(int type, string filename, int blockOffset) {
    return this->type == type && this->fileName == filename && this->offset == blockOffset;
}

bool BufferNode::isEmpty() {
    return data == NULL;
}


BufferNode::~BufferNode() {
    flush();
    delete[] data;
};


// Buffer Manager

BufferManager &BufferManager::GetInstance() {
//    cout << "#debug# Get Instance" << endl;
    static BufferManager instance;
    return instance;
}

BufferManager::BufferManager() : accNum(0) {
    bufferPool = new BufferNode[MAX_BLOCK];
//    cout << "#debug# Construct BufferManager" << endl;
}

BufferManager::~BufferManager() {
    delete[] bufferPool;
//    cout << "#debug# Deconstruct BufferManager" << endl;
}

BufferNode &BufferManager::getBlock(ObjectType type, string filename, int blockOffset) {
    accNum++;
    int minAge = 0x7fffffff, indexOfMinAge = 0;
    for (int i = 0; i < MAX_BLOCK; i++) {
        if (bufferPool[i].isMatch(type, filename, blockOffset)) {
            bufferPool[i].age = accNum;
//            cout << "#debug# Exist " + filename + " " << blockOffset << endl;
            return bufferPool[i];
        } else if (bufferPool[i].age < minAge && !bufferPool[i].pin) {
            minAge = bufferPool[i].age;
            indexOfMinAge = i;
        }
    }
    bufferPool[indexOfMinAge].age = accNum;
//    cout << "#debug# New block " << filename << " " << blockOffset << endl;
    bufferPool[indexOfMinAge].setBufferNode(type, filename, blockOffset);
    return bufferPool[indexOfMinAge];
}

