// LRU.cpp: implementation of the LRU class.
//
//////////////////////////////////////////////////////////////////////
#include "stdio.h"
#include "buf_common.h"
#include "db.h"
#include "replacer.h"
#include "LRUframe.h"
#include "hash.h"
#include "LRU.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LRU::LRU(int bufSize, LRUFrame **bufFrames, HashTable *table)
{
	m_current = 0;
	numOfBuf = bufSize;
	frames = bufFrames;
	hashTable = table;
	for(int i=0; i< bufSize; i++){
		bufFrames[i]->setList(&used);
		bufFrames[i]->setNum(i);
		used.push_back(i);
		list<int>::iterator pos=used.end();
		pos --;
		bufFrames[i]->setPos(pos);
	}
}

LRU::~LRU()
{

}
// bWriteBack is specially added to have write statistics
// 
int LRU::PickVictim(BOOL & bWriteBack)
{
	int numOfTest = 0;

	bWriteBack = FALSE;
	
	int frameNo = INVALID_FRAME;

/*	if(!used.empty()){
		printf(">>%d\n",used.front());
	}*/

	if(used.empty()){
		printf("no cache to use:-(\n");
	}
	else if(frames[used.front()]->IsVictim()){

		if(frames[used.front()]->GetPageID() != -1){
			hashTable->Delete(frames[used.front()]->GetPageID());
		}
		
		if(frames[used.front()]->IsDirty() )
		{
			bWriteBack = TRUE;
			frames[used.front()]->Write();
		}
		frames[used.front()]->EmptyIt();
		frameNo = used.front();
		used.pop_front();
	}
			
	return frameNo;
}