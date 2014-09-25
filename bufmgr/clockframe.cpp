//#include "stdafx.h"

#include "buf_common.h"
#include "db.h"
#include "clockframe.h"

//--------------------------------------------------------------------
// ClockFrame class
//
// A subclass of Frame.  Contain an extra reference bit for the use
// of Clock Replacement Policy.
//--------------------------------------------------------------------


ClockFrame::ClockFrame(Page* pg, DB* use_db) 
: Frame(pg, use_db)
{
	referenced = FALSE;
}


ClockFrame::~ClockFrame()
{

}

void ClockFrame::Unpin()
{
	Frame::Unpin();
	if (NotPinned())
		referenced = TRUE;
}

/*
BOOL ClockFrame::IsVictim()
{
	return ( !referenced && NotPinned());
}
*/

/*
BOOL ClockFrame::IsReferenced()
{
	return (referenced == (int)TRUE);
}
*/


Status ClockFrame::Free()
{
	if (Frame::Free() != OK)
		return FAIL;

	referenced = FALSE;
	return OK;
}

void ClockFrame::UnsetReferenced()
{
	referenced = FALSE;
}