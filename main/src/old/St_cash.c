/*
 *     Module Name       : ST_CASH.C
 *
 *     Type              : States CashierMode, CashPinCd, StartFloat
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
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 03-May-2002 Connection termination when till is not defined in the
 *             netnodes                                                J.D.M.
 * --------------------------------------------------------------------------
 * 16-Dec-2003 Added Donation functionality.                             M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "OLETools.h"
#include "ConDLL.H"
                                            /* Pos (library) include files   */
#include "comm_tls.h"
#include "date_tls.h"
#include "misc_tls.h"
#include "stri_tls.h"
                                            /* Toolsset include files.       */
#include "intrface.h"
#include "inp_mgr.h"
#include "state.h"
#include "tm_mgr.h"
#include "scrn_mgr.h"
#include "err_mgr.h"
#include "mapkeys.h"
#include "tot_mgr.h"

#include "pos_tm.h"                         /* Application include files.    */
#include "pos_recs.h"
#include "WPos_mn.h"
#include "pos_errs.h"
#include "pos_txt.h"
#include "pos_inp.h"
#include "pos_func.h"
#include "pos_keys.h"
#include "pos_vfy.h"
#include "pos_dflt.h"
#include "pos_st.h"
#include "pos_tot.h"
#include "pos_com.h"
#include "st_main.h"

static short vfy_cashno(_TCHAR *, short);
static void CashPinCd_VW(void);
static void CashPinCd_UVW(void);
static void StartFloat_VW(void);
static void StartFloat_UVW(void);
static short vfy_start_float(_TCHAR *, short);
static short start_shift(_TCHAR *, short);
static short cancel_shift(_TCHAR *, short);
static short shift_on_and_init_invoice(_TCHAR *, short);
static short setup_shift_on(SHIFT_ON_DEF *);
static short setup_shift_off(SHIFT_OFF_DEF *);
static short setup_xread(SHIFT_TDM_DEF *);
static short no_start_float(_TCHAR *, short);

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES  CASHIER MODE                                            */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* The N-key is inserted, lets logon the cashier.                            */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CashierMode_VW(void)
{
  static short no_cls_states[]={
       ST_CASHIER_MODE
      ,ST_CASH_PIN_CD
      ,ST_START_FLOAT
      ,0
  };

  /*                                                                       */
  /* Display till-no, sales/return-mode, start-of-day/new-shift            */
  /* and lan-state.                                                        */
  /*                                                                       */

  if (called_by_state(no_cls_states) == FAIL) {
    PosStandBy_VW();
  }

  scrn_select_window(CASHIER_LOGON_WINDOW);

  scrn_string_out(scrn_inv_TXT[19],7,24);     /* CASHIER ID:       */
  format_display(&dsp_cash_nm30, empty);

  scrn_string_out(scrn_inv_TXT[20],8,24);     /* PIN CODE:         */
  format_display_passwrd(&dsp_pincd4, empty);

  scrn_string_out(scrn_inv_TXT[21],10,24);    /* FLOAT             */
  format_display(&dsp_start_float13, empty);
//  view_lan_state(FORCED_UPDATE);          /* update ON- OFFLINE indicator.   */
} /* CashierMode_VW */

static VERIFY_ELEMENT CashierMode_VFY[] =
{
  ENTER_KEY,          vfy_cashno,
  KEYLOCK_SUPERVISOR, vfy_emul_keylock,
  KEYLOCK_EXCEPTION,  vfy_emul_keylock,
  KEYLOCK_LOCK,       (void *)NULL,
  CLEAR_KEY,          vfy_clear_key,
  OPEN_DRAWER_KEY,    open_and_close_drawer,
  UNKNOWN_KEY,        illegal_fn_key
};


extern INPUT_CONTROLLER Dcashid3K3n =
{
  (INPUT_DISPLAY *)&dsp_cash_id3,
  KEYBOARD_MASK |
  KEYLOCK_S_MASK | KEYLOCK_L_MASK | KEYLOCK_X_MASK,
  4, 4,
  (VERIFY_KEY *)&numeric
};


static PROCESS_ELEMENT CashierMode_PROC[] =
{
  ENTER_KEY,       start_shift,
  SUPERVISOR_OK,   start_shift,      /* Logon after S-key approval allowed. */
  UNKNOWN_KEY,     (void *)NULL
};


static CONTROL_ELEMENT CashierMode_CTL[] =
{
  ENTER_KEY,          &CashPinCd_ST,                        /* Cashno OK    */
  SUPERVISOR_OK,      &StartFloat_ST,
  KEYLOCK_SUPERVISOR, &SoMode_ST,
  KEYLOCK_LOCK,       &PosStandBy_ST,
  KEYLOCK_EXCEPTION,  &EdpMode_ST,
  EMUL_LOCK_EXCPT,    &EdpMenu_ST,        /* skip the pincd state */
  EMUL_LOCK_SUPER,    &SupervisorMode_ST, /* skip the pincd state */
  UNKNOWN_KEY,        &CashierMode_ST
};


extern STATE_OBJ CashierMode_ST =
{                         
  ST_CASHIER_MODE,
  CashierMode_VW,
  no_DFLT,
  &Dcashid3K3n,
  CashierMode_VFY,
  (void *)NULL,
  CashierMode_PROC,
  CashierMode_CTL
};


/*---------------------------------------------------------------------------*/
/*                               vfy_cashno                                  */
/*---------------------------------------------------------------------------*/
static short vfy_cashno(_TCHAR *data, short key)
{
  short cashno,
        status = FAIL,
        direct_logoff = 0,
        s_key_inserted = NO;
  _TCHAR buffer[15];


  if (get_bo_status() == CFGERR) {               /* no new shift possible */
    err_invoke(NET_CFG_NO_SHIFT);
    *data=_T('\0');
    return(UNKNOWN_KEY);
  }

  send_logoff_cashier = 0;

  /*                                                                       */
  /* Retrieve cashier-data from back-office.                               */
  /*                                                                       */
  if (!STR_ZERO(data)) {
    cashno = (short)_ttoi(data);
    if (get_cash_bo(cashno, &cash) == SUCCEED) {

      /* get_cash_bo() returns:                                            */
      /*  FAIL:    Unknown cashier no (S-key approval possible)            */
      /*  SUCCEED: Known cashier no,                                       */
      /*           reg_status == CASH_ALLOWED    :                         */
      /*                           cashier is allowed, send logoff at      */
      /*                           end of shift.                           */
      /*                         CASH_NOT_ALLOWED:                         */
      /*                           cashier not allowed, S-key also not     */
      /*                           allowed, send logoff                    */
      /*                         CASH_ALREADY_ON :                         */
      /*                           cashier already logged on, with         */
      /*                           S-key allowed, (don't) send a logoff.   */
      /*                                                                   */
      /*  29-dec-1992 Now in case of CASH_ALREADY_ON send(!) a logoff to   */
      /*              get that nasty 'Working' on BO out.                  */
      /*                                                                   */
      if (cash.reg_status == CASH_ALLOWED) {
        status = SUCCEED;
        send_logoff_cashier = 1;
      }
      else {
        if( cash.reg_status == CASH_NOT_ALLOWED ) {
          /* In this case no logon via S-key, the network is in            */
          /* the air so activate cashier via backoffice screen.            */
          err_invoke(CASHIER_NOT_ALLOWED);
          direct_logoff = 1;
        }
        else { /* CASH_ALREADY_ON */
          status=err_invoke(CASHIER_ALREADY_LOGON);
          if (status == SUCCEED) {
            s_key_inserted = YES;
            send_logoff_cashier = 1;
          }
          else {
            direct_logoff = 1;
          }
        }
      }

      if (cash.cur_date != pos_system.run_date) {
        err_invoke(ILLEGAL_SYSTEM_DATE);
        ftoa((double)cash.cur_date, 10, buffer);
        proc_set_date(buffer, 0);
      }
    }
    else {            /* No answer from BO, allowed with S-key approval.   */
      memset(&cash, 0, POS_CASH_SIZE);
      cash.empl_no = cashno;
      status = err_invoke(UNKNOWN_CASHIERNO);
      if (status == SUCCEED) {
        s_key_inserted = YES;
        send_logoff_cashier = 1;
      }
      else {
        direct_logoff = 1;
      }
    }

  }
  else {                                             /* Empty not legal.  */
    err_invoke(ZERO_NOT_LEGAL_ERROR);
  }

  if (status == FAIL) {
    key=UNKNOWN_KEY;
    *data=_T('\0');
  }
  else {
    if (s_key_inserted == YES) {
      key=SUPERVISOR_OK;
    }
  }

  if (direct_logoff) {
    logoff_cash_bo(cashno);
  }

  if (key != UNKNOWN_KEY) {
    format_display(&dsp_cash_nm30,cash.empl_name);
  }

  return(key);
} /* vfy_cashno */



/*---------------------------------------------------------------------------*/
/*                              start_shift                                  */
/*---------------------------------------------------------------------------*/
static short start_shift(_TCHAR *data, short key)
{
  short  shift_no, i;
  _TCHAR buffer[15];

  /*                                                                       */
  /* Cashier is logged on, initialise shift.                               */
  /*                                                                       */

                          /* Initialize shift_no with next shift number.   */
  if (TM_BOF_EOF!=tm_last(TM_SHFT_NAME, (void*)&c_shft)) {
    shift_no = c_shft.shift_no+1;
  }
  else {
    shift_no = 1;
  }

  if (status_of_pos==START_OF_DAY) {
    memset(&c_day, 0, sizeof(SHIFT_TDM_DEF));
  }

  memset(&c_shft, 0, sizeof(TM_SHFT_GROUP));
  c_shft.shift_no=shift_no;
  c_shft.cashier =cash.empl_no;
  c_shft.till_no =pos_system.till_no;

  /* Get invoice number from pos_system,           */
  /* because pos_invoice is not yet initialised.   */
  get_system_invoice_no(buffer);
  c_shft.invoice_on =_ttol(buffer);
  c_shft.invoice_off= c_shft.invoice_on;

  /* Rest of the elements where set to zero by the memset() function.      */
  /* Only the wallets must be initialised with a non-zero value.           */

  for (i=0; i<3; i++) {
    c_shft.wallet_no[i]=WALLET_NOT_USED;
  }

  return(key);
} /* start_shift */


/*---------------------------------------------------------------------------*/
/*                              close_shift                                  */
/*---------------------------------------------------------------------------*/
short close_shift(_TCHAR *data, short key)
{
  SHIFT_OFF_DEF shift_off;
  SHIFT_TDM_DEF xread;
  short i;

  /*                                                                       */
  /* Cashier has logged out, close shift.                                  */
  /*                                                                       */

  if (c_shft.shift_no!=0) {
    if (c_shft.time_off==9999) {  /* Current unclosed shift, process it.   */
      c_shft.time_off=get_current_time((_TCHAR *)NULL);
      c_shft.end_float=0.0;
      for(i=0; i<10; i++) {
        c_shft.end_float+=c_shft.paym_amnt[i];
      }
      c_shft.end_float+=c_shft.start_float;
      recover_shift_updt();
      tm_upda(TM_SHFT_NAME, (void*)&c_shft);

      if (send_logoff_cashier) {             /* Unlock this cashier on BO. */
        logoff_cash_bo(c_shft.cashier);
        send_logoff_cashier = 0;
      }

      setup_shift_off(&shift_off);          /* put SHIFT_OFF on the LAN.   */
      setup_xread(&xread); 
      if (train_mode == CASH_NORMAL)  {      /* Don't send in training mode */
        pos_put_rec(SHFT_TYPE, POS_SHFF_SIZE, SHIFT_OFF_FNO, (void*)&shift_off);
        pos_put_rec(SHFT_TYPE, POS_RECO_SIZE, RECOVER_FNO, (void*)&xread);
      }
    }
  }
  memset(&c_shft, 0, sizeof(TM_SHFT_NAME));

  return(key);
} /* close_shift */


/*---------------------------------------------------------------------------*/
/*                              cancel_shift                                 */
/*---------------------------------------------------------------------------*/
short cancel_shift(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Cashier cancels log in, clear current shift and unlock this cashier   */
  /* on BO.                                                                */
  /*                                                                       */

  if (send_logoff_cashier && c_shft.cashier!=0) {
    logoff_cash_bo(c_shft.cashier);
    send_logoff_cashier = 0;
  }
  memset(&c_shft, 0, sizeof(TM_SHFT_NAME));

  return(key);
} /* cancel_shift */



/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES  CASH PIN CODE                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Cashier has logged on, the cashier-record has been received from BO and   */
/* it's ok to log on.                                                        */
/*  - Retrieve pincode from keyboard and check this with the one generated.  */
/*  - If the pincode is legal, step to state 11 StartFloat.                  */
/*  - After three illegal pincodes the StandBy mode is entered, and only     */
/*    the S-key can be used.                                                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CashPinCd_VW(void)
{
  scrn_select_window(CASHIER_LOGON_WINDOW);
} /* CashPinCd_VW */


static void CashPinCd_UVW(void)
{
  scrn_select_window(CASHIER_LOGON_WINDOW);
  SetShowCursor(FALSE);
} /* CashPinCd_UVW */


static VERIFY_ELEMENT CashPinCd_VFY[] =
{
  ENTER_KEY,       vfy_cash_pincd,
  CLEAR_KEY,       vfy_clear_key,          /* Returns UNKNOWN_KEY           */
  NO_KEY,          cancel_shift,           /* Jump to previous field        */
  KEYLOCK_LOCK,    cancel_shift,
  OPEN_DRAWER_KEY, open_and_close_drawer,
  UNKNOWN_KEY,     illegal_fn_key
};


extern INPUT_CONTROLLER Dpincd4K4n  =
{
  (INPUT_DISPLAY *)&dsp_pincd4,
  KEYBOARD_MASK,
  5, 5,
  (VERIFY_KEY *)&numeric
};


static PROCESS_ELEMENT CashPinCd_PROC[] =
{
  /* Normally remove_shift is called in the _VFY part. This is not          */
  /* possible because PINCODE_NOK is returned by a _VFY function.           */
  PINCODE_NOK,     cancel_shift,
  UNKNOWN_KEY,     (void *)NULL
};

static CONTROL_ELEMENT CashPinCd_CTL[] =
{
  ENTER_KEY,       &StartFloat_ST,
  PINCODE_NOK,     &PosStandBy_ST,
  NO_KEY,          &CashierMode_ST,
  KEYLOCK_LOCK,    &PosStandBy_ST,
  UNKNOWN_KEY,     &CashPinCd_ST
};

extern STATE_OBJ CashPinCd_ST =
{                         
  ST_CASH_PIN_CD,
  CashPinCd_VW,
  no_DFLT,
  &Dpincd4K4n,
  CashPinCd_VFY,
  CashPinCd_UVW,
  CashPinCd_PROC,
  CashPinCd_CTL
};


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES  START FLOAT                                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Pincode is ok. Cashier has logged on.                                     */
/*  - Retrieve start-float.                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void StartFloat_VW(void)
{
  scrn_select_window(CASHIER_LOGON_WINDOW);
  assign_price = 0;
  /* The value of this variable is used in price_DFLT. Only if this         */
  /* value is 0 the input field is initialized.                             */

  rs_open_cash_drawer(CASH_DRAWER1);
} /* StartFloat_VW */


static VERIFY_ELEMENT StartFloat_VFY[] =
{
  ENTER_KEY,       vfy_start_float,
  CLEAR_KEY,       vfy_clear_key,
  NO_KEY,          (void *)NULL,
  KEYLOCK_LOCK,    (void *)NULL,
  OPEN_DRAWER_KEY, open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,     illegal_fn_key
};


extern INPUT_CONTROLLER Dstartfl13K13n=
{
  (INPUT_DISPLAY *)&dsp_start_float13,
  KEYBOARD_MASK,
  14, 14,
  (VERIFY_KEY *)&numeric_dnull
};

static PROCESS_ELEMENT StartFloat_PROC[] =
{
  ENTER_KEY,       shift_on_and_init_invoice,
  NO_KEY,          no_start_float,
  KEYLOCK_LOCK,    no_start_float,
  UNKNOWN_KEY,     (void *)NULL
};


static CONTROL_ELEMENT StartFloat_CTL[] =
{
  ENTER_KEY,       &CustomerMode_ST,
  NO_KEY,          &CashierMode_ST,
  KEYLOCK_LOCK,    &PosStandBy_ST,
  UNKNOWN_KEY,     &StartFloat_ST
};

extern STATE_OBJ StartFloat_ST =
{                         
  ST_START_FLOAT,
  StartFloat_VW,
  price_DFLT,
  &Dstartfl13K13n,
  StartFloat_VFY,
  (void *)NULL,
  StartFloat_PROC,
  StartFloat_CTL
};


/*---------------------------------------------------------------------------*/
/*                          no_start_float                                   */
/*---------------------------------------------------------------------------*/
static short no_start_float(_TCHAR *data, short key)
{
  cancel_shift(empty, 0);
  do_close_drawer(empty, 0);

  return(key);
} /* no_start_float */




/*---------------------------------------------------------------------------*/
/*                              vfy_start_float                              */
/*---------------------------------------------------------------------------*/
static short vfy_start_float(_TCHAR *data, short key)
{
  /* Check Start Float amount.                                             */
  if (STR_ZERO(data)) {
    err_invoke(ZERO_NOT_LEGAL_ERROR);
    key = UNKNOWN_KEY;
  }

  if (atof_price(data) > genvar.max_amnt_drawer) {
    err_invoke(EXCEED_MAX_START_FLOAT);
    key = UNKNOWN_KEY;
  }

  return(key);
} /* vfy_start_float */


/*---------------------------------------------------------------------------*/
/*                           shift_on_and_init_invoice                       */
/*---------------------------------------------------------------------------*/
/* - Send shift on to BO, and initialise the invoice.                        */
/*---------------------------------------------------------------------------*/
static short shift_on_and_init_invoice(_TCHAR *data, short key)
{
  SHIFT_ON_DEF shift_on;
  static short last_time_on = 0;

  /* INSERT MONEY AND CLOSE DRAWER                                         */
  wait_for_closed_drawer(input_TXT[38],(_TCHAR *)NULL);

  /* Make some final initialisations                                       */
  c_shft.date_on = pos_system.run_date;
  c_shft.time_on = get_current_time((_TCHAR *)NULL);
  while (c_shft.time_on == last_time_on) {
    err_invoke(CASHIER2FAST);               /* no 2 shifts within 1 minute */
    c_shft.time_on = get_current_time((_TCHAR *)NULL);
  }
  last_time_on       = c_shft.time_on;
  c_shft.time_off    = 9999;         /* 9999 indicates a not closed shift. */
  c_shft.start_float = atof_price(data);
  c_shft.end_float   = c_shft.start_float;

  tot_add_double(AMOUNT_IN_DRAWER, atof_price(data));

  if (tm_appe(TM_SHFT_NAME, (void*)&c_shft)>=0) {

    recover_shift_save();

    setup_shift_on(&shift_on);          /* Send shift-on to Backoffice     */
    if (train_mode == CASH_NORMAL) {    /* but don't send in training mode */
      pos_put_rec(SHFT_TYPE, POS_SHFO_SIZE, SHIFT_ON_FNO, (void*)&shift_on);
    }

    wallet_seq_no=0;                    /* Used in lift/refill actions.    */

    init_invoice(empty,0);
    init_cust_rec(&cust);

    status_of_pos=SHIFT_OPEN;          /* Next time it will be a start-of-shift.    */
  }
  else {
    err_invoke(TOO_MANY_SHIFTS);      /* TM_ error, cancel the shift and   */
    no_start_float(empty,0);          /* return to state CashierMode       */
    key = NO_KEY;
  }

  return(key);
} /* shift_on_and_init_invoice */


/*---------------------------------------------------------------------------*/
/*                            setup_shift_on                                 */
/*---------------------------------------------------------------------------*/
static short setup_shift_on(SHIFT_ON_DEF *shift_on)
{
  short  tmp;
  _TCHAR buffer[20];

  /*                                                                       */
  /* Fill structure with shift info from current shift.                    */
  /*                                                                       */

  tmp=invshtn;
  memcpy(&shift_on->delflg, &tmp, sizeof(short));

  shift_on->date_on    =c_shft.date_on;
  shift_on->time_on    =c_shft.time_on;
  shift_on->cashier_no =c_shft.cashier;
  shift_on->till_no    =pos_system.till_no;
  shift_on->start_float=c_shft.start_float;

  ftoa((double)c_shft.invoice_on, 19, buffer);
  shift_on->invoice_on=_ttol(buffer);

  return(SUCCEED);
} /* setup_shift_on */


/*---------------------------------------------------------------------------*/
/*                            setup_shift_off                                */
/*---------------------------------------------------------------------------*/
static short setup_shift_off(SHIFT_OFF_DEF *shift_off)
{
  short  tmp;
  _TCHAR buffer[20];

  /*                                                                       */
  /* Fill structure with shift info from current shift.                    */
  /*                                                                       */

  tmp=invshtf;
  memcpy(&shift_off->delflg, &tmp, sizeof(short));

  shift_off->date_on    =c_shft.date_on;
  shift_off->time_on    =c_shft.time_on;
  shift_off->cashier_no =c_shft.cashier;
  shift_off->till_no    =pos_system.till_no;
  shift_off->time_off   =c_shft.time_off;

  ftoa((double)c_shft.invoice_off, 19, buffer);
  shift_off->invoice_off=_ttol(buffer);

  return(SUCCEED);
} /* setup_shift_off */


/*---------------------------------------------------------------------------*/
/*                            setup_shift_lift                               */
/*---------------------------------------------------------------------------*/
short setup_shift_lift(SHIFT_LIFT_DEF *shift_lift)
{
  short tmp;

  /*                                                                       */
  /* Fill structure with shift info from current shift                     */
  /*                                                                       */
  /*   For shift_lift, the elements wallet_no and wallet_amount must be    */
  /*   assigned in the calling function.                                   */
  /*                                                                       */

  tmp=invshtl;
  memcpy(&shift_lift->delflg, &tmp, sizeof(short));

  shift_lift->date_on   =c_shft.date_on;
  shift_lift->time_on   =c_shft.time_on;
  shift_lift->cashier_no=c_shft.cashier;
  shift_lift->till_no   =pos_system.till_no;

  return(SUCCEED);
} /* setup_shift_lift */

/*---------------------------------------------------------------------------*/
/*                            setup_xread                                    */
/*---------------------------------------------------------------------------*/
static short setup_xread(SHIFT_TDM_DEF *xread)
{
  short tmp, i;

  tmp=invxred;
  memcpy(&xread->delflg, &tmp, sizeof(short));
  xread->date_on          = c_shft.date_on;
  xread->time_on          = c_shft.time_on;
  xread->cashier          = c_shft.cashier;
  xread->till_no          = c_shft.till_no;
  xread->time_off         = c_shft.time_off;
  xread->shift_no         = c_shft.shift_no;
  xread->invoice_on       = c_shft.invoice_on;
  xread->invoice_off      = c_shft.invoice_off;
  xread->nbr_inv_lines    = c_shft.nbr_inv_lines;
  xread->nbr_void_inv     = c_shft.nbr_void_inv;
  xread->nbr_void_lines   = c_shft.nbr_void_lines;
  xread->start_float      = c_shft.start_float;
  xread->end_float        = c_shft.end_float;
  xread->lift_refill_float= c_shft.lift_refill_float;
  for (i=0; i<3; i++) {
    xread->wallet_no[i]   = c_shft.wallet_no[i];
  }
  for (i=0; i<10; i++) {
    xread->paym_amnt[i]   = c_shft.paym_amnt[i];
  }
  xread->donation         = c_shft.donation;
  xread->rounded          = c_shft.rounded; // 3.4.7 acm -

  xread->percep_amount       = c_shft.percep_amount; //v3.6.1 acm -
  return(SUCCEED);
} /* setup_xread */
