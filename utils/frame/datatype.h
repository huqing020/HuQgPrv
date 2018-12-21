#ifndef __DATA_TYPE_LINX_H
#define __DATA_TYPE_LINX_H

#ifdef _MSC_VER
/// windows
#include <Windows.h>

#ifndef INT64
typedef unsigned __int64 UINT64;
typedef __int64 INT64;
#endif

#else
/// other os
#ifndef MAX_PATH
#define MAX_PATH	(260)
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef DWORD
typedef unsigned int DWORD;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef INT64
typedef unsigned long long UINT64;
typedef long long INT64;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef LOWORD
#define LOWORD(l)           ((WORD)(((DWORD)(l)) & 0xffff))
#endif

#ifndef HIWORD
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xffff))
#endif

#ifndef LPCSTR
typedef const char *LPCSTR;
#endif

#ifndef SOCKET
typedef int SOCKET;
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET	(SOCKET)(~0)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef CRITICAL_SECTION
#define CRITICAL_SECTION pthread_mutex_t
#endif

#define closesocket(fd) do \
	{ \
		close(fd); \
	} \
	while( 0 )

#define	ZeroMemory(Destination,Length) do \
	{ \
		memset((Destination),0,(Length));\
	} \
	while(0)

#define InitializeCriticalSection(csLock)	do \
	{ \
		pthread_mutexattr_t mattr;	\
		pthread_mutexattr_init(&mattr);	\
		pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);	\
		pthread_mutexattr_settype(&mattr,PTHREAD_MUTEX_RECURSIVE );	\
		pthread_mutex_init((csLock), &mattr);\
	} \
	while (0)

#define DeleteCriticalSection(csLock)	do \
	{ \
		pthread_mutex_destroy((csLock));	\
	} \
	while (0)

#define EnterCriticalSection(csLock)	do \
	{ \
		pthread_mutex_lock((csLock));	\
	} \
	while (0)

#define LeaveCriticalSection(csLock)	do	\
	{ \
		pthread_mutex_unlock((csLock));	\
	} \
	while (0)

#ifndef BOOL
typedef int	BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif /// _MSC_VER

#endif	/// __DATA_TYPE_LINX_H