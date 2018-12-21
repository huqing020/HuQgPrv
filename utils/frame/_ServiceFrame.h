#ifndef __SERVICEFRAME_H__
#define __SERVICEFRAME_H__

#include <string>
#include "common/common_types.h"

/*
 * 服务框架类：
 * 	用于定义一个服务运行框架
 */
class _ServiceFrame
{
//	构造和析构
protected:
	_ServiceFrame();
public:
	virtual ~_ServiceFrame();

//	配置文件
protected:
	std::string	_configfile;		//	配置文件名
	std::string	_exefilename;		//	执行文件名
	std::string	_exefilelocation;	//	执行文件所在路径

protected:
	//	调试线程
	static	void* _DebugThreadProc( void* param );
	//	退出信号响应函数
	static void		prog_exit( int signo );
	//	全局静态指针
	static _ServiceFrame*	gs_Service;

protected:
	//	加载配置
	virtual	BOOL		LoadCfgFile() = 0;
	//	保存配置
	virtual	void		SaveCfgFile() = 0;

public:
	//	运行服务
	virtual	int		RunService( int argc, char** argv );

protected:
	//	调试服务
	void			Debug( void );
	//	执行调试命令
	virtual	int		ProcessDebugCmd( char* cmd ) = 0;

protected:
	//	初始化服务
	virtual	int		Init( void ) = 0;
	//	服务主流程
	virtual	int		Run( void ) = 0;
	///	尝试退出
	virtual	void	tryExit( int ) = 0;
	//	退出服务时清理程序
	virtual	void	Exit( int ) = 0;
};

#endif /*__SERVICEFRAME_H__*/

