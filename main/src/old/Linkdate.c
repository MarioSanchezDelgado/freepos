/*
 *     Module Name       : LINKDATE.C
 *
 *     Type              : Generates linkdate
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
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <pos_txt.h>

static _TCHAR preamble[]    = _T("\n$#@MBSBEG@#$\n");
       _TCHAR link_date[30] = _T(__DATE__);
       _TCHAR link_time[20] = _T(__TIME__);
static _TCHAR postamble[]   = _T("\n$#@MBSEND@#$\n");

/* mon_trnames[ i ] contains name of i'th month for translation of __DATE__   */

static _TCHAR *montr_names[] =
{
  _T(""), _T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"),
      _T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec")
};

/*---------------------------------------------------------------------------*/
/*                             fill_compile_date                             */
/*---------------------------------------------------------------------------*/

void fill_compile_date(void)
{
  _TCHAR mm[4];
  _TCHAR ldate[40];
  int i;

  _tcsncpy(mm, _T(__DATE__), 3);
  _tcscpy(ldate, _T(__DATE__));
  mm[3] = _T('\0');
  for (i=1; i<=12; i++) {
    if(_tcscmp(mm, montr_names[i]) == 0 ) {
      break;
    }
  }
  _tcscpy(link_date, mon_names[i]);      /* Use mon_names (translated)        */
  _tcscat(link_date, ldate+3);           /* Month is not added but translated */

  _tcscpy(link_time, _T(__TIME__));
} /* fill_compile_date */
