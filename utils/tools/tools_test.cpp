/*
 * @Author: huqing 
 * @Date: 2019-01-04 14:54:06 
 * @Last Modified by: huqing
 * @Last Modified time: 2019-01-04 16:28:09
 */

#include <stdio.h>
#include <time.h>
#include "_tools.h"


template<typename t>
void printthings(t &val){
    std::cout << val << std::endl;
}


void testVarToStr();
void testGetGlobalSeq();
void testGetExedir();
void testgetSpecialDT();

int main()
{
   testVarToStr();
   testGetGlobalSeq();
   testGetExedir();
testgetSpecialDT();
    return 0;
}

void testVarToStr()
{
    int ival = 100;
    auto res = VarToStr(ival);
    printthings(res);
}

void testGetGlobalSeq()
{
    auto res = getGlobalSEQ();
    printthings(res);
}

void testGetExedir()
{
    auto res = getExeDir();
    printthings(res);
}

void testgetSpecialDT()
{
    auto res = getSpecialDT(time(NULL));
    printthings(res);
}