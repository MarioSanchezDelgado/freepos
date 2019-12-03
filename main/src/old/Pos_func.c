/*
 *     Module Name       : POS_FUNC.C
 *
 *     Type              : General Functions Definitions
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
 * 11-Dec-2000 Bug: in display_working simply clear line when on=NO    R.N.B.
 * --------------------------------------------------------------------------
 * 11-Dec-2000 sleep 1 millisecond in inp_idle to save CPU time        R.N.B.
 * --------------------------------------------------------------------------
 * 05-Jul-2001 Changed update of lan_indicator in application_inp_idle R.N.B.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 24-Apr-2002 Implemented use of Version_mgr instead of linkdate      J.D.M.
 * --------------------------------------------------------------------------
 * 16-Dec-2003 Added Donation functionality.                           M.W.
 * --------------------------------------------------------------------------
 * 12-Jul-2011 Added initialization of var buffer
 *             this is un fix / patch		                           A.C.M.
 * 22-Ene-2015 Modificación de fechas y de la fórmula para obtener    M.E.A.C. 
 *             de cupones de la Promoción UNILEVER - GLOBAL
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include "appbase.h"
#include <stdio.h>
#include <iostream.h>
#include <math.h>

#include "OLETools.h"
#include "ConDLL.H"
#include "Registry.h"

#include "stnetp24.h"                       /* Pos (library) include files   */
#include "comm_tls.h"
#include "intrface.h"
#include "stri_tls.h"
#include "date_tls.h"
#include "prn_mgr.h"

                                            /* Toolsset include files.       */
#include "tm_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "scrn_mgr.h"
#include "err_mgr.h"
#include "mapkeys.h"
#include "mem_mgr.h"
#include "tot_mgr.h"

#include "pos_tm.h"                         /* Application include files.    */
#include "pos_inp.h"
#include "pos_keys.h"
#include "pos_errs.h"
#include "pos_recs.h"
#include "pos_com.h"
#include "pos_tot.h"
#include "pos_func.h"
#include "pos_txt.h"
#include "pos_st.h"
#include "WPos_mn.h"
#include "st_main.h"
#include "Version_mgr.h"
#include <stdarg.h>   /* 27-Jan-2012 acm -  */
#include <stdarg.h>   /* 27-Jan-2012 acm -  */
#include <time.h>
#include "fmt_tool.h" /* 27-Jan-2012 acm -  */
#include "write.h" /* 27-Jan-2012 acm -  */
#include "Pos_recs.h"
#include <stdarg.h>

     int INVOICE_VALID = 0; /* AC2012-003 acm - */
static void unblank_crt_cdsp(SYSTEMTIME *);
static void save_screen(void);

int prom_type_MAKRO_ANIVERSARY  = 1;// v3.4.8 acm -
int prom_type_RASPA_Y_GANA      = 2;// v3.4.8 acm -
int prom_type_HORECA            = 3;// v3.4.8 acm -
int prom_type_CINE              = 4;// v3.5.0 acm -
int prom_type_FERIA_ESCOLAR     = 5;// v3.6.0 acm -
int prom_type_FIESTA_FUTBOL     = 6;// v3.6.0 acm -

#define prom_type_UNILEVER        7
#define prom_type_NAJAR           8

#define prom_case_logic_AMOUNT    1
#define prom_case_logic_QTY       2


double TOT_GEN_PERCEPTION       = 0;
double POS_ZERO_VALUE           = 0.000001;
double PERC_GEN_PERCEPTION      = 0;
int    IS_GEN_PERCEPTION        = 0;

int     PERCEPTION_ENABLED      = 1;

char usr_perception_document[500];
char usr_perception_name    [500];

char usr_document[500];
char usr_name    [500];

char last_perception_name    [500];
char last_perception_document[500];

char last_name    [500];
char last_document[500];

extern int prom_case_logic_CUPON_GLOBAL [CUPON_GLOBAL_MAX];
extern int isAnticipo_Valid(_TCHAR *fac_no);


extern SLL    voucher_hist;
extern SLL    voucher_anticipo_hist;
extern SLL    payment_items;
extern INVOICE_PAYM_ITEM_DEF voucher_items;


/*---------------------------------------------------------------------------*/
/*                            calc_incl_vat                                   */
/*---------------------------------------------------------------------------*/
double calc_incl_vat(double excl_vat, short vat_no) 
{
  return(excl_vat * (1+(get_vat_perc(vat_no)/100.0)));
} /* calc_incl_vat */

/*---------------------------------------------------------------------------*/
/*                           calc_excl_vat                                   */
/*---------------------------------------------------------------------------*/
double calc_excl_vat(double incl_vat, short vat_no) 
{
  return(incl_vat / (1+(get_vat_perc(vat_no)/100.0)));
} /* calc_excl_vat */

/*---------------------------------------------------------------------------*/
/*                           wait_for_key_S_or_NO                            */
/*---------------------------------------------------------------------------*/
/* retuns SUCCEED if supervisor approval has been given                      */
/*        FAIL if key NO has been pressed                                    */
/*---------------------------------------------------------------------------*/
short wait_for_key_S_or_NO(void)
{
  short key;

  /*                                                                         */
  /* Sequence:  S-KEY                                                        */
  /*       or:  NO                                                           */
  /*                                                                         */

  scrn_clear_window(ERROR_WINDOW_ROW2);
  scrn_string_out(input_TXT[27],0,0);
  FOREVER {
    key = rs_keylock_position();
    if (key == KEYLOCK_SUPERVISOR) {
      scrn_clear_window(ERROR_WINDOW_ROW2);
      return(SUCCEED);
    }

    key = inp_pick_up_key(KEYLOCK_S_MASK);
    if (key == NO_KEY) {
      scrn_clear_window(ERROR_WINDOW_ROW2);
      return(FAIL);
    }
    if (fn_inp_idle != (void *)NULL) {
      fn_inp_idle();                             /* Time to do other things. */
    }
  }
} /* wait_for_key_S_or_NO */

/*---------------------------------------------------------------------------*/
/*                           wait_for_super_appr_or_NO                       */
/*---------------------------------------------------------------------------*/
/* retuns SUCCEED if supervisor approval has been given                      */
/*        FAIL if key NO has been pressed                                    */
/*---------------------------------------------------------------------------*/
short wait_for_super_appr_or_NO(void)
{
  short key;

  /*                                                                         */
  /* Sequence:  S-KEY -> N-KEY                                               */
  /*       or:  NO                                                           */
  /*                                                                         */

  scrn_clear_window(ERROR_WINDOW_ROW2);
  scrn_string_out(input_TXT[27],0,0);
  FOREVER {
    key = rs_keylock_position();
    if (key == KEYLOCK_SUPERVISOR) {
      scrn_clear_window(ERROR_WINDOW_ROW2);
      scrn_string_out(input_TXT[28],0,0);

      rs_wait_keylock_pos(KEYLOCK_NORMAL);

      scrn_clear_window(ERROR_WINDOW_ROW2);
      return(SUCCEED);
    }

    key = inp_pick_up_key(KEYLOCK_S_MASK);
    if (key == NO_KEY) {
      scrn_clear_window(ERROR_WINDOW_ROW2);
      return(FAIL);
    }
    if (fn_inp_idle != (void *)NULL) {
      fn_inp_idle();                             /* Time do to other things. */
    }
  }
} /* wait_for_super_appr_or_NO */

/*---------------------------------------------------------------------------*/
/*                           wait_for_supervisor_approval                    */
/*---------------------------------------------------------------------------*/
short wait_for_supervisor_approval(void)
{
  /*                                                                         */
  /* Sequence:  S-KEY -> N-KEY                                               */
  /*                                                                         */
  /* Replaces the rs_wait_for_supervisor_approval.                           */
  /*                                                                         */

  rs_wait_keylock_pos(KEYLOCK_SUPERVISOR);

  scrn_clear_window(ERROR_WINDOW_ROW2);
  scrn_string_out(input_TXT[28],0,0);

  rs_wait_keylock_pos(KEYLOCK_NORMAL);

  return(SUCCEED);
} /* wait_for_supervisor_approval */

/*---------------------------------------------------------------------------*/
/*                           wait_for_closed_drawer                          */
/*---------------------------------------------------------------------------*/
void wait_for_closed_drawer(_TCHAR *msg1, _TCHAR *msg2)
{
  short status;

  status = rs_cash_drawer_status(CASH_DRAWER1);
  if (status == DRAWER_OPEN) {
    scrn_clear_window(ERROR_WINDOW_ROW1);
    if (msg1 != (_TCHAR *)NULL) {
      scrn_string_out(msg1,0,0);
    }
    scrn_clear_window(ERROR_WINDOW_ROW2);
    if (msg2 != (_TCHAR *)NULL) {
      scrn_string_out(msg2,0,0);
    }
  }
  while (status == DRAWER_OPEN) {
    status = rs_cash_drawer_status(CASH_DRAWER1);
    if (fn_inp_idle != (void *)NULL) {
      fn_inp_idle();
    }
  }
  scrn_clear_window(ERROR_WINDOW_ROW1);
  scrn_clear_window(ERROR_WINDOW_ROW2);
} /* wait_for_closed_drawer */

/*---------------------------------------------------------------------------*/
/*                           open_and_close_drawer                           */
/*---------------------------------------------------------------------------*/
short open_and_close_drawer(_TCHAR *data, short key)
{
  rs_open_cash_drawer(CASH_DRAWER1);
  wait_for_closed_drawer(input_TXT[29], (_TCHAR *)NULL);
  *data = _T('\0');

  return(key);
} /* open_and_close_drawer */

/*---------------------------------------------------------------------------*/
/*                               rnd_curr()                                  */
/*---------------------------------------------------------------------------*/
double rnd_curr(double dvalue)
{
  double factor;

  /*                                                                         */
  /* Round the double using the lowest currency used in this country.        */
  /*                                                                         */
  /* ! Be aware that, if low_curr == 0, there is no rounding at all! So if   */
  /* necessary you should make a call like: d = rnd_curr(floor_price(d));    */
  /*                                                                         */
  if (genvar.low_curr > 0.0 && dvalue != 0.0) {
    factor = 1.0 / genvar.low_curr;
    if (dvalue > 0) {
      return(floor(dvalue * factor + 0.5)/factor);
    }
    else {
      return(-floor(-dvalue * factor + 0.5)/factor);
    }
  }

  return(dvalue);
} /* rnd_curr() */

/*---------------------------------------------------------------------------*/
/*                           floor_price                                     */
/*---------------------------------------------------------------------------*/
/* Round a double value depending on decimals used.                          */
/*---------------------------------------------------------------------------*/
/* 12-Jul-2011 acm - old function floor_price
double floor_price(double dvalue)
{
  double factor;
  if (genvar.ind_price_dec == DECIMALS_YES) {
    factor = 100.0;
  }
  else {
    factor = 1.0;
  }
  if (dvalue >= 0.0) {
    return(floor((dvalue*factor)+0.5)/factor);
  }
  else {
    return(-floor((-dvalue*factor)+0.5)/factor);
  }
}*/

/* 12-Jul-2011 acm - new function floor_price*/
double floor_price(double dvalue)
{
  double factor;
  double rvalue;	//21-Jun-2011 acm -
  if (genvar.ind_price_dec == DECIMALS_YES) {
    factor = 100.0;
  }
  else {
    factor = 1.0;
  }
  if (dvalue >= 0.0) {
    rvalue = (floor((dvalue*factor)+0.501)/factor); //21-Jun-2011 acm - se cambio 0.5 por 0.501
  }
  else {
    rvalue = (-floor((-dvalue*factor)+0.501)/factor); //21-Jun-2011 acm - se cambio 0.5 por 0.501
  }
  return rvalue;	//21-Jun-2011 acm -
} 
/* floor_price */





/*---------------------------------------------------------------------------*/
/*                           ftoa                                            */
/*---------------------------------------------------------------------------*/
short ftoa(double dbl, short max_len, _TCHAR *result)
{
  _TCHAR buffer[64];

  _stprintf(buffer, _T("%-0.0f"), dbl);

  if ((short)_tcslen(buffer) > max_len) {
    err_invoke(PRICE_TOO_LARGE);
    *result = _T('\0');
    return(FAIL);
  }
  _tcscpy(result, buffer);

  return(SUCCEED);
} /* ftoa */

/*---------------------------------------------------------------------------*/
/*                           atof_price                                      */
/*---------------------------------------------------------------------------*/
/* Convert string to double, if 2 decimals are used, divide result.          */
/*---------------------------------------------------------------------------*/
double atof_price(_TCHAR *price_buffer)
{
  if (genvar.ind_price_dec == DECIMALS_YES) {

/*  atof changed to _tcstod, a Unicode aware Function !!!                    */
    return((double)_tcstod(price_buffer, NULL)/100.0);
  }
  else {
    return((double)_tcstod(price_buffer, NULL));
  }
} /* atof_price */

/*---------------------------------------------------------------------------*/
/*                           ftoa_price                                      */
/*---------------------------------------------------------------------------*/
/* Convert the double to a string with respect of decials used or not.       */
/* The string is COPIED in the argument result!                              */
/*---------------------------------------------------------------------------*/
short ftoa_price(double dbl, short max_len, _TCHAR *result)
{
  if (genvar.ind_price_dec == DECIMALS_YES) {
    dbl = dbl * 100;
  }

  return(ftoa(dbl, max_len, result));
} /* ftoa_price */

/*---------------------------------------------------------------------------*/
/*                           cls                                             */
/*---------------------------------------------------------------------------*/
/* Clear screen, redraw empty error lines and display lan-status.            */
/*---------------------------------------------------------------------------*/
void cls(void)
{
  ClearScreen();
  scrn_clear_window(ERROR_WINDOW_ROW1);
  scrn_clear_window(ERROR_WINDOW_ROW2);
  view_lan_state(FORCED_UPDATE);
#ifndef NO_VIEW_POS_STATE
  view_pos_state();
#endif
} /* cls */

#define DELAY_SCROLL_TIME 175

/*---------------------------------------------------------------------------*/
/*                           cdsp_closed_till                                */
/*---------------------------------------------------------------------------*/
short cdsp_closed_till(short mode, short stby)
{
  static short in_proc = FALSE;
  static SYSTEMTIME old_hts;
  SYSTEMTIME new_hts;

  _TCHAR buffer[21];
  static _TCHAR *p=genvar.prom_txt_cust;     /* scrolling promotion txt.     */
  static _TCHAR  prom_txt_array[LEN_PROM_TXT_CUST+20];
  static short change_prom_txt_flag = 0;

  memset(buffer, 0, sizeof(buffer));

  /* Check if Promotion Text has changed. If so concatenate 20 spaces        */

  if (_tcsncmp(genvar.prom_txt_cust, prom_txt_array, _tcslen(genvar.prom_txt_cust))) {
    _tcscpy(prom_txt_array, genvar.prom_txt_cust);
    ch_memset(buffer, _T(' '), 20*sizeof(_TCHAR));
    _tcscat(prom_txt_array, buffer);         /* 20 spaces for nice scrolling */
    change_prom_txt_flag = 0;
  }
  
  if (!*genvar.prom_txt_cust) {
    if (!change_prom_txt_flag) {
      ch_memset(buffer, _T(' '), 20*sizeof(_TCHAR));
      _tcscat(prom_txt_array, buffer);       /* 20 spaces for nice scrolling */
      change_prom_txt_flag = 1;
    }
  }

  if (mode == 0 && closed_till == NO) {        /* Display "CLOSED" + time.   */
    p = prom_txt_array;
    cdsp_clear();
    cdsp_write_string(cdsp_TXT[1],0,0);
    Sleep(20); /* This is done to prevent timing problems with the customer display */
    closed_till = YES;
  }

  GetLocalTime(&new_hts);
  unblank_crt_cdsp(&new_hts);

  if (new_hts.wMilliseconds - old_hts.wMilliseconds > DELAY_SCROLL_TIME ||
        old_hts.wMilliseconds - new_hts.wMilliseconds > DELAY_SCROLL_TIME) {

    /* Refresh scroll. promo-txt */
    _tcsncpy(buffer, p++, 20);
    if (_tcslen(buffer)<20) {
      _tcsncat(buffer, prom_txt_array, (20-_tcslen(buffer)));
    }
    if (*p==_T('\0')) {
      p = prom_txt_array;
    }

    cdsp_write_string(buffer,1,0);
    old_hts.wMilliseconds = new_hts.wMilliseconds;
  }
      
  /* Update the clock on customer display.                                   */
  /* Also retrieve general var's every hour in Standby-mode and after        */
  /* an end-of-day.                                                          */
  if (new_hts.wMinute != old_hts.wMinute ||
      new_hts.wHour   != old_hts.wHour ||
      mode == 0 ) {
    if (new_hts.wHour  != old_hts.wHour   &&
        status_of_pos == START_OF_DAY     &&
        (!in_proc)     && stby) {
      in_proc = TRUE;
      proc_init_environment_records(empty, 0);
      in_proc = FALSE;
      p = prom_txt_array;
    }

    _itot((int)new_hts.wHour*100 + (int)new_hts.wMinute, buffer, DECIMAL);
    format_string(&string_time5, buffer);
    cdsp_write_string(string_time5.result, 0, CDSP_RIGHT_JUSTIFY);
    old_hts.wHour   = new_hts.wHour;
    old_hts.wMinute = new_hts.wMinute;
  }

  return(SUCCEED);
} /* cdsp_closed_till */


/*---------------------------------------------------------------------------*/
/*                           application_inp_idle                            */
/*---------------------------------------------------------------------------*/
short application_inp_idle(void)
{
  SYSTEMTIME t;

  /*                                                                         */
  /* This function is called by the toolset's inp_idle() function.           */
  /* It handles the promotional-text and time on customer display            */
  /* when the till is closed.                                                */
  /* It also activates stnetp24 to handle some network activities.           */
  /*                                                                         */

  /* If some LAN-stuff is performed, it is shown on the CRT only for         */
  /* debugging!!!                                                            */

#define LEN_REG_SPINNER         3
#define LEN_P24_NUM_COMM_IDLES  3
  _TCHAR       reg_spinner[LEN_REG_SPINNER+1];
  _TCHAR       reg_p24_num_comm_idles[LEN_P24_NUM_COMM_IDLES+1];
  static short first_time = YES;
  static short spinner_active;
  static short spinner_delay;
  static short p24_number_of_comm_idles;
  short        i;
  _TCHAR      *parm;
  static _TCHAR indicator[]={_T('|'),  _T('\0'), _T('/'),
                             _T('\0'), _T('-') , _T('\0'),
                             _T('\\'), _T('\0'), _T('@'),
                             _T('\0')};
  static _TCHAR *p=indicator;
  static int   flag_indicator = 1;
  short        curr_scrn=scrn_get_current_window();
  static DWORD old_ticks = 0;
  DWORD        cur_ticks = GetTickCount();
  _TCHAR       dummy[150];

  if(first_time == YES) {
    ReadEnvironmentValue(TREE_SCRN_SETTINGS, _T("SPINNER_ACTIVE"), reg_spinner, LEN_REG_SPINNER);
    parm = _tcsupr(reg_spinner);
    spinner_active = NO;
    if (*parm) {
      if (!_tcscmp(parm, _T("YES"))) {
        spinner_active = YES;
      }
    }
    ReadEnvironmentValue(TREE_SCRN_SETTINGS, _T("SPINNER_DELAY"), reg_spinner, LEN_REG_SPINNER);
    parm = _tcsupr(reg_spinner);
    spinner_delay = 150;
    if (*parm) {
      spinner_delay = _ttoi(parm);
      if(spinner_delay > 999) {
        spinner_delay = 999;
      }
      if(spinner_delay < 0) {
        spinner_delay = 0;
      }
    }

    /* P24_NUMBER_OF_COMM_IDLES */
    ReadEnvironmentValue(TREE_P24_SETTINGS, _T("P24_NUMBER_OF_COMM_IDLES"),
                                           reg_p24_num_comm_idles,
                                           LEN_P24_NUM_COMM_IDLES);
    parm = _tcsupr(reg_p24_num_comm_idles);
    if (!*parm) {
      _stprintf(dummy, _T("%s\\P24_NUMBER_OF_COMM_IDLES not defined in registry,\n")
                       _T("program will be aborted!!!"), TREE_P24_SETTINGS);
      MessageBox(NULL, dummy, NULL, MB_OK|MB_SETFOREGROUND);
      exit(1);
    }
    p24_number_of_comm_idles = _ttoi(parm);
    if (p24_number_of_comm_idles < 0 ) {
      p24_number_of_comm_idles = 0;
    }
    if (p24_number_of_comm_idles > 999 ) {
      p24_number_of_comm_idles = 999;
    }
    first_time = NO;
  }

  if(spinner_active == YES) {
    if (cur_ticks < old_ticks) {
      old_ticks = cur_ticks;
    }
    else if ( (old_ticks+spinner_delay) < cur_ticks ) {
      scrn_select_window(LAN_STATUS_WINDOW);
      scrn_string_out(p,0,8);
      if(*(p+=2)==_T('@')) {
        p=indicator;
      }
      scrn_select_window(curr_scrn);
      old_ticks = cur_ticks;
    }
  }

  if (closed_till == YES) {
    cdsp_closed_till(1, FALSE);        /* Handles unblank_crt_cdsp                  */
  }
  else {
    GetLocalTime(&t);
    unblank_crt_cdsp(&t);       /* Do unblank() yourself                     */
  }

  /* Give OS some time ... */
  give_time_to_OS(PM_NOREMOVE, 1);

  /* Give stnetp24 some time ... */
  for(i=0; i < p24_number_of_comm_idles; i++) {
   comm_idle();
  }
  /* Update ON/OFFLINE indicator */
  view_lan_state(NO_FORCED_UPDATE);

#ifdef DEBUG
{
  char msg[60];

  _stprintf(msg,"status=%d, amount drawer %15.2f", status_of_pos, tot_ret_double(AMOUNT_IN_DRAWER));
  DisplayString(msg, 0, 0, TEXT_NORMAL);
}
#endif

  return(SUCCEED);
} /* application_inp_idle */

/*---------------------------------------------------------------------------*/
/*                           unblank_crt_cdsp                                */
/*---------------------------------------------------------------------------*/
/* Unblank operator display (CRT) and the customer display.                  */
/*---------------------------------------------------------------------------*/
static void unblank_crt_cdsp(SYSTEMTIME *new_hts)
{
  static SYSTEMTIME old_hts;
  static short first_time = 1;

  if(first_time) {
    first_time = 0;
    GetLocalTime(&old_hts);
  }

  if (new_hts->wMinute != old_hts.wMinute) {
    //cdsp_unblank(system_type);
    memcpy(&old_hts, new_hts, sizeof(SYSTEMTIME));
  }
} /* unblank_crt_cdsp */

/*---------------------------------------------------------------------------*/
/*                           display_working                                 */
/*---------------------------------------------------------------------------*/
void display_working(int on)
{
  unsigned short save_window = scrn_get_current_window();
  _TCHAR buffer[41];

  if (scrn_select_window(ERROR_WINDOW_ROW1) != SCRN_WINDOW_UNDEFINED) {
    ch_memset(buffer, _T(' '), sizeof(buffer));
    buffer[40] = _T('\0');
    if(on == YES) {
      _tcsncpy(buffer, prompt_TXT[1], _tcslen(prompt_TXT[1]));
    }
    scrn_string_out(buffer,0,0);
  }

  scrn_select_window(save_window);
} /* display_working */

/*---------------------------------------------------------------------------*/
/*                           cnvrt_tm2ctree                                  */
/*---------------------------------------------------------------------------*/
short cnvrt_tm2ctree(SHIFT_TDM_DEF *recover)
{
  /*                                                                         */
  /* Copy all elements in tm into the recovery record. The delflag of the    */
  /* recover structure is skipped. For the rest both records should be the   */
  /* same. NOTE: because of byte alignment in structures the char delflg     */
  /* will be two bytes instead of one for a character.                       */
  /*                                                                         */

  short i;
  int ix;

  recover->delflg = (_TCHAR)0;
  recover->shift_no          = c_shft.shift_no;
  recover->cashier           = c_shft.cashier;
  recover->till_no           = c_shft.till_no;
  recover->invoice_on        = c_shft.invoice_on;
  recover->invoice_off       = c_shft.invoice_off;
  recover->date_on           = c_shft.date_on;
  recover->time_on           = c_shft.time_on;
  recover->time_off          = c_shft.time_off;
  recover->nbr_inv_lines     = c_shft.nbr_inv_lines;
  recover->nbr_void_inv      = c_shft.nbr_void_inv;
  recover->nbr_void_lines    = c_shft.nbr_void_lines;
  recover->start_float       = c_shft.start_float;
  recover->end_float         = c_shft.end_float;
  recover->lift_refill_float = c_shft.lift_refill_float;
  for (i = 0; i < 3; i++) {
    recover->wallet_no[i]    = c_shft.wallet_no[i];
  }
  for (i = 0; i < 10; i++) {
    recover->paym_amnt[i]    = c_shft.paym_amnt[i];
  }
  recover->donation          = c_shft.donation;
  recover->cupon          	 = c_shft.cupon; /*AC2012-003 acm -*/
  recover->rounded           = c_shft.rounded; //v3.4.7 acm -
  recover->cupon_cine      	 = c_shft.cupon_cine; /*v3.5 acm -*/
  recover->vale_pavo      	 = c_shft.vale_pavo; /*v3.5 acm -*/
  recover->feria_escolar  	 = c_shft.feria_escolar; /*v3.5 acm -*/
  
  recover->percep_amount        = c_shft.percep_amount; //v3.6.1 acm -
  recover->fiesta_futbol        = c_shft.fiesta_futbol; /*v3.5 acm -*/

  for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
  {
     recover->cupon_global[ix]    = c_shft.cupon_global[ix];
  }

  return(SUCCEED);
} /* cnvrt_tm2ctree */

/*---------------------------------------------------------------------------*/
/*                           cnvrt_ctree2tm                                  */
/*---------------------------------------------------------------------------*/
short cnvrt_ctree2tm(SHIFT_TDM_DEF *recover)
{
  /*                                                                         */
  /* Copy all elements in the recovery record to tm. The delflag of the      */
  /* recover structure is skipped. For the rest both records should be the   */
  /* same. NOTE: because of byte alignment in structures the char delflg     */
  /* will be two bytes instead of one for a character.                       */
  /*                                                                         */

  short ix,i;

  c_shft.shift_no          = recover->shift_no;
  c_shft.cashier           = recover->cashier;
  c_shft.till_no           = recover->till_no;
  c_shft.invoice_on        = recover->invoice_on;
  c_shft.invoice_off       = recover->invoice_off;
  c_shft.date_on           = recover->date_on;
  c_shft.time_on           = recover->time_on;
  c_shft.time_off          = recover->time_off;
  c_shft.nbr_inv_lines     = recover->nbr_inv_lines;
  c_shft.nbr_void_inv      = recover->nbr_void_inv;
  c_shft.nbr_void_lines    = recover->nbr_void_lines;
  c_shft.start_float       = recover->start_float;
  c_shft.end_float         = recover->end_float;
  c_shft.lift_refill_float = recover->lift_refill_float;
  for (i = 0; i < 3; i++) {
    c_shft.wallet_no[i]    = recover->wallet_no[i];
  }
  for (i = 0; i < 10; i++) {
    c_shft.paym_amnt[i]    = recover->paym_amnt[i];
  }
  c_shft.donation          = recover->donation;
  c_shft.cupon             = recover->cupon; 		/*AC2012-003 acm -*/
  c_shft.rounded           = recover->rounded; 		//v3.4.7 acm -
  c_shft.cupon_cine        = recover->cupon_cine;   /*v3.5 acm -*/
  c_shft.vale_pavo         = recover->vale_pavo;   /*v3.5 acm -*/  

  c_shft.feria_escolar     = recover->feria_escolar;   /*v3.6.0 acm -*/  

  c_shft.percep_amount        = recover->percep_amount;      //v3.6.1 acm -

  c_shft.fiesta_futbol      = recover->fiesta_futbol;   /*v3.5 acm -*/

  for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
  {
        c_shft.cupon_global[ix]    = recover->cupon_global[ix];
  }

  return(SUCCEED);
} /* cnvrt_ctree2tm */

/*---------------------------------------------------------------------------*/
/*                           recover_shift_updt                              */
/*---------------------------------------------------------------------------*/
short recover_shift_updt(void)
{
  SHIFT_TDM_DEF recover;
  short status = FAIL;
  
  /*                                                                         */
  /* Update a SHIFT_TDM_DEF record to disk after reading contents from       */
  /* TM_SHFT_GROUP tm_shift.                                                 */
  /*                                                                         */

  if (cnvrt_tm2ctree(&recover) == SUCCEED) {
    status = pos_update_rec(RECO_TYPE, POS_RECO_SIZE, RECOVER_IDX,
                            RECOVER_FNO, (void*)&recover);
  }

  return(status);
} /* recover_shift_updt */

/*---------------------------------------------------------------------------*/
/*                           recover_shift_save                              */
/*---------------------------------------------------------------------------*/
short recover_shift_save(void)
{
  short         status = FAIL;
  SHIFT_TDM_DEF recover;
  
  /*                                                                         */
  /* Write a SHIFT_TDM_DEF record to disk after reading contents from        */
  /* TM_SHFT_GROUP node offset.                                              */
  /*                                                                         */

  if (cnvrt_tm2ctree(&recover) == SUCCEED) {
    status = pos_put_rec(RECO_TYPE, POS_RECO_SIZE,
                         RECOVER_FNO, (void*)&recover);
  }

  return(status);
} /* recover_shift_save */

/*---------------------------------------------------------------------------*/
/*                           recover_shift_remv                              */
/*---------------------------------------------------------------------------*/
short recover_shift_remv(void)
{
  short         status;
  SHIFT_TDM_DEF recover;

  /*                                                                         */
  /* Remove all SHIFT_TDM_DEF records from file.                             */
  /*                                                                         */
  
  status = pos_first_rec(RECO_TYPE, POS_RECO_SIZE,
                         RECOVER_IDX, (void*)&recover);
  while (status == SUCCEED) {
    status = pos_delete_rec(RECO_TYPE, POS_RECO_SIZE, RECOVER_IDX,
                            RECOVER_FNO, (void*)&recover);
    if (status == SUCCEED) {
      status = pos_first_rec(RECO_TYPE, POS_RECO_SIZE,
                             RECOVER_IDX, (void*)&recover);
    }
  }

  return(SUCCEED);
} /* recover_shift_remv */

/*---------------------------------------------------------------------------*/
/*                           recover_shift_2tm                               */
/*---------------------------------------------------------------------------*/
short recover_shift_2tm(void)
{
  short         status;
  SHIFT_TDM_DEF recover;

  /*                                                                         */
  /* Read all SHIFT_TDM_DEF records from c-tree and put them in tm.          */
  /*                                                                         */
  
  status = pos_first_rec(RECO_TYPE, POS_RECO_SIZE,
                         RECOVER_IDX, (void*)&recover);
  while (status == SUCCEED) {
    cnvrt_ctree2tm(&recover);
    tm_appe(TM_SHFT_NAME, (void*)&c_shft);
    status = pos_next_rec(RECO_TYPE, POS_RECO_SIZE,
                          RECOVER_IDX, (void*)&recover);
  }

  return(SUCCEED);
} /* recover_shift_2tm */

_TCHAR link_date[sizeof(__DATE__)/sizeof(_TCHAR)+1] = _T("");

/*---------------------------------------------------------------------------*/
/*                              get_compile_date                             */
/* - translates the compile date from version_mgr.c                          */
/*---------------------------------------------------------------------------*/
_TCHAR *get_compile_date(void)
{
  _TCHAR mm[4];
  _TCHAR ldate[40];
  int i;
  _TCHAR *montr_names[] = { _T(""), 
     _T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"),
     _T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec")
  };

  if (! *link_date) {
    _tcsncpy(mm, compile_date, 3);
    _tcscpy(ldate, compile_date);
    mm[3] = _T('\0');
    for (i=1; i<=12; i++) {
      if(_tcscmp(mm, montr_names[i]) == 0 ) {
        break;
      }
    }
    _tcscpy(link_date, mon_names[i]);      /* Use mon_names (translated)        */
    _tcscat(link_date, ldate+3);           /* Month is not added but translated */
  }

  return(link_date);
} /* get_compile_date */


/*---------------------------------------------------------------------------*/
/*                           ispassday                                       */
/*---------------------------------------------------------------------------*/
int ispassday()
{
  return (cust.cust_type_no==98);
}

int isFiscalClient()
{
  return (strlen(cust.fisc_no) == 11);// && invoice_type_choice == 0) {
}




/* 27-Jan-2012 acm - mark{*/

/*---------------------------------------------------------------------------*/
/*                           isvigente_promotion                             */
/*---------------------------------------------------------------------------*/
int isvigente_promotion()
{
  char dest[100]={0};
  long curr_date;

  time_t curtime = time(0); 
  struct tm now=*localtime(&curtime); 
  
  if ((prom_anniv_date_begin==0)||(prom_anniv_date_end==0)) return 0;

  strftime(dest, sizeof(dest)-1,"%m%d", &now);
  curr_date=atoi(dest);

  if (curr_date>=prom_anniv_date_begin && 
      curr_date<=prom_anniv_date_end )    return 1;
  
  return 0;
}
/*---------------------------------------------------------------------------*/
/*                           isvigente_horeca                                */
/*---------------------------------------------------------------------------*/
int isvigente_horeca()
{
  char dest[100]={0};
  long curr_date;

  time_t curtime = time(0); 
  struct tm now=*localtime(&curtime); 
  
  if ((promotion_horeca_date_begin==0)||(promotion_horeca_date_end==0)) return 0;

  strftime(dest, sizeof(dest)-1,"%m%d", &now);
  curr_date=atoi(dest);

  if (curr_date>=promotion_horeca_date_begin && 
      curr_date<=promotion_horeca_date_end )    return 1;
  
  return 0;
}

/*---------------------------------------------------------------------------*/
/*                           is_article_prom                                 */
/*---------------------------------------------------------------------------*/
int is_article_prom(long art_no, int prom_type)
{
  ARTICLE_PROM_DEF article_prom;  //acm - mark
  short status;

  article_prom.art_no = art_no;
  article_prom.prom_type=prom_type;
  status = pos_get_rec(ARTI_PROM_TYPE, ARTICLE_PROM_SIZE, ARTICLE_PROM_IDX,
                      (void*)&article_prom, (short) keyEQL);

  if(status == SUCCEED){
      
      return(1);
  }
  else {
    return(0); 
  }
}

short print_ln_fmt(short printer_id, char* format, ...)
{
    char buffer[1024];
    short ret;

    va_list argptr;
    va_start(argptr, format);

    _vsnprintf(buffer, 1023,format, argptr);

    ret= print_ln(printer_id,buffer);
    va_end(argptr);

    return ret;
}

short print_ln_fmt_centre(short printer_id, short size, char* format, ...)
{
    _TCHAR buffer[1024];
    _TCHAR buffer_centre[1024];
    short ret;

    va_list argptr;
    va_start(argptr, format);

    _vsnprintf(buffer, 1023,format, argptr);
    buffer[1023]=_T('\0');


    ch_memset(buffer_centre, _T(' '), size*sizeof(_TCHAR));
    buffer_centre[size]=_T('\0');

    if (_tcslen(buffer) >= size) {
      buffer[size]=_T('\0');
    }

    fmt_centre_justify_string(buffer_centre,  0, size-1, buffer);
    buffer_centre[size]=_T('\0');

    ret= print_ln(printer_id,buffer_centre);
    va_end(argptr);

    return ret;
}

/*---------------------------------------------------------------------------*/
/*                           get_num_article_prom                            */
/*---------------------------------------------------------------------------*/
//recupera el numero de articulos de promocion no repetidos
int get_num_article_prom(int prom_type)
{
  TM_INDX indx;
  TM_ITEM_GROUP c_item_tmp;
  int article_prom_n=0;
  int i;
  int __found; 
  
  //int gift_type__art_promocion=1;

  long  list_article[1000];
  memset(list_article, sizeof(list_article),0);

  indx = tm_frst(TM_ITEM_NAME, (void*)&c_item_tmp);
  while ( indx != TM_BOF_EOF ) {
    if (!c_item_tmp.voided){ 
      if (is_article_prom(c_item_tmp.arti.base.art_no, prom_type))
      {
        __found =0;
        for (i=0;i<article_prom_n;i++){
          if((list_article[i]==c_item_tmp.arti.base.art_no)){
            __found =1;
            break;
          }
        }
        if (__found==0){
          list_article[article_prom_n]= c_item_tmp.arti.base.art_no;
          article_prom_n++;
        }
      }
    }
    indx = tm_next(TM_ITEM_NAME, (void*)&c_item_tmp);
  }
  return article_prom_n;
} 
/*---------------------------------------------------------------------------*/
/*                           get_num_cupon                                   */
/*---------------------------------------------------------------------------*/
int get_num_cupon(double monto_venta)
{
  
  long num_article_prom =0;
 
  
  if (!ispassday()) // v3.4.9 acm - solo emitir cupon sin no es passday
  {
      if (monto_venta >= prom_anniv_price_cupon) 
      {
          num_article_prom++; // un cupon por ser la venta superior o igual a prom_anniv_price_cupon
          num_article_prom += get_num_article_prom(prom_type_MAKRO_ANIVERSARY);// un cupon por cada articulo d epromocion
          if (num_article_prom> 20) //no se puede entregar mas de 20 mupones
              num_article_prom =20;
      } 
  }
  return num_article_prom;
} 

/*---------------------------------------------------------------------------*/
/*                           get_num_gift                                    */
/*---------------------------------------------------------------------------*/
int get_num_gift(double monto_venta)
{
  if (monto_venta >= prom_anniv_price_gift)
  {
    return 1;
  } 
  return 0;
} 
/* 27-Jan-2012 acm - }*/


 #define ROUNDL( d ) ((long)((d) + ((d) > 0 ? 0.5 : -0.5)))

/*---------------------------------------------------------------------------*/
/*                           get_invoice_rounded                             */
/*---------------------------------------------------------------------------*/
// v3.4.7 acm - {
double get_invoice_rounded(double monto_vuelto) // v3.4.7 acm -
{
    int    centimo=0;

    long  monto_vuelto_1;
    long  monto_vuelto_2;

    double rounded= 0;
    long value    = 0;
    if (monto_vuelto<0.0)
    {
      monto_vuelto_1 = ROUNDL((-1.0*monto_vuelto*100.0));
      monto_vuelto_2 = monto_vuelto_1;

      centimo= monto_vuelto_1%5;
      if (centimo !=0)
      {
         monto_vuelto_2=monto_vuelto_1+(5-centimo);
      }
      rounded= (monto_vuelto_2-monto_vuelto_1)*1.0/100;
    }
    return rounded;
} 
// v3.4.7 acm -} 

// v3.4.8 acm -{ 

/*---------------------------------------------------------------------------*/
/*                           is_cust_prom_horeca                                 */
/*---------------------------------------------------------------------------*/
int is_cust_prom_horeca(long cust_no, long store_no)
{
  CUST_PROM_DEF cust_prom;  //acm - mark
  short status;

  cust_prom.cust_no   = cust_no;
  cust_prom.store_no  = store_no;
  cust_prom.prom_type = prom_type_HORECA;

  status = pos_get_rec(CUST_PROM_TYPE, CUST_PROM_SIZE, CUST_PROM_IDX,
                      (void*)&cust_prom, (short) keyEQL);

  if(status == SUCCEED){
      
      return(1);
  }
  else {
    return(0); 
  }
}

/*---------------------------------------------------------------------------*/
/*                           is_article_prom                                 */
/*---------------------------------------------------------------------------*/
int get_discount_prom_horeca(long art_no, double * valor)
{
  ARTICLE_PROM_DEF article_prom;  //acm - mark
  short status;

  article_prom.art_no   = art_no;
  article_prom.prom_type = prom_type_HORECA;

  status = pos_get_rec(ARTI_PROM_TYPE, ARTICLE_PROM_SIZE, ARTICLE_PROM_IDX,
                      (void*)&article_prom, (short) keyEQL);

  *valor =0;

  if(status == SUCCEED){
      *valor =-article_prom.prom_dvalue;
      return(1); 
  }
  else {
    return(0); 
  }
}

// v3.4.8 acm -} 

// v3.5.0 acm -  {

//===========================================================================================
//                                            BEGIN LIBRARY 
//===========================================================================================


#define REGEDIT_PROMOCION_MAX_ELEM       10
#define REGEDIT_PROMOCION_BYTES_ELEM    100

typedef struct REGEDIT_PROMOCION
{
   char reg_value[REGEDIT_PROMOCION_MAX_ELEM][REGEDIT_PROMOCION_BYTES_ELEM];
} REGEDIT_PROMOCION;

typedef struct ARTICLE_PROM_DEF_USR
{
  ARTICLE_PROM_DEF art_prom;

  short     vat_no;
  double    qty;
  double    price;
  double    goods_value;
  int       rule_type;
  double    rule_result_value;
  TM_INDX   indx;
} ARTICLE_PROM_DEF_USR;

#define MAX_NUM_ARTICLE_LIST  1000






char *strtrim (char *str, char *out)
{
  char *c;

  // Remove leading blanks 
  
  c = str; 
  while (c != '\0' && *c == ' ')
    c++;


  strcpy(out,c);

  // Remove trailing blanks 

  c = out + strlen (out) - 1;
  while (c >= out)
  {
        if (*c == ' ')
	        *c = '\0'; // This causes a bus error, why? 
        else
	        break;
     c--;
  }

  return out;
}


/*---------------------------------------------------------------------------*/
/*                           strnsplitv2                                     */
/*---------------------------------------------------------------------------*/

int strnsplitv2(char* str, REGEDIT_PROMOCION * splitstr, int num_items) 
{
     char* p;
     //char splitbuf[sizeof(REGEDIT_PROMOCION)*3];
     char buffer[REGEDIT_PROMOCION_MAX_ELEM*REGEDIT_PROMOCION_BYTES_ELEM];
     int i=0;

     strncpy(buffer, str,sizeof(buffer)-1);
     buffer[REGEDIT_PROMOCION_MAX_ELEM*REGEDIT_PROMOCION_BYTES_ELEM-1]=0;

     p = strtok(buffer,",");
     while(p!= NULL)
     {
         ///strtrim(
               //strcpy(splitbuf,p);
               strcpy(splitstr->reg_value[i],p);
               //strtrim(splitstr[i]);
               i++;
               p = strtok (NULL, ",");
               if (i>=num_items) 
                   break; /* 27-Jan-2012 acm - se agrego condicional*/
     }
     return i;
}

/*---------------------------------------------------------------------------*/
/*                         isvigente_fechapromocion                          */
/*---------------------------------------------------------------------------*/
int isvigente_fechapromocion(long prom_date_begin, long prom_date_end)
{
  char dest[100]={0};
  long curr_date;

  time_t curtime = time(0); 
  struct tm now=*localtime(&curtime); 
  
  if ((prom_date_begin==0)||(prom_date_end==0)) return 0;

  strftime(dest, sizeof(dest)-1,"%Y%m%d", &now);
  curr_date=atoi(dest);

  if (curr_date>=prom_date_begin && 
      curr_date<=prom_date_end )    return 1;
  
  return 0;
}


/*---------------------------------------------------------------------------*/
/*                           is_article_prom                                 */
/*---------------------------------------------------------------------------*/
int is_article_promv2(long art_no, int prom_type, ARTICLE_PROM_DEF * article_prom)
{
  //ARTICLE_PROM_DEF article_prom;  //acm - mark
  short status;

  article_prom->art_no = art_no;
  article_prom->prom_type=prom_type;
  status = pos_get_rec(ARTI_PROM_TYPE, ARTICLE_PROM_SIZE, ARTICLE_PROM_IDX,
                      (void*)article_prom, (short) keyEQL);

  if(status == SUCCEED)
  {
    return(1);
  }
  else {
    return(0); 
  }
}

int is_article_percepcion(long art_no, ARTICLE_DEF * article)
{
  ARTICLE_DEF tmp_article;  //acm - mark
  short status;

  
  tmp_article.art_no = art_no;
  status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
                      (void*)&tmp_article, (short) keyEQL);
  
  if (article)
    memcpy(article, &tmp_article, sizeof(ARTICLE_DEF) );


  if(status == SUCCEED)
  {
    
      return(tmp_article.arti_perception_ind);
  }
  else {
    return(0); 
  }
}

int is_article_detraccion(long art_no)//, ARTICLE_DEF * article)
{
  ARTICLE_DEF tmp_article;  //acm - mark
  short status;

  
  tmp_article.art_no = art_no;
  status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
                      (void*)&tmp_article, (short) keyEQL);
/*  
  if (article)
    memcpy(article, &tmp_article, sizeof(ARTICLE_DEF) );
*/

  if(status == SUCCEED)
  {
    return(tmp_article.arti_detraccion_ind);
  }
  else {
    return(0); 
  }
}

/*---------------------------------------------------------------------------*/
/*                           get_num_article_prom                            */
/*---------------------------------------------------------------------------*/
//recupera el numero de articulos de promocion no repetidos
int get_num_article_promv2(int prom_type, ARTICLE_PROM_DEF_USR list_article[])
{
  TM_INDX indx;
  TM_ITEM_GROUP c_item_tmp;
  int article_prom_n=0;
  int i;
  int __found; 
  ARTICLE_PROM_DEF article_prom;  //acm - mark
  //ARTICLE_PROM_DEF list_article[1000];

  ///ARTICLE_PROM_DEF *part_list;

  //part_list=&art_list[0];
  
  //int gift_type__art_promocion=1;
  /*
  long  list_article[1000];
  memset(list_article, sizeof(list_article),0);
  */

  article_prom_n=0;
  indx = tm_frst(TM_ITEM_NAME, (void*)&c_item_tmp);
  while ( indx != TM_BOF_EOF ) {
    if (!c_item_tmp.voided){ 
      memset(&article_prom, 0, sizeof(ARTICLE_PROM_DEF));

      if (is_article_promv2(c_item_tmp.arti.base.art_no, prom_type, &article_prom))
      {
          __found =0;
        for (i=0;i<article_prom_n;i++){
          if((list_article[i].art_prom.art_no==c_item_tmp.arti.base.art_no)){
            __found =1;
            break;
          }
        }
        //if (__found==0)
        if (c_item_tmp.arti.accumulated==FALSE)
        {
              list_article[article_prom_n].art_prom.delflg      = article_prom.delflg       ;
              list_article[article_prom_n].art_prom.art_no      = article_prom.art_no       ;
              list_article[article_prom_n].art_prom.prom_type   = article_prom.prom_type    ;
              list_article[article_prom_n].art_prom.art_status  = article_prom.art_status   ;
              list_article[article_prom_n].art_prom.prom_dvalue = article_prom.prom_dvalue  ;

              list_article[article_prom_n].vat_no               = c_item_tmp.arti.base.vat_no      ;
              list_article[article_prom_n].goods_value          = c_item_tmp.arti.base.goods_value ;
              list_article[article_prom_n].price                = c_item_tmp.arti.base.price       ;
              list_article[article_prom_n].qty                  = c_item_tmp.arti.base.qty         ;

              article_prom_n++;
        }
      }
    }
    indx = tm_next(TM_ITEM_NAME, (void*)&c_item_tmp);
  }
  return article_prom_n;
} 


int find_article_def(ARTICLE_PROM_DEF_USR * articles, int art_no)
{
    int __found;
    int i;
    int article_index=-1;

      __found =0;

    for (i=0;i<  i< MAX_NUM_ARTICLE_LIST; i++)
    {
        if (articles[i].art_prom.art_no==0)
        {
            article_index=-1;
            break;
        }

      if((articles[i].art_prom.art_no==art_no)){
        __found =1;
        article_index=i;
        break;
      }
    }
   return article_index;
}


double getPerceptionBase(TM_ITEM_GROUP c_item_tmp)
{
    double goods_value;
    double discount;
    goods_value=c_item_tmp.arti.base.goods_value;
    discount=floor_price(calc_incl_vat(c_item_tmp.arti.msam_disc1+ c_item_tmp.arti.msam_disc2, c_item_tmp.arti.base.vat_no));

    goods_value+=c_item_tmp.disc.base.goods_value+discount;

    //floor_price(calc_incl_vat(deposit.sell_pr, deposit.vat_no));


    return goods_value;
}
/*---------------------------------------------------------------------------*/
/*                           get_num_article_percepcion                      */
/*---------------------------------------------------------------------------*/
//recupera el numero de articulos de promocion no repetidos
int get_num_article_percepcion(ARTICLE_PROM_DEF_USR list_article[], 
                               char doc_type, 
                               double factor_perc,
                               double *out_total_value)
{
  TM_INDX       indx;
  TM_ITEM_GROUP c_item_tmp;
  double goods_value;
//  char      doc_type;
  double    total_value =0;
  int       __found; 
  double     rule_result_value_total = 0;
  long      art_no;
  int       __update;
  int   __perception_calculed;

  
  ARTICLE_DEF article;  //acm - mark
  
  ARTICLE_PROM_DEF_USR article_unique[MAX_NUM_ARTICLE_LIST];


  int article_n             =0;
  int article_index         =0;
  
  int article_unique_n      =0;
  int article_unique_index  =0;


  factor_perc= factor_perc/100;

  memset(article_unique, 0 , sizeof(article_unique));  

  
  article_n=0;
  indx = tm_frst(TM_ITEM_NAME, (void*)&c_item_tmp);
  while ( indx != TM_BOF_EOF ) {
    if (!c_item_tmp.voided){ 
      memset(&article, 0, sizeof(ARTICLE_DEF));

      if (is_article_percepcion(c_item_tmp.arti.base.art_no, &article))
      {
        /*
          __found =0;
        for (i=0;i<article_unique_n;i++){
          if((article_unique[i].art_prom.art_no==c_item_tmp.arti.base.art_no)){
            __found =1;
            article_unique_index=i;
            break;
          }
        }
        */
        // AGREGAR ARTICULO
        list_article[article_n].indx                 = indx ;

        list_article[article_n].art_prom.delflg      = article.delflg       ;
        list_article[article_n].art_prom.art_no      = article.art_no       ;
        list_article[article_n].art_prom.prom_type   = 0;//article.prom_type    ;
        list_article[article_n].art_prom.art_status  = article.art_status   ;
        list_article[article_n].art_prom.prom_dvalue = 0;//article.prom_dvalue  ;

        list_article[article_n].vat_no               = c_item_tmp.arti.base.vat_no      ;
        list_article[article_n].price                = c_item_tmp.arti.base.price       ;

        goods_value=getPerceptionBase(c_item_tmp); //goods_value=c_item_tmp.arti.base.goods_value;

        list_article[article_n].goods_value          = goods_value;//c_item_tmp.arti.base.goods_value ;
        list_article[article_n].qty                  = c_item_tmp.arti.base.qty         ;

        list_article[article_n].rule_type            = c_item_tmp.arti.base.arti_rule_ind ;
        list_article[article_n].rule_result_value    = 0  ;

        article_unique_index = find_article_def(article_unique, c_item_tmp.arti.base.art_no);
        if (article_unique_index !=-1) //found
        {
              article_unique[article_unique_index].goods_value += list_article[article_n].goods_value ;
              article_unique[article_unique_index].qty         += list_article[article_n].qty         ;

            __found =1;        
        }else
        {

            article_unique[article_unique_n].indx                 = list_article[article_n].indx ;

            article_unique[article_unique_n].art_prom.delflg      = list_article[article_n].art_prom.delflg      ;
            article_unique[article_unique_n].art_prom.art_no      = list_article[article_n].art_prom.art_no      ;
            article_unique[article_unique_n].art_prom.prom_type   = list_article[article_n].art_prom.prom_type   ;
            article_unique[article_unique_n].art_prom.art_status  = list_article[article_n].art_prom.art_status;
            article_unique[article_unique_n].art_prom.prom_dvalue = list_article[article_n].art_prom.prom_dvalue;

            article_unique[article_unique_n].vat_no               = list_article[article_n].vat_no ;
            article_unique[article_unique_n].price                = list_article[article_n].price  ;

            article_unique[article_unique_n].goods_value          = list_article[article_n].goods_value ;
            article_unique[article_unique_n].qty                  = list_article[article_n].qty         ;

            article_unique[article_unique_n].rule_type            = list_article[article_n].rule_type        ;
            article_unique[article_unique_n].rule_result_value    = list_article[article_n].rule_result_value;

            article_unique_n++;
            article_unique[article_unique_n].art_prom.art_no=0;

            __found =0;
        }
        article_n++;
        list_article[article_n].art_prom.art_no=0;

      }
    }
    indx = tm_next(TM_ITEM_NAME, (void*)&c_item_tmp);
  }

  __perception_calculed = 0;
    //calculo de monto de la percepcion
    if (factor_perc > POS_ZERO_VALUE)
    {
        if (doc_type=='F')
        {
            for (article_index=0;article_index<article_n; article_index++)
            {
               tm_read_nth(TM_ITEM_NAME, (void*)&c_item_tmp, (short) list_article[article_index].indx);

               if (c_item_tmp.arti.base.arti_perception_ind==1)
               {
                   goods_value=getPerceptionBase(c_item_tmp); //goods_value=c_item_tmp.arti.base.goods_value;
                   c_item_tmp.arti.base.percep_amount   = goods_value*factor_perc;

                   tm_upda_nth(TM_ITEM_NAME, (void*)&c_item_tmp, (short) list_article[article_index].indx);

                   __perception_calculed=1;
               } else
               {
                    //c_item_tmp.arti.base.percep_amount =0;  // fix 
               }
            }
        }
        else if (doc_type=='B')
        {
              for (article_index=0;article_index<article_n; article_index++)
              {
              
                  art_no  = list_article[article_index].art_prom.art_no;

                  article_unique_index = find_article_def(article_unique,  art_no );

                  __update =0;
                  if (article_unique[article_unique_index].rule_type==1)     // SIN MONTO
                  {
                      if (fabs(article_unique[article_unique_index].qty)>=2)
                      {
                            __update =1;
                      }

                  }
                  else if (article_unique[article_unique_index].rule_type==2)
                  {
                      if ((fabs(article_unique[article_unique_index].qty)>=2)&&(fabs(article_unique[article_unique_index].goods_value)>1500))   // MAYOR QUE 1500
                      {
                            __update =1;
                      }
                  }
                  else if (article_unique[article_unique_index].rule_type==3)
                  {
                      if ((fabs(article_unique[article_unique_index].qty)>=2)&&(fabs(article_unique[article_unique_index].goods_value)>100))    // MAYOR QUE 100
                      {
                            __update =1;
                      }
                  }

                  if (__update )
                  {
                       tm_read_nth(TM_ITEM_NAME, (void*)&c_item_tmp, (short) list_article[article_index].indx);

                       goods_value=getPerceptionBase(c_item_tmp);  //c_item_tmp.arti.base.goods_value
 
                       c_item_tmp.arti.base.percep_amount   = goods_value*factor_perc;

                       tm_upda_nth(TM_ITEM_NAME, (void*)&c_item_tmp, (short) list_article[article_index].indx);

                       __perception_calculed=1;
                  }else
                  {
                      if (c_item_tmp.arti.base.percep_amount!=0)
                      {
                           tm_read_nth(TM_ITEM_NAME, (void*)&c_item_tmp, (short) list_article[article_index].indx);
                           c_item_tmp.arti.base.percep_amount      = 0;
                           tm_upda_nth(TM_ITEM_NAME, (void*)&c_item_tmp, (short) list_article[article_index].indx);
                      }
                  }

              }
        }
  
    }
  

    //totalizar la percepcion
    total_value = 0;
    if (__perception_calculed)
    {
        indx = tm_frst(TM_ITEM_NAME, (void*)&c_item_tmp);
        while ( indx != TM_BOF_EOF ) 
        {
            if (!c_item_tmp.voided){ 
                
                total_value+=c_item_tmp.arti.base.percep_amount;
            }

            indx = tm_next(TM_ITEM_NAME, (void*)&c_item_tmp);
        }
    }
    /*
    for (article_index=0; article_index<article_n; article_index++)
    {
       total_value+=list_article[article_index].rule_result_value  ;
    }
    */
  *out_total_value = total_value;
  return article_n;
} 


/*---------------------------------------------------------------------------*/
/*                           read_regedit_promocion                          */
/*---------------------------------------------------------------------------*/

int  read_regedit_promocion(char * regname,  REGEDIT_PROMOCION * regedit_promocion)
{
    int reg_buffer_num_elem=0;
    int i=0;
    _TCHAR reg_buffer[REGEDIT_PROMOCION_BYTES_ELEM*REGEDIT_PROMOCION_MAX_ELEM];


    for  (i=0; i< REGEDIT_PROMOCION_MAX_ELEM;i++)
    {
        regedit_promocion->reg_value[i][0]=0;
    }

    SetRegistryEcho(FALSE);
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, regname, reg_buffer, sizeof(reg_buffer)-1);
    SetRegistryEcho(TRUE);

    if (strlen(reg_buffer)>0)
    {
        reg_buffer_num_elem = strnsplitv2(reg_buffer, regedit_promocion,REGEDIT_PROMOCION_MAX_ELEM);
    }

    return reg_buffer_num_elem;
}


/*---------------------------------------------------------------------------*/
/*                           read_regedit_value                              */
/*---------------------------------------------------------------------------*/

char * read_regedit_value(char * regname, char *default_value)
{
    static _TCHAR reg_buffer[REGEDIT_PROMOCION_BYTES_ELEM*REGEDIT_PROMOCION_MAX_ELEM];
    memset(reg_buffer, 0,sizeof(reg_buffer));

    SetRegistryEcho(FALSE);
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, regname, reg_buffer, sizeof(reg_buffer)-1);
    if (strlen(reg_buffer)==0)
    {
        strcpy(reg_buffer,default_value);
    }
    SetRegistryEcho(TRUE);

    return reg_buffer;
}
//===========================================================================================
//                                            END LIBRARY 
//===========================================================================================

/*---------------------------------------------------------------------------*/
/*                           Str2Long                                        */
/*---------------------------------------------------------------------------*/
long Str2Long(char * str, long default_value)
{
  long value;
  
  if (!str[0])
  {
    value=default_value;
  }else
  {
    value=atol(str);
  }
  return value;
}


void configuration_regedit_PROMOCION_CINE();
void configuration_regedit_FERIA_ESCOLAR();
void configuration_regedit_PERCEPCION();
void configuration_regedit_FIESTA_FUTBOL();
void configuration_regedit_CUPON_GLOBAL();

int is_promocion_FERIA_ESCOLAR( int * p_vigente );


void configuration_regedit_all()
{
  static int loaded=0;

  if (loaded) return;

  configuration_regedit_PROMOCION_CINE();
  configuration_regedit_FERIA_ESCOLAR();
  configuration_regedit_PERCEPCION();
  configuration_regedit_FIESTA_FUTBOL();
  configuration_regedit_CUPON_GLOBAL();

  loaded=1;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
long prom_type_CINE_Fecha_Ini ;
long prom_type_CINE_Fecha_Fin ;
long prom_type_CINE_Amount    ;

/*---------------------------------------------------------------------------*/
/*                         configuration_regedit_all                         */
/*---------------------------------------------------------------------------*/
void configuration_regedit_PROMOCION_CINE()
{
  static int loaded=0;
  REGEDIT_PROMOCION regedit_promocion;
  int reg_buffer_num_elem;

  //if (loaded) return;

  reg_buffer_num_elem       = read_regedit_promocion("PROMOCION_CINE",&regedit_promocion);

  prom_type_CINE_Amount     = Str2Long(regedit_promocion.reg_value[0],200);;
  prom_type_CINE_Fecha_Ini  = Str2Long(regedit_promocion.reg_value[1],20131101);
  prom_type_CINE_Fecha_Fin  = Str2Long(regedit_promocion.reg_value[2],20131130);


  //loaded=1;
}


typedef struct TProm_art_amount
{
    int size;
    double value;
    double   qty;
} TProm_art_amount;

/*---------------------------------------------------------------------------*/
/*                         is_promocion_cine                                 */
/*---------------------------------------------------------------------------*/
int is_promocion_cine(double monto_venta, long fini, long ffin)
{
  long num_article_prom =0;
  TProm_art_amount art[3];



  ARTICLE_PROM_DEF_USR list_article[MAX_NUM_ARTICLE_LIST];
  int nsize=0;
  int i=0;
  int marca;

///  configuration_promocion_cine();
  if (!isvigente_fechapromocion(fini,ffin)) return 0;

  memset(list_article, 0, sizeof(list_article)  );
  memset(art         , 0, sizeof(art)           );

  num_article_prom=get_num_article_promv2(prom_type_CINE /*4*/, list_article);
  for (i=0;i<num_article_prom;i++)
  {
      marca= ((int)list_article[i].art_prom.prom_dvalue);
      art[marca-1].size=1;
      art[marca-1].value+=list_article[i].goods_value;
  }

  /// Los articulos deben ser de al menos 2 marcas 
  if (art[0].size+ art[1].size +art[2].size<=1)
  {
      return 0;
  }
  /// El monto debe ser mayor o igual que 200 
  if (art[0].value+ art[1].value +art[2].value<monto_venta)
  {
      return 0;
  }
  return 1;
} 
/*---------------------------------------------------------------------------*/
/*                         check_print_promotion                             */
/*---------------------------------------------------------------------------*/
extern int is_PERCEPCION( int * p_vigente, 
                         double * p_total_perception,
                         double * p_porcentaje) ; ///double monto_venta, long fini, long ffin)

void check_print_promotion(short printer)
{
 
    int ix;
    if (is_promocion_cine(prom_type_CINE_Amount,prom_type_CINE_Fecha_Ini,prom_type_CINE_Fecha_Fin))
    {
        print_ln(printer, empty);
        print_ln_fmt_centre(printer, 40,prn_inv_TXT[84]);// GANO ENTRADA AL CINE
        //print_ln_fmt_centre(printer, 40,prn_invF_TXT[84]);// GANO ENTRADA AL CINE
        //return ;
    }

    {
        int vigencia_date=0;
        int num_cupon=is_promocion_FERIA_ESCOLAR(&vigencia_date);
        
        if (num_cupon)
        {
            print_ln(printer, empty);
            if (num_cupon==1)
                print_ln_fmt_centre(printer, 40,prn_inv_TXT[85],num_cupon);// 
            else
                 print_ln_fmt_centre(printer, 40,prn_inv_TXT[86],num_cupon);// 

        }else{
            if (vigencia_date)
            {
                print_ln(printer, empty);
                print_ln_fmt_centre(printer, 40,prn_inv_TXT[87],num_cupon);// 
            }
        }
    }


    {
        int vigencia_date=0;
        int num_cupon=is_promocion_FIESTA_FUTBOL(&vigencia_date);
        
        if (num_cupon)
        {
            print_ln(printer, empty);
            if (num_cupon==1)
                print_ln_fmt_centre(printer, 40,prn_inv_TXT[96],num_cupon);// 
            else
                 print_ln_fmt_centre(printer, 40,prn_inv_TXT[97],num_cupon);// 

        }else{
            if (vigencia_date)
            {
                print_ln(printer, empty);
                print_ln_fmt_centre(printer, 40,prn_inv_TXT[98],num_cupon);// 
            }
        }
    }

    {

        int vigencia_date=0;
        for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
        {
            if (cupon_global_vigente(ix)) 
            {
                int num_cupon=is_promocion_CUPON_GLOBAL(&vigencia_date,ix);
        
                if (num_cupon)
                {
                    print_ln(printer, empty);
                    if (num_cupon==1)
                        print_ln_fmt_centre(printer, 40,cupon_global_inv_TXT[ix][0],num_cupon);// 
                    else
                         print_ln_fmt_centre(printer, 40,cupon_global_inv_TXT[ix][1],num_cupon);// 

                }else{
                    if (vigencia_date)
                    {
                        print_ln(printer, empty);
                        print_ln_fmt_centre(printer, 40,cupon_global_inv_TXT[ix][2],num_cupon);// 
                    }
                }
            }
       }
    }

  /*mlsd P*/
  /* Aqui genera el footer con percepcion */
 /* if (IS_GEN_PERCEPTION)
  {
      char buff0[100];
      user_ftoa_porcentaje(PERC_GEN_PERCEPTION, buff0);*/
      //++//++//
/*      print_ln(printer, empty);
      print_ln_fmt_centre(printer, 40, prn_inv_TXT[89]);// 

      if (IS_ANTICIPO()){
          print_ln_fmt_centre(printer, 40, prn_inv_TXT[90], buff0, 
              get_invoice_perception_currmode() 
              -cur_anticipo.percepcion*/
              /*TOT_GEN_PERCEPTION*//*);//3.6.2 acm - 
      
      }else {
          print_ln_fmt_centre(printer, 40, prn_inv_TXT[90], buff0, 
              get_invoice_perception_currmode() */
              /*TOT_GEN_PERCEPTION*/ /*);//3.6.2 acm - 
      }
  }*/

    return ;
}

int get_num_cupon_cine()
{
    if (is_promocion_cine(prom_type_CINE_Amount,prom_type_CINE_Fecha_Ini,prom_type_CINE_Fecha_Fin))
    {
        return 1;
    }
    return 0;
}
int get_num_vale_pavo()
{
   return cur_valepavo.tag;
}

int is_payment_valepavo()
{
    if (get_num_vale_pavo())
    {
    
        return 0;    
    }
    return 0;
}

// v3.5.0 acm -  } 

// v3.5.1 acm -  { 
void printf_ln(short printer ,  const  char *format, ...)
{
	va_list args;
    char buffer[1000];

    va_start(args,format);

	vsprintf(buffer, format, args);
    print_ln(printer, buffer);

	va_end(args);
}


/*---------------------------------------------------------------------------*/
/*                           is_cust_prom_horeca                             */
/*---------------------------------------------------------------------------*/

int isValeTurkey_Valid(long valepavo_no, long vale_type)
{
  short status;

  cur_valepavo.vale_no      = valepavo_no;
  cur_valepavo.vale_type    = vale_type;
  cur_valepavo.tag          = 0;


  status = pos_get_rec(VALEPAVO_PROM_TYPE, VALEPAVO_PROM_SIZE, VALEPAVO_PROM_IDX,
                      (void*)&cur_valepavo, (short) keyEQL);

  if((status == SUCCEED)&&(cur_valepavo.status==0))
  {
    cur_valepavo.tag=1;
    return(1);
  }
  else {
    cur_valepavo.tag=0;  
    return(0); 
  }
}

VALEPAVO_PROM_DEF  cur_valepavo;
ANTICIPO_PROM_DEF  cur_anticipo;

int init_invoice_user(_TCHAR *data, short key)
{
    TOT_GEN_PERCEPTION = 0 ;

    memset(&cur_valepavo, 0,sizeof(VALEPAVO_PROM_DEF));
    cur_valepavo.tag=0;  ///

	usr_document[0]=0;
    usr_name[0]=0;
	
    usr_perception_document[0]=0;
    usr_perception_name[0]=0;


    memset(&cur_anticipo, 0,sizeof(ANTICIPO_PROM_DEF));
    cur_anticipo.tag=0;  ///

    return key;
}
// v3.5.1 acm -  }

int printf_log(char* format, ...)
{
  char buffer[16384];
  short ret;
  va_list argptr;

  FILE *f;

  char bufftime[20];
  struct tm *sTm;

  time_t now = time (0);
  sTm = gmtime (&now);

  strftime (bufftime, sizeof(bufftime), "%Y-%m-%d %H:%M:%S", sTm);
  //printf ("%s %s\n", buff, "Event occurred now");

  f=fopen("winpos_pe.log","ab");

  va_start(argptr, format);

  memset(buffer, 0, sizeof(buffer));
  _vsnprintf(buffer, 16383,format, argptr);

  ret= fprintf(f,"%s : %s\n", bufftime, buffer);
  //ret = fwrite(buffer, sizeof(char), strlen(buffer), f);
  va_end(argptr);

  fclose(f);
  return 1;
}



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
long prom_type_FERIA_ESCOLAR_Fecha_Ini ;
long prom_type_FERIA_ESCOLAR_Fecha_Fin ;
long prom_type_FERIA_ESCOLAR_Amount    ;

/*---------------------------------------------------------------------------*/
/*                         configuration_regedit_FERIA_ESCOLAR               */
/*---------------------------------------------------------------------------*/
void configuration_regedit_FERIA_ESCOLAR()
{
  static int loaded=0;
  REGEDIT_PROMOCION regedit_promocion;
  int reg_buffer_num_elem;

  //if (loaded) return;

  reg_buffer_num_elem       = read_regedit_promocion("FERIA_ESCOLAR",&regedit_promocion);

  prom_type_FERIA_ESCOLAR_Amount     = Str2Long(regedit_promocion.reg_value[0],40);;
  prom_type_FERIA_ESCOLAR_Fecha_Ini  = Str2Long(regedit_promocion.reg_value[1],20140201);
  prom_type_FERIA_ESCOLAR_Fecha_Fin  = Str2Long(regedit_promocion.reg_value[2],20140314);


  //loaded=1;
}


/*---------------------------------------------------------------------------*/
/*                         is_promocion_FERIA_ESCOLAR                        */
/*---------------------------------------------------------------------------*/

  //ARTICLE_PROM_DEF_USR list_article[1000]; FIX
int is_promocion_FERIA_ESCOLAR( int * p_vigente )///double monto_venta, long fini, long ffin)
{
  
  double monto_venta = prom_type_FERIA_ESCOLAR_Amount; 
  long fini          = prom_type_FERIA_ESCOLAR_Fecha_Ini; 
  long ffin          = prom_type_FERIA_ESCOLAR_Fecha_Fin;


  long num_article_prom =0;
  TProm_art_amount art[3];



  ARTICLE_PROM_DEF_USR list_article[MAX_NUM_ARTICLE_LIST];
  int nsize=0;
  int i=0;
  int marca;
  int num_cupones=0;

///  configuration_promocion_cine();
  if (!isvigente_fechapromocion(fini,ffin)) {
      if (p_vigente!=NULL) *p_vigente=0;
      return 0;
  } else{
      if (p_vigente!=NULL) *p_vigente=1;
  }

  memset(list_article, 0, sizeof(list_article)  );
  memset(art         , 0, sizeof(art)           );

  num_article_prom=get_num_article_promv2(prom_type_FERIA_ESCOLAR /*4*/, list_article);
  for (i=0;i<num_article_prom;i++)
  {
      marca= 1;//((int)list_article[i].art_prom.prom_dvalue);
      art[marca-1].size=1;
      art[marca-1].value+=list_article[i].goods_value;
      //art[marca-1].qty  +=list_article[i].q;
  }

  /// Los articulos deben ser de al menos 2 marcas 
  /*
  if (art[0].size+ art[1].size +art[2].size<=1)
  {
      return 0;
  }
  */
  /// El monto debe ser mayor o igual que 200 
  if (art[0].value + art[1].value +art[2].value<monto_venta)
  {
      return 0;
  }
  num_cupones= (int)( ((long)art[0].value) / ((long)monto_venta));
  return num_cupones;
} 
/*---------------------------------------------------------------------------*/
/*                         get_num_cupon_FERIA_ESCOLAR                       */
/*---------------------------------------------------------------------------*/

int get_num_cupon_FERIA_ESCOLAR()
{
    int num_promocion=0;
    int vigencia_date=0;

    num_promocion=is_promocion_FERIA_ESCOLAR( &vigencia_date );

    if (num_promocion)
    {
        return num_promocion;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
long prom_type_PERCEPCION_Fecha_Ini ;
long prom_type_PERCEPCION_Fecha_Fin ;
long prom_type_PERCEPCION_Amount    ;

/*---------------------------------------------------------------------------*/
/*                         configuration_regedit_PERCEPCION                  */
/*---------------------------------------------------------------------------*/
void configuration_regedit_PERCEPCION()
{
  static int loaded=0;
  REGEDIT_PROMOCION regedit_promocion;
  int   reg_buffer_num_elem;

  //if (loaded) return;


  PERCEPTION_ENABLED=0;
  if (strcmp(read_regedit_value("PERCEPTION_ENABLED","1"),"1")==0) // son iguales 
  {
        PERCEPTION_ENABLED=1;
  }

  reg_buffer_num_elem       = read_regedit_promocion("PERCEPCION",&regedit_promocion);

  prom_type_PERCEPCION_Amount     = Str2Long(regedit_promocion.reg_value[0],1500);;
  prom_type_PERCEPCION_Fecha_Ini  = Str2Long(regedit_promocion.reg_value[1],20140401);// empieza en abril
  prom_type_PERCEPCION_Fecha_Fin  = Str2Long(regedit_promocion.reg_value[2],21140301);

  //loaded=1;
}


/*---------------------------------------------------------------------------*/
/*                         is_PERCEPCION                                     */
/*---------------------------------------------------------------------------*/
int is_PERCEPCION( int * p_vigente, 
                  double * p_total_perception, 
                  double * p_porcentaje)///double monto_venta, long fini, long ffin)
{
  
  double monto_venta = prom_type_PERCEPCION_Amount; 
  long fini          = prom_type_PERCEPCION_Fecha_Ini; 
  long ffin          = prom_type_PERCEPCION_Fecha_Fin;


  long num_article_prom =0;
  TProm_art_amount art[3];



  ARTICLE_PROM_DEF_USR list_article[MAX_NUM_ARTICLE_LIST];
  int nsize=0;
  int i=0;
//  int marca;
  int num_cupones=0;
  double  factor_perception=0;
  char doc_type;
  double total_perception =0   ;


  if (!PERCEPTION_ENABLED)
  {
    return 0; 
  }

///  configuration_promocion_cine();
  if (!isvigente_fechapromocion(fini,ffin)) {
      if (p_vigente!=NULL) *p_vigente=0;
      return 0;
  } else{
      if (p_vigente!=NULL) *p_vigente=1;
  }

  if (is_client_pay_FACTURA()) // cliente con RUC
  {
    if (cust.cust_ret_agent_ind==1) 
    {
       factor_perception=0; //Si el cliente tiene RUC y es Agente de Retención: No Aplica calculo
    }
    else if (cust.cust_except_ind==1)
    {
        factor_perception=0; 
    }
    else  
    {
        if (cust.cust_perc_agent_ind ==1)
          factor_perception=0.5;  //Si el cliente tiene RUC y es Agente de Percepcion: Aplica el 0.5 % del total de artículos con IGV sujetos a Percepcion.
        else
          factor_perception=2;    //Si el cliente tiene RUC y no cumple con las condiciones anteriores se le aplica el 2% del total de artículos con IGV sujetos Percepcion (No se considera importe) 
    }
  }
  else
  {
      factor_perception=2; //Si el cliente tiene DNI (Consumidor Final), se aplica el 2% del total de artículos con IGV sujetos a Percepcion que supere los S/1,500.00.
  }

    // 1 si es agente de retencion y percepcion -- aplica el calculo de 0.5% o no

/*
    Si el cliente tiene RUC y es Agente de Retención: No Aplica calculo
    Si el cliente tiene RUC y es Agente de Percepcion: Aplica el 0.5 % del total de artículos con IGV sujetos a Percepcion.
    Si el cliente tiene RUC y no cumple con las condiciones anteriores se le aplica el 2% del total de artículos con IGV sujetos Percepcion (No se considera importe) 
    Si el cliente tiene DNI (Consumidor Final), se aplica el 2% del del total de artículos con IGV sujetos a Percepcion que supere los S/1,500.00.
*/


  memset(list_article, 0, sizeof(list_article)  );
  memset(art         , 0, sizeof(art)           );
  

  
  if ((selected_invoice_printer + 1)==1) //FACTURA
      doc_type='F';

  if ((selected_invoice_printer + 1)==2) //BOLETA
      doc_type='B';

  num_article_prom = get_num_article_percepcion(list_article, doc_type ,factor_perception, 
                                                &total_perception);

  if (p_total_perception)
     *p_total_perception = total_perception;

  if (p_porcentaje)
     *p_porcentaje = factor_perception;

  

  /*
  for (i=0;i<num_article_prom;i++)
  {
      marca= 1;//((int)list_article[i].art_prom.prom_dvalue);
      art[marca-1].size=1;
      art[marca-1].value+=list_article[i].goods_value;
  }
    */
  /// Los articulos deben ser de al menos 2 marcas 
  /*
  if (art[0].size+ art[1].size +art[2].size<=1)
  {
      return 0;
  }
  */
  /// El monto debe ser mayor o igual que 200 
  /*
  if (art[0].value + art[1].value +art[2].value<monto_venta)
  {
      return 0;
  }
  */
//  num_cupones= (int)( ((long)art[0].value) / ((long)monto_venta));
  if ((num_article_prom>0 )&& ( fabs(total_perception)>0.0001))
      return 1;
  else

     return 0;
} 

/*---------------------------------------------------------------------------*/
/*                         get_num_cupon_FERIA_ESCOLAR                       */
/*---------------------------------------------------------------------------*/
/*
int get_num_cupon_FERIA_ESCOLAR()
{
    int num_promocion=0;
    int vigencia_date=0;

    num_promocion=is_promocion_FERIA_ESCOLAR( &vigencia_date );

    if (num_promocion)
    {
        return num_promocion;
    }
    return 0;
}
*/
int is_client_pay_FACTURA()
{
   int is_print_factura=0;

   if (selected_invoice_printer==0) //selecciono Factura
       is_print_factura= 1;
   else 
       is_print_factura= 0;

   return is_print_factura;
}

void user_ftoa_porcentaje(double value, char *out_value)
{
    long val;

    val=  (long) value;
    if ((value -val) >0.0001)
        sprintf(out_value,"%3.1f" , value);
    else 
        sprintf(out_value,"%.f"  , value);
    
    strcat(out_value,"%");

}





double mod (double value , long divisor)
{
    return (long) fmod(value ,divisor );
}
char * substr(char * p, unsigned int position, int n)
{
  static char buffer[1000];

  buffer[0]=0;
  if (strlen(p)>= position+n)
  {
    strncpy(buffer,p+ position,n);
    buffer[n]=0;
  }
  return buffer;
}

long to_number(char * p)
{
  return atol(p);
}

int pe_val_dni( char  * txtDni) 
{
    int ret;

    if ((txtDni) &&(strlen(txtDni)==8))
        ret=1;
    else 
        ret=0;
    
    return ret;
}
int pe_val_ruc( char  * txtNroRuc) 
{
  int lnSuma              ;
  int lnResiduo           ;
  int lnResta             ;
  int lnDigitoVerificador ;

  
  lnSuma=0;
  lnSuma=lnSuma+to_number(substr(txtNroRuc, 1,1))*5;
  lnSuma=lnSuma+to_number(substr(txtNroRuc, 2,1))*4;
  lnSuma=lnSuma+to_number(substr(txtNroRuc, 3,1))*3;
  lnSuma=lnSuma+to_number(substr(txtNroRuc, 4,1))*2;
  lnSuma=lnSuma+to_number(substr(txtNroRuc, 5,1))*7;
  lnSuma=lnSuma+to_number(substr(txtNroRuc, 6,1))*6;
  lnSuma=lnSuma+to_number(substr(txtNroRuc, 7,1))*5;
  lnSuma=lnSuma+to_number(substr(txtNroRuc, 8,1))*4;
  lnSuma=lnSuma+to_number(substr(txtNroRuc, 9,1))*3;
  lnSuma=lnSuma+to_number(substr(txtNroRuc,10,1))*2;
  lnResiduo=mod(lnSuma,11);
  lnResta=11-lnResiduo;

  if (lnResta==10) 
    lnDigitoVerificador=0;
  else{
      if(lnResta==11) 
        lnDigitoVerificador=1;
      else
        lnDigitoVerificador=lnResta;
  }

  if (lnDigitoVerificador==to_number(substr(txtNroRuc,11,1)) ) 
    return 1;
  else
    return 0;
}

int getNameFromCust_Perc( _TCHAR  * dni, CUST_PERC_DEF * cust_perc)
{
    short status;

    CUST_PERC_DEF   cur_cust_perc;


    memset(&cur_cust_perc,0, sizeof(CUST_PERC_DEF));
    memset(cust_perc     ,0, sizeof(CUST_PERC_DEF));


    memset (cur_cust_perc.fisc_no, 0,     sizeof(cur_cust_perc.fisc_no)); 
    _tcscpy(cur_cust_perc.fisc_no, AnsiToUnicode(dni));

    status = pos_get_rec(CUST_PERC_TYPE, DEF_CUST_PERC_SIZE, CUST_PERC_FISCNO_IDX,
                      (void*)&cur_cust_perc, (short) keyEQL);

    if(status == SUCCEED)
    {
        memcpy(cust_perc, &cur_cust_perc, sizeof(CUST_PERC_DEF));
        return(1);
    }
    else {
        return(0); 
    }
}


int getNameFromCardHolder( _TCHAR  * dni, CARDHOLDER_DEF * cardholder)
{
    short status;

    CARDHOLDER_DEF   cur_cardholder;


    memset(&cur_cardholder,0, sizeof(CARDHOLDER_DEF));
    memset(cardholder     ,0, sizeof(CARDHOLDER_DEF));


    cur_cardholder.cust_key  =(cust.store_no * 1000000) + 
                               cust.cust_no;
     
    memset (cur_cardholder.doc_no, 0,     sizeof(cur_cardholder.doc_no)); 
    _tcscpy(cur_cardholder.doc_no,      AnsiToUnicode(dni));

    status = pos_get_rec(CARDHOLDER_TYPE, DEF_CARDHOLDER_SIZE, CARDHOLDER_FISCNO_IDX,
                      (void*)&cur_cardholder, (short) keyEQL);

    if(status == SUCCEED)
    {
        memcpy(cardholder, &cur_cardholder, sizeof(CARDHOLDER_DEF));
        return(1);
    }
    else { 
        return(0); 
    }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
long prom_type_FIESTA_FUTBOL_Fecha_Ini ;
long prom_type_FIESTA_FUTBOL_Fecha_Fin ;
long prom_type_FIESTA_FUTBOL_Amount    ;
long prom_type_FIESTA_FUTBOL_Cupon_MAX ;

/*---------------------------------------------------------------------------*/
/*                         configuration_regedit_FIESTA_FUTBOL               */
/*---------------------------------------------------------------------------*/
void configuration_regedit_FIESTA_FUTBOL()
{
  static int loaded=0;
  REGEDIT_PROMOCION regedit_promocion;
  int reg_buffer_num_elem;

  //if (loaded) return;

  reg_buffer_num_elem       = read_regedit_promocion("FIESTA_FUTBOL",&regedit_promocion);

  prom_type_FIESTA_FUTBOL_Amount     = Str2Long(regedit_promocion.reg_value[0],50);;
  prom_type_FIESTA_FUTBOL_Fecha_Ini  = Str2Long(regedit_promocion.reg_value[1],20140603);
  prom_type_FIESTA_FUTBOL_Fecha_Fin  = Str2Long(regedit_promocion.reg_value[2],20140630);
  prom_type_FIESTA_FUTBOL_Cupon_MAX  = Str2Long(regedit_promocion.reg_value[3],20);


  //loaded=1;
}


/*---------------------------------------------------------------------------*/
/*                         is_promocion_FIESTA_FUTBOL                        */
/*---------------------------------------------------------------------------*/

  
int is_promocion_FIESTA_FUTBOL( int * p_vigente )///double monto_venta, long fini, long ffin)
{
  
  double monto_venta = prom_type_FIESTA_FUTBOL_Amount; 
  long fini          = prom_type_FIESTA_FUTBOL_Fecha_Ini; 
  long ffin          = prom_type_FIESTA_FUTBOL_Fecha_Fin;
  double cupon_max   = prom_type_FIESTA_FUTBOL_Cupon_MAX ;


  long num_article_prom =0;
  TProm_art_amount art[3];



  ARTICLE_PROM_DEF_USR list_article[MAX_NUM_ARTICLE_LIST];
  int nsize=0;
  int i=0;
  int marca      =0;
  int num_cupones=0;

///  configuration_promocion_cine();
  if (!isvigente_fechapromocion(fini,ffin)) {
      if (p_vigente!=NULL) *p_vigente=0;
      return 0;
  } else{
      if (p_vigente!=NULL) *p_vigente=1;
  }

  memset(list_article, 0, sizeof(list_article)  );
  memset(art         , 0, sizeof(art)           );

  num_article_prom=get_num_article_promv2(prom_type_FIESTA_FUTBOL /*4*/, list_article);
  for (i=0;i<num_article_prom;i++)
  {
      marca= 1;//((int)list_article[i].art_prom.prom_dvalue);
      art[marca-1].size=1;
      art[marca-1].value+=list_article[i].goods_value;
  }

  /// Los articulos deben ser de al menos 2 marcas 
  /*
  if (art[0].size+ art[1].size +art[2].size<=1)
  {
      return 0;
  }
  */
  /// El monto debe ser mayor o igual que 200 
  if (art[0].value + art[1].value +art[2].value<monto_venta)
  {
      return 0;
  }

  num_cupones= (int)( ((long)art[0].value) / ((long)monto_venta));
  if (num_cupones > cupon_max)
    num_cupones=cupon_max ;
    
  return num_cupones;
} 
/*---------------------------------------------------------------------------*/
/*                         get_num_cupon_FIESTA_FUTBOL                       */
/*---------------------------------------------------------------------------*/

int get_num_cupon_FIESTA_FUTBOL()
{
    int num_promocion=0;
    int vigencia_date=0;

    num_promocion=is_promocion_FIESTA_FUTBOL( &vigencia_date );

    if (num_promocion)
    {
        return num_promocion;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
long prom_type_CUPON_GLOBAL_Fecha_Ini [CUPON_GLOBAL_MAX] ;
long prom_type_CUPON_GLOBAL_Fecha_Fin [CUPON_GLOBAL_MAX] ;
long prom_type_CUPON_GLOBAL_Amount    [CUPON_GLOBAL_MAX];
long prom_type_CUPON_GLOBAL_Cupon_MAX [CUPON_GLOBAL_MAX] ;

long prom_type_CUPON_GLOBAL_Amount_2        [CUPON_GLOBAL_MAX];
long prom_type_CUPON_GLOBAL_Amount_2_factor [CUPON_GLOBAL_MAX];
//++long prom_type_CUPON_GLOBAL_case_logic      [CUPON_GLOBAL_MAX];

/*---------------------------------------------------------------------------*/
/*                         cupon_global_vigente                              */
/*---------------------------------------------------------------------------*/

int cupon_global_vigente(int cupon_index)
{

    long fini          = 0;
    long ffin          = 0;
    
    int cupon_code=0;
    if ((cupon_index>=0 )&& (cupon_index<CUPON_GLOBAL_MAX))
    {
        cupon_code=prom_type_CUPON_GLOBAL[cupon_index];
        if (cupon_code!=0)
        {
            fini = prom_type_CUPON_GLOBAL_Fecha_Ini [cupon_index]; 
            ffin = prom_type_CUPON_GLOBAL_Fecha_Fin [cupon_index];
    
            if (isvigente_fechapromocion(fini,ffin)) 
            {
                return 1;
            }
        }
    }
    //int prom_type_CUPON_GLOBAL [CUPON_GLOBAL_MAX]=
    return 0;
}

/*---------------------------------------------------------------------------*/
/*                         configuration_regedit_CUPON_GLOBAL               */
/*---------------------------------------------------------------------------*/
void configuration_regedit_CUPON_GLOBAL()
{
  static int loaded=0;
  REGEDIT_PROMOCION regedit_promocion;
  int reg_buffer_num_elem;
  int ix;
  char  cupon_global_name[100];

  //if (loaded) return;


    for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
    {

        prom_type_CUPON_GLOBAL_Amount    [ix] = 0;;
        prom_type_CUPON_GLOBAL_Fecha_Ini [ix] = 0;
        prom_type_CUPON_GLOBAL_Fecha_Fin [ix] = 0;
        prom_type_CUPON_GLOBAL_Cupon_MAX [ix] = 0;

        prom_type_CUPON_GLOBAL_Amount_2  [ix] = 0;
        prom_type_CUPON_GLOBAL_Amount_2_factor [ix] = 0;
    }

    // 

    ///* 0000 */  prom_type_UNILEVER // free
    ix=0;
    sprintf(cupon_global_name,"CUPON_GLOBAL%d",ix);
    reg_buffer_num_elem       = read_regedit_promocion(cupon_global_name,&regedit_promocion);

    prom_type_CUPON_GLOBAL_Amount    [ix] = Str2Long(regedit_promocion.reg_value[0],20);        // MONTO DIVISION 1
    
    /* INI 22-01-2015  María E. Argandoña C. */
    prom_type_CUPON_GLOBAL_Fecha_Ini [ix] = Str2Long(regedit_promocion.reg_value[1],20150127);  // FECHA INICIO 
    prom_type_CUPON_GLOBAL_Fecha_Fin [ix] = Str2Long(regedit_promocion.reg_value[2],20150308);  // FECHA FIN
    
    //prom_type_CUPON_GLOBAL_Fecha_Ini [ix] = Str2Long(regedit_promocion.reg_value[1],20140604);  // FECHA INICIO 
    //prom_type_CUPON_GLOBAL_Fecha_Fin [ix] = Str2Long(regedit_promocion.reg_value[2],20140630);  // FECHA FIN
    
    /* FIN 22-01-2015  María E. Argandoña C. */
    
    prom_type_CUPON_GLOBAL_Cupon_MAX [ix] = Str2Long(regedit_promocion.reg_value[3],200000);    // MAXIMA CANTIDAD DE CUPONES A ENTREGAR
    prom_type_CUPON_GLOBAL_Amount_2  [ix] = Str2Long(regedit_promocion.reg_value[4],50);
    prom_type_CUPON_GLOBAL_Amount_2_factor[ix] = Str2Long(regedit_promocion.reg_value[5],3);
    //prom_type_CUPON_GLOBAL_case_logic[ix] = Str2Long(regedit_promocion.reg_value[6], prom_case_logic_AMOUNT); // FIX x monto
    
    ////* 0001 */ ,prom_type_NAJAR    // free
    ix=1; 
    sprintf(cupon_global_name,"CUPON_GLOBAL%d",ix);
    reg_buffer_num_elem       = read_regedit_promocion(cupon_global_name,&regedit_promocion);

    prom_type_CUPON_GLOBAL_Amount    [ix] = Str2Long(regedit_promocion.reg_value[0],3);         // MONTO DIVISION 1     //3 BOTELLAS
    prom_type_CUPON_GLOBAL_Fecha_Ini [ix] = Str2Long(regedit_promocion.reg_value[1],20141020);  // FECHA INICIO 
    prom_type_CUPON_GLOBAL_Fecha_Fin [ix] = Str2Long(regedit_promocion.reg_value[2],20141231);  // FECHA FIN
    prom_type_CUPON_GLOBAL_Cupon_MAX [ix] = Str2Long(regedit_promocion.reg_value[3],200000);    // MAXIMA CANTIDAD DE CUPONES A ENTREGAR
    prom_type_CUPON_GLOBAL_Amount_2  [ix] = Str2Long(regedit_promocion.reg_value[4],100);
    prom_type_CUPON_GLOBAL_Amount_2_factor[ix] = Str2Long(regedit_promocion.reg_value[5],1);
    //++prom_type_CUPON_GLOBAL_case_logic[ix] = Str2Long(regedit_promocion.reg_value[6], prom_case_logic_QTY); // FIX x monto
}

/*---------------------------------------------------------------------------*/
/*                         is_promocion_CUPON_GLOBAL                        */
/*---------------------------------------------------------------------------*/
  
  
int is_promocion_CUPON_GLOBAL( int * p_vigente, int cupon_index )///double monto_venta, long fini, long ffin)
{
  
  double monto_venta_1 = prom_type_CUPON_GLOBAL_Amount    [cupon_index]; 
  long fini            = prom_type_CUPON_GLOBAL_Fecha_Ini [cupon_index]; 
  long ffin            = prom_type_CUPON_GLOBAL_Fecha_Fin [cupon_index];
  double cupon_max     = prom_type_CUPON_GLOBAL_Cupon_MAX [cupon_index];

  double monto_venta_2          = prom_type_CUPON_GLOBAL_Amount_2[cupon_index];
  double monto_venta_2_factor   = prom_type_CUPON_GLOBAL_Amount_2_factor[cupon_index];

  int   case_logic             = prom_case_logic_CUPON_GLOBAL[cupon_index] ;

  

  long num_article_prom =0;
  TProm_art_amount art[3];
  int factor_cupon=0;



  ARTICLE_PROM_DEF_USR list_article[MAX_NUM_ARTICLE_LIST];
  int nsize=0;
  int i=0;
  int marca      =0;
  int num_cupones=0;
  double total_venta_product_prom;

///  configuration_promocion_cine();
  if (!isvigente_fechapromocion(fini,ffin)) {
      if (p_vigente!=NULL) *p_vigente=0;
      return 0;
  } else{
      if (p_vigente!=NULL) *p_vigente=1;
  }

  memset(list_article, 0, sizeof(list_article)  );
  memset(art         , 0, sizeof(art)           );

  num_article_prom=get_num_article_promv2(prom_type_CUPON_GLOBAL[cupon_index] /*4*/, list_article);
  
  for (i=0;i<num_article_prom;i++)
  {
      marca= 1;//((int)list_article[i].art_prom.prom_dvalue);
      art[marca-1].size=1;
      art[marca-1].value+=list_article[i].goods_value;
      art[marca-1].qty  +=list_article[i].qty;
  }

  if ( num_article_prom <= 0 )
  {
      return 0;
  }

  /// Los articulos deben ser de al menos 2 marcas 
  /*
  if (art[0].size+ art[1].size +art[2].size<=1)
  {
      return 0;
  }
  */
  /// El monto debe ser mayor o igual que 200 

  switch(case_logic)
  {
    case prom_case_logic_AMOUNT:
        total_venta_product_prom= art[0].value + art[1].value +art[2].value;
        break;

    case prom_case_logic_QTY:
        total_venta_product_prom= art[0].qty   + art[1].qty   +art[2].qty;

        break;

    default:
        total_venta_product_prom= art[0].value + art[1].value +art[2].value;

        break;
  }

  if (total_venta_product_prom < monto_venta_1)
  {
      return 0;
  }

  /*  INI 22-01-2015  María E. Argandoña C. */
  
  // num_cupones= (int)( ((long)total_venta_product_prom) / ((long)monto_venta_1));

 
  num_cupones= ((int)( ((long)total_venta_product_prom) / ((long)monto_venta_1)) )*2;
  
  //if (cupon_index == prom_type_UNILEVER)
  /*
  {
        if (monto_venta_2>monto_venta_1)
        {
            if (total_venta_product_prom>= monto_venta_2) 
            {
                num_cupones = monto_venta_2_factor *((int)( ((long)total_venta_product_prom) / ((long)monto_venta_2)));
                num_cupones+= (((long)total_venta_product_prom) %((long)monto_venta_2))/((long)monto_venta_1);
            }else{
                num_cupones= (int)( ((long)total_venta_product_prom) / ((long)monto_venta_1));
            }    
        }
  }
  
  */
  
  /*  FIN 22-01-2015  María E. Argandoña C. */

  if (num_cupones > cupon_max)
    num_cupones=cupon_max ;
    
  return num_cupones;
} 

/*---------------------------------------------------------------------------*/
/*                         get_num_cupon_CUPON_GLOBAL                       */
/*---------------------------------------------------------------------------*/

int get_num_cupon_CUPON_GLOBAL(int cupon_index)
{
    int num_promocion=0;
    int vigencia_date=0;

    num_promocion=is_promocion_CUPON_GLOBAL( &vigencia_date ,cupon_index);

    if (num_promocion)
    {
        return num_promocion;
    }
    return 0;
}


int prom_type_CUPON_GLOBAL [CUPON_GLOBAL_MAX]=
{
/* 0000 */  prom_type_UNILEVER // free
/* 0001 */ ,prom_type_NAJAR    // free
/* 0002 */ ,0 // free
/* 0003 */ ,0 // free
/* 0004 */ ,0 // free
/* 0005 */ ,0 // free
};


int prom_case_logic_CUPON_GLOBAL [CUPON_GLOBAL_MAX]=
{
/* 0000 */  prom_case_logic_AMOUNT // free
/* 0001 */ ,prom_case_logic_QTY    // free
/* 0002 */ ,0 // free
/* 0003 */ ,0 // free
/* 0004 */ ,0 // free
/* 0005 */ ,0 // free
};

_TCHAR * cupon_global_xr_TXT[CUPON_GLOBAL_MAX]=
{
/* 0000 */  _T("CUPON UNILEVER :%d") 
/* 0001 */ ,_T("CUPON NAJAR    :%d") //
/* 0002 */ ,_T("XXXXXXXXXXXX: %d") 
/* 0003 */ ,_T("XXXXXXXXXXXX: %d") 
/* 0004 */ ,_T("XXXXXXXXXXXX: %d") 
/* 0005 */ ,_T("XXXXXXXXXXXX: %d") 
};
 _TCHAR *cupon_global_inv_TXT[CUPON_GLOBAL_MAX][3]=
{
    {     
        /* 0000 */  _T("GANO %d CUPON - UNILEVER")
               

        /* 0000 */ ,_T("GANO %d CUPONES - UNILEVER")          
        /* 0000 */ ,_T("") /// mensaje en caso no obtenga cupones
    } ,  
    {     
        /* 0001 */  _T("GANO %d CUPON - PROMO NAJAR")            
        /* 0001 */ ,_T("GANO %d CUPONES - PROMO NAJAR")          
        /* 0001 */ ,_T("") /// mensaje en caso no obtenga cupones     
    } ,  
    {     
        /* 0002 */  _T("xxxxxxxxxxxxxxxxxxxxxxxx")            
        /* 0002 */ ,_T("xxxxxxxxxxxxxxxxxxxxxxxx")          
        /* 0002 */ ,_T("")   /// mensaje en caso no obtenga cupones   
    } ,  
    {     
        /* 0003 */  _T("xxxxxxxxxxxxxxxxxxxxxxxx")            
        /* 0003 */ ,_T("xxxxxxxxxxxxxxxxxxxxxxxx")          
        /* 0003 */ ,_T("")   /// mensaje en caso no obtenga cupones   
    } ,  
    {     
        /* 0004 */  _T("xxxxxxxxxxxxxxxxxxxxxxxx")            
        /* 0004 */ ,_T("xxxxxxxxxxxxxxxxxxxxxxxx")          
        /* 0004 */ ,_T("")   /// mensaje en caso no obtenga cupones
    } ,  
    {     
        /* 0005 */  _T("xxxxxxxxxxxxxxxxxxxxxxxx")            
        /* 0005 */ ,_T("xxxxxxxxxxxxxxxxxxxxxxxx")          
        /* 0005 */ ,_T("")   /// mensaje en caso no obtenga cupones
    }   
};
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


 ////
/*

  1. Solo permitir una NC
  2. Acumular el monto de la NC al medio de pago.
  3. La percepcion , monto total y igv debe ser menor o igual que el monto de la NC
 
 */
 ///
short  vfy_voucher_nc(_TCHAR * data, short key, int  * handled)
{     
  _TCHAR   linvoice_buffer[100];
  long    linvoice_no; 
  int cc;
  VOUCHER_ANTICIPO_DEF  voucher;
  //long                  voucher_no;
  int status=0;
  char voucher_data[100];
  double tot_extra, tot_excl, tot_incl, tot_vat;  /****************************/
  double tot_incl_voucher;
  

  *handled=0;

  if ( _tcslen(data) <= 8 )
  {
     return key;
  }

  *handled=1;

 
/*
 *  We use a voucher prefix to be partly compatible with Argentina, it is fixed to 1.
 */
    strtrim(data,voucher_data);
    {    
      char *pp=voucher_data;
      while ((*pp)&&*pp!=' ')
      {    
            pp++;
      }
      if (*pp==' ') *pp='-';
    }
  //++voucher_no                = _ttol(voucher_data);
/*
  voucher_items.seq_no      = VOUCHER_PREFIX;
  voucher_items.paym_cd     = used_curr_type;
  _tcscpy(voucher_items.id, voucher_data);
*/
  if (!isAnticipo_Valid( voucher_data ))
  {    
     err_invoke(ANTICIPO_NOTFOUND); 
     *data = _T('\0');
     return(UNKNOWN_KEY);

  }else
  {

        ///VALIDAR QUE LOS MONTOS DEL ANTIDICIPO SEAN MENORES O IGUALES QUE EL VALOR DEL TICKET 
        if (IS_GEN_PERCEPTION)
        {
            if ((cur_anticipo.percepcion > TOT_GEN_PERCEPTION  )
                &&fabs( cur_anticipo.percepcion-TOT_GEN_PERCEPTION)>0.01)
            {
                 err_invoke(ANTICIPO_PERCEPCION_ERR); 
                 *data = _T('\0');
                 return(UNKNOWN_KEY);
            }
        }

        tot_excl = tot_ret_double(TOT_GEN_EXCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_EXCL));
        tot_incl = tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL));
        tot_vat  = tot_ret_double(TOT_GEN_VAT)  + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_VAT));

        if ((cur_anticipo.igv> tot_vat )&&fabs(cur_anticipo.igv -tot_vat  )>0.01)
        {
             err_invoke(ANTICIPO_IGV_ERR); 
             *data = _T('\0');
             return(UNKNOWN_KEY);
        }

        if ((cur_anticipo.monto_total > tot_incl  )&&fabs(cur_anticipo.monto_total-tot_incl )>0.01)
        {
             err_invoke(ANTICIPO_TOTAL_ERR); 
             *data = _T('\0');
             return(UNKNOWN_KEY);
        }

        //TOT_GEN_PERCEPTION                  = floor_price(TOT_GEN_PERCEPTION) ;
        //pos_invoice.invoice_percep_amount   = TOT_GEN_PERCEPTION;


        ///VALIDAR QUE EL ANTICIPO-BOLETA SEA TICKET BOLETA Y
        ///            EL ANTICIPO-FACTURA SEA TICKET FACTURA

        


         if (cur_anticipo.tipo_doc==1&&(!is_client_pay_FACTURA())) 
         {
             err_invoke(ANTICIPO_DOC_TYPE_FAC_ERR); 
             *data = _T('\0');
             return(UNKNOWN_KEY);

         }

         if (cur_anticipo.tipo_doc==2&&(is_client_pay_FACTURA())) 
         {
             err_invoke(ANTICIPO_DOC_TYPE_BOL_ERR); 
             *data = _T('\0');
             return(UNKNOWN_KEY);

         }

         if (cur_anticipo.tipo_doc==1) //FACT
         {
             if ( strcmp(cur_anticipo.fisc_no, cust.fisc_no)!=0 ) 
             {
                 err_invoke(ANTICIPO_FIS_NO_ERR); 
                 *data = _T('\0');
                 return(UNKNOWN_KEY);
             }
         }
         if (cur_anticipo.tipo_doc==2) //BOLETA
         {
            if ( strcmp(cur_anticipo.fisc_no, cust.fisc_no)==0 ) 
            {
                // ok
            } 
            else if( IS_GEN_PERCEPTION && 
                     (strcmp(usr_perception_document, cust.fisc_no)==0) ) 
            //if(IS_PERCEPCION()/*&& (percepcion.)*/ )
            {

                ///usr_perception_document
                // ok
            } 
            else
            {
                 err_invoke(ANTICIPO_FIS_NO_ERR); 
                 *data = _T('\0');
                 return(UNKNOWN_KEY);
             }
         }

        get_invoice_no(linvoice_buffer);
        linvoice_no=atol(linvoice_buffer);

        cc=get_voucher_bo_anticipo(cur_anticipo.nro_seq,//.vale_no, 
                              0, //cur_anticipo.vale_type, 
                              //NOT_
                              USED, 
                              pos_system.store_no,
                              cust.cust_no,
                              pos_invoice.invoice_till_no , //till_no
                              0, //selected_invoice_printer +1,  //invoice_type
                              linvoice_no ,                 //invoice_no
                              pos_invoice.invoice_date,     //invoice_date
                              pos_invoice.invoice_sequence_no,
                              &voucher,3 );
        voucher.paym_cd       = used_curr_type;

        if (cc!=SUCCEED)
        {
            err_invoke(VALEPAVO_CONECTION_ERROR);
            *data = _T('\0');
            return(UNKNOWN_KEY);
        }
        if (voucher.status != NOT_USED) 
        {
            if (voucher.status == USED )
            {
              err_invoke(ANTICIPO_USED); 
            }
            else if (voucher.status == 98)
            {  
              err_invoke(ANTICIPO_NOTFOUND); 
            }else if (voucher.status == 99)
            {
               err_invoke(ANTICIPO_BD_ERR); 
            }else
                err_invoke(ANTICIPO_UNKNOWN); 

            *data = _T('\0');
            return(UNKNOWN_KEY);                                 /* simulate NO-key */
        }

        tot_incl_voucher=atof(voucher.amount); 
        if (fabs(tot_incl_voucher - cur_anticipo.monto_total)>0.01)
        {
             err_invoke(ANTICIPO_TOTAL_ERR); 
             *data = _T('\0');
             return(UNKNOWN_KEY);
        }

          /*                                    */
          /* Update cur_valepavo                */
          /*                                    */
                    
         /*           
          cur_valepavo.status=1;
          status=pos_update_rec(VALEPAVO_PROM_TYPE, VALEPAVO_PROM_SIZE, 
              VALEPAVO_PROM_IDX, VALEPAVO_PROM_FNO,(void*)&cur_valepavo);

        */
          /*
          if( status==SUCCEED ) {
            if ((request->paym_cd >= PAYM_WAY_0) && (request->paym_cd < MAX_PAYM_WAYS)) {
              memcpy(&payment[request->paym_cd],request,POS_PAYM_SIZE);
            }
          }
          */
      }

  voucher_items.seq_no      = VOUCHER_PREFIX;
  voucher_items.paym_cd     = voucher.paym_cd;
  ltoa(voucher.seq_no,voucher_items.id,10);    /* voucher_data*/
  //_tcscpy(voucher_items.id,   voucher.seq_no/* voucher_data*/);

  /* Status changed from NOT_USED -> USED on backoffice.           */
  /* Book the amount on the returned paymenttype                   */
  key = book_voucher(voucher.paym_cd, _tcstod(voucher.amount,NULL), 0);
  if (key == UNKNOWN_KEY) 
  {
    if (get_voucher_bo_anticipo(cur_anticipo.nro_seq,//.vale_no, 
                              0, //cur_anticipo.vale_type, 
                              NOT_USED, 
                              pos_system.store_no,
                              cust.cust_no,
                              pos_invoice.invoice_till_no , //till_no
                              0, //selected_invoice_printer +1,  //invoice_type
                              linvoice_no ,                 //invoice_no
                              pos_invoice.invoice_date,     //invoice_date
                              pos_invoice.invoice_sequence_no,
                              &voucher,3 )        != SUCCEED) {
      err_invoke(NO_CONNECTION);
    }
    *data = _T('\0');
    return(NO_KEY);                                   /* simulate NO-key */
  }

  voucher_items.paym_date   = pos_system.run_date;
  voucher_items.paym_amount = _tcstod(voucher.amount, NULL);

  voucher.prefix   = voucher_items.seq_no;
  voucher.paym_cd  = voucher_items.paym_cd;
  voucher.cust_no  = cust.cust_no;
  voucher.store_no = cust.store_no;
  voucher.status   = USED;
  voucher.seq_no   = (long)_tcstol(voucher_items.id, NULL, 10);
  //_tcscpy(voucher.amount, data);

  sll_add(&payment_items, &voucher_items);/* Add to payment item.               */
  sll_add(&voucher_hist, &voucher);       /* Add to hist for possible canceling */
  return(key);

//  sll_add(&payment_items, &voucher_items);/* Add to payment item.            */
//  sll_add(&voucher_anticipo_hist, &voucher);       /* Add to hist for possible cancel */

  //return(key);
}   


 
/*---------------------------------------------------------------------------*/
/*                           is_cust_prom_horeca                             */
/*---------------------------------------------------------------------------*/

int isAnticipo_Valid(_TCHAR *fac_no)
//int isAnticipo_Valid(long anticipo_no, long vale_type)
{
    short status;

    ANTICIPO_PROM_DEF   anticipo;

    memset(&anticipo    ,0, sizeof(ANTICIPO_PROM_DEF));
    memset(&cur_anticipo,0, sizeof(ANTICIPO_PROM_DEF));

    memset (anticipo.fac_no, 0 , sizeof(anticipo.fac_no)); 
    _tcscpy(anticipo.fac_no, AnsiToUnicode(fac_no));


    status = pos_get_rec(ANTICIPO_PROM_TYPE, ANTICIPO_PROM_SIZE, ANTICIPO_PROM_FACTNO_IDX,
                      (void*)&anticipo, (short) keyEQL);


  if((status == SUCCEED)&&(anticipo.ind_uso==0))
  {
    anticipo.tag=1;
    memcpy(&cur_anticipo, &anticipo, sizeof(ANTICIPO_PROM_DEF));
    return(1);
  }
  else {
    cur_anticipo.tag=0;  
    return(0); 
  }
}


int IS_ANTICIPO()
{
    return (cur_anticipo.tag==1);
}
