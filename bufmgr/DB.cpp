// DB.cpp: implementation of the DB class.
//
//////////////////////////////////////////////////////////////////////

#include"stdio.h"
#include"stdlib.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "io.h"
#include"buf_common.h"
#include "DB.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DB::DB(const char* name, int in_num_pages, int in_page_size)
{
	  // Create the file; fail if it's already there; open it in read/write
      // mode.
//    fd = open( name, O_RDWR | O_CREAT | O_EXCL, 0666 );
	fd = open( name, O_RDWR | O_CREAT|O_BINARY, 0666 );
//	fd = open( name, _A_NORMAL, 0666 );
	num_pages = in_num_pages;
	page_size = in_page_size;

    if ( fd < 0 ) {
		printf("error in open file.\n");
		return;
    }
}

DB::~DB()
{
	close( fd );
    fd = -1;
}

void DB::read_page(PageID pageno, Page* pageptr)
{
    if ( pageno < 0 || pageno >= (int)num_pages ){
		printf("No such page\n");
		return;
	}

      // Seek to the correct page
	lseek( fd, pageno*page_size, SEEK_SET );

      // Read the appropriate number of bytes.
	read(fd, pageptr, page_size);
}

void DB::write_page(PageID pageno, Page* pageptr)
{
    if ( pageno < 0 || pageno >= (int)num_pages ){
		printf("No such page\n");
		return;
	}

     // Seek to the correct page
    lseek( fd, pageno*page_size, SEEK_SET );

    // Write the appropriate number of bytes.
    write( fd, pageptr, page_size );

}

