#include "syslogMgr.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

FCSysLog::FCSysLog()
	:	m_strIdent( "" )
	,	m_iOption( 0 )
	,	m_iFacility( 0 )
	,	m_bEnable( false )
{
	InitializeCriticalSection(&m_csLock);
}

FCSysLog::~FCSysLog()
{
	closelog();
	DeleteCriticalSection(&m_csLock);
}

void		FCSysLog::initLog()
{
	openlog(m_strIdent.c_str(),m_iOption,m_iFacility);
}

void		FCSysLog::setConfig(const char *pszIdent,int iOpt,int iFac,bool bEnable /*= true*/)
{
	if (pszIdent)
		m_strIdent = pszIdent;

	m_iOption = iOpt;
	m_iFacility = iFac;
	m_bEnable = bEnable;
}

void		FCSysLog::log(int iPri,const char *pFmt,...)
{
	if (!m_bEnable)
		return;

	FCAutoLock	atLock(&m_csLock);

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
	syslog(iPri,chMsg);
}