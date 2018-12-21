#include "_logUnit.h"
#include "tools/_tools.h"
#include <iostream>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

_logUnit::_logUnit(void)
	:	m_bLock( false )
	,	m_bScreen( false )
	,	m_strLogFile( "" )
	,	m_emLogLev( LogLev_Trace )
#ifdef _MSC_VER
	,	m_hFile( INVALID_HANDLE_VALUE )
#else
	,	m_hFile( -1 )
#endif
	,	m_bStat( false )
{
	InitializeCriticalSection(&m_csLock);
}


_logUnit::~_logUnit(void)
{
	closeLogUnit();
	DeleteCriticalSection(&m_csLock);
}

bool	_logUnit::initLogUnit(const std::string &strPath,emLOGLEV emLogLevS,bool bScreen /*= false*/,bool bLock /*= false*/)
{
	m_strLogFile = strPath;
	m_emLogLev = emLogLevS;
	m_bLock = bLock;
	m_bScreen = bScreen;

	if (!createDirectory(strPath.c_str()))
		return false;

	return openLogUnit();
}

void	_logUnit::closeLogUnit()
{
#ifdef _MSC_VER
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
#else
	if (m_hFile != -1)
	{
		close(m_hFile);
		m_hFile = -1;
	}
#endif
}

void	_logUnit::Log(emLOGLEV emLogLev,const char *pInfo,const char *pMsg)
{
#ifdef _MSC_VER
	if (m_emLogLev < emLogLev || m_hFile == INVALID_HANDLE_VALUE)
		return;
#else
	if (m_emLogLev < emLogLev || m_hFile == -1)
		return;
#endif

	std::string strMsg = getformatstr("%s | %s | %s",getTimeNowEx(1).c_str(),pInfo,pMsg);
	printScreen(strMsg);
	FCAutoLock csLock(&m_csLock,m_bLock);
	try
	{
		checkBackUp();
		checkLogUnit();

#ifdef _MSC_VER
		DWORD dwRes = 0;
		if (!WriteFile(m_hFile,strMsg.c_str(),strMsg.length(),&dwRes,NULL))
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
		if (!WriteFile(m_hFile,"\n",strlen("\n"),&dwRes,NULL))
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
#else
		write(m_hFile,strMsg.c_str(),strMsg.length());
		write(m_hFile,"\n",strlen("\n"));
#endif
	}
	catch (...)
	{
		DWORD err = 0;
#ifdef _MSC_VER
		err = GetLastError();
#else
		err = errno;
#endif
		printf("LogUint catch exception, err[%d]\n", err);
	}
}

bool	_logUnit::checkLogUnit()
{
#ifdef _MSC_VER
	if (m_hFile == INVALID_HANDLE_VALUE)
#else
	if (m_hFile == -1)
#endif
		return openLogUnit();

	return true; 
}
bool	_logUnit::openLogUnit()
{
	backUpFile(true);

#ifdef _MSC_VER
	HANDLE hFile = CreateFile(m_strLogFile.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
#else
	int hFile = open(m_strLogFile.c_str(),O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
	if (hFile < 0)
		return false;
#endif

	m_hFile = hFile;
	m_dwDT = (DWORD)getDTNow(1);

	m_bStat = true;
	return true;
}

bool	_logUnit::checkBackUp()
{
	DWORD dwNow = (DWORD)getDTNow(1);

	if (dwNow != m_dwDT)
	{
		backUpFile();
		m_dwDT = dwNow;
	}
	return true;
}
bool	_logUnit::backUpFile(bool bInit /*= false*/)
{
	size_t sPos = m_strLogFile.rfind('\\');
	if (sPos == std::string::npos)
		sPos = m_strLogFile.rfind('/');

	if (sPos == std::string::npos)
	{
		return false;
	}

	std::string strPath = m_strLogFile.substr(0,sPos + 1);
	std::string strName = m_strLogFile.substr(sPos + 1,m_strLogFile.length() - sPos - 1);
	strName = "BACK" + getTimeNow() + "_" + strName;
	std::string strTgt = strPath + strName;

	if (bInit)
	{
#ifdef _MSC_VER
		if (!_access(m_strLogFile.c_str(),0))
			MoveFile(m_strLogFile.c_str(),strTgt.c_str());
#else
		if (!access(m_strLogFile.c_str(),0))
			moveFile(m_strLogFile.c_str(),strTgt.c_str());
#endif
	}
	else
	{
#ifdef _MSC_VER
		CopyFile(m_strLogFile.c_str(),strTgt.c_str(),FALSE);
		SetFilePointer(m_hFile,0,0,FILE_BEGIN);
		SetEndOfFile(m_hFile);
		FlushFileBuffers(m_hFile);
#else
		copyFile(m_strLogFile.c_str(),strTgt.c_str());
		lseek(m_hFile,0,SEEK_SET);
		ftruncate (m_hFile,0);
#endif
	}
	return true;
}

void	_logUnit::printScreen(std::string &strMsg)
{
	if (m_bScreen)
		std::cout << strMsg << std::endl;
}