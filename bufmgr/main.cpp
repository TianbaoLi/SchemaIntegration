#include "buf_common.h"
#include "db.h"
#include "bufmgr.h"
#include "fileManager.h"
#include "index_Manager.h"
#include "stdio.h"
#include "time_test.h"
#include "queryParser.h"
#include "queryProcessor.h"
#include "hash.h"
#include "time.h"

#define QUERY_LENGTH 200

void main(int argc, void* argv[]){

/*	if(argc < 5){
		printf("too little paremeters!!!!\n");
		exit(0);
	}*/

	int page_size=atoi((char*)argv[2]);
	int page_num = atoi((char*)argv[3]);
	initHash(page_num);

	if(((char*)argv[1])[0]=='s'){//s is to store XML document
		//the first number is page size, the second number is page number
		//the third parameter is filename
		index_Manager* im = new index_Manager;
		im->convert_store((char*)argv[4],1024*1024,"index.xml","extent.dat",page_num,page_size);
		delete im;	
	}
	else if(((char*)argv[1])[0]=='q'){//q is to execute query
		//the first number is page size, the second number is page number
		//the third parameter is the file contains all the queries

		//int page_size=atoi((char*)argv[2]);
		//int page_num = atoi((char*)argv[3]);
		queryProcessor* qp = new queryProcessor("extent.dat", page_size, page_num);
		qp->loadIndex("index.xml");
		char query[QUERY_LENGTH];
		FILE* in = fopen((char*)argv[4],"r");
		FILE* out = fopen("result.txt","w");
		time_test tm;
		while(!feof(in)){
			fscanf(in,"%s",query);
			printf("%s\n",query);
			fprintf(out,"%s\n",query);
			tm.time_begin();
			qp->query(query);
			tm.time_end();
			printf("run time:%d\n",tm.result);
			fprintf(out,"run time:%d\n",tm.result);
			fprintf(out,"Physical IO:%d\n\n",qp->getPhysicalIO());
		}

		fclose(in);
		fclose(out);
		delete qp;
	}
}