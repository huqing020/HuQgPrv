/*
 * @Author: huqing 
 * @Date: 2018-11-23 10:13:02 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-23 10:16:14
 */

#ifndef _DEF_CALLBACK_FUNC_LOG_HEAD
#define _DEF_CALLBACK_FUNC_LOG_HEAD

enum EmCBLogLev {
	enCBLogLev_FAIL,
	enCBLogLev_WARN,
	enCBLogLev_INFO,
	enCBLogLev_TRACE,
	enCBLogLev_DEBUG_1,
	enCBLogLev_DEBUG_2,
	enCBLogLev_DEBUG_3,
	enCBLogLev_DEBUG_4,
	enCBLogLev_DEBUG_5,
};

typedef void(*cbFuncLog)(const char *pstrMsg, int iLen, EmCBLogLev emloglev);

class cbLogFrm
{
public:
	cbLogFrm();
	virtual ~cbLogFrm();

	void	setCBFuncLog(cbFuncLog cbfunc);
protected:
	void	Fail(const char *pstrFmt, ...);
	void	Warn(const char *pstrFmt, ...);
	void	Info(const char *pstrFmt, ...);
	void	Trace(const char *pstrFmt, ...);
	void	Debug(const char *pstrFmt, ...);
	void	Debug1(const char *pstrFmt, ...);
	void	Debug2(const char *pstrFmt, ...);
	void	Debug3(const char *pstrFmt, ...);
	void	Debug4(const char *pstrFmt, ...);
	void	Debug5(const char *pstrFmt, ...);

protected:
	virtual	void cbLogPrintMsg(const char *pstrMg, int iLen, EmCBLogLev emLev);

private:
	cbFuncLog	m_cbFunc;
};

#endif	/// _DEF_CALLBACK_FUNC_LOG_HEAD