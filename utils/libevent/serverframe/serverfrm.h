/*
 * @Author: huqing 
 * @Date: 2018-12-10 10:28:06 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-12-10 10:52:29
 */

#ifndef _LIBEVENT_SERVER_FRAME_HEAD
#define _LIBEVENT_SERVER_FRAME_HEAD

#include "../../frame/_threadModel.h"
#include "../../frame/cbfuncLog.h"

#include <string>

struct event_base;
struct bufferevent;
class serverfrm :
    public FCThreadModel,
    public cbLogFrm
{
public:
    serverfrm();
    virtual ~serverfrm();

	virtual bool	initialModel();
	virtual bool	exitModel();

protected:
	virtual	bool	runloop();


private:
	struct	event_base	*m_pEvBase;
	struct	bufferevent	*m_pBufferEnv;

private:
	std::string			m_strAddr;
	int					m_iPort;
};


//////////////////////////////////////////////////////////////////////////
/// worker
class Worker
{
public:
	Worker();
	~Worker();
protected:
private:
};

#endif  /// _LIBEVENT_SERVER_FRAME_HEAD