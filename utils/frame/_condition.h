#ifndef	__CONDITION_H__
#define	__CONDITION_H__

#include "_mutexlock.h"
#include <errno.h>

class Condition
	: public _noncopyable
{
public:
	explicit Condition( MutexLock& lock )
		: _lock( lock )
	{
		pthread_cond_init( &_pcond, NULL );
	}

	virtual ~Condition()
	{
		pthread_cond_destroy( &_pcond );
	}

public:
	void wait()
	{
		MutexLock::UnassignGuard ug( _lock );
		pthread_cond_wait( &_pcond, _lock.getPthreadMutex() );
	}

	bool waitForSeconds( int seconds )
	{
		struct timespec abstime;
		clock_gettime( CLOCK_REALTIME, &abstime );
		abstime.tv_sec += seconds;
		MutexLock::UnassignGuard ug( _lock );
		return ( ETIMEDOUT == pthread_cond_timedwait( &_pcond, _lock.getPthreadMutex(), &abstime ) );
	}

	void notify()
	{
		pthread_cond_signal( &_pcond );
	}

	void notifyAll()
	{
		pthread_cond_broadcast( &_pcond );
	}

private:
	MutexLock&		_lock;
	pthread_cond_t	_pcond;
};

#endif	/*__CONDITION_H__*/

