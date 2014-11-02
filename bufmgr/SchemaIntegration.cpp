#define _CRT_SECURE_NO_WARNINGS
#include "buf_common.h"
#include "db.h"
#include "bufmgr.h"
#include <cstdio>
#include <vector>
#include <ctime>
#include <algorithm>

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
	Concept(char aid[30],char aname[200],char atype[30],Concept *afather)
	{
		Clear();
		memcpy(id,aid,strlen(aid));
		memcpy(name,aname,strlen(aname));
		memcpy(type,atype,strlen(atype));
		father=afather;
	}
	void Clear()
	{
		memset(id,0,sizeof(id));
		memset(name,0,sizeof(name));
		memset(type,0,sizeof(type));
		father=NULL;
	}
};

class Road
{
public:
	Concept *start;
	Concept *end;
	int length;
	Road()
	{
		start=NULL;
		end=NULL;
		length=0;
	}
	Road(Concept *astart,Concept *aend,int alength)
	{
		start=new Concept(astart->id,astart->name,astart->type,astart->father);
		end=new Concept(aend->id,aend->name,aend->type,aend->father);
		length=alength;
	}
};
class Answer
{
public:
	char name[200];
	int length;
	Answer()
	{
		memset(name,0,sizeof(name));
		length=0;
	}
	Answer(char aName[200],int aLength)
	{
		Answer();
		strcpy(name,aName);
		length=aLength;
	}
};

//vector <int> Length_SubClassOf;
//vector <int> Length_Type;
vector <Concept*> Result;
vector <Road*> RoadList;
vector <Road*> RoadListTemp;
vector <Concept*> ListTarget;
vector <Answer*> AnswerList;

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


char* page = NULL;

int num_page = 32;//the size of buffer
int page_size = 1024; //the size of page

BufMgr* bmSubClassOf = new BufMgr("SubClassOf.txt", num_page, 1024*1024, page_size, page_size);
BufMgr* bmType = new BufMgr("Types.txt", num_page, 1024*1024, page_size, page_size);
BufMgr* bmMatch = new BufMgr("Types.txt", num_page, 1024*1024, page_size, page_size);
int page_no_SubClassOf = 0;
int offset_SubClassOf = 0;
int page_no_Type = 0;
int offset_Type = 0;
int page_no_Match = 0;
int offset_Match = 0;


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

	/*for(vector<str>::iterator Iter=StrList.begin();Iter!=StrList.end();Iter++)
		for(int i=0;i<Iter->len;i++)
			writeBytes(bm, page, &Iter->s[i], 1, page_no, offset);*/

	/*int total_page = page_no;
	int total_offset = offset;

	page_no = 0;
	offset = 0;
	while(page_no <= total_page){
		readBytes(bm,&c,1,page,page_no,offset);
		printf("%c",c);
	}*/

	/*delete bm;
	bm = NULL;*/
}

void getSubClassOf()
{
	FILE *SubClassIn,*Length_SubClassOf;
	SubClassIn=fopen("rdfsSubClassOf.tsv","r");
	Length_SubClassOf=fopen("Length_SubClassOf.txt","w");
	bmSubClassOf->PinPage(page_no_SubClassOf, page);
	FILE *Out;
	Out=fopen("SubClassOf.txt","w");
	char temp[800],result[800];
	bool validflag,idflag,wordnet,yago;
	int length,colon;
	Concept *c=NULL,*f=NULL;
	c=new Concept();
	f=new Concept();
	while(EOF)
	{
		validflag=1;
		length=0;
		wordnet=0;
		yago=0;
		colon=0;
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
		for(int i=0;i<(int)strlen(temp)-4;i++)
			if(temp[i]=='y'&&temp[i+1]=='a'&&temp[i+2]=='g'&&temp[i+3]=='o')
			{
				yago=1;
				break;
			}
		for(int i=0;i<(int)strlen(temp);i++)
			if(temp[i]==':')
				colon++;
		if(yago==1||colon>1)
			continue;
		c->Clear();
		f->Clear();
		int i=0,j=0;
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
		if(strcmp(c->id,"u4ru69_1m6_ibvh7i")==0)
			continue;
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
		/*c->father=f;
		List.push_back(c);
		List.push_back(f);*/
		length+=6;
		//length++;/////////////////////////////////////////////////////////////////////////////////////////////////////
		//printf("%d\n",length);
		sprintf(result," %s %s %s %s %s %s",c->id,c->name,c->type,f->id,f->name,f->type);
		/*str *s=new str(result,length);
		StrList.push_back(*s);
		delete s;*/
		/*for(int k=0;k<(int)strlen(result);k++)
			writeBytes(bmSubClassOf, page, &result[k], 1, page_no_SubClassOf, offset_SubClassOf);*/
		sprintf(result," %s %s %s %s %s %s",c->id,c->name,c->type,f->id,f->name,f->type);
		fprintf(Out,"%s",result);
		//Length_SubClassOf.push_back(length);
		fprintf(Length_SubClassOf,"%d\n",length);
		//printf("%s\n\n",temp);
		//system("pause");
	}
	//memset(page+offset_SubClassOf,0,page_size-offset_SubClassOf);
	//bmSubClassOf->UnpinPage(page_no_SubClassOf,true);
	delete f;
	delete c;
	fclose(Length_SubClassOf);
	fclose(Out);
	//system("pause");
}

void getTypes()
{
	FILE *TypeIn,*Length_Type;
	TypeIn=fopen("rdfTypes.tsv","r");
	Length_Type=fopen("Length_Type.txt","w");
	bmType->PinPage(page_no_Type, page);
	FILE *Out;
	Out=fopen("Types.txt","w");
	char temp[800],result[800];
	int length,colon;
	bool wordnet,yago,valid;
	Concept *c=NULL,*f=NULL;
	c=new Concept();
	f=new Concept();
	while(EOF)
	{
		length=0;
		wordnet=0;
		yago=0;
		colon=0;
		valid=0;
		if(fgets(temp,1000,TypeIn)==NULL)
			break;
		if(strcmp(temp,"")==0)
			break;
		for(int i=0;i<(int)strlen(temp)-4;i++)
			if(temp[i]=='y'&&temp[i+1]=='a'&&temp[i+2]=='g'&&temp[i+3]=='o')
			{
				yago=1;
				break;
			}
		for(int i=0;i<(int)strlen(temp);i++)
			if(temp[i]==':')
				colon++;
		if(yago==1||colon>1)
			continue;
		c->Clear();
		f->Clear();
		int i=0,j=0;
		while(temp[i]!='_')
			i++;
		i++;
		while(temp[i]!='>')
			i++;
		i+=3;
		j=0;
		while(temp[i]!='>'||temp[i+1]>'9')
		{
			/*if(temp[i]=='&')
				break;*/
			c->name[j]=temp[i];
			length++;
			i++;
			j++;
		}
		if(strcmp(c->name,"C")==0)
		{
			j+=0;
		}
		strcpy(c->type,"root");
		length+=4;
		i++;
		j=0;
		while(temp[i]!='<')
			i++;
		j=0;
		i++;
		for(int k=i;k<=(int)strlen(temp);k++)
			if(temp[k]=='_')
			{
				valid=1;
				break;
			}
		if(valid==0)
			continue;
		if(temp[i+1]=='o')
			wordnet=1;
		while(temp[i]!='_')
		{
			f->type[j]=temp[i];
			length++;
			i++;
			j++;
		}
		i++;
		j=0;
		if(wordnet==1)
		{
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
		}
		else
		{
			while(temp[i]!='>'&&temp[i]!='<')
			{
				if(temp[i]=='&')
					break;
				f->name[j]=temp[i];
				length++;
				i++;
				j++;
			}
		}
		/*c->father=f;
		List.push_back(c);
		List.push_back(f);*/
		length+=6;
		//printf("%d\n",length);
		sprintf(result," %s %s %s %s %s %s",c->id,c->name,c->type,f->id,f->name,f->type);
		/*str *s=new str(result,length);
		StrList.push_back(*s);
		delete s;*/
		/*for(int k=0;k<(int)strlen(result);k++)
			writeBytes(bmType, page, &result[k], 1, page_no_Type, offset_Type);*/
		//Length_Type.push_back(length);
		fprintf(Length_Type,"%d\n",length);
		//printf("%s",result);
		//printf("%s\n\n",temp);
		fprintf(Out,"%s",result);
		//system("pause");
	}
	/*memset(page+offset_Type,0,page_size-offset_Type);
	bmType->UnpinPage(page_no_Type,true);*/
	delete f;
	delete c;
	fclose(Length_Type);
	fclose(Out);
	//system("pause");
}

void readTypes(char target[200],BufMgr *bm,int num_page,int page_size)
{
	FILE *Length_Type;
	Length_Type=fopen("Length_Type.txt","r");
	char* page = NULL;
	int page_no = 0;
	int offset = 0;
	bm->PinPage(page_no, page);
	char temp[800];
	int total_page = num_page;
	int total_offset = offset;
	int i,j;
	char tempname[200];
	int data=0;
	Concept *c=new Concept();
	Concept *f=new Concept();
	bool end=0;
	bool got=0;
	int length=0;
	while(page_no <= total_page)
	{
		if(end==1)
			break;
		while(fscanf(Length_Type,"%d",&length)==1)
		{
			i=1;
			j=0;
			data++;
			c->Clear();
			f->Clear();
			memset(temp,0,sizeof(temp));
			memset(tempname,0,sizeof(tempname));
			readBytes(bm,temp,length,page,page_no,offset);
			if(data==9017808)
			{
				data+=0;
			}
			if(temp[0]==0)
			{
				continue;
			}
			while(temp[i]!=' ')
			{
				c->id[j]=temp[i];
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				tempname[j]=temp[i];
				i++;
				j++;
			}
			if((got==1)&&strcmp(Result[Result.size()-1]->name,tempname)!=0)
			{
				end=1;
				break;
			}
			if(strcmp(target,tempname)==0)
			{
				c->Clear();
				f->Clear();
				i=1;
				j=0;
				while(temp[i]!=' ')
				{
					c->id[j]=temp[i];
					i++;
					j++;
				}
				i++;
				j=0;
				while(temp[i]!=' ')
				{
					c->name[j]=temp[i];
					i++;
					j++;
				}
				i++;
				j=0;
				while(temp[i]!=' ')
				{
					c->type[j]=temp[i];
					i++;
					j++;
				}
				i++;
				j=0;
				while(temp[i]!=' ')
				{
					f->id[j]=temp[i];
					i++;
					j++;
				}
				i++;
				j=0;
				while(temp[i]!=' ')
				{
					f->name[j]=temp[i];
					i++;
					j++;
				}
				i++;
				j=0;
				while(i<(int)strlen(temp))
				{
					f->type[j]=temp[i];
					i++;
					j++;
				}
				//c->father=f;
				Concept *fcopy=new Concept(f->id,f->name,f->type,f->father);
				Concept *ccopy=new Concept(c->id,c->name,c->type,fcopy);
				Result.push_back(ccopy);
				got=1;
				//printf("%s\n",temp);
				//return temp;
			}
		}
	}
	delete f;
	delete c;
	fclose(Length_Type);
}

bool judge(char target1[200],char target2[200],int threshold)
{
	for(vector<Road*>::iterator Iter1=RoadList.begin();Iter1!=RoadList.end();Iter1++)
		if(strcmp(target1,(*Iter1)->start->name)==0)
			for(vector<Road*>::iterator Iter2=RoadList.begin();Iter2!=RoadList.end();Iter2++)
				if(strcmp(target2,(*Iter2)->start->name)==0)
					if(strcmp((*Iter1)->end->name,(*Iter2)->end->name)==0&&(*Iter1)->length+(*Iter2)->length<=threshold)
					{
						/*printf("GOT\n");
						printf("Length is:%d\n",(*Iter1)->length+(*Iter2)->length);
						printf("Common Father is:%s\n",(*Iter1)->end->name);*/
						printf("%s\n",target2);
						return 1;
					}
	return 0;
}

bool readSubClassOf(char target[200],BufMgr *bm,int num_page,int page_size,int threshold)
{
	FILE *Length_SubClassOf;
	Length_SubClassOf=fopen("Length_SubClassOf.txt","r");
	char* page = NULL;
	int page_no = 0;
	int offset = 0;
	bm->PinPage(page_no, page);
	char temp[800];
	int total_page = num_page;
	int total_offset = offset;
	int i,j;
	char tempname[200],tempfathername[200];
	int data=0;
	Concept *c=new Concept();
	Concept *f=new Concept();
	int length=0;
	while(page_no <= total_page)
	{
		while(fscanf(Length_SubClassOf,"%d",&length)==1)
		{
			i=1;
			j=0;
			data++;
			c->Clear();
			f->Clear();
			memset(temp,0,sizeof(temp));
			memset(tempname,0,sizeof(tempname));
			memset(tempfathername,0,sizeof(tempfathername));
			readBytes(bm,temp,length,page,page_no,offset);
			if(temp[0]==0)
				break;
			while(temp[i]!=' ')
			{
				c->id[j]=temp[i];
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				tempname[j]=temp[i];
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				tempfathername[j]=temp[i];
				i++;
				j++;
			}
			for(vector<Road*>::iterator Iter=RoadList.begin();Iter!=RoadList.end();Iter++)
				if(strcmp(tempname,(*Iter)->end->name)==0||strcmp(tempfathername,(*Iter)->end->name)==0)
				{
					c->Clear();
					f->Clear();
					i=1;
					j=0;
					while(temp[i]!=' ')
					{
						c->id[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(temp[i]!=' ')
					{
						c->name[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(temp[i]!=' ')
					{
						c->type[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(temp[i]!=' ')
					{
						f->id[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(temp[i]!=' ')
					{
						f->name[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(i<(int)strlen(temp))
					{
						f->type[j]=temp[i];
						i++;
						j++;
					}
					//c->father=f;
					Concept *ccopy=new Concept(c->id,c->name,c->type,c->father);
					Concept *fcopy=new Concept(f->id,f->name,f->type,f->father);
					//Concept *ccopy=new Concept(c->id,c->name,c->type,fcopy);
					//Result.push_back(ccopy);
					if(strcmp(tempname,(*Iter)->end->name)==0&&(*Iter)->length<=threshold/2)
						RoadListTemp.push_back(new Road((*Iter)->start,fcopy,(*Iter)->length+1));
					if(strcmp(tempfathername,(*Iter)->end->name)==0)
						RoadListTemp.push_back(new Road((*Iter)->start,ccopy,(*Iter)->length+1));
					//if(judge(target1,target2,threshold)==1)
					//	return 1;
					//(*Iter)->end=new Concept(f->id,f->name,f->type,f->father);
					//(*Iter)->length++;
					//printf("%s\n",temp);
					//return temp;
				}
		}
	}
	delete f;
	delete c;
	fclose(Length_SubClassOf);
	return 0;
}

void matchTypes(char target[200],BufMgr *bm,int num_page,int page_size)
{
	FILE *Length_Type;
	Length_Type=fopen("Length_Type.txt","r");
	char* page = NULL;
	int page_no = 0;
	int offset = 0;
	bm->PinPage(page_no, page);
	char temp[800];
	int total_page = num_page;
	int total_offset = offset;
	int i,j;
	char tempname[200],tempfathername[200];
	int data=0;
	Concept *c=new Concept();
	Concept *f=new Concept();
	int length=0;
	while(page_no <= total_page)
	{
		while(fscanf(Length_Type,"%d",&length)==1)
		{
			i=1;
			j=0;
			data++;
			c->Clear();
			f->Clear();
			memset(temp,0,sizeof(temp));
			memset(tempname,0,sizeof(tempname));
			memset(tempfathername,0,sizeof(tempfathername));
			readBytes(bm,temp,length,page,page_no,offset);
			if(data==9017808)
			{
				data+=0;
			}
			if(temp[0]==0)
			{
				continue;
			}
			while(temp[i]!=' ')
			{
				c->id[j]=temp[i];
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				tempname[j]=temp[i];
				i++;
				j++;
			}
			if(strcmp(tempname,"Herbert_Olivecrona")==0)
			{
				j+=0;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				tempfathername[j]=temp[i];
				i++;
				j++;
			}
			/*for(vector<Road*>::iterator Iter=RoadList.begin();Iter!=RoadList.end();Iter++)
				if(strcmp((*Iter)->end->name,"Vowel_letters")==0)
				{
					j++;
					j--;
				}*/
			for(vector<Road*>::iterator Iter=RoadList.begin();Iter!=RoadList.end();Iter++)
				if(strcmp(tempfathername,(*Iter)->end->name)==0&&strcmp(tempname,target)!=0)
				{
					AnswerList.push_back(new Answer(tempname,(*Iter)->length+1));
					//printf("%s\t%d\n",tempname,(*Iter)->length+1);
				}
		}
	}
	delete f;
	delete c;
	fclose(Length_Type);
}

bool ResultSort(Road *lt,Road *rt)
{
	if(strcmp((lt->end->name),(rt->end->name))>0)
		return 1;
	else
		return 0;
}

bool AnswerSort(Answer *lt,Answer *rt)
{
	if(lt->length!=rt->length)
		return lt->length<rt->length;
	else if(strcmp((lt->name),(rt->name))>=0)
		return 0;
	else
		return 1;
}

void join1(char target[800],int threshold)
{
	Result.clear();
	RoadList.clear();
	AnswerList.clear();
	Result.reserve(10000);
	RoadList.reserve(100000);
	AnswerList.reserve(10000);
	int len=0;
	readTypes(target,bmType,page_no_Type,page_size);
	if(len==Result.size())
	{
		printf("Didn`t Find Target1!\n");
		return;
	}
	len=Result.size();
	while(Result.empty()==0)
	{
		Concept *c=Result.front();
		Result.erase(Result.begin());
		RoadList.push_back(new Road(c,c->father,1));
	}
	for(int i=2;i<threshold;i++)
	{
		readSubClassOf(target,bmSubClassOf,page_no_SubClassOf,page_size,threshold);
		while(RoadListTemp.empty()==0)
		{
			Road *r=RoadListTemp.front();
			RoadListTemp.erase(RoadListTemp.begin());
			RoadList.push_back(new Road(r->start,r->end,r->length));
		}
		//("%d\n",i);
	}
	sort(RoadList.begin(),RoadList.end(),ResultSort);
	page_no_Type=0;
	matchTypes(target,bmType,page_no_Type,page_size);
	sort(AnswerList.begin(),AnswerList.end(),AnswerSort);
	for(vector<Answer*>::iterator Iter=AnswerList.begin();Iter!=AnswerList.end()-1;Iter++)
		if(strcmp((*Iter)->name,target)==0||strcmp((*Iter)->name,(*(Iter+1))->name)==0)
		{
			Iter=AnswerList.erase(Iter);
			Iter--;
		}
	char filename[100];
	memset(filename,0,sizeof(filename));
	strcpy(filename,"Schema Integration of ");
	strcat(filename,target);
	strcat(filename," by Join Algorithm 1 at Threshold of ");
	char thre[5];
	memset(thre,0,sizeof(thre));
	_itoa(threshold,thre,10);
	strcat(filename,thre);
	strcat(filename,".txt");
	FILE *IntegrationOut=fopen(filename,"w");
	int SchemaCount=0;
	for(vector<Answer*>::iterator Iter=AnswerList.begin();Iter!=AnswerList.end();Iter++)
	{
		fprintf(IntegrationOut,"%s\t%d\n",(*Iter)->name,(*Iter)->length);
		SchemaCount++;
	}
	fprintf(IntegrationOut,"Integrated Schema Amount:%d\n",SchemaCount);
	printf("Integrated Schema Amount:%d\n",SchemaCount);
	fclose(IntegrationOut);
}

bool readSubClassOf(BufMgr *bm,int num_page,int page_size,char target1[800],char target2[800],int threshold)
{
	FILE *Length_SubClassOf;
	Length_SubClassOf=fopen("Length_SubClassOf.txt","r");
	char* page = NULL;
	int page_no = 0;
	int offset = 0;
	bm->PinPage(page_no, page);
	char temp[800];
	int total_page = num_page;
	int total_offset = offset;
	int i,j;
	char tempname[200];
	int data=0;
	Concept *c=new Concept();
	Concept *f=new Concept();
	int length=0;
	while(page_no <= total_page)
	{
		while(fscanf(Length_SubClassOf,"%d",&length)==1)
		{
			i=1;
			j=0;
			data++;
			c->Clear();
			f->Clear();
			memset(temp,0,sizeof(temp));
			memset(tempname,0,sizeof(tempname));
			readBytes(bm,temp,length,page,page_no,offset);
			if(temp[0]==0)
				break;
			while(temp[i]!=' ')
			{
				c->id[j]=temp[i];
				i++;
				j++;
			}
			if(strcmp(c->id,"u4ru69_1m6_ibvh7i")==0)
				continue;
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				tempname[j]=temp[i];
				i++;
				j++;
			}
			for(vector<Road*>::iterator Iter=RoadList.begin();Iter!=RoadList.end();Iter++)
				if(strcmp(tempname,(*Iter)->end->name)==0)
				{
					c->Clear();
					f->Clear();
					i=1;
					j=0;
					while(temp[i]!=' ')
					{
						c->id[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(temp[i]!=' ')
					{
						c->name[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(temp[i]!=' ')
					{
						c->type[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(temp[i]!=' ')
					{
						f->id[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(temp[i]!=' ')
					{
						f->name[j]=temp[i];
						i++;
						j++;
					}
					i++;
					j=0;
					while(i<(int)strlen(temp))
					{
						f->type[j]=temp[i];
						i++;
						j++;
					}
					//c->father=f;
					Concept *fcopy=new Concept(f->id,f->name,f->type,f->father);
					//Concept *ccopy=new Concept(c->id,c->name,c->type,fcopy);
					//Result.push_back(ccopy);
					RoadList.push_back(new Road((*Iter)->start,fcopy,(*Iter)->length+1));
					if(judge(target1,target2,threshold)==1)
						return 1;
					//(*Iter)->end=new Concept(f->id,f->name,f->type,f->father);
					//(*Iter)->length++;
					//printf("%s\n",temp);
					//return temp;
				}
		}
	}
	delete f;
	delete c;
	fclose(Length_SubClassOf);
	return 0;
}

void match(char target1[800],char target2[800],int threshold)
{
	/*Concept *c;
	while(Result.empty()==0)
	{
		c=Result.front();
		Result.erase(Result.begin());
		RoadList.push_back(new Road(c,c->father,1));
	}*/
	for(vector<Concept*>::iterator Iter=Result.begin();Iter!=Result.end();Iter++)
		RoadList.push_back(new Road(*Iter,(*Iter)->father,1));
	if(judge(target1,target2,threshold)==1)
			return;
	while(1)
	{
		if(judge(target1,target2,threshold)==1)
			return;
		bool flag=0;
		for(vector<Road*>::iterator Iter1=RoadList.begin();Iter1!=RoadList.end();Iter1++)
			if((*Iter1)->length>=threshold)
			{
				flag=1;
				break;
			}
		if(flag==1)
			return;
		if(readSubClassOf(bmSubClassOf,page_no_SubClassOf,page_size,target1,target2,threshold)==1)
			return;
	}
}

void join2(char target[800],int threshold)
{
	FILE *Length_Type;
	Length_Type=fopen("Length_Type.txt","r");
	char* page = NULL;
	int page_no = 0;
	int offset = 0;
	bmMatch->PinPage(page_no, page);
	char temp[800];
	int total_page = num_page;
	int total_offset = offset;
	int i,j;
	int data=0;
	int length=0;
	char lastRecord[200];
	memset(lastRecord,0,sizeof(lastRecord));
	Concept *c=new Concept();
	Concept *f=new Concept();
	readTypes(target,bmType,page_no_Type,page_size);
	ListTarget.reserve(100);
	while(Result.empty()==0)
	{
		Concept *c=Result.front();
		Result.erase(Result.begin());
		ListTarget.push_back(c);
	}
	Result.clear();
	Result.reserve(100);
	while(page_no <= total_page)
	{
		while(fscanf(Length_Type,"%d",&length)==1)
		{
			c->Clear();
			f->Clear();
			memset(temp,0,sizeof(temp));
			readBytes(bmMatch,temp,length,page,page_no,offset);
			i=1;
			j=0;
			while(temp[i]!=' ')
			{
				c->id[j]=temp[i];
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				c->name[j]=temp[i];
				i++;
				j++;
			}
			if(strcmp(c->name,"Vern_Graham")==0)
			{
				j+=0;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				c->type[j]=temp[i];
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				f->id[j]=temp[i];
				i++;
				j++;
			}
			i++;
			j=0;
			while(temp[i]!=' ')
			{
				f->name[j]=temp[i];
				i++;
				j++;
			}
			i++;
			j=0;
			while(i<(int)strlen(temp))
			{
				f->type[j]=temp[i];
				i++;
				j++;
			}
			//c->father=f;
			//Concept *fcopy=new Concept(f->id,f->name,f->type,f->father);
			//Concept *ccopy=new Concept(c->id,c->name,c->type,fcopy);
			//Result.push_back(ccopy);
			//printf("%s\n",temp);
			//return temp;
			if(strcmp(lastRecord,c->name)==0)
			{
				Concept *fcopy=new Concept(f->id,f->name,f->type,f->father);
				Concept *ccopy=new Concept(c->id,c->name,c->type,fcopy);
				Result.push_back(ccopy);
			}
			else
			{
				//printf("%s\n",lastRecord);
				if(strcmp(lastRecord,"")!=0)
				{
					for(vector<Concept*>::iterator Iter=ListTarget.begin();Iter!=ListTarget.end();Iter++)
					{
						Concept *fcopy=new Concept((*Iter)->father->id,(*Iter)->father->name,(*Iter)->father->type,(*Iter)->father->father);
						Concept *ccopy=new Concept((*Iter)->id,(*Iter)->name,(*Iter)->type,fcopy);
						Result.push_back(ccopy);
					}
					match(target,lastRecord,threshold);
				}
				memset(lastRecord,0,sizeof(lastRecord));
				strcpy(lastRecord,c->name);
				for(vector<Concept*>::iterator Iter=Result.begin();Iter!=Result.end();Iter++)
				{
					delete (*Iter)->father;
					delete *Iter;
				}
				for(vector<Road*>::iterator Iter=RoadList.begin();Iter!=RoadList.end();Iter++)
				{
					//delete (*Iter)->start->father;
					delete (*Iter)->start;
					//delete (*Iter)->end->father;
					delete (*Iter)->end;
					delete *Iter;
				}
				Result.clear();
				RoadList.clear();
				Result.reserve(100);
				RoadList.reserve(10000);
				Concept *fcopy=new Concept(f->id,f->name,f->type,f->father);
				Concept *ccopy=new Concept(c->id,c->name,c->type,fcopy);
				Result.push_back(ccopy);
				//printf("%s\n",temp);
				//return temp;
			}
		}
	}
	fclose(Length_Type);
}

void initFreebase()
{
	getTypes();
	getSubClassOf();
}

int main()
{
	initHash(num_page); //init hash function
	char Target[800]="A";
	int Threshold=4;
	time_t Start1,End1;
	//time_t Start2,End2;
	//initFreebase();


	time(&Start1);
	join1(Target,Threshold);
	time(&End1);
	printf("Join 1 Used Time is:%d\n",(int)difftime(End1,Start1));

	/*time(&Start2);
	join2(Target,Threshold);
	time(&End2);
	printf("Join 2 Used Time is:%d\n",(int)difftime(End2,Start2));*/

	return 0;
}
