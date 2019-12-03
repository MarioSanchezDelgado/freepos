/*
 *     Module Name       : POS_VFY.C
 *
 *     Type              : Include File Verification Functions
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

#ifndef __POS_VFY_H__
#define __POS_VFY_H__

#ifdef __cplusplus
extern "C" {
#endif

extern short illegal_fn_key(_TCHAR *, short);
extern short vfy_non_empty_zero(_TCHAR *, short);
extern short vfy_non_zero(_TCHAR *,short);
extern short vfy_max_amount_drawer_or_zero(_TCHAR *,short);
extern short vfy_empty_field(_TCHAR *, short);
extern short vfy_and_clear_field(_TCHAR *, short);
extern short vfy_ocia_not_legal(_TCHAR *, short);
extern short vfy_key_in_N(_TCHAR *, short);
extern short vfy_cash_pincd(_TCHAR *, short);
extern short vfy_edp_pincd(_TCHAR *, short);
extern short vfy_super_pincd(_TCHAR *, short);
extern short vfy_emul_keylock(_TCHAR *, short);

#ifdef __cplusplus
}
#endif

#endif
