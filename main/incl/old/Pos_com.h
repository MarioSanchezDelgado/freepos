/*
 *     Module Name       : POS_COM.H
 *
 *     Type              : POS ctree & communication declarations
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
 * 11-Dec-2000 Added functions for asynchronous request for crazy arts R.N.B.
 * --------------------------------------------------------------------------
 * 23-Jan-2002 Changes for Credit note. Ask for amount if offline.       M.W.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 14-Oct-2004 Put multisam actions in memory for faster performance.  J.D.M.
 * --------------------------------------------------------------------------
 * 28-Jul-2005 Summarize tax codes on total screen.                      M.W.
 * --------------------------------------------------------------------------
 * 02-Aug-2005 Search customer by Fiscal number.                         M.W.
 * --------------------------------------------------------------------------
 * 16-Aug-2005 Select customer if searched by fisc_no                    M.W.
 * --------------------------------------------------------------------------
 * 01-May-2007 Added init_till_table() and update_till_callback()      J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_COM_H__
#define __POS_COM_H__

#pragma warning(push) /* Push current warning level to stack. */
#pragma warning(disable: 4786) /* Disable warning 4786.       */

#ifdef __cplusplus

#include <map>
using namespace std;

extern multimap<MMML_KEY, MMML_DEF> multisam_definitions;

extern "C" {
#endif

#define CUST_CHKD_WRONG     -1             /* Chk dig has changed           */
#define CUST_NUM_WRONG      -3             /* Invalid custno, never allowed */
#define INVALID_CUST_ON_POS -4          /* invalid genvar.cust_on_pos value */
#define PEND_INV_FOUND      -5   /* pending invoice found for this customer */
#define FISC_NO_NOT_FOUND   -6   /* no customer found by search by fisc_no  */
#define FISC_NO_MANY_FOUND  -7   /* more than one customer found (fisc_no)  */

#define CASH_NOT_ALLOWED 0
#define CASH_ALLOWED     1
#define CASH_ALREADY_ON  2
#define MAX_VAT          10
#define MAX_CURR         10


extern void    init_cust_rec(POS_CUST_DEF *);
extern short   get_cust_data(_TCHAR *, POS_PEND_CUST_DEF *);
extern short   get_cust_cr(long, POS_CUST_DEF *);
extern short   get_cust_fisc_cr(long, POS_CUST_FISC_DEF *);
extern short   get_cash_bo(short, CASH_DEF *);
extern short   logoff_cash_bo(short);
extern short   get_paym( PAYMENT_DEF *);
extern short   init_tax_table(void);
extern double  get_vat_perc(short);
extern short   get_vat_sum_cd(short);
extern short   init_paym(void);
extern short   init_till_table(void);
extern short   init_environment_records(short, void (*)(void));
extern short   init_date_system(void);
extern short   put_paym(PAYMENT_DEF *);
extern short   get_stand_rate(short, CURR_DEF *);
extern short   get_first_stand_rate(CURR_DEF *);
extern short   get_cart_bo(short, long, CRAZY_ART_DEF *, long, short);
extern short   get_cart_callback(void *);
extern short   return_pos_status(struct POS_STATUS *);
extern short   wait_for_cart(void);
extern short   activate_pwdn_callback(void *);
extern short   get_voucher_bo(long, long, long, short, long, VOUCHER_DEF *);
extern short   get_paym_cd_cash(void);
extern short   update_multisam_definitions_callback(void *);
extern void    update_multisam_definitions(void);
extern short   update_till_callback(void*);


//v.3.5.1 acm -{
short get_voucher_bo_turkey(long req_vale_no,   long req_vale_type, long req_voucher_status,
                            
                            short store_no,     long inv_cust_no, 
                            long inv_till_no,   long inv_invoice_type,  
                            long inv_invoice_no,long inv_invoice_date,
                            long inv_sequence_no,
                            VOUCHER_VALE_DEF *vouchrec, int time_wait);

//v.3.5.1 acm -}


//v.3.5.1 acm -{
short get_voucher_bo_anticipo(  long req_vale_no,   long req_vale_type, long req_voucher_status,
                                short store_no,     long inv_cust_no, 
                                long inv_till_no,   long inv_invoice_type,  
                                long inv_invoice_no,long inv_invoice_date,
                                long inv_sequence_no,
                                VOUCHER_ANTICIPO_DEF *vouchrec, int time_wait);

//v.3.5.1 acm -}


#ifdef __cplusplus
}
#endif

#pragma warning(pop) /* Pop old warning level from stack. */

#endif
