#include "_threadModel.h"

_LOGERR*	_LOGERR::_Ins()
{
	static _LOGERR objLog;
	return &objLog;
}

_LOGERR::~_LOGERR(){
	if (m_pFile)
		fclose(m_pFile);
	m_pFile = NULL;

	DeleteCriticalSection(&m_csLock);
}

void	_LOGERR::log(const char *pMsg){

	FCAutoLock atLock(&m_csLock);

	if (!pMsg)
	{
		printf("exception msg [%s]\n",pMsg);
		return;
	}

	if (!m_pFile)
	{
		printf("exception file is null\n");
		return;
	}

	fwrite(pMsg,strlen(pMsg),1,m_pFile);
}

_LOGERR::_LOGERR(){
	m_strErrLog = "./ErrMsg.log";
	m_pFile = NULL;

	initLogErr();

	InitializeCriticalSection(&m_csLock);
}

void	_LOGERR::initLogErr()
{
	if (m_strErrLog.empty())
		m_strErrLog = "./ErrMsg.log";

	m_pFile = fopen(m_strErrLog.c_str(),"a+");

	if (!m_pFile)
	{
#ifdef _MSC_VER
		printf("initLogErr error, error = %d\n", GetLastError());
#else
		printf("initLogErr error, error = %d\n", errno);
#endif
		return;
	}
}

FCThreadModel::FCThreadModel()
#ifdef _MSC_VER
	:	m_hThread(INVALID_HANDLE_VALUE)
#else
	:	m_hThread( 0 )
#endif
	,	m_bExitFlag(false)
	,	m_iDelay( 0 )
{
}

FCThreadModel::~FCThreadModel()
{
	stopSvr();
}

bool	FCThreadModel::startSvr(int iDelay /*= 0*/)
{
	m_iDelay = iDelay;
	if (!initialModel())
		return false;

#ifdef _MSC_VER
	m_hThread = CreateThread(NULL, 0, threadProc, this, 0, NULL);
//		m_hThread = (HANDLE)_beginthreadex(NULL, 0, threadProc, this, 0, NULL);
	if (m_hThread == INVALID_HANDLE_VALUE)
		return false;
#else
	pthread_attr_t   thrdAttr;
	pthread_attr_init(&thrdAttr);
	/* 设置堆栈大小*/        
	if(pthread_attr_setstacksize(&thrdAttr, 1024*1024) != 0)	//1024 K
		return false;

	if (pthread_create(&m_hThread,&thrdAttr,threadProc,this))
		return false;
#endif
	return true;
}

void	FCThreadModel::stopSvr()
{
	setExitFlag();

#ifdef _MSC_VER
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_hThread,INFINITE);
// 			if (WAIT_OBJECT_0 != WaitForSingleObject(m_hThread,5 * 1000))
// 			{
// 				TerminateThread(m_hThread,0);
// 			}
		CloseHandle(m_hThread);
		m_hThread = INVALID_HANDLE_VALUE;
	}
#else
	if (m_hThread != 0)
	{
		pthread_join(m_hThread,NULL);
		m_hThread = 0;
	}
#endif

	unInitModel();
}

#ifdef _MSC_VER
static DWORD CALLBACK threadProc(LPVOID lp)
{
	FCThreadModel *pthis = (FCThreadModel *)lp;
	return pthis->doThreadProc();
}
#else
static void*	threadProc(void *lp)
{
	FCThreadModel *pthis = (FCThreadModel *)lp;
	return pthis->doThreadProc();
}
#endif

#ifdef _MSC_VER
DWORD		FCThreadModel::doThreadProc()
#else
void*		FCThreadModel::doThreadProc()
#endif
{
	//////////////////////////////////////////////////////////////////////////
	/// start delay
		
#ifdef _MSC_VER
	Sleep(1000 * m_iDelay);
#else
	usleep(1000 * 1000 * m_iDelay);
#endif

	char szMsg[102400] = {0};
	m_bExitFlag = false;
	while (true)
	{
		try
		{
			if (checkExitFlag())
			{
				exitModel();
				break;
			}

			/// do something
			if (!runloop())
			{
#ifdef _MSC_VER		/// 等待10ms
				Sleep(10);
#else
				usleep(1000 * 10);
#endif			
			}
		}
		catch (std::exception &e)
		{
			sprintf(szMsg,"%s ,catch an exception ,errMsg = %s\n",__FUNCTION__,e.what());
			_LOGERR::_Ins()->log(szMsg);
		}
		catch (...)
		{
			sprintf(szMsg,"%s ,catch an exception\n",__FUNCTION__);
			_LOGERR::_Ins()->log(szMsg);
		}
	}

#ifdef _MSC_VER
	return 0;
#else
	return NULL;
#endif
}