/*
 *     Module Name       : POS_KEYS.H
 *
 *     Type              : Input function key definitions
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
 * 05-Dec-2001 Changes for Article Finder.                               M.W.
 * --------------------------------------------------------------------------
 * 23-Jan-2002 Changes for Credit note. Ask for amount if offline.       M.W.
 * --------------------------------------------------------------------------
 * 25-Mar-2005 VOUCHER renamed to PAY_BY_VOUCHER because of conflict 
 *             with VOUCHER structure in storerec.h                      M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 * 16-Aug-2005 Select customer if searched by fisc_no                    M.W.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_KEYS_H__
#define __POS_KEYS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* DOUBLE_NULL_KEY (= dummy convert key). This one is defined in inp_mgr.h */

/*
 *    These keys are used for Control Purposes (Don't Change them)
 *    These are for the POS Invoice transactions
 */

#define EXIT_FIELD             500      /* Exit field after pressing CLEAR   */
#define CASHNO_OK              510
#define SUPERVISOR_OK          511 
#define PINCODE_NOK            512
#define ARTICLE_NOT_OK         513
#define CUSTNO_OK              514
#define ARTICLE_OK             515
#define PRICE_ART              516
#define WEIGHT_ART             517
#define WWEIGHT_ART            518
#define PWEIGHT_ART            519
#define AMNT_NOT_ENOUGH        520
#define AMNT_ENOUGH            521
#define PRICE_TOO_LARGE_KEY    522
#define ILLEGAL_VALUES         523
#define CUST_EXPIRED           524
#define EMUL_LOCK_EXCPT        525
#define EMUL_LOCK_SUPER        526
#define PAY_BY_VOUCHER         527 /* VOUCHER in conflict with a structure of */
                                   /* same name in storerec.h                 */
#define VOUCHER_AMOUNT         528
#define SELECT_PRINTER_OK      529
#define SELECT_CUST_KEY        530

#define QUERY_OKAY             550
#define QUERY_NO_RECORDS       551
//#define SELECT_DOCUMENT_OK     552

#ifdef __cplusplus
}
#endif

#endif
