#include "_EpollCtx.h"
#include "../log/log.h"
#include <sys/poll.h>
#include <netinet/tcp.h>

///	_SocketCtx  构造与析构
_SocketCtx::_SocketCtx( int fd )
	: _socket(fd)
{
}

_SocketCtx::~_SocketCtx()
{
	_SocketCtx::Close();
}

///     开始监听
BOOL    _SocketCtx::Listen( int port, int backlog/* = 128*/ )
{
        _SocketCtx::Close();

        _socket = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
        if( _socket < 0 ){
                ERROR("创建SOCKET失败");
                return FALSE;
        }

        int on = 1;
        if( setsockopt( _socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ) ) ){
                ERROR("REUSEADDR 失败");
                return FALSE;
        }

        struct sockaddr_in local;
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        local.sin_family = AF_INET;
        local.sin_port = htons( port );

	///	绑定端口并开启侦听
        if( bind( _socket, (struct sockaddr *)&local, sizeof(local) )!=0 || listen( _socket, backlog ) != 0 ){
                close( _socket );
                _socket = -1;
                ERROR( "绑定端口或侦听失败!" );
                return FALSE;
        }

	///	设置为非阻塞
	int	value = fcntl( _socket, F_GETFL, 0 );
	value |= O_NONBLOCK;
	fcntl( _socket, F_SETFL, value );

        return TRUE;
}

///     连接
BOOL    _SocketCtx::Connect( const char* host, int port, int timeout/* = 5*/ )
{
	_SocketCtx::Close();

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr( host );
	if (server.sin_addr.s_addr == INADDR_NONE )
	{
		struct hostent *inhost = gethostbyname( host );
		if ( inhost )
		{
			int i = 0;
			for(; inhost->h_addr_list[i]; i++)
			{
				_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
				if( _socket < 0 ){
					ERROR("创建SOCKET失败");
					return FALSE;
				}
				///     设置为非阻塞
				int     value = fcntl( _socket, F_GETFL, 0 );
				value |= O_NONBLOCK;
				fcntl( _socket, F_SETFL, value );

				memcpy( &server.sin_addr, inhost->h_addr_list[i], inhost->h_length );
				int ret = connect( _socket, (struct sockaddr *)&server, sizeof( server ) );
				if( ret >= 0 )
					break;
				
				if( errno == EINPROGRESS )
				{
					struct pollfd   fds;
					fds.fd = _socket;
					fds.events = POLLWRNORM;
					if( poll( &fds, 1, timeout*1000 ) == 1 && ( fds.revents & POLLWRNORM )== POLLWRNORM )
						break;
				}

				close( _socket );
				_socket = -1;
			}
			if( NULL == inhost->h_addr_list[i] )
			{
				ERROR( "连接失败" );
				return FALSE;
			}
		}
		else
		{
			ERROR("域名解析失败");
			return FALSE;
		}
	}
	else
	{
		_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if( _socket < 0 ){
			ERROR("创建SOCKET失败");
			return FALSE;
		}
		///     设置为非阻塞
		int     value = fcntl( _socket, F_GETFL, 0 );
		value |= O_NONBLOCK;
		fcntl( _socket, F_SETFL, value );

		int ret = connect( _socket, (struct sockaddr *)&server, sizeof( server ) );
		if( ret < 0 )
		{
			if( errno == EINPROGRESS )
			{///	正在建立连接
				struct pollfd   fds;
				fds.fd = _socket;
				fds.events = POLLWRNORM;
				if( poll( &fds, 1, timeout*1000 ) != 1 || ( fds.revents & POLLWRNORM ) != POLLWRNORM )
				{
					ERROR( "连接超时" );
					close( _socket );
					_socket = -1;
					return FALSE;
				}
			}
			else
			{
				ERROR( "连接失败" );
				close( _socket );
				_socket = -1;
				return FALSE;
			}
		}
	}

	return TRUE;
}

///     关闭
void    _SocketCtx::Close()
{
	if( _socket >= 0 )
	{
		close( _socket );
		_socket = -1;
	}
}

///	保持连接
BOOL	_SocketCtx::KeepAlive( int keepAlived/* = 1*/
							 , int keepIdle/* = 600*/
							 , int keepintvl/* = 10*/
							 , int keepCount/* = 10*/ )
{
	if( _socket < 0 )
		return FALSE;

	int nodelay = 1;

	if( setsockopt( _socket, IPPROTO_TCP, TCP_NODELAY, (void *)&nodelay, sizeof( nodelay ) ) )
		return FALSE;

	if( setsockopt( _socket, SOL_SOCKET, SO_KEEPALIVE, &keepAlived, sizeof( keepAlived ) ) )
		return FALSE;

	if( 0 == keepAlived )
		return TRUE;

	if( setsockopt( _socket, SOL_TCP, TCP_KEEPIDLE, &keepIdle, sizeof( keepIdle ) ) )
		return FALSE;

	if( setsockopt( _socket, SOL_TCP, TCP_KEEPINTVL, &keepintvl, sizeof( keepintvl ) ) )
		return FALSE;

	if( setsockopt( _socket, SOL_TCP, TCP_KEEPCNT, &keepCount, sizeof( keepCount ) ) )
		return FALSE;

	return TRUE;
}

///     处理EPOLLIN事件
int	_SocketCtx::ProcessEpollIn( void )
{
	return -1;
}

///     处理EPOLLOUT事件
int	_SocketCtx::ProcessEpollOut( void )
{
	return -1;
}

///	是否有数据待发送
bool	_SocketCtx::HasData2Send( void )
{
	return false;
}

///	_EpollCtx  构造与析构
_EpollCtx::_EpollCtx()
	: _epfd( -1 )
	, _epSize( 100 )
	, _epEvents( NULL )
{
}

_EpollCtx::~_EpollCtx()
{
	_EpollCtx::CloseEpoll();
}

///     初始化epoll文件
BOOL    _EpollCtx::InitEpoll( int epsize )
{
	_EpollCtx::CloseEpoll();

	if( epsize > 0 )
		_epSize = epsize;

	_epEvents = new struct epoll_event[ _epSize ];

	_epfd = epoll_create( _epSize );
        if( _epfd < 0 ){
                FATAL("创建epoll文件描述失败");
                return FALSE;
        }

	return TRUE;
}

///     添加监听事件
BOOL    _EpollCtx::AddSocket( _SocketCtx*  ctx, unsigned int events )
{
	if( _epfd < 0 )
		return FALSE;

	if( NULL == ctx || ctx->_socket < 0 )
		return FALSE;

	if ( !_lock.lock() )
	{
		DEBUG("lock failed");
		return FALSE;
	}

	_epEvent.data.ptr = ctx;
	_epEvent.events = events;
	if( epoll_ctl( _epfd, EPOLL_CTL_ADD, ctx->_socket, &_epEvent ) < 0 ){
		ERROR("添加监听事件失败");
		_lock.unlock();
		return FALSE;
        }

	_lock.unlock();

	return TRUE;
}

///     修改监听事件
BOOL    _EpollCtx::ModifySocket( _SocketCtx* ctx, unsigned int events )
{
	if( _epfd < 0 )
		return FALSE;

	if( NULL == ctx || ctx->_socket < 0 )
		return FALSE;

	if( !_lock.lock() )
	{
		DEBUG("lock failed");
		return FALSE;
	}

	_epEvent.data.ptr = ctx;
	_epEvent.events = events;
	if( epoll_ctl( _epfd, EPOLL_CTL_MOD, ctx->_socket, &_epEvent ) < 0 ){
		ERROR("修改监听事件失败");
		_lock.unlock();
		return FALSE;
	}

	_lock.unlock();
	return TRUE;
}

///	删除监听事件
BOOL	_EpollCtx::RemoveSocket( _SocketCtx* ctx )
{
	if( _epfd < 0 )
		return FALSE;

	if( NULL == ctx || ctx->_socket < 0 )
		return FALSE;

	if( !_lock.lock() )
	{
		DEBUG("lock failed");
		return FALSE;
	}

	if( epoll_ctl( _epfd, EPOLL_CTL_DEL, ctx->_socket, &_epEvent ) < 0 ){
		ERROR("删除监听事件失败");
		_lock.unlock();
		return FALSE;
	}

	_lock.unlock();
	return TRUE;
}

///     关闭epoll文件
void    _EpollCtx::CloseEpoll( void )
{
	if( _epfd >= 0 )
	{
		close( _epfd );
		_epfd = -1;
	}

	if( NULL != _epEvents )
	{
		delete[] _epEvents;
		_epEvents = NULL;
	}
}

///     执行监控
void     _EpollCtx::DoDetect( void )
{
	if( _epfd < 0 )
		return;

	if( !_lock.lock() )
		return;

	int nEventNum = epoll_wait( _epfd, _epEvents, _epSize, 0 );
	_lock.unlock();

	if( nEventNum <= 0 )
		usleep( 1000 );

	for( int i = 0; i < nEventNum; i++ )
	{
		_SocketCtx*	pCtx = ( _SocketCtx* )_epEvents[i].data.ptr;
		if( NULL != pCtx )
		{
			bool bNeedRemove = false;
			if( _epEvents[i].events & ( EPOLLIN|EPOLLERR ) )
			{
				if( pCtx->ProcessEpollIn() == 0 )
					bNeedRemove = true;
			}
			if( _epEvents[i].events & ( EPOLLOUT|EPOLLERR ) )
			{
				if( pCtx->ProcessEpollOut() == 0 )
					bNeedRemove = true;
			}

			if( bNeedRemove && RemoveSocket( pCtx ) )
			{
				OnSocketRemoved( pCtx );
			}
			else if( pCtx->HasData2Send() )
			{
				ModifySocket( pCtx, ( EPOLLOUT|EPOLLERR ) );
			}
			else
			{
				ModifySocket( pCtx, ( EPOLLIN|EPOLLERR ) );
			}
		}
	}
}

