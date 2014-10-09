#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include "buf_common.h"
#include "db.h"
#include "bufmgr.h"
#include "stdio.h"

class Concept
{
public:
	char id[30];
	char name[200];
	char type[30];
	Concept *father;
	Concept()
	{
		memset(id,0,sizeof(id));
		memset(name,0,sizeof(name));
		memset(type,0,sizeof(type));
		father=NULL;
	}
};

class str
{
public:
	char s[500];
	int len;
	str()
	{
		memset(s,0,sizeof(s));
		len=0;
	}
	str(char ss[500], int alen)
	{
		strcpy(s,ss);
		len=alen;
	}
};

vector <Concept*> List;
vector <str> StrList;

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

void DiskRW(char fileWrite[]){

	char* page = NULL;

	int num_page = 32;//the size of buffer
	int page_size = 64; //the size of page

	initHash(num_page); //init hash function


	BufMgr* bm = new BufMgr(fileWrite, num_page, 1024*1024, page_size, page_size);
	int page_no = 0;
	int offset = 0;
	bm->PinPage(page_no, page);

	/*FILE* in = fopen("a.txt", "r");
	char c;
	while(!feof(in)){
		fscanf(in,"%c",&c);
		writeBytes(bm, page, &c, 1, page_no,offset);
	}
	fclose(in);*/

	for(std::vector<str>::iterator Iter=StrList.begin();Iter!=StrList.end();Iter++)
		for(int i=0;i<Iter->len;i++)
			writeBytes(bm, page, &Iter->s[i], 1, page_no, offset);

	/*int total_page = page_no;
	int total_offset = offset;

	page_no = 0;
	offset = 0;
	while(page_no <= total_page){
		readBytes(bm,&c,1,page,page_no,offset);
		printf("%c",c);
	}*/

	delete bm;
	bm = NULL;
}

void getSubClassOf()
{
	FILE *SubClassIn;
	SubClassIn=fopen("rdfsSubClassOf.tsv","r");
	char temp[500],result[500];
	bool validflag,idflag,wordnet;
	int length,i,j;
	while(EOF)
	{
		validflag=1;
		length=0;
		wordnet=0;
		if(fgets(temp,500,SubClassIn)==NULL)
			break;
		if(strcmp(temp,"")==0)
			break;
		if(temp[1]=='h'&&temp[2]=='b'&&temp[3]=='f'&&temp[4]=='i'&&temp[5]=='x')
			continue;
		for(int k=2;k<(int)strlen(temp);k++)
			if(temp[k-2]=='<'&&temp[k]=='>')
			{
				validflag=0;
				break;
			}
		if(validflag==0)
			continue;
		Concept *c=NULL,*f=NULL;
		c=new Concept();
		f=new Concept();
		i=0;
		j=0;
		if(temp[0]=='\t')
		{
			i++;
			wordnet=1;
		}
		else
		{
			while(temp[i]!='_')
				i++;
			i++;
			while(temp[i]!='>')
			{
				c->id[j]=temp[i];
				length++;
				i++;
				j++;
			}
			i+=2;
		}
		i++;
		j=0;
		while(temp[i]!='_')
		{
			c->type[j]=temp[i];
			length++;
			i++;
			j++;
		}
		i++;
		j=0;
		if(wordnet==0)
			while(temp[i]!='>'||temp[i+1]>'9')
			{
				if(temp[i]=='&')
					break;
				c->name[j]=temp[i];
				length++;
				i++;
				j++;
			}
		else
			while(temp[i]!='_'||temp[i+1]>'9')
			{
				if(temp[i]=='&')
					break;
				c->name[j]=temp[i];
				length++;
				i++;
				j++;
			}
		i++;
		j=0;
		while(temp[i]!='>'&&temp[i]>='0'&&temp[i]<='9')
		{
			c->id[j]=temp[i];
			length++;
			i++;
			j++;
		}
		while(temp[i]!='<')
			i++;
		j=0;
		i++;
		idflag=0;
		for(int k=i;k<(int)strlen(temp);k++)
			if(temp[k]>='0'&&temp[k]<='9')
			{
				idflag=1;
				break;
			}
		if(idflag==0)
			continue;
		while(temp[i]!='_')
		{
			f->type[j]=temp[i];
			length++;
			i++;
			j++;
		}
		i++;
		j=0;
		while(temp[i]!='_'||temp[i+1]>'9')
		{
			if(temp[i]=='&')
				break;
			f->name[j]=temp[i];
			length++;
			i++;
			j++;
		}
		i++;
		j=0;
		while(temp[i]!='>'&&temp[i]>='0'&&temp[i]<='9')
		{
			f->id[j]=temp[i];
			length++;
			i++;
			j++;
		}
		c->father=f;
		List.push_back(c);
		List.push_back(f);
		length+=6;
		//printf("%d\n",length);
		sprintf(result,"@%s@%s@%s@%s@%s@%s\n",c->id,c->name,c->type,f->id,f->name,f->type);
		//printf("%s",result);
		StrList.push_back(*(new str(result,length)));
		//delete f;
		delete c;
		//printf("%s\n\n",temp);
		//system("pause");
	}
}

int main()
{
	char fileTo[100]="SubClassOf.txt";
	getSubClassOf();
	DiskRW(fileTo);
	return 0;
}
