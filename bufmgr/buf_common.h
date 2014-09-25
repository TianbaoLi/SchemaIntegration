#define BOOL bool
#define TRUE true
#define FALSE false
#define DWORD long
#define PageID int 
#define Page char

#define FAIL 0
#define OK 1
#define BUFMGR 2


#define Status int

#ifndef BUF_COMM
#define BUF_COMM

#include <list>
using namespace std;

extern long hashA, hashB, largePrime;
void initHash(long entries);
#endif
