/*
 * @Author: huqing 
 * @Date: 2018-11-22 14:51:26 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-26 15:46:13
 */

#ifndef _CLIENT_LIBEVNET_FRAMEWORK_HEADER_
#define _CLIENT_LIBEVNET_FRAMEWORK_HEADER_

#include "../../frame/_threadModel.h"
#include "../../frame/_lock.h"
#include "../../frame/cbfuncLog.h"

#include "../servercfg.h"

#include <string>
#include <list>

struct event_base;
struct bufferevent;
struct cbuffstt;
class clientfrmModel :   
    public FCThreadModel,
    public cbLogFrm
{
private:
    /* data */
public:
    clientfrmModel();
    virtual ~clientfrmModel();

    void    setNetCfg(std::string strAddr, int iPort);
    void    setLogDir(std::string strDir);
	void	setAliveCfg(int iAliveFlag = 0, int iIntervalSend = 15, int iIntervalRcv = 60);

	virtual bool	initialModel();
	virtual bool	exitModel();

    virtual	bool	runloop();
	virtual	void	setExitFlag();

public:
    void    cb_buffEvent_data_read(struct bufferevent *bev, void *ctx);
    void    cb_buffEvent_data_write(struct bufferevent *bev, void *ctx);
    void    cb_buffEvent_event(struct bufferevent *bev, short what, void *ctx);

    /// @return size for dealed, -1 is failure
    virtual int     dealData(char *pBuff, int iLen) = 0;
    virtual void    dealEvent() = 0;

protected:  /// server opt
    bool    connectSvr();
	void	disconnSvr();
    bool    checkConn(){
        return connectSvr();
    }
    bool    checkAlive();
	bool	checkSendFlag();

protected:
    bool    sendMsg(const char* pstrMsg, int iLen);
    bool    sendMsg(const std::string &strMsg);
    bool    IsEmptySendBuf();

private:
    bool    setBuff(cbuffstt *pstt);
    bool    getBuff(cbuffstt *&pstt);
	void	clearBuff();

protected:
    virtual bool    checkAliveMsg() = 0;
    virtual bool    afterConnectSvr();

private:

    struct event_base*  m_pEnvBase;
    struct bufferevent* m_pBufEnvClt;
	bool			m_bConnected;
	bool			m_bHasEvent;

    std::string     m_strLogDir;

protected:
	std::string     m_strAddr;
	int             m_iPort;

	//////////////////////////////////////////////////////////////////////////
	/// alive check
	int				m_iCheckFlag;			/// 0x01-checksend		0x10-checkrecv		other-nocheck

	/// send check 
    int             m_iIntervalCA_S;		/// send check interval
	int             m_iLastCA_S;			/// last send check time flag

	/// recv check
	int				m_iIntervalCA_R;		/// recv check interval
	int				m_iLastCA_R;			/// last recv check time flag

private:    /// message
    std::list< cbuffstt* >  m_lstSendBuf;
    CRITICAL_SECTION    m_csLock;

    /// readbuffer
    cbuffstt*       m_pReadBuffer;
    cbuffstt*       m_pSendBuffer;
};

#endif  /// _CLIENT_LIBEVNET_FRAMEWORK_HEADER_