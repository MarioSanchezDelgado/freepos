/*
 *     Module Name       : DATE_TLS.H
 *
 *     Type              : Include file general date/time functions
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
 * 21-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * 04-Oct-2000 Added function timestamp()                              J.D.M.
 * 23-Jan-2001 Added function date_time_file_extension()                 M.W.
 * 03-Oct-2002 Added get_current_date().                               J.D.M.
 * 31-Jan-2003 Added function return_system_date()                     J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef _date_tls_h_
#define _date_tls_h_

#ifdef __cplusplus
extern "C" {
#endif
                                                 /* used by get_current_date */
enum DATE_TIME_FORMAT {
  DATE_YYMMDD
 ,DATE_YYYYMMDD
 ,TIME_HHMI
 ,TIME_HHMISS
};

extern BOOL  set_system_date(long);
extern BOOL  set_system_time(int);
extern short get_current_time(_TCHAR *);
extern long  get_current_date(short, _TCHAR *);
extern short get_jul_date(long, _TCHAR *);
extern short check_date(long);
extern short check_time(int);
extern void  prn_fmt_date(long, _TCHAR *, _TCHAR **, _TCHAR *);
extern short get_current_weekday(long);
extern long  date_x_days_back(short);
extern _TCHAR *timestamp(void);
extern _TCHAR *date_time_file_extension(void);
extern _TCHAR* return_system_date();

#ifdef __cplusplus
}
#endif

#endif
