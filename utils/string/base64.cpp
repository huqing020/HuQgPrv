#include "base64.h"
#include <stdlib.h>
#include <string.h>

 const char *base64_table =       "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 const char *base64_table_url =   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

char*       encodeBase64(const char *pbuf, int iLen)
{
    int iLenTgt = 0;
    if (iLen % 3 == 0)
        iLenTgt = iLen / 3 * 4;
    else
        iLenTgt = (iLen / 3 + 1) * 4;
    
    char *pRes = (char *)malloc(iLenTgt + 1);
    if (!pRes)
        return NULL;

    memset(pRes, 0, iLenTgt + 1);

    char *pIn = (char *)pbuf;
    char *pOut = pRes;
    int i = 0, j = 0;

    for (i = 0, j = 0; i < iLen / 3 * 3; i += 3, j += 4)
    {
        unsigned int uVal = ((unsigned int)pbuf[i + 0] << 16) | ((unsigned int)pbuf[i + 1] << 8) | (unsigned int)pbuf[i + 2];
        pOut[j + 0] = base64_table[uVal >> 18 & 0x3f];
        pOut[j + 1] = base64_table[uVal >> 12 & 0x3f];
        pOut[j + 2] = base64_table[uVal >>  6 & 0x3f];
        pOut[j + 3] = base64_table[uVal & 0x3f];
    }

    int iLast = iLen - i;
    if (iLast == 0)
        return pRes;

    unsigned int uVal = (unsigned int)pbuf[i + 0] << 16;
    if (iLast == 2)
        uVal |= (unsigned int)pbuf[i + 1] << 8;
    pOut[j + 0] = base64_table[uVal >> 18 & 0x3f];
    pOut[j + 1] = base64_table[uVal >> 12 & 0x3f];

    switch (iLast)
    {
    case 1:
        {
            pOut[j + 2] = '=';
            pOut[j + 3] = '=';
        }
        break;
    case 2:
        {
            pOut[j + 2] = base64_table[uVal >> 6 & 0x3f];
            pOut[j + 3] = '=';
        }
        break;
    }
    pOut[j + 4] = 0;
    return pRes;
}

std::string encodeBase64(std::string &str)
{
    char *pRes = encodeBase64(str.c_str(), str.length());
    if (pRes == NULL)
        return "";
    
    std::string strRes = pRes;
    free(pRes);
    return strRes;
}

int num_strchr(const char *str, char c) // 
{
    const char *pindex = strchr(str, c);
    if (NULL == pindex){
        return -1;
    }
    return pindex - str;
}

char*       decodeBase64Ex(const char *pbuf, int &iResLen)
{
    int iLen = strlen(pbuf);

    char *pRes = (char *)malloc(iLen);
    if (!pRes)
        return NULL;

     int i = 0, j=0;
    int trans[4] = {0,0,0,0};
    for (;pbuf[i]!='\0';i+=4){
        // 每四个一组，译码成三个字符
        trans[0] = num_strchr(base64_table, pbuf[i]);
        trans[1] = num_strchr(base64_table, pbuf[i+1]);
        // 1/3
        pRes[j++] = ((trans[0] << 2) & 0xfc) | ((trans[1]>>4) & 0x03);

        if (pbuf[i+2] == '='){
            continue;
        }
        else{
            trans[2] = num_strchr(base64_table, pbuf[i + 2]);
        }
        // 2/3
        pRes[j++] = ((trans[1] << 4) & 0xf0) | ((trans[2] >> 2) & 0x0f);

        if (pbuf[i + 3] == '='){
            continue;
        }
        else{
            trans[3] = num_strchr(base64_table, pbuf[i + 3]);
        }

        // 3/3
        pRes[j++] = ((trans[2] << 6) & 0xc0) | (trans[3] & 0x3f);
    }

    pRes[j] = '\0';
    iResLen = j;
    return pRes;
}


char*       decodeBase64(const char *pbuf, int &iResLen)
{
    int iLen = strlen(pbuf);

    if (iLen % 4)
    {
        iLen = (iLen / 4 + 1) * 4 + 1;
        char *pNew = (char *)malloc(iLen);
        memset(pNew, 0, iLen);
        memcpy(pNew, pbuf, strlen(pbuf));

        return decodeBase64Ex(pNew, iResLen);
    }
    else{
        return decodeBase64Ex(pbuf, iResLen);
    }
}      

bool        decodeBase64(const std::string &str, std::string &strRes)
{
    int iLen = 0;
    char *pRes = decodeBase64(str.c_str(), iLen);
    if (pRes == NULL)
        return false;
    
    strRes = pRes;
    free(pRes);
    return true;
}