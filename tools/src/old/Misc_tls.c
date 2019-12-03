/*
 *     Module Name       : MISC_TLS.C
 *
 *     Type              : Miscellaneous functions
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
 * --------------------------------------------------------------------------
 * 19-Mar-2001 Added semaphore functionality.                          J.D.M.
 * --------------------------------------------------------------------------
 * 26-Mar-2003 Added functions to compare doubles and floats.          J.D.M.
 * --------------------------------------------------------------------------
 * 22-Oct-2004 Added Semaphore_Get() to retrieve the value.            J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>
                                            /* System include files.         */
#include <math.h>
                                            /* Pos (library) include files   */
#include "misc_tls.h"
#include "date_tls.h"
#include "stri_tls.h"
                                            /* Toolsset include files.       */
#include "scrn_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "mem_mgr.h"

/*---------------------------------------------------------------------------*/
/*                             vfy_range                                     */
/*---------------------------------------------------------------------------*/
/* In case 'data' is unequal zero and lies between 'lower' and 'upper',      */
/* return SUCCEED.                                                           */
/*---------------------------------------------------------------------------*/
short vfy_range(_TCHAR *data, short lower, short upper)
{
  short option;

  if (*data != _T('\0')) {
    option=_ttoi(data);
    if (option >= lower && option <= upper) {
      return(SUCCEED);
    }
  }

  return(!SUCCEED);
} /* vfy_range */

/*---------------------------------------------------------------------------*/
/*                             vfy_clear_key                                 */
/*---------------------------------------------------------------------------*/
/* Clear 'data' except for a possible minus-sign.                            */
/*---------------------------------------------------------------------------*/
short vfy_clear_key(_TCHAR *data, short key)
{
  if (*data==_T('-')) {
    *(data+1)=_T('\0');
  }
  else {
    *data=_T('\0');
  }

  return(UNKNOWN_KEY);
} /* vfy_clear_key */

/*---------------------------------------------------------------------------*/
/*                             vfy_clear_all_key                             */
/*---------------------------------------------------------------------------*/
/* Clear all 'data'                                                          */
/*---------------------------------------------------------------------------*/
short vfy_clear_all_key(_TCHAR *data, short key)
{
  *data=_T('\0');

  return(UNKNOWN_KEY);
} /* vfy_clear_all_key */

/*---------------------------------------------------------------------------*/
/*                             pos_calc_pincd                                */
/* - run_date may be 0 when pin_cd is independent of date.                   */
/*---------------------------------------------------------------------------*/
int pos_calc_pincd(int cash_no, _TCHAR *pincd_s, long run_date)
{
  int  pincd,
       day_no = 0,
       year = 0;

  if (run_date > 0) {
    day_no = (int) get_jul_date(run_date, NULL);
    year = (int)(run_date / 10000L);
  }

  pincd = (int)fmod(
                    fabs( (sin((double)cash_no) +
                           cos((double)day_no) +
                           tan((double)year)
                          ) * 10000.0
                        )
                    , 10000.0
                   );

  if (pincd_s) {    /* save result in buffer */
    _itot(pincd, pincd_s, 10);
  }

  return(pincd);
} /* pos_calc_pincd */


/*---------------------------------------------------------------------------*/
/*                             element_in_set                                */
/*---------------------------------------------------------------------------*/
short element_in_set(short element, short *set)
{
  while (*set != 0) {
    if (*set == element) {
      return(SUCCEED);
    }
    else {
      set++;
    }
  }

  return(FAIL);
} /* element_in_set */


/*---------------------------------------------------------------------------*/
/*                             called_by_state                               */
/*---------------------------------------------------------------------------*/
short called_by_state(short *calling_states)
{
  return (element_in_set(state_previous_number(), calling_states));
} /* called_by_state */


/*---------------------------------------------------------------------------*/
/*                             display_prompt_right                          */
/*---------------------------------------------------------------------------*/
void display_prompt_right(_TCHAR *prompt, unsigned short window)
{
  _TCHAR           buffer[81];
  unsigned short save_window;
  short          max_len;
  WINDOW         wininfo;

  save_window = scrn_get_current_window();
  if (scrn_select_window(window) != SCRN_WINDOW_UNDEFINED &&
      scrn_get_info(window, &wininfo) == SUCCEED) {
    max_len = wininfo.num_of_cols;
    if (max_len > (short)_tcslen(prompt)) {
      max_len = _tcslen(prompt);
    }
    _tcsncpy(buffer, prompt, max_len);
    *(buffer+max_len) = _T('\0');
    scrn_string_out(buffer, 0, (short)(wininfo.num_of_cols - _tcslen(buffer)));
  }
  scrn_select_window(save_window);
} /* display_prompt_right */


/*---------------------------------------------------------------------------*/
/*                             display_prompt                                */
/*---------------------------------------------------------------------------*/
void display_prompt(_TCHAR *prompt, unsigned short window)
{
  unsigned short save_window;

  save_window = scrn_get_current_window();
  if (scrn_clear_window(window) != SCRN_WINDOW_UNDEFINED) {
    scrn_string_out(prompt,0,0);
  }
  scrn_select_window(save_window);
} /* display_prompt */


/*---------------------------------------------------------------------------*/
/*                             check_barcode_ean                             */
/*---------------------------------------------------------------------------*/
short check_barcode_ean(_TCHAR *barcode)
{
  short i, len, sum_od, sum_ev;

  len    = _tcslen(barcode);
  sum_od = sum_ev = 0;

  for (i = len - 2; i >= 0; i -= 2) {
    sum_ev += barcode[i]   - _T('0');
    sum_od += barcode[i+1] - _T('0');
  }
  if (i == -1) {
    sum_od += barcode[0] - _T('0');
  }
  if ((sum_ev * 3 + sum_od) % 10) {
    return(FAIL);
  }

  return(SUCCEED);
} /* check_barcode_ean */

/*---------------------------------------------------------------------------*/
/*                            display_percentage()                           */
/*---------------------------------------------------------------------------*/
void display_percentage(short cnt_groups,
                        short proc_groups, unsigned short window)
{
  _TCHAR buffer[60];

  if (proc_groups == 0 && cnt_groups == 0) {
    display_prompt_right(_T(""), window);
  }
  else {
    if (proc_groups > cnt_groups) {
      proc_groups=cnt_groups;
    }
    if (cnt_groups <= 0) {
      cnt_groups=1;
    }
    _stprintf(buffer, _T("                     Processed %3.0d%%"), proc_groups*100/cnt_groups);
    display_prompt_right(buffer, window);
  }

  return;
} /* display_percentage */

/*---------------------------------------------------------------------------*/
/*                            Semaphore_P                                    */
/*---------------------------------------------------------------------------*/
void Semaphore_P(long* Variable) {
  while(*Variable<=0) {
    give_time_to_OS(PM_NOREMOVE, 1);
  }
  InterlockedDecrement(Variable);
} /* Semaphore_P */

/*---------------------------------------------------------------------------*/
/*                            Semaphore_V                                    */
/*---------------------------------------------------------------------------*/
void Semaphore_V(long* Variable) {
  InterlockedIncrement(Variable);
} /* Semaphore_V */

/*---------------------------------------------------------------------------*/
/*                            Semaphore_Set                                  */
/*---------------------------------------------------------------------------*/
void Semaphore_Set(long* Variable, long Value) {
  InterlockedExchange(Variable, Value);
} /* Semaphore_Set */

/*---------------------------------------------------------------------------*/
/*                            Semaphore_Get                                  */
/*---------------------------------------------------------------------------*/
long Semaphore_Get(long* Variable) {
  return InterlockedExchangeAdd(Variable, 0);
} /* Semaphore_Get */

#define DEFAULT_NO_OF_DECIMALS 5
/*---------------------------------------------------------------------------*/
/*                               Dbl_EQ                                      */
/*---------------------------------------------------------------------------*/
short Dbl_EQ(double Double1, double Double2, short NoOfDecimals) {
  if(NoOfDecimals<0) {
    NoOfDecimals=DEFAULT_NO_OF_DECIMALS;
  }
  if(floor(Double1*pow(10,NoOfDecimals))==floor(Double2*pow(10,NoOfDecimals))) {
    return TRUE;
  }
  return FALSE;
} /* Dbl_EQ */

/*---------------------------------------------------------------------------*/
/*                               Dbl_GT                                      */
/*---------------------------------------------------------------------------*/
short Dbl_GT(double Double1, double Double2, short NoOfDecimals) {
  if(NoOfDecimals<0) {
    NoOfDecimals=DEFAULT_NO_OF_DECIMALS;
  }
  if(floor(Double1*pow(10,NoOfDecimals))>floor(Double2*pow(10,NoOfDecimals))) {
    return TRUE;
  }
  return FALSE;
} /* Dbl_GT */

/*---------------------------------------------------------------------------*/
/*                              Dbl_GTEQ                                     */
/*---------------------------------------------------------------------------*/
short Dbl_GTEQ(double Double1, double Double2, short NoOfDecimals) {
  if(NoOfDecimals<0) {
    NoOfDecimals=DEFAULT_NO_OF_DECIMALS;
  }
  if(floor(Double1*pow(10,NoOfDecimals))>=floor(Double2*pow(10,NoOfDecimals))) {
    return TRUE;
  }
  return FALSE;
} /* Dbl_GTEQ */

/*---------------------------------------------------------------------------*/
/*                               Dbl_LT                                      */
/*---------------------------------------------------------------------------*/
short Dbl_LT(double Double1, double Double2, short NoOfDecimals) {
  if(NoOfDecimals<0) {
    NoOfDecimals=DEFAULT_NO_OF_DECIMALS;
  }
  if(floor(Double1*pow(10,NoOfDecimals))<floor(Double2*pow(10,NoOfDecimals))) {
    return TRUE;
  }
  return FALSE;
} /* Dbl_LT */

/*---------------------------------------------------------------------------*/
/*                              Dbl_LTEQ                                     */
/*---------------------------------------------------------------------------*/
short Dbl_LTEQ(double Double1, double Double2, short NoOfDecimals) {
  if(NoOfDecimals<0) {
    NoOfDecimals=DEFAULT_NO_OF_DECIMALS;
  }
  if(floor(Double1*pow(10,NoOfDecimals))<=floor(Double2*pow(10,NoOfDecimals))) {
    return TRUE;
  }
  return FALSE;
} /* Dbl_LTEQ */
