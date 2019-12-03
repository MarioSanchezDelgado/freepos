/*
 *     Module Name       : MISC_TLS.H
 *
 *     Type              : Include file miscellaneous functions
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
 * 19-Mar-2001 Added semaphore functionality.                          J.D.M.
 * --------------------------------------------------------------------------
 * 26-Mar-2003 Added functions to compare doubles and floats.          J.D.M.
 * --------------------------------------------------------------------------
 * 22-Oct-2004 Added Semaphore_Get() to retrieve the value.            J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef _misc_tls_h_
#define _misc_tls_h_

#ifdef __cplusplus
extern "C" {
#endif

extern short  vfy_range(_TCHAR *, short, short);
extern short  vfy_clear_key(_TCHAR *, short);
extern short  vfy_clear_all_key(_TCHAR *, short);

extern int    pos_calc_pincd(int, _TCHAR *, long);

extern short  element_in_set(short, short *);
extern short  called_by_state(short *);

extern void   display_prompt_right(_TCHAR *, unsigned short);
extern void   display_prompt(_TCHAR *, unsigned short);

extern short  check_barcode_ean(_TCHAR *);
extern void   display_percentage(short, short, unsigned short);

extern void   Semaphore_P(long*);
extern void   Semaphore_V(long*);
extern void   Semaphore_Set(long*, long);
extern long   Semaphore_Get(long*);

extern short  Dbl_EQ(double, double, short);
extern short  Dbl_GT(double, double, short);
extern short  Dbl_GTEQ(double, double, short);
extern short  Dbl_LT(double, double, short);
extern short  Dbl_LTEQ(double, double, short);

#ifdef __cplusplus
}
#endif

#endif
