#include "_netFunc.h"
#include "clientframe.h"
#include "../cbuffstt/cbuffstt.h"

#include <time.h>

#define UTL_RECV_SIZE 1024

clientFrame::clientFrame()
	:	m_sock( INVALID_SOCKET )
	,	m_strAddr("")
	,	m_iPort(9967)
	,	m_bCheckAlive(false)
	,	m_iLastCATm((int)time(NULL))
	,	m_iIntervalCA(10)
{
	InitializeCriticalSection(&m_csLock);

	m_tvTimeOutConn.tv_sec = 5;
	m_tvTimeOutConn.tv_usec = 0;

	m_tvTimeOutSlt.tv_sec = 0;
	m_tvTimeOutSlt.tv_usec = 50;

	m_lstSendBuf.clear();
	m_pReadBuffer = NewCBuffStt();
	m_pSendBuffer = NULL;
}

clientFrame::~clientFrame()
{
	stopSvr();

	if (m_pReadBuffer) {
		FreeCBuffStt(m_pReadBuffer);
		m_pReadBuffer = NULL;
	}

	if (m_pSendBuffer) {
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

bool	clientFrame::initialModel()
{	return true; }

bool	clientFrame::exitModel()
{
	disconnSvr();
	return true;
}

void	clientFrame::setNetCfg(const std::string &strIP, int iPort)
{
	m_strAddr = strIP;
	m_iPort = iPort;
}
void	clientFrame::setConnectTimeOut(int iSec, int iuSec)
{
	m_tvTimeOutConn.tv_sec = iSec;
	m_tvTimeOutConn.tv_usec = iuSec;
}
void	clientFrame::setSelectTimeOut(int iSec, int iuSec)
{
	m_tvTimeOutSlt.tv_sec = iSec;
	m_tvTimeOutSlt.tv_usec = iuSec;
}

void	clientFrame::statCheckAlive(bool bEnable /*= false*/)
{
	m_bCheckAlive = bEnable;
}

void waitfortimeval(struct timeval &tv) 
{
#ifdef _MSC_VER
	long lsleep = tv.tv_sec * 1000 + tv.tv_usec;
	Sleep(lsleep);
#else
	long lsleep = tv.tv_sec * 1000000 + tv.tv_usec * 1000;
	usleep(lsleep);
#endif
}

bool	clientFrame::runloop()
{
	if (!checkConn())
	{
		disconnSvr();

		Fail("checkConn failure");
		waitfortimeval(m_tvTimeOutConn);
		return false;
	}

	if (!checkAlive())
	{
		disconnSvr();

		Fail("checkAlive failure");
		waitfortimeval(m_tvTimeOutConn);
		return false;
	}

	return dealNetWorkIO();
}

bool    clientFrame::connectSvr()
{
	if (m_sock != INVALID_SOCKET)
		return true;

	SOCKET s = ConnectSync(m_strAddr.c_str(), m_iPort);
//	SOCKET s = ConnectAsync(m_strAddr.c_str(), m_iPort);
	if (s == INVALID_SOCKET)
	{
		Fail("connect server failure, url[%s:%d], err[%d]", m_strAddr.c_str(), m_iPort, WSAGetLastError());
		return false;
	}

	m_sock = s;
	m_iLastCATm = (int)time(NULL);
	return afterConnectSvr();
}
void	clientFrame::disconnSvr()
{
	if (m_sock != INVALID_SOCKET)
		closesocket(m_sock);

	m_sock = INVALID_SOCKET;
	clearBuff();
}
bool    clientFrame::checkConn()
{
	return connectSvr();
}

bool    clientFrame::checkAlive()
{
	if (!m_bCheckAlive)
		return true;

	int iNow = (int)time(NULL);
	if (m_iLastCATm && m_iLastCATm + m_iIntervalCA > iNow)
		return true;

	return checkAliveMsg();
}

bool	clientFrame::checkSendFlag()
{
	FCAutoLock atLock(&m_csLock);
	if (m_lstSendBuf.empty())
		return false;
	return true;
}

bool	clientFrame::dealNetWorkIO()
{
	fd_set fdrecv, fdsend;
	FD_ZERO(&fdrecv);
	FD_ZERO(&fdsend);

	FD_SET(m_sock, &fdrecv);
	
	if (checkSendFlag())
		FD_SET(m_sock, &fdsend);

	int iMaxfd = m_sock;
#ifndef _MSC_VER
	iMaxfd++;
#endif

	int iRes = select(iMaxfd, &fdrecv, &fdsend, NULL, &m_tvTimeOutSlt);

	if (iRes < 0)
	{/// error,reconnect
		disconnSvr();

		Fail("select failure, errno[%d]", WSAGetLastError());
		waitfortimeval(m_tvTimeOutConn);
	}
	else if (iRes > 0)
	{
		if (FD_ISSET(m_sock, &fdrecv))
		{/// recv
			if (!dealEnv_Recv())
			{
				disconnSvr();

				Fail("dealEvnRecv failure, disconnect from server");
				waitfortimeval(m_tvTimeOutConn);
			}
		}

		if (FD_ISSET(m_sock, &fdsend))
		{/// send
			if (!dealEnv_Send()) 
			{
				disconnSvr();

				Fail("dealEnvSend failure, disconnect from server");
				waitfortimeval(m_tvTimeOutConn);
			}
		}
	}
	return true;
}

bool	clientFrame::dealEnv_Recv()
{
	if (!CheckCBuffSttSize(m_pReadBuffer, UTL_RECV_SIZE) && 
		!ResizeCBuffStt(m_pReadBuffer, m_pReadBuffer->iMax + LEN_CBUFFSTAT_UNIT))
	{
		Fail("dealEvnRecv, check recv buf size failure, size[%d]", m_pReadBuffer->iMax + UTL_RECV_SIZE);
		return false;
	}

	char *pRcvBuf = m_pReadBuffer->pBuf + m_pReadBuffer->iCur;
	int iRcvLen = m_pReadBuffer->iMax - m_pReadBuffer->iCur;

	int iRcv = recv(m_sock, pRcvBuf, iRcvLen, 0);
	if (iRcv == 0)
	{
		Fail("recv failure, ret is 0");
		return false;
	}
	else if (iRcv < 0)
	{
		int iErr = WSAGetLastError();
#ifdef _MSC_VER
		if (iErr != WSAEWOULDBLOCK)
#else
		if (EWOULDBLOCK != iErr && EAGAIN != iErr && EINTR != iErr)
#endif
		{
			Fail("recv failure, err[%d]", iErr);
			return false;
		}
	}
	else {
		m_pReadBuffer->iCur += iRcv;

		int iR = dealData(m_pReadBuffer->pBuf, m_pReadBuffer->iCur);

		if (iR == -1)
		{
			Fail("dealdata failure");
			return false;
		}
		else if (iR > 0)
		{
			if (iR == m_pReadBuffer->iCur)
				ResetCBuffStt(m_pReadBuffer);
			else if (iR > m_pReadBuffer->iCur)
			{
				Fail("dealdata failure, deal len[%d], cur len[%d]", iR, m_pReadBuffer->iCur);
				return false;
			}
			else 
			{
				pRcvBuf = m_pReadBuffer->pBuf;
#ifdef _MSC_VER
				memmove_s(pRcvBuf, m_pReadBuffer->iMax, pRcvBuf + iR, m_pReadBuffer->iCur - iR);
#else
				memmove(pRcvBuf, pRcvBuf + iR, m_pReadBuffer->iCur - iR);
#endif
				m_pReadBuffer->iCur -= iR;
			}
		}
	}
	return true;
}

bool	clientFrame::dealEnv_Send()
{
	if (!m_pSendBuffer && (!getBuff(m_pSendBuffer) || !m_pSendBuffer))
		return true;

	if (m_pSendBuffer->iCur <= 0)
	{
		FreeCBuffStt(m_pSendBuffer);
		m_pSendBuffer = NULL;
		return true;
	}

	char *pBufSend = m_pSendBuffer->pBuf;

	int iSend = send(m_sock, pBufSend, m_pSendBuffer->iCur, 0);
	if (iSend == 0)
	{
		Fail("send failure, send result is 0");
		return false;
	}
	if (iSend < 0)
	{
		int iErr = WSAGetLastError();
#ifdef _MSC_VER
		if (iErr != WSAEWOULDBLOCK)
#else
		if (EWOULDBLOCK != iErr && EAGAIN != iErr && EINTR != iErr)
#endif
		{
			Fail("send failure, send len[%d], err[%d]", m_pSendBuffer->iCur, iErr);
			return false;
		}
	}
	else {
		if (iSend >= m_pSendBuffer->iCur)
		{
			FreeCBuffStt(m_pSendBuffer);
			m_pSendBuffer = NULL;
		}
		else {
			pBufSend = m_pSendBuffer->pBuf;

#ifdef _MSC_VER
			memmove_s(pBufSend, m_pSendBuffer->iMax, pBufSend + iSend, m_pSendBuffer->iCur - iSend);
#else
			memmove(pBufSend, pBufSend + iSend, m_pSendBuffer->iCur - iSend);
#endif
			m_pSendBuffer->iCur -= iSend;
		}
	}
	return true;
}

bool    clientFrame::sendMsg(const char* pstrMsg, int iLen)
{
	cbuffstt *pcb = NewCBuffStt(pstrMsg, iLen);
	if (!pcb)
		return false;

	if (!setBuff(pcb))
	{
		FreeCBuffStt(pcb);
		return false;
	}
	return true;
}
bool    clientFrame::sendMsg(const std::string &strMsg)
{
	return sendMsg(strMsg.c_str(), strMsg.length());
}
bool    clientFrame::IsEmptySendBuf()
{
	FCAutoLock atlock(&m_csLock);

	if (m_pSendBuffer)
		return false;

	if (!m_lstSendBuf.empty())
		return false;
	return true;
}

bool    clientFrame::setBuff(cbuffstt *pstt)
{
	FCAutoLock atLock(&m_csLock);
	m_lstSendBuf.push_back(pstt);
	return true;
}
bool    clientFrame::getBuff(cbuffstt *&pstt)
{
	FCAutoLock atlock(&m_csLock);

	if (m_lstSendBuf.empty())
		return false;

	pstt = m_lstSendBuf.front();
	m_lstSendBuf.pop_front();
	return true;
}
void	clientFrame::clearBuff()
{
	FCAutoLock atlock(&m_csLock);

	if (m_pSendBuffer)
	{
		FreeCBuffStt(m_pSendBuffer);
	}

	ResetCBuffStt(m_pReadBuffer);

	for (std::list< cbuffstt* >::iterator itor = m_lstSendBuf.begin(); itor != m_lstSendBuf.end(); itor++)
	{
		FreeCBuffStt(*itor);
	}
	m_lstSendBuf.clear();
}

bool    clientFrame::checkAliveMsg() 
{return true;}

bool    clientFrame::afterConnectSvr()
{return true;}