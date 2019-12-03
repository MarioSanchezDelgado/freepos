/*
 *     Module Name       : POS_BP3.H
 *
 *     Type              : Article accumulation and printing
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

#ifndef __POS_BP3_H__
#define __POS_BP3_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PRN_IMM
extern short accumulate_invoice(void);
extern void  print_invoice(void);
extern void  clean_invoice(void);
#endif
extern void  print_invoice_type_unsorted(void);

#ifdef PRN_IMM
extern void print_total_msam(void);
#else
void print_total_msam(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
