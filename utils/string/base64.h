/*
 * @Author: huqing 
 * @Date: 2018-06-27 16:53:46 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-06-27 16:57:03
 */

#include <string>

char*       encodeBase64(const char *pbuf, int iLen);
std::string encodeBase64(std::string &str);

bool        decodeBase64(const char *pbuf, char *&pRes, int &iResLen);
bool        decodeBase64(const std::string &str, std::string &strRes);
