/*
 *     Module Name       : TOT_MGR.H
 *
 *     Type              : Include file totals manager
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
 * 17-May-2002 Added tot_set_double                                    R.N.B.
 * --------------------------------------------------------------------------
 */

#ifndef TOTAL_H
#define TOTAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* FUNCTIONS                                                                  */
extern _TCHAR *tot_ret_string(short, _TCHAR *);
extern short  tot_add_double(short, double);
extern short  tot_set_double(short, double);
extern double tot_ret_double(short);
extern short  tot_reset_double(short);
extern short  tot_init(short);
extern void   tot_deinit(void);
extern short  tot_add_string(short,_TCHAR *);

#ifdef __cplusplus
}
#endif

#endif
