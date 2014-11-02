//#include "stdafx.h"

/*#include <system_defs.h>
#include <db.h>
#include <buf_frame.h>
*/
/*#ifdef _DEBUG
#define new DEBUG_NEW
#endif*/
#include "stdlib.h"
#include "stdio.h"

#include "buf_common.h"
#include "db.h"
#include "buf_frame.h"
//using namespace std;

#define INVALID_PAGE -1
//--------------------------------------------------------------------
// Frame class
//
// A frame maintains the following information :
// - a pinCount to indicate how many times the page is pinned
// - a dirty bit to indicate if the page is dirty or not.
// - data which contains the actual page 
// - pid which is the page id of the page that resides in the frame.
//
// Some of the function here should be defined as inline.  But were not 
// to make the code easier to read.  Most functions are self explainatory.
//--------------------------------------------------------------------

//   Note that m_nUnPinnedFrames + m_nInvalidFrames
// is equal to # total frames available for replacement.
//   totalFrame - m_nUnPinnedFrames + m_nUnusedFrames == pinnedFrame

// INVALID_PAGE (assert: pin_count==0)
int Frame::m_nInvalidFrames = 0;
// Valid Pid && pin_count == 0
int Frame::m_nUnPinnedFrames = 0; // valid but not pinned

Frame::Frame(Page * pg, DB* use_db)
{
	m_curPid = INVALID_PAGE;   // pid == INVALID_PAGE <=> empty frame
	m_data = pg; // new Page();
	m_pinCount = 0;
	m_dirty = FALSE;
	EXDB_DB = use_db;
	
	// Initially, all frames are not used. 
	m_nInvalidFrames++;
}


Frame::~Frame()
{
	// We do not count frames that do not exist :-)
	// Thus, if the frame is currently counted as invalid, de-count it
	if( !IsValid() )
	{
		//ASSERT( m_pinCount == 0 );
		// it is currently counted
		// de-count it
		m_nInvalidFrames--;
	}
	else if( NotPinned() )
	{
		m_nUnPinnedFrames--;
	}
	else
	{
		//ASSERT( FALSE );
	}
}


void Frame::Pin()
{
	//ASSERT( IsValid() );

	if( m_pinCount == 0 )
	{
		m_nUnPinnedFrames--;
	}

	m_pinCount++;
	//ASSERT( m_pinCount > 0 );
}


void Frame::Unpin()
{
	//ASSERT( IsValid() );

	m_pinCount--;
	//ASSERT( m_pinCount >=0 );

	if( m_pinCount == 0 )
	{
		m_nUnPinnedFrames++;
	}
}


/*
BOOL Frame::NotPinned() const
{
	return m_pinCount == 0;
}
*/


BOOL Frame::IsValid() const
{
	return m_curPid != INVALID_PAGE;
}

// 
// Frame::Write count
Status Frame::Write()
{
	Status st;

	if (m_dirty)
	{
		EXDB_DB->write_page(m_curPid, m_data);
		//ASSERT(st == OK);
		m_dirty = FALSE;
		return FALSE;
	}
	else
	{
		return OK;
	}
}

void Frame::DirtyIt()
{
	m_dirty = TRUE;
}


BOOL Frame::IsDirty() const
{
	return m_dirty;
}


void Frame::EmptyIt()
{
	//ASSERT( m_pinCount == 0 );
	//ASSERT( !m_dirty );

	SetPageID( INVALID_PAGE );

/*	if( m_pinCount )
	{
		m_nUnPinnedFrames++;
	}

	m_pinCount = 0;
	m_dirty = FALSE;
*/
}

Status Frame::Read(PageID pageid)
{
	SetPageID(pageid);
	EXDB_DB->read_page(m_curPid, m_data);
	return 1;
}


void Frame::SetPageID(PageID pageid)
{
	if( m_curPid == INVALID_PAGE )
	{
		//ASSERT( m_pinCount == 0 );

		// current not used
		if( pageid != INVALID_PAGE )
		{
			// now changed
			m_nInvalidFrames--;
			m_nUnPinnedFrames++;
		}
	}
	else
	{
		// not counted
		if( pageid == INVALID_PAGE )
		{
			//ASSERT( m_pinCount == 0 );
			m_nInvalidFrames++;
			m_nUnPinnedFrames--;
		}
	}
	m_curPid = pageid;
}


PageID Frame::GetPageID() const
{
	return m_curPid;
}

// Free the disk page associated with the buffer page
// The pin count should be no more than 1
// 
Status Frame::Free()
{
	Status st;
	
	//ASSERT( IsValid() );

	if (m_pinCount > 1)
	{
		printf( "   Free a page that is pinned more than once.\n");
		//cerr << "   Free a page that is pinned more than once.\n";
		return FAIL;
	}
	else
	{
		if (m_pinCount == 1)
		{
			Unpin();
		}
		
		// Since we are deallocating it, 
		// we do not need to write back anyway
		m_dirty = FALSE;  // cheat the EmptyIt method

		//][!	The orginal code has a "big" bug here. 
		/*		A typical scenario is as follows:
					BufMgr::FreePage()
						Frame::Free()	//<== here
							DB::DeallocatePage()
							Frame::EmptyIt()
						hashTable::Delete()

				Notice that DeallocatePage will bring space map pages into 
				buffer. When the buffer pages are in contention, the current 
				frame will be used to load space map pages (say Page 5) (and 
				the hash table now contain (5, thisframeno)). However, this 
				frame's page id is cleared to INVALID_PAGE by EmptyIt(). And 
				the later call of hashTabe::Delete() will remove 
				(oldpageid, thisframeno) mapping. Therefore, the 
				(5, thisframeno) is still in the hashtable, but the frameno 
				now contains other page.
		*/
		/*
		s = EXDB_DB->DeallocatePage(m_pid);
		//ASSERT( s == OK );
		EmptyIt();
		*/

		PageID		nOldPageID;
		
		nOldPageID = m_curPid;
		EmptyIt();

//		st = EXDB_DB->DeallocatePage(nOldPageID);
		//ASSERT( st == OK );
	}
	return st;
}


BOOL Frame::HasPageID( PageID id ) const
{
	return (m_curPid == id);
}


Page *Frame::GetPage()
{
	return m_data;
}

int Frame::GetPinCount() const
{
	return m_pinCount;
}
