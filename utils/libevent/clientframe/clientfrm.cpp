/*
 * @Author: huqing 
 * @Date: 2018-11-22 14:52:39 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-26 15:50:17
 */

#include "../../frame/_netFunc.h"
#include "clientfrm.h"

#include <time.h>
#include <string.h>
#include "../event/include/event2/event.h"
#include "../event/include/event2/buffer.h"
#include "../event/include/event2/bufferevent.h"
#include "../event/include/event2/thread.h"

#include "../../cbuffstt/cbuffstt.h"
#include "../../tools/_tools.h"

void    callback_buffEvent_data_read(struct bufferevent *bev, void *ctx){
    clientfrmModel *pclt = reinterpret_cast<clientfrmModel*>(ctx);
    if (!pclt)
        return;

    pclt->cb_buffEvent_data_read(bev, ctx);
}

void    callback_buffEvent_data_write(struct bufferevent *bev, void *ctx){
    clientfrmModel *pclt = reinterpret_cast<clientfrmModel*>(ctx);
    if (!pclt)
        return;

    pclt->cb_buffEvent_data_write(bev, ctx);
}

void    callback_buffEvent_event(struct bufferevent *bev, short what, void *ctx){
    clientfrmModel *pclt = reinterpret_cast<clientfrmModel*>(ctx);
    if (!pclt)
        return;

    pclt->cb_buffEvent_event(bev, what, ctx);
}

clientfrmModel::clientfrmModel()
    :   m_pEnvBase( NULL )
    ,   m_pBufEnvClt( NULL )
	,	m_bConnected( false )
	,	m_bHasEvent( false )
    ,   m_strAddr( "127.0.0.1" )
    ,   m_iPort( 9967 )
	,	m_iCheckFlag( 0 ) 
	,	m_iIntervalCA_S( 15 )
	,	m_iLastCA_S( (int)time(NULL) )
	,	m_iIntervalCA_R( 60 )
	,	m_iLastCA_R( 0 )
{
#ifdef _MSC_VER
	evthread_use_windows_threads();
#endif

    InitializeCriticalSection(&m_csLock);
    m_pReadBuffer = NewCBuffStt();
    m_pSendBuffer = NULL;
}

clientfrmModel::~clientfrmModel()
{
    if (m_pBufEnvClt){
        bufferevent_free(m_pBufEnvClt);
        m_pBufEnvClt = NULL;
    }

    if (m_pEnvBase){
        event_base_free(m_pEnvBase);
        m_pEnvBase = NULL;
    }

    if (m_pReadBuffer){
        FreeCBuffStt(m_pReadBuffer);
        m_pReadBuffer = NULL;
    }

    if (m_pSendBuffer){
        FreeCBuffStt(m_pSendBuffer);
        m_pSendBuffer = NULL;
    }

    for (std::list< cbuffstt* >::iterator itor = m_lstSendBuf.begin(); itor != m_lstSendBuf.end(); itor++)
    {
        FreeCBuffStt(*itor);
    }
    m_lstSendBuf.clear();
    DeleteCriticalSection(&m_csLock);
}

void    clientfrmModel::setNetCfg(std::string strAddr, int iPort)
{
    m_strAddr = strAddr;
    m_iPort = iPort;
}

void    clientfrmModel::setLogDir(std::string strDir)
{
    if (!strDir.empty())
        m_strLogDir = strDir;
    else{
        m_strLogDir = getExeDir();
    }
}

void	clientfrmModel::setAliveCfg(int iAliveFlag /*= 0*/, int iIntervalSend /*= 15*/, int iIntervalRcv /*= 60*/)
{
	m_iCheckFlag = iAliveFlag;
	m_iIntervalCA_S = iIntervalSend;
	m_iIntervalCA_R = iIntervalRcv;
}

bool	clientfrmModel::initialModel()
{
    /// init libevent
    m_pEnvBase = event_base_new();
    if (!m_pEnvBase)
    {
        Warn("clientfrmModel event_base_new failure, errMsg[%s]", 
            evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        return false;
    }

    return connectSvr();
}

bool	clientfrmModel::exitModel()
{
    return true;
}

bool	clientfrmModel::runloop()
{
    if (!checkConn())
    {
#ifdef _MSC_VER
        Sleep(1000 * 3);
#else
        usleep(1000 * 1000 * 3);
#endif
        return false;
    }

    if (!checkAlive())
    {
		disconnSvr();
        return false;
    }

	/// checksendBuf
	if (checkSendFlag())
	{
		m_bHasEvent = true;
	}

    /// loop
	event_base_loop(m_pEnvBase, EVLOOP_NONBLOCK);

	if (m_bHasEvent)
	{
		m_bHasEvent = false;
		return true;
	}
	bool bRet = m_bHasEvent ? true : false;
	m_bHasEvent = false;
	return bRet;
}

void	clientfrmModel::setExitFlag()
{
	FCThreadModel::setExitFlag();

	if (m_pEnvBase)
		event_base_loopexit(m_pEnvBase, NULL);
}

bool    clientfrmModel::afterConnectSvr()
{
	if (!m_pBufEnvClt)
    {
        Warn("func[%s], check buffer event failure", __FUNCTION__);
		return false;
    }

	return bufferevent_enable(m_pBufEnvClt, EV_READ | EV_PERSIST) ? false : true;
}

void    clientfrmModel::cb_buffEvent_data_read(struct bufferevent *bev, void *ctx)
{
    struct evbuffer *pInput = bufferevent_get_input(bev);
    size_t stLen = evbuffer_get_length(pInput);
    
    if (!CheckCBuffSttSize(m_pReadBuffer, m_pReadBuffer->iCur + stLen) && !ResizeCBuffStt(m_pReadBuffer, m_pReadBuffer->iMax + stLen))
    {
		Fail("cb_buffEvent_data_read check read buffer failure, size[%u]", stLen);
		disconnSvr();
        return;
    }

    size_t stRead = evbuffer_copyout(pInput, m_pReadBuffer->pBuf + m_pReadBuffer->iCur, stLen);
    if (stRead == (size_t)-1)
    {
		Fail("evbuffer_copyout failure, will disconnectSvr, errormsg[%s]", 
			evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		disconnSvr();
        return;
    }

	if (evbuffer_drain(pInput, stRead) < 0)
	{
		Fail("evbuffer_drain failure, will disconnectSvr");
		disconnSvr();
		return;
	}

	m_pReadBuffer->iCur += stRead;
    int iTotal = dealData(m_pReadBuffer->pBuf, m_pReadBuffer->iCur);
    if (iTotal == -1)
    {
        Fail("dealData failure, will disconnectSvr");
		disconnSvr();
        return;
    }

    if (iTotal == m_pReadBuffer->iCur)
    {
        ResetCBuffStt(m_pReadBuffer);
    }
    else if (iTotal < m_pReadBuffer->iCur)
    {
        memmove(m_pReadBuffer->pBuf, m_pReadBuffer->pBuf + iTotal, m_pReadBuffer->iCur - iTotal);
        m_pReadBuffer->iCur -= iTotal;
    }
    else
    {
        Fail("dealData failure, deallen[%d], curLen[%d]", iTotal, m_pReadBuffer->iCur);
		disconnSvr();
        return;
    }

	m_bHasEvent = true;
	m_iLastCA_R = (int)time(NULL);
}

void    clientfrmModel::cb_buffEvent_data_write(struct bufferevent *bev, void *ctx)
{
    if (IsEmptySendBuf())
        return;

    if (!m_pSendBuffer && (!getBuff(m_pSendBuffer) || !m_pSendBuffer))
        return;

    evbuffer *pOut = bufferevent_get_output(bev);    
    if (!pOut)
        return;

    if (evbuffer_add(pOut, m_pSendBuffer->pBuf, m_pSendBuffer->iCur))
        return;

    FreeCBuffStt(m_pSendBuffer);
    m_pSendBuffer = NULL;
}

// #define BEV_EVENT_READING	0x01	/**< error encountered while reading */
// #define BEV_EVENT_WRITING	0x02	/**< error encountered while writing */
// #define BEV_EVENT_EOF		0x10	/**< eof file reached */
// #define BEV_EVENT_ERROR		0x20	/**< unrecoverable error encountered */
// #define BEV_EVENT_TIMEOUT	0x40	/**< user-specified timeout reached */
// #define BEV_EVENT_CONNECTED	0x80	/**< connect operation finished. */
void    clientfrmModel::cb_buffEvent_event(struct bufferevent *bev, short what, void *ctx)
{
// 	if ((what & BEV_EVENT_READING) || (what & BEV_EVENT_WRITING) || (what & BEV_EVENT_EOF) ||
// 		(what & BEV_EVENT_ERROR))
	if (what & BEV_EVENT_ERROR)
	{
		Fail("error encountered, flag[%d], errmsg[%s]", 
			what, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		disconnSvr();
	}
	else if (what & BEV_EVENT_EOF)
	{
		Fail("server disconnect, flag[%d], errmsg[%s]",
			what, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		disconnSvr();
	}
	else if (what & BEV_EVENT_CONNECTED)
	{
		m_bConnected = true;
		m_iLastCA_R = (int)time(NULL);

		clientfrmModel::afterConnectSvr();
		dealEvent();
	}
	else {
		Fail("error encountered, flag[%d], errmsg[%s]",
			what, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		disconnSvr();
	}
}

#define EVENT_BASE_CONN

bool    clientfrmModel::connectSvr()
{
    if (m_pBufEnvClt)
        return true;

#ifdef EVENT_BASE_CONN
	m_pBufEnvClt = bufferevent_socket_new(m_pEnvBase, -1, BEV_OPT_CLOSE_ON_FREE);
#else
	int iSock = ConnectSync(m_strAddr.c_str(), m_iPort);
	if (iSock == -1)
    {
        Fail("ConnectSync failure, addr[%s:%d], error", m_strAddr.c_str(), m_iPort, WSAGetLastError());
		return false;
    }
	evutil_make_socket_nonblocking(iSock);
	m_pBufEnvClt = bufferevent_socket_new(m_pEnvBase, iSock, BEV_OPT_CLOSE_ON_FREE);
#endif

	if (!m_pBufEnvClt)
    {
        Fail("bufferevent_socket_new failure, result bufferevent is nil, errmsg[%s]",
            evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        return false;
    }

#ifdef EVENT_BASE_CONN
    struct sockaddr_in  svr_addr;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(m_iPort);
    svr_addr.sin_addr.s_addr = inet_addr(m_strAddr.c_str());

    int iStat = bufferevent_socket_connect(m_pBufEnvClt, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if (-1 == iStat)
	{
		Fail("bufferevent_socket_connect failure, msg[%s]", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		return false;
	}
#endif
	bufferevent_setcb(m_pBufEnvClt, callback_buffEvent_data_read, callback_buffEvent_data_write, callback_buffEvent_event, this);
	m_iLastCA_S = (int)time(NULL);
	return true;
//    return afterConnectSvr();
}

void    clientfrmModel::disconnSvr(){
    if (m_pBufEnvClt){
        bufferevent_free(m_pBufEnvClt);
        m_pBufEnvClt = NULL;
    }

	clearBuff();

	ResetCBuffStt(m_pReadBuffer);
	ResetCBuffStt(m_pSendBuffer);

	m_bConnected = false;

	m_iLastCA_R = 0;
	m_iLastCA_S = 0;
}

bool    clientfrmModel::checkAlive()
{
	if (!m_iCheckFlag)
		return true;

	if (m_iCheckFlag & 0x01)
	{/// check send
		if (!m_iLastCA_S || m_iLastCA_S + m_iIntervalCA_S > int(time(NULL)))
		{
			if (!checkAliveMsg())
				return false;

			m_iLastCA_S = (int)time(NULL);
		}
	}

	if (m_iCheckFlag & 0x10)
	{/// check recv
		if (!m_iLastCA_R)
			return true;

		if (m_iLastCA_R + m_iIntervalCA_R < (int)time(NULL))
			return false;
	}

	return true;
}

bool	clientfrmModel::checkSendFlag()
{
	if (!m_pBufEnvClt || IsEmptySendBuf())
		return false;

	bufferevent_enable(m_pBufEnvClt, EV_WRITE);	
	return true;
}

bool    clientfrmModel::sendMsg(const char* pstrMsg, int iLen){
    if (!pstrMsg || iLen <= 0)
        return false;

    cbuffstt *pstt = NewCBuffStt(pstrMsg, iLen);
    if (!pstt || !pstt->pBuf)
    {
        Fail("func[%s], NewCBuffStt failure, bufsize[%d]", __FUNCTION__, iLen);
        FreeCBuffStt(pstt);
        return false;
    }

	return setBuff(pstt);
}

bool    clientfrmModel::sendMsg(const std::string &strMsg){
    return sendMsg(strMsg.c_str(), strMsg.length());
}

bool    clientfrmModel::IsEmptySendBuf(){
    FCAutoLock atLock(&m_csLock);
    return m_lstSendBuf.empty();
}

bool    clientfrmModel::setBuff(cbuffstt *pstt)
{
    if (!pstt){
        return false;
    }

    if (!pstt->pBuf)
    {
        FreeCBuffStt(pstt);
        return false;
    }

    FCAutoLock atLock(&m_csLock);
    m_lstSendBuf.push_back(pstt);
    return true;
}

bool    clientfrmModel::getBuff(cbuffstt *&pstt)
{
    if (m_lstSendBuf.empty())
        return false;

    FCAutoLock atLock(&m_csLock);

    if (m_lstSendBuf.empty())
        return false;

    pstt = m_lstSendBuf.front();
    m_lstSendBuf.pop_front();
    return true;
}

void	clientfrmModel::clearBuff()
{
	if (m_lstSendBuf.empty())
		return;

	FCAutoLock atLock(&m_csLock);
	if (m_lstSendBuf.empty())
		return;

	while (m_lstSendBuf.empty())
	{
		cbuffstt *pstt = m_lstSendBuf.front();
		m_lstSendBuf.pop_front();
		FreeCBuffStt(pstt);
	}
}