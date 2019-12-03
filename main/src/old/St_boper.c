/*
 *     Module Name       : ST_BOPER.C
 *
 *     Type              : States COperatorMode, BTillLift, BTillWallet,
 *                         BTillRefill, BTillRefillOk, BXRCashno
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
                                            /* Pos (library) include files   */
#include "appbase.h"
#include "stri_tls.h"
#include "misc_tls.h"

#include "tm_mgr.h"                         /* Toolsset include files.       */
#include "inp_mgr.h"
#include "state.h"
#include "bp_mgr.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "err_mgr.h"
#include "mapkeys.h"

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
#include "WPos_mn.h"
#include "st_main.h"

static short operator_menu(_TCHAR *, short);
static short BXRead(_TCHAR *, short);

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES  BREAK OPERATOR MENU MODE                                */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Previous state was EndBreak (14).     The OPERATOR-Key was pressed and   */
/*  confirmed with te S-key.                                                 */
/*   - Display Operator menu, and execute menu-functions.                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void BOperatorMode_VW(void)
{
  scrn_clear_window(OPERATOR_WINDOW);
  bcoperator_menu(empty, 0);            /* Initialise the menu.             */
//  view_lan_state(FORCED_UPDATE);        /* update ON- OFFLINE indicator.    */
} /* BOperatorMode_VW */

extern VERIFY_ELEMENT BOperatorMode_VFY[] =
{                              
  ENTER_KEY,            bcoperator_menu,
  CLEAR_KEY,            vfy_clear_key,
  KEYLOCK_NORMAL,       (void *)NULL,
  KEYLOCK_LOCK,         (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

extern PROCESS_ELEMENT BOperatorMode_PROC[] =
{
  2,                    EndOfShift,
  3,                    EndOfDay,
  KEYLOCK_LOCK,         proc_clr_scrn,
  KEYLOCK_NORMAL,       proc_clr_scrn,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT BOperatorMode_CTL[] =
{
  1, &BXRCashno_ST,                    /* Retreive cashno                   */
  2, &PosStandBy_ST,                   /* Loop to standby-mode              */
  3, &PosStandBy_ST,                   /* Loop to standby-mode              */
  4, &BTillLift_ST,
  5, &BTillRefill_ST,
  6, &BOperatorMode_ST,
  KEYLOCK_LOCK,   &StartBreak_ST,
  KEYLOCK_NORMAL, &StartBreak_ST,
  UNKNOWN_KEY,    &BOperatorMode_ST
};

extern INPUT_CONTROLLER Dopermnu1K3n;

extern STATE_OBJ BOperatorMode_ST =
{                         
  ST_BOPERATOR_MODE,
  BOperatorMode_VW,
  no_DFLT,
  &Dopermnu1K3n,
  BOperatorMode_VFY,
  (void *)NULL,
  BOperatorMode_PROC,
  BOperatorMode_CTL
};


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   BREAK OPERATOR TILL LIFT                               */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Perform a till-lift.                                                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/


static CONTROL_ELEMENT BTillLift_CTL[] =
{
  NO_KEY,          &BOperatorMode_ST,
  ENTER_KEY,       &BTillWallet_ST,
  KEYLOCK_NORMAL,  &StartBreak_ST,
  UNKNOWN_KEY,     &CTillLift_ST
};


extern STATE_OBJ BTillLift_ST =
{                         
  ST_BTILL_LIFT,
  OpTillLift_VW,
  no_DFLT,
  &Dwallamnt13n,
  OpTillLift_VFY,
  (void *)NULL,
  OpTillLift_PROC,
  BTillLift_CTL
};


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   BREAK OPERATOR WALLET NUMBER                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Retreive wallet-number for till-lift.                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static CONTROL_ELEMENT BTillWallet_CTL[] =
{
  NO_KEY,          &BOperatorMode_ST,
  ENTER_KEY,       &BOperatorMode_ST,
  ILLEGAL_VALUES,  &BTillLift_ST,       /* Both fields zero, retry.          */
  KEYLOCK_NORMAL,  &StartBreak_ST,
  UNKNOWN_KEY,     &BTillWallet_ST
};


extern STATE_OBJ BTillWallet_ST =
{                         
  ST_BTILL_WALLET,
  (void *)NULL,
  no_DFLT,
  &Dwallno3n,
  OpTillWallet_VFY,
  (void *)NULL,
  OpTillWallet_PROC,
  BTillWallet_CTL
};


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   BREAK OPERATOR TILL REFILL                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Perform a till-refill.                                                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static CONTROL_ELEMENT BTillRefill_CTL[] =
{
  NO_KEY,          &BOperatorMode_ST,
  ENTER_KEY,       &BTillRefillOk_ST,
  KEYLOCK_NORMAL,  &StartBreak_ST,
  UNKNOWN_KEY,     &BTillRefill_ST
};


extern STATE_OBJ BTillRefill_ST =
{                         
  ST_BTILL_REFILL,
  OpTillRefill_VW,
  no_DFLT,
  &Dwallamnt13n,
  OpTillRefill_VFY,
  (void *)NULL,
  OpTillRefill_PROC,
  BTillRefill_CTL
};


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   BREAK OPERATOR TILL REFILL OK                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Retreive confirmation to perform a till-refill.                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static CONTROL_ELEMENT BTillRefillOk_CTL[] =
{
  NO_KEY,          &BTillRefill_ST,
  ENTER_KEY,       &BOperatorMode_ST,
  KEYLOCK_NORMAL,  &StartBreak_ST,
  UNKNOWN_KEY,     &BTillRefillOk_ST
};


extern STATE_OBJ BTillRefillOk_ST =
{                         
  ST_BTILL_REFILLOK,
  OpTillRefillOk_VW,
  no_DFLT,
  &DMode1K1,
  OpTillRefillOk_VFY,
  OpTillRefillOk_UVW,
  OpTillRefillOk_PROC,
  BTillRefillOk_CTL
};


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURES   ENTER CASHIER NUMBER FOR X-READ                        */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Retreive cashier number to perform a x-read.                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

extern PROCESS_ELEMENT BXRCashno_PROC[] =
{
  ENTER_KEY,            BXRead,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT BXRCashno_CTL[] =
{
  ENTER_KEY,       &BOperatorMode_ST,
  NO_KEY,          &BOperatorMode_ST,
  KEYLOCK_NORMAL,  &BOperatorMode_ST,
  UNKNOWN_KEY,     &BXRCashno_ST
};

extern STATE_OBJ BXRCashno_ST =
{                         
  ST_BXR_CASHNO,
  OpXRCashno_VW,
  no_DFLT,
  &XRcashid3K3n,
  OpXRCashno_VFY,
  (void *)NULL,
  BXRCashno_PROC,
  BXRCashno_CTL
};

/*---------------------------------------------------------------------------*/
/*                                BXRead                                     */
/*---------------------------------------------------------------------------*/
static short BXRead(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Perform an X-read of the requested cashier-number (data).             */
  /* Be sure the current shift is activated after performing the X-read.   */
  /*                                                                       */
  proc_x_read((short)_ttoi(data));
  tm_last(TM_SHFT_NAME, (void*)&c_shft);
  *data=_T('\0');

  return(key);
} /* BXRead */
