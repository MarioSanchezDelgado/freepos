/*
 *     Module Name       : WRITE.H
 * 
 *     Type              : Application Data Names
 *  
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
 * 17-Jan-2000 Unicode                                                 M.W.
 * 04-Oct-2000 Added function clear_all_used_print_files()             J.D.M.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 */

#ifndef __WRITE_H__
#define __WRITE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CALCULATE_IT          1
#define PRINT_IT              2

typedef struct prn_object {
  short (*fn)(struct prn_object *, short, short, short);
  short ln;
} PRN_OBJECT;

typedef struct {
  PRN_OBJECT *obj;
  short offs;
} TRAIL, HEAD;

extern _TUCHAR init_codes[];
extern short size_init_codes;
extern _TUCHAR cpi_lpi_codes[];
extern short size_cpi_lpi_codes;

extern short write_ln(PRN_OBJECT *, TRAIL *, HEAD *, short, short);
extern short print_ln(short, _TCHAR *);
extern short print_skip_ln(short, _TCHAR *);
extern short print_cut(short);
extern short print_color(short, short);
extern void  clear_all_used_print_files(void);
extern void  send_cpi_and_lpi(short);
extern void  send_init_codes(short);

extern void  send_formfeed(short);
extern void  prn_print(short, _TCHAR*);

#ifdef __cplusplus
}
#endif

#endif
