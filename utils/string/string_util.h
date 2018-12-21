#ifndef __STRING_UTIL_HEAD_C
#define __STRING_UTIL_HEAD_C

#include <string>

/// trim	去除字符串头尾部的空格
std::string trim(std::string &str);
std::string ltrim(std::string &str);
std::string rtrim(std::string &str);

/// 字符串转换大小写
std::string makeUpper(const char* str);
std::string makeUpper(std::string &str);
std::string makeLower(const char* str);
std::string makeLower(std::string &str);

/// remove 删除串中指定字符
int			remove(std::string &str,char cFlag);
int			remove(std::string &str,const std::string &strFlag);

/// replace 使用指定的字符(串)，替换源字符串中的内容
int			replace(std::string &str,char cFlagOld,char cFlagNew);
int			replace(std::string &str,const std::string &strFlagOld,const std::string &strFlagNew);

#endif  /* __STRING_UTIL_HEAD_C */
