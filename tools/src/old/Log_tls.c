/*
 *     Module Name       : LOG_TLS.C
 *
 *     Type              : Logging to file
 *
 *     Author/Location   : Pedro Meuwissen, Getronics Nieuwegein
 * 
 *     Copyright Makro International AG
 *               Aspermonstrasse 24
 *               7006 CHUR
 *               Switzerland
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE       REASON                                         CALL #   AUTHOR
 * --------------------------------------------------------------------------
 * Mar-28-2000 Initial Release                                         J.D.M.
 * May-01-2000 Changed MAX_FILE_NO and MAX_NO_DPRINTFS because the
 *             tills were rebooted too many times and log files
 *             (used for logging of EFT errors) were overwritten.      J.D.M.
 *             Added the parameter 'fname' again to dprintf.           J.D.M.
 * Oct-25-2000 Made MAX_FILE_NO and MAX_NO_DPRINTFS flexible through
 *             the function dinit().                                   J.D.M.
 * Jan-18-2001 Solved bug in dinit(): it crashed when no DATADIR.      R.N.B.
 * Apr-27-2001 Solved bug in dinit(): only fprintf when the filepointer
 *             exists.                                                 J.D.M.
 * --------------------------------------------------------------------------
 */

/************************************************************************
 * This source makes it possible to log information to disk while       *
 * the POS application is running.                                      *
 * After the call dinit() a new file with a higher number in its        *
 * name is opened to write information to. This also happenes when      *
 * the number of dprintfs has reached MAX_NO_DPRINTFS.                  *
 * When the number in the file has reached MAX_FILE_NO the first        *
 * file will be overwritten again. This means that logging is circular. *
 * The last filenumber used is written into a file called 'lastfno'.    *
 * The function dprintf() always writes to the last file initialized    *
 * with dinit().                                                        *
 ************************************************************************/

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>

#include "Log_tls.h"
#include "Date_tls.h"
#include "Intrface.h"

#define BUF_FN 100
static _TCHAR debugfile_nm[BUF_FN];
static _TCHAR name_ext[BUF_FN+10];
static short file_no = 1;       /* Initially we have file number 1. */
static short log_init = FALSE;

static double MaxNoDprintfs;             /* Maximum dprintfs per log file. */
static short  MaxFileNo;                   /* Maximum number of log files. */
#define MAX_NO_DPRINTFS   1000000.0     /* Max Boundary for MaxNoDprintfs. */
#define MIN_NO_DPRINTFS   100.0         /* Min Boundary for MaxNoDprintfs. */
#define MAX_FILE_NO       999               /* Max Boundary for MaxFileNo. */
#define MIN_FILE_NO       1                 /* Min Boundary for MaxFileNo. */
#define MAX_LEN_FILENAME  12 /* Maximum characters to be copied from fname */
                             /* in the function dprintf().                 */

/*-------------------------------------------------------------------------*/
/*                              dinit                                      */
/*-------------------------------------------------------------------------*/
short dinit(_TCHAR *filename, short max_file_no, double max_no_dprintfs)
{
  FILE  *f;
  FILE  *lastfno;

  log_init = FALSE; /* Used in dprintf to determine if logging is allowed */

  if(_tcslen(filename) >= (short)BUF_FN) {
    return(FAIL);
  }

  MaxFileNo = max_file_no;
  if(MaxFileNo > MAX_FILE_NO) {
    MaxFileNo = MAX_FILE_NO;
  }
  if(MaxFileNo < MIN_FILE_NO) {
    MaxFileNo = MIN_FILE_NO;
  }

  MaxNoDprintfs = max_no_dprintfs;
  if(MaxNoDprintfs > MAX_NO_DPRINTFS) {
    MaxNoDprintfs = MAX_NO_DPRINTFS;
  }
  if(MaxNoDprintfs < MIN_NO_DPRINTFS) {
    MaxNoDprintfs = MIN_NO_DPRINTFS;
  }

  if(MaxFileNo == /*Hardcoded!->*/1) {
    MaxNoDprintfs = MAX_NO_DPRINTFS;
  }

  lastfno = _tfopen(DATADIR _T("LASTFNO.TXT"), _T("r"));
  if(lastfno) {
    _ftscanf(lastfno, _T("%d"), &file_no);
    file_no++;
    if(file_no > MaxFileNo) {
      file_no = 1;
    }
    fclose(lastfno);
  }

  /* save current file number */
  lastfno = _tfopen(DATADIR _T("LASTFNO.TXT"), _T("w"));
  if(lastfno) {
    _ftprintf(lastfno, _T("%d"), file_no);
    fclose(lastfno);
  }
  else {
    return(FAIL);
  }

  _stprintf(debugfile_nm, _T("%s"), filename);
  _stprintf(name_ext, _T("%s%03d.TXT"), debugfile_nm, file_no);
  f = _tfopen(name_ext, _T("w"));
  if(f) {
    _ftprintf(f, _T("-------------- NEW LOG FILE -------------\n"));
    _ftprintf(f, _T("\n"));
    _ftprintf(f, _T("---------- START OF EXECUTION -----------\n"));
    fclose(f);
  }
  else {
    return(FAIL);
  }

  log_init = TRUE; /* Used in dprintf to determine if logging is allowed */
  return(SUCCEED);
} /* dinit */

/*-------------------------------------------------------------------------*/
/*                              dprintf                                    */
/*-------------------------------------------------------------------------*/
/* Examples of dprintf():                                                  */
/*                                                                         */
/* dprintf((_TCHAR*)0, 0, _T("Comment"));                                  */
/* dprintf((_TCHAR*)__FILE__, 0, _T("Time: %s", timestamp()));             */
/* dprintf((_TCHAR*)__FILE__, __LINE__, _T("i=%d", i));                    */
/* dprintf((_TCHAR*)0, __LINE__, _T("Hello world"));                       */
/*-------------------------------------------------------------------------*/
void dprintf(_TCHAR *fname, long line, _TCHAR *c, ...)
{
  va_list        args;
  FILE          *f;
  FILE          *lastfno;
  static double  no_dprintfs=0;
  _TCHAR         buffer[MAX_LEN_FILENAME+1];
  short          length_fname, i, j;

  if(log_init == FALSE) { /* No init -> No logging! */
    return;
  }

  f=_tfopen(name_ext, _T("a"));

  if(fname) {
    length_fname = _tcslen(fname);
    ch_memset(buffer, 0, sizeof(buffer));

    for(i=length_fname-1, j=0;
        i>=0 && j<MAX_LEN_FILENAME && *(fname+i) != _T('\\') && *(fname+i) != _T(':');) {
      buffer[j++]=fname[i--];
    }
    buffer[j]=_T('\0');
    _ftprintf(f, _T("%-*.*s"), MAX_LEN_FILENAME, MAX_LEN_FILENAME, _tcsrev(buffer));

    if(line) {
      _ftprintf(f, _T(", ")); /* Add separator before the line number */
    }
  }

  if(line) {
    _ftprintf(f, _T("%05d"), line);
  }

  if(fname || line) {
    _ftprintf(f, _T(": ")); /* Add separator before the debug information */
  }

  va_start(args, c);
  _vftprintf(f, c, args);
  va_end(args);
  _ftprintf(f, _T("\n"));

  no_dprintfs++;
  if(no_dprintfs > MaxNoDprintfs) {

    no_dprintfs=0;
    file_no++;
    if(file_no > MaxFileNo) {
      file_no = 1;
    }

    /* save new file number */
    lastfno = _tfopen(DATADIR _T("LASTFNO.TXT"), _T("w"));
    if(lastfno) {
      _ftprintf(lastfno, _T("%d"), file_no);
      fclose(lastfno);
    }

    /* Determine new filename */
    _stprintf(name_ext, _T("%s%03d.TXT"), debugfile_nm, file_no);

    /* close current file */
    if(f) {
      _ftprintf(f, _T("--------------- END OF FILE -------------\n"));
      _ftprintf(f, _T("--------- CONTINUED IN %s ---------\n"), name_ext);
      fclose(f);
    }

    /* and open the new one */
    f = _tfopen(name_ext, _T("w"));
    _ftprintf(f, _T("---------- CONTINUEING EXECUTION --------\n"));
  }

  if(f) {
    fclose(f);
  }
} /* dprintf */
