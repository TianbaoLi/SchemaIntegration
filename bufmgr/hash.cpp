//#include "stdafx.h"
//#include <fstream>

#define  _CRT_SECURE_NO_WARNINGS
#include"stdlib.h"
#include"stdio.h"
#include "buf_common.h"
#include "db.h"
#include "hash.h"

//using namespace std;

#define INVALID_PAGE -1


//-----------------------------------------------------
// Map, MapIterator, Bucket & HashTable.
//-----------------------------------------------------
// a hash table consists of buckets
// HashTable class hashes the pid to the appropriate bucket
// Buckets contain linear doubly linked lists of Map
//-----------------------------------------------------



//-----------------------------------------------------
// Map class
//
// A node in a linear doubly link list with dummy head.
// Contains the (pid, frameNo) pair.
//-----------------------------------------------------


Map::Map(PageID p, int f)
{
	pid = p;
	frameNo = f;
	next = NULL;
	prev = NULL;
}


Map::~Map()
{
	
}


void Map::AddBehind(Map *m)
{
	m->next = next;
	m->prev = this;
	if (next)
	{
		next->prev = m;
	}
	next = m;
}


void Map::DeleteMe()
{
	if (next)
		next->prev = prev;
	if (prev)
		prev->next = next;
}

/*
BOOL Map::HasPageID(PageID p)
{
	return pid == p;
}
*/


int Map::FrameNo()
{
	return frameNo;
}


//-----------------------------------------------------
// MapIterator class
//
// A iterator for the link list consists of Map object.
//-----------------------------------------------------


MapIterator::MapIterator(Map *maps)
{
//	ASSERT( maps->pid == INVALID_PAGE );
	current = maps->next; // skip the dummy
}

/*
Map* MapIterator::operator() ()
{
	if (!current)
	{
		return NULL;
	}
	else
	{
		Map *tmp;
		tmp = current;
		current = current->next;
		return tmp;
	}
}
*/


//--------------------------------------------------------
// Bucket class
// 
// A bucket contains a linear doubly link list of Map, and 
// support insert/delete/lookup operation on the link list.
//--------------------------------------------------------

Bucket::Bucket()
{
	maps = new Map(INVALID_PAGE, -1);
}


Bucket::~Bucket()
{
	EmptyIt();

	// delete the head of the map list
	delete maps;
}


void Bucket::Insert(PageID pid, int frameNo)
{
	Map *m = new Map(pid, frameNo);

	if (maps)
	{
		// Add behind the first (dummy) node,
		// which means insert in front of the
		// list.

		maps->AddBehind(m); 
	}
	else
	{
		maps = m;
	}
}


/*
Status Bucket::Delete(PageID pid)
{
	MapIterator next(maps);
	Map *curr;

	// Iterate thru the list and delete the
	// one with matching pid.

	while (curr = next())
	{
		if (curr->HasPageID(pid))
		{
			curr->DeleteMe();
			delete curr;
			return OK;
		}
	}
	return FAIL;		
}
*/
Status Bucket::Delete(PageID pid)
{
	MapIterator next(maps);
	Map *curr;

	// Iterate thru the list and delete the
	// one with matching pid.
  int cnt = 0;
 #ifdef TEST
	printf("\n>>-----------------------------\n");
#endif
	while (curr = next())
	{
#ifdef TEST
    cnt++;
	printf("%d\t",curr->PageId());
#endif
    if (curr->HasPageID(pid))
		{
			curr->DeleteMe();
			delete curr;
			//printf("len = %d\n", cnt);

#ifdef TEST
			printf("\n>>-----------------------------\n");
#endif
			return OK;
		}
	}
#ifdef TEST
	printf("\n>>-----------------------------\n");
#endif
	return FAIL;		
}


int Bucket::Find(PageID pid)
{
	MapIterator next(maps);
	Map *curr;
	
	// Iterate thru the list and return the frameNo
	// of the node with matching pid.

	while (curr = next())
	{
		if (curr->HasPageID(pid))
		{
			return curr->FrameNo();
		}
	}
	//return INVALID_FRAME;
  return -1;
}


void Bucket::EmptyIt()
{
	MapIterator next(maps);
	Map *curr;
	
	// Iterate thru the list, delete every node.

	while (curr = next())
	{
		curr->DeleteMe();
		delete curr;
	}


}


//------------------------------------------------------
// HashTable class
// 
// Just hash the pid to the appropiate bucket, and call 
// the appropiate function for that bucket.
//-------------------------------------------------------

HashTable::HashTable(int numOfBuf)
{
	//decide the number of buckets
	// number of buckets is the largest prime number that is smaller than numOfBuf
	int min_num = 3;
	for(int i=numOfBuf; i >= min_num ; i--)
	{
		if( is_prime(i) )
		{
			m_num_buckets = i; 
			break;
		}
	}

//	ASSERT( m_num_buckets >=23 );
	
	// allocate buckets
	buckets = new Bucket[m_num_buckets];
//	printf("number of bucket: %d\n",m_num_buckets);
	// for debug
//	page_5_count = 0;
}
void HashTable::Insert(PageID pid, int frameNo)
{
//	ASSERT(pid != INVALID_PAGE);
	buckets[HASH(pid, m_num_buckets)].Insert(pid, frameNo);
}


Status HashTable::Delete(PageID pid)
{
	return buckets[HASH(pid, m_num_buckets)].Delete(pid);
}


int HashTable::LookUp(PageID pid)
{
//	ASSERT(pid != INVALID_PAGE);
	return buckets[HASH(pid, m_num_buckets)].Find(pid);
}

void HashTable::EmptyIt()
{
	int i;
	
	for (i = 0; i < m_num_buckets; i++)
	{
		buckets[i].EmptyIt();
	}
}

HashTable::~HashTable()
{
	delete []buckets;
}

void HashTable::DumpHash(const char *out_file)
{
	FILE* out=fopen(out_file,"a");
	fprintf(out,"\n>>------------------------------\n");

	int count = 0;
	int empty_number=0;
	for(int i=0; i<m_num_buckets; i++)
	{
		fprintf(out,"bucket %d:",i);

		MapIterator next(buckets[i].maps);
		Map *curr;
		int b=0;
	
		// Iterate thru the list and return the frameNo
		// of the node with matching pid.
		while (curr = next())
		{
			count++;
			fprintf(out,"(%d, %d),",curr->PageId(),curr->FrameNo());
			b=1;
		}
		if(b==0){
			empty_number += 1;
		}
		fprintf(out,"\n");
	}
	fprintf(out,"number of items:%d\n",count);
	fprintf(out,">>------------------------------\n");
	fclose(out);

	out = fopen("empty.txt","a");
	fprintf(out,"%d\n",empty_number);
	fclose(out);
}

PageID Map::PageId()
{
	return pid;
}
