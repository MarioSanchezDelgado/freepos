/*
 *     Module Name       : BACK_TLS.H
 *
 *     Type              : Functions to maintain the invoice backlog file
 *                         on a check-out
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
 * 13-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 * 03-Feb-2003 If backlog is corrupt, till must be closed. File will be 
 *             renamed in <backlog_name>.BAD.<date>.<time>.              M.W.
 * --------------------------------------------------------------------------
 */

#ifndef _back_tls_h_
#define _back_tls_h_

#include "storerec.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TAG                    500

struct IBL_HEADER {
  long wpos;
  long rpos;
  long dps1;                          /* delete position shift        */
  long dps2;                          /* delete position invoice      */
  long spos;                          /* current shift tag position   */
  long ipos;                          /* current invoice tag position */
};

struct IBL_PREFIX {
  int type;
  int size;
};

/*************************************************************/
/* NOTE: MAX_IBL_RECORD_DATA_SIZE is calculated in storerec.h*/
/*************************************************************/
struct IBL_RECORD {
  int    type;
  int    size;
  BYTE   data[MAX_IBL_RECORD_DATA_SIZE];
};

struct IBL_TAG {
  long bpos;                             /* begin position            */
  long epos;                             /* end position              */
  int  type;                             /* shift or invoice          */
  int  deleted;
  long key1;                             /* invoice number or date_on */
  long key2;                             /* time_on                   */
};

/* exported functions */
extern int   open_backlog(_TCHAR *);
extern void  close_backlog(void);
extern short writ_backlog_header(void);
extern void  resetbacklog(void);
extern int   add2backlog(enum INVTYPE, int, void *);
extern int   addshifttag  (long, short);
extern int   addinvoicetag(long);
extern int   updshifttag  (void);
extern int   updinvoicetag(void);
extern int   delshifttag  (long, long);
extern int   delinvoicetag(long);
extern void  getheaderinfo(long *);
extern int   getshifton (void *, int, int);
extern int   getshiftoff(void *, int, int);
extern int   getbacklog (int *, void *);
extern int   scanbacklog(int *, void *);
extern int   switch_backlog(int, long, short, _TCHAR *);
extern void  rename_corrupt_backlog(void);

#ifdef __cplusplus
}
#endif

#endif
