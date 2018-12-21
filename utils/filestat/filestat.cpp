/*
 * @Author: huqing 
 * @Date: 2018-11-27 19:10:16 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-28 15:25:38
 */

#include "filestat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#include <Windows.h>
#define strcasecmp	_stricmp
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

/// 判断文件是否存在
bool    FScheckFileExist(const char *pstrFile)
{
    if (!pstrFile || strcasecmp(pstrFile, "") == 0)
        return false;

#ifdef _MSC_VER
    if (_access(pstrFile, 0))
#else
    if (access(pstrFile, 0))
#endif
        return false;
    return true;
}

/// 获取文件大小
long    FSgetFileSize(FILE *pFile)
{
    if (!pFile)
        return -1;

    fseek(pFile, 0, SEEK_END);
    long lLen = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    return lLen;
}


#ifdef _MSC_VER
bool    FScreateDirctory(const char *pstrDir)
#else
bool    FScreateDirctory(const char *pstrDir, int iDirPermission /*= S_IRUSR|S_IWUSR|S_IXUSR*/)
#endif
{
	if (NULL == pstrDir)
	{
		return false;
	}
	const int iPathLength = static_cast<int> (strlen(pstrDir));
	if (iPathLength > 256)
	{
		return false;
	}

	char szPathBuffer[256] = { 0 };
	memcpy(szPathBuffer, pstrDir, iPathLength);
	for (int i = 0; i < iPathLength; ++i)
	{
		char &refChar = szPathBuffer[i];
		if ((('/' == refChar || '\\' == refChar)) && (0 != i))
		{
			refChar = '\0';
			int iStatus = 0;
#ifdef _MSC_VER
			iStatus = _access(szPathBuffer, 0 );
#else
			iStatus = access(szPathBuffer,0);
#endif

			if (0 != iStatus)
			{
				if ((ENOTDIR == errno) || (ENOENT == errno))
				{
#ifdef _MSC_VER
					iStatus = _mkdir(szPathBuffer);
#else
					iStatus = mkdir(szPathBuffer, iDirPermission);
#endif
					
					if (0 != iStatus)
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}

#ifdef _MSC_VER
			refChar = '\\';
#else
			refChar = '/';
#endif
		}
	}
	return true;
}


/// 文件删除
bool    FSdeleteFile(const char *pstrFile)
{
    if (!FScheckFileExist(pstrFile))
        return true;

    return remove(pstrFile) ? false : true;
}

/// 文件拷贝
bool    FScopyFile(const char *pstrSrc, const char *pstrDest, bool bforce /*= true*/)
{
    if (!FScheckFileExist(pstrSrc) || !pstrDest)
        return false;

#ifdef _MSC_VER
    return CopyFile(pstrSrc, pstrDest, FALSE) ? true : false;
#else
    char szcmd[1024] = {0};
    sprintf(szcmd, "cp %s %s %s", bforce ? "-f" : "", pstrSrc, pstrDest);
    return system(szcmd) ? false : true;
#endif
}

/// 文件移动
bool    FSmoveFile(const char *pstrSrc, const char *pstrDest, bool bforce /*= true*/)
{
    if (!FScheckFileExist(pstrSrc) || !pstrDest)
        return false;

#ifdef _MSC_VER
    return MoveFile(pstrSrc, pstrDest) ? true : false;
#else
    char szcmd[1024] = {0};
    sprintf(szcmd, "mv %s %s %s", bforce ? "-f" : "", pstrSrc, pstrDest);
    return system(szcmd) ? false : true;
#endif
}

/// 文件重命名
bool    FSrenameFile(const char *pstrOld, const char *pstrNew)
{
    if (!FScheckFileExist(pstrOld))
        return false;

    return rename(pstrOld, pstrNew) ? false : true;
}