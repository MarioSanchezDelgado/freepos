/*
 *     Module Name       : POS_BP2.H
 *
 *     Type              : Include file application print functions
 *                         (invoice, x-read, z-read)
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
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_BP2_H__
#define __POS_BP2_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RESET                 0         /* action codes in assembling total  */
#define INCREMENT             1         /*   lines of the invoice            */

#define DESCR_SIZE           33         /* length of article description     */
#define PAYM_DESCR_SIZE      15         /* length of payment description     */

#define START_LN_PROMO       5          /* first line of promo text          */
#define START_LN_CUST        11         /* start line for printing cust head.*/
#define START_LN_ART         16         /* article line /linea donde se      */
	                                    /* empieza a pintar los articulos    */
#define START_LN_XREAD       17         /* X/Z Read                          */

#define LN_TRANS_INV          6         /* transport lines to next page      */
#define LN_BOT_INV            1         /* space lines on bottom of invoice  */
#define LN_PG_INV            60         /* lines per page on invoice         */

#define LN_TEAR_OFF           0         /* nr. lines tear-off till 1st line  */
                                        /* before printing LN_TEAR_OFF lines */
                                        /* are skipped reversed, after print */
                                        /* -ing LN_TEAR_OFF lines skipped    */
                                        /* forward                           */

/*******/

#define REV_NBR_LINES         3         /* number of reverse feed lines      */
#define CH_WIDTH             65         /* cheque width (number of chars)    */  
#define CH_LINES             14         /* cheque number of lines            */
#define CH_START              4         /* cheque offset in lines            */

#define CH_ANUM_OFFS_1        3         /* start of alphanum amount line 1   */
#define CH_ANUM_OFFS_2        0         /* start of alphanum amount line 2   */
#define CH_ANUM_SIZE_1       (CH_WIDTH -  3 - CH_ANUM_OFFS_1)
#define CH_ANUM_SIZE_2       (CH_WIDTH - 49 - CH_ANUM_OFFS_2)

/*******/

#define LN_PG_DISC           22         /* max number for discount block     */
#define LEFT_MARGIN          10         /* left margin for xread/zread       */

extern short fn_forward_skip(PRN_OBJECT *, short, short, short);
extern short fn_reverse_skip(PRN_OBJECT *, short, short, short);
extern short fn_init_prn(PRN_OBJECT *, short, short, short);

extern short fn_xr_zr_init(PRN_OBJECT *, short, short, short);
extern short fn_xr_header(PRN_OBJECT *, short, short, short);
extern short fn_xr_paym_amnts(PRN_OBJECT *, short, short, short);
extern short fn_xr_paym_totals_per_cshr(PRN_OBJECT *, short, short, short);
extern short fn_xr_zr_paym_header(PRN_OBJECT *, short, short, short);
extern short fn_xr_zr_time_header(PRN_OBJECT *, short, short, short);
extern short fn_xr_zr_shift_per_time(PRN_OBJECT *, short, short, short);
extern short fn_xr_till_totals(PRN_OBJECT *, short, short, short);

/*********/
extern short fn_zr_connect(PRN_OBJECT *, short, short, short);
/*********/
 
extern short fn_zr_header(PRN_OBJECT *, short, short, short);
extern short fn_zr_paym_totals_per_cshr(PRN_OBJECT *, short, short, short);
extern short fn_zr_paym_totals_per_till(PRN_OBJECT *, short, short, short);
extern short fn_zr_till_totals(PRN_OBJECT *, short, short, short);

extern short fn_i_total_lines(PRN_OBJECT *, short, short, short);
extern short fn_i_article_line(PRN_OBJECT *, short, short, short);
extern short fn_i_deposit_line(PRN_OBJECT *, short, short, short);
extern short fn_i_discount_line(PRN_OBJECT *, short, short, short);
extern short fn_i_customer(PRN_OBJECT *, short, short, short);
extern short fn_i_void_inv(PRN_OBJECT *, short, short, short);
extern short fn_i_to_end_of_page(PRN_OBJECT *, short, short, short);
extern short fn_i_subtotal(PRN_OBJECT *, short, short, short);
extern short fn_i_skip_one_line(PRN_OBJECT *, short, short, short);
extern short fn_i_empty_bottom(PRN_OBJECT *, short, short, short);
extern short fn_i_to_transport(PRN_OBJECT *, short, short, short);
extern short fn_i_transported(PRN_OBJECT *, short, short, short);
extern short fn_i_promo(PRN_OBJECT *, short, short, short);
extern short fn_i_cust_fee_line(PRN_OBJECT *, short, short, short);
extern short fn_i_msam_header(PRN_OBJECT *, short, short, short);
extern short fn_i_msam_line(PRN_OBJECT *, short, short, short);
extern short fn_i_msam_total(PRN_OBJECT *, short, short, short);
extern short fn_i_small_header(PRN_OBJECT *, short, short, short);
extern short fn_i_small_article(PRN_OBJECT *, short, short, short);
extern short fn_i_small_deposit(PRN_OBJECT *, short, short, short);
extern short fn_i_small_discount(PRN_OBJECT *, short, short, short);
extern short fn_i_small_total(PRN_OBJECT *, short, short, short);
extern short fn_i_small_tax(PRN_OBJECT *, short, short, short);
extern short fn_i_small_footer(PRN_OBJECT *, short, short, short);
extern short fn_i_small_void(PRN_OBJECT *, short, short, short);
extern short fn_i_small_promo(PRN_OBJECT *, short, short, short);
extern short fn_i_small_cust_fee(PRN_OBJECT *, short, short, short);
extern short fn_i_small_msam_header(PRN_OBJECT *, short, short, short);
extern short fn_i_small_msam_line(PRN_OBJECT *, short, short, short);
extern short fn_i_small_msam_total(PRN_OBJECT *, short, short, short);

extern short calc_nbr_paym_ways(void);
extern short str_paym_descr(_TCHAR *, _TCHAR *);

extern short paym_amnts_tot_cshr(_TCHAR *, _TCHAR *, _TCHAR *);
extern short paym_amnts_tot_cshr_TR(_TCHAR *, _TCHAR *, _TCHAR *,short);
extern short paym_amnts_tot_till_TR(_TCHAR *first, _TCHAR *next, _TCHAR *till_tot,short printer);

extern short prnt_totals(short);
extern short prnt_total_vat(short, _TCHAR *);
extern void  prnt_subtot_per_vat(short, short, _TCHAR *, _TCHAR *, short);
extern void  prnt_packs_part(short, short, _TCHAR *);
extern short prnt_total_paym(short, _TCHAR *);

extern short init_paym_tot(_TCHAR[][TOTAL_BUF_SIZE + 1]);



#ifdef __cplusplus
}
#endif

#endif
