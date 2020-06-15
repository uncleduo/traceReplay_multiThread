#include "traceUtils.h"

using namespace std;

string getParentDirPath(string filePath){
    int splitPos;
    for (splitPos = filePath.size()-1; splitPos >=0; splitPos--)
    {
        if (filePath[splitPos]=='/')
        {
            return filePath.substr(0, splitPos);
        }
    }
    return "";
}

//copy splitCode from Internet
std::vector< std::string > splitString_STL(const std::string& str, const std::string& pattern){
    std::vector<std::string> subStrings;
    if (str.empty())
        return subStrings;
    std::string strs = str + pattern;

    size_t pos = strs.find(pattern);
    size_t size = strs.size();

    while (pos != std::string::npos) {
        std::string sub = strs.substr(0,pos);
        subStrings.push_back(sub);
        strs = strs.substr(pos+1,size);
        pos = strs.find(pattern);
    }
    return subStrings;
}