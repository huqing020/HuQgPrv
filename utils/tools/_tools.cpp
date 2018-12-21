/*
 * Tools.cpp
 *
 *  Created on: Feb 10, 2015
 *      Author: root
 */

#include "_tools.h"

#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#else
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef va_copy
#define va_copy(destination, source) ((destination) = (source))
#endif /// va_copy

int			fc_StrToInt(std::string strVal)
{
	int iVal = 0;
	fc_memcpy(&iVal,strVal.c_str(),strVal.size());
	return iVal;
}

#define NUM_UNIT_LEN 10240
std::string getformatstr(const char *pszFmt,va_list &pArg)
{
	std::string strRes = "";
	size_t stLen = 4096;
	char *pMsg = NULL;

ALLOCMEM:

	va_list vlCpy;	
	va_copy(vlCpy, pArg);
	int iLen = vsnprintf(NULL, 0, pszFmt, vlCpy);
	va_end(vlCpy);

	if (strlen(pszFmt) + iLen > stLen)
		stLen += iLen;

	pMsg = (char *)calloc(1, stLen);
	if (!pMsg)
		return "";

	try 
	{
#if defined _MSC_VER || defined _WIN32 || defined _WIN64
		int iRet = vsprintf_s(pMsg, stLen - 1, pszFmt, pArg);
#else
		int iRet = vsprintf(pMsg, pszFmt, pArg);
#endif

		if (iRet < 0)
		{
			stLen += NUM_UNIT_LEN;
			goto ALLOCMEM;
		}

		pMsg[iRet] = 0;
	}
	catch (...)
	{
	}
	strRes = pMsg;
	free(pMsg);
	return strRes;
}

std::string getformatstr(const char *lpszFormat,...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start( pArg, lpszFormat );
		strRes = getformatstr(lpszFormat,pArg);
		va_end( pArg );
	}
	catch (...)
	{
		strRes = "";
	}
	return strRes;
}

/// 0 YmdHMS	1 Ymd	2HMS	default 0
long long 	getDTNow(int iFormat/* = 0*/,bool bUtc/* = false*/)
{
	if (iFormat == 0)
		return StrToDigit<long long>(getTimeNow());
	else if (iFormat == 1)
		return StrToDigit<long long>(getTimeNow(2));
	else if (iFormat == 2)
		return StrToDigit<long long>(getTimeNow(3));
	return StrToDigit<long long>(getTimeNow());
}

/// 0 YmdHMS	1 Y-m-d H:M:S	2 Ymd	3HMS	default 0
std::string	getTimeNow(int iFormat/* = 0*/,bool bUtc/* = false*/)
{
	time_t ttm;
	time(&ttm);
	return getSpecialDT(ttm,iFormat,bUtc);
}

/// 0 YmdHMS	1 Y-m-d H:M:S	2 Ymd	3HMS	default 0
std::string	getTimeNowEx(int iFormat/* = 0*/,bool bUtc/* = false*/)
{
	char szDT[48] = {0};

#ifdef _MSC_VER
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	sprintf_s(szDT,"%s.%03ld",getSpecialDT(time(NULL),iFormat,bUtc).c_str(),systm.wMilliseconds);
#else
	struct timeval tv;
	gettimeofday(&tv,NULL);
	sprintf(szDT,"%s.%03ld",getSpecialDT(tv.tv_sec,iFormat,bUtc).c_str(),tv.tv_usec/1000);
#endif
	return szDT;
}

/// 0 YmdHMS	1 Y-m-d H:M:S	2 Ymd	3HMS	default 0
std::string	getSpecialDT(time_t time,int iFormat/* = 0*/,bool bUtc/* = false*/)
{
	tm objtm;

#ifdef _MSC_VER
	if (bUtc)	gmtime_s(&objtm,&time);
	else		localtime_s(&objtm,&time);
#else
	if (bUtc)	gmtime_r(&time,&objtm);
	else		localtime_r(&time, &objtm);
#endif
	char chDT[30] = {0};
	std::string strFormat = "";
	switch (iFormat)
	{
	case 0:
		strFormat = "%Y%m%d%H%M%S";
		break;
	case 1:
		strFormat = "%Y-%m-%d %H:%M:%S";
		break;
	case 2:
		strFormat = "%Y%m%d";
		break;
	case 3:
		strFormat = "%H%M%S";
		break;
	default:
		strFormat = "%Y%m%d%H%M%S";
		break;
	}
	size_t stRet = strftime(chDT,29,strFormat.c_str(),&objtm);
	chDT[stRet] = 0;
	return chDT;
}

time_t		getTimeTBySpecialDT(int iYear,int iMon,int iDay,int iHour,int iMin,int iSec,bool bUtc/* = false*/)
{
	tm objtm;
	memset(&objtm,0,sizeof(tm));
	objtm.tm_year 	= iYear - 1900;	objtm.tm_mon	= iMon - 1;
	objtm.tm_mday	= iDay;			objtm.tm_hour	= iHour;
	objtm.tm_min 	= iMin;			objtm.tm_sec	= iSec;
#ifdef _MSC_VER
	if (bUtc)	return _mkgmtime(&objtm);
#else
	if (bUtc)	return timegm(&objtm);
#endif
	return mktime(&objtm);
}

long long	getOffTM(int iYear,int iMon,int iDay,int iHour,int iMin,int iSec,
	int iYearOff,int iMonOff,int iDayOff,int iHourOff,int iMinOff,int iSecOff,int iFormat /*= 0*/,bool bUtc /*= false*/)
{
	tm objtm;
	memset(&objtm,0,sizeof(tm));
	objtm.tm_year 	= iYear - 1900 + iYearOff;	
	objtm.tm_mon	= iMon - 1 + iMonOff;
	objtm.tm_mday	= iDay + iDayOff;
	objtm.tm_hour	= iHour + iHourOff;
	objtm.tm_min 	= iMin + iMinOff;			
	objtm.tm_sec	= iSec + iSecOff;

	time_t ttm = 0;
#ifdef _MSC_VER
	if (bUtc)	
		ttm = _mkgmtime(&objtm);
#else
	if (bUtc)	
		ttm = timegm(&objtm);
#endif
	else
		ttm = mktime(&objtm);

#ifdef _MSC_VER
	if (bUtc)	gmtime_s(&objtm,&ttm);
	else		localtime_s(&objtm,&ttm);
#else
	if (bUtc)	gmtime_r(&ttm,&objtm);
	else		localtime_r(&ttm, &objtm);
#endif

	long long dwTM = 0;
	switch (iFormat)
	{
	case 0:	/// YYYYMMDDHHMMSS
		dwTM = (objtm.tm_year + 1900) * 10000000000 + (objtm.tm_mon + 1) * 100000000 + objtm.tm_mday * 1000000 +
			objtm.tm_hour * 10000 + objtm.tm_min * 100 + objtm.tm_sec;
		break;
	case 1:	/// YYYYMMDD
		dwTM = (objtm.tm_year + 1900) * 10000 + (objtm.tm_mon + 1) * 100 + objtm.tm_mday;
		break;
	case 2:	/// HHMMSS
		dwTM = objtm.tm_hour * 10000 + objtm.tm_min * 100 + objtm.tm_sec;
		break;
	default:
		dwTM = 0;
		break;
	}
	return dwTM;
}

/// 获取最近周一 0为周日  1为周一...   dwDT格式为YYYYMMDD
time_t		getSpecialWeekDay(int iYear,int iMon,int iDay,int iHour,int iMin,int iSec,int iWeekDay /*= 0*/,bool bUtc /*= false*/)
{
	struct tm objTM;
	memset(&objTM,0,sizeof(objTM));
	objTM.tm_year = iYear - 1900;
	objTM.tm_mon = iMon - 1;
	objTM.tm_mday = iDay;
	objTM.tm_hour = iHour;
	objTM.tm_min = iMin;
	objTM.tm_sec = iSec;

	time_t ttm = 0;
#ifdef _MSC_VER
	if (bUtc)	
		ttm = _mkgmtime(&objTM);
#else
	if (bUtc)	
		ttm = timegm(&objTM);
#endif
	else
		ttm = mktime(&objTM);

#ifdef _MSC_VER
	if (bUtc)	gmtime_s(&objTM,&ttm);
	else		localtime_s(&objTM,&ttm);
#else
	if (bUtc)	gmtime_r(&ttm,&objTM);
	else		localtime_r(&ttm,&objTM);
#endif

	objTM.tm_mday -= objTM.tm_wday - iWeekDay;

	ttm = 0;
#ifdef _MSC_VER
	if (bUtc)	
		ttm = _mkgmtime(&objTM);
#else
	if (bUtc)	
		ttm = timegm(&objTM);
#endif
	else
		ttm = mktime(&objTM);

	return ttm;
}

struct tm	getTMBySpecialDT(time_t time,bool bUtc/* = false*/)
{
	struct tm _tm;

#ifdef _MSC_VER
	if (bUtc)
		gmtime_s(&_tm,&time);
	else localtime_s(&_tm,&time);
#else
	if (bUtc)
		gmtime_r(&time,&_tm);
	else localtime_r(&time,&_tm);
#endif	
	return _tm;
}

//////////////////////////////////////////////////////////////////////////
///判断周六、周日	0=周六	1=周日	-1
int			whetherWeekEnd(time_t tt,bool bUtc/* = false*/)
{
	tm objtm;

#ifdef _MSC_VER

	if (bUtc)	gmtime_s(&objtm,&tt);
	else		localtime_s(&objtm,&tt);

#else

	if (bUtc)	gmtime_r(&tt,&objtm);
	else		localtime_r(&tt,&objtm);

#endif

	int iRet = -1;
	switch (objtm.tm_wday)
	{
	case 0:
		iRet = 1;
		break;
	case 6:
		iRet = 0;
		break;
	default:
		break;
	}
	return iRet;
}

int			whetherWeekEnd(int iYear,int iMon,int iDay,bool bUtc /*= false*/)
{
	return whetherWeekEnd(getTimeTBySpecialDT(iYear,iMon,iDay,0,0,0,bUtc),bUtc);
}

int			getWeek(time_t tt,bool bUtc /*= false*/)
{
	tm objtm;

#ifdef _MSC_VER

	if (bUtc)	gmtime_s(&objtm,&tt);
	else		localtime_s(&objtm,&tt);

#else

	if (bUtc)	gmtime_r(&tt,&objtm);
	else		localtime_r(&tt,&objtm);

#endif
	return objtm.tm_wday;	/// days since Sunday - [0,6]
}
int			getWeek(int iYear,int iMon,int iDay,bool bUtc /*= false*/)
{
	return getWeek(getTimeTBySpecialDT(iYear,iMon,iDay,0,0,0,bUtc),bUtc);
}

void 		fc_memcpy(void *pTgt,const void *pSource,int ilen)
{
	if (!pTgt) return;
	memcpy(pTgt,pSource,ilen);
}

long 		getRandom(int iRgMax/* = 100*/,int iRgMin/* = 0*/)
{
	srand((unsigned int)time(NULL));
	
	return rand()%iRgMax + iRgMin;
}

std::string	getFFLInfo(const char *pFile ,const char * pFunc,int ilen)
{
	return getformatstr("(%s#%d) <%s>",pFile,ilen,pFunc);
}


#ifdef _MSC_VER
char* 		MapMemFile (const char* szFileName, unsigned int length,HANDLE &handleFile,HANDLE &handleMap)
{
	if (_access(szFileName,0))
		return NULL;

	handleFile = CreateFile(szFileName,
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (!handleFile || handleFile == INVALID_HANDLE_VALUE)
		return NULL;

	DWORD dwLow = 0;
	dwLow = GetFileSize(handleFile,NULL);
	
	handleMap = CreateFileMapping(handleFile,NULL,PAGE_READWRITE,0,dwLow,NULL);
	if (!handleMap || handleMap == INVALID_HANDLE_VALUE)
	{
		CloseHandle(handleFile);
		handleFile = NULL;
		return NULL;
	}

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	DWORD dwAlcGrnLty = info.dwAllocationGranularity;

	DWORD dwLen = (dwLow / dwAlcGrnLty + 1) * dwAlcGrnLty;
	char *pBuff = (char *)MapViewOfFile(handleMap,FILE_MAP_ALL_ACCESS,0,0,dwLen);
	if (!pBuff)
	{
		CloseHandle(handleMap);
		CloseHandle(handleFile);
		handleMap = NULL;
		handleFile = NULL;
	}

	return pBuff;
}

void			unMapMemFile(char *pBuf,unsigned int lenght,HANDLE &handleFile,HANDLE &handleMap)
{
	if (!handleFile || handleFile == INVALID_HANDLE_VALUE || !handleMap || handleMap == INVALID_HANDLE_VALUE || !pBuf)
		return;

	UnmapViewOfFile(pBuf);
	CloseHandle(handleMap);
	CloseHandle(handleFile);
}
#else
char* 		MapMemFile (const char* szFileName, unsigned int length)
{
	char*    point = NULL;
	unsigned int    len = 0;
	bool     bCanInit = false;

	int fd = open(szFileName, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
	if (fd < 0)
	{
		//文件已经存在
		fd = open(szFileName, O_RDWR, S_IRUSR|S_IWUSR);
		if (fd < 0)
		{
			return NULL;
		}

		len = lseek(fd, 0, SEEK_END);
		bCanInit = false;
	}
	else
	{
		len = 0;
		bCanInit = true;
	}

	if (fd < 0)
	{
		return NULL;
	}
	if (len < length)
	{
		ftruncate (fd, length);
	}
	point = (char *)mmap(0, length, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
	if (point == NULL)
	{
		return NULL;
	}
	close (fd);

	if (bCanInit)
	{
		memset(point, 0, length);
	}
	return point;
}

void			unMapMemFile(char *pBuf,unsigned int length)
{
	if (!pBuf || !length)
		return;
	munmap(pBuf,length);
}
#endif 

bool 		createDirectory(const char *szDirectoryPath, int iDirPermission/* = 0744*/)
{
	if (NULL == szDirectoryPath)
	{
		return false;
	}
	const int iPathLength = static_cast<int> (strlen(szDirectoryPath));
	if (iPathLength > 256)
	{
		return false;
	}

	char szPathBuffer[256] = { 0 };
	memcpy(szPathBuffer, szDirectoryPath, iPathLength);
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
					iStatus = mkdir(szPathBuffer,S_IRUSR|S_IWUSR|S_IXUSR);
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


bool		copyFile(const char* pExitFile,const char *pNewFile, bool bFailifExit /*= false*/)
{
#ifdef _MSC_VER
	if (!CopyFile(pExitFile,pNewFile,bFailifExit ? TRUE : FALSE))
		return false;
#else
	char szCmd[1024] = {0};
	sprintf(szCmd,"cp %s %s %s",bFailifExit ? "":"-f",pExitFile,pNewFile);
	if (system(szCmd))
		return false;
#endif
	return true;
}

bool		moveFile(const char* pExitFile,const char *pNewFile, bool bFailifExit /*= false*/)
{
#ifdef _MSC_VER
	if (!MoveFile(pExitFile,pNewFile))
		return false;
#else
	char szCmd[1024] = {0};
	sprintf(szCmd,"mv %s %s %s",bFailifExit ? "":"-f",pExitFile,pNewFile);
	if (system(szCmd))
		return false;
#endif
	return true;
}

std::string	getExeDir()
{
#ifndef MAX_PATH
#define MAX_PATH 256
#endif

	char szPath[MAX_PATH] = {0};

#ifdef _MSC_VER

	GetModuleFileName(NULL,szPath,MAX_PATH);

#else

	//获取当前程序绝对路径    
	int cnt = readlink("/proc/self/exe", szPath, MAX_PATH);
	if (cnt < 0 || cnt >= MAX_PATH)
	{
		throw(strerror(errno));
	}

#endif
	
	std::string strDir = szPath;
	size_t stPos = strDir.rfind('\\');
	if (stPos == std::string::npos)
		stPos = strDir.rfind('/');

	if (stPos == std::string::npos)
		return strDir;

	return strDir.substr(0,stPos + 1);
}

static	unsigned int	sDwGlobalSeq = 0;
//////////////////////////////////////////////////////////////////////////
/// 获取全局SEQ
unsigned int getGlobalSEQ()
{
#ifdef _MSC_VER
	return InterlockedIncrement(&sDwGlobalSeq);
#else
	return __sync_add_and_fetch(&sDwGlobalSeq,1);
#endif
}

/// 校验和
unsigned int generateCheckSum(char *pBuf,unsigned int uLen)
{
	unsigned int i = 0,uCKS = 0;
	for (i = 0,uCKS = 0; i < uLen;uCKS += (unsigned int)pBuf[i++]);

	return getCheckSum(uCKS);
}
unsigned int getCheckSum(unsigned int uNumber)
{
	return uNumber % 256;
}

bool		 splitStr(const std::string &strSrc,const std::string &strSign,std::vector< std::string > &vtRes)
{
	if (strSrc.empty())
		return true;

	size_t sPos = 0,sOffSet = 0;

	sPos = strSrc.find(strSign.c_str());
	while (sPos != std::string::npos)
	{
		vtRes.push_back(strSrc.substr(sOffSet,sPos - sOffSet));
		sOffSet = sPos + strSign.length();

		if (sOffSet >= strSrc.length())
			break;

		sPos = strSrc.find(strSign.c_str(),sOffSet);
	}

	if (sPos == std::string::npos)
		vtRes.push_back(strSrc.substr(sOffSet,strSrc.length() - sOffSet));
	
	return true;
}

void		ToNetBytesOrder(const void* pSrc, void* pDest, int iSize)
{
	if (NULL != pSrc && NULL != pDest)
	{
		char* p1 = (char*)pSrc;
		char* p2 = (char*)pDest;
		for (int i = 0; i < iSize; i++)
		{
			p2[iSize - 1 - i] = p1[i];
		}
	}
}

int			SetBinaryMsgHead(unsigned int uMsgType, unsigned int uBodyLength, char* pBuffer, int iBufferLen)
{
	if (NULL == pBuffer || (unsigned int)iBufferLen < 2*sizeof(unsigned int ))
	{
		return -1;
	}

	ToNetBytesOrder(&uMsgType, pBuffer, sizeof(unsigned int ));
	ToNetBytesOrder(&uBodyLength, pBuffer+sizeof(unsigned int ), sizeof(unsigned int ));
	return 2 * sizeof(unsigned int );
}