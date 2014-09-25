//#include "stdafx.h"

#include <map>
//#include <fstream>

#include "buf_common.h"
#include "db.h"
#include "buf_frame.h"
#include "clockframe.h"
#include "hash.h"
#include <math.h>
#include "replacer.h"
#include "stdio.h"
#include "LRUframe.h"
#include "LRU.h"
#include "bufmgr.h"

#define INVALID_PAGE -1

using namespace std;

// map<PageID, int> pid2miss;
//--------------------------------------------------------------------
// Constructor for BufMgr
//
// Input   : bufSize  - number of pages in the this buffer manager
// Output  : None
// PostCond: All frames are empty.
//--------------------------------------------------------------------

enum bufErrCodes {
    BUF_FULL,
};


static const char* bufErrMsgs[] = {
    "Buffer is full",         // BUF_FULL
};

//static error_string_table bufTable( BUFMGR, bufErrMsgs );

// 
// 
BufMgr::BufMgr(const char* filename, int numOfBuf, int file_size, int in_page_size, DWORD BytesPerSector)
{
	

	EXDB_DB = new DB(filename,file_size, in_page_size);
	m_numOfBuf = numOfBuf;
	m_BytesPerSector = BytesPerSector;
	page_size = in_page_size;

	m_nTotalPin = 0;
	m_nTotalMiss = 0;

	int status = initBufMgr();	
}


//--------------------------------------------------------------------
// Destructor for BufMgr
//
// Input   : None
// Output  : None
//--------------------------------------------------------------------

BufMgr::~BufMgr()
{   
	Status status;
	status = FlushAllPages(FALSE);
//	ASSERT( status == OK );

	for (int i = 0; i < m_numOfBuf; i++)
		delete frames[i];
	delete []frames;
	delete replacer;
	delete hashTable;
	delete []m_pBufStartAddr;
}


//--------------------------------------------------------------------
// BufMgr::FlushAllPages
//
// Input    : bAllowPinned - whether to allow existing pinned frames
//			  bFlushPinned - if there is pinned page, whether to write it back to disk
// Output   : None
// Purpose  : Flush all pages in this buffer pool to disk.
//			  After the call, all non-pinned frames are empty
// Condition: All pages in the buffer pool must not be pinned.
// PostCond : All dirty pages in the buffer pool are written to 
//            disk (even if some pages are pinned). All frames are empty.
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------

Status BufMgr::FlushAllPages(BOOL bAllowPinned, BOOL bFlushPinnedDirty)
{
	int i;
	int goingToFail;

	goingToFail = FALSE;
	
	int pinned_buffers = 0;

	if( !bAllowPinned )
	{
		for (i = 0; i < m_numOfBuf; i++)
		{
			if (frames[i]->IsValid())
			{
//				ASSERT( frames[i]->NotPinned() );
				
				if( frames[i]->IsDirty() )
				{
					m_nTotalWrite ++;
					frames[i]->Write();
				}

				//][    Added by fervvec.
				//Since it empties the hashtable later, the pages cannot be used.
				//So it MUST be set to invalid. Or the replacer will behave wrong by
				//deleting an entry from the hashtable when picking up a victim.
				frames[i]->EmptyIt();
				//][    End of Modification
			}
		}
		hashTable->EmptyIt();

//		ASSERT( !goingToFail);
	}
	else
	{
		for (i = 0; i < m_numOfBuf; i++)
		{
			if (frames[i]->IsValid())
			{
				if(frames[i]->NotPinned())
				{
					if( frames[i]->IsDirty() )
					{
						m_nTotalWrite ++;
						frames[i]->Write();
					}

					hashTable->Delete(frames[i]->GetPageID());
					//][    Added by fervvec.
					//Since it empties the hashtable later, the pages cannot be used.
					//So it MUST be set to invalid. Or the replacer will behave wrong by
					//deleting an entry from the hashtable when picking up a victim.
					frames[i]->EmptyIt();
					//][    End of Modification
				}
				else
				{
					// The page is pinned
					pinned_buffers++;

					// the frame is pinned
					if( bFlushPinnedDirty )
					{
						if( frames[i]->IsDirty() )
						{
							m_nTotalWrite ++;
							frames[i]->Write();
						}
					}
				}
			}
		}
	}

//	ASSERT( pinned_buffers + Frame::m_nInvalidFrames + Frame::m_nUnPinnedFrames 
//		== m_numOfBuf);

	return 1;
}


//--------------------------------------------------------------------
// BufMgr::FlushPages
//
// Input    : pid  - page id of a particular page 
// Output   : None
// Purpose  : Flush the page with the given pid to disk.
// Condition: The page with page id = pid must be in the buffer,
//            and is not pinned. pid cannot be INVALID_PAGE.
// PostCond : The page with page id = pid is written to disk if it's dirty. 
//            The frame where the page resides is empty.
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------


Status BufMgr::FlushPage(PageID pid, BOOL bAllowPinned, BOOL bFlushPinned)
{
	int frameNo;

	frameNo = FindFrame(pid);
	if (frameNo == INVALID_PAGE)
	{
		printf("Error : Unable to find the page with page id %d\n",pid);
		return 0;
	}
	
	if (frames[frameNo]->NotPinned())
	{
		if( frames[frameNo]->IsDirty() )
		{
			m_nTotalWrite ++;
			frames[frameNo]->Write();
		}
		// ][ Added by ryan
		//Since it empties the hashtable later, the pages cannot be used.
        //So it MUST be set to invalid. Or the replacer will behave wrong by
        //deleting an entry from the hashtable when picking up a victim.
		frames[frameNo]->EmptyIt(); 
		return hashTable->Delete(pid);
	}
	else
	{
		if( !bAllowPinned )
		{
			printf("class BufMgr: error flushing a pinned page.\n");
//			Beep(1000, 200);
//			cerr << "class BufMgr: error flushing a pinned page "<<endl;
//			ASSERT( FALSE ); //by ryan
			return 0;
		}
		// check whether to write the page
		if( bFlushPinned )
		{
			if( frames[frameNo]->IsDirty() )
			{
				m_nTotalWrite ++;
				frames[frameNo]->Write();
			}
		}
	}
	return 1;
} 

//--------------------------------------------------------------------
// BufMgr::PinPage
//
// Input    : pid     - page id of a particular page 
//            isEmpty - (optional, default to FALSE) if true indicate
//                      that the page to be pinned is an empty page.
// Output   : page - a pointer to a page in the buffer pool. 
//			  (not touched if fail)
// Purpose  : Pin the page with page id = pid to the buffer.  
//            Read the page from disk unless isEmpty is TRUE or unless
//            the page is already in the buffer.
// Condition: Either the page is already in the buffer, or there is at
//            least one frame available in the buffer pool for the 
//            page.
// PostCond : The page with page id = pid resides in the buffer and 
//            is pinned. The number of pin on the page increase by
//            one.
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------

Status BufMgr::PinPage(PageID pid, Page*& page, BOOL isEmpty)
{
//    hashTable->DumpHash("hash.txt");
	
	int frameNo;

	m_nTotalPin++;
	
	frameNo = FindFrame(pid);
	// 
	if (frameNo == INVALID_FRAME)
	{
		// Page not in buffer.
		// 
		BOOL bWriteBack;
		frameNo = replacer->PickVictim(bWriteBack);
		if( bWriteBack ) { m_nTotalWrite++; }

		if (frameNo == INVALID_FRAME)
		{
			
			// We always assume that the buffer will not be full.
			// Simply stop the program if the buffer gets full.
			// 
			printf("ERROR: Buffer pool is full");
//			cerr << "ERROR: Buffer pool is full" <<endl;
//			cerr << "Program will exit"<<endl;
//			Beep(2000, 500);
			exit(1);

			return 2;
		}
		if (!isEmpty)
		{
			// Not empty. Read it in from Disk.
			frames[frameNo]->Read(pid);

			m_nTotalMiss++;

			if( m_LastMissPid + 1 != pid )
			{
				m_nTotalRandMiss++;
			}

			m_LastMissPid = pid;
		}
		else
		{
			frames[frameNo]->SetPageID(pid);
		}
		hashTable->Insert(pid, frameNo);
	}
	else 
	{
//		ASSERT( frames[frameNo]->GetPageID() == pid );
	}
	frames[frameNo]->Pin();
	page = frames[frameNo]->GetPage();   
	
//	printf("%d -> %d\n",pid, frameNo);

	return 1;
} 

//--------------------------------------------------------------------
// BufMgr::UnpinPage
//
// Input    : pid     - page id of a particular page 
//            dirty   - indicate whether the page with page id = pid
//                      is dirty or not. (Optional, default to FALSE)
// Output   : None
// Purpose  : Unpin the page with page id = pid in the buffer. Mark 
//            the page dirty if dirty is TRUE.  
// Condition: The page is already in the buffer and is pinned.
// PostCond : The page is unpinned and the number of pin on the
//            page decrease by one. 
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------


Status BufMgr::UnpinPage(PageID pid, BOOL dirty)
{
	int frameNo;

	frameNo = FindFrame(pid);

	if (frameNo == INVALID_FRAME)
	{
		printf("Page %d is not in the buffer\n",pid);
//		cerr << "   Page " << pid << " is not in the buffer\n";

//		ASSERT( FALSE );

		return BUFMGR;
	}
	
//	ASSERT( frames[frameNo]->GetPageID() == pid );
	if (frames[frameNo]->NotPinned())
	{
		
		printf(" Trying to unpin page %d which is not pinned\n", pid);
//		cerr << "   " << pid << ", which is not pinned.\n";

//		ASSERT( FALSE );

		return BUFMGR;
	}

	if (dirty)
		frames[frameNo]->DirtyIt();

	frames[frameNo]->Unpin();
    return OK;
}

//--------------------------------------------------------------------
// BufMgr::FreePage
//
// Input    : pid     - page id of a particular page 
// Output   : None
// Purpose  : Free the memory allocated for the page with 
//            page id = pid  
// Condition: Either the page is already in the buffer and is pinned
//            no more than once, or the page is not in the buffer.
// PostCond : The page is unpinned, and the frame where it resides in
//            the buffer pool is freed.  Also the page is deallocated
//            from the database. 
// Return   : OK if operation is successful.  FAIL otherwise.
// Note     : You can call EXDB_DB->DeallocatePage(pid) to
//            deallocate a page.
//--------------------------------------------------------------------

Status BufMgr::FreePage(PageID pid)
{
	int frameNo;
	Status s;
	
//	cout<<"@ Free Page "<<pid<<endl; 

	frameNo = FindFrame(pid);
	if (frameNo == INVALID_FRAME)
	{
		//return EXDB_DB->DeallocatePage(pid);
	}
	else
	{	
		//][!	Modified by fervvac.
		//	Due to the bug in the original code:
		//	The current protocol is to remove any trace in the buffer
		//	manager about the page (to be abandoned), and call DB::DeAllocatePage()
		//	in the last place to ensure the correctness.
		/*
		s = frames[frameNo]->Free();
		ASSERT (s == OK);
		hashTable->Delete(pid);
		*/
		hashTable->Delete(pid);
		s = frames[frameNo]->Free();
//		ASSERT (s == OK);
		//][	End
		
		return s;
	}
}

//--------------------------------------------------------------------
// BufMgr::NewPage
//
// Input    : howMany - (optional, default to 1) how many pages to 
//                      allocate.
// Output   : pid  - the page id of the first page (as output by
//                   DB::AllocatePage) allocated.
//            page - a pointer to the page in memory.
// Purpose  : Allocate howMany number of pages, and pin the first page
//            into the buffer. 
// Condition: howMany > 0 and there is at least one free buffer space
//            to hold a page.
// PostCond : The page with page id = pid is pinned into the buffer.
// Return   : OK if operation is successful.  FAIL otherwise.
// Note     : You can call DB::AllocatePage() to allocate a page.  
//            You should call DB:DeallocatePage() to deallocate the
//            pages you allocate if you failed to pin the page in the
//            buffer.
//--------------------------------------------------------------------


Status BufMgr::NewPage (PageID& pid, Page*& page, int howMany)
{
//	Status s;
	
	// allocating new pages does not incur disk I/O
	// because whole space map is pinned into buffers

/* 	s = EXDB_DB->AllocatePage(pid, howMany);

	if (s != OK)
	{
		cerr << "  BufMgr :: Unable to allocate " << howMany << " pages\n";
		return FAIL;
	}*/

	return PinPage(pid, page, TRUE) ;
}

//--------------------------------------------------------------------
// BufMgr::WriteBack()
//
// Input    : None
// Output   : None
// Purpose	: Write dirty pages to disk and undirty the buffer pages
// Return   : OK
//--------------------------------------------------------------------

Status BufMgr::WriteBack()
{
	int i;

	for (i = 0; i < m_numOfBuf; i++)
	{
		if (frames[i]->IsValid() && 
			frames[i]->IsDirty() )
		{	
			m_nTotalWrite ++;
			frames[i]->Write();
		}
	}

	return OK;
}

//--------------------------------------------------------------------
// BufMgr::GetNumOfUnpinnedBuffers
//
// Input    : None
// Output   : None
// Purpose  : Find out how many unpinned locations are in the buffer
//            pool, excluding invalid frames
// Return   : The number of unpinned buffers in the buffer pool.
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// BufMgr::GetNumOfBuffers
//
// Input    : None
// Output   : None
// Purpose  : Find out how many buffers are there in the buffer pool.
// PreCond  : None
// PostCond : None
// Return   : The number of buffers in the buffer pool.
//--------------------------------------------------------------------

int BufMgr::GetNumOfBuffers() const
{
    return m_numOfBuf;
}


//--------------------------------------------------------------------
// BufMgr::FindFrame
//
// Input    : pid - a page id 
// Output   : None
// Purpose  : Look for the page in the buffer pool, return the frame
//            number if found.
// PreCond  : None
// PostCond : None
// Return   : the frame number if found. INVALID_FRAME otherwise.
//--------------------------------------------------------------------

int BufMgr::FindFrame( PageID pid )
{
	return hashTable->LookUp(pid);	
}


void BufMgr::DumpBuf(const char *out_file)
{
/*	ofstream out_file_stream(out_file);
//	ostream *output = &ios;
	for(int i=0; i<m_numOfBuf;i++)
	{
		if(frames[i]->IsValid())
		{
			out_file_stream<<"frame "<<i<<": ";
			out_file_stream<<"pageID = "<<frames[i]->GetPageID();
			out_file_stream<<", pinCount = "<<frames[i]->GetPinCount()<<endl;
		}
	}*/
}

// BufMgr::ResetBufferSize
// Purpose	: reset the buffer pool sizes
// Note		: This function can only be called by SystemDefs
// 
Status BufMgr::ResetBufferSize(int bufSize)
{
	// there should be no pinned pages
	Status st = FlushAllPages(FALSE);
	if( st != OK )
	{
//		ASSERT( st == OK );
		return st; // for release version
	}

	//delete old frames
	for (int i = 0; i < m_numOfBuf; i++)
	{
		delete frames[i];
	}
	delete []frames;

	//delete hash table and replacer
	delete replacer;
	delete hashTable;
	delete []m_pBufStartAddr;

	//frames = new ClockFramePtr[bufSize];
	m_numOfBuf = bufSize;

	st = initBufMgr();
	
	if( st == OK )
	{
		ResetStat();
	}
	return st;
}

void BufMgr::GetStat(int & pinNo, int & missNo, int & writeno) const
{ 
	pinNo = m_nTotalPin; 
	missNo = m_nTotalMiss;
	writeno = m_nTotalWrite;
}

// all pointers can be NULL
// 
void BufMgr::GetStat(int * pinNo, int * missNo, int * randMissNo, int * writeNo) const
{
//	ASSERT( m_nTotalMiss >= m_nTotalRandMiss );

	if( pinNo )
	{
		*pinNo = m_nTotalPin;
	}
	if( missNo )
	{
		*missNo = m_nTotalMiss;
	}
	if( randMissNo )
	{
		*randMissNo = m_nTotalRandMiss;
	}
	if( writeNo )
	{
		*writeNo = m_nTotalWrite;
	}
}
void BufMgr::ResetStat()
{
	m_nTotalPin = 0; 
	m_nTotalMiss = 0; 
	m_nTotalRandMiss = 0;
	m_nTotalWrite = 0;
	m_LastMissPid = INVALID_PAGE;
}

Status BufMgr::initBufMgr()
{
	// allocate enough buffer space, make each buffer page sector-aligned
	DWORD mem_size = ( m_numOfBuf + 1 ) * page_size;

	m_pBufStartAddr = new char[mem_size];
	
	if( !m_pBufStartAddr )
	{
		return 0;
	}

	DWORD address = (DWORD) m_pBufStartAddr;
	DWORD ext = address % m_BytesPerSector;

	if(ext != 0)
	{
		ext = m_BytesPerSector - ext;
	}
	
	char * p = m_pBufStartAddr + ext;

#ifdef USE_LRU
	frames = new LRUFrame *[m_numOfBuf];
#else
	frames = new ClockFrame *[m_numOfBuf];
#endif
	for (int i = 0; i < m_numOfBuf; i++)
	{
		char * pCur = p + page_size * i;
#ifdef USE_LRU
		frames[i] = new LRUFrame( (Page*)pCur, EXDB_DB );
#else
		frames[i] = new ClockFrame( (Page*)pCur, EXDB_DB );
#endif
	}

	hashTable = new HashTable(m_numOfBuf);
#ifdef USE_LRU
	replacer = new LRU(m_numOfBuf, frames, hashTable);
#else
	replacer = new Clock( m_numOfBuf, frames, hashTable );
#endif

	return 1;
}

Status BufMgr::FlushUnpinnedPages()
{
	int i;
	Status s = OK;
	for (i = 0; s == OK && i < m_numOfBuf; i++)
	{
		if (frames[i]->IsValid())
		{
			if(frames[i]->NotPinned())
			{
				s = FlushPage(frames[i]->GetPageID());
			}
		}
	}
	return s;
}
/*
void BufMgr::OutputPidMissCount(std::ostream *output)
{
	map<PageID, int>::iterator it;
	for(it = pid2miss.begin(); it != pid2miss.end(); it++)
	{
		PageID pid = (*it).first;
		int count = (*it).second;
		(*output)<<pid<<"\t"<<count<<endl;
	}
}
*/

//--------------------------------------------------------------------
// BufMgr::GetNumOfUnusedFrames
//
// Input    : None
// Output   : None
// Purpose  : Find out how many invalid frames in the buffer pool
//--------------------------------------------------------------------
/*
int BufMgr::GetNumOfUnusedFrames() const
{
	return Frame::m_nInvalidFrames;
}
*/
int BufMgr::GetNumOfAvailableBuffers() const
{
	return Frame::m_nUnPinnedFrames + Frame::m_nInvalidFrames;
}

int BufMgr::getLogicalIO()
{
	return m_nTotalPin;
}

int BufMgr::getPhysicalIO()
{
	return m_nTotalMiss;
}

int BufMgr::getPagesize()
{
	return page_size;
}
