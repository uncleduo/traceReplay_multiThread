#ifndef TRACE_ITEM_H
#define TRACE_ITEM_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/xattr.h>
#include <unistd.h>
#include <fcntl.h> 
#include<dirent.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>

using namespace std;

enum opType{
    //attr op
    op_getattr,
    op_setattr,
    op_getxattr,
    op_setxattr,
    op_removexattr,

    //file op
    op_create,
    op_lookup,
    op_read,
    op_write,
    op_open_r,
    op_open_w,
    op_close,
    op_unlink,
    op_rename,

    //dir op
    op_mkdir,
    op_opendir,
    op_readdir,
    op_closedir,
    op_rmdir,

    //other op
    op_flush,
    op_symlink,
    op_error,
    op_emptyline,
};

typedef struct stTraceItem
{
    opType traceOptype;
    string opName;
    string filePath;
    vector<string> parameterList;

}traceItemST;

typedef struct stDirItem{
    DIR * dirP;
    string dirPath;
}dirItemST;

std::vector< std::string > splitString_STL(const std::string& str, const std::string& pattern);


traceItemST getTrace(string lineContent, string workPath);

void handleTraceItem(traceItemST traceItem);

void initFileSet(traceItemST traceItem);

int genFromFileSet();


#endif