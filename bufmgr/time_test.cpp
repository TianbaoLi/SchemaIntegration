// time_test.cpp: implementation of the time_test class.
//
//////////////////////////////////////////////////////////////////////

#include "time_test.h"
#include "windows.h"
#include "Mmsystem.h"
#pragma comment(lib,"winmm.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

time_test::time_test()
{
	be=0;

}

time_test::~time_test()
{

}

void time_test::time_begin()
{
//	be=clock();
	be=timeGetTime();
}

void time_test::time_end()
{
	long t=timeGetTime();
	long re=t-be;
	result=re;///CLOCKS_PER_SEC;
	//printf("%ld %ld\n",t, be);	
}