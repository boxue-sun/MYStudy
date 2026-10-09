/*********************************************************************************
 * @file		file_util.h
 * @brief		file_util belongs to CICTCI
 * @details
 * @author		cs
 * @date		 2024-02-01
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *   2024-02-01 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _FILEUTIL_H
#define _FILEUTIL_H
#include "define.h"
#include <string>
NAMESPACE_AFL_START

namespace FileUtil
{
string getBinaryPath();
string getBinaryName();
string getBinaryDir();


bool   checkDirectory(std::string dirPath);
bool   isDirectory(const char* dir);
bool   createRecursionDir(const char* dir);

string dirName(const char* dir);
string baseName(const char* dir);

bool   isFileExist(const char* filepath);
long   getFileSize(FILE* file);
long   getFileSize(const char* filepath);
size_t readFile(const char* filepath, std::string& buf);
}

using namespace FileUtil;

NAMESPACE_AFL_END
#endif /* _FILEUTIL_H */
