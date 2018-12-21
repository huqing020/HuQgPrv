#ifndef	__NO_COPYABLE_H__
#define	__NO_COPYABLE_H__

class _noncopyable
{
protected:
	_noncopyable(){}
	~_noncopyable(){}

private:
	_noncopyable( const _noncopyable& );
	const _noncopyable& operator=( const _noncopyable& );
};

#endif	/*__NO_COPYABLE_H__*/

