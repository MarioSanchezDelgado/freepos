/*
 *     Module Name       : ST_CUST.C
 *
 *     Type              : States CustomerMode, Customer_Fee
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
 * 11-Dec-2000 Instead of waiting for crazy articles, initiate an      R.N.B.
 *             asynchronous request
 * --------------------------------------------------------------------------
 * 30-Jan-2001 Added reset multisam to init_invoice                    R.N.B.
 * --------------------------------------------------------------------------
 * 29-Mar-2001 Added OCIA2_DATA.                                       R.N.B.
 * --------------------------------------------------------------------------
 * 21-Sep-2001 Added Pending Invoice functionality.                      M.W.
 * --------------------------------------------------------------------------
 * 09-Apr-2002 Added Customer Fee on Day Pass                            M.W.
 * --------------------------------------------------------------------------
 * 11-Dec-2002 Pending invoice: Print barcode on invoice i.s.o. art_no   M.W.
 * --------------------------------------------------------------------------
 * 20-Jan-2003 Added Backlog corrupt message.                            M.W.
 * --------------------------------------------------------------------------
 * 21-Jan-2003 Bugfix: Don't accept customers if not found on local Pos  M.W.
 * --------------------------------------------------------------------------
 * 03-Feb-2003 If backlog is corrupt, till must be closed. File will be 
 *             renamed in <backlog_name>.BAD.<date>.<time>.              M.W.
 * --------------------------------------------------------------------------
 * 27-Aug-2003 Reset of all total carried forward totals added in
 *             init_invoice().                                         J.D.M.
 * --------------------------------------------------------------------------
 * 16-Dec-2003 Added Donation functionality.                             M.W.
 * --------------------------------------------------------------------------
 * 17-Feb-2004 A Separate 'carried forward' total is used to store           
 *             MultiSam discounts in order to avoid rounding diffs.      P.M.
 * --------------------------------------------------------------------------
 * 12-Mar-2004 Bugfix. reset_multisam_discounts() may also change          
 *             nbr_inv_lines. Function now first in init_invoice().      M.W.
 * --------------------------------------------------------------------------
 * 22-Oct-2004 Added update_multisam_definitions() to init_invoice().  J.D.M.
 * --------------------------------------------------------------------------
 * 02-Aug-2005 Search customer by Fiscal number.                         M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 * 16-Aug-2005 Select customer if searched by fisc_no                    M.W.
 * --------------------------------------------------------------------------
 * 13-Mar-2006 Adapted inc_invoice_no and dec_invoice_no to also handle
 *             the small invoice sequence number.                      J.D.M.
 * --------------------------------------------------------------------------
 * 01-May-2007 Added check_small_invoice_seq()                         J.D.M.
 * --------------------------------------------------------------------------
 * 12-Ago-2011 Added Vale Turkey                                        ACM -
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>                          /* System include files.         */
#include <time.h>
                                            /* Pos (library) include files   */
#include "stnetp24.h"
#include "DllMnp24.h"
#include "comm_tls.h"
#include "intrface.h"
#include "date_tls.h"
#include "misc_tls.h"
#include "stri_tls.h"
                                            /* Toolsset include files.       */
#include "tot_mgr.h"
#include "bp_mgr.h"
#include "prn_mgr.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "tm_mgr.h"
#include "err_mgr.h"
#include "mapkeys.h"
#include "llist.h"
#include "sll_mgr.h"

#include "pos_tm.h"                         /* Application include files.    */
#include "pos_errs.h"
#include "pos_recs.h"
#include "pos_txt.h"
#include "pos_inp.h"
#include "pos_func.h"
#include "pos_keys.h"
#include "pos_vfy.h"
#include "pos_dflt.h"
#include "pos_st.h"
#include "pos_scrl.h"
#include "pos_tot.h"
#include "pos_log.h"
#include "write.h"
#include "pos_bp1.h"
#include "pos_com.h"
#include "st_main.h"
#include "WPos_mn.h"
#include "pos_bp3.h"
#include "pos_msam.h"
#include "condll.h"
#include "pos_edoc.h"

#define CUST_NOT_BLOCKED        empty   /* Empty=not-blocked, else blocked.  */

static void  CustomerMode_VW(void);
static void  CustomerMode_UVW(void);
static short vfy_custno(_TCHAR *, short);
static short cancel_cust(_TCHAR *, short);
/*static */ short start_invoice(_TCHAR *, short);
static short start_document(_TCHAR *data, short key);//v3.6.1 acm -
static short toggle_pos_mode(_TCHAR *, short);
static void  init_cfee(void);

static double fee_to_pay;
static short  fee_expire_1month;

static void   CustomerFee_VW(void);
static void   CustomerFee_UVW(void);
static short  start_invoice_cfee(_TCHAR *, short);
static void   start_pay_cfee(short, double);
static short  no_invoice(_TCHAR *, short);
static short  cust_fee_menu(_TCHAR *, short);
static double determine_fee(short, short);
static long   date2days(long);

static void  view_select_cust_scrn(void);
static short vfy_select_cust(_TCHAR *, short);

static void  Calculadora_VW(void); 
static CONTROL_ELEMENT Calculadora_CTL[9];

static void add_articles_pending_invoice(long);
static void search_by_fisc_no(void);

static short check_cust_blocked(_TCHAR *);

short pending_invoice = FALSE;
short use_fisc_no = FALSE;

SLL     customers_by_fiscno;
short   cust_choice;
short   nbr_of_custs;

static short check_small_invoice_seq();

extern int valeturkey_press_key ;   /* 12-Ago-2011 acm -  flag key press turkey*/
extern int paymentway_bono_pressed; /* 12-Ago-2011 acm -  flag key press payment bono*/

short de_reverse_mode = NO; 
/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE           CALCULATOR                                      */
/* Invoking a calculator                                                     */
/*---------------------------------------------------------------------------*/
static void Calculadora_VW(void)
{
  static short no_cls_states[]={
  ST_CALCULADORA
	,0
  };

  if (ST_CALCULADORA != state_previous_number()) {
    Calculadora_CTL[0].next = state_previous_address();
  }

  tela_calc();

} /* Calculadora_VW() */


static VERIFY_ELEMENT Calculadora_VFY[] =
{
  DOT_KEY,           vfy_calc, 
  ENTER_KEY,         vfy_calc,   
  LINE_UP_KEY,       vfy_calc,
  LINE_DOWN_KEY,     vfy_calc,
  PAGE_UP_KEY,       vfy_calc,
  PAGE_DOWN_KEY,     vfy_calc,
  TOTAL_KEY,         vfy_calc,
  NEW_PAGE_KEY,      (void *)NULL,
  CLEAR_KEY,         (void *)NULL,
  UNKNOWN_KEY,       illegal_fn_key
};

extern INPUT_CONTROLLER Dvr_calc11 =
{
  (INPUT_DISPLAY *)&dsp_vr_calc11,
  KEYBOARD_MASK,
  12,
  12,
  (VERIFY_KEY *)&numeric_punct
};

static PROCESS_ELEMENT Calculadora_PROC[] =
{
  ENTER_KEY,        view_calc_scrn,
  LINE_UP_KEY,      view_calc_scrn,
  LINE_DOWN_KEY,    view_calc_scrn,
  PAGE_UP_KEY,      view_calc_scrn,
  PAGE_DOWN_KEY,    view_calc_scrn,
  TOTAL_KEY,        view_calc_scrn,
  CLEAR_KEY,        inic_calc,
  UNKNOWN_KEY,      (void *)NULL
};


static CONTROL_ELEMENT Calculadora_CTL[] =
{
  NEW_PAGE_KEY,     NULL,          /* filled in view function */
  ENTER_KEY,        &Calculadora_ST,
  LINE_UP_KEY,      &Calculadora_ST,
  LINE_DOWN_KEY,    &Calculadora_ST,
  PAGE_UP_KEY,      &Calculadora_ST,
  PAGE_DOWN_KEY,    &Calculadora_ST,
  CLEAR_KEY,        &Calculadora_ST,
  TOTAL_KEY,        &Calculadora_ST,
  UNKNOWN_KEY,      &Calculadora_ST
};


extern STATE_OBJ Calculadora_ST =
{
  ST_CALCULADORA,
  Calculadora_VW,
  no_DFLT,
  &Dvr_calc11,
  Calculadora_VFY,
  Input_UVW,
  Calculadora_PROC,
  Calculadora_CTL
};

/*---------------------------------------------------------------------------*/
/*                              vfy_calc()                                   */
/* check valid number                                                        */
/*---------------------------------------------------------------------------*/
short vfy_calc(_TCHAR *data, short key)
{
  double numero;

  tot_reset_double(CALC_INP);    
  if (*data == _T('\0')) {                
    if (key == DOT_KEY) {  
      *data=(_TCHAR)0;           
    }
    else {                        
      inic_sinal(key);         
    }
  } 
  else if (_tcslen(data) > 11) {         
    err_invoke(VALUE_TOO_LARGE);
    key=UNKNOWN_KEY;
  } 
  else {    
    numero = _tcstod(data, NULL);
    tot_add_double(CALC_INP, numero);  
  }

  if (key == DOT_KEY) {  
    return(entra_decimal(data, key));
  }
  else {       
    return(key);
  }
} /* vfy_calc() */

/*---------------------------------------------------------------------------*/
/*                             entra_decimal()                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
short entra_decimal(_TCHAR *data, short key)
{
  short key2;
  double numero;
  _TCHAR buffer[20];

  ch_memset(buffer, _T(' '), sizeof(buffer));
  buffer[19] = '\0';
  ftoa((double)floor_price(tot_ret_double(CALC_INP) * 100.0), 13, buffer);
  format_string(&string_preco11, buffer);
  strcpy_no_null(buffer, string_preco11.result);
  scrn_string_out(buffer, 18, 23);
  ch_memset(buffer, _T(' '), sizeof(buffer));
  buffer[19] = '\0';

  SetShowCursor(FALSE);
  SetShowCursor(TRUE);

  do {
    *data = _T('\0'); 
    // Display within while loop due to errors during typing.    
    format_display(&dsp_vr_calc11, empty);
    key2 = inp_get_data(&Dvr_calc11, data);
    if( key2==CLEAR_KEY ) {
       *data = _T('\0'); 
       scrn_string_out(buffer, 18, 23);
       key2 = UNKNOWN_KEY;
    }
    else if( key2 == NEW_PAGE_KEY ) {
          *data = _T('\0'); 
    }
  } while( key2 != NEW_PAGE_KEY    && key2 != ENTER_KEY       &&
           key2 != TOTAL_KEY       && key2 != LINE_UP_KEY     &&
           key2 != LINE_DOWN_KEY   && key2 != PAGE_UP_KEY     &&
           key2 != PAGE_DOWN_KEY   && key2 != UNKNOWN_KEY     &&
           key2 != ENTER_KEY       && key2 != TOTAL_KEY );

  SetShowCursor(FALSE);

  _stprintf(buffer, _T(".%s"),data);
  numero = _tcstod(buffer, NULL);
  tot_add_double(CALC_INP, numero);  
  return(key2);       
} /* entra_decimal() */

/*---------------------------------------------------------------------------*/
/*                       view_calc_scrn()                                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/
short view_calc_scrn(_TCHAR *data, short key)
{
  double tot_calc;

  tot_calc = (double)tot_ret_double(CALC_TOT);
  tot_reset_double(CALC_TOT);

  if (*data != _T('\0')) {                
    switch(sinal[2]) {
      case _T('+') : tot_add_double(CALC_TOT, (double)
                     tot_calc + tot_ret_double(CALC_INP));
                     break;
      case _T('_') : tot_add_double(CALC_TOT, (double)
                     tot_calc - tot_ret_double(CALC_INP));
                     break;
      case _T('*') : tot_add_double(CALC_TOT, (double)
                     tot_calc * tot_ret_double(CALC_INP));
                     break;
      case _T('/') : if (tot_ret_double(CALC_INP)) { 
                       tot_add_double(CALC_TOT, (double)
                       tot_calc / tot_ret_double(CALC_INP));
                     } 
                     else {
                       err_invoke(ZERO_NOT_LEGAL_ERROR);
                       tot_add_double(CALC_TOT, (double)tot_calc);
                       return(key);
                     }
                     break;
      default:  
                tot_add_double(CALC_TOT,
                (double)tot_ret_double(CALC_INP));
                break;
    }

    if (tot_ret_double(CALC_TOT) > 99999999999.9999) {
      err_invoke(VALUE_TOO_LARGE);
      tot_reset_double(CALC_TOT);
      tot_add_double(CALC_TOT, (double)tot_calc);
    }
  } 
  else { 
    if (tot_calc != 0.0) {
      tot_add_double(CALC_TOT, (double)tot_calc);
    }
  }
  inic_sinal(key);

  tela_calc(); 
  tot_reset_double(CALC_INP);    
  *data=_T('\0');
  return(key);
} /* view_calc_scrn() */

/*---------------------------------------------------------------------------*/
/*                             inic_calc()                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
short inic_calc(_TCHAR *data, short key)
{
  if ( (key != CLEAR_KEY) ) {
    if ( (key != TOTAL_KEY) ) {
      tot_reset_double(CALC_TOT);
    }
    inic_sinal(TOTAL_KEY); 
  }
    tela_calc();
    return(key);
} /* inic_calc() */

/*---------------------------------------------------------------------------*/
/*                             tela_calc()                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void tela_calc(void)
{
  _TCHAR buffer[30];

  cls();
  scrn_select_window(PAYMENT_1_WINDOW);
  scrn_clear_window(PAYMENT_1_WINDOW);
  scrn_string_out(menu_TXT[40], 0, 6);              // write CALCULATOR
  scrn_select_window(PAYMENT_2_WINDOW);
  scrn_clear_window(PAYMENT_2_WINDOW);
  scrn_string_out((_TCHAR *) scrn_inv_TXT[7], 2, 7); // TOTAL
  _stprintf(buffer, _T("%15.4lf"),(double)tot_ret_double(CALC_TOT));
  scrn_string_out(buffer, 2, 19);
  scrn_string_out(sinal, 5, 25);
} /* tela_calc() */

/*---------------------------------------------------------------------------*/
/*                             inic_sinal()                                  */
/*                                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void inic_sinal(short key)
{
  switch(key) {
    case ENTER_KEY:
    case LINE_UP_KEY:   _tcscpy(sinal, _T("  +"));      // 3 characters
                        break;
    case LINE_DOWN_KEY: _tcscpy(sinal, _T("  _"));
                        break;
    case PAGE_UP_KEY:   _tcscpy(sinal, _T("  *"));
                        break;
    case PAGE_DOWN_KEY: _tcscpy(sinal, _T("  /"));
                        break;
    default:            _tcscpy(sinal, _T("   "));
                        break;
  }
} /* inic_sinal() */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   CUSTOMER MODE                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Cashier has logged in, start reading the customer number or start a       */
/* cashier-break. Hitting the "OPERATOR" key will invoke COperatorMode.      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CustomerMode_VW(void)
{
  static short cls_states[]={
       ST_START_BREAK
      ,ST_END_BREAK
      ,ST_COPERATOR_MODE
      ,ST_START_FLOAT
      ,ST_INVOICING
      ,ST_DO_TOTAL
      ,ST_SHOW_CHANGE
      ,ST_CUSTOMER_FEE
      ,ST_CALCULADORA
      ,ST_SELECT_CUST
      ,0
  };

  /*                                                                       */
  /* Setup invoice screen.                                                 */
  /*                                                                       */

  if( called_by_state(cls_states)==SUCCEED ) {
    cls();
  }

  InvoicingHead_VW();
  InvoicingDescr_VW();
  InvoicingScrl_VW();
  view_total(OPER_SCRN);
  InvoicingSubt_VW();
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[1],0,16);

  closed_till=NO;
  cdsp_clear();
  cdsp_write_string(cdsp_TXT[2],0,0);             /* OPEN                  */
  cdsp_write_string(cdsp_TXT[3],1,0);             /* NEXT CUSTOMER PLEASE  */
 
  status_of_pos = SHIFT_OPEN;
  use_fisc_no = FALSE;

  return;
} /* CustomerMode_VW */


static void CustomerMode_UVW(void)
{
  scrn_clear_window(FISC_NO_WINDOW);
  scrn_select_window(INV_HEADER_WINDOW);
  SetShowCursor(FALSE);
} /* CustomerMode_UVW */


static VERIFY_ELEMENT CustomerMode_VFY[] =
{
  ENTER_KEY,          vfy_custno,
  OCIA1_DATA,         vfy_custno,
  OCIA2_DATA,         vfy_custno,
  CLEAR_KEY,          vfy_clear_key,
  CREDIT_KEY,         toggle_pos_mode,
  PRINTER_UP_KEY,     proc_new_line,
  KEYLOCK_LOCK,       (void *)NULL,
  OPERATOR_KEY,       (void *)NULL,            /* Approval in state-engine. */
  NEW_PAGE_KEY,       (void *)NULL,            /* calculator */
  OPEN_DRAWER_KEY,    open_and_close_drawer,   /* Approval in state-engine. */
  UNKNOWN_KEY,        illegal_fn_key
};


extern INPUT_CONTROLLER Dcustno14KOKl14n =
 {
  (INPUT_DISPLAY *)&dsp_artno14,
  KEYBOARD_MASK | OCIA1_MASK | OCIA2_MASK | KEYLOCK_L_MASK,
  15, 15,
  (VERIFY_KEY *)&numeric_punct
};


static PROCESS_ELEMENT CustomerMode_PROC[] =
{
  //CUSTNO_OK,     start_invoice, //v3.6.1 acm  -
  CUSTNO_OK,       start_document, //v3.6.1 acm  -
  NO_KEY,          cancel_cust,
  SELECT_CUST_KEY, (void *)NULL,
  UNKNOWN_KEY,     (void *)NULL
};


static CONTROL_ELEMENT CustomerMode_CTL[] =
{
  
  //CUSTNO_OK,       &Invoicing_ST, //v3.6.1 acm -
  CUSTNO_OK,       &SelectDocument_ST, //v3.6.1 acm -

  CUST_EXPIRED,    &CustomerFee_ST,
  KEYLOCK_LOCK,    &StartBreak_ST,
  OPERATOR_KEY,    &COperatorMode_ST,
  NEW_PAGE_KEY,    &Calculadora_ST, /* calculator */
  NO_KEY,          &CustomerMode_ST,
  SELECT_CUST_KEY, &SelectCustomer_ST,
  UNKNOWN_KEY,     &CustomerMode_ST
};


extern STATE_OBJ CustomerMode_ST =
{
  ST_CUSTOMER_MODE,
  CustomerMode_VW,
  no_DFLT,
  &Dcustno14KOKl14n,
  CustomerMode_VFY,
  CustomerMode_UVW,
  CustomerMode_PROC,
  CustomerMode_CTL
};

/*---------------------------------------------------------------------------*/
/*                              cancel_cust                                  */
/*---------------------------------------------------------------------------*/
static short cancel_cust(_TCHAR *data, short key)
{
  init_cust_rec(&cust);
  *data=_T('\0');
  scrn_clear_window(FISC_NO_WINDOW);
  use_fisc_no = FALSE;

  return(key);
} /* cancel_cust */


/*---------------------------------------------------------------------------*/
/*                          toggle_pos_mode                                  */
/*---------------------------------------------------------------------------*/
static short toggle_pos_mode(_TCHAR *data, short key)
{
  /*                                                                         */
  /* Before entering a customer number, the pos-mode for one invoice         */
  /* can be toggled by pressing the CREDIT key.                              */
  /* The variable toggled is next_invoice_mode, so the variable              */
  /* invoice_mode stays intact. The invoice_mode will get the value of       */
  /* next_invoice_mode at the time the next customer is accepted.            */
  /* Above is needed to handle the 'reprint invoice' function for the        */
  /* current customer.                                                       */
  /*                                                                         */

  if (next_invoice_mode == -1) {
    next_invoice_mode = pos_system.current_mode;
  }
  if (next_invoice_mode == SALES) {
    next_invoice_mode = RETURN;
    display_prompt(scrn_inv_TXT[16], ERROR_WINDOW_ROW1);
	reverse_invoice_active = YES;
	de_reverse_mode = YES;
	printf_log("kapum1[%d]", de_reverse_mode);
  }
  else {
    next_invoice_mode = SALES;
    display_prompt(scrn_inv_TXT[15], ERROR_WINDOW_ROW1);
	reverse_invoice_active = NO;
  }

#ifndef NO_VIEW_POS_STATE
  view_pos_state();
#endif
  return(key);
} /* toggle_pos_mode */

/*---------------------------------------------------------------------------*/
/*                              vfy_custno                                   */
/*---------------------------------------------------------------------------*/
static short vfy_custno(_TCHAR *data, short key)
{
  unsigned short      current_scrn=scrn_get_current_window();
  short               get_status;
  POS_PEND_CUST_DEF   pend_cust;

  if(check_small_invoice_seq()==FAIL) {
    return UNKNOWN_KEY;
  } 

  /*                                                                       */
  /* Verify Customer Number.                                               */
  /*                                                                       */

  scrn_select_window(INV_HEADER_WINDOW);

  /*                                                                       */
  /*                           v-obligated-v                               */
  /* Customer number:  [99] [9]99999 {0-9|.}9                              */
  /*           [store-no] cust-no {check-dig} seq-no                       */
  /*                                                                       */

  init_cust_rec(&cust);
  sll_init(&customers_by_fiscno, sizeof(POS_CUST_DEF));

  if (get_backlog_corrupt_status()==TRUE) {
    err_invoke(BACKLOG_CORRUPT);
    return(NO_KEY);
  }

  if (*data == _T('\0')) {
    search_by_fisc_no();
    return(UNKNOWN_KEY);
  }
  else if(_tcslen(data) < 7 && get_use_fisc_no()==FALSE) {
    err_invoke(CUST_ILLEGAL_PASSPORT_NUMBER);
    return(NO_KEY);
  }
  else {
    /*                                                                     */
    /* Customer is legal, get it from Customer Reception                   */
    /*                                                                     */
    /*   get_cust_data(custno, cust_rec)                                   */
    /*                                                                     */

    init_invoice(empty,0);
    init_cfee();
    if (next_invoice_mode != -1) {   /* There was a toggle on cust. level. */
      invoice_mode      = next_invoice_mode;
      invoice_line_mode = invoice_mode;
      next_invoice_mode = -1;
    }

    get_status = get_cust_data(data, &pend_cust);
    memcpy(&cust, &pend_cust.cust, POS_CUST_SIZE);

    /* Never accept customer not found by fisc_no!                         */
    /* Cust_no and store_no are unknown so nothing can be done             */
    if (get_status == FISC_NO_NOT_FOUND) {
      err_invoke(CUST_FISC_NO_ERROR);
      return(NO_KEY);
    }

    /* More than one customer is found by search on fisc_no!               */
    /* Have to go to other state to select correct customer                */
    if (get_status == FISC_NO_MANY_FOUND) {
      return(SELECT_CUST_KEY);
    }

    if (get_status == CUST_NUM_WRONG) {
      err_invoke(CUST_ILLEGAL_PASSPORT_NUMBER);
      return(NO_KEY);
    }

  /**Verifica si se puede generar Factura**/
  /*if(strlen(cust.fisc_no) != 11 ) {*/
	   /*err_invoke(CUST_NO_FACTURA);*/
 /* }*/

  /**Fin Verifica si se puede generar Factura**/


    else if (get_status == SUCCEED) {
      if ((key=check_cust_blocked(cust.cust_bl_cd))!=CUSTNO_OK) {
        return(NO_KEY);
      }
    }
    else if (get_status == PEND_INV_FOUND) {
      pending_invoice = TRUE;
      key=CUSTNO_OK;
    }
    else if (get_status == CUST_CHKD_WRONG) { /* checkdigit/seqno incorr.  */
      if (err_invoke(CUST_PASSPORT_INVALID) == SUCCEED) {
        key=CUSTNO_OK;
      }
      else {
        return(NO_KEY);
      }
    }               /* Customer not known on Custumor Reception or on WinPOS */
    else if (get_status == FAIL || get_status == 101) { /* INOT_ERR in Ctree */
      if (pos_system.store_no == cust.store_no) {
        err_invoke(CUST_DELETE_PASSPORT);
        return(NO_KEY);
      }
      else {
        key=CUSTNO_OK;
      }
    }
    else if (get_status == INVALID_CUST_ON_POS) {
      if (err_invoke(INVALID_GENV_CD) == SUCCEED) {
        key=CUSTNO_OK;
      }
      else {
        return(NO_KEY);
      }
    }
    else {
      key=CUSTNO_OK;               /* TIME_OUT, allways ok.                */
    }
  }

  if (key == CUSTNO_OK) {

    valeturkey_press_key    =0; /* 12-Ago-2011 acm -+++ vfy_custno inicializa el flag de vale turkey key press */
    paymentway_bono_pressed =0; /* 12-Ago-2011 acm - */

    pos_invoice.invoice_time = get_current_time((_TCHAR *)NULL);
             /* Check expiring date only in case customer-info is received */

    /* Get crazy-article information for this customer */
    sll_init(&crazy_art_hist, sizeof(CRAZY_ART_DEF));
    get_cart_bo(cust.store_no, cust.cust_no, NULL, -1, -1);

    if (pending_invoice) { /* copy fee from pending_record */
      start_pay_cfee(pend_cust.fee_status, pend_cust.fee_amount);
    }
    else if (get_status == SUCCEED) {
      /* Check now if the 'exp_date' - 'current date' < 1 month */
      if ((date2days(cust.exp_date) - date2days(pos_system.run_date) <= 31L) ||
          (cust.card_type_no == genvar.daypass_card_type_no)) {
        /* passp expired or within a month  */
        key = CUST_EXPIRED;
        fee_to_pay = determine_fee(cust.cardholder_no, (short) cust.card_type_no);
        if (fee_to_pay == 0.0) { /* Payment of fee automatic */
          key = CUSTNO_OK;
          start_pay_cfee(FEE_STAT_PAY, fee_to_pay);
        }
      }
    }
  }

  InvoicingHead_VW();
  scrn_select_window(current_scrn);

  return(key);
} /* vfy_custno */

/*---------------------------------------------------------------------------*/
/*                            search_by_fisc_no                              */
/*---------------------------------------------------------------------------*/
static void search_by_fisc_no(void)
{
  unsigned short save_window;

  save_window=scrn_get_current_window();

  scrn_select_window(FISC_NO_WINDOW);
  scrn_clear_window(FISC_NO_WINDOW);

  if (use_fisc_no == FALSE) {
    scrn_string_out(scrn_inv_TXT[33],2,8);
    use_fisc_no = TRUE;
  }
  else {
    use_fisc_no = FALSE;
  }
  scrn_select_window(save_window);

} /* search_by_fisc_no */

/*---------------------------------------------------------------------------*/
/*                              init_invoice                                 */
/*---------------------------------------------------------------------------*/
short init_invoice(_TCHAR *data, short key)
{
  short         i;
  int ix;

  /*                                                                       */
  /* Start of a new invoice.                                               */
  /*                                                                       */
  /* The TM, all paymentway-totals, subtotals etc. are cleared.            */
  /* Also the pos_invoice is initialised.                                  */
  /*                                                                       */

                                                  /* reset MultiSAM totals */
  reset_multisam_discounts();
  update_multisam_definitions();

  /* Reset pos_invoice.                                                    */
  pos_invoice.invoice_till_no    = pos_system.till_no;
  pos_invoice.invoice_no         = pos_system.invoice_no;
  pos_invoice.invoice_cashier_no = c_shft.cashier;
  pos_invoice.invoice_date       = pos_system.run_date;
  pos_invoice.invoice_time       = get_current_time( (_TCHAR *)NULL );
  pos_invoice.invoice_donation   = 0.0;
  pos_invoice.invoice_sequence_no= pos_system.small_inv_seq;

  pos_invoice.invoice_cupon      = 0;  /* AC2012-003 acm - */
  pos_invoice.invoice_gift       = 0;  /* AC2012-003 acm - */
  pos_invoice.invoice_rounded    = 0;  // v3.4.7 acm - 
  pos_invoice.invoice_cupon_cine = 0;  /* v3.5 acm - */
  pos_invoice.invoice_vale_pavo  = 0;       /* v3.5 acm - */
  pos_invoice.invoice_feria_escolar = 0;    /* v3.6.0 acm - */
  

  pos_invoice.invoice_percep_amount   = 0;    /* v3.6.1 acm - */
  
  pos_invoice.invoice_percep_custdoc[0]  = 0;    /* v3.6.1 acm - */
  pos_invoice.invoice_percep_custname[0] = 0;    /* v3.6.1 acm - */

  pos_invoice.invoice_fiesta_futbol = 0;    /* v3.6.0 acm - */

  pos_invoice.invoice_serie[0] = 0;		// mlsd FE
  pos_invoice.invoice_correlative = 0;	// mlsd FE
  
  for (ix=0; ix<CUPON_GLOBAL_MAX; ix++)
  {
        pos_invoice.invoice_cupon_global[ix] = 0;
  }
  /* Reset some globals.                                                   */
  last_item      = TM_ROOT;
  corr_item      = TM_ROOT;
  display_item   = TM_ROOT;

  nbr_inv_lines  =0;
  nbr_log_lines  =0;
  nbr_void_lines =0;
  invoice_mode      = pos_system.current_mode;
  invoice_line_mode = pos_system.current_mode;

  for(i=DEV_PRINTER1; i < NUMBER_OF_PRINTERS; i++) {
    if (pos_printer_info[i].printersize == PRINTER_SIZE_NORMAL) {
      selected_printer = i;
    }
  }

//    /* Removing scrolling lines from scrl-mgr (does not clear window!)     */
//    scrl_delete(INV_ART_LIST_WINDOW);
//    scrn_clear_window(INV_ART_LIST_WINDOW);

#ifndef PRN_IMM
  clean_invoice();                       /* remove the sort array          */
#endif

  reset_voucher_items();        /* Reset Payment Items and Voucher history */

  /* Reset TM articles                                                     */
  tm_reset_struct(TM_ITEM_NAME);
  reset_paymentways();

  for (i=0; i<=9; i++) {
    tot_reset_double((short)(TOT_EXCL_0+i));
    tot_reset_double((short)(TOT_INCL_0+i));
    tot_reset_double((short)(TOT_VAT_0+i));
    tot_reset_double((short)(SUB_TOT_EXCL_0+i));
    tot_reset_double((short)(SUB_TOT_INCL_0+i));
    tot_reset_double((short)(SUB_TOT_VAT_0+i));
  }

  tot_reset_double(TOT_GEN_EXCL);
  tot_reset_double(TOT_GEN_VAT);
  tot_reset_double(TOT_GEN_INCL);
  tot_reset_double(SUB_TOT_GEN_INCL);
  tot_reset_double(TOT_PAID);
  tot_reset_double(TOT_PACKS);
  tot_reset_double(TOT_SUB_PACKS);

  tot_reset_double(TOT_CARR_FORWD_EXCL);
  tot_reset_double(TOT_CARR_FORWD_VAT);
  tot_reset_double(TOT_CARR_FORWD_INCL);
  tot_reset_double(TOT_CARR_FMSAM_EXCL);
  tot_reset_double(TOT_CARR_FMSAM_VAT);
  tot_reset_double(TOT_CARR_FMSAM_INCL); 
  for (i=0; i<=9; i++) {
    tot_reset_double((short)(TOT_CARR_FORWD_EXCL_0+i));
    tot_reset_double((short)(TOT_CARR_FORWD_INCL_0+i));
    tot_reset_double((short)(TOT_CARR_FORWD_VAT_0+i));
    tot_reset_double((short)(TOT_CARR_FMSAM_EXCL_0+i));
    tot_reset_double((short)(TOT_CARR_FMSAM_INCL_0+i));
    tot_reset_double((short)(TOT_CARR_FMSAM_VAT_0+i));
  }

  tot_reset_double(TOT_CHANGE);
  tot_reset_double(TOT_CHANGE_CD);

  tot_reset_double(TOT_CREDIT_AMNT);
  tot_reset_double(TOT_CREDIT_VAT_AMNT);
  tot_reset_double(TOT_CREDIT_VAT_NO);
  tot_reset_double(TOT_CREDIT_PAYM_CD);

  
  valeturkey_press_key   =0; /* 12-Ago-2011 acm -+++ init_invoice inicializa el flag de vale turkey key press*/
  paymentway_bono_pressed=0; /* 12-Ago-2011 acm - */


  key=init_invoice_user(data, key);

  return(key);
} /* init_invoice */

//v3.6.1 acm -{
static short start_document(_TCHAR *data, short key)
{
    return(key);
}
//v3.6.1 acm -}
/*---------------------------------------------------------------------------*/
/*                              start_invoice                                */
/*---------------------------------------------------------------------------*/
/*static */ short start_invoice(_TCHAR *data, short key)
{
  _TCHAR buffer[15];
  long   cust_key;

  /*                                                                       */
  /* Start of a new invoice.                                               */
  /* The TDM, all paymentway-totals, subtotals etc. are already cleared by */
  /* init_invoice() called in shift_on_and_init_invoice().                 */
  /*                                                                       */
  /* Print customer data, create invoice number.                           */
  /*                                                                       */
  /* First check if there is a Pending Invoice for the customer            */

  get_invoice_no(buffer);         /* Retrieve invoice number and           */
  scrn_string_out(buffer,0,57);   /* display it.                           */

  if (pending_invoice) {
    cust_key = (cust.store_no * 1000000L) + cust.cust_no;
    add_articles_pending_invoice(cust_key);
    pending_invoice = FALSE;
    delete_pending_invoice(cust_key);
  }

#ifdef PRN_IMM
  if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
    bp_now(BP_SMALL_INVOICE, BP_INV_INIT, 0);         /* Reset page number.  */
    bp_now(BP_SMALL_INVOICE, BP_SMALL_HEADER, 0);
  }
  else {
    bp_now(BP_INVOICE, BP_INV_INIT, 0);         /* Reset page number.        */
    bp_now(BP_INVOICE, BP_REVERSE_SKIP, 0);
    bp_now(BP_INVOICE, BP_CUSTOMER_LINES, 0);   /* Print cust-data           */
  }

  if (pos_invoice.invoice_fee_status != FEE_STAT_NONE) {
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_CUST_FEE, 0); /* Print cust-fee-data */
    }
    else {
      bp_now(BP_INVOICE, BP_CUST_FEE_LINE, 0);  /* Print cust-fee-data       */
    }
  }
#endif

  return(key);
} /* start_invoice */

/*---------------------------------------------------------------------------*/
/*                          get_invoice_no                                   */
/*---------------------------------------------------------------------------*/
short get_invoice_no(_TCHAR *data)
{
  _TCHAR buffer[13];
  _TCHAR buffer1[13];

  /*                                                                       */
  /* Assemble the invoice number using pos_invoice.                        */
  /*                                                                       */

  _tcscpy(buffer, _T("00000000000"));
  _tcscpy(buffer1, _T("00000000000"));

  get_jul_date(pos_invoice.invoice_date, buffer+3);
  _itot(pos_invoice.invoice_no, (buffer1+4), DECIMAL);

  _tcscat(buffer, buffer1+_tcslen(buffer1)-3);
  _tcscpy(data, buffer+_tcslen(buffer)-6);

  return(SUCCEED);
} /* get_invoice_no */


/*---------------------------------------------------------------------------*/
/*                          get_system_invoice_no                            */
/*---------------------------------------------------------------------------*/
short get_system_invoice_no(_TCHAR *data)
{
  _TCHAR buffer[13];
  _TCHAR buffer1[13];

  /*                                                                       */
  /* Assemble the invoice number using pos_system.                         */
  /*                                                                       */

  _tcscpy(buffer, _T("00000000000"));
  _tcscpy(buffer1, _T("00000000000"));

  get_jul_date(pos_system.run_date, buffer+3);
  _itot(pos_system.invoice_no, (buffer1+4), DECIMAL);

  _tcscat(buffer, buffer1+_tcslen(buffer1)-3);
  _tcscpy(data, buffer+_tcslen(buffer)-6);

  return(SUCCEED);
} /* get_system_invoice_no */


/*---------------------------------------------------------------------------*/
/*                         check_small_invoice_seq                           */
/*---------------------------------------------------------------------------*/
static short check_small_invoice_seq()
{
  if (  pos_system.small_inv_seq < till.start_sequence_no
      ||pos_system.small_inv_seq > till.end_sequence_no) {
      err_invoke(PRN_SMALL_SEQ_FINISHED);
    return FAIL;
  }

  return SUCCEED;
} /* check_small_invoice_seq */


/*---------------------------------------------------------------------------*/
/*                              inc_invoice_no                               */
/*---------------------------------------------------------------------------*/
void inc_invoice_no(void)
{
  /* Update invoice-no.                                                    */
  if (++pos_system.invoice_no == 1000) {
    pos_system.invoice_no=0;
  }
  if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
    if(++pos_system.small_inv_seq > 999999999) {
      pos_system.small_inv_seq = 1;
    }
  }

  if (train_mode == CASH_NORMAL) {            /* don't save in train.mode. */
    pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
  }

  pos_invoice.invoice_no   = pos_system.invoice_no;
  pos_invoice.invoice_date = pos_system.run_date;
  pos_invoice.invoice_sequence_no = pos_system.small_inv_seq;
} /* inc_invoice_no */


/*---------------------------------------------------------------------------*/
/*                              dec_invoice_no                               */
/*---------------------------------------------------------------------------*/
void dec_invoice_no(void)
{
  /* Decrement invoice-no.                                                   */
  if (--pos_system.invoice_no == -1) {
    pos_system.invoice_no = 999;
  }
  if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
    if(--pos_system.small_inv_seq < 1) {
      pos_system.small_inv_seq = 999999999;
    }
  }

  if (train_mode == CASH_NORMAL) {            /* don't save in train.mode.   */
    pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
  }

  pos_invoice.invoice_no   = pos_system.invoice_no;
  pos_invoice.invoice_date = pos_system.run_date;
  pos_invoice.invoice_sequence_no = pos_system.small_inv_seq;
} /* dec_invoice_no */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   CUSTOMER FEE                                            */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Customer membershipfee has expired.                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void CustomerFee_VW(void)
{
  _TCHAR   cdspbuf[80], cdspbuf2[80];
  _TCHAR   datebuf[40];
  CURR_DEF hcurr;
  short    cdsp_txtno;

  static short cls_states[]={
      ST_CUSTOMER_MODE,
      ST_SELECT_CUST,
      0
  };
  /* Setup invoice screen.                                                 */
  /*                                                                       */
  if (called_by_state(cls_states)==SUCCEED) {

    cls();
    scrn_clear_window(OPERATOR_WINDOW);
    cdsp_clear();

    /* Check now if the 'expired date' -  'current date' < 0*/
    if (date2days(cust.exp_date) - date2days(pos_system.run_date) >= 0L) {
       /* passp not expired but within a month */
       fee_expire_1month = TRUE;
       cdsp_txtno = 11;
    }
    else {
      fee_expire_1month = FALSE;
      cdsp_txtno = 12;
    }

    /* CDSP: the customer membership exp_date                           */
    if (cust.card_type_no != genvar.daypass_card_type_no) {
      prn_fmt_date(cust.exp_date, genvar.date_format, mon_names, datebuf);
      _stprintf(cdspbuf, cdsp_TXT[cdsp_txtno], datebuf);
      cdsp_write_string(cdspbuf,0,0);             /* MEMBERSHIP EXPIRES   */
    }
    else {
      cdsp_write_string(cdsp_TXT[14],0,0);        /* DAYPASS:             */
      fee_expire_1month = FALSE;
    }

    cust_fee_menu(empty, UNKNOWN_KEY);

    /* CDSP: Display the customer membership-fee  */
    if (get_stand_rate(1, &hcurr) == FAIL) {
      if(get_first_stand_rate(&hcurr) == FAIL) {
        ;
      }
    }
    ftoa_price(fee_to_pay * 100, 11, cdspbuf2);
    format_string(&string_price9_2, cdspbuf2);

    _stprintf(cdspbuf2, _T("%2s%s"), hcurr.currency_cd, string_price9_2.result);
    _stprintf(cdspbuf, cdsp_TXT[13], cdspbuf2);
    cdsp_write_string(cdspbuf,1,CDSP_RIGHT_JUSTIFY); /* PAYMENT FEE DUE        */
  }
} /* CustomerFee_VW */


static void CustomerFee_UVW(void)
{
  return;
} /* CustomerMode_UVW */

static PROCESS_ELEMENT CustomerFee_PROC[] =
{
  1,                    start_invoice_cfee,
  2,                    no_invoice,
  3,                    start_invoice_cfee,
#ifdef CFEE_OVERULE_ONE_YEAR
  4,                    start_invoice_cfee,
#endif
  5,                    start_invoice_cfee,
  6,                    start_invoice,
  KEYLOCK_NORMAL,       proc_clr_scrn,
  UNKNOWN_KEY,          (void *)NULL
};

static VERIFY_ELEMENT CustomerFee_VFY[] =
{
  ENTER_KEY,            cust_fee_menu,
  CLEAR_KEY,            vfy_clear_key,
  KEYLOCK_NORMAL,       (void *)NULL,
  KEYLOCK_LOCK,         (void *)NULL,
  UNKNOWN_KEY,          illegal_fn_key
};


static INPUT_CONTROLLER Dcfeemnu1K3n=
{
  (INPUT_DISPLAY *)&cfee_menu1,
  KEYBOARD_MASK,
  2, 2,
  (VERIFY_KEY *)&numeric
};


static CONTROL_ELEMENT CustomerFee_CTL[] =
{
  1,   &Invoicing_ST,
  2,   &CustomerMode_ST,
  3,   &Invoicing_ST,
#ifdef CFEE_OVERULE_ONE_YEAR
  4,   &Invoicing_ST,
#endif
  5,   &Invoicing_ST,
  6,   &Invoicing_ST,
  UNKNOWN_KEY,  &CustomerFee_ST
};


extern STATE_OBJ CustomerFee_ST =
{
  ST_CUSTOMER_FEE,
  CustomerFee_VW,
  no_DFLT,
  &Dcfeemnu1K3n,
  CustomerFee_VFY,
  (void *) NULL,
  CustomerFee_PROC,
  CustomerFee_CTL
};


/*---------------------------------------------------------------------------*/
/*                              determine_fee                                */
/*---------------------------------------------------------------------------*/
static double determine_fee(short card_holder_no, short card_type_no)
{
  short status;
  CFEE_DEF cfeerec;

  /* Determine with card_type_no and cardholder_no the customer fee */

  cfeerec.cfee_key = (card_type_no * 100) + card_holder_no;
  status = pos_get_rec(CFEE_TYPE, POS_CFEE_SIZE, CUST_FEE_IDX, (void*)&cfeerec,
                         (short)keyLTE);
  if (status != SUCCEED) {
    return (0.0);
  }

  if ((cfeerec.nbr_card_from <= card_holder_no      &&
       card_holder_no        <= cfeerec.nbr_card_to &&
       card_type_no          == cfeerec.card_type_no) ||
      (card_type_no == genvar.daypass_card_type_no &&
       card_type_no == cfeerec.card_type_no)) {
    if (invoice_mode == RETURN) {
      return(-cfeerec.fee_amount);
    }
    else {
      return(cfeerec.fee_amount);
    }
  }
  else {
    return(0.0);
  }
} /* determine_fee */

/*---------------------------------------------------------------------------*/
/*                         custfee_menu                                      */
/*---------------------------------------------------------------------------*/
static short cust_fee_menu(_TCHAR *data, short key)
{
  short   max_choice;
  short   menu_txt;
  short   menu_strings[5];        /* Refer to index of menu_TXT[x].        */
  _TCHAR  datebuf[40];
  _TCHAR  dspbuf[80];
  _TCHAR  dspbuf2[80];
  CURR_DEF hcurr;

  /*                                                                       */
  /* View the customerfee menu, activate an option if key is               */
  /* within menu-range.                                                    */
  /*                                                                       */

  if(fee_expire_1month) {
    max_choice = 2;
    menu_txt   = 29;
    menu_strings[0] = 32;
    menu_strings[1] = 33;
    menu_strings[2] = 0;
  }
  else {
    max_choice = 2;
    menu_strings[2] = 0;
    menu_txt   = 30;

    if (invoice_mode == RETURN) {
      menu_strings[0] = 35;
      menu_strings[1] = 36;
    }
    else {
      menu_strings[0] = 32;
      menu_strings[1] = 34;
    }
  }

  if (key == UNKNOWN_KEY) {
    /* Display the menu header.                                            */
    scrn_select_window(OPERATOR_WINDOW);
    scrn_string_out(menu_TXT[37], 1, 18);             /* CUSTOMER_FEE MENU */

    /* Display the customer store_no, cust_no and name or display DAY PASS */
    if (cust.card_type_no!=genvar.daypass_card_type_no) {
      _stprintf(dspbuf , menu_TXT[38], cust.cust_no);
      _stprintf(dspbuf2, _T(" %2d"), cust.store_no);
      _tcscat(dspbuf, dspbuf2);
      scrn_string_out(dspbuf, 3, 18);
    }
    else {
      scrn_string_out(menu_TXT[41], 3, 18);
    }

    /* Display the customer membership exp_date if no DAY PASS             */
    if (cust.card_type_no!=genvar.daypass_card_type_no) {
      prn_fmt_date(cust.exp_date, genvar.date_format, mon_names, datebuf);
      _stprintf(dspbuf, menu_TXT[menu_txt], datebuf);
      scrn_string_out(dspbuf, 4, 18);
    }

    /* Display the customer membership-fee and number of cardholders       */
    if (get_stand_rate(1, &hcurr) == FAIL) {
      if(get_first_stand_rate(&hcurr) == FAIL) {
        ;
      }
    }
    ftoa_price(fee_to_pay * 100, 11, dspbuf2);
    format_string(&string_price9_2, dspbuf2);
    _stprintf(dspbuf2, _T("%2s %s"), hcurr.currency_cd, string_price9_2.result);
    
    if (cust.card_type_no!=genvar.daypass_card_type_no) {
      _stprintf(dspbuf, menu_TXT[31], dspbuf2, cust.cardholder_no);
    }
    else {
      _stprintf(dspbuf, menu_TXT[42], dspbuf2);
    }
    scrn_string_out(dspbuf, 5, 18);

    /* Display menu */
    view_menu(8, 18, menu_strings, empty);
    scrn_string_out(menu_TXT[10], 15, 18);
    return(key);
  }
  else {                       /* A choice is made, check for legal value. */
    if (vfy_range(data, 1, max_choice) != SUCCEED) {
      err_invoke(INVALID_MENU_OPTION);
      *data=_T('\0');
      return(UNKNOWN_KEY);
    }
    else {
      view_menu(8, 18, menu_strings, data);
      key = _ttoi(data);
      switch (key) {
        case 1:
          if (invoice_mode == RETURN) {
            *data=_T('\0');
            key = 6;
          }
          break;
        case 2:
          if (fee_expire_1month) {
            *data=_T('\0');
            key = 5;
          }
          if (invoice_mode == RETURN) {
            if (err_invoke(CFEE_RETURN) != SUCCEED) {
              *data=_T('\0');
              view_menu(8, 18, menu_strings, data);    /* clear highlite */
              key = UNKNOWN_KEY;
            }
            else {
              key = 1;
            }
          }
          break;
        case 3:
          if (err_invoke(CFEE_OVERRULE_ONCE) != SUCCEED) {
            *data=_T('\0');
            view_menu(8, 18, menu_strings, data);    /* clear highlite */
            key = UNKNOWN_KEY;
          }
          break;
#ifdef CFEE_OVERULE_ONE_YEAR
        case 4:
          if (err_invoke(CFEE_OVERRULE_ONE_YEAR) != SUCCEED) {
            *data=_T('\0');
            view_menu(8, 18, menu_strings, data);    /* clear highlite */
            key = UNKNOWN_KEY;
          }
          break;
#endif
        default:
          break;
      }
      return(key);
    }
  }
} /* cust_fee_menu */

/*---------------------------------------------------------------------------*/
/*                         start_invoice_cfee                                */
/*---------------------------------------------------------------------------*/
static short start_invoice_cfee(_TCHAR *data, short key)
{
  scrn_clear_window(OPERATOR_WINDOW);
  switch (key) {
    case 1:
      start_pay_cfee(FEE_STAT_PAY, fee_to_pay);             /* Pay fee */
      break;
    case 3:
      start_pay_cfee(FEE_STAT_CANCEL, fee_to_pay);    /* Overrule once */
      break;
#ifdef CFEE_OVERULE_ONE_YEAR                          /* overrule whole year */
    case 4:
      start_pay_cfee(FEE_STAT_CANCEL_YEAR, fee_to_pay);
      break;
#endif
    default:
      break;
  }
  start_invoice(data, key);
  view_total( OPER_SCRN | CUST_SCRN );

  return(key);
} /* start_invoice_cfee */

/*---------------------------------------------------------------------------*/
/*                         start_pay_cfee                                    */
/*---------------------------------------------------------------------------*/
static void start_pay_cfee(short status, double pay_fee)
{
  short i;

  /* Adapt header of invoice */
  pos_invoice.invoice_fee_status = status;

  /* Add pay_fee to totals in case no cancelling is done */
  if (status == FEE_STAT_PAY) {

    pos_invoice.invoice_fee_amount = pay_fee;
    tot_add_double(TOT_FEE_AMOUNT,   pay_fee);
    tot_add_double(TOT_GEN_EXCL,     pay_fee);

    if (genvar.price_incl_vat==INCLUSIVE) {

      /* Total amount inclusive vat per vat-code.                              */
      tot_add_double((short)(TOT_INCL_0 + genvar.cfee_vat_no), 
                             floor_price(calc_incl_vat(pay_fee, genvar.cfee_vat_no)));
      tot_add_double((short)(TOT_GEN_INCL), floor_price(calc_incl_vat(pay_fee, genvar.cfee_vat_no)));

      /* Total amount exclusive vat per vat-code.                              */
      tot_reset_double((short)(TOT_EXCL_0 + genvar.cfee_vat_no));
      tot_add_double((short)(TOT_EXCL_0 + genvar.cfee_vat_no), floor_price(calc_excl_vat(
                          tot_ret_double((short)(TOT_INCL_0 + genvar.cfee_vat_no)),genvar.cfee_vat_no)));

      tot_reset_double(TOT_GEN_EXCL);
      for ( i = 0 ; i < 10 ; i++ ) {
        tot_add_double(TOT_GEN_EXCL,tot_ret_double((short)(TOT_EXCL_0 + i)));
      }
    }
    else {                                                        /* EXCLUSIVE */
      /* Total amount exclusive vat per vat-code.                              */
      tot_add_double((short)(TOT_EXCL_0 + genvar.cfee_vat_no), pay_fee);
      tot_add_double(TOT_GEN_EXCL, pay_fee);

      /* Total amount inclusive vat per vat-code.                              */
      tot_reset_double((short)(TOT_INCL_0 + genvar.cfee_vat_no));
      tot_add_double((short)(TOT_INCL_0 + genvar.cfee_vat_no), floor_price(calc_incl_vat(
                        tot_ret_double((short)(TOT_EXCL_0 + genvar.cfee_vat_no)),genvar.cfee_vat_no)));

      tot_reset_double(TOT_GEN_INCL);
      for ( i = 0 ; i < 10 ; i++ ) {
        tot_add_double(TOT_GEN_INCL,tot_ret_double((short)(TOT_INCL_0 + i)));
      }
    }

  }
  return;
} /* start_pay_cfee */

/*---------------------------------------------------------------------------*/
/*                         init_cfee                                         */
/*---------------------------------------------------------------------------*/
static void init_cfee(void)
{
  pos_invoice.invoice_fee_status = FEE_STAT_NONE;
  pos_invoice.invoice_fee_amount = 0.0;
  tot_reset_double(TOT_FEE_AMOUNT);
  fee_to_pay  = 0.0;
} /* init_cfee */

/*---------------------------------------------------------------------------*/
/*                         no_invoice                                        */
/*---------------------------------------------------------------------------*/
static short no_invoice(_TCHAR *data, short key)
{
  init_cust_rec(&cust);
  return(key);
} /* no_invoice */

/*---------------------------------------------------------------------------*/
/*                         date2days                                         */
/* Converts date-format in days  (not accurate!)                             */
/*---------------------------------------------------------------------------*/
static long date2days(long l_date)
{
  long h_date;

  h_date = (l_date / 10000L) * 365L;
  h_date += (long) ((((l_date / 100L) % 100L) -1) * 30L);
  h_date += (long) (l_date % 100L);
  return(h_date);
} /* date2days */

/*---------------------------------------------------------------------------*/
/*                   add_articles_pending_invoice                            */
/*---------------------------------------------------------------------------*/
static void add_articles_pending_invoice(long cust_key)
{
  POS_PEND_INVL_DEF   pend_invl_rec;

  _TCHAR  disc_amount[20];
  _TCHAR  amount[20], art_no[20];
  _TCHAR  weight[40];
  _TCHAR  qty[40];
  short   status,     art_type;
  short   seq_no=0;


  cust_key = (cust.store_no * 1000000L) + cust.cust_no;

  pend_invl_rec.cust_key = cust_key;
  pend_invl_rec.seq_no = ++seq_no;

  status = pos_get_rec(PEND_TYPE, POS_PEND_INVL_SIZE, PEND_INVL_IDX,
                       (void*)&pend_invl_rec, (short)keyEQL);

  if (status == SUCCEED) {
    do {
      
      memset(&c_item, 0, sizeof(TM_ITEM_GROUP));

      c_item.arti.base.qty    = pend_invl_rec.qty;
      _tcscpy(c_item.arti.base.bar_cd, pend_invl_rec.barcode);
      _stprintf(art_no, _T("%ld"), pend_invl_rec.art_no);
      _stprintf(amount, _T("%ld"), (long)pend_invl_rec.amount);

      if (pend_invl_rec.disc_amount != 0) { /* DISCOUNT FIRST */

        _stprintf(disc_amount, _T("%ld"), (long)pend_invl_rec.disc_amount);
        art_type = discnt_vfy_and_start_artno(art_no, ENTER_KEY);

        if (art_type == PRICE_ART) { /* FILL IN THE PRICE IN CASE OF PRICE_ART */
          c_item.arti.base.goods_value = pend_invl_rec.amount;
          c_item.arti.base.price       = pend_invl_rec.amount * pend_invl_rec.qty;
        }else if (art_type == WWEIGHT_ART) 
        {
          //c_item.arti.base.goods_value = pend_invl_rec.amount;
          c_item.arti.base.price       = pend_invl_rec.pend_price; //fix recuperar  // acm7 -
        
        }

        vfy_discount_amount(disc_amount, ENTER_KEY);
      }
      else {
        art_type = vfy_and_start_artno(art_no, ENTER_KEY);
      }

      switch(art_type) {

        case PRICE_ART:
          _stprintf(amount, _T("%ld"), (long)(pend_invl_rec.amount / pend_invl_rec.qty));
          vfy_art_price(amount, ENTER_KEY);
          break;

        case PWEIGHT_ART:
          vfy_weight_price(amount, ENTER_KEY);
          break;

        case WWEIGHT_ART:
          //++vfy_weight(amount, ENTER_KEY);
          _stprintf(qty, _T("%ld"), (long)(pend_invl_rec.qty*1000)); //fix recuperar
          vfy_weight(qty, ENTER_KEY);  //vfy_weight(amount, ENTER_KEY); // fix PERCEPTION
          break;

        default:
          break;
      }

      if (art_type != ARTICLE_NOT_OK) {
        accept_active_item(art_no, ARTICLE_OK); /* Add article to invoice  */
      }

      memset(&pend_invl_rec, 0, POS_PEND_INVL_SIZE);
      pend_invl_rec.cust_key = cust_key;
      pend_invl_rec.seq_no = ++seq_no;

      status = pos_get_rec(PEND_TYPE, POS_PEND_INVL_SIZE, PEND_INVL_IDX,
                           (void*)&pend_invl_rec, (short)keyEQL);

    } while (status == SUCCEED && pend_invl_rec.cust_key == cust_key);
  }
} /* add_articles_pending_invoice */

/*---------------------------------------------------------------------------*/
/*                            get_use_fisc_no                                */
/*---------------------------------------------------------------------------*/
short get_use_fisc_no()
{
  return use_fisc_no;
} /* get_use_fisc_no */

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   SELECT CUSTOMER                                         */
/*---------------------------------------------------------------------------*/

static void SelectCustomer_VW(void)
{
  view_select_cust_scrn();
  view_lan_state(FORCED_UPDATE);      /* update ON- OFFLINE indicator.    */

} /* SelectCustomer_VW */

static void SelectCustomer_UVW(void)
{
} /* SelectCustomer_UVW */

static VERIFY_ELEMENT SelectCustomer_VFY[] =
{
  ENTER_KEY,            vfy_select_cust,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT SelectCustomer_PROC[] =
{
  CUSTNO_OK,            start_document, //start_invoice, //v3.6.1 acm -
  NO_KEY,               cancel_cust,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT SelectCustomer_CTL[] =
{
  CUST_EXPIRED,    &CustomerFee_ST,
  CUSTNO_OK,      /* &Invoicing_ST, */ &SelectDocument_ST, //v3.6.1 acm - 
  NO_KEY,          &CustomerMode_ST,
  UNKNOWN_KEY,     &SelectCustomer_ST
};

extern INPUT_CONTROLLER Dselectcust1n =
 {
  (INPUT_DISPLAY *)&dsp_select_cust,
  KEYBOARD_MASK,
  1, 2,
  (VERIFY_KEY *)&numeric
};

extern STATE_OBJ SelectCustomer_ST =
{
  ST_SELECT_CUST,
  SelectCustomer_VW,
  no_DFLT,
  &Dselectcust1n,
  SelectCustomer_VFY,
  SelectCustomer_UVW,
  SelectCustomer_PROC,
  SelectCustomer_CTL
};

/*---------------------------------------------------------------------------*/
/*                       view_select_cust_scrn                               */
/*---------------------------------------------------------------------------*/
static void view_select_cust_scrn()
{
  _TCHAR           buffer[80];
  POS_CUST_DEF     cust_rec;


  scrn_clear_window(SELECT_CUST_WINDOW);
  scrn_select_window(SELECT_CUST_WINDOW);

  scrn_string_out(scrn_inv_TXT[36], 1, 3);

  nbr_of_custs = 0;
  while (sll_read(&customers_by_fiscno, nbr_of_custs, &cust_rec) == SUCCEED) {

    nbr_of_custs++;
    _stprintf(buffer, scrn_inv_TXT[37], nbr_of_custs, cust_rec.store_no, cust_rec.cust_no, 
                                     cust_rec.name, cust_rec.fisc_no);
    scrn_string_out(buffer, (short)(nbr_of_custs+2), 3);
  }

  scrn_string_out(input_TXT[51], 14, 22); /* SELECT CUSTOMER FROM LIST       */
} /* view_select_cust_scrn */


/*---------------------------------------------------------------------------*/
/*                          vfy_select_cust                                  */
/*---------------------------------------------------------------------------*/
static short vfy_select_cust(_TCHAR *data, short key)
{
  POS_PEND_CUST_DEF   pend_cust;

  if (!*data) {
    err_invoke(INVALID_KEY_ERROR);
    return UNKNOWN_KEY;
  }

  cust_choice = _ttoi(data)-1;

  if (cust_choice<0 || cust_choice>=nbr_of_custs) {
    err_invoke(INVALID_KEY_ERROR);
    return UNKNOWN_KEY;
  }

  sll_read(&customers_by_fiscno, cust_choice, &cust);

  if ((key=check_cust_blocked(cust.cust_bl_cd))!=CUSTNO_OK) {
    return(NO_KEY);
  }

  pos_invoice.invoice_time = get_current_time((_TCHAR *)NULL);
           /* Check expiring date only in case customer-info is received */

  /* Get crazy-article information for this customer */
  sll_init(&crazy_art_hist, sizeof(CRAZY_ART_DEF));
  get_cart_bo(cust.store_no, cust.cust_no, NULL, -1, -1);

  init_cust_rec(&pend_cust.cust);
  pend_cust.cust.cust_key = cust.cust_key;
  if (check_pending_invoice(&pend_cust)==TRUE) {
    start_pay_cfee(pend_cust.fee_status, pend_cust.fee_amount);
  }
  else {
    /* Check now if the 'exp_date' - 'current date' < 1 month */
    if ((date2days(cust.exp_date) - date2days(pos_system.run_date) <= 31L) ||
        (cust.card_type_no == genvar.daypass_card_type_no)) {
      /* passp expired or within a month  */
      key = CUST_EXPIRED;
      fee_to_pay = determine_fee(cust.cardholder_no, (short) cust.card_type_no);
      if (fee_to_pay == 0.0) { /* Payment of fee automatic */
        key = CUSTNO_OK;
        start_pay_cfee(FEE_STAT_PAY, fee_to_pay);
      }
    }
  }
  return(key);
} /* vfy_select_cust */


/*---------------------------------------------------------------------------*/
/*                          check_cust_blocked                               */
/*---------------------------------------------------------------------------*/
short check_cust_blocked(_TCHAR* cust_blocked)
{
  if (_tcscmp(cust_blocked, CUST_NOT_BLOCKED) != 0) {
    error_extra_msg=cust_blocked;
    if (err_invoke(CUST_BLOCKED) == SUCCEED) {
      return(CUSTNO_OK);
    }
    else {
      return(NO_KEY);
    }
  }
  else {                      /* Customer not blocked.  */
    return(CUSTNO_OK);
  }
} /* check_cust_blocked */


/*---------------------------------------------------------------------------*/
/*                          check_pending_invoice                            */
/*---------------------------------------------------------------------------*/
short check_pending_invoice(POS_PEND_CUST_DEF* pend_cust_rec)
{
  _TCHAR   error_message[30];
  short    status;

  pending_invoice = FALSE;

  status = pos_get_rec(PEND_TYPE, POS_PEND_CUST_SIZE, PEND_CUST_IDX,
                       (void*)pend_cust_rec, (short)keyEQL);

  if (status == SUCCEED) {
    _stprintf(error_message, _T("%02d:%02d"), pend_cust_rec->invoice_time/100, pend_cust_rec->invoice_time%100);
    error_extra_msg=error_message;
    if (err_invoke(GET_PENDING_INVOICE) == SUCCEED) {
      pending_invoice = TRUE;
    } 
  }  
  return pending_invoice;

} /* check_pending_invoice */
