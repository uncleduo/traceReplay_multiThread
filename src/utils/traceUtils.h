#ifndef TRACE_UTILS_H
#define TRACE_UTILS_H

#include <vector>
#include <string>
using namespace std;

string getParentDirPath(string filePath);

//copy splitCode from Internet
std::vector< std::string > splitString_STL(const std::string& str, const std::string& pattern);
#endif