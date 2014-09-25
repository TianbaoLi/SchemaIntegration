// DB.h: interface for the DB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DB_H__43B042BC_61D4_4A8A_9142_58B7A57419C8__INCLUDED_)
#define AFX_DB_H__43B042BC_61D4_4A8A_9142_58B7A57419C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DB  
{
public:
	DB(const char* name, int num_pages, int page_size);
	virtual ~DB();

	void write_page(PageID pageno, Page* pageptr);
	void read_page(PageID pageno, Page* pageptr);
private:
	int fd;//the handle of file
	int num_pages;
	int page_size;

};

#endif // !defined(AFX_DB_H__43B042BC_61D4_4A8A_9142_58B7A57419C8__INCLUDED_)
