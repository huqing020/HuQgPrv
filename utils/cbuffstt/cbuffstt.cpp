/*
 * @Author: huqing 
 * @Date: 2018-11-22 17:00:06 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-23 08:52:27
 */

#include "cbuffstt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cbuffstt*   NewCBuffStt(int iLen /*= LEN_CBUFFSTAT_UNIT*/){
    cbuffstt *pstt = new cbuffstt();
    if (!pstt)
        return NULL;

    pstt->pBuf = (char *)calloc(iLen, 1);
    if (!pstt->pBuf){
        delete pstt;
        return NULL;
    }

    pstt->iCur = 0;
    pstt->iMax = iLen;
    return pstt;
}

cbuffstt*   NewCBuffStt(const char *pMsg, int iLen){
    int iResLen = (iLen / LEN_CBUFFSTAT_UNIT + 1) * LEN_CBUFFSTAT_UNIT;
    cbuffstt *pstt = NewCBuffStt(iResLen);
    if (!pstt)
        return NULL;
    memcpy(pstt->pBuf, pMsg, iLen);
    pstt->iCur = iLen;
    return pstt;
}

void        ResetCBuffStt(cbuffstt *pstt){
    if (!pstt)
        return;

    if (!pstt->pBuf)
        return;

    memset(pstt->pBuf, 0, pstt->iMax);
    pstt->iCur = 0;
}
bool        CheckCBuffSttSize(cbuffstt *pstt, int iLenNew){
    if (!pstt)
        return false;
    
    if (pstt->iMax > iLenNew + pstt->iCur)
        return true;

    return false;
}
bool        ResizeCBuffStt(cbuffstt *pstt, int iLenNew, bool bReInit /*= false*/){
    if (!pstt)
        return false;

    char *pBuf = (char *)realloc(pstt->pBuf, iLenNew * sizeof(char));
    if (!pBuf)
        return false;

    pstt->pBuf = pBuf;
    pstt->iMax = iLenNew;

    if (bReInit){
        memset(pstt->pBuf, 0, pstt->iMax * sizeof(char));
        pstt->iCur = 0;
    }
    return true;
}
void        FreeCBuffStt(cbuffstt *pstt){
    if (!pstt)
        return;

    if (pstt->pBuf){
        free(pstt->pBuf);
    }

    delete pstt;
}