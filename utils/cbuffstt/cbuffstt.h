/*
 * @Author: huqing 
 * @Date: 2018-11-22 17:00:16 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-23 08:52:03
 */

#ifndef __CC_BUFFER_STRUCT_HEAD
#define __CC_BUFFER_STRUCT_HEAD

#define LEN_CBUFFSTAT_UNIT  1024*4

struct cbuffstt
{
    char*   pBuf;
    int     iCur;
    int     iMax;
};

cbuffstt*   NewCBuffStt(int iLen = LEN_CBUFFSTAT_UNIT);
cbuffstt*   NewCBuffStt(const char *pMsg, int iLen);
void        ResetCBuffStt(cbuffstt *pstt);
bool        CheckCBuffSttSize(cbuffstt *pstt, int iLenNew);
bool        ResizeCBuffStt(cbuffstt *pstt, int iLenNew, bool bReInit = false);
void        FreeCBuffStt(cbuffstt *pstt);


#endif  /// __CC_BUFFER_STRUCT_HEAD