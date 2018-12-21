#include "_logMgr.h"
#include "tools/_tools.h"
#include <stdarg.h>
#include <string.h>

#ifndef _MSC_VER
#include <sys/time.h>
#include <sys/stat.h>
#endif

_logUnit _logMgr::m_sLogUnit;

_logMgr::_logMgr(void)
{
	InitializeSRWLock(&m_csLockRW);
	m_mapLog.clear();
}

_logMgr::~_logMgr(void)
{
	for (UNORDERMAPLOG::iterator itor = m_mapLog.begin() ; 
		itor != m_mapLog.end() ; itor++)
	{
		if (itor->second)
		{
			itor->second->closeLogUnit();
			delete itor->second;
		}
	}
	m_mapLog.clear();

	m_sLogUnit.closeLogUnit();

	DeleteSRWLock(&m_csLockRW);
}

bool	_logMgr::initMgr(const std::string strTag /*= "GLB"*/,const std::string &strPath /*= "./log/Running.log"*/,emLOGLEV emLLV /*= LogLev_Debug*/,
	bool bScreen /*= false*/,bool bLock /*= true*/)
{
	createDirectory(strPath.c_str());
	return m_sLogUnit.initLogUnit(strPath,emLLV,bScreen,bLock);
}

bool	_logMgr::initLog(const std::string &strTag,const std::string &strPath,emLOGLEV emLogLevS,bool bScreen /*= false*/,bool bLock /*= false*/)
{
	FCAutoSRWLockW	csLock(&m_csLockRW);

	if (!m_sLogUnit.getInitStat())
		initMgr();

	UNORDERMAPLOG::iterator itor = m_mapLog.find(strTag);
	if (itor != m_mapLog.end())
		return false;

	_logUnit *pLog = new _logUnit();

	if (!pLog->initLogUnit(strPath,emLogLevS,bScreen,bLock))
	{
		delete pLog;
		return false;
	}

	m_mapLog[strTag] = pLog;
	return true;
}

void	_logMgr::log(const std::string &strTag,emLOGLEV emLogLevS,const char *pInfo,const char *pFmt,...)
{
	FCAutoSRWLockR	csLock(&m_csLockRW);

	_logUnit *pLog = NULL;
	UNORDERMAPLOG::iterator itor = m_mapLog.find(strTag);
	if (itor != m_mapLog.end())
		pLog = itor->second;
	else pLog = &m_sLogUnit;

	char	chMsg[4096] = {0};
	try
	{
		va_list	pArg;
		va_start( pArg, pFmt );
		int iRet = 0;
#ifdef _MSC_VER
		iRet = vsprintf_s(chMsg,pFmt,pArg);
#else
		iRet = vsprintf(chMsg,pFmt,pArg);
#endif
		if (iRet < 4096)
			chMsg[iRet] = 0;
		else chMsg[4095] = 0;
		va_end( pArg );
	}
	catch (...)
	{
		memset(chMsg,0,sizeof(chMsg));
	}
	
	pLog->Log(emLogLevS,pInfo,chMsg);
}

_LogMsgTM::_LogMsgTM(const std::string &strTag /*= ""*/,const std::string &strMsg /*= ""*/,emLOGLEV emLev /*= LogLev_Trace*/)
	:	m_strTag( strTag )
	,	m_strMsg( strMsg )
	,	m_emLev( emLev )
{
#ifdef _MSC_VER
	GetLocalTime(&m_tmCrt);
#else
	gettimeofday(&m_tmCrt,NULL);
#endif
}

#ifndef LPTIMEVAL
typedef struct timeval*	LPTIMEVAL;
#endif

#ifdef _MSC_VER
long long	getTimeMSSecond(LPSYSTEMTIME ptm)
#else
long long	getTimeMSSecond(LPTIMEVAL ptm)
#endif
{
	long long llRes = 0;
#ifdef _MSC_VER
	struct tm objTM;
	objTM.tm_year = ptm->wYear;
	objTM.tm_mon = ptm->wMonth - 1;
	objTM.tm_mday = ptm->wDay;
	objTM.tm_hour = ptm->wHour;
	objTM.tm_min = ptm->wMinute;
	objTM.tm_sec = ptm->wSecond;

	llRes = (long long)mktime(&objTM) * 1000 + ptm->wMilliseconds;
#else
	llRes = ptm->tv_sec * 1000 + ptm->tv_usec / 1000;
#endif

	return llRes;
}

#ifdef _MSC_VER
DWORD	getIntervalTM(LPSYSTEMTIME ptm)
#else
DWORD	getIntervalTM(LPTIMEVAL ptm)
#endif
{
	long long llVal = 0;
	long long llNow = 0;
#ifdef _MSC_VER
	SYSTEMTIME sTM;
	GetLocalTime(&sTM);

	llNow = getTimeMSSecond(&sTM);
#else
	struct timeval sTM;
	gettimeofday(&sTM,NULL);

	llNow = getTimeMSSecond(&sTM);
#endif
	llVal = getTimeMSSecond(ptm);

	return llNow - llVal;
}

_LogMsgTM::~_LogMsgTM()
{
	if (m_strMsg.empty())
		LOG(m_strTag,m_emLev,getformatstr("task is [unknown] ,time = [%u ms]",getIntervalTM(&m_tmCrt)).c_str());
	else
		LOG(m_strTag,m_emLev,getformatstr("task is [%s] ,time = [%u ms]",m_strMsg.c_str(),getIntervalTM(&m_tmCrt)).c_str());
}