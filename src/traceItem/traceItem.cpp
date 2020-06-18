#include "traceItem.h"
#include "../utils/traceUtils.h"
#include <errno.h>

int wrongTraceCount = 0;

std::set<std::string> fileSet, dirSet, dirCreateSet, fileCreateSet, dirGenSet, fileGenSet;
map<string, DIR *> openedDirDict;
map<string, int> openedFileDict;
//set<opType> uselessOP{op_error, op_emptyline, op_flush, op_getattr, op_read, op_write, op_symlink, op_close, op_closedir};
set<opType> uselessOP{op_error, op_emptyline, op_flush, op_getattr, op_read, op_write, op_symlink};

bool check_uselessOP(opType target)
{
    if (uselessOP.find(target) == uselessOP.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

traceItemST getTrace(string lineContent, string workPath)
{
    std::vector<std::string> subString = splitString_STL(lineContent, " ");
    traceItemST traceItem;
    int pos = 9;
    if (lineContent.empty())
    {
        //cout << "ERR: Empty Line." <<endl;
        traceItem.traceOptype = op_emptyline;
        return traceItem;
    }

    if (subString.size() <= 10)
    {
        cout << "ERR: " << lineContent << endl;
        traceItem.traceOptype = op_error;
        return traceItem;
    }
    traceItem.opName = subString[pos];

    // set trace type
    if (traceItem.opName == "getattr")
    {
        traceItem.traceOptype = op_getattr;
    }
    else if (traceItem.opName == "setattr")
    {
        traceItem.traceOptype = op_setattr;
    }
    else if (traceItem.opName == "getxattr")
    {
        traceItem.traceOptype = op_getxattr;
    }
    else if (traceItem.opName == "setxattr")
    {
        traceItem.traceOptype = op_setxattr;
    }
    else if (traceItem.opName == "removexattr")
    {
        traceItem.traceOptype = op_removexattr;
    }
    else if (traceItem.opName == "create")
    {
        traceItem.traceOptype = op_create;
    }
    else if (traceItem.opName == "lookup")
    {
        traceItem.traceOptype = op_lookup;
    }
    else if (traceItem.opName == "read")
    {
        traceItem.traceOptype = op_read;
    }
    else if (traceItem.opName == "write")
    {
        traceItem.traceOptype = op_write;
    }
    else if (traceItem.opName == "open.r")
    {
        traceItem.traceOptype = op_open_r;
    }
    else if (traceItem.opName == "open.w")
    {
        traceItem.traceOptype = op_open_w;
    }
    else if (traceItem.opName == "close")
    {
        traceItem.traceOptype = op_close;
    }
    else if (traceItem.opName == "unlink")
    {
        traceItem.traceOptype = op_unlink;
    }
    else if (traceItem.opName == "rename")
    {
        traceItem.traceOptype = op_rename;
    }
    else if (traceItem.opName == "mkdir")
    {
        traceItem.traceOptype = op_mkdir;
    }
    else if (traceItem.opName == "opendir")
    {
        traceItem.traceOptype = op_opendir;
    }
    else if (traceItem.opName == "readdir")
    {
        traceItem.traceOptype = op_readdir;
    }
    else if (traceItem.opName == "closedir")
    {
        traceItem.traceOptype = op_closedir;
    }
    else if (traceItem.opName == "rmdir")
    {
        traceItem.traceOptype = op_rmdir;
    }
    else if (traceItem.opName == "flush")
    {
        traceItem.traceOptype = op_flush;
    }
    else if (traceItem.opName == "symlink")
    {
        traceItem.traceOptype = op_symlink;
    }
    else
    {
        traceItem.traceOptype = op_error;
        wrongTraceCount++;
        cout << "GET ERR:" << lineContent << endl;
    }

    //simply transform the path

    if (subString[10] == "")
    {
        cout << "No path" << endl;
        traceItem.filePath = "";
    }
    else if (subString[10].find("/") == string::npos and subString[10] != "#0x1")
    {
        if (subString[10].compare(0, 3, "#0x1"))
        {
            traceItem.filePath = workPath;
        }
        else
        {
            cout << "[ERR]: Wrong trace" << lineContent << endl;
            traceItem.traceOptype = op_error;
            wrongTraceCount++;
            return traceItem;
        }
    }
    /*else if (subString[10]=="#0x1")
    {
        traceItem.filePath = workPath;
        //cout << traceItem.filePath <<endl;
    }*/
    else
    {
        traceItem.filePath = subString[10].replace(subString[10].find("#0x1"), 4, workPath);
        //cout << traceItem.filePath <<endl;
    }
    for (pos = 11; pos < subString.size(); pos++)
    {
        traceItem.parameterList.push_back(subString[pos]);
    }

    return traceItem;
}

void handleTraceItem(traceItemST traceItem)
{
    DIR *openedDir = NULL;
    int openFileMode = O_WRONLY;
    int openedFile;
    char xattrValueBuf[1024] = {0};
    struct stat statTempBuf;
    string rename_oldname, rename_newname;
    switch (traceItem.traceOptype)
    {
    case op_emptyline:
        break;
    case op_error:
        cout << "ERR" << endl;
        break;
    case op_flush:
    case op_read:
    case op_write:
        break;
    case op_getattr:
        break;
    case op_setattr:
        if (chmod(traceItem.filePath.c_str(), 0777) != 0)
        {
            cout << "[ERR]: chmod file " << traceItem.filePath << " failed" << endl;
        }
        break;
    case op_getxattr:
        if (getxattr(traceItem.filePath.c_str(), traceItem.parameterList[0].c_str(), xattrValueBuf, 1024) == -1)
        {
            //cout<<"[ERR]: get xattr " << traceItem.parameterList[0] << " of " <<traceItem.filePath<<" failed"<<endl;
        }
        break;
    case op_setxattr:
        cout << "[NOTICE]: there is a setxattr on " << traceItem.filePath.c_str() << endl;
        if (setxattr(traceItem.filePath.c_str(), traceItem.parameterList[0].c_str(), traceItem.parameterList[1].c_str(), 1024, XATTR_CREATE) != 0)
        {
            cout << "[ERR]: set xattr " << traceItem.parameterList[0] << " of " << traceItem.filePath << " failed" << endl;
        }
        break;
    case op_removexattr:
        if (removexattr(traceItem.filePath.c_str(), traceItem.parameterList[0].c_str()) == -1)
        {
            //cout<<"[ERR]: remove xattr " << traceItem.parameterList[0] << " of " <<traceItem.filePath<<" failed"<<endl;
        }
        break;
    case op_create:
        openedFile = creat(traceItem.filePath.c_str(), 0666);
        if (openedFile)
        {
            openedFileDict[traceItem.filePath] = openedFile;
        }
        else
        {
            cout << "[ERR]: create file " << traceItem.filePath << " failed" << endl;
        }
        break;
    case op_lookup:
        stat(traceItem.filePath.c_str(), &statTempBuf);
        break;

    case op_open_r:
        openFileMode = O_RDONLY;
    case op_open_w:
        if (openedFileDict[traceItem.filePath]){
            close(openedFileDict[traceItem.filePath]);
        }
        openedFile = open(traceItem.filePath.c_str(), openFileMode);
        if (openedFileDict[traceItem.filePath])
        {
            cout << "[SUCESS]: open old file " << traceItem.filePath << " MODE: " << openFileMode << endl;
        }
        else
        {
            if (openedFile == -1)
            {
                cout << "[ERR]: open file " << traceItem.filePath << " failed, ERRNO: " << errno << " MODE: " << openFileMode << endl;

                //cout<< "[INFO] opened file count:" << openedFileDict.size() << endl;
                return;
            }
            openedFileDict[traceItem.filePath] = openedFile;
        }
        break;
    case op_close:
        //Many errors were reported while tar. Try to remove it
        if (openedFileDict[traceItem.filePath])
        {
            if (close(openedFileDict[traceItem.filePath]) != 0)
            {
                cout << "[ERR]: close file " << traceItem.filePath << " failed: close failed" << endl;
            }
            openedFileDict[traceItem.filePath] = 0;
        }
        else
        {
            cout << "[ERR]: close file " << traceItem.filePath << " failed: no fd" << endl;
        }
        break;
    case op_unlink:
        if (unlink(traceItem.filePath.c_str()) != 0)
        {
            cout << "[ERR]: unlink " << traceItem.filePath << " failed" << endl;
        }

        break;
    case op_rename:
        rename_oldname = traceItem.filePath;
        rename_newname = getParentDirPath(traceItem.filePath) + "/" + traceItem.parameterList[0];
        if (rename(rename_oldname.c_str(), rename_newname.c_str()) != 0)
        {
            cout << "[ERR]: rename file " << traceItem.filePath << " to " << rename_oldname << endl;
        }
        break;
    case op_mkdir:
        if (mkdir(traceItem.filePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
        {
            cout << "[ERR]: mkdir " << traceItem.filePath << " failed" << endl;
        }
        else
        {
            //cout<<"[SUCESS]: mkdir dir "<<traceItem.filePath<<endl;
        }

        break;
    case op_opendir:
        if (openedDirDict[traceItem.filePath])
        {
            closedir(openedDirDict[traceItem.filePath]);
        }

        openedDir = opendir(traceItem.filePath.c_str());
        if (openedDirDict[traceItem.filePath])
        {
            //cout<<"[SUCESS]: open old dir "<<traceItem.filePath<<endl;
        }
        else
        {
            if (openedDir == NULL)
            {
                cout << "ERR: opendir " << traceItem.filePath << " failed" << endl;
                //cout<< "[INFO] opened dir count:" << openedDirDict.size() << endl;
                return;
            }

            //cout<<"[SUCESS]: open new dir "<<traceItem.filePath<<endl;
            openedDirDict[traceItem.filePath] = openedDir;
        }
        break;
    case op_readdir:
        if (openedDirDict[traceItem.filePath] != NULL)
        {
            if (readdir(openedDirDict[traceItem.filePath]) == NULL)
            {
                cout << "ERR: readdir " << traceItem.filePath << " failed: read failed" << endl;
            }
        }
        else
        {
            cout << "ERR: readdir " << traceItem.filePath << " failed: dir is closed" << endl;
        }

        break;
    case op_closedir:
        //Many errors were reported while tar. Try to remove it
        if (closedir(openedDirDict[traceItem.filePath]) == 0)
        {
            if (openedDirDict[traceItem.filePath] != NULL)
            {
                //cout<<"[DO]: set "<<traceItem.filePath<<" to NULL"<<endl;
                openedDirDict[traceItem.filePath] = NULL;
            }
        }
        else
        {
            cout << openedDirDict.size() << endl;
            cout << "[ERR]: closedir " << traceItem.filePath << " failed" << endl;
        }
        break;
    case op_rmdir:
        if (rmdir(traceItem.filePath.c_str()) != 0)
        {
            cout << "[ERR]: rmdir " << traceItem.filePath << "failed" << endl;
        }
        else
        {
            //cout<<"[SUCESS]: mkdir dir "<<traceItem.filePath<<endl;
        }
        break;
    case op_symlink:
        cout << "[ERR]: symlink not finished" << endl;
        break;
    default:
        cout << traceItem.opName << endl;
        break;
    }
}

void initFileSet(traceItemST traceItem)
{
    const bool is_in = false;
    switch (traceItem.traceOptype)
    {
    case op_getattr:
    case op_setattr:
    case op_getxattr:
    case op_setxattr:
    case op_removexattr:

        break;

    //file op
    case op_create:
        fileCreateSet.insert(traceItem.filePath);
        //cout << "Create FILE" << traceItem.filePath <<endl;
        break;
    case op_lookup:
    case op_read:
    case op_write:
    case op_open_r:
    case op_open_w:
    case op_close:
    case op_unlink:
    case op_rename:
        fileSet.insert(traceItem.filePath);
        //cout << "Find FILE" << traceItem.filePath <<endl;
        break;

    //dir op
    case op_mkdir:
        dirCreateSet.insert(traceItem.filePath);
        dirSet.insert(traceItem.filePath);
        //cout << "Create DIR" << traceItem.filePath <<endl;
        break;
    case op_opendir:
    case op_readdir:
    case op_closedir:
    case op_rmdir:
        dirSet.insert(traceItem.filePath);
        //cout << "Find DIR" << traceItem.filePath <<endl;
        break;
    //other op
    case op_flush:
    case op_symlink:
    case op_error:
        break;

    default:
        break;
    }
}

int genFromFileSet()
{
    std::set<std::string> fileTempSet;
    set_difference(fileSet.begin(), fileSet.end(), fileCreateSet.begin(), fileCreateSet.end(), inserter(fileTempSet, fileTempSet.begin()));
    set_difference(fileTempSet.begin(), fileTempSet.end(), dirSet.begin(), dirSet.end(), inserter(fileGenSet, fileGenSet.begin()));
    set_difference(dirSet.begin(), dirSet.end(), dirCreateSet.begin(), dirCreateSet.end(), inserter(dirGenSet, dirGenSet.begin()));
    set<string>::iterator dirIt = dirGenSet.begin();

    string shellInstruction;
    string parentDirPath;
    while (dirIt != dirGenSet.end())
    {
        shellInstruction = "mkdir -p " + *dirIt;
        system(shellInstruction.c_str());
        ++dirIt;
    }
    set<string>::iterator fileIt = fileGenSet.begin();
    while (fileIt != fileGenSet.end())
    {
        parentDirPath = getParentDirPath(*fileIt);
        if (dirCreateSet.count(parentDirPath))
        {
            fileGenSet.erase(*fileIt);
            //cout << "erase: " << *fileIt << "cause parent: " << parentDirPath <<endl;
        }
        ++fileIt;
    }

    fileIt = fileGenSet.begin();
    while (fileIt != fileGenSet.end())
    {
        shellInstruction = " _dir_new=`dirname " + *fileIt + "` ;" + "mkdir -p $_dir_new; touch " + *fileIt;
        system(shellInstruction.c_str());
        ++fileIt;
    }

    cout << "FILE SIZE:" << fileSet.size() << endl;
    cout << "FILE CREATE SIZE:" << fileCreateSet.size() << endl;
    cout << "DIR SIZE:" << dirSet.size() << endl;
    cout << "DIR CREATE SIZE:" << dirCreateSet.size() << endl;
    cout << "DIR GEN SIZE:" << dirGenSet.size() << endl;
    cout << "FILE GEN SIZE:" << fileGenSet.size() << endl;

    return wrongTraceCount;
}
