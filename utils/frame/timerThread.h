#ifndef __HEAD_TIME_THREAD__
#define __HEAD_TIME_THREAD__

#include "_threadModel.h"
#include "datatype.h"
#include <map>

typedef void (* lpfnTimer)(HWND, UINT, UINT);
#ifndef TIMERPROC
#define TIMERPROC lpfnTimer
#endif

#pragma pack(1)
typedef struct _tgTimer
{
	struct _tgTmKey{
		HWND		hWnd;				/// 定时器所有者句柄 64位下，HWND占8字节，UINT占4字节
		UINT		uIDEvent;			/// 时间ID

		bool operator < (const _tgTmKey& tgKey) const
		{
			if (this->hWnd < tgKey.hWnd)
				return true;
			if (this->hWnd == tgKey.hWnd && this->uIDEvent < tgKey.uIDEvent)
				return true;

			return false;
		}

		bool operator == (const _tgTmKey& tgKey)	const
		{
			if (this->hWnd == tgKey.hWnd && this->uIDEvent == tgKey.uIDEvent)
				return true;
			return false;
		}
	};
	_tgTmKey	tgKey;
	UINT		uType;				/// 类型 0新增	1删除
	UINT		uElapse;			/// 事件触发间隔
	UINT		uLastTM;			/// 最近触发事件
	TIMERPROC	timerProc;			/// 回调函数
}stTgTimer,*LPstTgTimer;
#pragma pack()

class Timer
{
public:
	Timer()			
	{
		InitializeCriticalSection(&m_csLockR);	
		InitializeCriticalSection(&m_csLockW);
	}
	virtual ~Timer()
	{	
		DeleteCriticalSection(&m_csLockR);
		DeleteCriticalSection(&m_csLockW);
	}

	virtual	void OnTimer(HWND hWnd,UINT nIDEvent){}

	UINT		SetTimer(UINT nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc = NULL)
	{
		return	SetTimer(getTimerHWnd(), nIDEvent, uElapse, lpTimerFunc);
	}

	BOOL		KillTimer(UINT uIDEvent)
	{
		return	KillTimer(getTimerHWnd(), uIDEvent);
	}

	int			DealTimer()
	{
		int iRet = -1;
		time_t tmNow = time(NULL);
		LSTTIMERPTR lstEventDl;lstEventDl.clear();
		LSTTIMER lstEventWait;lstEventWait.clear();
		{
			NTLockGuard lock(&m_csLockW);
			lstEventWait.swap(m_lstEventWait);
		}

		{
			NTLockGuard	lock(&m_csLockR);
			if (m_mapEvent.empty() && lstEventWait.empty())
				return -1;

			while (!lstEventWait.empty())
			{
				_tgTimer stTimer = lstEventWait.front();
				lstEventWait.pop_front();

				if (0 == stTimer.uType)
				{/// 新增
					m_mapEvent.insert(std::pair<_tgTimer::_tgTmKey, _tgTimer>(stTimer.tgKey,stTimer));
				}
				else
				{/// 删除
					MAPTIMER::iterator mItor = m_mapEvent.find(stTimer.tgKey);
					if (mItor != m_mapEvent.end())
						m_mapEvent.erase(mItor);
				}
			}

			lstEventWait.clear();
			for (MAPTIMER::iterator itor = m_mapEvent.begin() ; itor != m_mapEvent.end() ; itor++)
			{
				if (itor->second.uElapse + itor->second.uLastTM > tmNow)
					continue;

				lstEventDl.push_back(&itor->second);

				if (itor->second.timerProc)
				{
					itor->second.timerProc(itor->second.tgKey.hWnd,itor->second.tgKey.uIDEvent,itor->second.uElapse);
				}
				else OnTimer(itor->second.tgKey.hWnd,itor->second.tgKey.uIDEvent);

				itor->second.uLastTM = tmNow;
				iRet = 0;
			}
		}

		_tgTimer *pstTimer = NULL;
		while (!lstEventWait.empty())
		{
			pstTimer = lstEventDl.front();
			lstEventDl.pop_front();

			if (!pstTimer)
				continue;

			if (pstTimer->timerProc)
				pstTimer->timerProc(pstTimer->tgKey.hWnd,pstTimer->tgKey.uIDEvent,pstTimer->uElapse);
			else OnTimer(pstTimer->tgKey.hWnd,pstTimer->tgKey.uIDEvent);

			pstTimer->uLastTM = tmNow;
			iRet = 0;
		}

		return iRet;
	}

	HWND		getTimerHWnd(){		return (HWND)this;	}

private:
	UINT		SetTimer(HWND hWnd,UINT nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc = NULL)
	{
		NTLockGuard	lock(&m_csLockW);

		_tgTimer stTimer;
		stTimer.uType = 0;
		stTimer.tgKey.hWnd = hWnd;
		stTimer.tgKey.uIDEvent = nIDEvent;
		stTimer.timerProc = lpTimerFunc;
		stTimer.uElapse = uElapse;
		stTimer.uLastTM = 0;
		m_lstEventWait.push_back(stTimer);

		return 0;
	}

	BOOL		KillTimer(HWND hWnd,UINT uIDEvent)
	{
		NTLockGuard lock(&m_csLockW);

		_tgTimer stTimer;
		stTimer.uType = 1;
		stTimer.tgKey.hWnd = hWnd;
		stTimer.tgKey.uIDEvent = uIDEvent;
		stTimer.timerProc = NULL;
		stTimer.uElapse = 0;
		stTimer.uLastTM = 0;
		m_lstEventWait.push_back(stTimer);

		return TRUE;
	}

private:
	CRITICAL_SECTION	m_csLockR;
	CRITICAL_SECTION	m_csLockW;
	typedef std::map< _tgTimer::_tgTmKey , _tgTimer >	MAPTIMER;
	typedef std::list< _tgTimer >		LSTTIMER;
	typedef std::list< _tgTimer *>		LSTTIMERPTR;
	MAPTIMER			m_mapEvent;
	LSTTIMER			m_lstEventWait;
};

class TimerThread	:	public Timer,
	public FCThreadModel
{
public:
	TimerThread(){}
	virtual ~TimerThread(){}
		
	virtual bool	initialModel()	{	return true;	}
	virtual bool	exitModel()		{	return true;	}
	virtual bool	runloop()
	{
		return DealTimer() ? false : true;
	}
};

#endif	/// __HEAD_TIME_THREAD__