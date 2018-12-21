#ifndef	__BLOCKING_QUEUE_H__
#define	__BLOCKING_QUEUE_H__

#include "_condition.h"
#include <deque>

template< typename T >
class BlockingQueue
	: public _noncopyable
{
public:
	BlockingQueue()
		: _lock()
		, _notEmpty( _lock )
		, _queue()
	{
	}

	virtual ~BlockingQueue()
	{
		_queue.clear();
	}

public:
	void put( const T& x )
	{
		MutexLockGuard lock( _lock );
		_queue.push_back( x );
		_notEmpty.notify();
	}

	bool	take( T& out, int timeWait = 1 )
	{
		MutexLockGuard lock( _lock );
		while( _queue.empty() )
		{
			if( _notEmpty.waitForSeconds( timeWait ) )
				return false;
		}
		assert( !_queue.empty() );
		out = _queue.front();
		_queue.pop_front();
		return true;
	}

	size_t size() const
	{
		MutexLockGuard lock( _lock );
		return _queue.size();
	}

private:
	mutable MutexLock	_lock;
	Condition			_notEmpty;
	std::deque<T>		_queue;
};

#endif	/*__BLOCKING_QUEUE_H__*/

