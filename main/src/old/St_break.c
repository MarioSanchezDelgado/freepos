/*
 *     Module Name       : ST_BREAK.C
 *
 *     Type              : "Coffee" break states StartBreak, EndBreak
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
                                            /* POS (library) include files.  */
#include "misc_tls.h"
#include "misc_tls.h"
#include "stri_tls.h"
#include "storerec.h"
                                            /* Toolsset include files.       */
#include "inp_mgr.h"
#include "scrn_mgr.h"
#include "state.h"
#include "tm_mgr.h"
#include "mapkeys.h"
                                            /* Application include files.    */
#include "pos_inp.h"
#include "pos_recs.h"
#include "pos_tm.h"
#include "pos_dflt.h"
#include "pos_func.h"
#include "pos_keys.h"
#include "pos_st.h"
#include "pos_txt.h"
#include "pos_vfy.h"
#include "WPos_mn.h"
#include "st_main.h"
#include "condll.h"

/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   StartBreak_ST                                         */
/*-------------------------------------------------------------------------*/
static void StartBreak_VW(void)
{
  /* Next piece of code looks like PosStandBy_VW().                       */
  static short cls_states[]={
         ST_CUSTOMER_MODE
        ,ST_END_BREAK
        ,ST_BOPERATOR_MODE
        ,ST_COPERATOR_MODE
        ,0
  };

  if( called_by_state(cls_states)==SUCCEED ) {
    cls();
  }

  Basic_VW();
  scrn_string_out(scrn_inv_TXT[22], 5, 24);
  status_of_pos = CASHIER_ON_BREAK;
  cdsp_closed_till(0, FALSE);
  SetShowCursor(FALSE);

//  if( called_by_state(cls_states) != SUCCEED ) {
//    view_lan_state(FORCED_UPDATE); /* put at end because an error may occur */
//  }
} /* StartBreak_VW */

static VERIFY_ELEMENT StartBreak_VFY[] =
{                              
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER DkeyposNS =
{
  (INPUT_DISPLAY *)&dsp_keypos,
  KEYLOCK_N_MASK,
  0, 0,
  (VERIFY_KEY *)&no_data
};

static CONTROL_ELEMENT StartBreak_CTL[] =
{
  KEYLOCK_NORMAL,       &EndBreak_ST,
  UNKNOWN_KEY,          &StartBreak_ST
};

extern STATE_OBJ StartBreak_ST =
{                         
  ST_START_BREAK,
  StartBreak_VW,
  no_DFLT,
  &DkeyposNS,
  StartBreak_VFY,
  (void *)NULL,
  (void *)NULL,
  StartBreak_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   EndBreak                                              */
/*-------------------------------------------------------------------------*/
static void EndBreak_VW(void)
{
  /* Next piece of code looks like CashierMode_VW().                       */
  _TCHAR buffer[8];
  int cashno;

  scrn_select_window(CASHIER_LOGON_WINDOW);

  scrn_string_out(scrn_inv_TXT[19], 7, 24);                 /* CASHIER ID: */
  cashno=c_shft.cashier;
  if (cashno > 0) {
    _itot(cashno, buffer, DECIMAL);
    format_display(&dsp_cash_id3, buffer);
    if (cashno == cash.empl_no) {
      format_display(&dsp_cash_nm30, cash.empl_name);
    }
    else {
      format_display(&dsp_cash_nm30, empty);
    }
  }
  scrn_string_out(scrn_inv_TXT[20], 8, 24);                  /* PIN CODE:  */
  format_display_passwrd(&dsp_pincd4, empty);
} /* EndBreak_VW */


static VERIFY_ELEMENT EndBreak_VFY[] =
{             
  ENTER_KEY,            vfy_cash_pincd,
  CLEAR_KEY,            vfy_clear_key,
  OPERATOR_KEY,         (void *)NULL,
  KEYLOCK_LOCK,         (void *)NULL,
  KEYLOCK_SUPERVISOR,   vfy_key_in_N,
  OPEN_DRAWER_KEY,      open_and_close_drawer,
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER Dpincd4 =
{
  (INPUT_DISPLAY *)&dsp_pincd4,
  KEYBOARD_MASK | KEYLOCK_L_MASK | KEYLOCK_S_MASK,
  5, 5,
  (VERIFY_KEY *)&numeric
};

static PROCESS_ELEMENT EndBreak_PROC[] =
{
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT EndBreak_CTL[] =
{
  ENTER_KEY,            &CustomerMode_ST,
  CLEAR_KEY,            &EndBreak_ST,
  PINCODE_NOK,          &CustomerMode_ST,
  OPERATOR_KEY,         &BOperatorMode_ST,
  KEYLOCK_LOCK,         &StartBreak_ST,
  KEYLOCK_NORMAL,       &CustomerMode_ST,
  UNKNOWN_KEY,          &EndBreak_ST
};

extern STATE_OBJ EndBreak_ST =
{                         
  ST_END_BREAK,
  EndBreak_VW,
  no_DFLT,
  &Dpincd4,
  EndBreak_VFY,
  (void *)NULL,
  EndBreak_PROC,
  EndBreak_CTL
};
