//#include "stdafx.h"

#include "stdio.h"
#include "buf_common.h"
#include "db.h"
#include "clockframe.h"
#include "replacer.h"

//------------------------------------------------------
// Replacer class
//
// is a virtual class that does nothing.
//------------------------------------------------------
 
Replacer::Replacer()
{

}

Replacer::~Replacer()
{

}


//------------------------------------------------------
// Clock class
//
// is a subclass of Replacer.  The main function is
// PickVictim(), which execute the Clock Replacement
// algorithm.
//------------------------------------------------------


Clock::Clock(int bufSize, ClockFrame **bufFrames, HashTable *table)
{
	m_current = 0;
	numOfBuf = bufSize;
	frames = bufFrames;
	hashTable = table;
}


Clock::~Clock()
{

}
// bWriteBack is specially added to have write statistics
// 
int Clock::PickVictim(BOOL & bWriteBack)
{
	int numOfTest = 0;

	bWriteBack = FALSE;
	
	int frameNo = INVALID_FRAME;

	while(numOfTest != 2*numOfBuf)
	{
		if(frames[m_current]->IsVictim())
		{
			if(frames[m_current]->IsValid())
			{
				
//				printf(">>kick out %d %d\n",m_current, frames[m_current]->GetPageID());
				// If the page is not empty, write the frame
				// out and remove the (pid, frameNo) pair
				// from the hashTable.
				hashTable->Delete(frames[m_current]->GetPageID());

				// 
				if( frames[m_current]->IsDirty() )
				{
					bWriteBack = TRUE;
					frames[m_current]->Write();
				}

				frames[m_current]->EmptyIt();
			}
			// ][ Debugged on Feb. 12, 2003.
			// Before return, we should move forward the clock pointer.
			// Otherwise, the same point may be 
			frameNo = m_current;
			
			m_current = (m_current+1) % numOfBuf;

			break;
		}
		else if (frames[m_current]->IsReferenced())
		{
			frames[m_current]->UnsetReferenced();
		}
		else
		{
			// This frame must be pinned!
//			ASSERT( !frames[m_current]->NotPinned() );

			m_current = (m_current+1) % numOfBuf;
		}
		numOfTest++;
	}

	// if reach here then no free buffer;

	return frameNo;
}