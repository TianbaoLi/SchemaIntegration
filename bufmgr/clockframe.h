#ifndef _CLOCKFRAME_H_
#define _CLOCKFRAME_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "buf_frame.h"

class  ClockFrame : public Frame 
{
	private :
		
		BOOL referenced;

 	public :

		ClockFrame(Page* pg, DB* use_db);
		~ClockFrame();
	
		void Unpin();
		Status Free();
		void UnsetReferenced();
		inline BOOL IsReferenced() 
    {
      return (referenced == (int)TRUE);
    }

    inline BOOL IsVictim()
    {
	    return ( !referenced && NotPinned());
    }
};

#endif