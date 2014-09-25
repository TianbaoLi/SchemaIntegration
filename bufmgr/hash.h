#ifndef _HASH_H_
#define _HASH_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <exdb.h>
//#include <buf_frame.h>

//#define HASH(num, entries)  ((num) % (entries))
//#define HASH(num, entries)  ((num*rand()+rand())%3149521%(entries))
//#define HASH(num, entries)  ((num*131+113)%3149521%(entries))
	
/*inline long HASH(long num, long entries){
	srand( (unsigned)time( NULL ) );
	return ((num*rand()+rand())%3149521%(entries))
}*/

#include <stdio.h>
// the hash function
inline 
long HASH(long num, long entries){
  //printf(">> %d, %d, %d, %d\n", hashA, hashB, largePrime, entries);
	return (((hashA * num + hashB) % largePrime) % entries);
	//return num%entries;
}

class Map
{
	friend class MapIterator;

private :

	int pid;
	int frameNo;
	class Map *next;
	class Map *prev;

public :
	PageID PageId();
	
	Map(PageID p, int f);
	~Map();
	void AddBehind(Map *m);
	void DeleteMe();
	
  inline BOOL HasPageID(PageID p)
  {
	  return pid == p;
  }

	int FrameNo();

};


class MapIterator
{
	Map *head;
	Map *current;

public :

	MapIterator(Map *maps);
	Map* operator() ()
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

};


class Bucket
{
	friend class HashTable;
private:

	Map *maps;

public :

	Bucket();
	~Bucket();
	void Insert(PageID pid, int frameNo);
	Status Delete(PageID pid);
	int Find(PageID pid);
	void EmptyIt();
};


class HashTable
{
private:

	Bucket * buckets;

public :
	void DumpHash(const char *out_file);
	~HashTable();
	HashTable(int numOfBuf);
	void Insert(PageID pid, int frameNo);
	Status Delete(PageID pid);
	int LookUp(PageID pid);
	void EmptyIt();
private:
	int m_num_buckets;
	bool is_prime(int n)
	{
		for(int i=2; i< n; i++)
		{
			if( n % i == 0 )
			{
				return false;
			}
		}
		return true;
	}
};


#endif