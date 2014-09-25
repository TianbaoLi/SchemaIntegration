// LRU.h: interface for the LRU class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LRU_H__4DA9CE47_717B_49A4_ABFF_1161FC247486__INCLUDED_)
#define AFX_LRU_H__4DA9CE47_717B_49A4_ABFF_1161FC247486__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "replacer.h"
#include "hash.h"
#include "LRUframe.h"

class LRU : public Replacer  
{
public:
	LRU(int bufSize, LRUFrame **bufFrames, HashTable *table);
	virtual ~LRU();
	int PickVictim(BOOL & bWriteBack);
private:
	int m_current;
	int numOfBuf;
	LRUFrame **frames;
	HashTable *hashTable;
	list<int> used;
};

#endif // !defined(AFX_LRU_H__4DA9CE47_717B_49A4_ABFF_1161FC247486__INCLUDED_)
