/*
 *     Module Name       : POS_BP1.C
 *
 *     Type              : Include file Application Buffered Print Functions
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
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_BP1_H__
#define __POS_BP1_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BP_SMALL_INVOICE     57         /* Printer Listname for small inv.   */
#define BP_INVOICE           58         /* Printer Listname for invoice      */
#define BP_XREAD             59         /* Printer Listname for X-read       */
#define BP_ZREAD             60         /* Printer Listname for Z-read       */

                /* NOTE: the numbers below correspond to the position of the */
                /* function in array bp_alt_fn[]. See Pos_bp1.c.             */
#define BP_INV_INIT               0
#define BP_PROMO_LINES            1
#define BP_CUSTOMER_LINES         2
#define BP_ARTICLE_LINE           3
#define BP_DEPOSIT_LINE           4
#define BP_DISCOUNT_LINE          5
#define BP_TOTAL_LINES            6
#define BP_DUMMY                  7
#define BP_VOID_INV               8
#define BP_START_TRAINING         9
#define BP_END_TRAINING          10
#define BP_SUBTOTAL_LINES        11
#define BP_SKIP_ONE_LINE         12
#define BP_TO_TRANSPORT_LINE     13
#define BP_TRANSPORTED_LINE      14
#define BP_EMPTY_BOTTOM_LINES    15
#define BP_CUST_FEE_LINE         16
#define BP_MSAM_HEADER           17
#define BP_MSAM_LINE             18
#define BP_MSAM_TOTAL            19
#define BP_SMALL_HEADER          20
#define BP_SMALL_ARTICLE         21
#define BP_SMALL_DEPOSIT         22
#define BP_SMALL_DISCOUNT        23
#define BP_SMALL_TOTAL           24
#define BP_SMALL_TAX             25
#define BP_SMALL_FOOTER          26
#define BP_SMALL_VOID            27
#define BP_SMALL_PROMO           28
#define BP_SMALL_CUST_FEE        29
#define BP_SMALL_MSAM_HEADER     30
#define BP_SMALL_MSAM_LINE       31
#define BP_SMALL_MSAM_TOTAL      32
                              /* 33..35 are freely available */
#define BP_FORWARD_SKIP          36
#define BP_REVERSE_SKIP          37
#define BP_XR_ZR_INIT            38
#define BP_ZR_PER_TILL           39
#define BP_XR_PER_CASHIER        40

extern short get_short(_TCHAR *);
extern short get_first_cshr_tdm(short);

extern _TUCHAR ec_init_codes[];
extern short   ec_init_len;
extern _TUCHAR ec_rev_lf[];
extern short   ec_rev_lf_len;

extern PRN_OBJECT forward_skip;
extern PRN_OBJECT reverse_skip;
extern PRN_OBJECT init_prn;

extern PRN_OBJECT ch_side_1;
extern PRN_OBJECT ch_side_2; 

extern PRN_OBJECT xr_zr_init;
extern PRN_OBJECT xr_header;
extern PRN_OBJECT xr_zr_paym_header;
extern PRN_OBJECT xr_paym_amnts;
extern PRN_OBJECT xr_paym_totals_per_cshr;
extern PRN_OBJECT xr_zr_time_header;
extern PRN_OBJECT xr_zr_shift_per_time;
extern PRN_OBJECT xr_till_totals;

/*******/
extern PRN_OBJECT zr_connect;
/*******/
 

extern PRN_OBJECT zr_header;
extern PRN_OBJECT zr_paym_totals_per_cshr;
extern PRN_OBJECT zr_paym_totals_per_till;
extern PRN_OBJECT zr_till_totals;

extern PRN_OBJECT zr_connect;

extern PRN_OBJECT i_total_lines;
extern PRN_OBJECT i_article_line;
extern PRN_OBJECT i_customer;
extern PRN_OBJECT i_void_inv;
extern PRN_OBJECT i_to_end_of_page;
extern PRN_OBJECT i_subtotal;
extern PRN_OBJECT i_skip_one_line;
extern PRN_OBJECT i_deposit_line;
extern PRN_OBJECT i_discount_line;
extern PRN_OBJECT i_promo;
extern PRN_OBJECT i_empty_bottom;
extern PRN_OBJECT i_to_transport;
extern PRN_OBJECT i_transported;
extern PRN_OBJECT i_cust_fee_line;
extern PRN_OBJECT i_msam_header;
extern PRN_OBJECT i_msam_line;
extern PRN_OBJECT i_msam_total;
extern PRN_OBJECT i_small_header;
extern PRN_OBJECT i_small_article;
extern PRN_OBJECT i_small_deposit;
extern PRN_OBJECT i_small_discount;
extern PRN_OBJECT i_small_total;
extern PRN_OBJECT i_small_tax;
extern PRN_OBJECT i_small_footer;
extern PRN_OBJECT i_small_void;
extern PRN_OBJECT i_small_promo;
extern PRN_OBJECT i_small_cust_fee;
extern PRN_OBJECT i_small_msam_header;
extern PRN_OBJECT i_small_msam_line;
extern PRN_OBJECT i_small_msam_total;

#ifdef __cplusplus
}
#endif

#endif