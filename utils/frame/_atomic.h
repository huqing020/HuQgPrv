#ifndef	__ATOMIC_H__
#define	__ATOMIC_H__

#include "_noncopyable.h"
#include <stdint.h>

template<typename T>
class AtomicIntegerT
	: public _noncopyable
{
///	构造与析构
public:
	AtomicIntegerT()
		: _value(0)
	{
	}

	T	get()
	{
		return __sync_val_compare_and_swap( &_value, 0, 0 );
	}

	T	getAndAdd( T x )
	{
		return __sync_fetch_and_add( &_value, x );
	}

	T	addAndGet( T x )
	{
		return __sync_add_and_fetch( &_value, x );
	}

	T	incrementAndGet()
	{
		return __sync_add_and_fetch( &_value, 1 );
	}

	T	decrementAndGet()
	{
		return __sync_add_and_fetch( &_value, -1 );
	}

	void	add( T x )
	{
		__sync_fetch_and_add( &_value, x );
	}

	void	increment()
	{
		 __sync_add_and_fetch( &_value, 1 );
	}

	void	decrement()
	{
		__sync_add_and_fetch( &_value, -1 );
	}

	T	getAndSet( T newValue )
	{
		return __sync_lock_test_and_set( &_value, newValue );
	}

private:
	T	_value;
};

typedef	AtomicIntegerT< int32_t >	AtomicInt32;
typedef AtomicIntegerT< int64_t >	AtomicInt64;

#endif	/*__ATOMIC_H__*/

