// LRUFrame.h: interface for the LRUFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LRUFRAME_H__3EFF313B_3537_4ABC_96CD_B048CB6BA1F1__INCLUDED_)
#define AFX_LRUFRAME_H__3EFF313B_3537_4ABC_96CD_B048CB6BA1F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "buf_frame.h"

class LRUFrame : public Frame  
{
private:
	int itr;//its number
	bool referenced;
	list<int>* f_list;//pointer to list in replacer
	list<int>::iterator pos;
	bool posNull;
public:
	void Pin();
	void setPos(list<int>::iterator ptr);
	void setNum(int num);
	void setList(list<int>* l);

	LRUFrame(Page* pg, DB* use_db);
	~LRUFrame();
	
	void Unpin();
	Status Free();
	void UnsetReferenced();
	inline BOOL IsReferenced() 
    {
      return (referenced == (int)TRUE);
    }

    inline BOOL IsVictim()
    {
	    return NotPinned();
    }
};

#endif // !defined(AFX_LRUFRAME_H__3EFF313B_3537_4ABC_96CD_B048CB6BA1F1__INCLUDED_)
