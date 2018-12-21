#include "_ServiceFrame.h"
#include <stdio.h>
#include <signal.h>
#include "string/string_util.h"
//#include "../log/log.h"
#include "log/_logMgr.h"

_ServiceFrame*	_ServiceFrame::gs_Service = NULL;

///	服务退出事件响应
void	_ServiceFrame::prog_exit( int signo )
{
	char sMsg[1024] = {0};
	sprintf(sMsg,"%s :%s ,signal[%d]",getTimeNowEx(1).c_str(),__FFL_INFO__,signo);
	std::cout << sMsg << std::endl;

	//	设置忽略信号量处理
	signal( SIGINT, SIG_IGN );
	signal( SIGKILL, SIG_IGN );
	signal( SIGTERM, SIG_IGN );
	signal( SIGBUS, SIG_IGN );

	//	保存配置
	gs_Service->SaveCfgFile();

	//	退出清理
	gs_Service->tryExit( signo );
}

///	构造与析构
_ServiceFrame::_ServiceFrame()
	: _configfile( "config.json" )
	, _exefilename( "" )
	, _exefilelocation( "" )
{
	if( NULL != gs_Service )
		throw(0);
	gs_Service = this;
}

_ServiceFrame::~_ServiceFrame()
{
}

///	运行服务
int	_ServiceFrame::RunService( int argc, char** argv )
{
	char	configfile[128] = {0};
	int	ch;
	bool	isdaemon = true;
	
	char	exec_name [BUFSIZ] = {0};
	readlink ("/proc/self/exe", exec_name, BUFSIZ);
	char* lastslot = NULL;
	lastslot = strrchr( exec_name, '/' );
	if( lastslot == NULL )
	{
		_exefilename = argv[0];
		_exefilelocation = "";
	}
	else
	{
		_exefilename = (const char*)&lastslot[1];
		lastslot[1] = 0;
		_exefilelocation = exec_name;
	}

	//	检测参数表
	while( ( ch = getopt( argc, argv, "f:d" ) ) != -1 )
	{
		switch( ch )
		{
			case 'f':
			{//	-fFILENAME用于指定配置文件名称
				strncpy( configfile, optarg, sizeof( configfile ) );
				break;				
			}
			case 'd':
			{//	-d用于指定调试模式
				isdaemon = false;
				break;
			}			
		}
	}
	
	if( strlen( configfile ) > 0 )
		_configfile = configfile;

	//	根据参数设置后台运行
	if( isdaemon )
	{
		int pid = 0;
		
		//	主进程设置忽略子进程信号(否则主进程必须等待子进程结束才能退出)
		signal( SIGCHLD, SIG_IGN );

		pid = fork();

		if( pid < 0 )
		{//	fork失败
			perror( "fork" );
			exit( -1 );
		}
		else if( pid > 0 )
			exit( 0 );	//	主进程，退出

		//	子进程
		setsid();
	}

	//	恢复对子进程信号的默认响应
	signal( SIGCHLD, SIG_DFL );
	//	忽略SIGPIPE信号( SIGPIPE的默认处理方法是退出进程 )
	signal( SIGPIPE, SIG_IGN );
	//	将SIGINT(中断进程信号,通常由Ctrl-C导致)
	//	SIGKILL(杀死进程信号,此信号不能被阻塞,处理和忽略)
	//	SIGTERM(终止进程信号,通常由kill导致)
	//	交由prog_exit函数处理,从而使得进程退出时能正常进行清理动作
	signal( SIGINT, prog_exit );
	signal( SIGKILL, prog_exit );
	signal( SIGTERM, prog_exit );

	//	载入配置
	if( !LoadCfgFile() )
		return -1;

	if( isdaemon )
	{//	后台运行服务运行
		//      初始化
		int nRet = Init();
		if( nRet )
		{
			tryExit(nRet);
			goto DealEnd;
		}
		Run();
	}
	else
	{//	调试
		Debug();
	}

DealEnd:
	Exit(0);

	return 0;
}

//	调试服务
void	_ServiceFrame::Debug( void )
{
	TRACE( "GLB","<%s> is running in DEBUG mode.", _exefilename.c_str() );
	char	cmd[ 1024 ] = {0};
	while( 1 )
	{
		fputs( ">", stdout );
		fgets( cmd, 1024, stdin );
		char*	cmdLine = TrimRude( cmd );

		if( strcasecmp( cmdLine, "quit" ) == 0 )
			break;
		else if( strcasecmp( cmdLine, "start" ) == 0 ){
			pthread_t	tid;
			pthread_attr_t	thread_attr;

			pthread_attr_init(&thread_attr);	
			/* 设置堆栈大小*/        
			if( pthread_attr_setstacksize( &thread_attr, 1024*1024 ) == 0 )
			{
				if( pthread_create( &tid, &thread_attr, _ServiceFrame::_DebugThreadProc, this ) ){
					FAIL("GLB","创建调试线程失败");
				}
				else{
					pthread_detach( tid );
				}
			}
			else{
				FAIL("GLB","pthread_attr_setstacksize failed.");
			}
		}
		else if( strcasecmp( cmdLine, "stop" ) == 0 )
			tryExit( 0 );
		else
			ProcessDebugCmd( cmdLine );
	}

	prog_exit(0);
}

///	调试服务线程
void* _ServiceFrame::_DebugThreadProc( void* param )
{
	_ServiceFrame*	pService = ( _ServiceFrame* )param;
	if( pService )
	{
		//      初始化
                int nRet = pService->Init();
                if( nRet )
                        return 0;

		pService->Run();
		pService->Exit(0);
	}

	return 0;
}

