#ifndef _HUQG_LOG_MGR_H
#define _HUQG_LOG_MGR_H

#include "_logUnit.h"
#include <string>
#include "frame/_lock.h"
#include "frame/_singleton.h"
#include "tools/_tools.h"

#ifdef _MSC_VER
#include <unordered_map>
typedef std::unordered_map< std::string, _logUnit* >		UNORDERMAPLOG;
#else
#include <tr1/unordered_map>
typedef std::tr1::unordered_map< std::string, _logUnit* >	UNORDERMAPLOG;
#endif

class _logMgr	:	public _singleton< _logMgr >
{
	friend class _singleton< _logMgr >;
public:
	~_logMgr(void);

	static bool	initMgr(const std::string strTag = "GLB",const std::string &strPath = "./log/Running.log",emLOGLEV emLLV = LogLev_Debug,
		bool bScreen = false,bool bLock = true);

	bool	initLog(const std::string &strTag,const std::string &strPath,emLOGLEV emLogLevS,bool bScreen = false,bool bLock = false);
	void	log(const std::string &strTag,emLOGLEV emLogLevS,const char *pInfo,const char *pFmt,...);
protected:
	_logMgr(void);

private:
	UNORDERMAPLOG		m_mapLog;
	SRWLOCK				m_csLockRW;
	static _logUnit		m_sLogUnit;			/// 全局
};

#define		LOGMGR				_logMgr::GetInstance()
#define		LOG(TAG,LEV,...)	LOGMGR->log(TAG,LEV,__FFL_INFO__,__VA_ARGS__)
#define		TRACE(TAG,...)		LOGMGR->log(TAG,LogLev_Trace,__FFL_INFO__,__VA_ARGS__)
#define		DEBUG(TAG,...)		LOGMGR->log(TAG,LogLev_Debug,__FFL_INFO__,__VA_ARGS__)
#define		INFO(TAG,...)		LOGMGR->log(TAG,LogLev_Info,__FFL_INFO__,__VA_ARGS__)
#define		WARN(TAG,...)		LOGMGR->log(TAG,LogLev_Warning,__FFL_INFO__,__VA_ARGS__)
#define		FAIL(TAG,...)		LOGMGR->log(TAG,LogLev_Fail,__FFL_INFO__,__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
///	打印任务处理时间间隔
#include <time.h>
class _LogMsgTM
{
public:
	_LogMsgTM(const std::string &strTag = "",const std::string &strMsg = "",emLOGLEV emLev = LogLev_Trace);
	~_LogMsgTM();

protected:

private:
#ifdef _MSC_VER
	SYSTEMTIME		m_tmCrt;
#else
	struct timeval	m_tmCrt;
#endif
	std::string		m_strTag;
	std::string		m_strMsg;
	emLOGLEV			m_emLev;
};


#endif	/// _HUQG_LOG_MGR_H