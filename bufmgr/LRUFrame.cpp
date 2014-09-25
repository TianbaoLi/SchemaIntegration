// LRUFrame.cpp: implementation of the LRUFrame class.
//
//////////////////////////////////////////////////////////////////////


#include "buf_common.h"
#include "db.h"
#include "LRUFrame.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LRUFrame::LRUFrame(Page* pg, DB* use_db) 
: Frame(pg, use_db)
{
	referenced = FALSE;
	f_list = NULL;
	//pos = NULL;
	posNull = 1;
	itr = 0;
	this->SetPageID(-1);
}


LRUFrame::~LRUFrame()
{

}

void LRUFrame::Unpin()
{
	Frame::Unpin();
	if (NotPinned())
		referenced = TRUE;
	if(f_list != NULL && this->GetPinCount()==0){
		f_list->push_back(itr);
		list<int>::iterator temp_pos = f_list->end();
		temp_pos --;
		pos = temp_pos;
		posNull = 1;
	}
//	printf("%d\n",f_list->size());
}


Status LRUFrame::Free()
{
	if (Frame::Free() != OK)
		return FAIL;

	referenced = FALSE;
	return OK;
}

void LRUFrame::UnsetReferenced()
{
	referenced = FALSE;
}

void LRUFrame::setList(list<int>* l)
{
	f_list = l;
}

void LRUFrame::setNum(int num)
{
	itr = num;

}

void LRUFrame::setPos(list<int>::iterator ptr)
{
	pos = ptr;

}

void LRUFrame::Pin()
{
	Frame::Pin();
	if(posNull==0){
//		f_list->erase(pos);
		f_list->remove(*pos);
	}
}
