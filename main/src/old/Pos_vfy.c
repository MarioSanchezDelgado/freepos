/*
 *     Module Name       : POS_VFY.C
 *
 *     Type              : Verification Functions
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
#include "appbase.h"
#include "inp_mgr.h"                        /* Toolsset include files.       */
#include "state.h"
#include "tm_mgr.h"
#include "tot_mgr.h"
#include "err_mgr.h"
#include "mapkeys.h"
#include "misc_tls.h"

#include "pos_vfy.h"                        /* Application include files.    */
#include "pos_errs.h"
#include "pos_keys.h"
#include "pos_tm.h"
#include "pos_tot.h"
#include "pos_recs.h"
#include "pos_func.h"
#include "pos_recs.h"
#include "WPos_mn.h"

/*---------------------------------------------------------------------------*/
/*                           illegal_fn_key                                  */
/*---------------------------------------------------------------------------*/
short illegal_fn_key(_TCHAR *data, short key)
{
  err_invoke(ILLEGAL_FUNCTION_KEY_ERROR);

  return(UNKNOWN_KEY);
} /* illegal_fn_key */

/*---------------------------------------------------------------------------*/
/*                           vfy_non_empty_zero                              */
/*---------------------------------------------------------------------------*/
short vfy_non_empty_zero(_TCHAR *data, short key)
{
  /*                                                                        */
  /* If data not empty, it must be > 0.                                     */
  /*                                                                        */
  if (*data!=_T('\0') && STR_ZERO(data)) {
    err_invoke(ZERO_NOT_LEGAL_ERROR);
    return(UNKNOWN_KEY);
  }

  return(key);
} /* vfy_non_empty_zero */

/*---------------------------------------------------------------------------*/
/*                           vfy_non_zero                                    */
/*---------------------------------------------------------------------------*/
short vfy_non_zero(_TCHAR *data, short key)
{

  if (_tcstod(data, NULL) != (double)0) {
    return(key);
  }
  *data=_T('\0');
  err_invoke(ZERO_NOT_LEGAL_ERROR);

  return(UNKNOWN_KEY);
} /* vfy_non_zero */

/*---------------------------------------------------------------------------*/
/*                    vfy_max_amount_drawer_or_zero                          */
/*---------------------------------------------------------------------------*/
short vfy_max_amount_drawer_or_zero(_TCHAR *data, short key)
{

  if (_tcstod(data, NULL) != (double)0) {

    if (tot_ret_double(AMOUNT_IN_DRAWER)+(double)atof_price(data) > genvar.max_amnt_drawer) {
      err_invoke(EXCEED_MAX_AMOUNT_CASHDRAWER);
      return(UNKNOWN_KEY);
    }

    return(key);
  }
  *data=_T('\0');
  err_invoke(ZERO_NOT_LEGAL_ERROR);

  return(UNKNOWN_KEY);
} /* vfy_max_amount_drawer_or_zero */

/*---------------------------------------------------------------------------*/
/*                           vfy_empty_field                                 */
/*---------------------------------------------------------------------------*/
short vfy_empty_field(_TCHAR *data, short key)
{
  if (*data == _T('\0')) {
    return(key);
  }
  *data=_T('\0');
  err_invoke(FIELD_NOT_EMPTY);

  return(UNKNOWN_KEY);
} /* vfy_empty_field */

/*---------------------------------------------------------------------------*/
/*                           vfy_and_clear_field                             */
/*---------------------------------------------------------------------------*/
short vfy_and_clear_field(_TCHAR *data, short key)
{
  if (*data != _T('\0')) {
    *data=_T('\0');
    err_invoke(FIELD_NOT_EMPTY);
  }

  return(key);
} /* vfy_and_clear_field */

/*---------------------------------------------------------------------------*/
/*                           vfy_ocia_not_legal                              */
/*---------------------------------------------------------------------------*/
short vfy_ocia_not_legal(_TCHAR *data, short key)
{
  /*                                                                       */
  /* It is not allowed to use the scanner on the current input-field.      */
  /* Make some noise and display message.                                  */
  /*                                                                       */

  err_invoke(INP_OCIA_NOT_LEGAL);
  *data=_T('\0');

  return(UNKNOWN_KEY);
} /* vfy_ocia_not_legal */

/*---------------------------------------------------------------------------*/
/*                           vfy_key_in_N                                    */
/*---------------------------------------------------------------------------*/
short vfy_key_in_N(_TCHAR *data, short key)
{
  rs_wait_for_key_in_N();

  return(KEYLOCK_NORMAL);
} /* vfy_key_in_N */

/*---------------------------------------------------------------------------*/
/*                           vfy_cash_pincd                                  */
/*---------------------------------------------------------------------------*/
short vfy_cash_pincd(_TCHAR *data, short key)
{
  if ((int)_tcstol(data,NULL,10) != pos_calc_pincd(c_shft.cashier, NULL,
                                           pos_system.run_date)) {
    err_invoke((ERR_NAME)(CASH_PINCD_MISTAKE1 + cash_pincd_mistakes));
    *data = _T('\0');
    key = UNKNOWN_KEY;
    if (++cash_pincd_mistakes >= 3) {
      key = PINCODE_NOK;
      cash_pincd_mistakes = 0;
    }
  }
  else {
    cash_pincd_mistakes = 0;
  }

  return (key);
} /* vfy_cash_pincd */


/*---------------------------------------------------------------------------*/
/*                           vfy_edp_pincd                                   */
/*---------------------------------------------------------------------------*/
short vfy_edp_pincd(_TCHAR *data, short key)
{
  if ((int)_tcstol(data,NULL,10) != pos_calc_pincd(genvar.super_pin, NULL,0)) {
    *data = _T('\0');
    key = UNKNOWN_KEY;
  }

  return (key);
} /* vfy_edp_pincd */

/*-------------------------------------------------------------------------*/
/*                           vfy_super_pincd                               */
/*-------------------------------------------------------------------------*/
short vfy_super_pincd(_TCHAR *data, short key)
{
  if ((short)_tcstol(data,NULL,10) != genvar.super_pin) {
    *data = _T('\0');
    key = UNKNOWN_KEY;
  }

  return (key);
} /* vfy_super_pincd */

/*---------------------------------------------------------------------------*/
/*                               vfy_keylock_emul                            */
/* - this function is used to skip the pin_cd states for the EDP and super-  */
/*   visor menu's when the keylock emulator is used (pincd was already asked)*/
/*---------------------------------------------------------------------------*/
short vfy_emul_keylock(_TCHAR *data, short key)
{
  if (keylock_attached==NO) {
    switch (key) {
      case KEYLOCK_EXCEPTION:
        key=EMUL_LOCK_EXCPT;
        break;
      case KEYLOCK_SUPERVISOR:
        key=EMUL_LOCK_SUPER;
        break;
      default:
        break;
    }
  }

  return(key);
} /* vfy_emul_keylock */
