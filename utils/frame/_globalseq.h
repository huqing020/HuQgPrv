/*
 * @Author: huqing 
 * @Date: 2018-11-26 16:41:59 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-26 16:50:51
 */

#ifndef	__SEQ_GLOBAL_SEQ_HEAD
#define __SEQ_GLOBAL_SEQ_HEAD

#include <stdlib.h>
#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif /// _MSC_VER

static	unsigned int	sglobalSeq = 0;
//////////////////////////////////////////////////////////////////////////
/// 获取全局SEQ
unsigned int getGlobalSeq()
{
#ifdef _MSC_VER
	return InterlockedIncrement(&sglobalSeq);
#else
	return __sync_add_and_fetch(&sglobalSeq, 1);
#endif
}

#endif	/// __SEQ_GLOBAL_SEQ_HEAD