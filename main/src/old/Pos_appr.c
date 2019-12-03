/*
 *     Module Name       : POS_APPR.C
 *
 *     Type              : Approval array and functions
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
 * 29-Mar-2001 Added OCIA2_DATA.                                       R.N.B.
 * --------------------------------------------------------------------------
 * 29-May-2001 Removed dependency on mask in KeylockEmulApprove        R.N.B.
 * --------------------------------------------------------------------------
 * 21-Sep-2001 Added Pending Invoice functionality.                      M.W.
 * --------------------------------------------------------------------------
 * 03-Oct-2002 Recursion flag ask_pincd_keyl_emul().                   J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
                                            /* POS (library) include files.  */
#include "appbase.h"
#include "stri_tls.h"

#include "state.h"
#include "scrn_mgr.h"
#include "tm_mgr.h"
#include "err_mgr.h"

#include "pos_tm.h"                         /* Application include files.    */
#include "pos_errs.h"
#include "pos_txt.h"
#include "pos_inp.h"
#include "Pos_recs.h"
#include "pos_func.h"
#include "pos_vfy.h"
#include "pos_dflt.h"
#include "pos_st.h"
#include "pos_scrl.h"
#include "pos_recs.h"
#include "st_main.h"
#include "WPos_mn.h"
#include "condll.h"

#include "mapkeys.h"

static short appr_S_key(short, short);
static short appr_SN_key(short, short);
static short check_invoice(short, short);
static short appr_VOID_key(short, short);
static short chk_inv_and_appr_SN(short, short);
static short ask_pincd_keyl_emul(short, short (*)(_TCHAR *, short)); 


APPR_ELEMENT extra_approval[] = {
 /* Key                  In which state       msg_index   approval function  */
    CREDIT_KEY,          ST_CUSTOMER_MODE,       2,       (void*)appr_SN_key
   ,OPEN_DRAWER_KEY,     ST_NULL,                3,       (void*)appr_SN_key
   ,VOID_INVOICE_KEY,    ST_NULL,                1,       (void*)appr_VOID_key
   ,OPERATOR_KEY,        ST_NULL,                4,       (void*)appr_S_key
    /* The following keys are only legal if nbr_inv_lines < 999              */
   ,TIMES_KEY,           ST_INVOICING,           0,       (void*)check_invoice
   ,OCIA1_DATA,          ST_INVOICING,           0,       (void*)check_invoice
   ,OCIA2_DATA,          ST_INVOICING,           0,       (void*)check_invoice
   ,ENTER_KEY,           ST_INVOICING,           0,       (void*)check_invoice
   ,REPEAT_LAST_ITEM_KEY,ST_INVOICING,           0,       (void*)check_invoice
   ,VOID_INVOICE_KEY,    ST_INVOICING,           0,       (void*)check_invoice
   ,SAVE_INVOICE_KEY,    ST_INVOICING,           0,       (void*)check_invoice
   ,CREDIT_KEY,          ST_INVOICING,           6,       (void*)chk_inv_and_appr_SN
   ,DISCOUNT_KEY,        ST_INVOICING,           7,       (void*)chk_inv_and_appr_SN
   /* End of this array indicated by UNKNOWN_KEY.                            */
   ,UNKNOWN_KEY,       ST_NULL,                  0,       (void*)0
};


/*-------------------------------------------------------------------------*/
/*                               appr_S_key                                */
/*-------------------------------------------------------------------------*/
static short appr_S_key(short key, short msg_index)
{
  short show_cur_sav = GetShowCursor();

  /*                                                                       */
  /* Display approval message and wait for S-key or NO.                    */
  /*                                                                       */
  inp_abort_data();

  SetShowCursor(FALSE);

  scrn_clear_window(ERROR_WINDOW_ROW1);
  scrn_string_out(appr_msg_TXT[msg_index],0,0);
  if (wait_for_key_S_or_NO() == FAIL) {
    key = UNKNOWN_KEY;
  }
  scrn_clear_window(ERROR_WINDOW_ROW1);
  scrn_clear_window(ERROR_WINDOW_ROW2);

  SetShowCursor(show_cur_sav);

  return(key);
} /* appr_S_key */


/*-------------------------------------------------------------------------*/
/*                               appr_SN_key                               */
/*-------------------------------------------------------------------------*/
static short appr_SN_key(short key, short msg_index)
{
  short show_cur_sav = GetShowCursor();

  /*                                                                       */
  /* Display approval message and wait for S-key followed by the N-key     */
  /* or NO.                                                                */
  /*                                                                       */
  inp_abort_data();

  SetShowCursor(FALSE);

  scrn_clear_window(ERROR_WINDOW_ROW1);
  scrn_string_out(appr_msg_TXT[msg_index],0,0);
  if (wait_for_key_S_or_NO() == FAIL) {
    key = UNKNOWN_KEY;
  }
  else {
    if (rs_keylock_position()==KEYLOCK_SUPERVISOR) {
      wait_for_supervisor_approval();               /* S-key -> N-key      */
    }
  }
  scrn_clear_window(ERROR_WINDOW_ROW1);
  scrn_clear_window(ERROR_WINDOW_ROW2);

  SetShowCursor(show_cur_sav);

  return(key);
} /* appr_SN_key */


/*---------------------------------------------------------------------------*/
/*                           check_invoice                                   */
/*---------------------------------------------------------------------------*/
static short check_invoice(short key, short msg_index)
{
  /*                                                                         */
  /* Check invoice status during invoicing, detects:                         */
  /*                                                                         */
  /*    - number of lines to log to BO overflow                              */
  /*        nbr_inv_lines contain the number of invoice-lines to be          */
  /*        logged to BO. (An article with deposit counts for 2!             */
  /*    - (Cash-float to low     -> No check, F. Gijbels, dec 92)            */
  /*    - (Total amount to large -> No check, F. Gijbels, dec 92)            */
  /*                                                                         */

  if (nbr_inv_lines >= TM_ITEM_MAXI-2) { /* Maximum number of invoice lines. */
    err_invoke(TOO_MANY_INVOICE_LINES);
    return(UNKNOWN_KEY);
  }

  return(key);
} /* check_invoice */


/*---------------------------------------------------------------------------*/
/*                           appr_VOID_key                                   */
/*---------------------------------------------------------------------------*/
static short appr_VOID_key(short key, short msg_index)
{
  /*                                                                         */
  /* Approve VOID_INVOICE_KEY with Supervisor key.                           */
  /*                                                                         */
  /* Some stores don't want to approve the VOID key if there are no lines    */
  /* in the invoice. This is usefull in case a wrong customer number         */
  /* is entered. To activate this feature, uncomment the lines below.        */
  /*                                                                         */

//  if (nbr_inv_lines > 0 || nbr_void_lines > 0) {
  return(appr_SN_key(key, msg_index));
//  }
//  else {
//    return(key);
//  }
} /* appr_VOID_key */


/*---------------------------------------------------------------------------*/
/*                           chk_inv_and_appr_SN                             */
/*---------------------------------------------------------------------------*/
static short chk_inv_and_appr_SN(short key, short msg_index)
{
  /*                                                                         */
  /* Check invoice and if OK, ask for approval S-key.                        */
  /*                                                                         */
  if (check_invoice(key, 0) != UNKNOWN_KEY &&
      appr_SN_key(key, msg_index) == key) {
    c_item.approved = TRUE;
    return(key);
  }

  return(UNKNOWN_KEY);
} /* chk_inv_and_appr_SN */


/*---------------------------------------------------------------------------*/
/*                           KeylockEmulApprove                              */
/*---------------------------------------------------------------------------*/
short KeylockEmulApprove(short key, short mask)
{
  short  ret_bool = FALSE;

#ifdef NO_KEYL_VFY
  short WARNING_NO_PINCD_VERIFY; /* Don't change or use this variable. It is */
                                 /* meant to generate a compiler warning!    */
  return TRUE;
#endif

  switch (key) {
    case KEYLOCK_SUPERVISOR:
        ret_bool = ask_pincd_keyl_emul(3, vfy_super_pincd);
      break;
    case KEYLOCK_NORMAL: /* no pin_cd */
        ret_bool = TRUE;
      break;
    case KEYLOCK_LOCK:  /* no pin_cd */
        ret_bool = TRUE;
      break;
    case KEYLOCK_EXCEPTION:
        ret_bool = ask_pincd_keyl_emul(9, vfy_edp_pincd);
      break;
    default:
      break;
  }                              

  return (ret_bool);
} /* KeylockEmulApprove */

static INPUT_CONTROLLER Dmanpin4Kn =
{
  (INPUT_DISPLAY *)&dsp_manpincd4,
  KEYBOARD_MASK,
  5,
  5,
  (VERIFY_KEY *)&numeric_no_err
};

/*---------------------------------------------------------------------------*/
/*                           ask_pincd_keyl_emul                             */
/*---------------------------------------------------------------------------*/
short ask_pincd_keyl_emul(short prompt_idx, short (*vfy_pincd_func)(_TCHAR *, short))
{
  _TCHAR buf[6];
  _TCHAR text_sav[75];
  short  show_cur_sav, row_sav, col_sav;
  short  row, col, key;
  short  ret_bool = FALSE;
  short  num_tries = 0;
  static short busy = 0;
                                             /* am I called recursively? */
  if (busy) {
    return(FALSE);
  }

  busy++;

  GetCaretPosition(&row_sav, &col_sav);           /* save caret position */
  show_cur_sav = GetShowCursor();

  scrn_select_window(ERROR_WINDOW_ROW2);
  scrn_get_string(0, 0, text_sav, 70);
  *buf = _T('\0');

  for (;;) {
    SetShowCursor(TRUE);
    scrn_clear_window(ERROR_WINDOW_ROW2);
    scrn_string_out(prompt_TXT[prompt_idx],0,0);
    format_display_passwrd(&dsp_manpincd4, buf);
    scrn_get_csr(&row, &col);
    SetCaretPosition(col, row);

    key = inp_get_data(&Dmanpin4Kn, buf);
    if (key == CLEAR_KEY) {
      *buf=_T('\0');
    }
    else if (key == NO_KEY) {
      break;
    }
    else if (key==ENTER_KEY) {
      if (vfy_pincd_func(buf, key) != UNKNOWN_KEY) {
        ret_bool = TRUE;                               /* pin_cd okay */
        break;
      }
      rs_error_tone();               /* Wrong pin_cd, make some noice */
      if (num_tries++ >= 3) {
        ;
      }
    }
  }
                                       /* restore situation as before */
  SetShowCursor(show_cur_sav);
  SetCaretPosition(row_sav, col_sav);
  scrn_clear_window(ERROR_WINDOW_ROW2);
  scrn_string_out(text_sav, 0, 0);

  busy--;

  return (ret_bool);
} /* ask_pincd_keyl_emul */
