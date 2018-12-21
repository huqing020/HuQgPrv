/*
 * @Author: huqing 
 * @Date: 2018-11-23 10:13:02 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-23 10:16:14
 */

#include "cbfuncLog.h"
#include "tools/_tools.h"
#include <stdarg.h>

cbLogFrm::cbLogFrm()
	: m_cbFunc(NULL)
{

}

cbLogFrm::~cbLogFrm()
{

}

void	cbLogFrm::setCBFuncLog(cbFuncLog cbfunc)
{
	m_cbFunc = cbfunc;
}

extern std::string getformatstr(const char *pszFmt, va_list &pArg);
void	cbLogFrm::Fail(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_FAIL);
}
void	cbLogFrm::Warn(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_WARN);
}
void	cbLogFrm::Info(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_INFO);
}
void	cbLogFrm::Trace(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_TRACE);
}
void	cbLogFrm::Debug(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_DEBUG_1);
}
void	cbLogFrm::Debug1(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_DEBUG_1);
}
void	cbLogFrm::Debug2(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_DEBUG_2);
}
void	cbLogFrm::Debug3(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_DEBUG_3);
}
void	cbLogFrm::Debug4(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_DEBUG_4);
}
void	cbLogFrm::Debug5(const char *pstrFmt, ...)
{
	std::string strRes = "";
	try
	{
		va_list	pArg;
		va_start(pArg, pstrFmt);
		strRes = getformatstr(pstrFmt, pArg);
		va_end(pArg);
	}
	catch (...)
	{
		return;
	}

	cbLogPrintMsg(strRes.c_str(), strRes.length(), enCBLogLev_DEBUG_5);
}

void	cbLogFrm::cbLogPrintMsg(const char *pstrMg, int iLen, EmCBLogLev emLev)
{
	if (m_cbFunc)
		m_cbFunc(pstrMg, iLen, emLev);
}