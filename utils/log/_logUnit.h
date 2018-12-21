#ifndef _HUQG_LOG_UNIT_H
#define _HUQG_LOG_UNIT_H

#include <string>
#include "frame/_lock.h"

enum	emLOGLEV
{
	LogLev_Fail = 0,
	LogLev_Warning,
	LogLev_Info,
	LogLev_Debug,
	LogLev_Trace,
};

class _logUnit
{
public:
	_logUnit(void);
	~_logUnit(void);

	bool	initLogUnit(const std::string &strPath,emLOGLEV emLogLevS,bool bScreen = false,bool bLock = false);
	void	closeLogUnit();

	void	Log(emLOGLEV emLogLev,const char *pInfo,const char *pMsg);

	bool	getInitStat()	{	return m_bStat;	}
private:
	bool	checkLogUnit();
	bool	openLogUnit();

	bool	checkBackUp();
	bool	backUpFile(bool bInit = false);

	void	printScreen(std::string &strMsg);
private:
	bool				m_bLock;			/// 文件锁
	bool				m_bScreen;		/// 打印到屏幕
	std::string		m_strLogFile;

	emLOGLEV			m_emLogLev;

	/// 
	CRITICAL_SECTION	m_csLock;

#ifdef _MSC_VER
	HANDLE				m_hFile;
#else
	int					m_hFile;
#endif

	DWORD				m_dwDT;
	bool				m_bStat;
};

#endif	/// _HUQG_LOG_UNIT_H