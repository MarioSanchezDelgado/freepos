/*
 *     Module Name       : MEM_MGR.C
 *
 *     Type              : Memory manager modules
 *                         
 *
 *     Author/Location   : Getronics, Distribution & Retail, Nieuwegein
 *
 *     Copyright Makro International AG
 *               Aspermonstrasse 24
 *               7006 CHUR
 *               Switzerland
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 15-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * 10-Nov-2000 Bugfix in function give_time_to_OS(). DispatchMessage()
 *             is only called when option == PM_REMOVE.                J.D.M.
 * --------------------------------------------------------------------------
 * 11-Dec-2000 Added Sleep to give_time_to_OS to save CPU time         R.N.B.
 * --------------------------------------------------------------------------
 * 12-Dec-2000 mem_free : Don't free pointer if NULL                     M.W.
 * --------------------------------------------------------------------------
 * 01-Oct-2002 Removed 'far' from the pointers                         J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "err_mgr.h"
#include "mem_mgr.h"


/******************************************************************************
Function    : mem_allocate                                     
Description : Allocate memory
INPUT       : - unsigned short size : how much memory has to be allocated
OUTPUT      : - void * MEM_UNAVAILABLE, could not be enough memory allocated
                memory block which is allocated, otherwise
*******************************************************************************/

void *mem_allocate( unsigned int size )
{
  void *memblock = malloc( size );                  /* try to allocate memory */

  if (memblock == NULL) {                    /* if failed, give error message */
    err_invoke( MEM_UNAVAILABLE );
  }
  /*_heapmin()*/ /* Releases unused heap memory to the operating system.*/
  return(memblock);
}


/******************************************************************************
Function    : mem_reallocate
Description : Reallocate memory
INPUT       : - void * prt          : old pointer
            : - unsigned short size : how much memory has to be reallocated
OUTPUT      : - void * MEM_UNAVAILABLE, could not be enough memory allocated
                memory block which is allocated, otherwise
*******************************************************************************/

void *mem_reallocate(void * ptr, unsigned int size)
{
  void *memblock = realloc( ptr, size );
  if (memblock == NULL) {                    /* if failed, give error message */
    err_invoke( MEM_UNAVAILABLE );
  }
  /*_heapmin()*/ /* Releases unused heap memory to the operating system.*/
  return(memblock);
}


/******************************************************************************
Function    : mem_free
Description : Free allocated memory
INPUT       : - void *ptr : pointer to memory which was allocated
OUTPUT      : - void , no returning value
******************************************************************************/

void mem_free(void *ptr)
{
  if (ptr) {
    free(ptr);
    /*_heapmin()*/ /* Releases unused heap memory to the operating system.*/
  }
}


/**************************************************************************/
/*                          GIVE_TIME_TO_OS                               */
/**************************************************************************/

void give_time_to_OS(long option, long sleep_msecs)
{
  MSG msg;

  /* Option can be PM_REMOVE or PM_NOREMOVE */
  if (PeekMessage(&msg, NULL, 0, 0, option) ) {
    TranslateMessage(&msg);
    if(option == PM_REMOVE) {
      DispatchMessage(&msg);
    }
  }

  if (sleep_msecs) {
#ifdef _DEBUG
    if(sleep_msecs < 50) {
      sleep_msecs *= 10; /* in debug mode more sleep time is needed relatively */
    }
#endif
    Sleep(sleep_msecs);
  }
  return;
} /* give_time_to_OS */
