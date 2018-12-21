#ifndef	__CIRCULAR_QUEUE_H__
#define	__CIRCULAR_QUEUE_H__

#include "_condition.h"
#include <deque>

template< typename T >
class CircularQueue
	: public _noncopyable
{
public:
	CircularQueue( unsigned int size = 100 )
		: _size( size )
		, _lock()
		, _notEmpty( _lock )
		, _notFull( _lock )
		, _queue()
	{
	}

	virtual ~CircularQueue()
	{
		_queue.clear();
	}

public:
	bool	put( const T& x, int waitTime = 1 )
	{
		MutexLockGuard lock( _lock );
		if( _queue.size() >= _size )
		{
			if( _notFull.waitForSeconds( waitTime ) )
			{
				while( _queue.size() >= _size )
					_queue.pop_front();
			}
		}
		_queue.push_back( x );
		_notEmpty.notify();
		return true;
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
		_notFull.notify();
		return true;
	}

	size_t size() const
	{
		MutexLockGuard lock( _lock );
		return _queue.size();
	}

private:
	unsigned int		_size;
	mutable MutexLock	_lock;
	Condition			_notEmpty;
	Condition			_notFull;
	std::deque<T>		_queue;
};

#endif	/*__CIRCULAR_QUEUE_H__*/

