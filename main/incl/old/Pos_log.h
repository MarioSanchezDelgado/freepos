/*
 *     Module Name       : POS_LOG.H
 *
 *     Type              : Application Data Names
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
 * 13-Dec-1999 Initial Release WinPOS                                  E.J.
 * --------------------------------------------------------------------------
 * 21-Sep-2001 Added Pending Invoice functionality.                      M.W.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_LOG_H__
#define __POS_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NORMAL_INVOICE      0
#define VOIDED_INVOICE      1

extern void cre_invoice(short voided);
extern void save_article_lines(void);
extern void delete_pending_invoice(long);

#ifdef __cplusplus
}
#endif

#endif
