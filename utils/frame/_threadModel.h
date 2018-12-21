#ifndef _WIN_THREAD_MODEL_H
#define _WIN_THREAD_MODEL_H

#include "_lock.h"
#include <exception>
#include <string>
#include <string.h>

#ifndef _MSC_VER
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#else
#include <process.h>
#endif

class _LOGERR
{
public:
	static _LOGERR*	_Ins();
	~_LOGERR();

	void		log(const char *pMsg);

protected:
	_LOGERR();

	void		initLogErr();
private:
	std::string			m_strErrLog;
	FILE*				m_pFile;
	CRITICAL_SECTION	m_csLock;
};


#ifdef _MSC_VER
static DWORD CALLBACK threadProc(LPVOID lp);
#else
static void*	threadProc(void *lp);
#endif

class FCThreadModel
{
public:
	FCThreadModel();
	virtual ~FCThreadModel();

	bool	startSvr(int iDelay = 0);
	void	stopSvr();

	virtual bool	initialModel() = 0;
	virtual bool	exitModel() = 0;

	virtual	void	setExitFlag()	{	m_bExitFlag = true;	}
	virtual bool	checkExitFlag()	{	return m_bExitFlag;	}

#ifdef _MSC_VER
	DWORD		doThreadProc();
#else
	void*		doThreadProc();
#endif
protected:

	virtual	bool	runloop() = 0;
	virtual	void	unInitModel(){}
private:
#ifdef _MSC_VER
	HANDLE		m_hThread;
#else
	pthread_t	m_hThread;
#endif
	
	bool		m_bExitFlag;

	int		m_iDelay;			/// start delay	(s)
};

#endif	/// _WIN_THREAD_MODEL_H