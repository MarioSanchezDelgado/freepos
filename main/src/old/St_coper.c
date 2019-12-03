/*
 *     Module Name       : ST_COPER.C
 *
 *     Type              : States COperatorMode, CTillLift, CTillWallet,
 *                         CTillRefill, CTillRefillOk, XRCashno
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
 * 10-Sep-2001 Added display of deleting invoice data at end of day    J.D.M.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 25-Oct-2002 Added some forward skips and reverse skips.             J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "stnetp24.h"                       /* Pos (library) include files   */
#include "comm_tls.h"
#include "misc_tls.h"
#include "stri_tls.h"
#include "intrface.h"

#include "inp_mgr.h"                        /* Toolsset include files.       */
#include "err_mgr.h"
#include "state.h"
#include "bp_mgr.h"
#include "tm_mgr.h"
#include "tot_mgr.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "mapkeys.h"
#include "ConDll.h"

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
#include "write.h"
#include "pos_bp1.h"
#include "pos_com.h"
#include "pos_tot.h"
#include "WPos_mn.h"
#include "st_main.h"

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES  CUSTOMER OPERATOR MENU MODE                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Previous state was CustomerMode (12). The OPERATOR-Key was pressed and   */
/*  confirmed with te S-key.                                                 */
/*   - Display Operator menu, and execute menu-functions.                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void COperatorMode_VW(void)
{
  scrn_clear_window(OPERATOR_WINDOW);
  bcoperator_menu(empty, 0);            /* Initialise the menu.            */
} /* COperatorMode_VW */


extern VERIFY_ELEMENT COperatorMode_VFY[] =
{                              
  ENTER_KEY,            bcoperator_menu,
  CLEAR_KEY,            vfy_clear_key,
  KEYLOCK_NORMAL,       (void *)NULL,
  KEYLOCK_LOCK,         (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

INPUT_CONTROLLER Dopermnu1K3n=
{
  (INPUT_DISPLAY *)&oper_menu3,
  KEYBOARD_MASK |
  KEYLOCK_N_MASK | KEYLOCK_L_MASK,
  2, 2,
  (VERIFY_KEY *)&numeric
};


extern PROCESS_ELEMENT COperatorMode_PROC[] =
{
  2,                    EndOfShift,
  3,                    EndOfDay,
#ifndef PRN_IMM
  6,                    reprint_invoice,
#endif
  KEYLOCK_NORMAL,       proc_clr_scrn,
  UNKNOWN_KEY,          (void *)NULL
};


static CONTROL_ELEMENT COperatorMode_CTL[] =
{
  1,              &XRCashno_ST,
  2,              &PosStandBy_ST,
  3,              &PosStandBy_ST,
  4,              &CTillLift_ST,
  5,              &CTillRefill_ST,
  KEYLOCK_LOCK,   &StartBreak_ST,
  KEYLOCK_NORMAL, &CustomerMode_ST,
  UNKNOWN_KEY,    &COperatorMode_ST
};


extern STATE_OBJ COperatorMode_ST =
{
  ST_COPERATOR_MODE,
  COperatorMode_VW,
  no_DFLT,
  &Dopermnu1K3n,
  COperatorMode_VFY,
  (void *)NULL,
  COperatorMode_PROC,
  COperatorMode_CTL
};


/*---------------------------------------------------------------------------*/
/*                             proc_clr_scrn                                 */
/*---------------------------------------------------------------------------*/
short proc_clr_scrn(_TCHAR *data, short key)
{
  cls();
  return(key);
} /* proc_clr_scrn */


/*---------------------------------------------------------------------------*/
/*                         bcoperator_menu                                   */
/*---------------------------------------------------------------------------*/
short bcoperator_menu(_TCHAR *data, short key)
{
  short max_choice;
  short menu_strings[] = {        /* Refer to index of menu_TXT[x].        */
#ifdef PRN_IMM
    1, 2, 3, 4, 5, 0
#else
    1, 2, 3, 4, 5, 28, 0
#endif
  };

  /*                                                                       */
  /* View the customer/break operator menu, activate an option if key is   */
  /* within menu-range.                                                    */
  /*                                                                       */

#ifdef PRN_IMM
  max_choice = 5;
#else  
  max_choice = 6;
#endif

  if (cust.cust_no < 0) {
    menu_strings[5] = 0;
    max_choice = 5;
  }

  if( key == UNKNOWN_KEY ) {
    /* Display the menu.                                                 */
    scrn_select_window(OPERATOR_WINDOW);
    scrn_string_out(menu_TXT[11], 1,24);                /* OPERATOR MENU */
    view_menu(7, 24, menu_strings, empty);
    scrn_string_out(menu_TXT[10],15,24);                /* Prompt.       */
    return(key);
  }
  else {
    /* A choice is made, check for legal value.                          */
    if( vfy_range(data,1,max_choice) != SUCCEED ) {
      err_invoke(INVALID_MENU_OPTION);                   /* out of range */
      *data=_T('\0');                                    /* reset buffer */
      return(UNKNOWN_KEY);
    }
    else {
      SetShowCursor(FALSE);
      view_menu(7, 24, menu_strings, data);
      return(_ttoi(data));
    }
  }
} /* bcoperator_menu */


/*---------------------------------------------------------------------------*/
/*                         view_menu                                         */
/*---------------------------------------------------------------------------*/
void view_menu(short y, short x, short *menu_strings, _TCHAR *active)
{
  short i=1;
  _TCHAR buffer1[5];
  _TCHAR buffer[83];

  /*                                                                       */
  /* View a menu, if active > 0  then invert menu option active.           */
  /*                                                                       */

  while (*menu_strings != 0) {
    ch_memset(buffer, _T(' '), sizeof(buffer));
    if (_ttoi(active) == i) {
      buffer[0]=_T('\1');                             /* inverse-video     */
    }
    else {
      buffer[0]=_T('\2');                             /* normal-video      */
    }
    _itot(i,buffer1,DECIMAL);
    _tcscat(buffer1,_T("."));
    strcpy_no_null(buffer+1, buffer1);
    _tcscpy(buffer+5, menu_TXT[*menu_strings]);
    //_tcscat(buffer, _T("\2"));                           /* normal-video      */
    scrn_string_out(buffer, (short)(y+i), x);
    menu_strings++;
    i++;
  }
} /* view_menu */


/*---------------------------------------------------------------------------*/
/*                           EndOfShift                                      */
/*---------------------------------------------------------------------------*/
short EndOfShift(_TCHAR *data, short key)
{
  short cashno;

  /*                                                                       */
  /* Perform an end of shift:                                              */
  /*   Close, save and log shift                                           */
  /*   Print x-read                                                        */
  /*                                                                       */

  display_working(YES);

  init_invoice(empty,0);
  init_cust_rec(&cust);

  cashno=c_shft.cashier;              /* Get cashno before closing shift!  */
  close_shift(empty,0);               /* Close save and log shift.         */
  rs_open_cash_drawer(CASH_DRAWER1);
  proc_x_read(cashno);
  display_working(NO);

  status_of_pos = START_OF_SHIFT;
  tot_reset_double(AMOUNT_IN_DRAWER);

  wait_for_closed_drawer(input_TXT[29],(_TCHAR *)NULL);

  return(key);
} /* EndOfShift */


/*---------------------------------------------------------------------------*/
/*                              EndOfDay                                     */
/*---------------------------------------------------------------------------*/
short EndOfDay(_TCHAR *data, short key)
{
  short cashno, conv;

  /*                                                                       */
  /* Perform an end of day:                                                */
  /*   Close, save and log current shift                                   */
  /*   If there are any shifts, print the Z-read.                          */
  /*   Remove any exsistant shifts                                         */
  /*                                                                       */

  display_working(YES);

  init_invoice(empty,0);
  init_cust_rec(&cust);

  if (c_shft.time_off==9999) {    /* Current unclosed shift, process it.   */
    cashno=c_shft.cashier;        /* Get cashno before closing shift!      */
    close_shift(empty,0);         /* Close save, log and print shift.      */
    proc_x_read(cashno);
  }
  else {
    close_shift(empty,0);             /* Close save, log and print shift.  */
  }

  rs_open_cash_drawer(CASH_DRAWER1);
  if (tm_frst(TM_SHFT_NAME, (void*)&c_shft)>=0) {
                                  /* If a shift is available, print it.    */
  /*mlsd*/
   bp_now(BP_ZREAD, BP_REVERSE_SKIP, (TM_INDX)0);
   bp_now(BP_ZREAD, BP_XR_ZR_INIT, (TM_INDX)0);
   bp_now(BP_ZREAD, BP_ZR_PER_TILL, (TM_INDX)0);
   bp_now(BP_ZREAD, BP_FORWARD_SKIP, 0);
  /*mlsd*/
  }

  /* If there is a Zread available, log it only in non-training mode.      */
  if (c_day.shift_no > 0 && train_mode == CASH_NORMAL) {
    conv = invzred;
    memcpy((void*)&c_day.delflg, (void*)&conv, sizeof(short));
    pos_put_rec(SHFT_TYPE, POS_RECO_SIZE, RECOVER_FNO, (void*)&c_day);
  }

  recover_shift_remv();           /* Remove any existent shifts from disk. */

  tm_reset_struct(TM_SHFT_NAME);  /* Remove all shifts from the TM.        */
  tm_reset_struct(TM_ITEM_NAME);  /* Remove all items from the TM.         */

  display_prompt(prompt_TXT[10], ERROR_WINDOW_ROW1);
  end_of_day_p24();                          /* Inform stnetp24 of the eod */
  scrn_clear_window(ERROR_WINDOW_ROW1);

  status_of_pos=START_OF_DAY;         /* Causes display of START OF DAY    */
  tot_reset_double(AMOUNT_IN_DRAWER);

  pos_system.current_mode=pos_system.default_mode;
  if( train_mode == CASH_NORMAL) {/* don't save in train.mode  */
    pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
  }

  invoice_mode=pos_system.current_mode;
  invoice_line_mode=invoice_mode;

  display_working(NO);
#ifndef NO_VIEW_POS_STATE
  view_pos_state();
#endif
  wait_for_closed_drawer(input_TXT[29],(_TCHAR *)NULL);

  return(key);
} /* EndOfDay */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   CUSTOMER OPERATOR TILL LIFT                            */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Perform a till-lift.                                                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static CONTROL_ELEMENT CTillLift_CTL[] =
{
  NO_KEY,          &COperatorMode_ST,
  ENTER_KEY,       &CTillWallet_ST,
  KEYLOCK_NORMAL,  &COperatorMode_ST,
  UNKNOWN_KEY,     &CTillLift_ST
};

extern STATE_OBJ CTillLift_ST =
{
  ST_CTILL_LIFT,
  OpTillLift_VW,
  no_DFLT,
  &Dwallamnt13n,
  OpTillLift_VFY,
  (void *)NULL,
  OpTillLift_PROC,
  CTillLift_CTL
};


/*---------------------------------------------------------------------------*/
/*                          do_close_drawer                                  */
/*---------------------------------------------------------------------------*/
short do_close_drawer(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Only wait for closed drawer.                                          */
  /*                                                                       */
  wait_for_closed_drawer(input_TXT[29], (_TCHAR *)NULL);

  return(key);
} /* do_close_drawer */


/*---------------------------------------------------------------------------*/
/*                          start_till_lift                                  */
/*---------------------------------------------------------------------------*/
short start_till_lift(_TCHAR *data, short key)
{
  _TCHAR buffer[32];

  /*                                                                       */
  /* Get the till-lift structure, empty it and insert amount.              */
  /*                                                                       */

  if (!STR_ZERO(data)) {
    *buffer= _T('-');
    _tcscpy(buffer+1,data);
    till_lift.wallet_amount=(double)atof_price(buffer);
  }
  else {
    till_lift.wallet_amount=0.0;
  }

  return(key);
} /* start_till_lift */


/*---------------------------------------------------------------------------*/
/*                              vfy_wallet_amnt                              */
/*---------------------------------------------------------------------------*/
short vfy_wallet_amnt(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Check wether the wallet-amount exceeds the till-float (END_FLOAT_LEAF */
  /* in SHIFT_GROUP).                                                      */
  /*                                                                       */

  if (c_shft.end_float<atof_price(data)) {
    err_invoke(LIFT_AMNT_EXCEED_TILL_AMNT);
    *data=_T('\0');
    return(UNKNOWN_KEY);
  }

  return(key);
} /* vfy_wallet_amnt */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES  CUSTOMER OPERATOR WALLET NUMBER                         */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Retrieve wallet-number for till-lift.                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/


static CONTROL_ELEMENT CTillWallet_CTL[] =
{
  NO_KEY,          &COperatorMode_ST,
  ENTER_KEY,       &COperatorMode_ST,
  ILLEGAL_VALUES,  &CTillLift_ST,       /* Both fields zero, retry.         */
  KEYLOCK_NORMAL,  &COperatorMode_ST,
  UNKNOWN_KEY,     &CTillWallet_ST
};

extern STATE_OBJ CTillWallet_ST =
{
  ST_CTILL_WALLET,
  (void *)NULL,
  no_DFLT,
  &Dwallno3n,                  
  OpTillWallet_VFY,
  (void *)NULL,
  OpTillWallet_PROC,
  CTillWallet_CTL
};


/*---------------------------------------------------------------------------*/
/*                         proc_till_lift_refill                             */
/*---------------------------------------------------------------------------*/
short proc_till_lift_refill(_TCHAR *data, short key)
{
  short i;

  /*                                                                       */
  /* Process the till-lift record, started in start_till_lift(), if one    */
  /* of the fields wallet_amount or wallet_no unequal zero.                */
  /*                                                                       */
  /* Insert wallet number in till-lift record and decrese end_float_leaf   */
  /* for this shift with the previous entered amount.                      */
  /* Also put the till-lift record on the LAN.                             */
  /*                                                                       */

  if (*data!=_T('\0') || till_lift.wallet_amount!=0.0) {
                    /* One of the fields is filled, process the till-lift. */
    if (c_shft.shift_no!=0) {
      if( *data!=_T('\0')) {
        till_lift.wallet_no=(short)_ttoi(data);
        for (i=0; i<3 && c_shft.wallet_no[i]!=WALLET_NOT_USED; i++) {
          ;
        }
        if (i<3) {
          c_shft.wallet_no[i]=till_lift.wallet_no;
        }
        /* else, there are already three lifts/refills, this wallet        */
        /* number is lost for the X/Z-read                                 */
      }
      else {
        till_lift.wallet_no=0;        /* wallet number was not entered.    */
      }

      if (till_lift.wallet_amount!=0.0) {
        c_shft.end_float        +=till_lift.wallet_amount;
        c_shft.lift_refill_float+=till_lift.wallet_amount;
        tot_add_double(AMOUNT_IN_DRAWER, till_lift.wallet_amount);
      }

      till_lift.seq_no=++wallet_seq_no;

      /* put SHIFT_LIFT on the LAN.                                        */
      setup_shift_lift(&till_lift);
      if (train_mode == CASH_NORMAL) {  /* don't send in training mode     */
        pos_put_rec(SHFT_TYPE, POS_SHFL_SIZE, SHIFT_LIFT_FNO, (void*)&till_lift);
      }

      tm_upda(TM_SHFT_NAME, (void*)&c_shft);
      recover_shift_updt();
    }
    else {
      key = UNKNOWN_KEY;
    }
  }
  else {            /* Both amount and wallet are zero/empty, try again.   */
    err_invoke(CANCEL_TILL_LIFT_REFILL);
    key = ILLEGAL_VALUES;
  }
  do_close_drawer((_TCHAR *)NULL, 0);

  return(key);
} /* proc_till_lift_refill */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES  CUSTOMER OPERATOR TILL REFILL                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Perform a till-refill.                                                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static CONTROL_ELEMENT CTillRefill_CTL[] =
{
  NO_KEY,          &COperatorMode_ST,
  ENTER_KEY,       &CTillRefillOk_ST,
  KEYLOCK_NORMAL,  &COperatorMode_ST,
  UNKNOWN_KEY,     &CTillRefill_ST
};

extern STATE_OBJ CTillRefill_ST =
{
  ST_CTILL_REFILL,
  OpTillRefill_VW,
  no_DFLT,
  &Dwallamnt13n,
  OpTillRefill_VFY,
  (void *)NULL,
  OpTillRefill_PROC,
  CTillRefill_CTL
};

/*---------------------------------------------------------------------------*/
/*                               start_till_refill                           */
/*---------------------------------------------------------------------------*/
short start_till_refill(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Get the till-lift structure, empty it and insert amount.              */
  /*                                                                       */

  if (!STR_ZERO(data)) {
    till_lift.wallet_amount=(double)atof_price(data);
  }
  else {
    till_lift.wallet_amount=0.0;
  }

   return(key);
} /* start_till_refill */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   CUSTOMER OPERATOR TILL REFILL OK                       */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Retrieve confirmation to perform a till-refill.                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static CONTROL_ELEMENT CTillRefillOk_CTL[] =
{
  NO_KEY,          &CTillRefill_ST,
  ENTER_KEY,       &COperatorMode_ST,
  KEYLOCK_NORMAL,  &COperatorMode_ST,
  UNKNOWN_KEY,     &CTillRefillOk_ST
};


extern STATE_OBJ CTillRefillOk_ST =
{
  ST_CTILL_REFILLOK,
  OpTillRefillOk_VW,
  no_DFLT,
  &DMode1K1,
  OpTillRefillOk_VFY,
  OpTillRefillOk_UVW,
  OpTillRefillOk_PROC,
  CTillRefillOk_CTL
};

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   ENTER CASHIER NUMBER FOR X-READ                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Retrieve cashier number to perform a x-read.                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

extern INPUT_CONTROLLER XRcashid3K3n =
{
  (INPUT_DISPLAY *)&XRdsp_cash_id3,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  4, 4,
  (VERIFY_KEY *)&numeric
};


static CONTROL_ELEMENT XRCashno_CTL[] =
{
  ENTER_KEY,       &COperatorMode_ST,
  NO_KEY,          &COperatorMode_ST,
  KEYLOCK_NORMAL,  &COperatorMode_ST,
  UNKNOWN_KEY,     &XRCashno_ST
};


extern STATE_OBJ XRCashno_ST =
{
  ST_XR_CASHNO,
  OpXRCashno_VW,
  no_DFLT,
  &XRcashid3K3n,
  OpXRCashno_VFY,
  (void *)NULL,
  OpXRCashno_PROC,
  XRCashno_CTL
};


/*---------------------------------------------------------------------------*/
/*                                XRead                                      */
/*---------------------------------------------------------------------------*/
short XRead(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Perform an X-read of the requested cashier-number (data).             */
  /*                                                                       */
  proc_x_read((short)_ttoi(data));
  *data=_T('\0');

  return(key);
} /* XRead */


/*---------------------------------------------------------------------------*/
/*                           proc_x_read                                     */
/*---------------------------------------------------------------------------*/
short proc_x_read(short requested_cashno)
{
  TM_INDX shft_indx;

  /*                                                                       */
  /* Print all shift-data of the requested cashier number.                 */
  /*                                                                       */

  shft_indx=tm_frst(TM_SHFT_NAME, (void*)&c_shft);

  while (shft_indx >= 0 && c_shft.cashier != requested_cashno) {
    shft_indx=tm_next(TM_SHFT_NAME, (void*)&c_shft);
  }

  if (shft_indx < 0) {            /* If cashier-no not found, give error.  */
    err_invoke(CASHIER_NOT_AVAILABLE);
  }
  else if (c_shft.cashier == requested_cashno) {
  /*mlsd*/
    bp_now(BP_XREAD, BP_REVERSE_SKIP, 0);
    bp_now(BP_XREAD, BP_XR_ZR_INIT, 0);
    bp_now(BP_XREAD, BP_XR_PER_CASHIER, 0);
    bp_now(BP_XREAD, BP_FORWARD_SKIP, 0);
  /*mlsd*/
  }

  return(SUCCEED);
} /* proc_x_read */
