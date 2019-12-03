/*
 *     Module Name       : POS_TRAN.H
 *
 *     Type              : Convert numeric into alpha-numeric
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

#ifndef __POS_TRAN_H__
#define __POS_TRAN_H__

#ifdef __cplusplus
extern "C" {
#endif


#if LANGUAGE == ESPANOL
#define MAX_AMOUNT_LENGTH 7             /*      9.999.999                    */
#else
#define MAX_AMOUNT_LENGTH 11            /* 99.999.999.999                    */
#endif

extern _TCHAR *num_to_alpha(_TCHAR*, _TCHAR);

#ifdef __cplusplus
}
#endif

#endif
