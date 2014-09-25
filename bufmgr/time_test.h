// time_test.h: interface for the time_test class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIME_TEST_H__EE6AF6BD_F427_4A70_998B_474513F9FBF2__INCLUDED_)
#define AFX_TIME_TEST_H__EE6AF6BD_F427_4A70_998B_474513F9FBF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "windows.h"
//#include "Mmsystem.h"

class time_test  
{
	//clock_t be;
	long be;
public:
	void time_end();
	void time_begin();
//	double result;
	long result;
	time_test();
	virtual ~time_test();

};

#endif // !defined(AFX_TIME_TEST_H__EE6AF6BD_F427_4A70_998B_474513F9FBF2__INCLUDED_)
