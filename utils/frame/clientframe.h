#ifndef _CLIENT_FRAME_HEAD
#define _CLIENT_FRAME_HEAD

#include "_threadModel.h"
#include "datatype.h"
#include "cbfuncLog.h"

#include <string>
#include <list>

struct cbuffstt;
class clientFrame	:	
	public	FCThreadModel,
	public	cbLogFrm
{
public:
	clientFrame();
	virtual ~clientFrame();

	virtual bool	initialModel();
	virtual bool	exitModel();

	void	setNetCfg(const std::string &strIP, int iPort);
	void	setConnectTimeOut(int iSec, int iuSec);
	void	setSelectTimeOut(int iSec, int iuSec);
	void	statCheckAlive(bool bEnable = false);

protected:
	virtual	bool	runloop();

	/// @return size for dealed, -1 is failure
	virtual int     dealData(char *pBuff, int iLen) = 0;

protected:  /// server opt
	bool    connectSvr();
	void	disconnSvr();
	bool    checkConn();
	bool    checkAlive();
	bool	checkSendFlag();

	bool	dealNetWorkIO();
	bool	dealEnv_Recv();
	bool	dealEnv_Send();

protected:
	bool    sendMsg(const char* pstrMsg, int iLen);
	bool    sendMsg(const std::string &strMsg);
	bool    IsEmptySendBuf();

private:
	bool    setBuff(cbuffstt *pstt);
	bool    getBuff(cbuffstt *&pstt);
	void	clearBuff();

protected:
	virtual bool    checkAliveMsg();
	virtual bool    afterConnectSvr();

private:
	SOCKET			m_sock;
	std::string		m_strAddr;
	int				m_iPort;

	struct timeval	m_tvTimeOutConn;
	struct timeval	m_tvTimeOutSlt;

	bool			m_bCheckAlive;
	int				m_iLastCATm;		/// checkAlive
	int				m_iIntervalCA;

private:
	std::list< cbuffstt* >  m_lstSendBuf;
	CRITICAL_SECTION    m_csLock;

	/// readbuffer
	cbuffstt*       m_pReadBuffer;
	cbuffstt*       m_pSendBuffer;
};

#endif	/// _CLIENT_FRAME_HEAD