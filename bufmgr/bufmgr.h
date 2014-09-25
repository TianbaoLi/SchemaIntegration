
#ifndef _BUF_H_
#define _BUF_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "db.h"
#include "LRUframe.h"

//#define USE_LRU

class ClockFrame;
class HashTable;
class Replacer;

class BufMgr 
{
	public:
		int getPagesize();
		int getPhysicalIO();
		int getLogicalIO();
		int GetNumOfBuffers() const;
		Status FlushUnpinnedPages();
		void DumpBuf(const char *out_file);
		Status WriteBack();

		BufMgr(const char* filename, int numOfBuf, int file_size, int in_page_size, DWORD BytesPerSector);

		~BufMgr();      
		
		Status PinPage( PageID pid, Page*& page, BOOL emptyPage = FALSE );
		Status UnpinPage( PageID pid, BOOL dirty=FALSE );
		Status NewPage( PageID& pid, Page*& firstpage,int howmany=1 ); 
		Status FreePage( PageID pid ); 
		
		
		Status FlushPage( PageID pid, BOOL bAllowPinned = TRUE, BOOL bFlushPinned = FALSE);
		Status FlushAllPages(BOOL bAllowPinned = TRUE, BOOL bFlushPinned = FALSE);
		
		void  GetStat(int & pinNo, int & missNo, int & writeno) const;
		void GetStat(int * pinNo, int * missNo, int * randMissNo, int * writeNo) const;
		void   ResetStat();
		int GetNumOfAvailableBuffers() const;
		
	private:
		Status ResetBufferSize(int bufSize);
		Status initBufMgr();

		HashTable *hashTable;
#ifdef USE_LRU
		LRUFrame **frames;
#else
		ClockFrame **frames;
#endif
		Replacer *replacer;

		char * m_pBufStartAddr; //by ryan

		int   m_numOfBuf;
		DWORD m_BytesPerSector; // a disk parameter passed in

		int FindFrame( PageID pid );

		int m_nTotalPin;
		int m_nTotalMiss;

		// We call a miss as random miss if the pageid
		// is not next to the last miss pageid
		// (used to record random I/O number)
		int m_nTotalRandMiss; // <= total miss
		PageID m_LastMissPid;

		int m_nTotalWrite; //by rayn

		//added by ssos
		DB* EXDB_DB;
		int page_size;
};


#endif // _BUF_H
