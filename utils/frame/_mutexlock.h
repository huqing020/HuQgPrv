#ifndef	__MUTEX_LOCK_H__
#define	__MUTEX_LOCK_H__

#include "../thread/_thread.h"
#include "_noncopyable.h"

class MutexLock
	: _noncopyable
{
public:
	MutexLock()
		: _holder( 0 )
	{
		pthread_mutex_init( &_mutex, NULL );
	}

	virtual ~MutexLock()
	{
		assert( _holder == 0 );
		pthread_mutex_destroy( &_mutex );
	}

	bool isLockedByThisThread() const
	{
		return _holder == CurrentThread::tid();
	}

	void assertLocked() const
	{
		assert( isLockedByThisThread() );
	}

	void lock()
	{
		pthread_mutex_lock( &_mutex );
		assignHolder();
	}

	void unlock()
	{
		unassignHolder();
		pthread_mutex_unlock( &_mutex );
	}

	pthread_mutex_t* getPthreadMutex()
	{
		return &_mutex;
	}

private:
	void unassignHolder()
	{
		_holder = 0;
	}

	void assignHolder()
	{
		_holder = CurrentThread::tid();
	}

private:
	friend class Condition;
	class UnassignGuard
		: public _noncopyable
	{
	public:
		UnassignGuard( MutexLock& lock )
			: _lock( lock )
		{
			_lock.unassignHolder();
		}

		virtual ~UnassignGuard()
		{
			_lock.assignHolder();
		}
	private:
		MutexLock&	_lock;
	};

private:
	pthread_mutex_t	_mutex;
	pid_t			_holder;
};

class MutexLockGuard
	: public _noncopyable
{
public:
	explicit MutexLockGuard( MutexLock& lock )
		: _lock( lock )
	{
		_lock.lock();
	}

	virtual ~MutexLockGuard()
	{
		_lock.unlock();
	}

private:
	MutexLock& _lock;
};

#endif	/*__MUTEX_LOCK_H__*/

