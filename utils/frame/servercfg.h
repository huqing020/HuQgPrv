/*
 * @Author: huqing 
 * @Date: 2018-11-23 10:13:02 
 * @Last Modified by: huqing
 * @Last Modified time: 2018-11-23 10:16:14
 */

#ifndef __SVR_CFG_HEAD
#define __SVR_CFG_HEAD

struct tgSvrCfg{
    char    szIP[48];
    int     iPort;
    char    szUser[128];
    char    szPwd[256];
};

#endif  /// __SVR_CFG_HEAD