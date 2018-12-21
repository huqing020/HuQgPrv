#include "string_util.h"
#include <string.h>
#include <algorithm>

/// trim	去除字符串头尾部的空格
std::string trim(std::string &str)
{
	rtrim(str);
	ltrim(str);
	return str;
}
std::string ltrim(std::string &str)
{
	return str.erase(0, str.find_first_not_of(' '));
}
std::string rtrim(std::string &str)
{
	return str.erase(str.find_last_not_of(' ') + 1);
}

/// 字符串转换大小写
std::string makeUpper(std::string &str)
{
	transform(str.begin(),str.end(),str.begin(),::toupper);
	return str;
}
std::string makeUpper(const char* str)
{
	std::string strRes(str);
	return makeUpper(strRes);
}
std::string makeLower(std::string &str)
{
	transform(str.begin(),str.end(),str.begin(),::tolower);
	return str;
}
std::string makeLower(const char* str)
{
	std::string strRes(str);
	return makeLower(strRes);
}

/// remove 删除串中指定字符
int			remove(std::string &str,char cFlag)
{
	std::string strFlag(&cFlag);
	return remove(str,strFlag);
}

int			remove(std::string &str,const std::string &strFlag)
{
	if (str.empty() || strFlag.empty())
		return 0;

	size_t sPos = 0;
	int	iTol = 0;

	while ((sPos = str.find(strFlag)) != std::string::npos)
	{
		str.erase(sPos,strFlag.length());
		iTol++;
	}
	return iTol;
}

/// replace 使用指定的字符(串)，替换源字符串中的内容
int			replace(std::string &str,char cFlagOld,char cFlagNew)
{
	std::string strFlagOld(&cFlagOld);
	std::string strFlagNew(&cFlagNew);
	return replace(str,strFlagOld,strFlagNew);
}

int			replace(std::string &str,const std::string &strFlagOld,const std::string &strFlagNew)
{
	if (str.empty() || strFlagOld.empty() || strFlagNew.empty())
		return 0;

	size_t sPos = 0;
	int	iTol = 0;

	while ((sPos = str.find(strFlagOld.c_str())) != std::string::npos)
	{
		str.replace(sPos,strFlagOld.length(),strFlagNew.c_str());
		iTol++;
	}
	
	return iTol;
}
