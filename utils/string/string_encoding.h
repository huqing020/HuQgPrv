#ifndef _STRING_ENCODING_H
#define _STRING_ENCODING_H
#include "iconv/iconv.h"
#include <string>

/// 字符转码
int		code_convert(const char *from_charset, const char *to_charset, const char *inbuf, size_t inlen, char *outbuf, size_t outlen);

/// utf转gbk
int		utf2gbk(const char *inbuf, int inlen, char *outbuf, int outlen);
char*	utf2gbk(const char *inbuf, int inlen);

/// gbk转utf
int		gbk2utf(const char *inbuf, int inlen, char *outbuf, int outlen);
char*	gbk2utf(const char *inbuf, int inlen);

bool	IsTextUTF8(const char* str, long length);

std::string	urlEncode(std::string strUrl);

char *	url_encode(char const *s, int len, int *new_length);

#endif  /* _STRING_ENCODING_H */
