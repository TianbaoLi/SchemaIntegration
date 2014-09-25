#ifndef _REPLACER_H_
#define _REPLACER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "clockframe.h"
#include "LRUframe.h"
#include "hash.h"

class Replacer 
{
	public :

		Replacer();
		~Replacer();

		virtual int PickVictim(BOOL & bWriteBack) = 0;
};

class Clock : public Replacer
{
public:		
	Clock( int bufSize, ClockFrame  **frames, HashTable *hashTable );
	~Clock();
	int PickVictim(BOOL & bWriteBack);
private :
	// current position to check
	int m_current;
	int numOfBuf;
	ClockFrame **frames;
	HashTable *hashTable;
};
#endif