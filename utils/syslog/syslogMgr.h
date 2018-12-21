#ifndef __SYS_LOG_MGR_H
#define __SYS_LOG_MGR_H

#include "../frame/_lock.h"
#include <string>
#include <syslog.h>

class FCSysLog
{
public:
	static FCSysLog* getInstance()
	{
		static FCSysLog objSysLog;
		return &objSysLog;
	}

protected:
	FCSysLog();

public:	
	~FCSysLog();

	void		initLog();
	void		setConfig(const char *pszIdent,int iOpt,int iFac, bool bEnable = true);
	void		log(int iPri,const char *pFmt,...);

protected:
private:

	std::string		m_strIdent;		/// 消息源
	int				m_iOption;		/// 消息控制标记
	int				m_iFacility;		/// 消息存储位置		LOG_LOCAL1

	bool				m_bEnable;
	CRITICAL_SECTION	m_csLock;			/// 锁
};

#endif	/// __SYS_LOG_MGR_H