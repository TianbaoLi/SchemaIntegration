#ifndef _FRAME_H_
#define _FRAME_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define INVALID_FRAME -1

class Frame 
{
	friend class BufMgr;
	public :
		int GetPinCount() const;
		
		Frame(Page * pg, DB* use_db);
		~Frame();
		void Pin();
		virtual void Unpin();
		void EmptyIt();
		void DirtyIt();
		void SetPageID(PageID pid);
		BOOL IsDirty() const;
		BOOL IsValid() const;
		Status Write();
		Status Read(PageID pid);
		virtual Status Free();

    BOOL NotPinned() const
    {
      return m_pinCount == 0; 
    }

		BOOL HasPageID(PageID pid) const;
		PageID GetPageID() const;
		Page *GetPage();
private :
		static int m_nInvalidFrames; // the number of frames that are invalid
		static int m_nUnPinnedFrames; 
		PageID m_curPid;
		Page   *m_data;
		int    m_dirty;
		int    m_pinCount;

		DB* EXDB_DB;

};
#endif