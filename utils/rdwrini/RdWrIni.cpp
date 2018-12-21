/********************************************************************  
created:    2009/10/10  
filename:   RdWrIni.cpp,RdWrIni.h
author:     YRC  
purpose:    Read and Write Ini file  
*********************************************************************/  

#include "RdWrIni.h"
#include <pthread.h>

pthread_mutex_t gcsLock;

class GRdWrLock
{
public:
	GRdWrLock()
	{
		pthread_mutex_init(&gcsLock,NULL);
	}
	~GRdWrLock()
	{
		pthread_mutex_destroy(&gcsLock);
	}
};

class AutoLock
{
public:
	AutoLock(pthread_mutex_t *pLock)
		:	m_pLock( pLock )
	{
		if (m_pLock)
			pthread_mutex_lock(m_pLock);
	}

	~AutoLock()
	{
		if (m_pLock)
			pthread_mutex_unlock(m_pLock);

		m_pLock = NULL;
	}

private:
	pthread_mutex_t *	m_pLock;
};


GRdWrLock	gLock;

LPSTR g_pData = NULL;    // 存储整个INI文件数据的缓冲区    

/************************************************************************  
**函数：CopyFile  
**功能：The CopyFile function copies an existing file to a new file. 

**参数：  
		lpExistingFileName[in] -Pointer to a null-terminated string
							    that specifies the name of an existing file. 
		lpNewFileName[in]      - Pointer to a null-terminated string that specifies the name of the new file. 
		bFailIfExists[in]      - Specifies how this operation is to proceed if a file of the same name as that 
							      specified by lpNewFileName already exists.
								   If this parameter is TRUE and the new file already exists, the function fails. 
									If this parameter is FALSE and the new file already exists, 
									the function overwrites the existing file and succeeds. 
**返回：  
		TRUE - success  
		FALSE - fail 
**作者：YRC
**日期：09.10.20  
**备注：
************************************************************************/  

BOOL CopyFile(LPCSTR lpExistingFileName, // name of an existing file
			  LPCSTR lpNewFileName,      // name of new file
			  BOOL bFailIfExists         // operation if file exists
			  )
{
	INT fdIn = 0, fdOut = 0;
	struct stat statinfo;
	DWORD dwSize =0;
	void *pSrc =NULL, *pDst = NULL;
	
	memset(&statinfo,0, sizeof(statinfo));
	if (!lpExistingFileName || !lpExistingFileName)
	{
		return FALSE;
	}
	
	fdIn = open(lpExistingFileName, O_RDONLY);
	if (fdIn<0)
	{
		perror("CopyFile open");
		return FALSE;
	}
	// get file info
	if (fstat(fdIn, &statinfo) < 0) 
	{
		perror("CopyFile fstat ");
	}
	dwSize = statinfo.st_size;
	//printf("statinfo.st_size = %d\n ", statinfo.st_size);
	if (bFailIfExists)
	{
		fdOut = open(lpNewFileName, O_RDWR | O_CREAT | O_EXCL,S_IRWXU);
		if (fdOut== -1)
		{
			perror("CopyFile new file fail");
			return FALSE;
		}
	}
	else
	{
		fdOut = open(lpNewFileName, O_RDWR | O_CREAT |O_TRUNC,S_IRWXU);
		if (fdOut== -1)
		{
			perror("CopyFile new file fail ");
			return FALSE;
		}
	}
	//printf("CopyFile lseek\n");
	if (lseek(fdOut, dwSize - 1, SEEK_SET) == -1) 
	{
		perror("CopyFile lseek ");
		return FALSE;
	}
	
	if (write(fdOut, "", 1) != 1) 
	{
		perror("the file isn't writable ");
		return FALSE;
	}
	//printf("CopyFile write\n");
	if ((pSrc = mmap(NULL, dwSize, PROT_READ, MAP_SHARED, fdIn, 0)) == MAP_FAILED)
	{
		perror("mmap ");
		return FALSE;	
	}
	//printf("CopyFile mmap 1\n");
	if ((pDst = mmap(NULL, dwSize, PROT_READ|PROT_WRITE, MAP_SHARED, fdOut, 0)) == MAP_FAILED) 
	{
		perror("mmap ");
		return FALSE;
	}
	//printf("CopyFile mmap 2\n");
	//dose copy the file
	memcpy(pDst,pSrc, dwSize);
	close(fdOut);
	close(fdIn);

//	if (pSrc)
//	{
//		free(pSrc);
//	}
//	if (pDst)
//	{
//		free(pDst);
//	}
//	printf("CopyFile exit\n");
	return TRUE;
}

/************************************************************************  
**函数：GetLine  
**功能：获取在g_pData中从dwOffset位置开始的一行数据并保存到pLine，同时把偏移量dwOffset  
		移到下一行行首  
**参数：  
		pLine[out]   - 接收一行数据(不包括\r\n)  
		dwOffset[in] - 要读取的那一行的开始位置  
		dwSize[in]   - INI文件大小  
**返回：  
		正确 - 下一行行首的位置  
		错误 - 0  
**作者：YRC
**日期：09.10.19  
**备注：
************************************************************************/   
INT	GetLine(LPSTR pLine, DWORD dwOffset, DWORD dwSize)
{
	int len = 0;   
    int len2 = 0;   
    // Look for the end of the line.    
    while ( dwOffset + len < dwSize   
        && '\r' != g_pData[dwOffset+len] && '\n' != g_pData[dwOffset+len])   
    {   
        if( g_pData[dwOffset+len]==0 )   
            break;   
        pLine[len] = g_pData[dwOffset+len] ;   
        ++len;   
    }   
	
    pLine[len] = 0 ;   
    // Now push the internal offset past the newline.    
    // (We assume \r\n pairs are always in this order)    
    if (dwOffset + len + len2 < dwSize && '\r' == g_pData[dwOffset+len+len2])   
        ++len2;   
    if (dwOffset + len + len2+1 < dwSize && '\n' == g_pData[dwOffset+len+len2])   
        ++len2;   
    if (2 >= len + len2 && (dwOffset +2 >=dwSize) )   
        return 0;   
	
    dwOffset += len + len2;   
    return dwOffset;   
}

/************************************************************************  
**函数：IsComment  
**功能：判断是不是注释行  
**参数：  
		pLine[in] - INI的一行数据  
**返回：  
		1 - 注释行  
		0 - 不是注释行  
**备注：  
		1). 空行也视为注释行  
************************************************************************/ 

BOOL IsComment(LPCSTR pLine)
{
	if (!pLine || 0 == strlen(pLine) || '#' == *pLine)
        return TRUE;

	for (unsigned int i = 0 ; i < strlen(pLine) ; i++)
	{
		if (pLine[i] != ' ' && pLine[i] == '#')
			return TRUE;
	}
    return FALSE;  
}

/************************************************************************  
**函数：IsSection  
**功能：判断是不是段名  
**参数：  
		pLine[in] - INI的一行数据  
**返回：  
		1 - 是段名  
		0 - 不是  
************************************************************************/   
BOOL IsSection(LPCSTR pLine)   
{   
    if (pLine && '[' == *pLine)   
        return TRUE;   
    return FALSE;   
}   

/************************************************************************  
**函数：IsSectionName  
**功能：判断是INI文件的一行(pLine)是不是要找的段名(pSection)  
**参数：  
pLine[in]    - INI文件的一行数据  
pSection[in] - 要找的段名  
**返回：  
1 - 是  
0 - 不是  
**备注：  
************************************************************************/   
BOOL IsSectionName(LPCSTR pLine, LPCSTR pSection)   
{   
    if (IsSection(pLine))   
    {   
        DWORD len = strlen(pSection);   
        if (strlen(pLine) - 2 == len && 0 == strncasecmp(pLine+1, pSection, len))   
            return TRUE;
    }   
    return FALSE;   
}  


/************************************************************************  
**函数：IsKey  
**功能：判断INI文件中一行的数据是不是要找的键名,如果是并读取键值  
**参数：  
		pLine[in] - INI文件某行数据  
		pKeyName[in] - 要寻找的键名  
		pValue[out] - 键值  
		dwValLen[out] - 键值pValue大小(in bytes)  
**返回：  
		1 - 是，同时pValue返回键值  
		0 - 不是，pValue为NULL  
**作者：YRC  
**日期：09.10.19  
**备注：  
************************************************************************/   
BOOL IsKey(LPSTR pLine , LPCSTR pKeyName, LPSTR* pValue, DWORD* dwValLen )   
{   
    LPSTR pEqual = NULL;   
    DWORD length = 0, len = 0;   
	
    if(!pLine || !pKeyName)   
        return FALSE;   
	
    // pLine是不是注释行    
    if (IsComment( pLine ))   
        return FALSE;   
	
    // 寻找"="号    
    pEqual = strchr(pLine, '=' );   
    if (!pEqual)   
        return FALSE;   
	
    // 寻找键名最后一字符的位置    
    //while (pEqual - 1 >= pLine && iswspace(*(pEqual-1)))   
	while(pEqual - 1 >= pLine && ((*(pEqual-1)) == '\r' || (*(pEqual-1)) == '\n')) 
        --pEqual;   
    // Badly formed file.    
    if (pEqual - 1 < pLine)   
        return FALSE;

    // 键名长度    
    length = pEqual - pLine;
	while (length > 0 && pLine[length - 1] == ' ')
		length--;
	
    len = strlen(pKeyName);   
    //if (len == length && 0 == _wcsnicmp(pLine, pKeyName, len))    
    if (length == len && 0 == strncasecmp(pLine, pKeyName, len))   
    {   
        *pValue = strchr(pLine, '=' );   
        ++(*pValue);   
        *dwValLen = strlen(pLine) - ((*pValue) - pLine);   
		
        // 去掉紧跟在"="号后的所有空格    
        //while (0 < *dwValLen && iswspace(**pValue))  
		while (0 < *dwValLen && ((**pValue) == ' '))   
        {   
            ++(*pValue);   
            --(*dwValLen);   
        }   
        while (0 < *dwValLen && ((*pValue)[*dwValLen-1] == ' '))   
        {   
            --(*dwValLen);   
        }   
        // If the string is surrounded by quotes, remove them.    
        if ('"' == (**pValue))   
        {   
            ++(*pValue);   
            --(*dwValLen);   
            if ('"' == (*pValue)[*dwValLen-1])   
            {   
                --(*dwValLen);   
            }   
        }   
        return TRUE;   
    }   
    else   
    {   
        *pValue = NULL;   
        return FALSE;   
    }   
}   

/************************************************************************  
**函数：ReadIniFile  
**功能：打开文件，并将文件数据拷贝到一缓冲区g_pData中   
**参数：  
		lpFileName[in] - INI文件名，如果没有路径，默认路径当前应用程序路劲 
**返回：  
		非0 - 该INI文件的大小(in bytes)  
		0   - 失败  
**作者：YRC  
**日期：09.10.20  
**备注：  

************************************************************************/   
INT ReadIniFile(LPCSTR lpFileName)   
{   
    int nReturn = 0;   
	struct stat stat;
    int fdFile =0;   
    DWORD dwSize =0 ;   
    DWORD  dwRet = FALSE; // bUnicode - 标志INI文件是不是Unicode文件    
    BYTE* pData = NULL; 
	char buf[255] = {0};
	
    if (!lpFileName)   
    {   
        nReturn = 0;   
        goto exit;   
    }   
	
    // ReadIniFile the file.    
	fdFile = open(lpFileName, O_RDONLY);
	if(fdFile < 0)
	{
		perror("Open file fail");
		nReturn = 0;   
        goto exit;   
	}
	//printf("+++++++fdFile = %d\n", fdFile);
	if(read(fdFile, buf,sizeof(buf))>0)
	{
//		printf("++++++buf :%s \n",buf);
	}

    // Get its size.    
	
	if((fstat(fdFile,&stat) < 0) )
	{
		nReturn = 0;   
        goto exit;   
	}
	
	dwSize = stat.st_size;
	
    // Next, load the data.    
    //printf("ReadIniFile, the size of ini file [%s] is [%d]\n", lpFileName, dwSize);     
    if (0 < dwSize)   
    {   
        pData = (BYTE *)malloc((dwSize + sizeof(char)));   
        if (!pData)   
        {   
//			printf("ReadIniFile, no enough memory\n");   
            nReturn = 0;   
            goto exit;   
        }   
        memset(pData,0,dwSize + sizeof(char));
		lseek(fdFile, 0, SEEK_SET);
		dwRet = read(fdFile, pData, dwSize);
        close(fdFile);   
        if (dwRet != dwSize)   
        {   
			
//            printf("ReadIniFile, ReadFile fail, err = %s\n",strerror(errno));    
            nReturn = 0;   
            goto exit;   
        }   
		
        // Create the buffer.    
        if( g_pData )   
        {   
            free(g_pData);   
            g_pData = NULL;   
        }   
		
        g_pData = (char *)malloc(dwSize*sizeof(char));   
        if(!g_pData)   
        {   
//            printf("ReadIniFile, no enough momery\n");   
            nReturn = 0;   
            goto exit;   
        }   
		
		memcpy(g_pData, pData, dwSize);     
        nReturn = dwSize;   
    } 
	else
	{
		//printf("read file dwSize<0\n");
		goto exit;
	}
	
exit:   
    if(pData)   
    {   //printf("ReadIniFile free\n");
        free(pData);   
        pData = NULL;   
    }   
    if(fdFile)   
    {  
		//printf("ReadIniFile close\n");
        close(fdFile);   
        fdFile = 0;   
    }  
	//printf("exit ReadIniFile\n");
    return nReturn;   
}   


/************************************************************************  
**函数：GetString  
**功能：读INI文件  
**参数：  
		lpAppName[in]         - 字段名  
		lpKeyName[in]         - 键名  
		lpReturnedString[out] - 键值  
		nSize[in]             - 键值缓冲区大小(in characters )  
		lpFileName[in]        - 完整的INI文件名  
**返回：Returns the number of bytes read.  
**作者：YRC  
**日期：09.10.20  
**备注：  
************************************************************************/   
DWORD GetString( LPCSTR lpAppName,LPCSTR lpKeyName,   
                LPSTR lpReturnedString, DWORD nSize,LPCSTR lpFileName)   
{     
    DWORD dwSize , cchCopied;   
    DWORD dwOffset = 0;   
    char pLine[MAX_PATH] = {0} ;   
    DWORD dwValLen = 0;   
	
    if (!lpAppName || !lpFileName)   
        return 0;   
	
    //打开文件，将文件数据拷贝到缓冲区g_pData中，返回INI文件大小    
    if (0 == (dwSize = ReadIniFile(lpFileName)))   
    {   
//        printf("GetString, Could not ReadIniFile INI file: %s\n", lpFileName);   
        return 0;   
    }   
	
    cchCopied = 0;   
    while ( 0 != (dwOffset = GetLine( pLine , dwOffset , dwSize )))   
    {   
        //RETAILMSG(1,(_T("%s\n"),szLine));    
        // 是不是注释行    
        if (IsComment(pLine))   
            continue;   
		
        // 是不是段名    
        if (IsSection(pLine))   
        {   
            // 是不是我们要找的段名    
            if (IsSectionName(pLine,lpAppName))   
            {   
                // 寻找我们要的键名    
                while ( 0 != (dwOffset = GetLine(pLine , dwOffset , dwSize)))   
                {   
                    LPSTR pValue=NULL;   

					if (IsComment(pLine))   
						continue;
					
                    if (IsSection(pLine))   
                        break;   
					
                    if (IsKey(pLine , lpKeyName, &pValue, &dwValLen))   
                    {   
                        cchCopied = Min(dwValLen, nSize-1);   
                        memcpy(lpReturnedString, pValue, cchCopied);   
                        lpReturnedString[cchCopied] = 0;   
                        // We're done.    
                        break;                         
                    }   
                }   
                break;   
            }   
        }   
    }   
	
    return cchCopied;   
}  

/************************************************************************  
**函数：GetPrivateProfileString  
**功能：LINUX下读取INI文件中某段名/键名的键值的字符串  
**参数：  
		lpAppName[in]         - points to section name  
		lpKeyName[in]         - points to key name  
		lpDefault[in]         - points to default string  
		lpReturnedString[out] - points to destination buffer  
		nSize[in]             - size of destination buffer "lpReturnedString"(in characters)  
		lpFileName[in]        - points to initialization filename     
**返回：The return value is the number of characters copied to the buffer,   
		not including the terminating null character.          
**作者：YRC 
**日期：09.10.20  
**备注：  
1). 如果INI文件没有你关心的数据，返回默认值lpDefault  
************************************************************************/   
DWORD GetPrivateProfileString(   
							  LPCSTR lpAppName,   
							  LPCSTR lpKeyName,   
							  LPCSTR lpDefault,    
							  LPSTR  lpReturnedString,   
							  DWORD   Size,   
							  LPCSTR lpFileName )   
{   
	AutoLock	csLock(&gcsLock);
	
    DWORD dwRc = 0, dwReturn = 0;   
    if(!lpAppName || !lpKeyName || !lpReturnedString || !lpFileName || Size<=0 )   
        return 0;   
	
    dwRc = GetString(lpAppName,lpKeyName,lpReturnedString,Size,lpFileName);   
    if(dwRc != 0)   
    {   
        dwReturn = dwRc;   
    }   
    else   
    {   
        if(lpDefault)   
        {   
            memcpy(lpReturnedString, lpDefault, Size);   
            lpReturnedString[Size-1] = 0;   
        }   
        else   
            *lpReturnedString = 0;   
        dwReturn = strlen(lpReturnedString);   
    }   
	
    // 释放内存    
    if (NULL != g_pData)   
    {   
        free( g_pData );   
        g_pData = NULL;   
    }   
	
    return dwReturn;   
}   

/************************************************************************  
**函数：GetPrivateProfileInt  
**功能： retrieves an integer associated with a key in the  
		specified section of the given initialization file  
**参数：  
		LPCTSTR lpAppName,  // address of section name  
		LPCTSTR lpKeyName,  // address of key name  
		INT nDefault,       // return value if key name is not found  
		LPCTSTR lpFileName  // address of initialization filename  
**返回：  
		The return value is the integer equivalent of the string following   
		the specified key name in the specified initialization file. If the   
		key is not found, the return value is the specified default value.   
		If the value of the key is less than zero, the return value is zero.   
**作者：YRC 
**日期：09.10.20 
**备注：  
************************************************************************/   
UINT GetPrivateProfileInt(   
						  LPCSTR lpAppName,   
						  LPCSTR lpKeyName,    
						  INT nDefault,   
						  LPCSTR lpFileName )   
{   
	AutoLock	csLock(&gcsLock);

    char szRet[80] ={0}; 
	DWORD cch =0;
	
    if(!lpAppName || !lpKeyName || !lpFileName )   
        return 0;   
	
    cch = GetString(lpAppName, lpKeyName, szRet, sizeof(szRet)/sizeof(char), lpFileName);   
	
    // 释放内存    
    if (NULL != g_pData)   
    {   
        free( g_pData );   
        g_pData = NULL;   
    }   
	
    if (cch)   
        return atoi(szRet);   
    else   
        return nDefault;       
}  

/************************************************************************  
**函数：WriteLine  
**功能：向文件写入一行数据(包括回车换行符)  
**参数：  
		hOutput[in] - 已打开的文件句柄  
		pLine[in]   - 要写入的一行数据  
**返回：NONE  
**作者：YRC  
**日期：09.10.20
**备注：  
		1). 写入一行数据，也包括写入行末的"\r\n"两个字符  
		3). 注意不要将结束符也写入文件  
************************************************************************/   
void WriteLine(INT hOutput , LPCSTR pLine)   
{   
	if (pLine)   
	{   
		write(hOutput, pLine, strlen(pLine)*sizeof(char));   
		write(hOutput, "\r\n", 2*sizeof(char));   
	}   
}   

/************************************************************************  
**函数：WritePrivateProfileString  
**功能：LINUX环境下，向指定INI文件指定段名写入字符串数据  
**参数：  
		lpAppName[in]  
		Pointer to a null-terminated string containing section name. If  
		the section does not exit, it is created.  
		lpKeyName[in]  
		Pointer to a null-terminated string containing key name. If the  
		key does not exit in the specified section pointed to by the lpAppName   
		parameter, it's created. If this parameter is NULL, the ertire section,   
		including all keys within the section, is deleted. When deleting a   
		section, leave the comments intact.  
		lpString[in]  
		pointer to a null-terminated string to be written to the file.   
		If this parameter is NULL, the key pointed to by the lpKeyName   
		parameter is deleted.  
		lpFileName[in]  
		Pointer to a null-terminated string that names the initialization file.   
**返回：  
		FALSE - fail  
		TRUE  - success  
**作者：YRC  
**日期：09.10.20
**备注：  
		1). 先将要修改的INI文件的全部数据读取到全局内存g_pData中  
		2). 在g_pData中定位到我们要修改的位置，将其它数据和我们修改的数据写入一临时ini文件  
		3). 最后将临时ini文件覆盖原来的ini文件，再删除临时ini文件  
		4). 主要的API函数：  
		creat、lseek、close、remove、open、mmap 
		5). 如果lpKeyName == NULL, 删除整个段, 如果lpString == NULL, 删除健  
************************************************************************/   
BOOL WritePrivateProfileString(   
							   LPCSTR lpAppName,   
							   LPCSTR lpKeyName,   
							   LPCSTR lpString,   
							   LPCSTR lpFileName)   
{   
	AutoLock	csLock(&gcsLock);

    DWORD dwSize, dwOffset;                // dwSize - ini文件大小, dwOffset - 偏移量    
    BOOL  bReadLine = TRUE;   
    BOOL  bWrote = FALSE;   
    char pszLine[MAX_PATH] = {0};         // 存储一行的数据    
    char pszIniFileTemp[MAX_PATH] = {0};  // 临时ini文件的名称(包括路径)    
    INT  fdOutputFile ;   
    LPSTR pValue;   
    DWORD dwValLen;     
	
    dwOffset = 0;   
    if (!lpFileName)   
        return FALSE;   
	
    // 读取INI文件内容到全局变量g_pData内存中
    dwSize = ReadIniFile(lpFileName);   
	
    //RETAILMSG(1, (TEXT("lpFileName=[%s], dwSize=[%d]"), lpFileName, dwSize));    
	
    // Create the output file.    
    //wcscpy(pszIniFileTemp, lpFileName);    
	
    sprintf(pszIniFileTemp, "%s.ini.tmp",lpAppName);   
//	printf("pszIniFileTemp = %s\n", pszIniFileTemp);
    
    fdOutputFile = creat(pszIniFileTemp, S_IRWXU| S_IRWXO |S_IRWXG);
	
    if (-1 == fdOutputFile)   
    {   
//        perror("Could not open output file");   
        return FALSE;   
    }   
	
    // 将所有数据写入临时ini文件    
    for (;;)   
    {   
        // The bReadLine flag is used to not read a new line after we break    
        // out of the inner loop. We've already got a line to process.    
        if (bReadLine)   
        {   
            dwOffset = GetLine( pszLine , dwOffset , dwSize );   
            if (!dwOffset)     
			{
				//printf("dwOffset = %d \n", dwOffset);
                break;
			}   
        }   
        bReadLine = TRUE;   
        // Skip past comments.    
        if (IsComment(pszLine))   
        {   
            WriteLine(fdOutputFile , pszLine);   
            continue;   
        }   
        // Found a section name.    
        if (IsSection(pszLine))   
        {   
            // It's the section we want.    
            if (IsSectionName(pszLine , lpAppName))   
            {   
                // 以下如果lpKeyName为NULL，删除整个段    
                if (lpKeyName)   
                    WriteLine(fdOutputFile , pszLine);   
				
                // Process the whole section.    
                while (0 != (dwOffset = GetLine( pszLine , dwOffset , dwSize )))   
                {   
                    // Reached the end of the section.    
                    if (IsSection(pszLine))   
                    {   
                        bReadLine = FALSE;   
                        // This line will be written in the outer loop.    
                        break;   
                    }   
                    // When deleting a section, leave the comments intact.    
                    else if (IsComment(pszLine))   
                    {   
                        WriteLine(fdOutputFile , pszLine);   
                        continue;   
                    }   
                    // Got the value we want.    
                    if (!bWrote && IsKey(pszLine , lpKeyName, &pValue, &dwValLen))   
                    {   
                        bWrote = TRUE;   
                        // 如果lpString为NULL，删除健lpKeyName    
                        if(lpString)   
                            WriteValue(fdOutputFile , NULL, lpKeyName, lpString);   
                    }   
                    else   
                    {   
                        if (lpKeyName)   
                            WriteLine(fdOutputFile , pszLine);   
                    }   
					
                    if(dwOffset >= dwSize)   
                        break ;   
                }   
				
                // 如果在段名lpAppName下键名lpKeyName不存在，则新建键名lpKeyName和键值lpString    
                if (!bWrote)   
                {   
                    bWrote = TRUE;   
                    WriteValue(fdOutputFile, NULL, lpKeyName, lpString);   
                }   
            }   
            else   
                WriteLine(fdOutputFile , pszLine);   
        }   
        else   
            WriteLine(fdOutputFile , pszLine);   
		
        if(dwOffset ==0)    
            break;   
    }   
	
    // 如果指定的段名lpAppName不存在，则新建段名lpAppName及键名lpKeyName和键值lpString    
    if (!bWrote && lpKeyName && lpString)   
    {   
//		printf("create key! lpAppName = %s, lpKeyName = %s , lpString =%s \n",lpAppName,lpKeyName,lpString);
        WriteValue(fdOutputFile , lpAppName, lpKeyName, lpString);   
    }   
	
    // 用临时ini文件覆盖原来的ini文件并删除临时ini文件    
    if (-1 != fdOutputFile)   
    {   
        lseek(fdOutputFile, 0, SEEK_END);   
        close(fdOutputFile);   
        CopyFile(pszIniFileTemp, lpFileName, FALSE);   
        if(remove(pszIniFileTemp) !=0)
		{
			perror("remove file fail ");
		}
		
    }   
	
    // 释放ReadIniFile函数的全局内存    
    if (NULL != g_pData)   
    {   
        free( g_pData ) ;   
        g_pData = NULL ;   
    }   
	
    return TRUE;   
}   

/************************************************************************  
**函数：WritePrivateProfileInt  
**功能：LINUX环境下，向指定INI文件指定段名写入整型数据  
**参数：参考WritePrivateProfileString函数  
**返回：  
		FALSE - fail  
		TRUE  - success  
**作者：YRC  
**日期：09.10.20
**备注：  
************************************************************************/   
BOOL WritePrivateProfileInt(   
							LPCSTR lpAppName,   
							LPCSTR lpKeyName,   
							INT     Value,   
							LPCSTR lpFileName)   
{      
    char ValBuf[16]={0};       
	
    sprintf( ValBuf, "%d", Value);       
    return( WritePrivateProfileString(lpAppName, lpKeyName, ValBuf, lpFileName) );    
}   

/************************************************************************  
**函数：WriteValue  
**功能：向指定INI文件中写入段名、键名和键值  
**参数：  
		m_hOutput[in]  
		pointer to the handle of ini file.  
		pAppName[in]  
		Pointer to a null-terminated string containing the name of the section  
		in which data is written. If this parameter is NULL, the WriteValue  
		function just wirte the pKeyName and pString.  
		pKeyName[in]  
		Pointer to a null-terminated string containing the name of the key in  
		which data is writtern.   
		pString[in]  
		Pointer to a null-terminated string to be written to the file.   
**返回：NONE  
**作者：YRC  
**日期：09.10.20 
**备注：  
		1). 要成功写入INI文件，键名pKeyName和键值pString都不能为NULL。  
		2). 如果段名pAppName为NULL，则只写入键名pKeyName和键值pString。  
		3). 注意往INI文件写入字符串时，不要写入结束符。  
************************************************************************/   
void WriteValue(INT fdOutput, LPCSTR pAppName, LPCSTR pKeyName, LPCSTR pString)   
{   
 
    if (pKeyName && pString)   
    {   
        if (pAppName)       // 写入段名    
        {   
			write(fdOutput, "[", sizeof(char));   
			write(fdOutput, pAppName, strlen(pAppName)*sizeof(char));   
			write(fdOutput, "]\r\n", 3*sizeof(char));   									
        }   
		// 写入健名和键值   
		write(fdOutput, pKeyName, strlen(pKeyName)*sizeof(char));   
		write(fdOutput, "=", sizeof(char));   
		write(fdOutput, pString, strlen(pString)*sizeof(char));   
		write(fdOutput, "\r\n", 2*sizeof(char));  
    }   
}