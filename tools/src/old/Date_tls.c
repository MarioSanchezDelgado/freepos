/*
 *     Module Name       : DATE_TLS.C
 *
 *     Type              : General date/time functions
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
 * 13-Jan-2000 Added function get_current_weekday (for mml-interpreter)  R.L.
 * 04-Oct-2000 Added function timestamp()                              J.D.M.
 * 15-Nov-2000 Replaced localtime by GetLocalTime                      R.N.B.
 * 23-Jan-2001 Added function date_time_file_extension()                 M.W.
 * 03-Oct-2002 Added get_current_date().                               J.D.M.
 * 31-Jan-2003 Added function return_system_date()                     J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

                                            /* System include files.       */
#include <time.h>
#include <winbase.h>
#include <stdio.h>
                                            /* Toolsset include files.     */
#include "Date_tls.h"

/*-------------------------------------------------------------------------*/
/*                            set_system_date                              */
/*-------------------------------------------------------------------------*/
/* Set the system clock to long date (format YYYYMMDD).                    */
/*-------------------------------------------------------------------------*/
BOOL set_system_date(long date)
{
  SYSTEMTIME win_systemtime;
  BOOL       status;

  /* Before calling SetLocalTime we still have to set some priviliges
     with for example GetTokenInformation(), AdjustTokenPriviliges() etc.
     This makes sure that the new system date can be set with any user!
   */

  GetLocalTime(&win_systemtime);
  win_systemtime.wDay   = (WORD) (date % 100L);
  win_systemtime.wMonth = (WORD)((date / 100L) % 100L);
  win_systemtime.wYear  = (WORD) (date / 10000L);
  status = SetLocalTime(&win_systemtime);
  status = SetLocalTime(&win_systemtime); /* A second time, thanks to Bill */

  return(status);
} /* set_system_date */


/*-------------------------------------------------------------------------*/
/*                           set_system_time                               */
/*-------------------------------------------------------------------------*/
/* Set the system clock time to int tim (format HHMM).                     */
/*-------------------------------------------------------------------------*/
BOOL set_system_time(int tim)
{
  SYSTEMTIME win_systemtime;
  BOOL       status;

  /* Before calling SetLocalTime we still have to set some priviliges
     with for example GetTokenInformation(), AdjustTokenPriviliges() etc.
     This makes sure that the new system time can be set with any user!
   */

  GetLocalTime(&win_systemtime);
  win_systemtime.wHour         = (WORD)(tim / 100);
  win_systemtime.wMinute       = (WORD)(tim % 100);
  win_systemtime.wSecond       = (WORD)0;
  win_systemtime.wMilliseconds = (WORD)0;
  status = SetLocalTime(&win_systemtime);
  status = SetLocalTime(&win_systemtime); /* A second time, thanks to Bill */

  return(status);
} /* set_system_time */


/*-------------------------------------------------------------------------*/
/*                              get_current_time                           */
/*-------------------------------------------------------------------------*/
/* Returns the time (hour+min) as a short and copy's the time in str_time. */
/*-------------------------------------------------------------------------*/
short get_current_time(_TCHAR *str_time)
{
  int        hhmm;
  SYSTEMTIME st;

  GetLocalTime(&st);

  hhmm = st.wHour * 100 + st.wMinute;
  if (str_time) {
    _itot(hhmm, str_time, 10);
  }
  
  return(hhmm);
} /* get_current_time */

/*-------------------------------------------------------------------------*/
/*                              get_current_date                           */
/*-------------------------------------------------------------------------*/
/* Returns the date as a long and copies the date in str_date.             */
/*-------------------------------------------------------------------------*/
long get_current_date(short fmt, _TCHAR *str_date)
{
  SYSTEMTIME st;
  long       date;

  GetLocalTime(&st);

  switch (fmt) {
  case DATE_YYMMDD:
    date = (st.wYear%100)*10000 + st.wMonth*100 + st.wDay;
    break;
  case DATE_YYYYMMDD:
    date = st.wYear*10000 + st.wMonth*100 + st.wDay;
    break;
  case TIME_HHMI:
    date = st.wHour*100 + st.wMinute;
    break;
  case TIME_HHMISS:
    date = st.wHour*10000 + st.wMinute*100 + st.wSecond;
    break;
  default:
    date = 0;
    break;
  }

  if (str_date) {
    _ltot(date, str_date, 10);
  }

  return(date);
} /* get_current_date */

/*-------------------------------------------------------------------------*/
/*                        get_current_weekday                              */
/*-------------------------------------------------------------------------*/
/* returns the weekday(sunday-saturday : 0-6) as a short                   */
/*-------------------------------------------------------------------------*/
short get_current_weekday(long yyyymmdd)
{
  struct tm tim;
  time_t dummy_time;

  tim.tm_mday   = (int) (yyyymmdd % 100L);
  tim.tm_mon = ((int)((yyyymmdd / 100L) % 100L))-1;
  tim.tm_year  = ((int) (yyyymmdd / 10000L))-1900;
  tim.tm_sec=0;
  tim.tm_hour=0;
  tim.tm_wday=0;
  tim.tm_yday=0;
  tim.tm_isdst=0;
  tim.tm_min=0;

  dummy_time = mktime(&tim);
  return ((short)tim.tm_wday);
}

 
/*-------------------------------------------------------------------------*/
/*                              get_jul_date                               */
/*-------------------------------------------------------------------------*/
/* Puts the julian date of 'l_date' (YYYYMMDD) in 'result'.                */
/*-------------------------------------------------------------------------*/
short get_jul_date(long l_date, _TCHAR *result)
{
  short year, month, day, jul_day;
  short mon_julian[] =
          { 0,   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334 };
          /* mon_julian[ 3 ] contains days in a non leap year till march 1 */

  year  = (short)(l_date / (long)10000);
  month = (short)((l_date / (long)100) % (long)100);
  day   = (short)(l_date % (long)100);
  if (month >= 3 && year%4 == 0) {
    jul_day = mon_julian[month] + day + 1;
  }
  else {
    jul_day = mon_julian[month] + day;
  }
  
  if (result) {
    _itot(jul_day, result, 10);
  }

  return (jul_day);
} /* get_jul_date */

/*-------------------------------------------------------------------------*/
/*                             check_date                                  */
/*-------------------------------------------------------------------------*/
/* Check if date in format 'YYYYMMDD' is a legal date value.               */
/*-------------------------------------------------------------------------*/
short check_date(long date)
{
  short mm_days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  short yyyy, mm, dd;

  yyyy       = (short)(date / 10000L);
  mm_days[1] = (yyyy % 4 == 0)? 29: 28;
  mm         = (short)((date / 100L) % 100L);
  dd         = (short)(date % 100L);
  if (yyyy < 1900 || yyyy > 2100 || mm < 1 || mm > 12 ||
       dd < 1 || dd > mm_days[mm-1]) {
    return(FAIL);
  }
  else {
    return(SUCCEED);
  }
} /* check_date */

/*-------------------------------------------------------------------------*/
/*                    date_x_days_back                                     */
/*-------------------------------------------------------------------------*/
/* Calculate date x days back.                                             */
/* Input : date in YYYYMMDD format                                         */
/*         x_days = number of days back                                    */
/* Output: date x_days back                                                */         
/*-------------------------------------------------------------------------*/
long date_x_days_back(short x_days)
{
  short      year, month, day, day_in_year, i;
  long       return_date;
  SYSTEMTIME st;

  /* first 0 is for array position 0 which is not a month                  */
  /* 400 is to stop at the end of the array                                */

  short mon_julian[] =
          {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 400};

  GetLocalTime(&st);

  year  = st.wYear;
  month = st.wMonth;
  day   = st.wDay;

  day_in_year = mon_julian[month] + day;

  if (month >= 3 && year%4 == 0) {
    day++;
  }
  
  while (x_days >= day_in_year) {
    year--;
    day_in_year += 365;
    }

  day_in_year -= x_days;

  i = 1;
  while (day_in_year - mon_julian[i] > 0) {
    i++;
  }

  month = i-1;
  day   = day_in_year - mon_julian[i-1];

  return_date = year*10000L + month*100L + day;

  return(return_date);
} /* date_x_days_back */

/*-------------------------------------------------------------------------*/
/*                             check_time                                  */
/*-------------------------------------------------------------------------*/
/* Check if time in format HHMM is a legal time value.                     */
/*-------------------------------------------------------------------------*/
short check_time(int time)
{
  if (time < 0 || time >= 2400 || time % 100 >= 60) {
    return(FAIL);
  }
  else {
    return(SUCCEED);
  }
} /* check_time */

/*-------------------------------------------------------------------------*/
/*                              prn_fmt_date                               */
/*-------------------------------------------------------------------------*/
/* Puts the julian date of 'l_date' (YYYYMMDD) in 'result' according to    */
/* 'date_format'.                                                          */
/*-------------------------------------------------------------------------*/
void prn_fmt_date(long l_date, _TCHAR *date_format,
                  _TCHAR **mon_names, _TCHAR *result)
{
  short  year, month, day;
  _TCHAR *g;

  g     = date_format;
  year  = (short)(l_date / (long)10000);
  month = (short)((l_date / (long)100) % (long)100);
  day   = (short)(l_date % (long)100);
  while (*g) {
    switch (*g) {
      case _T('D'):
        *result++ = (_TCHAR)(day / 10 + _T('0'));             /* 'DD'   */
        *result++ = (_TCHAR)(day % 10 + _T('0'));
        g += 2;
        break;
      case _T('M'):
        if (*(g+1) == _T('O')) {                            /* 'MON'  */
          _tcscpy(result, mon_names[month]);
          result += 3;
          g += 3;
        }
        else {                                          /* 'MM'   */
          *result++ = (_TCHAR)(month / 10 + _T('0'));
          *result++ = (_TCHAR)(month % 10 + _T('0'));
          g += 2;
        }
        break;
      case _T('Y'):
        if (*(g+2) == _T('Y')) {                            /* 'YYYY' */
          *result++ = (_TCHAR)(year / 1000 + _T('0'));
          *result++ = (_TCHAR)((year / 100) % 10 + _T('0'));
          *result++ = (_TCHAR)((year / 10) % 10 + _T('0'));
          *result++ = (_TCHAR)(year % 10 + _T('0'));
          g += 4;
        }
        else {                                          /* 'YY'   */
          *result++ = (_TCHAR)((year / 10) % 10 + _T('0'));
          *result++ = (_TCHAR)(year % 10 + _T('0'));
          g += 2;
        }
        break;
      default:
        *result++ = *g++;
        break;
    }
  }
  *result = _T('\0');
} /* prn_fmt_date */

/*-------------------------------------------------------------------------*/
/*                            timestamp                                    */
/*-------------------------------------------------------------------------*/
_TCHAR *timestamp(void)
{
  SYSTEMTIME win_systemtime;
  static _TCHAR tstamp[20];

  GetLocalTime(&win_systemtime);
  _stprintf(tstamp, _T("%02d:%02d:%02d %02d-%02d"),
                    win_systemtime.wHour, win_systemtime.wMinute,
                    win_systemtime.wSecond, win_systemtime.wDay,
                    win_systemtime.wMonth);
  return(tstamp);
} /* timestamp */

/*-------------------------------------------------------------------------*/
/*                         date_time_file_extension                        */
/*-------------------------------------------------------------------------*/
_TCHAR *date_time_file_extension(void)
{
  SYSTEMTIME win_systemtime;
  static _TCHAR tstamp[20];

  GetLocalTime(&win_systemtime);
  _stprintf(tstamp, _T("%02d%02d%04d.%02d%02d%02d"),
                    win_systemtime.wDay, win_systemtime.wMonth, win_systemtime.wYear,
                    win_systemtime.wHour, win_systemtime.wMinute, win_systemtime.wSecond);
  return(tstamp);
} /* date_time_file_extension */

/*-------------------------------------------------------------------------*/
/*                         return_system_date                              */
/*-------------------------------------------------------------------------*/
_TCHAR* return_system_date()
{
  SYSTEMTIME win_systemtime;
  static _TCHAR system_date[11];

  GetLocalTime(&win_systemtime);
  _stprintf(system_date, _T("%02d-%02d-%04d"),
                    win_systemtime.wDay,
                    win_systemtime.wMonth,
                    win_systemtime.wYear);

  return system_date;
} /* return_system_date */
