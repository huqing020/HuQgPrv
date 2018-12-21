#ifndef	__SINGLETON_H__
#define	__SINGLETON_H__

///	进程单一实例对象
template< class T > class _singleton
{
public:
	///	获取单一实例对象
	static	T*	GetInstance( void )
	{
		static	T _instance;
		return &_instance;
	}
};

#endif	/*__SINGLETON_H__*/

