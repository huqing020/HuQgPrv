#ifndef _FC_CUSTOM_LOCK_H
#define _FC_CUSTOM_LOCK_H

#include "datatype.h"

#ifdef _MSC_VER

#define DeleteSRWLock( rwlock )	do	\
		{	\
		} while (0)

#else
#include <pthread.h>

#ifndef SRWLOCK
typedef	pthread_rwlock_t	SRWLOCK;
#endif

#ifndef PSRWLOCK
typedef	pthread_rwlock_t*	PSRWLOCK;
#endif

#ifndef CRITICAL_SECTION
typedef pthread_mutex_t		CRITICAL_SECTION
#endif

#ifndef LPCRITICAL_SECTION
typedef pthread_mutex_t*	LPCRITICAL_SECTION;
#endif

#define	InitializeSRWLock( rwlock )		do	\
		{	\
			pthread_rwlock_init(rwlock,NULL);	\
		} while (0)

#define DeleteSRWLock( rwlock )			do	\
		{	\
			pthread_rwlock_destroy(rwlock);		\
		} while (0)


inline BOOL	TryEnterCriticalSection( LPCRITICAL_SECTION lpCriticalSection )
{
	return pthread_mutex_trylock(lpCriticalSection) ? FALSE : TRUE;
}

/// 读写锁  写锁定
inline void	AcquireSRWLockExclusive( PSRWLOCK SRWLock )
{
	pthread_rwlock_wrlock(SRWLock);
}

inline void	ReleaseSRWLockExclusive( PSRWLOCK SRWLock )
{
	pthread_rwlock_unlock(SRWLock);
}

inline BOOL	TryAcquireSRWLockShared( PSRWLOCK SRWLock )
{
	return pthread_rwlock_tryrdlock(SRWLock) ? FALSE : TRUE;
}

/// 读写锁  读锁定
inline void	AcquireSRWLockShared( PSRWLOCK SRWLock )
{
	pthread_rwlock_rdlock(SRWLock);
}

inline void	ReleaseSRWLockShared( PSRWLOCK SRWLock )
{
	pthread_rwlock_unlock(SRWLock);
}

inline BOOL	TryAcquireSRWLockExclusive( PSRWLOCK SRWLock )
{
	return pthread_rwlock_trywrlock(SRWLock) ? FALSE : TRUE;
}

#endif

//////////////////////////////////////////////////////////////////////////
/// 自动锁定，未获得锁则阻塞知道获得锁
class FCAutoLock
{
public:
	FCAutoLock(LPCRITICAL_SECTION lpcs,bool bLock = true)
		:	m_pcs( lpcs )
		,	m_bLock( bLock )
	{
		if (m_pcs && m_bLock)
			EnterCriticalSection(m_pcs);
	}

	~FCAutoLock()
	{
		if (m_pcs && m_bLock)
			LeaveCriticalSection(m_pcs);
		m_pcs = NULL;
		m_bLock = false;
	}
protected:
private:
	LPCRITICAL_SECTION		m_pcs;
	bool					m_bLock;
};


//////////////////////////////////////////////////////////////////////////
/// 人工判断并锁定
class FCManualLock
{
public:
	FCManualLock(LPCRITICAL_SECTION lpcs)	:	m_bLock( false )
	{
		m_pcs = lpcs;
	}

	~FCManualLock()
	{
		releaseLock();
	}

	bool	tryLock()
	{
		if (!m_bLock && m_pcs && TryEnterCriticalSection(m_pcs))
		{
			m_bLock = true;
			return true;
		}
		else
		{
			m_bLock = false;
			m_pcs = NULL;
			return false;
		}
	}

	void	releaseLock()
	{
		if (m_pcs && m_bLock)
			LeaveCriticalSection(m_pcs);

		m_bLock = false;
		m_pcs = NULL;
	}

protected:
private:
	LPCRITICAL_SECTION	m_pcs;
	bool				m_bLock;
};


//////////////////////////////////////////////////////////////////////////
/// 读写锁
/// 自动读锁
class FCAutoSRWLockR
{
public:
	FCAutoSRWLockR(PSRWLOCK	csLockRW)
	{
		m_csLockRW = csLockRW;
		if (m_csLockRW)
			AcquireSRWLockShared(m_csLockRW);
	}
	~FCAutoSRWLockR()
	{
		if (m_csLockRW)
			ReleaseSRWLockShared(m_csLockRW);
		m_csLockRW = NULL;
	}
protected:
private:
	PSRWLOCK	m_csLockRW;
};

/// 人工读锁
class FCManualSRWLockR
{
public:
	FCManualSRWLockR(PSRWLOCK	csLockRW)	
	{
		m_csLockRW = csLockRW;
	}
	~FCManualSRWLockR()
	{
		release();
	}

	bool	trylock()
	{
		if (m_csLockRW && TryAcquireSRWLockShared(m_csLockRW))
		{
			return true;
		}
		else
		{
			m_csLockRW = NULL;
			return false;
		}
	}

	void	release()
	{
		if (m_csLockRW)
			ReleaseSRWLockShared(m_csLockRW);

		m_csLockRW = NULL;
	}

protected:
private:
	PSRWLOCK	m_csLockRW;
};

/// 自动写锁
class FCAutoSRWLockW
{
public:
	FCAutoSRWLockW(PSRWLOCK	csLockRW)
	{
		m_csLockRW = csLockRW;
		if (m_csLockRW)
			AcquireSRWLockExclusive(m_csLockRW);
	}
	~FCAutoSRWLockW()
	{
		if (m_csLockRW)
			ReleaseSRWLockExclusive(m_csLockRW);
		m_csLockRW = NULL;
	}
protected:
private:
	PSRWLOCK	m_csLockRW;
};

/// 人工写锁
class FCManualSRWLockW
{
public:
	FCManualSRWLockW(PSRWLOCK	csLockRW)	:	m_bLock( false )
	{
		m_pcsLockRW = csLockRW;
	}
	~FCManualSRWLockW()
	{
		release();
	}

	bool	trylock()
	{
		if (!m_bLock && m_pcsLockRW && TryAcquireSRWLockExclusive(m_pcsLockRW))
		{
			m_bLock = true;
			return true;
		}
		else 
		{
			m_bLock = false;
			m_pcsLockRW = NULL;
			return false;
		}
	}

	void	release()
	{
		if (m_bLock && m_pcsLockRW)
			ReleaseSRWLockExclusive(m_pcsLockRW);

		m_bLock = false;
		m_pcsLockRW = NULL;
	}
protected:
private:
	PSRWLOCK	m_pcsLockRW;
	bool		m_bLock;
};

#endif	/// _FC_CUSTOM_LOCK_H