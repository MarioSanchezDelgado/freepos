/*
 *     Module Name       : ST_OPER.C
 *
 *     Type              : States SoMode, OperatorMode, OpXRCashno, SupervisorMode
 *                         SvXRCashno, LogOn, ChangeMode, Training, GeneralVars,
 *                         PaymentWays, PaymAmount, PaymType, PaymExtraVat,
 *                         PaymExtraMin, PaymExtraPerc, FillSysDate, FillSysTime,
 *                         FillGvAmnt, DefaultMode
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
 * 19-Jul-2001 When changing the keylock to CashierMode_ST the function
 *             proc_init_environment_records is called                 J.D.M.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <time.h>
#include <dos.h>


#include "stnetp24.h"                       /* Pos (library) include files   */
#include "comm_tls.h"
#include "intrface.h"
#include "date_tls.h"
#include "misc_tls.h"
#include "stri_tls.h"

#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "tm_mgr.h"
#include "inp_mgr.h"
#include "prn_mgr.h"
#include "state.h"
#include "err_mgr.h"
#include "fmt_tool.h"
#include "bp_mgr.h"
#include "mapkeys.h"

#include "pos_tm.h"                         /* Application include files.  */
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
#include "pos_scrl.h"
#include "pos_com.h"

#include "write.h"
#include "pos_bp1.h"
#include "st_main.h"
#include "condll.h"
#include "pos_edoc.h"

static short operator_menu(_TCHAR *, short);
static short supervisor_menu(_TCHAR *, short);
static short general_menu(_TCHAR *, short);
static short proc_paym_cd_choice(_TCHAR *, short);
static short proc_show_logon(_TCHAR *, short);
static short proc_change_current_mode(_TCHAR *, short);
static short proc_change_train_mode(_TCHAR *, short);
static short proc_amnt_choice(_TCHAR *, short);
static void  view_c_payment_ways_menu(short);
static short proc_paym_amount(_TCHAR *, short);
static short proc_extra_min(_TCHAR *, short);
static short proc_extra_perc(_TCHAR *, short);
static short proc_extra_vat(_TCHAR *, short);
static short proc_set_time(_TCHAR *, short);
static short proc_genvar_amount(_TCHAR *, short);
static short proc_change_default_mode(_TCHAR *, short);
static short proc_paym_type(_TCHAR *, short);

/* Next variable needed for communication between states PaymentWays and   */
/* PaymAmount and states GeneralVars and FillGVAmnt. This is cheaper than  */
/* 9 + 2 extra states.                                                     */
static short last_choice = 0;

/*-------------------------------------------------------------------------*/
/* Some verify functions                                                   */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*                           vfy_menu_option10                             */
/*-------------------------------------------------------------------------*/
short vfy_menu_option10(_TCHAR *data, short key)
{
  /* Check: if 1 =< _ttoi(data) <= 10                                       */
  /* If ok, return key.                                                    */

  if (vfy_range(data, 1, 10) != SUCCEED) {
    err_invoke(INVALID_MENU_OPTION);                       /* out of range */
    *data = _T('\0');                                     /* reset buffer */
    key = UNKNOWN_KEY;
  }

  return key;
} /* vfy_menu_option10 */


/*-------------------------------------------------------------------------*/
/*                           vfy_date                                      */
/*-------------------------------------------------------------------------*/
short vfy_date(_TCHAR *data, short key)
{
  if (check_date(_ttol(data)) == FAIL) {
    err_invoke(ILLEGAL_DATE_VALUE);
    *data = _T('\0');
    return(UNKNOWN_KEY);
  }

  return(key);
} /* vfy_date */


/*-------------------------------------------------------------------------*/
/*                           vfy_time                                      */
/*-------------------------------------------------------------------------*/
short vfy_time(_TCHAR *data, short key)
{
  if (check_time(_ttoi(data)) == FAIL) {
    err_invoke(ILLEGAL_TIME_VALUE);
    *data = _T('\0');
    return(UNKNOWN_KEY);
  }

  return(key);
} /* vfy_time */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE  SoMode                                                 */
/*-------------------------------------------------------------------------*/
static void SoMode_VW(void)
{
  static short no_cls_states[]={
       ST_SO_MODE
      ,0
  };

  if( called_by_state(no_cls_states)==FAIL ) {
    PosStandBy_VW();
  }

  scrn_string_out(input_TXT[8], 7, 24); /* SUPERVISOR CODE OR <OPERATOR>   */
  format_display_passwrd(&dsp_1pincd4, empty);
  SetShowCursor(TRUE);
} /* SoMode_VW */

static VERIFY_ELEMENT SoMode_VFY[] =
{                              
  OPERATOR_KEY,         (void *)NULL,
  ENTER_KEY,            vfy_super_pincd,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       proc_init_environment_records,
  KEYLOCK_LOCK,         (void *)NULL,
  KEYLOCK_EXCEPTION,    (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER DpinK4nLln =
{
  (INPUT_DISPLAY *)&dsp_1pincd4,
  KEYBOARD_MASK | KEYLOCK_N_MASK | KEYLOCK_L_MASK | KEYLOCK_X_MASK,
  5,
  5,
  (VERIFY_KEY *)&numeric
};

static CONTROL_ELEMENT SoMode_CTL[] =
{
  OPERATOR_KEY,         &OperatorMode_ST,
  ENTER_KEY,            &SupervisorMode_ST,
  KEYLOCK_NORMAL,       &CashierMode_ST,
  KEYLOCK_LOCK,         &PosStandBy_ST,
  KEYLOCK_EXCEPTION,    &EdpMode_ST,
  UNKNOWN_KEY,          &SoMode_ST
};

extern STATE_OBJ SoMode_ST =
{                         
  ST_SO_MODE,                           /* a (unique) identifying short    */
  SoMode_VW,                            /* a pointer to a view function    */
  no_DFLT,                              /* a pointer to a default function */
  &DpinK4nLln,                          /* a pointer to an input structure */
  SoMode_VFY,                           /* a pointer to a verify table     */
  (void *)NULL,                         /* a pointer to an unview function */
  (void *)NULL,                         /* a pointer to a process table    */
  SoMode_CTL                            /* a pointer to a control table    */
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE  OperatorMode                                           */
/*-------------------------------------------------------------------------*/
void OperatorMode_VW(void)
{
  scrn_clear_window(OPERATOR_WINDOW);
  operator_menu(empty, 0);                  /* Initialise the menu.        */
} /* OperatorMode_VW */

extern VERIFY_ELEMENT OperatorMode_VFY[] =
{                              
  ENTER_KEY,            operator_menu,
  CLEAR_KEY,            vfy_clear_key,
  KEYLOCK_NORMAL,       (void *)NULL,
  KEYLOCK_LOCK,         (void *)NULL,
  KEYLOCK_EXCEPTION,    (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};


extern INPUT_CONTROLLER Dopermnu1K1n = {
  (INPUT_DISPLAY *)&oper_menu1,
  KEYBOARD_MASK |
  KEYLOCK_N_MASK | KEYLOCK_L_MASK,
  2, 2,
  (VERIFY_KEY *)&numeric
};


extern PROCESS_ELEMENT OperatorMode_PROC[] =
{
  2,                    EndOfDay,
  KEYLOCK_NORMAL,       proc_init_environment_records,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT OperatorMode_CTL[] =
{
  1,                    &OpXRCashno_ST,
  2,                    &OperatorMode_ST,
  KEYLOCK_NORMAL,       &CashierMode_ST,
  KEYLOCK_LOCK,         &PosStandBy_ST,
  KEYLOCK_EXCEPTION,    &EdpMode_ST,
  UNKNOWN_KEY,          &OperatorMode_ST
};

extern STATE_OBJ OperatorMode_ST =
{                         
  ST_OPERATOR_MODE,                     /* a (unique) identifying short    */
  OperatorMode_VW,                      /* a pointer to a view function    */
  no_DFLT,                              /* a pointer to a default function */
  &Dopermnu1K1n,                        /* a pointer to an input structure */
  OperatorMode_VFY,                     /* a pointer to a verify table     */
  (void *)NULL,                         /* a pointer to an unview function */
  OperatorMode_PROC,                    /* a pointer to a process table    */
  OperatorMode_CTL                      /* a pointer to a control table    */
};


/*-------------------------------------------------------------------------*/
/*                         operator_menu                                   */
/*-------------------------------------------------------------------------*/
static short operator_menu(_TCHAR *data, short key)
{
  short menu_strings[] = {        /* Refer to index of menu_TXT[x].      */
      1, 3, 0,
  };

  /*                                                                     */
  /* View the operator menu, activate an option if key is within menu-   */
  /* range.                                                              */
  /*                                                                     */

  if( key == UNKNOWN_KEY ) {
    /* Display the menu.                                                 */
    scrn_select_window(OPERATOR_WINDOW);
    scrn_string_out(menu_TXT[11], 1,24);        /* OPERATOR MENU */
    view_menu(6, 24, menu_strings, empty);
    scrn_string_out(menu_TXT[10],13,23);        /* Prompt.       */
    return(key);
  }
  else {
    /* A choice is made, check for legal value.                          */
    if( vfy_range(data,1,2) != SUCCEED ) {
      err_invoke(INVALID_MENU_OPTION);                  /* out of range  */
      *data=_T('\0');                                   /* reset buffer  */
      return UNKNOWN_KEY;
    }
    else {
      view_menu(6, 24, menu_strings, data);
      return(_ttoi(data));
    }
  }
} /* operator_menu */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   OpXRCashno                                            */
/*-------------------------------------------------------------------------*/
void OpXRCashno_VW(void)
{
  scrn_draw_line(14, 0, 80, _T('_'));       /* draw '_' on row=14 col=0 len=80 */
  scrn_clear_rest_of_line( scrn_get_current_window() ,15 ,1 );
  scrn_string_out(input_TXT[2], 16, 27);    /* ENTER CASHIER NUMBER */
} /* OpXRCashno_VW */

extern VERIFY_ELEMENT OpXRCashno_VFY[] =
{
  ENTER_KEY,            vfy_non_zero,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};


extern PROCESS_ELEMENT OpXRCashno_PROC[] =
{
  ENTER_KEY,            XRead,
  UNKNOWN_KEY,          (void *)NULL
};


static CONTROL_ELEMENT OpXRCashno_CTL[] =
{
  ENTER_KEY,            &OperatorMode_ST,
  NO_KEY,               &OperatorMode_ST,
  KEYLOCK_NORMAL,       &OperatorMode_ST,
  UNKNOWN_KEY,          &OpXRCashno_ST
};

extern STATE_OBJ OpXRCashno_ST =
{                         
  ST_OP_XR_CASHNO,
  OpXRCashno_VW,
  no_DFLT,
  &XRcashid3K3n,
  OpXRCashno_VFY,
  (void *)NULL,
  OpXRCashno_PROC,
  OpXRCashno_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   OpTillLift                                            */
/*-------------------------------------------------------------------------*/
void OpTillLift_VW(void)
{
  /* Used in COperatorMode and BOperatorMode.                              */
  scrn_draw_line(14, 0, 80, _T('_'));       /* draw '_' on row=14 col=0 len=80 */
  scrn_clear_rest_of_line( scrn_get_current_window() ,15 ,1 );
  scrn_string_out(input_TXT[9], 16 ,24);    /* AMOUNT LIFTED:              */
  format_display(&dsp_wallamnt15, empty);
  scrn_string_out(input_TXT[10], 18 ,24);   /* WALLET NUMBER:              */
  format_display(&dsp_wallno3, empty);
  rs_open_cash_drawer(CASH_DRAWER1);
} /* OpTillLift_VW */


/* Used in COperatorMode and BOperatorMode.                                */
extern VERIFY_ELEMENT OpTillLift_VFY[] =
{
  ENTER_KEY,            vfy_wallet_amnt,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

/* Used in COperatorMode and BOperatorMode.                                */
extern INPUT_CONTROLLER Dwallamnt13n =
{
  (INPUT_DISPLAY *)&dsp_wallamnt15,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  15,
  14,
  (VERIFY_KEY *)&numeric_dnull
};

/* Used in COperatorMode and BOperatorMode.                                */
extern PROCESS_ELEMENT OpTillLift_PROC[] =
{
  ENTER_KEY,            start_till_lift,
  NO_KEY,               do_close_drawer,
  KEYLOCK_NORMAL,       do_close_drawer,
  UNKNOWN_KEY,          (void *)NULL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   OpTillWallet                                          */
/*-------------------------------------------------------------------------*/

/* Used in COperatorMode and BOperatorMode.                                */
extern VERIFY_ELEMENT OpTillWallet_VFY[] =
{
  ENTER_KEY,            vfy_non_empty_zero,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

/* Used in COperatorMode and BOperatorMode.                                */
extern INPUT_CONTROLLER Dwallno3n =
{
  (INPUT_DISPLAY *)&dsp_wallno3,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  4,
  4,
  (VERIFY_KEY *)&numeric
};

/* Used in COperatorMode and BOperatorMode.                                */
extern PROCESS_ELEMENT OpTillWallet_PROC[] =
{
  ENTER_KEY,            proc_till_lift_refill,
  NO_KEY,               do_close_drawer,
  KEYLOCK_NORMAL,       do_close_drawer,
  UNKNOWN_KEY,          (void *)NULL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   OpTillRefill                                          */
/*-------------------------------------------------------------------------*/

/* Used in COperatorMode and BOperatorMode.                                */
void OpTillRefill_VW(void)
{
  scrn_draw_line(14, 0, 80, _T('_'));
  scrn_clear_rest_of_line( scrn_get_current_window() ,15 ,1 );
  scrn_string_out(input_TXT[11], 16, 24);   /* REFILL AMOUNT:              */
  format_display(&dsp_wallamnt15, empty);
  rs_open_cash_drawer(CASH_DRAWER1);
} /* OpTillRefill_VW */

/* Used in COperatorMode and BOperatorMode.                                */
extern VERIFY_ELEMENT OpTillRefill_VFY[] =
{
  ENTER_KEY,            vfy_max_amount_drawer_or_zero,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

/* Used in COperatorMode and BOperatorMode.                                */
extern PROCESS_ELEMENT OpTillRefill_PROC[] =
{
  ENTER_KEY,            start_till_refill,
  NO_KEY,               do_close_drawer,
  KEYLOCK_NORMAL,       do_close_drawer,
  UNKNOWN_KEY,          (void *)NULL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   OpTillRefillOk                                        */
/*-------------------------------------------------------------------------*/

/* Used in COperatorMode and BOperatorMode.                                */
void OpTillRefillOk_VW(void)
{
  scrn_string_out(input_TXT[12], 18, 24);   /* REFILL AMOUNT OK Y/N        */
} /* OpTillRefillOk_VW */

/* Used in COperatorMode and BOperatorMode.                                */
void OpTillRefillOk_UVW(void)
{
  scrn_clear_rest_of_line(OPERATOR_WINDOW, 17, 0);
} /* OpTillRefillOk_UVW */

/* Used in COperatorMode and BOperatorMode.                                */
extern VERIFY_ELEMENT OpTillRefillOk_VFY[] =
{
  ENTER_KEY,            (void *)NULL,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

/* Used in COperatorMode and BOperatorMode.                                */
extern INPUT_CONTROLLER DMode1K1 =
{
  (INPUT_DISPLAY *)&dsp_1mode1,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  1,
  1,
  (VERIFY_KEY *)&no_data
};

/* Used in COperatorMode and BOperatorMode.                                */
extern PROCESS_ELEMENT OpTillRefillOk_PROC[] =
{
  ENTER_KEY,            proc_till_lift_refill,
  KEYLOCK_NORMAL,       do_close_drawer,
  UNKNOWN_KEY,          (void *)NULL
};

 
/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE  SupervisorMode                                         */
/*-------------------------------------------------------------------------*/
static void SupervisorMode_VW(void)
{
  scrn_clear_window(OPERATOR_WINDOW);
  supervisor_menu(empty, 0);               /* Initialise the menu.         */
} /* SupervisorMode_VW */

static VERIFY_ELEMENT SupervisorMode_VFY[] =
{                              
  ENTER_KEY,            supervisor_menu,
  CLEAR_KEY,            vfy_clear_key,
  KEYLOCK_NORMAL,       (void *)NULL,
  KEYLOCK_LOCK,         (void *)NULL,
  KEYLOCK_EXCEPTION,    vfy_emul_keylock,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT SupervisorMode_PROC[] =
{
  2,                    EndOfDay,
  KEYLOCK_NORMAL,       proc_init_environment_records,
  UNKNOWN_KEY,          (void *)NULL
};

static INPUT_CONTROLLER Dsupermnu1K1n = {
  (INPUT_DISPLAY *)&oper_menu1,
  KEYBOARD_MASK |
  KEYLOCK_N_MASK | KEYLOCK_X_MASK | KEYLOCK_L_MASK,
  2, 2,
  (VERIFY_KEY *)&numeric
};

static CONTROL_ELEMENT SupervisorMode_CTL[] =
{
  KEYLOCK_NORMAL,       &CashierMode_ST,
  KEYLOCK_LOCK,         &PosStandBy_ST,
  KEYLOCK_EXCEPTION,    &EdpMode_ST,
  EMUL_LOCK_EXCPT,      &EdpMenu_ST,   /* skip the pincd verification */
  1,                    &SvXRCashno_ST,
  2,                    &SupervisorMode_ST,
  3,                    &LogOn_ST,
  4,                    &ChangeMode_ST,
  UNKNOWN_KEY,          &SupervisorMode_ST
};

extern STATE_OBJ SupervisorMode_ST =
{                         
  ST_SUPERVISOR_MODE,
  SupervisorMode_VW,
  no_DFLT,
  &Dsupermnu1K1n,
  SupervisorMode_VFY,
  (void *)NULL,
  SupervisorMode_PROC,
  SupervisorMode_CTL
};


/*-------------------------------------------------------------------------*/
/*                         supervisor_menu                                 */
/*-------------------------------------------------------------------------*/
static short supervisor_menu(_TCHAR *data, short key)
{
  short menu_strings[] = {        /* Refer to index of menu_TXT[x].      */
      1, 3, 6, 7, 0,
  };

  /*                                                                     */
  /* View the operator menu, activate an option if key is within menu-   */
  /* range.                                                              */
  /*                                                                     */

  if( key == UNKNOWN_KEY ) {
      /* Display the menu.                                               */
    scrn_select_window(OPERATOR_WINDOW);
    scrn_string_out(menu_TXT[12], 1, 24);       /* SUPERVISOR MENU       */
    view_menu(4, 24, menu_strings, empty);
    scrn_string_out(menu_TXT[10],13,24);         /* Prompt.      */
    return(key);
  }
  else {
    /* A choice is made, check for legal value.                          */
    if( vfy_range(data,1,4) != SUCCEED ) {
        err_invoke(INVALID_MENU_OPTION);                 /* out of range */
        *data=_T('\0');                                  /* reset buffer */
        return UNKNOWN_KEY;
    }
    else {
      view_menu(4, 24, menu_strings, data);
      return(_ttoi(data));
    }
  }
} /* supervisor_menu */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   SvXRCashno                                            */
/*-------------------------------------------------------------------------*/
static void SvXRCashno_VW(void)
{
  scrn_draw_line(14, 0, 80, _T('_'));       /* draw '_' on row=14 col=0 len=80 */
  scrn_clear_rest_of_line(0, 15, 1);
  scrn_string_out(input_TXT[2], 16, 27);
} /* SvXRCashno_VW */

static VERIFY_ELEMENT SvXRCashno_VFY[] =
{
  ENTER_KEY,            vfy_non_zero,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern PROCESS_ELEMENT SvXRCashno_PROC[] =
{
  ENTER_KEY,            XRead,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT SvXRCashno_CTL[] =
{
  ENTER_KEY,            &SupervisorMode_ST,
  NO_KEY,               &SupervisorMode_ST,
  KEYLOCK_NORMAL,       &SupervisorMode_ST,
  UNKNOWN_KEY,          &SvXRCashno_ST
};

extern STATE_OBJ SvXRCashno_ST =
{                         
  ST_SV_XR_CASHNO,
  SvXRCashno_VW,
  no_DFLT,
  &XRcashid3K3n,
  SvXRCashno_VFY,
  (void *)NULL,
  SvXRCashno_PROC,
  SvXRCashno_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   LogOn                                                 */
/*-------------------------------------------------------------------------*/
static void LogOn_VW(void)
{
  scrn_draw_line(13, 0, 80, _T('_'));
  scrn_string_out(scrn_inv_TXT[19], 15, 24);
  format_display(&dsp_cashier3, empty);
  scrn_string_out(scrn_inv_TXT[20], 17, 24);    /* PIN CODE                */
} /* LogOn_VW */

static VERIFY_ELEMENT LogOn_VFY[] =
{
  ENTER_KEY,            vfy_non_zero,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER DcashK3n =
{
  (INPUT_DISPLAY *)&dsp_cashier3,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  4,
  4,
  (VERIFY_KEY *)&numeric
};


static PROCESS_ELEMENT LogOn_PROC[] =
{
  ENTER_KEY,            proc_show_logon,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT LogOn_CTL[] =
{
  ENTER_KEY,            &SupervisorMode_ST,
  NO_KEY,               &SupervisorMode_ST,
  KEYLOCK_NORMAL,       &SupervisorMode_ST,
  UNKNOWN_KEY,          &LogOn_ST
};

extern STATE_OBJ LogOn_ST =
{                         
  ST_LOG_ON,
  LogOn_VW,
  no_DFLT,
  &DcashK3n,
  LogOn_VFY,
  (void *)NULL,
  LogOn_PROC,
  LogOn_CTL
};


/*-------------------------------------------------------------------------*/
/*                             wait_for_input                              */
/*-------------------------------------------------------------------------*/
void wait_for_input(short keylock_pos)
{
  /* This function returns if a key is pressed or the keylock is NOT in    */
  /* position keylock_pos.                                                 */

  inp_abort_data();

  for (;;) {
    if (inp_data_avail(KEYLOCK_N_MASK|KEYLOCK_X_MASK|
                       KEYLOCK_S_MASK|KEYLOCK_L_MASK)) {
      inp_abort_data();
      break;
    }
    if (rs_keylock_position() != keylock_pos) {
      break;
    }
    if (fn_inp_idle != (void *)NULL) {
      fn_inp_idle();
    }
  }
} /* wait_for_input */


/*-------------------------------------------------------------------------*/
/*                             proc_show_logon                             */
/*-------------------------------------------------------------------------*/
static short proc_show_logon(_TCHAR *data, short key)
{
  _TCHAR pincd_s[5];

  /*                                                                       */
  /* Display the cashier pincode. This is initiated by the supervisor.     */
  /*                                                                       */
  pos_calc_pincd(_ttoi(data),pincd_s, pos_system.run_date);
  scrn_string_out(_T("0000"), 17, 37);
  scrn_string_out(pincd_s, 17, (short)(41-_tcslen(pincd_s)));
  scrn_string_out(input_TXT[40], 20, 24);
  SetShowCursor(FALSE);
  wait_for_input(KEYLOCK_SUPERVISOR);       /* wait for a key or not S-key */
  return key;
} /* proc_show_logon */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   ChangeMode                                            */
/*-------------------------------------------------------------------------*/
static void ChangeMode_VW(void)
{
  scrn_select_window(OPERATOR_WINDOW);
  scrn_draw_line(13, 0, 80, _T('_'));
  scrn_string_out(input_TXT[13], 15, 24);   /* CURRENT MODE:               */
                                            /* RETURN / SALES TILL         */
  scrn_string_out(
      ((pos_system.current_mode == RETURN)? menu_TXT[13]: menu_TXT[14]), 15, 38);
  scrn_string_out(input_TXT[25], 17, 24);   /* SWITCH TO                   */
  scrn_string_out(
      ((pos_system.current_mode == RETURN)? menu_TXT[14]: menu_TXT[13]), 17, 34);
  scrn_string_out(input_TXT[14], 17, 48);   /* Y/N?                        */

  if(pos_system.current_mode ==RETURN) {
    reverse_invoice_active = YES;
  }
  else {
    reverse_invoice_active = NO;
  }

} /* ChangeMode_VW */

static VERIFY_ELEMENT ChangeMode_VFY[] =
{
  ENTER_KEY,            (void *)NULL,
  NO_KEY,               (void *)NULL,
  CLEAR_KEY,            (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT ChangeMode_PROC[] =
{
  ENTER_KEY,            proc_change_current_mode,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT ChangeMode_CTL[] =
{
  ENTER_KEY,            &SupervisorMode_ST,
  NO_KEY,               &SupervisorMode_ST,
  CLEAR_KEY,            &ChangeMode_ST,
  KEYLOCK_NORMAL,       &SupervisorMode_ST,
  UNKNOWN_KEY,          &ChangeMode_ST
};

extern STATE_OBJ ChangeMode_ST =
{                         
  ST_CHANGE_MODE,                       /* a (unique) identifying short    */
  ChangeMode_VW,                        /* a pointer to a view function    */
  no_DFLT,                              /* a pointer to a default function */
  &DMode1K1,                            /* a pointer to an input structure */
  ChangeMode_VFY,                       /* a pointer to a verify table     */
  (void *)NULL,                         /* a pointer to an unview function */
  ChangeMode_PROC,                      /* a pointer to a process table    */
  ChangeMode_CTL                        /* a pointer to a control table    */
};


/*-------------------------------------------------------------------------*/
/*                      proc_change_current_mode                           */
/*-------------------------------------------------------------------------*/
static short proc_change_current_mode(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Toggle the current mode SALES <-> RETURN.                             */
  /*                                                                       */
printf_log("INGRESO A proc_change_current_mode");
  pos_system.current_mode = (pos_system.current_mode==SALES)? RETURN: SALES;

  if ( train_mode == CASH_NORMAL) { /* don't save in train.mode  */
    pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
  }
  
#ifndef NO_VIEW_POS_STATE
  /* set the invoice_line_mode because view_pos_state uses this */
  invoice_line_mode = pos_system.current_mode;
  view_pos_state();
#endif

  return(key);
} /* proc_change_current_mode */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   Training                                              */
/*-------------------------------------------------------------------------*/
static void Training_VW(void)
{
  scrn_draw_line(13, 0, 80, _T('_'));
  scrn_string_out(input_TXT[13], 15, 24);   /* NORMAL / TRAINING TILL      */
  scrn_string_out(
          ((train_mode == 0)? menu_TXT[15]: menu_TXT[16]), 15, 38);
  scrn_string_out(input_TXT[25], 17, 24);   /* SWITCH TO                   */
  scrn_string_out(
          ((train_mode == 0)? menu_TXT[16]: menu_TXT[15]), 17, 34);
  scrn_string_out(input_TXT[14], 17, 48);
} /* Training_VW */

static VERIFY_ELEMENT Training_VFY[] =
{
  ENTER_KEY,            (void *)NULL,
  NO_KEY,               (void *)NULL,
  CLEAR_KEY,            (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT Training_PROC[] =
{
  ENTER_KEY,            proc_change_train_mode,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT Training_CTL[] =
{
  ENTER_KEY,            &EdpMenu_ST,
  NO_KEY,               &EdpMenu_ST,
  CLEAR_KEY,            &Training_ST,
  KEYLOCK_NORMAL,       &EdpMenu_ST,
  UNKNOWN_KEY,          &Training_ST
};

extern STATE_OBJ Training_ST =
{                         
  ST_TRAINING,                          /* a (unique) identifying short    */
  Training_VW,                          /* a pointer to a view function    */
  no_DFLT,                              /* a pointer to a default function */
  &DMode1K1,                            /* a pointer to an input structure */
  Training_VFY,                         /* a pointer to a verify table     */
  (void *)NULL,                         /* a pointer to an unview function */
  Training_PROC,                        /* a pointer to a process table    */
  Training_CTL                          /* a pointer to a control table    */
};
                        

/*-------------------------------------------------------------------------*/
/*                           proc_change_train_mode                        */
/*-------------------------------------------------------------------------*/
static short proc_change_train_mode(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Toggle pos mode: TRAINING <-> NORMAL                                  */
  /*                                                                       */

  if (status_of_pos!=START_OF_DAY) {
    err_invoke(NO_TRAINING);
    key=NO_KEY;
  }
  else {
    if (train_mode==CASH_TRAINING) {
      train_mode=CASH_NORMAL;
      if (GetPrinterSize(selected_printer)!=PRINTER_SIZE_SMALL) {
        bp_now(BP_INVOICE, BP_REVERSE_SKIP, 0);
        bp_now(BP_INVOICE, BP_END_TRAINING, 0);
        bp_now(BP_INVOICE, BP_FORWARD_SKIP, 0);
      }
      pos_read_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
    }
    else {
      train_mode=CASH_TRAINING;
      if (GetPrinterSize(selected_printer)!=PRINTER_SIZE_SMALL) {
        bp_now(BP_INVOICE, BP_REVERSE_SKIP, 0);
        bp_now(BP_INVOICE, BP_START_TRAINING, 0);
        bp_now(BP_INVOICE, BP_FORWARD_SKIP, 0);
      }
    }
#ifndef NO_VIEW_POS_STATE
    view_pos_state();
#endif
  }

  return(key);
} /* proc_change_train_mode */



/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   GeneralVars                                           */
/*-------------------------------------------------------------------------*/
static void GeneralVars_VW(void)
{
  scrn_clear_window(OPERATOR_WINDOW);
  general_menu(empty, UNKNOWN_KEY);         /* initialise menu.            */
} /* GeneralVars_VW */

static VERIFY_ELEMENT GeneralVars_VFY[] =
{                              
  ENTER_KEY,            general_menu,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT GeneralVars_PROC[] =
{
  3,                    proc_amnt_choice,
  4,                    proc_amnt_choice,
  UNKNOWN_KEY,          (void *)NULL
};

static INPUT_CONTROLLER Dopermnu2K1n = {
  (INPUT_DISPLAY *)&oper_menu1,
  KEYBOARD_MASK,
  2, 2,
  (VERIFY_KEY *)&numeric
};

static CONTROL_ELEMENT GeneralVars_CTL[] =
{
  1,                    &FillSysDate_ST,
  2,                    &FillSysTime_ST,
  3,                    &FillGVAmnt_ST,
  4,                    &FillGVAmnt_ST,
  5,                    &FillDefaultMode_ST,
  6,                    &PaymentWays_ST,
  CLEAR_KEY,            &GeneralVars_ST,
  NO_KEY,               &EdpMenu_ST,
  UNKNOWN_KEY,          &GeneralVars_ST
};

extern STATE_OBJ GeneralVars_ST =
{                         
  ST_GENERAL_VARS,
  GeneralVars_VW,
  no_DFLT,
  &Dopermnu2K1n,
  GeneralVars_VFY,
  (void *)NULL,
  GeneralVars_PROC,
  GeneralVars_CTL
};
              
/*-------------------------------------------------------------------------*/
/*                           proc_amnt_choice                              */
/*-------------------------------------------------------------------------*/
static short proc_amnt_choice(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Initialise the global variable which is used to interact between      */
  /* several states.                                                       */
  /*                                                                       */

  last_choice = key;

  return(key);
} /* proc_amnt_choice */


/*-------------------------------------------------------------------------*/
/*                         general_menu                                    */
/*-------------------------------------------------------------------------*/
static short general_menu(_TCHAR *data, short key)
{
  short menu_strings[] = {        /* Refer to index of menu_TXT[x].      */
      19, 20, 21, 22, 23, 24, 0
  };

  /*                                                                     */
  /* View the general menu, activate an option if key is within menu-    */
  /* range.                                                              */
  /*                                                                     */

  if( key == UNKNOWN_KEY ) {
    /* Display the menu.                                                 */
    scrn_select_window(OPERATOR_WINDOW);
    scrn_string_out(menu_TXT[18], 1, 24);
    view_menu(4, 24, menu_strings, empty);
    scrn_string_out(menu_TXT[10], 13, 24);     /* Prompt.        */
    return(key);
  }
  else {
    /* A choice is made, check for legal value.                          */
    if( vfy_range(data,1,6) != SUCCEED ) {
      err_invoke(INVALID_MENU_OPTION);                   /* out of range */
      *data=_T('\0');                                    /* reset buffer */
      return UNKNOWN_KEY;
    }
    else {
      view_menu(4, 24, menu_strings, data);
      return(_ttoi(data));
    }
  }
} /* general_menu */



/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   PaymentWays                                           */
/*-------------------------------------------------------------------------*/
static void PaymentWays_VW(void)
{
  scrn_clear_window(OPERATOR_WINDOW);
  view_c_payment_ways_menu(0);
} /* PaymentWays_VW */

static VERIFY_ELEMENT PaymentWays_VFY[] =
{                              
  ENTER_KEY,            vfy_menu_option10,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER Dopermnu2K2n =
{
  (INPUT_DISPLAY *)&oper_menu2,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  3,
  3,
  (VERIFY_KEY *)&numeric
};


static PROCESS_ELEMENT PaymentWays_PROC[] =
{
  ENTER_KEY,            proc_paym_cd_choice,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT PaymentWays_CTL[] =
{
  ENTER_KEY,            &PaymAmount_ST,
  CLEAR_KEY,            &PaymentWays_ST,
  NO_KEY,               &GeneralVars_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &PaymentWays_ST
};

extern STATE_OBJ PaymentWays_ST =
{                         
  ST_PAYMENT_WAYS,
  PaymentWays_VW,
  no_DFLT,
  &Dopermnu2K2n,
  PaymentWays_VFY,
  (void *)NULL,
  PaymentWays_PROC,
  PaymentWays_CTL
};

/*-------------------------------------------------------------------------*/
/*                         view_c_payment_ways_menu                        */
/*-------------------------------------------------------------------------*/
static void view_c_payment_ways_menu(short activate)
{
  /* View the Payment ways menu, if (activate) then invert menu option     */
  /* last chosen.                                                          */
  short i;
  PAYMENT_DEF paym;
  _TCHAR buf[20];
  _TCHAR buffer[83];

  scrn_string_out(menu_TXT[17], 1, 24); /* PAYMENT WAYS MENU               */
  scrn_string_out(menu_TXT[25], 3,  3); /* Header text                     */
  for (i = 1; i <= 10; i++) {
    ch_memset(buffer, _T(' '), sizeof(buffer));
    paym.paym_cd = (i==10) ? 0 : i;
    get_paym(&paym);
    if(activate && i==last_choice) {
      buffer[0]=_T('\1');                                   /* inverse-video   */
    }
    else {
      buffer[0]=_T('\2');                                   /* normal-video    */
    }
    _itot(i, buf, 10);
    _tcscat(buf, _T("."));
    fmt_right_justify_string(buffer+1,0,2,buf);
    strcpy_no_null(buffer+5, paym.paym_descr);
    ftoa_price(paym.paym_limit, 18, buf);
    format_string(&string_price19, buf);
    strcpy_no_null(buffer+21, string_price19.result);

    ftoa(paym.extra_perc*100, 6, buf);
    format_string(&string_perc, buf);
    strcpy_no_null(buffer+42, string_perc.result);

    ftoa_price(paym.min_extra_amount, 18, buf);
    format_string(&string_price8, buf);
    strcpy_no_null(buffer+52, string_price8.result);

    if (*paym.paym_type) {
      buffer[69] = *paym.paym_type;
    }
    buffer[70]=_T('\0');

    //_tcscat(buffer, _T("\002"));
    scrn_string_out(buffer ,(short)(4+i), 3);
  }

  /* Display prompt only if no active option.                              */
  if (!activate) {
    scrn_string_out(menu_TXT[10], 16, 24);
  }
} /* view_c_payment_ways_menu */


/*-------------------------------------------------------------------------*/
/*                          proc_paym_cd_choice                            */
/*-------------------------------------------------------------------------*/
static short proc_paym_cd_choice(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Initialise the global variable which is used to interact between      */
  /* several states.                                                       */
  /*                                                                       */

  last_choice = _ttoi(data);

  return(key);
} /* proc_paym_cd_choice */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   PaymAmount                                            */
/*-------------------------------------------------------------------------*/
static void PaymAmount_VW(void)
{
  view_c_payment_ways_menu(1);          /* high lite chosen option         */
  scrn_draw_line(16,0,80,_T('_'));          /* draw '_' on row=14 col=0 len=80 */
  scrn_string_out(input_TXT[15], 18, 7);    /* NEW LIMIT                   */
  format_display(&dsp_float13p2k2, empty);
  format_display(&dsp_perc, empty);
  format_display(&dsp_min_extra, empty);
  format_display(&dsp_type, empty);
} /* PaymAmount_VW */

static VERIFY_ELEMENT PaymAmount_VFY[] =
{
  ENTER_KEY,            vfy_non_zero,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER Dfloat15K15n =
{
  (INPUT_DISPLAY *)&dsp_float13p2k2,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  16,
  16,
  (VERIFY_KEY *)&numeric_dnull
};

static PROCESS_ELEMENT PaymAmount_PROC[] =
{
  ENTER_KEY,            proc_paym_amount,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT PaymAmount_CTL[] =
{
  ENTER_KEY,            &PaymExtraPerc_ST,
  CLEAR_KEY,            &PaymAmount_ST,
  NO_KEY,               &PaymentWays_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &PaymAmount_ST
};

extern STATE_OBJ PaymAmount_ST =
{                         
  ST_PAYM_AMOUNT,
  PaymAmount_VW,
  no_DFLT,
  &Dfloat15K15n,
  PaymAmount_VFY,
  (void *)NULL,
  PaymAmount_PROC,
  PaymAmount_CTL
};


/*-------------------------------------------------------------------------*/
/*                          proc_paym_amount                               */
/*-------------------------------------------------------------------------*/
static short proc_paym_amount(_TCHAR *data, short key)
{
  PAYMENT_DEF paym;
  /*                                                                       */
  /* Save entered payment amount.                                          */
  /*                                                                       */

  paym.paym_cd = (last_choice==10) ? 0 : last_choice;
  get_paym(&paym);                       /* needed for payment description */
  paym.paym_limit = atof_price(data);
  put_paym(&paym);

  return(key);
} /* proc_paym_amount */



/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   PaymExtraPerc                                         */
/*-------------------------------------------------------------------------*/
static short vfy_less_100(_TCHAR *data, short key)
{
  if (_tcstod(data, NULL) <= (double)10000) {    /* 100.00 */
    return key;
  }
  *data = _T('\0');
  err_invoke(INVALID_KEY_ERROR);

  return(UNKNOWN_KEY);
} /* vfy_less_100 */


static void PaymExtraPerc_VW(void)
{
  view_c_payment_ways_menu(1);          /* high lite chosen option         */
  format_display(&dsp_perc, empty);
} /* PaymExtraPerc_VW */

static VERIFY_ELEMENT PaymExtraPerc_VFY[] =
{
  ENTER_KEY,            vfy_less_100,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER DExtraPerc =
{
  (INPUT_DISPLAY *)&dsp_perc,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  6,
  6,
  (VERIFY_KEY *)&numeric_dnull
};

static PROCESS_ELEMENT PaymExtraPerc_PROC[] =
{
  ENTER_KEY,            proc_extra_perc,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT PaymExtraPerc_CTL[] =
{
  ENTER_KEY,            &PaymExtraMin_ST,
  CLEAR_KEY,            &PaymExtraPerc_ST,
  NO_KEY,               &PaymentWays_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &PaymExtraPerc_ST
};

extern STATE_OBJ PaymExtraPerc_ST =
{                         
  ST_EXTRA_PERC,
  PaymExtraPerc_VW,
  no_DFLT,
  &DExtraPerc,
  PaymExtraPerc_VFY,
  (void *)NULL,
  PaymExtraPerc_PROC,
  PaymExtraPerc_CTL
};


/*-------------------------------------------------------------------------*/
/*                          proc_extra_perc                                */
/*-------------------------------------------------------------------------*/
static short proc_extra_perc(_TCHAR *data, short key)
{
  PAYMENT_DEF paym;

  /*                                                                       */
  /* Save entered payment extra percentage.                                */
  /*                                                                       */

  paym.paym_cd=last_choice%10;
  get_paym(&paym);                       /* needed for payment description */
  paym.extra_perc = (double)(_tcstod(data, NULL)/100.0);
  put_paym(&paym);

  return(key);
} /* proc_extra_perc */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   PaymExtraMin                                          */
/*-------------------------------------------------------------------------*/
static void PaymExtraMin_VW(void)
{
  view_c_payment_ways_menu(1);          /* high lite chosen option         */
  format_display(&dsp_min_extra, empty);
} /* PaymExtraMin_VW */

static VERIFY_ELEMENT PaymExtraMin_VFY[] =
{
  ENTER_KEY,            (void *)NULL,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER DExtraMin =
{
  (INPUT_DISPLAY *)&dsp_min_extra,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  9,
  9,
  (VERIFY_KEY *)&numeric_dnull
};

static PROCESS_ELEMENT PaymExtraMin_PROC[] =
{
  ENTER_KEY,            proc_extra_min,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT PaymExtraMin_CTL[] =
{
  ENTER_KEY,            &PaymType_ST,
  CLEAR_KEY,            &PaymExtraMin_ST,
  NO_KEY,               &PaymentWays_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &PaymExtraMin_ST
};

extern STATE_OBJ PaymExtraMin_ST =
{                         
  ST_EXTRA_MIN,
  PaymExtraMin_VW,
  no_DFLT,
  &DExtraMin,
  PaymExtraMin_VFY,
  (void *)NULL,
  PaymExtraMin_PROC,
  PaymExtraMin_CTL
};


/*-------------------------------------------------------------------------*/
/*                          proc_extra_min                                 */
/*-------------------------------------------------------------------------*/
static short proc_extra_min(_TCHAR *data, short key)
{
  PAYMENT_DEF paym;
  /*                                                                       */
  /* Save entered minimum extra amount.                                    */
  /*                                                                       */

  paym.paym_cd=last_choice%10;
  get_paym(&paym);                       /* needed for payment description */
  paym.min_extra_amount=atof_price(data);
  put_paym(&paym);

  return(key);
} /* proc_extra_min */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   PaymExtraVat                                          */
/*-------------------------------------------------------------------------*/
static short vfy_less_10(_TCHAR *data, short key)
{
  if (_ttoi(data) < 10) {
    return(key);
  }
  *data=_T('\0');
  err_invoke(INVALID_KEY_ERROR);

  return(UNKNOWN_KEY);
} /* vfy_less_10 */


static void PaymExtraVat_VW(void)
{
  view_c_payment_ways_menu(1);          /* high lite chosen option         */
  format_display(&dsp_vat, empty);
} /* PaymExtraVat_VW */

static VERIFY_ELEMENT PaymExtraVat_VFY[] =
{
  ENTER_KEY,            vfy_less_10,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER DExtraVat =
{
  (INPUT_DISPLAY *)&dsp_vat,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  2,
  2,
  (VERIFY_KEY *)&numeric
};

static PROCESS_ELEMENT PaymExtraVat_PROC[] =
{
  ENTER_KEY,            proc_extra_vat,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT PaymExtraVat_CTL[] =
{
  ENTER_KEY,            &PaymType_ST,
  CLEAR_KEY,            &PaymExtraVat_ST,
  NO_KEY,               &PaymentWays_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &PaymExtraVat_ST
};

extern STATE_OBJ PaymExtraVat_ST =
{                         
  ST_EXTRA_VAT,
  PaymExtraVat_VW,
  no_DFLT,
  &DExtraVat,
  PaymExtraVat_VFY,
  (void *)NULL,
  PaymExtraVat_PROC,
  PaymExtraVat_CTL
};


/*-------------------------------------------------------------------------*/
/*                          proc_extra_vat                                 */
/*-------------------------------------------------------------------------*/
static short proc_extra_vat(_TCHAR *data, short key)
{
  PAYMENT_DEF paym;
  /*                                                                       */
  /* Save entered extra vat code.                                          */
  /*                                                                       */

  paym.paym_cd=last_choice%10;
  get_paym(&paym);                       /* needed for payment description */
  paym.vat_no=(short)_ttoi(data);
  put_paym(&paym);

  return(key);
} /* proc_extra_min */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   PaymType                                              */
/*-------------------------------------------------------------------------*/
static void PaymType_VW(void)
{
  view_c_payment_ways_menu(1);          /* high lite chosen option         */
  format_display(&dsp_type, empty);
} /* PaymType_VW */

static VERIFY_ELEMENT PaymType_VFY[] =
{
  ENTER_KEY,            (void *)NULL,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER DPaymType =
{
  (INPUT_DISPLAY *)&dsp_type,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  2,
  2,
  (VERIFY_KEY *)&numeric
};

static PROCESS_ELEMENT PaymType_PROC[] =
{
  ENTER_KEY,            proc_paym_type,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT PaymType_CTL[] =
{
  ENTER_KEY,            &PaymentWays_ST,
  CLEAR_KEY,            &PaymType_ST,
  NO_KEY,               &PaymentWays_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &PaymType_ST
};

extern STATE_OBJ PaymType_ST =
{                         
  ST_PAYM_TYPE,
  PaymType_VW,
  no_DFLT,
  &DPaymType,
  PaymType_VFY,
  (void *)NULL,
  PaymType_PROC,
  PaymType_CTL
};


/*-------------------------------------------------------------------------*/
/*                          proc_paym_type                                 */
/*-------------------------------------------------------------------------*/
static short proc_paym_type(_TCHAR *data, short key)
{
  PAYMENT_DEF paym;
  /*                                                                       */
  /* Save entered extra vat code.                                          */
  /*                                                                       */

  paym.paym_cd=last_choice%10;
  get_paym(&paym);                       /* needed for payment description */

  if (*data) {
    _tcscpy(paym.paym_type, data);
  }
  else {
    _tcscpy(paym.paym_type, _T("0"));
  }

  put_paym(&paym);

  return(key);
} /* proc_paym_type */

                   
/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE    FillSysDate                                          */
/*-------------------------------------------------------------------------*/
static void FillSysDate_VW(void)
{
  _TCHAR date_buf[10];                    /* for date in format YYYYMMDD     */

  scrn_draw_line(13, 0, 80, _T('_'));
  scrn_string_out(input_TXT[16], 15, 24);   /* CURRENT DATE :              */
  _ltot(pos_system.run_date, date_buf, 10);
  scrn_string_out(date_buf, 15, 39);
  scrn_string_out(input_TXT[17], 15, 48);   /* (FORMAT YYYYMMDD)           */
  scrn_string_out(input_TXT[18], 17, 24);   /* NEW DATE     :              */
  format_display(&dsp_dateU, empty);
} /* FillSysDate_VW */

static VERIFY_ELEMENT FillSysDate_VFY[] =
{
  ENTER_KEY,            vfy_date,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER DdateKn =
{
  (INPUT_DISPLAY *)&dsp_dateU,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  9,
  9,
  (VERIFY_KEY *)&numeric
};


static PROCESS_ELEMENT FillSysDate_PROC[] =
{
  ENTER_KEY,            proc_set_date,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT FillSysDate_CTL[] =
{
  ENTER_KEY,            &GeneralVars_ST,
  NO_KEY,               &GeneralVars_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &FillSysDate_ST
};

extern STATE_OBJ FillSysDate_ST =
{                         
  ST_FILL_SYS_DATE,
  FillSysDate_VW,
  no_DFLT,
  &DdateKn,
  FillSysDate_VFY,
  (void *)NULL,
  FillSysDate_PROC,
  FillSysDate_CTL
};

/*-------------------------------------------------------------------------*/
/*                              proc_set_date                              */
/*-------------------------------------------------------------------------*/
/* Set the system clock to string 'data' (format YYYYMMDD).                */
/*-------------------------------------------------------------------------*/
short proc_set_date(_TCHAR *data, short key)
{
  BOOL status;
  long date;

  date=_ttol(data);
  status = set_system_date(date);
  pos_system.run_date = date;
  if( train_mode == CASH_NORMAL) { /* don't save in train.mode  */
    pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
  }

  return(key);
} /* proc_set_date */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE    FillSysTime                                          */
/*-------------------------------------------------------------------------*/
static void FillSysTime_VW(void)
{
  _TCHAR time_buf[6];

  scrn_draw_line(13, 0, 80, _T('_'));
  scrn_string_out(input_TXT[19], 15, 24);   /* CURRENT TIME :              */
  get_current_time(time_buf);
  format_string(&string_time5, time_buf);
  scrn_string_out(string_time5.result, 15, 39);
  scrn_string_out(input_TXT[20], 17, 24);   /* NEW TIME     :              */
  format_display(&dsp_time4, empty);
} /* FillSysTime_VW */

static VERIFY_ELEMENT FillSysTime_VFY[] =
{
  ENTER_KEY,            vfy_time,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER DtimeK4n =
{
  (INPUT_DISPLAY *)&dsp_time4,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  5,
  5,
  (VERIFY_KEY *)&numeric
};
   
static PROCESS_ELEMENT FillSysTime_PROC[] =
{
  ENTER_KEY,            proc_set_time,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT FillSysTime_CTL[] =
{
  ENTER_KEY,            &GeneralVars_ST,
  NO_KEY,               &GeneralVars_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &FillSysTime_ST
};

extern STATE_OBJ FillSysTime_ST =
{                         
  ST_FILL_SYS_TIME,                     /* a (unique) identifying short    */
  FillSysTime_VW,                       /* a pointer to a view function    */
  no_DFLT,                              /* a pointer to a default function */
  &DtimeK4n,                            /* a pointer to an input structure */
  FillSysTime_VFY,                      /* a pointer to a verify table     */
  (void *)NULL,                         /* a pointer to an unview function */
  FillSysTime_PROC,                     /* a pointer to a process table    */
  FillSysTime_CTL                       /* a pointer to a control table    */
};


/*-------------------------------------------------------------------------*/
/*                            proc_set_time                                */
/*-------------------------------------------------------------------------*/
/* Initialise the system clock.                                            */
/*-------------------------------------------------------------------------*/
static short proc_set_time(_TCHAR *data, short key)
{
  BOOL status;

  status = set_system_time(_ttoi(data));

  return(key);
} /* proc_set_time */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE    FillGVAmnt                                           */
/*-------------------------------------------------------------------------*/
static void FillGVAmnt_VW(void)
{
  _TCHAR buf[20];

  scrn_draw_line(13, 0, 80, _T('_'));
  switch (last_choice) {
    case 3:
      scrn_string_out(input_TXT[21], 15, 7);   /* CURRENT DEF FLOAT .*/
      ftoa_price(genvar.def_fl_nm, 18, buf);
      break;
    case 4:
      scrn_string_out(input_TXT[22], 15, 7);   /* CURRENT DEFAULT .. */
      ftoa_price(genvar.def_fl_rt, 18, buf);
      break;
    default:
      break;
  }
  format_string(&string_price19, buf);
  scrn_string_out(string_price19.result, 15, 42);

  scrn_string_out(input_TXT[23], 17, 24);             /* NEW AMOUNT:       */
  format_display(&dsp_float13p2k3, empty);
} /* FillGVAmnt_VW */

static VERIFY_ELEMENT FillGVAmnt_VFY[] =
{
  ENTER_KEY,            vfy_non_zero,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER Dfloat15K15n2 =
{
  (INPUT_DISPLAY *)&dsp_float13p2k3,
  KEYBOARD_MASK | KEYLOCK_N_MASK,
  16,
  16,
  (VERIFY_KEY *)&numeric_dnull
};

static PROCESS_ELEMENT FillGVAmnt_PROC[] =
{
  ENTER_KEY,            proc_genvar_amount,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT FillGVAmnt_CTL[] =
{
  ENTER_KEY,            &GeneralVars_ST,
  CLEAR_KEY,            &FillGVAmnt_ST,
  NO_KEY,               &GeneralVars_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &FillGVAmnt_ST
};

extern STATE_OBJ FillGVAmnt_ST =
{                         
  ST_FILL_GV_AMNT,
  FillGVAmnt_VW,
  no_DFLT,
  &Dfloat15K15n2,
  FillGVAmnt_VFY,
  (void *)NULL,
  FillGVAmnt_PROC,
  FillGVAmnt_CTL
};

/*-------------------------------------------------------------------------*/
/*                            proc_genvar_amount                           */
/*-------------------------------------------------------------------------*/
static short proc_genvar_amount(_TCHAR *data, short key)
{
  double res;

  /*                                                                       */
  /* Save entered amount.                                                  */
  /*                                                                       */

  res=atof_price(data);
  switch (last_choice) {
    case 3:
      genvar.def_fl_nm = res;
      break;
    case 4:
      genvar.def_fl_rt = res;
      break;
    default:
      break;
  }
  pos_upda_genvar(POS_GENV_SIZE, GENVAR_FNO, &genvar);

  return(key);
} /* proc_genvar_amount */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE    FillDefaultMode                                      */
/*-------------------------------------------------------------------------*/
static void FillDefaultMode_VW(void)
{
  scrn_draw_line(13, 0, 80, _T('_'));
  scrn_string_out(input_TXT[24], 15, 24);       /* CURRENT DEFAULT MODE:   */
  if (pos_system.default_mode == RETURN) {
    scrn_string_out(menu_TXT[13] , 15, 46);
    scrn_string_out(input_TXT[25], 17, 24);     /* SWITCH TO               */
    scrn_string_out(menu_TXT[14] , 17, 34);
	reverse_invoice_active = YES;
  }
  else {
    scrn_string_out(menu_TXT[14] , 15, 46);
    scrn_string_out(input_TXT[25], 17, 24);     /* SWITCH TO               */
    scrn_string_out(menu_TXT[13] , 17, 34);
  }
  scrn_string_out(input_TXT[14], 17, 48);       /* Y/N?                    */
} /* FillDefaultMode_VW */

static VERIFY_ELEMENT FillDefaultMode_VFY[] =
{
  ENTER_KEY,            (void *)NULL,
  CLEAR_KEY,            (void *)NULL,
  NO_KEY,               (void *)NULL,
  KEYLOCK_NORMAL,       (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT FillDefaultMode_PROC[] =
{
  ENTER_KEY,            proc_change_default_mode,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT FillDefaultMode_CTL[] =
{
  ENTER_KEY,            &GeneralVars_ST,
  NO_KEY,               &GeneralVars_ST,
  CLEAR_KEY,            &FillDefaultMode_ST,
  KEYLOCK_NORMAL,       &GeneralVars_ST,
  UNKNOWN_KEY,          &FillDefaultMode_ST
};

extern STATE_OBJ FillDefaultMode_ST =
{                         
  ST_FILL_DEFAULT_MODE,                 /* a (unique) identifying short    */
  FillDefaultMode_VW,                   /* a pointer to a view function    */
  no_DFLT,                              /* a pointer to a default function */
  &DMode1K1,                            /* a pointer to an input structure */
  FillDefaultMode_VFY,                  /* a pointer to a verify table     */
  (void *)NULL,                         /* a pointer to an unview function */
  FillDefaultMode_PROC,                 /* a pointer to a process table    */
  FillDefaultMode_CTL                   /* a pointer to a control table    */
};


/*-------------------------------------------------------------------------*/
/*                            proc_change_default_mode                     */
/*-------------------------------------------------------------------------*/
static short proc_change_default_mode(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Toggle the pos's default mode SALES <-> RETURN                        */
  /*                                                                       */

  if (pos_system.default_mode==SALES) {
    pos_system.default_mode=RETURN;
	reverse_invoice_active = YES;
  }
  else {
    pos_system.default_mode=SALES;
	reverse_invoice_active = NO;
  }
  pos_system.current_mode=pos_system.default_mode;

  if (train_mode == CASH_NORMAL) {            /* don't save in train.mode  */
    pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
  }

  /* Handling the default cashdrawer amount... */
  invoice_mode = pos_system.current_mode;
  /* ...and the Status indicator               */
  invoice_line_mode = invoice_mode;

#ifndef NO_VIEW_POS_STATE
  view_pos_state();
#endif

  return(key);
} /* proc_change_default_mode */
