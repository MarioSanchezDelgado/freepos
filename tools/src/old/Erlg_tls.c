/*
 *     Module Name       : ERLG_TLS.C
 *
 *     Type              : Functions to write the network error-log file
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
 * 13-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 * 15-Nov-2000 Replaced localtime by GetLocalTime                      R.N.B.
 * --------------------------------------------------------------------------
 * 23-Jan-2001 size P12.LOG extended to 100K                             M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <io.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
                                     /* POS.lib include files              */
#define ERLG_INTERN                  /* because of the varargs declaration */
#include "erlg_tls.h"
#include "stri_tls.h"

extern short err_handle;             /* file handle owned by stnetp12      */

#define LOGFILESIZE  102400          /* WinPOS logfile 100K (DosPOS 8K)    */
                                     /* local 'hidden' functions           */
static void  err_disp(int, _TCHAR *);
static _TCHAR *sys_err_str(void);

/*--------------------------------------------------------------------*/
/* err_disp()                                                         */
/*--------------------------------------------------------------------*/
static void err_disp(int show, _TCHAR *mess)
{

  static long fpos = 0;
  long    sstatus;
  BYTE    date[30];
  int     len, status;
  BYTE   *mess_ansi;
  SYSTEMTIME st;

  if (err_handle == 0) {             /* no file present                    */
    return;
  }
  
  GetLocalTime(&st);

  sprintf(date, "%4d-%02d-%02d/%02d:%02d:%02d",
                st.wYear, st.wMonth, st.wDay,
                st.wHour, st.wMinute, st.wSecond);
  strcat(date, " ");
  sstatus = lseek(err_handle, fpos, SEEK_SET);
  len = strlen(date);                /* number of bytes */
  status = _write(err_handle, date, len);
  fpos += len;

  _tcscat(mess, _T("\n<eof>\n"));
  mess_ansi = UnicodeToAnsi(mess);       /* convert to ansi string because */
                                         /* bytes are written              */
  len = strlen(mess_ansi);
  status = _write(err_handle, mess_ansi, len);
  fpos += len - 5;
  _close(_dup(err_handle));
  if (fpos > LOGFILESIZE) {          /* wrap around                        */
    fpos = 0;
  }
} /* err_disp */


/*--------------------------------------------------------------------*/
/* sys_err_str()                                                      */
/*--------------------------------------------------------------------*/

static _TCHAR *sys_err_str(void)
{
  static _TCHAR  msgstr[200];
  int   error_number;

  error_number = GetLastError();
  if (error_number != 0) {
      _stprintf(msgstr, _T(" (Error_number = %d)"), error_number);
  }
  else {
    msgstr[0] = _T('\0');
  }

  return(msgstr);
} /* sys_err_str */


/*--------------------------------------------------------------------*/
/* err_sys()                                                          */
/*--------------------------------------------------------------------*/

void err_sys(_TCHAR *first, ...)
{
  va_list argp;
  _TCHAR  emesgstr[255] = {0};

  va_start(argp, first);
  _vstprintf(emesgstr, first, argp);
  va_end(argp);
  _tcscat(emesgstr, sys_err_str());
  err_disp(1, emesgstr);

  exit(1);
} /* err_sys */


/*--------------------------------------------------------------------*/
/* err_shw()                                                          */
/*--------------------------------------------------------------------*/

void err_shw(_TCHAR *first, ...)
{
  va_list argp;
  _TCHAR  emesgstr[255] = {0};

  va_start(argp, first);
  _vstprintf(emesgstr, first, argp);
  va_end(argp);
  _tcscat(emesgstr, sys_err_str());
  err_disp(0, emesgstr);
  SetLastError(0);

  return;
} /* err_shw */
