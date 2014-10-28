#define _CRT_SECURE_NO_WARNINGS
#include "buf_common.h"
#include "db.h"
#include "bufmgr.h"
#include <cstdio>
#include <vector>
#include <ctime>

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

//vector <int> Length_SubClassOf;
//vector <int> Length_Type;
vector <Concept*> Result;
vector <Road*> RoadList;

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
int page_no_SubClassOf = 0;
int offset_SubClassOf = 0;
int page_no_Type = 0;
int offset_Type = 0;


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
	/*FILE *Out;
	Out=fopen("SubClassOf.txt","w");*/
	char temp[500],result[500];
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
		for(int k=0;k<(int)strlen(result);k++)
			writeBytes(bmSubClassOf, page, &result[k], 1, page_no_SubClassOf, offset_SubClassOf);
		//Length_SubClassOf.push_back(length);
		fprintf(Length_SubClassOf,"%d\n",length);
		//printf("%s\n\n",temp);
		//system("pause");
	}
	memset(page+offset_SubClassOf,0,page_size-offset_SubClassOf);
	bmSubClassOf->UnpinPage(page_no_SubClassOf,true);
	delete f;
	delete c;
	fclose(Length_SubClassOf);
	//fclose(Out);
	//system("pause");
}

void getTypes()
{
	FILE *TypeIn,*Length_Type;
	TypeIn=fopen("rdfTypes.tsv","r");
	Length_Type=fopen("Length_Type.txt","w");
	bmType->PinPage(page_no_Type, page);
	/*FILE *Out;
	Out=fopen("Types.txt","w");*/
	char temp[1000],result[1000];
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
		for(int k=0;k<(int)strlen(result);k++)
			writeBytes(bmType, page, &result[k], 1, page_no_Type, offset_Type);
		//Length_Type.push_back(length);
		fprintf(Length_Type,"%d\n",length);
		//printf("%s",result);
		//printf("%s\n\n",temp);
		//fprintf(Out,"%s",result);
		//system("pause");
	}
	memset(page+offset_Type,0,page_size-offset_Type);
	bmType->UnpinPage(page_no_Type,true);
	delete f;
	delete c;
	fclose(Length_Type);
	//fclose(Out);
	//system("pause");
}

void readTypes(char target[800],BufMgr *bm,int num_page,int page_size)
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

bool judge(char target1[800],char target2[800],int threshold)
{
	for(vector<Road*>::iterator Iter1=RoadList.begin();Iter1!=RoadList.end();Iter1++)
		if(strcmp(target1,(*Iter1)->start->name)==0)
			for(vector<Road*>::iterator Iter2=RoadList.begin();Iter2!=RoadList.end();Iter2++)
				if(strcmp(target2,(*Iter2)->start->name)==0)
					if(strcmp((*Iter1)->end->name,(*Iter2)->end->name)==0&&(*Iter1)->length+(*Iter2)->length<=threshold)
					{
						printf("GOT\n");
						printf("Length is:%d\n",(*Iter1)->length+(*Iter2)->length);
						printf("Common Father is:%s\n",(*Iter1)->end->name);
						return 1;
					}
	return 0;
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


void join(char target1[800],char target2[800],int threshold)
{
	Result.reserve(100);
	RoadList.reserve(10000);
	int len=0;
	readTypes(target1,bmType,page_no_Type,page_size);
	if(len==Result.size())
	{
		printf("Didn`t Find Target1!\n");
		return;
	}
	len=Result.size();
	readTypes(target2,bmType,page_no_Type,page_size);
	if(len==Result.size())
	{
		printf("Didn`t Find Target2!\n");
		return;
	}
	len=Result.size();
	while(Result.empty()==0)
	{
		Concept *c=Result.front();
		Result.erase(Result.begin());
		RoadList.push_back(new Road(c,c->father,1));
	}
	if(judge(target1,target2,threshold)==1)
			return;
	while(1)
	{
		if(judge(target1,target2,threshold)==1)
			return;
		bool flag=1;
		for(vector<Road*>::iterator Iter1=RoadList.begin();Iter1!=RoadList.end();Iter1++)
			if((*Iter1)->length<=threshold)
			{
				flag=0;
				break;
			}
		if(flag==1)
			return;
		if(readSubClassOf(bmSubClassOf,page_no_SubClassOf,page_size,target1,target2,threshold)==1)
			return;
	}
}

void initFreebase()
{
	getTypes();
	getSubClassOf();
}

int main()
{
	initHash(num_page); //init hash function
	char Target1[800]="Abraham_Lincoln";
	char Target2[800]="Deanna_C._C._Peluso";
	int Threshold=20;
	time_t Start,End;
	//initFreebase();
	time(&Start);
	join(Target1,Target2,Threshold);
	time(&End);
	printf("Used Time is:%d\n",(int)difftime(End,Start));
	return 0;
}
