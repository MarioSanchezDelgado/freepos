/*
 *     Module Name       : TM_MGR.C
 *
 *     Type              : Transaction Manager
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
 * 14-Dec-1999 Initial Release WinPOS                                    P.M.
 * --------------------------------------------------------------------------
 * 10-Jan-1999 Added function tm_remv                                    P.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "registry.h"
#include "mem_mgr.h"
#include "tm_mgr.h"

struct TM_POINTERS {
  long    offset;        /* where the first structure of this group starts */
  long    space;         /* total space occupied by this group structure   */
  int     size;          /* size of the structure                          */
  int     max;           /* maximum allowed for this structures            */
  TM_INDX count;         /* number of current written structures           */
  TM_INDX cur;           /* last accessed OR last tried number             */
  TM_NAME name;          /* structure identifier                           */
};

static struct TM_POINTERS tm_ptr[TM_MAX_GROUPS];
static int    tm_ptr_cur = 0;                /* current initialized groups */

short   tm_search_entry(TM_NAME name);
long    tm_calc_fpos(short entry);
TM_INDX tm_getr(short entry, void *data);
short   tm_writ(long pos, void *data, int len);
short   tm_read(long pos, void *data, int len);

static BYTE *tm_area=NULL;
static long  TM_RAMSIZE = 0;

/*-------------------------------------------------------------------------*/
/*                         tm_define_struct                                */
/*-------------------------------------------------------------------------*/
  short
tm_define_struct(TM_NAME name, int max, int size)
{
  if (tm_ptr_cur >= TM_MAX_GROUPS) {
    return(TM_2MANY_GROUPS);
  }

  memset(&tm_ptr[tm_ptr_cur], 0, sizeof(struct TM_POINTERS));

  tm_ptr[tm_ptr_cur].size = size;
  tm_ptr[tm_ptr_cur].max  = max;
  tm_ptr[tm_ptr_cur].count= 0;
  tm_ptr[tm_ptr_cur].cur  = 0;
  tm_ptr[tm_ptr_cur].name = name;
  tm_ptr[tm_ptr_cur].space= (long)max * (long)size;
  if (tm_ptr_cur == 0) {
    tm_ptr[tm_ptr_cur].offset = 0;
  }
  else {
    tm_ptr[tm_ptr_cur].offset  = tm_ptr[tm_ptr_cur - 1].offset;
    tm_ptr[tm_ptr_cur].offset += tm_ptr[tm_ptr_cur - 1].space;
  }

  tm_ptr_cur++;

  return(0);
} /* tm_define_struct */

/*-------------------------------------------------------------------------*/
/*                             tm_exit                                     */
/*-------------------------------------------------------------------------*/
void tm_exit(void)
{
  mem_free(tm_area);
} /* tm_exit */

/*-------------------------------------------------------------------------*/
/*                             tm_init                                     */
/* - call AFTER all tm structs have been defined using tm_define_struct()  */
/*-------------------------------------------------------------------------*/
short tm_init(void)
{
  short i;

  if ( tm_area == NULL ) {
                                    /* calculate the memory that is needed */
    TM_RAMSIZE = 0;
    for (i=0; i<tm_ptr_cur; i++) {
      TM_RAMSIZE += tm_ptr[i].space;
    }
    TM_RAMSIZE += 100;                       /* pad a few bytes to be sure */
                                      /* try to allocate the needed memory */
    tm_area = (BYTE *)mem_allocate(TM_RAMSIZE);
    if ( tm_area == NULL) {
      return (MEM_UNAVAILABLE);
    }
  }

  memset((void*)tm_area, 0, TM_RAMSIZE);

  return(SUCCEED);
} /* tm_init */

/*-------------------------------------------------------------------------*/
/*                         tm_reset_struct                                 */
/*-------------------------------------------------------------------------*/
  short
tm_reset_struct(TM_NAME name)
{
  short entry;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  tm_ptr[entry].count = 0;
  tm_ptr[entry].cur = 0;
  return(0);
} /* tm_reset_struct */

/*-------------------------------------------------------------------------*/
/*                         tm_upda_nth                                     */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_upda_nth(TM_NAME name, void *data, TM_INDX indx)
{
  short entry, status;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  if (indx > tm_ptr[entry].count) {               /* not there (yet?)      */
    return(TM_BOF_EOF);
  }
  tm_ptr[entry].cur = indx;
  status = tm_writ(tm_calc_fpos(entry), data, tm_ptr[entry].size);
  if (status < 0) {
    return(status);
  }
  return(tm_ptr[entry].cur);                      /* return current index  */
} /* tm_upda_nth */

/*-------------------------------------------------------------------------*/
/*                         tm_read_nth                                     */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_read_nth(TM_NAME name, void *data, TM_INDX indx)
{
  short entry;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  if (indx > tm_ptr[entry].count) {               /* not there (yet?)      */
    return(TM_BOF_EOF);
  }
  tm_ptr[entry].cur = indx;
  return(tm_getr(entry, data));
} /* tm_read_nth */

/*-------------------------------------------------------------------------*/
/*                         tm_appe                                         */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_appe(TM_NAME name, void *data)
{
  short entry, status;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  if (tm_ptr[entry].count >= (TM_INDX)tm_ptr[entry].max) {
    return(TM_2MANY_RECORDS);
  }
  tm_ptr[entry].count++;
  tm_ptr[entry].cur = tm_ptr[entry].count;
  status = tm_writ(tm_calc_fpos(entry), data, tm_ptr[entry].size);
  if (status < 0) {
    return(status);
  }
  return(tm_ptr[entry].cur);                      /* return current index  */
} /* tm_appe */

/*-------------------------------------------------------------------------*/
/*                             tm_remv                                     */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_remv(TM_NAME name)
{
  short entry;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  if (tm_ptr[entry].count < 1) {
    return(tm_ptr[entry].cur);
  }
  tm_ptr[entry].count--;
  tm_ptr[entry].cur = tm_ptr[entry].count;
  return(tm_ptr[entry].cur);                      /* return current index  */
} /* tm_remv */


/*-------------------------------------------------------------------------*/
/*                         tm_upda                                         */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_upda(TM_NAME name, void *data)
{
  short entry, status;

  if ((entry = tm_search_entry(name)) < 0) {       /* search NAME in tm_ptr */
    return(entry);
  }
  status = tm_writ(tm_calc_fpos(entry), data, tm_ptr[entry].size);
  if (status < 0) {
    return(status);
  }
  return(tm_ptr[entry].cur);                      /* return current index  */
} /* tm_upda */

/*-------------------------------------------------------------------------*/
/*                         tm_frst                                         */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_frst(TM_NAME name, void *data)
{
  short entry;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  if (tm_ptr[entry].count == 0) {
    return(TM_BOF_EOF);
  }
  tm_ptr[entry].cur = 1;
  return(tm_getr(entry, data));
} /* tm_frst */

/*-------------------------------------------------------------------------*/
/*                         tm_last                                         */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_last(TM_NAME name, void *data)
{
  short entry;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  if (tm_ptr[entry].count == 0) {
    return(TM_BOF_EOF);
  }
  tm_ptr[entry].cur = tm_ptr[entry].count;
  return(tm_getr(entry, data));
} /* tm_last */

/*-------------------------------------------------------------------------*/
/*                         tm_next                                         */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_next(TM_NAME name, void *data)
{
  short entry;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  if (tm_ptr[entry].cur >= tm_ptr[entry].count) {
    return(TM_BOF_EOF);
  }
  tm_ptr[entry].cur++;
  return(tm_getr(entry, data));
} /* tm_next */

/*-------------------------------------------------------------------------*/
/*                         tm_prev                                         */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_prev(TM_NAME name, void *data)
{
  short entry;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  if (tm_ptr[entry].cur <= 1) {
    return(TM_BOF_EOF);
  }
  tm_ptr[entry].cur--;
  return(tm_getr(entry, data));
} /* tm_prev */

/*-------------------------------------------------------------------------*/
/*                         tm_coun                                         */
/*-------------------------------------------------------------------------*/
  int
tm_coun(TM_NAME name)
{
  short entry;

  if ((entry = tm_search_entry(name)) < 0) {      /* search NAME in tm_ptr */
    return(entry);
  }
  return(tm_ptr[entry].count);                    /* return nr of structs  */
} /* tm_coun */

/*-------------------------------------------------------------------------*/
/*                         tm_search_entry                                 */
/*-------------------------------------------------------------------------*/
  short
tm_search_entry(TM_NAME name)
{
  short i;

  for (i = 0; i < tm_ptr_cur; i++) {
    if (tm_ptr[i].name == name) {
      return(i);
    }
  }
  return(TM_UNDEFINED);
} /* tm_search_entry */

/*-------------------------------------------------------------------------*/
/*                         tm_calc_fpos                                    */
/*-------------------------------------------------------------------------*/
  long
tm_calc_fpos(short entry)
{
  long pos;

  pos  = ((long)tm_ptr[entry].cur - 1) * (long)tm_ptr[entry].size;
  pos += tm_ptr[entry].offset;
  return(pos);
} /* tm_calc_fpos */

/*-------------------------------------------------------------------------*/
/*                         tm_getr                                         */
/*-------------------------------------------------------------------------*/
  TM_INDX
tm_getr(short entry, void *data)
{
  short status;

  status = tm_read(tm_calc_fpos(entry), data, tm_ptr[entry].size);
  if (status < 0) {
    return(status);
  }
  return(tm_ptr[entry].cur);                      /* return current index  */
} /* tm_getr */

/*-------------------------------------------------------------------------*/
/*                         tm_writ                                         */
/*-------------------------------------------------------------------------*/
  short
tm_writ(long pos, void *data, int len)
{
  if ( tm_area == NULL) {
    return(TM_IO_ERROR);
  }

  if ( pos+len > TM_RAMSIZE) {
    return(TM_IO_ERROR);
  }

  memcpy((void*)(tm_area + pos), data, len);

  return 0;
} /* tm_writ */

/*-------------------------------------------------------------------------*/
/*                         tm_read                                         */
/*-------------------------------------------------------------------------*/
  short
tm_read(long pos, void *data, int len)
{
  if ( tm_area == NULL) {
    return(TM_IO_ERROR);
  }

  if ( pos+len > TM_RAMSIZE) {
    return(TM_IO_ERROR);
  }

  memcpy(data, (void*)(tm_area + pos), len);

  return 0;
} /* tm_read */
