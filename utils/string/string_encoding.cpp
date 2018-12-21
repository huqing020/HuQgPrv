#include "string_encoding.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

int		code_convert(const char *from_charset, const char *to_charset, const char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	int iRet = -1;
	iconv_t cd;
	char *pinbuf = const_cast<char*>(inbuf);
	const char **pin = (const char **)&pinbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd ==  (iconv_t)-1)
		return iRet;

	memset(outbuf,0,outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == (size_t)-1)
	{
		iconv_close(cd);
		return iRet;
	}
	else iRet = 0;
	iconv_close(cd);
	return iRet;
}

int		utf2gbk(const char *inbuf, int inlen, char *outbuf, int outlen)
{
	return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}

char*	utf2gbk(const char *inbuf, int inlen)
{
	int iLen = inlen * 2 + 100;
	char *pBuf = new char[iLen];
	memset(pBuf, 0, iLen);
	if (utf2gbk(inbuf, inlen, pBuf, iLen) == -1)
	{
		delete[]pBuf;
		pBuf = NULL;
	}
	return pBuf;
}

int		gbk2utf(const char *inbuf, int inlen, char *outbuf, int outlen)
{
	return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}

char*	gbk2utf(const char *inbuf, int inlen)
{
	int iLen = inlen * 2 + 100;
	char *pBuf = new char[iLen];
	memset(pBuf, 0, iLen);
	if (gbk2utf(inbuf, inlen, pBuf, iLen) == -1) 
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return pBuf;
}

bool	IsTextUTF8(const char* str, long length)
{
	int i;
	///	UFT8可用1-6个字节编码,ASCII用一个字节
	int nBytes = 0;
	unsigned char chr;
	//如果全部都是ASCII, 说明不是UTF-8
	bool bAllAscii = true;
	for(i = 0; i < length; i++)
	{
		chr= *(str + i);
		if( (chr & 0x80) != 0 )
		{
			/// 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
			bAllAscii= false;
		}

		if(nBytes == 0)
		{
			//如果不是ASCII码,应该是多字节符,计算字节数
			if(chr >= 0x80)
			{
				if(chr >= 0xFC && chr <= 0xFD)
				{
					nBytes = 6;
				}
				else if(chr >= 0xF8)
				{
					nBytes = 5;
				}
				else if(chr >= 0xF0)
				{
					nBytes = 4;
				}
				else if(chr >= 0xE0)
				{
					nBytes = 3;
				}
				else if(chr >= 0xC0)
				{
					nBytes = 2;
				}
				else
				{
					return false;
				}
				nBytes--;
			}
		}
		else
		{
			/// 多字节符的非首字节,应为 10xxxxxx
			if( (chr & 0xC0) != 0x80 )
			{
				return false;
			}
			nBytes--;
		}
	}

	if( nBytes > 0 )
	{
		/// 违返规则
		return false;
	}

	if( bAllAscii )
	{
		/// 如果全部都是ASCII, 说明不是UTF-8
		return false;
	}
	return true;
}

std::string	urlEncode(std::string strUrl)
{
	std::string strRes = "";
	if (!IsTextUTF8(strUrl.c_str(), strUrl.length()))
	{
		char *pRes = gbk2utf(strUrl.c_str(), strUrl.length());
		strRes = pRes;
		delete[]pRes;
	}
	else strRes = strUrl;

	int iResLen = 0;
	char *pRes = url_encode(strRes.c_str(), strRes.length(), &iResLen);
	if (!pRes)
		return "";

	strRes = pRes;
	free(pRes);
	return strRes;
}

static unsigned char hexchars[] = "0123456789ABCDEF";
char *url_encode(char const *s, int len, int *new_length)
{
	register unsigned char c;
	unsigned char *to, *start;
	unsigned char const *from, *end;

	from = (unsigned char *)s;
	end = (unsigned char *)s + len;
	start = to = (unsigned char *)calloc(1, 3 * len + 1);

	while (from < end)
	{
		c = *from++;

		if (c == ' ')
		{
			*to++ = '+';
		}
		else if ((c < '0' && c != '-' && c != '.') ||
			(c < 'A' && c > '9') ||
			(c > 'Z' && c < 'a' && c != '_') ||
			(c > 'z'))
		{
			to[0] = '%';
			to[1] = hexchars[c >> 4];
			to[2] = hexchars[c & 15];
			to += 3;
		}
		else
		{
			*to++ = c;
		}
	}
	*to = 0;
	if (new_length)
	{
		*new_length = to - start;
	}
	return (char *)start;
}
