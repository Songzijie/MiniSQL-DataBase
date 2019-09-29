/*
 * by Zijie Song
 */

#include <Index.h>

Index::Index(string filename) :index_name(filename), datawidth{4, 4, 20} {
    fstream index_file;
    index_file.open(("db_files/" + filename).c_str(), ios::in);
    if (!index_file) {
        index_file.open(("db_files/" + filename).c_str(), ios::out);
        curn = 0;
    }else{
        index_file.seekp(0, ios::end);
        long size = index_file.tellg();
        curn = size / BLOCK_SIZE;
        index_file.close();
        if (curn == 0)
            return;
        char* get = new char[BLOCK_SIZE];
        BufferNode &t1 = buffer.getBlock(MINI_INDEX, index_name, 0);
        t1.readBlock(*get, BLOCK_SIZE, 0);
        datatype = *(int*)(get+20);
        maxn = (BLOCK_SIZE-NINFO)/(datawidth[datatype]+PINFO)-1;
    }

}

int Index::remove(IndexInfo bo) {
    char *curblock = new char[BLOCK_SIZE];
    int size = curn;
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
            int blocknum = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] +
                                     4);
            int offset = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] +
                                   8);
            if (blocknum == bo.blockNum && offset == bo.offset){
                *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype] + 16) = 1;
                t1.writeBlock(*curblock, BLOCK_SIZE, 0);
                return 1;
            }
            sibling = *(int *) (curblock + NINFO + sibling * (datawidth[datatype] + PINFO) + datawidth[datatype]);
        }
    }
    return 0;
}
