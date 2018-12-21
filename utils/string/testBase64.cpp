#include <stdio.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include "base64.h"

void testBase64_1();
void testBase64_2();

int main()
{
//    testBase64_1();
    testBase64_2();
    return 0;
}

void testBase64_1()
{
    std::string str = "DHD($C$5,$D$7:$F$7,\"LTD\",\"\",\"CDR=ALL\",\"PER=A\",\"COM=H\",\"ADJ=N\",\"PTYPE=NET\",\"UNIT=ONE\",\"DIR=V\",\"SORT=DESC\",\"ROW=1145,COL=4\")";
    std::string strRes = encodeBase64(str);
    std::cout << "str = [" << str << "], strRes = [" << strRes << "]" << std::endl;

    bool bRes = decodeBase64(strRes, str);
    if (!bRes)
        return;

    std::cout << "decode res = [" << str << "]" << std::endl;
}

void testBase64_2()
{
    std::string str = "REhEKCRDJDUsJEQkNzokRiQ3LCJMVEQiLCIiLCJDRFI9QUxMIiwiUEVSPUEiLCJDT009SCIsIkFESj1OIiwiUFRZUEU9TkVUIiwiVU5JVD1PTkUiLCJESVI9ViIsIlNPUlQ9REVTQyIsIlJPVz0xMTQ1LENPTD00Iik";
    std::string strRes = "";

    bool bRes = decodeBase64(str, strRes);
    if (!bRes)
        return;

    std::cout << "decode res = [" << strRes << "]" << std::endl;
}