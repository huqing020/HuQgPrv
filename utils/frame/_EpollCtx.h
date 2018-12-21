#ifndef	__EPOLL_CTX_H__
#define	__EPOLL_CTX_H__

#include "../common/common_types.h"
#include "../util.h"
#include <sys/epoll.h>
#include "ThreadMutex.h"

class _EpollCtx;

///	套接字
class _SocketCtx
{
	friend class _EpollCtx;
///	构造与析构
public:
	_SocketCtx( int fd );
	virtual ~_SocketCtx();

public:
	int	_socket;			///	socket套接字

public:
	///	开始监听
	BOOL	Listen( int port, int backlog = 128 );
	///	连接
	BOOL	Connect( const char* host, int port, int timeout = 5 );
	///	关闭
	virtual	void	Close();

public:
	///	保持连接
	BOOL	KeepAlive( int keepAlived = 1, int keepIdle = 600, int keepintvl = 10, int keepCount = 10 );

protected:
	///	处理EPOLLIN事件
	virtual	int ProcessEpollIn( void );
	///	处理EPOLLOUT事件
	virtual	int ProcessEpollOut( void );
	///	是否有数据待发送
	virtual	bool HasData2Send( void );
};

///	Epoll文件
class _EpollCtx
{
public:
	_EpollCtx();
	virtual ~_EpollCtx();

private:
	ThreadMutex				_lock;		///	线程安全锁
	int						_epfd;		///	epoll文件描述
	int						_epSize;	///	最大监听数
	struct	epoll_event		_epEvent;	///	交换epoll事件
	struct	epoll_event*	_epEvents;	///	全部epoll事件

public:
	///	初始化epoll文件
	BOOL	InitEpoll( int epsize );
	///	添加监听事件
	BOOL	AddSocket( _SocketCtx*	ctx, unsigned int events );
	///	修改监听事件
	BOOL	ModifySocket( _SocketCtx* ctx, unsigned int events );
	///	删除监听事件
	BOOL	RemoveSocket( _SocketCtx* ctx );
	///	关闭epoll文件
	void	CloseEpoll( void );
	///	执行监控
	void	DoDetect( void );

protected:
	///	SOCKET移除事件
	virtual	void	OnSocketRemoved( _SocketCtx* ctx )
	{
		if( ctx )
			ctx->Close();
	}
};

#endif	/*__EPOLL_CTX_H__*/

