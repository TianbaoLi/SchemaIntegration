#define _CRT_SECURE_NO_WARNINGS
#include "bufmgr/buf_common.h"
#include "bufmgr/db.h"
#include "bufmgr/bufmgr.h"
#include "stdio.h"

//read bytes in buf with size length
//the bytes is read to page
//the position is (blockid, offset)
void readBytes(BufMgr* bm, char *buf, int length, char *& page, int& blockid, int& offset)
{
	if(length< bm->getPagesize() - offset){
		memcpy(buf,page + offset,length);
		offset += length;
	}
	else{
		int temp = bm->getPagesize() - offset;
		memcpy(buf, page+offset, temp);

		bm->UnpinPage(blockid);
		blockid += 1;
		bm->PinPage(blockid, page);

		memcpy(buf+temp, page, length - temp);
		offset = length- temp;
	}
}

//write bytes in context with size length
//the bytes is write to page
//the position is (blockid, offset)
void writeBytes(BufMgr* bm, char *&page, char *context, int length, int& page_no, int& offset)
{
	if(bm->getPagesize()-offset >= length){
		memcpy(page+offset,context,length);
		offset += length;

	}
	else{
		int temp = bm->getPagesize() - offset;
		memcpy(page+offset, context, temp);
		bm->UnpinPage(page_no,true);
		
		
		page_no += 1;
		offset = 0;
		bm->PinPage(page_no, page);
		memcpy(page, context+temp, length - temp);
		offset = length-temp;
	}
}

void main(int argc, void* argv[]){

	if(argc < 2){
		exit(0);
	}

	char* page = NULL;

	FILE* in = fopen((char*)argv[1], "r");

	char filename[100] = "test.txt";
	int num_page = 32;//the size of buffer
	int page_size = 32; //the size of page

	initHash(num_page); //init hash function

	BufMgr* bm = new BufMgr(filename, num_page, 1024*1024, page_size, page_size);
	int page_no = 0;
	int offset = 0;
	bm->PinPage(page_no, page);

	char c;

	while(!feof(in)){
		fscanf(in,"%c",&c);
		writeBytes(bm, page, &c, 1, page_no,offset);
	}
	fclose(in);

	int total_page = page_no;
	int total_offset = offset;

	page_no = 0;
	offset = 0;
	while(page_no <= total_page){
		readBytes(bm,&c,1,page,page_no,offset);
		printf("%c",c);
	}

	delete bm;
	bm = NULL;
}