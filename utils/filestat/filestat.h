/*
 * @Author: huqing 
 * @Date: 2018-11-27 19:08:45 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-28 15:15:08
 */

#include <stdio.h>

#ifndef _MSC_VER
#include <sys/stat.h>
#endif


/// 判断文件是否存在
bool    FScheckFileExist(const char *pstrFile);

/// 获取文件大小
long    FSgetFileSize(FILE *pFile);

/// 创建目录
#ifdef _MSC_VER
bool    FScreateDirctory(const char *pstrDir);
#else
bool    FScreateDirctory(const char *pstrDir, int iDirPermission = S_IRUSR|S_IWUSR|S_IXUSR);
#endif

/// 文件删除
bool    FSdeleteFile(const char *pstrFile);

/// 文件拷贝
bool    FScopyFile(const char *pstrSrc, const char *pstrDest, bool bforce = true);

/// 文件移动
bool    FSmoveFile(const char *pstrSrc, const char *pstrDest, bool bforce = true);

/// 文件重命名
bool    FSrenameFile(const char *pstrOld, const char *pstrNew);