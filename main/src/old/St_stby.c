/*
 *     Module Name       : ST_STBY.C
 *
 *     Type              : States PosStandBy, EdpMode
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
 * 06-Jun-2001 Also do init_environment_records when 
 *             there was a previous error in init_environment_records  R.N.B.
 * --------------------------------------------------------------------------
 * 14-Jun-2001 Bugfix in init_environment_records                      P.M.
 * --------------------------------------------------------------------------
 * 19-Jul-2001 When changing the keylock to CashierMode_ST the function
 *             proc_init_environment_records is called                 J.D.M.
 * --------------------------------------------------------------------------
 * 24-Apr-2002 Implemented use of Version_mgr instead of linkdate      J.D.M.
 * --------------------------------------------------------------------------
 * 03-May-2002 Connection termination when till is not defined in the
 *             netnodes                                                J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>

#include "stnetp24.h"                       /* Pos (library) include files   */
#include "comm_tls.h"
#include "intrface.h"
#include "stri_tls.h"
#include "misc_tls.h"

#include "tm_mgr.h"                         /* Toolsset include files.       */
#include "inp_mgr.h"
#include "state.h"
#include "scrn_mgr.h"
#include "err_mgr.h"
#include "state.h"
#include "mapkeys.h"

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
#include "pos_com.h"
#include "st_main.h"
#include "condll.h"
#include "rmml.h"
#include "Pos_msam.h"
#include "Version_mgr.h"

static short vfy_n_key(_TCHAR *, short);
static short edp_menu(_TCHAR *, short);

/*---------------------------------------------------------------------------*/
/*                               view_lan_state()                            */
/*---------------------------------------------------------------------------*/
void view_lan_state(short update)
{  
  static short   last_lan_status = -1;        /* force update the first time */
  short          current_lan_status;
  unsigned short save_window;
  
  if (last_lan_status == -2) {
    return;  /* waiting for err_invoke! */
  }

  current_lan_status = get_bo_status();

  if (current_lan_status != last_lan_status ||
      update == FORCED_UPDATE) {
    save_window = scrn_get_current_window();
    scrn_select_window(LAN_STATUS_WINDOW);

    switch (current_lan_status) {
      case CONN:
        scrn_string_out(scrn_inv_TXT[3],0,0);  /* ONLINE  */
        break;
      case CFGERR:
        scrn_string_out(scrn_inv_TXT[5],0,0);  /* CFG_ERR */
        if (current_lan_status != last_lan_status) {
          last_lan_status = -2;
          if (err_invoke(NET_CONFIG_ERROR) == -1) {
            last_lan_status = -1;
            scrn_select_window(save_window);
            return;
          }
        }
        break;
      default: /* GONE */
        scrn_string_out(scrn_inv_TXT[4],0,0);  /* OFFLINE */
        break;
    }

    scrn_select_window(save_window);
    last_lan_status = current_lan_status;
  }

  return;
} /* view_lan_state */

/*---------------------------------------------------------------------------*/
/*                               view_pos_state()                            */
/*---------------------------------------------------------------------------*/
void view_pos_state(void)
{
  /*                                                                       */
  /* POS state indicating wether in NORMAL/TRAINING and SALES/RETURN mode. */
  /*                                                                       */
  /* In case the next customer has the invoice mode toggled, the mode of   */
  /* the next customer is displayed.                                       */
  /*                                                                       */

  unsigned short save_window = scrn_get_current_window();
  _TCHAR         buffer[4]   = _T("   ");
  short          mode        = invoice_line_mode;

  if (state_number() == ST_CUSTOMER_MODE) {
    if (next_invoice_mode != -1) {
      mode = next_invoice_mode;      
    }
    else {
      mode = pos_system.current_mode;
    }
  }

  if (mode == RETURN) {
    *buffer = *scrn_inv_TXT[1];
  }
  if (train_mode == CASH_TRAINING) {
    *(buffer+2) = *scrn_inv_TXT[2];
  }

  scrn_select_window(POS_STATUS_WINDOW);
  scrn_string_out(buffer,0,0);
  scrn_select_window(save_window);

  return;
} /* view_pos_state */

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   STANDBY MODE                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* The first state to execute by the state-engine.                           */
/* Also the POS 'inactive' state.                                            */
/*                                                                           */
/*   - At this point the standby-mode is active until one of the key's is    */
/*     turned to a non-LOCK position.                                        */
/*   - In standby-mode, LAN processing is active;                            */
/*        . Sending, not yet sended invoices,                                */
/*        . Receiving PLU-updates and other updates which are send by BO.    */
/*        . more?                                                            */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void Basic_VW(void)
{
  _TCHAR buffer[10];

  scrn_select_window(CASHIER_LOGON_WINDOW);

  _itot(pos_system.till_no, buffer, DECIMAL);
  scrn_string_out(scrn_inv_TXT[23],0,0);  /* NO:                           */
  scrn_string_out(buffer,0,4);

  scrn_string_out(
          scrn_inv_TXT[(pos_system.current_mode == SALES)? 15: 16], 0, 24);
} /* Basic_VW */


void PosStandBy_VW(void)
{
  _TCHAR buffer[80];
  
  cdsp_closed_till(0, TRUE);          /* Display CLOSED, time and promotxt */
  cls();

  Basic_VW();                     /* Basic screen layout in standby-mode.  */

  if (status_of_pos == START_OF_DAY) {
    scrn_string_out(scrn_inv_TXT[17],5,24); /* START OF DAY      */
  }
  else {
    scrn_string_out(scrn_inv_TXT[18],5,24); /* START OF SHIFT    */
  }

  /* Version by number of time and date included */
  _stprintf(buffer, scrn_inv_TXT[26], get_version_no(),
                                      get_compile_date(),
                                      compile_time, (PERCEPTION_ENABLED?"CON PERCEPCION":"<SIN PERCEPCION>"));
  scrn_string_out(buffer,21,1);                       /* VERSION */

  SetShowCursor(FALSE);
} /* PosStandBy_VW */


static void PosStandBy_UVW(void)
{
  /* Nothing to do yet */
  return;
} /* PosStandBy_UVW */


static VERIFY_ELEMENT PosStandBy_VFY[] =
{                              
  KEYLOCK_NORMAL,     vfy_n_key,
  KEYLOCK_SUPERVISOR, vfy_emul_keylock,
  KEYLOCK_EXCEPTION,  vfy_emul_keylock,
  OPEN_DRAWER_KEY,    open_and_close_drawer,  /* Approval in state-engine.  */
  UNKNOWN_KEY,        illegal_fn_key
};

extern INPUT_CONTROLLER DstK1nLlnsU =
{
  (INPUT_DISPLAY *)&dsp_keypos,
  KEYLOCK_S_MASK | KEYLOCK_N_MASK | KEYLOCK_X_MASK,
  1, 1,
  (VERIFY_KEY *)&numeric
};

static PROCESS_ELEMENT PosStandBy_PROC[] =
{
  KEYLOCK_NORMAL, proc_init_environment_records,
  UNKNOWN_KEY,    (void *)NULL,
};

static CONTROL_ELEMENT PosStandBy_CTL[] =
{
  KEYLOCK_SUPERVISOR, &SoMode_ST,
  KEYLOCK_NORMAL,     &CashierMode_ST,
  KEYLOCK_EXCEPTION,  &EdpMode_ST,
  EMUL_LOCK_EXCPT,    &EdpMenu_ST,        /* skip the pincd state */
  EMUL_LOCK_SUPER,    &SupervisorMode_ST, /* skip the pincd state */
  UNKNOWN_KEY,        &PosStandBy_ST
};


extern STATE_OBJ PosStandBy_ST =
{                         
  ST_POS_STAND_BY,
  PosStandBy_VW,
  no_DFLT_2,
  &DstK1nLlnsU,
  PosStandBy_VFY,
  PosStandBy_UVW,
  PosStandBy_PROC,
  PosStandBy_CTL
};


/*---------------------------------------------------------------------------*/
/*                               vfy_n_key                                   */
/*---------------------------------------------------------------------------*/
static short vfy_n_key(_TCHAR *data, short key)
{
  scrn_select_window(CASHIER_LOGON_WINDOW);
  return(key);
} /* vfy_n_key */

/*---------------------------------------------------------------------------*/
/*                      proc_init_environment_records                        */
/*---------------------------------------------------------------------------*/
short proc_init_environment_records(_TCHAR *data, short key)
{
  _TCHAR buffer[5];
  short status;
  static short busy = FALSE;

  /*                                                                       */
  /* At start of day, initialise:                                          */
  /*                                                                       */
  /*    - pos_system                                                       */
  /*    - system date                                                      */
  /*    - genvar                                                           */
  /*    - payment ways / limits                                            */
  /*    - tax percentage                                                   */
  /*                                                                       */

  if (busy == TRUE) {
	  /* err_invoke will also call this function via    */
	  /* application_inp_idle. In this case a recursive */
	  /* loop will be created with a crash as result.   */
    return(key);
  }
  busy = TRUE;
  
  if (status_of_pos==START_OF_DAY || err_init_environment==TRUE) {
    display_working(YES);
    if ((status=init_environment_records(!NO_WINDOW, NULL)) != SUCCEED) {
      _itot(status, buffer, DECIMAL);
      error_extra_msg=buffer;
      err_invoke(INIT_ENVIRONMENT_ERROR);
    }
    setup_inp_mgr_price_fmt(genvar.ind_price_dec);
    display_working(NO);
  }

  multisam_discounts(TRUE);      /* check discount actions for syntax errors */
  busy = FALSE;

  return(key);
} /* proc_init_environment_records */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   EDP MODE                                               */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Via this state the EDP menu is invoked, after entering the EDP-pincode.   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void EdpMode_VW(void)
{
  static short no_cls_states[]={
       ST_POS_STAND_BY
      ,0
  };

  if (called_by_state(no_cls_states) == FAIL) {
    PosStandBy_VW();
  }
  scrn_string_out(input_TXT[39], 7, 24);    /* ENTER EDP-PINCODE:            */
  SetShowCursor(TRUE);
} /* EdpMode_VW */

static VERIFY_ELEMENT EdpMode_VFY[] =
{                              
  ENTER_KEY,          vfy_edp_pincd,
  CLEAR_KEY,          vfy_clear_key,
  KEYLOCK_LOCK,       (void *)NULL,
  KEYLOCK_NORMAL,     proc_init_environment_records,
  KEYLOCK_SUPERVISOR, (void *)NULL,
  OPEN_DRAWER_KEY,    open_and_close_drawer,  /* Approval in state-engine.   */
  UNKNOWN_KEY,        illegal_fn_key
};

extern INPUT_CONTROLLER DpinK4nLln1 =
{
  (INPUT_DISPLAY *)&dsp_1pincd4,
  KEYBOARD_MASK | KEYLOCK_L_MASK | KEYLOCK_N_MASK | KEYLOCK_S_MASK,
  5,
  5,
  (VERIFY_KEY *)&numeric
};

static CONTROL_ELEMENT EdpMode_CTL[] =
{
  ENTER_KEY,          &EdpMenu_ST,
  KEYLOCK_LOCK,       &PosStandBy_ST,
  KEYLOCK_NORMAL,     &CashierMode_ST,
  KEYLOCK_SUPERVISOR, &SoMode_ST,
  UNKNOWN_KEY,        &EdpMode_ST
};

extern STATE_OBJ EdpMode_ST =
{                         
  ST_EDP_MODE,
  EdpMode_VW,
  no_DFLT,
  &DpinK4nLln1,
  EdpMode_VFY,
  (void *)NULL,                         /* a pointer to an unview function */
  (void *)NULL,                         /* a pointer to a process table    */
  EdpMode_CTL
};


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE  EdpMenu                                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* EDP menu including exit the program.                                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void EdpMenu_VW(void)
{
  scrn_clear_window(OPERATOR_WINDOW);
  edp_menu(empty, 0);                      /* Initialise the menu.         */
}

static VERIFY_ELEMENT EdpMenu_VFY[] =
{                              
  ENTER_KEY,            edp_menu,
  CLEAR_KEY,            vfy_clear_key,
  KEYLOCK_NORMAL,       (void *)NULL,
  KEYLOCK_LOCK,         (void *)NULL,
  KEYLOCK_SUPERVISOR,   vfy_emul_keylock,
  OPEN_DRAWER_KEY,      open_and_close_drawer, /* Approval in state-engine.*/
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT EdpMenu_PROC[] =
{
  KEYLOCK_NORMAL,       proc_init_environment_records,
  UNKNOWN_KEY,          (void *)NULL
};

static INPUT_CONTROLLER Dsupermnu1K1n = {
  (INPUT_DISPLAY *)&oper_menu1,
  KEYBOARD_MASK |
  KEYLOCK_N_MASK | KEYLOCK_S_MASK | KEYLOCK_L_MASK,
  2, 2,
  (VERIFY_KEY *)&numeric
};

static CONTROL_ELEMENT EdpMenu_CTL[] =
{
  KEYLOCK_NORMAL,       &CashierMode_ST,
  KEYLOCK_LOCK,         &PosStandBy_ST,
  KEYLOCK_SUPERVISOR,   &SoMode_ST,
  EMUL_LOCK_SUPER,      &SupervisorMode_ST,        /* skip the pincd state */
  1,                    (void *)NULL,                      /* exit program */
  2,                    &GeneralVars_ST,
  3,                    &Training_ST,
  UNKNOWN_KEY,          &EdpMenu_ST
};

extern STATE_OBJ EdpMenu_ST =
{                         
  ST_EDP_MENU,
  EdpMenu_VW,
  no_DFLT,
  &Dsupermnu1K1n,
  EdpMenu_VFY,
  (void *)NULL,
  EdpMenu_PROC,
  EdpMenu_CTL
};


/*---------------------------------------------------------------------------*/
/*                         edp_menu                                          */
/*---------------------------------------------------------------------------*/
static short edp_menu(_TCHAR *data, short key)
{
  short menu_strings[] = {          /* Refer to index of menu_TXT[x].      */
    26, 8, 9, 0,
  };

  if (key == UNKNOWN_KEY) {
    /* Display the menu.                                                   */
    scrn_select_window(OPERATOR_WINDOW);
    scrn_string_out(menu_TXT[27], 1, 24);              /* EDP MEU          */
    view_menu(4, 24, menu_strings, empty);
    scrn_string_out(menu_TXT[10],13,24);       /* prompt           */
    return(key);
  }
  else {
    /* A choice is made, check for legal value.                            */
    if( 
    vfy_range(data,1,3) != SUCCEED ) {
      err_invoke(INVALID_MENU_OPTION);                 /* out of range     */
      *data=_T('\0');                                         /* reset buffer     */
      return(UNKNOWN_KEY);
    }
    else {
      view_menu(4, 24, menu_strings, data);
      return(_ttoi(data));
    }
  }
} /* edp_menu */
