/*
 * _tools.h
 *
 *  Created on: Feb 10, 2015
 *      Author: root
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <errno.h>
#include <math.h>
#include <cmath>
#include <vector>
using namespace std;

template<class T>
string		VarToStr(const T &objT)
{
	std::stringstream ss;
	ss << objT;
	return ss.str();
}

template<class T>
T			StrToDigit(string strVal)
{
	T tVal;
	std::stringstream ss(strVal);
	if ((ss>>tVal).fail())
	{
		tVal = 0;
	}
	return tVal;
}

template <class T>
std::string	fc_NumToStr(T tVal)
{
	char chVal[128] = {0};
	memcpy(chVal,&tVal,sizeof(T));
	return chVal;
}

int			fc_StrToInt(std::string strVal);

std::string	getformatstr(const char *lpszFormat,...);

/// 0 YmdHMS	1 Ymd	2	HMS		default 0
long long	getDTNow(int iFormat = 0,bool bUtc = false);
/// 0 YmdHMS	1 Y-m-d H:M:S	2 Ymd	3HMS	default 0
std::string	getTimeNow(int iFormat = 0,bool bUtc = false);
/// 0 YmdHMS	1 Y-m-d H:M:S	2 Ymd	3HMS	default 0
std::string	getTimeNowEx(int iFormat = 0,bool bUtc = false);
/// 0 YmdHMS	1 Y-m-d H:M:S	2 Ymd	3HMS	default 0
std::string	getSpecialDT(time_t time,int iFormat = 0,bool bUtc = false);
time_t 		getTimeTBySpecialDT(int iYear,int iMon,int iDay,int iHour,int iMin,int iSec,bool bUtc = false);
struct tm	getTMBySpecialDT(time_t time,bool bUtc = false);

long long	getOffTM(int iYear,int iMon,int iDay,int iHour,int iMin,int iSec,
	int iYearOff,int iMonOff,int iDayOff,int iHourOff,int iMinOff,int iSecOff,int iFormat = 0,bool bUtc = false);

/// 获取最近周一 0为周日  1为周一...   dwDT格式为YYYYMMDD
time_t		getSpecialWeekDay(int iYear,int iMon,int iDay,int iHour,int iMin,int iSec,int iWeekDay = 0,bool bUtc = false);

///判断周六、周日	0=周六	1=周日	-1
int			whetherWeekEnd(time_t tt,bool bUtc = false);
int			whetherWeekEnd(int iYear,int iMon,int iDay,bool bUtc = false);
int			getWeek(time_t tt,bool bUtc = false);
int			getWeek(int iYear,int iMon,int iDay,bool bUtc = false);

void 		fc_memcpy(void *pTgt,const void *pSource,int ilen);
long 		getRandom(int iRgMax = 100,int iRgMin = 0);

std::string	getFFLInfo(const char *pFile ,const char * pFunc,int ilen);

#define		__TIMENOW_(i,j)			getTimeNow(i,j).c_str()
#define		__TIMENOW_EX_(i,j)		getTimeNowEx(i,j).c_str()
#define 	__FFL_INFO__			getFFLInfo(__FILE__,__FUNCTION__,__LINE__).c_str()

#ifdef _WIN32
char* 		MapMemFile (const char* szFileName, unsigned int length,HANDLE &handleFile,HANDLE &handleMap);
void		unMapMemFile(char *pBuf,unsigned int lenght,HANDLE &handleFile,HANDLE &handleMap);
#else
char* 		MapMemFile (const char* szFileName, unsigned int length);
void		unMapMemFile(char *pBuf,unsigned int length);
#endif

bool 		createDirectory(const char *szDirectoryPath, int iDirPermission = 0744);
bool		copyFile(const char* pExitFile,const char *pNewFile, bool bFailifExit = false);
bool		moveFile(const char* pExitFile,const char *pNewFile, bool bFailifExit = false);

std::string	getExeDir();

//////////////////////////////////////////////////////////////////////////
/// 获取全局SEQ
unsigned int getGlobalSEQ();

/// 校验和
unsigned int generateCheckSum(char *pBuf,unsigned int uLen);
unsigned int getCheckSum(unsigned int uNumber);
bool		 splitStr(const std::string &strSrc,const std::string &strSign,std::vector< std::string > &vtRes);

#define EPSILON_FLOAT 1E-7
#define EPSILON_DOUBLE 1E-16
inline bool floatIsZero(float &fVal)
{
	if (fabs(fVal) <= EPSILON_FLOAT)
		return true;
	return false;
}

inline bool doubleIsZero(double &dVal)
{
	if (fabs(dVal) <= EPSILON_DOUBLE)
		return true;
	return false;
}

/// double转整形，四舍五入
inline double round(double r)  
{  
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);  
}  

//转换成网络字节序
void		ToNetBytesOrder(const void* pSrc, void* pDest, int iSize);
int			SetBinaryMsgHead(unsigned int uMsgType, unsigned int uBodyLength, char* pBuffer, int iBufferLen);

#endif /* TOOLS_H_ */