/*
 *     Module Name       : POS_BP1.C
 *
 *     Type              : Application Buffered Print Functions
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

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include "appbase.h"

#include <math.h>
                                            /* POS (library) include files.  */
#include "stri_tls.h"

                                            /* Toolsset include files.       */
#include "fmt_tool.h"
#include "tot_mgr.h"
#include "err_mgr.h"
#include "tm_mgr.h"
#include "bp_mgr.h"
#include "prn_mgr.h"
#include "inp_mgr.h"
#include "write.h"

#include "pos_inp.h"                        /* Application include files.    */
#include "pos_tm.h"
#include "pos_bp1.h"
#include "pos_recs.h"
#include "WPos_mn.h"


static TRAIL trail_none[] = {(PRN_OBJECT *)NULL, (short)0 };

static TRAIL trail_empt[] = {&i_empty_bottom,    (short)0,
                             (PRN_OBJECT *)NULL, (short)0 };

static TRAIL trail_tran[] = {&i_to_transport,    (short)0,
                             &i_empty_bottom,    (short)0,
                             (PRN_OBJECT *)NULL, (short)0 };

static HEAD  head_none[]  = {(PRN_OBJECT *)NULL, (short)0 };

static HEAD  head_init[]  = {&i_promo,           (short)0,
                             &xr_zr_init,        (short)0,
                             (PRN_OBJECT *)NULL, (short)0 };

static HEAD  head_tran[]  = {&i_promo,           (short)0,
                             &i_customer,        (short)0,
                             &i_transported,     (short)0,
                             (PRN_OBJECT *)NULL, (short)0 };

static HEAD  head_prom[]  = {&i_promo,           (short)0,
                             (PRN_OBJECT *)NULL, (short)0 };

/*----------------------------------------------------------------------------*/
/*                           bp_forward_skip()                                */
/*----------------------------------------------------------------------------*/
static short bp_forward_skip(short group, short prnt)
{
  write_ln(&forward_skip, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_forward_skip() */

/*----------------------------------------------------------------------------*/
/*                           bp_reverse_skip()                                */
/*----------------------------------------------------------------------------*/
static short bp_reverse_skip(short group, short prnt)
{
  write_ln(&reverse_skip, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_reverse_skip() */

/*----------------------------------------------------------------------------*/
/*                           bp_xr_zr_init                                    */
/*----------------------------------------------------------------------------*/
static short bp_xr_zr_init(short group, short prnt)
{
  
  write_ln(&init_prn,   trail_none, head_none, 0, prnt);
  write_ln(&xr_zr_init, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_xr_zr_init */

/*----------------------------------------------------------------------------*/
/*                           bp_zr_per_till                                   */
/*----------------------------------------------------------------------------*/
/* Prints a Z-read off the till (i.e. X-reads of all the cashiers who worked  */
/* on this till.                                                              */
/*----------------------------------------------------------------------------*/
static short bp_zr_per_till(short group, short prnt)
{
  TM_INDX cur_indx, old_indx;
  short cur_cashier_no;
  short already_processed;

  cur_indx = tm_frst(TM_SHFT_NAME, (void*)&c_shft);
  cur_cashier_no=c_shft.cashier;
  /*prnt = prnt + 1;*/
  write_ln(&zr_connect,        trail_none, head_init, 0, prnt);
  write_ln(&zr_header,         trail_none, head_init, 0, prnt);
  write_ln(&xr_zr_paym_header, trail_none, head_init, 0, prnt);

  while (cur_indx>=0) {
    already_processed = 0;
    old_indx=tm_prev(TM_SHFT_NAME, (void*)&c_shft);
    while (old_indx>=0) {
      if (cur_cashier_no==c_shft.cashier) {
        already_processed = 1;
        break;
      }
      old_indx=tm_prev(TM_SHFT_NAME, (void*)&c_shft);
    }
    if (!already_processed) {
      tm_read_nth(TM_SHFT_NAME, (void*)&c_shft, cur_indx);
      write_ln(&zr_paym_totals_per_cshr, trail_empt, head_init, 0, prnt);
    }
    tm_read_nth(TM_SHFT_NAME, (void*)&c_shft, cur_indx);
    cur_indx=tm_next(TM_SHFT_NAME, (void*)&c_shft);
    cur_cashier_no=c_shft.cashier;
  }
  write_ln(&zr_paym_totals_per_till, trail_empt, head_init, 0, prnt);
  write_ln(&xr_zr_time_header,       trail_empt, head_init, 0, prnt);
  cur_indx=tm_frst(TM_SHFT_NAME, (void*)&c_shft);
  while (cur_indx>=0) {
    write_ln(&xr_zr_shift_per_time, trail_none, head_init, 0, prnt);
    tm_read_nth(TM_SHFT_NAME, (void*)&c_shft, cur_indx);
    cur_indx=tm_next(TM_SHFT_NAME, (void*)&c_shft);
  }
  write_ln(&zr_till_totals,   trail_empt, head_init, 0, prnt);

  /*Se usa solo para factura grande
  write_ln(&i_to_end_of_page, trail_empt, head_init, 0, prnt);
  write_ln(&i_empty_bottom,   trail_empt, head_init, 0, prnt);
  write_ln(&i_promo,          trail_empt, head_init, 0, prnt);
  */
  return(SUCCEED);
} /* bp_zr_per_till */

/*----------------------------------------------------------------------------*/
/*                           bp_xr_per_cashier                                */
/*----------------------------------------------------------------------------*/
/* Prints an X-read of the requested cashier.                                 */
/*----------------------------------------------------------------------------*/
static short bp_xr_per_cashier(short group, short prnt)
{
  short strt_indx, cur_indx, xr_cshr;

  /*                                                                         */
  /* c_shft holds the cashier number. Re-step to this item in tm to get      */
  /* the index of c_shift.                                                   */
  /*                                                                         */

  xr_cshr=c_shft.cashier;
  strt_indx=tm_frst(TM_SHFT_NAME, (void*)&c_shft);

  while (strt_indx>=0 && c_shft.cashier!=xr_cshr) {
    strt_indx=tm_next(TM_SHFT_NAME, (void*)&c_shft);
  }

  if (c_shft.cashier != xr_cshr) {
    return(SUCCEED);
  }
   /*prnt=prnt+1;*/   /*X y Z em tirrilla Jonathan*/
  cur_indx=strt_indx;
  do {
    if (c_shft.cashier==xr_cshr) {
      write_ln(&xr_header,         trail_empt, head_init, 0, prnt);
      write_ln(&xr_zr_paym_header, trail_empt, head_init, 0, prnt);
      write_ln(&xr_paym_amnts,     trail_empt, head_init, 0, prnt);
    }
    tm_read_nth(TM_SHFT_NAME, (void*)&c_shft, cur_indx);
    cur_indx=tm_next(TM_SHFT_NAME, (void*)&c_shft);
  } while (cur_indx>=0);                      /* until end of TM_SHFT_GROUP  */

  tm_read_nth(TM_SHFT_NAME, (void*)&c_shft, strt_indx);
  write_ln(&xr_paym_totals_per_cshr, trail_empt, head_init, 0, prnt);
  write_ln(&xr_zr_time_header,       trail_empt, head_init, 0, prnt);

  cur_indx=strt_indx;
  tm_read_nth(TM_SHFT_NAME, (void*)&c_shft, strt_indx);
  do {
    if (c_shft.cashier==xr_cshr) {
      write_ln(&xr_zr_shift_per_time, trail_empt, head_init, 0, prnt);
    }
    tm_read_nth(TM_SHFT_NAME, (void*)&c_shft, cur_indx);
    cur_indx=tm_next(TM_SHFT_NAME, (void*)&c_shft);
  } while (cur_indx>=0);

  tm_read_nth(TM_SHFT_NAME, (void*)&c_shft, strt_indx);
  write_ln(&xr_till_totals, trail_empt, head_init, 0, prnt);
  tm_read_nth(TM_SHFT_NAME, (void*)&c_shft, strt_indx);

  /* se usa solo para factura grande Jonathan
  write_ln(&i_to_end_of_page, trail_empt, head_init, group, prnt);
  write_ln(&i_empty_bottom,   trail_empt, head_init, group, prnt);
  write_ln(&i_promo,          trail_empt, head_init, group, prnt);
   */

  return(SUCCEED);
} /* bp_xr_per_cashier */

/*----------------------------------------------------------------------------*/
/*                           bp_i_inv_init                                    */
/*----------------------------------------------------------------------------*/
/* Initialize global variables at the start of an invoice.                    */
/*----------------------------------------------------------------------------*/
static short bp_i_inv_init(short group, short prnt)
{
  write_ln(&init_prn, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_inv_init */

/*----------------------------------------------------------------------------*/
/*                           bp_i_promo                                       */
/*----------------------------------------------------------------------------*/
/* This function prints 4 lines of promotional text.                          */
/*----------------------------------------------------------------------------*/
static short bp_i_promo(short group, short prnt)
{
  write_ln(&i_promo, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_promo */

/*----------------------------------------------------------------------------*/
/*                           bp_i_customer                                    */
/*----------------------------------------------------------------------------*/
/* This function is called after the customer number is scanned; prints 3     */
/* lines customer address.                                                    */
/*----------------------------------------------------------------------------*/
static short bp_i_customer(short group, short prnt)
{
  write_ln(&i_customer, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_customer */

/*----------------------------------------------------------------------------*/
/*                           bp_i_article_line                                */
/*----------------------------------------------------------------------------*/
static short bp_i_article_line(short group, short prnt)
{
  write_ln(&i_article_line, trail_tran, head_tran, 0, prnt);
  return(SUCCEED);
} /* bp_i_article_line */

/*----------------------------------------------------------------------------*/
/*                           bp_i_deposit_line                                */
/*----------------------------------------------------------------------------*/
static short bp_i_deposit_line(short group, short prnt)
{
  write_ln(&i_deposit_line, trail_tran, head_tran, 0, prnt);
  return(SUCCEED);
} /* bp_i_deposit_line */

/*----------------------------------------------------------------------------*/
/*                           bp_i_discount_line                               */
/*----------------------------------------------------------------------------*/
static short bp_i_discount_line(short group, short prnt)
{
  write_ln(&i_discount_line, trail_tran, head_tran, 0, prnt);
  return(SUCCEED);
} /* bp_i_discount */

/*----------------------------------------------------------------------------*/
/*                           bp_i_total_lines                                 */
/*----------------------------------------------------------------------------*/
static short bp_i_total_lines(short group, short prnt)
{
  write_ln(&i_total_lines,  trail_tran, head_tran, 0, prnt);
  write_ln(&i_empty_bottom, trail_tran, head_tran, 0, prnt);
  if (copy_invoice == YES) {
    copy_invoice_active = !copy_invoice_active;
  }
  write_ln(&i_promo,        trail_tran, head_tran, 0, prnt);

  return(SUCCEED);
} /* bp_i_total_lines */

/*----------------------------------------------------------------------------*/
/*                           bp_i_void_inv                                    */
/*----------------------------------------------------------------------------*/
static short bp_i_void_inv(short group, short prnt)
{
  write_ln(&i_void_inv,     trail_empt, head_prom, 0, prnt);
  write_ln(&i_empty_bottom, trail_empt, head_prom, 0, prnt);
  write_ln(&i_promo,        trail_empt, head_prom, 0, prnt);

  return(SUCCEED);
} /* bp_i_void_inv */

/*----------------------------------------------------------------------------*/
/*                           bp_i_start_training                              */
/*----------------------------------------------------------------------------*/
static short bp_i_start_training(short group, short prnt)
{
  write_ln(&i_to_end_of_page, trail_none, head_none, 0, prnt);
  write_ln(&i_empty_bottom,   trail_none, head_none, 0, prnt);
  write_ln(&i_promo,          trail_none, head_none, 0, prnt);

  return(SUCCEED);
} /* bp_i_start_training */

/*----------------------------------------------------------------------------*/
/*                           bp_i_end_training                                */
/*----------------------------------------------------------------------------*/
short bp_i_end_training(short group, short prnt)
{
  write_ln(&i_to_end_of_page, trail_none, head_none, 0, prnt);
  write_ln(&i_empty_bottom,   trail_none, head_none, 0, prnt);
  write_ln(&i_promo,          trail_none, head_none, 0, prnt);

  return(SUCCEED);
} /* bp_i_end_training */

/*----------------------------------------------------------------------------*/
/*                           bp_i_subtotal                                    */
/*----------------------------------------------------------------------------*/
static short bp_i_subtotal(short group, short prnt)
{
  write_ln(&i_subtotal,     trail_tran, head_tran, 0, prnt);
  write_ln(&i_empty_bottom, trail_tran, head_tran, 0, prnt);
  write_ln(&i_promo,        trail_tran, head_tran, 0, prnt);
  write_ln(&i_customer,     trail_tran, head_tran, 0, prnt);

  return(SUCCEED);
} /* bp_i_subtotal */

/*----------------------------------------------------------------------------*/
/*                           bp_i_skip_one_line                               */
/*----------------------------------------------------------------------------*/
static short bp_i_skip_one_line(short group, short prnt)
{
  write_ln(&i_skip_one_line, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_skip_one_line */

/*----------------------------------------------------------------------------*/
/*                           bp_i_empty_bottom                                */
/*----------------------------------------------------------------------------*/
/* Prints a number of empty lines at the bottom of the invoice. The number of */
/* empty lines is stored in the variable LN_BOT.                              */
/*----------------------------------------------------------------------------*/
static short bp_i_empty_bottom(short group, short prnt)
{
  write_ln(&i_empty_bottom, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_empty_bottom */

/*----------------------------------------------------------------------------*/
/*                           bp_i_to_transport                                */
/*----------------------------------------------------------------------------*/
/* Prints a number of lines at the bottom of the invoice with the transported */
/* value to the next page. The number of 'transport lines' is stored in the   */
/* variable LN_TRANS.                                                         */
/*----------------------------------------------------------------------------*/
static short bp_i_to_transport(short group, short prnt)
{
  write_ln(&i_to_transport, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_to_transport */

/*----------------------------------------------------------------------------*/
/*                           bp_i_transported                                 */
/*----------------------------------------------------------------------------*/
/* Prints one line at the top of the invoice (next page) with the transported */
/* value off the previous page.                                               */
/*----------------------------------------------------------------------------*/
static short bp_i_transported(short group, short prnt)
{
  write_ln(&i_transported, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_transported */

/*----------------------------------------------------------------------------*/
/*                           bp_i_msam_header                                 */
/*----------------------------------------------------------------------------*/
static short bp_i_msam_header (short group, short prnt)
{
  write_ln(&i_msam_header,  trail_tran, head_tran, 0, prnt);
  return(SUCCEED);
} /* bp_i_msam_header */

/*----------------------------------------------------------------------------*/
/*                           bp_i_msam_line                                   */
/*----------------------------------------------------------------------------*/
static short bp_i_msam_line (short group, short prnt)
{
  write_ln(&i_msam_line,  trail_tran, head_tran, 0, prnt);
  return(SUCCEED);
} /* bp_i_msam_line */

/*----------------------------------------------------------------------------*/
/*                           bp_i_msam_total                                  */
/*----------------------------------------------------------------------------*/
static short bp_i_msam_total (short group, short prnt)
{
  write_ln(&i_msam_total,  trail_tran, head_tran, 0, prnt);
  return(SUCCEED);
} /* bp_i_msam_total */

/*----------------------------------------------------------------------------*/
/*                           bp_i_cust_fee_line                               */
/*----------------------------------------------------------------------------*/
static short bp_i_cust_fee_line(short group, short prnt)
{
  write_ln(&i_cust_fee_line, trail_tran, head_tran, 0, prnt);
  return(SUCCEED);
} /* bp_i_cust_fee_line */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_header                                */
/*----------------------------------------------------------------------------*/
static short bp_i_small_header(short group, short prnt)
{
  write_ln(&i_small_header, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_header */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_article                               */
/*----------------------------------------------------------------------------*/
static short bp_i_small_article(short group, short prnt)
{
  write_ln(&i_small_article, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_article */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_deposit                               */
/*----------------------------------------------------------------------------*/
static short bp_i_small_deposit(short group, short prnt)
{
  write_ln(&i_small_deposit, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_deposit */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_discount                              */
/*----------------------------------------------------------------------------*/
static short bp_i_small_discount(short group, short prnt)
{
  write_ln(&i_small_discount, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_discount */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_total                                 */
/*----------------------------------------------------------------------------*/
static short bp_i_small_total(short group, short prnt)
{
  write_ln(&i_small_total, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_total */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_tax                                   */
/*----------------------------------------------------------------------------*/
static short bp_i_small_tax(short group, short prnt)
{
  write_ln(&i_small_tax, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_tax */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_footer                                */
/*----------------------------------------------------------------------------*/
static short bp_i_small_footer(short group, short prnt)
{
  write_ln(&i_small_footer, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_footer */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_void                                  */
/*----------------------------------------------------------------------------*/
static short bp_i_small_void(short group, short prnt)
{
  write_ln(&i_small_void, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_void */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_promo                                 */
/*----------------------------------------------------------------------------*/
static short bp_i_small_promo(short group, short prnt)
{
  write_ln(&i_small_promo, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_promo */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_cust_fee                              */
/*----------------------------------------------------------------------------*/
static short bp_i_small_cust_fee(short group, short prnt)
{
  write_ln(&i_small_cust_fee, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_cust_fee */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_msam_header                           */
/*----------------------------------------------------------------------------*/
static short bp_i_small_msam_header(short group, short prnt)
{
  write_ln(&i_small_msam_header, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_msam_header */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_msam_line                             */
/*----------------------------------------------------------------------------*/
static short bp_i_small_msam_line(short group, short prnt)
{
  write_ln(&i_small_msam_line, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_msam_line */

/*----------------------------------------------------------------------------*/
/*                           bp_i_small_msam_total                            */
/*----------------------------------------------------------------------------*/
static short bp_i_small_msam_total(short group, short prnt)
{
  write_ln(&i_small_msam_total, trail_none, head_none, 0, prnt);
  return(SUCCEED);
} /* bp_i_small_msam_total */

/*----------------------------------------------------------------------------*/
/*                           bp_i_dummy                                       */
/*----------------------------------------------------------------------------*/
static short bp_i_dummy(short group, short prnt)
{
  return(SUCCEED);
} /* bp_i_dummy */


/*----------------------------------------------------------------------------*/
/*                           bp_alt_fn                                        */
/*----------------------------------------------------------------------------*/
/* NOTE: do not change the indexes of the functions in the array without      */
/* changing the defines in Pos_bp1.h.                                         */
/*----------------------------------------------------------------------------*/
extern short (*bp_alt_fn[])(short, short) = {
/*                                                                           */
/* Offset: Function:                           Offset-name:                  */
/*                                                                           */
/* 00 */   bp_i_inv_init,                   /* INV_INIT                      */
/* 01 */   bp_i_promo,                      /* PROMO                         */
/* 02 */   bp_i_customer,                   /* CUSTOMER                      */
/* 03 */   bp_i_article_line,               /* ARTICLE_LINE                  */
/* 04 */   bp_i_deposit_line,               /* DEPOSIT_LINE                  */
/* 05 */   bp_i_discount_line,              /* DISCOUNT_LINE                 */
/* 06 */   bp_i_total_lines,                /* TOTAL_LINES                   */
/* 07 */   bp_i_dummy,                      /* DUMMY                         */
/* 08 */   bp_i_void_inv,                   /* VOID_INV                      */
/* 09 */   bp_i_start_training,             /* START_TRAINING                */
/* 10 */   bp_i_end_training,               /* END_TRAINING                  */
/* 11 */   bp_i_subtotal,                   /* SUBTOTAL                      */
/* 12 */   bp_i_skip_one_line,              /* SKIP_ONE_LINE                 */
/* 13 */   bp_i_to_transport,               /* TO_TRANSPORT                  */
/* 14 */   bp_i_transported,                /* TRANSPORTED                   */
/* 15 */   bp_i_empty_bottom,               /* EMPTY_BOTTOM                  */
/* 16 */   bp_i_cust_fee_line,              /* CUST_FEE_LINE                 */
/* 17 */   bp_i_msam_header,                /* MSAM DISCOUNT SECTION HEADER  */
/* 18 */   bp_i_msam_line,                  /* MSAM DISCOUNT SECTION LINE    */
/* 19 */   bp_i_msam_total,                 /* MSAM DISCOUNT SECTION TOTALS  */
/* 20 */   bp_i_small_header,               /* HEADER FOR SMALL INVOICE      */
/* 21 */   bp_i_small_article,              /* ARTICLE PART SMALL INVOICE    */
/* 22 */   bp_i_small_deposit,              /* DEPOSIT PART SMALL INVOICE    */
/* 23 */   bp_i_small_discount,             /* DISCOUNT PART SMALL INVOICE   */
/* 24 */   bp_i_small_total,                /* TOTAL SECTION SMALL INVOICE   */
/* 25 */   bp_i_small_tax,                  /* TAX SECTION SMALL INVOICE     */
/* 26 */   bp_i_small_footer,               /* FOOTER FOR SMALL INVOICE      */
/* 27 */   bp_i_small_void,                 /* VOID FOR SMALL INVOICE        */
/* 28 */   bp_i_small_promo,                /* PROMO FOR SMALL INVOICE       */
/* 29 */   bp_i_small_cust_fee,             /* CUST FEE FOR SMALL INVOICE    */
/* 30 */   bp_i_small_msam_header,          /* MSAM HEADER FOR SMALL INVOICE */
/* 31 */   bp_i_small_msam_line,            /* MSAM LINE FOR SMALL INVOICE   */
/* 32 */   bp_i_small_msam_total,           /* MSAM TOTAL FOR SMALL INVOICE  */
/* 33 */   bp_i_dummy,
/* 34 */   bp_i_dummy,
/* 35 */   bp_i_dummy,
/* 36 */   bp_forward_skip,                 /* FORWARD_SKIP                  */
/* 37 */   bp_reverse_skip,                 /* REVERSE_SKIP                  */
/* 38 */   bp_xr_zr_init,                   /* XR_ZR_INIT                    */
/* 39 */   bp_zr_per_till,                  /* ZR_PER_TILL                   */
/* 40 */   bp_xr_per_cashier,               /* XR_PER_CASHIER                */
/* 41 */   (void *)NULL
};

extern const short bp_number = sizeof(bp_alt_fn) / sizeof(void *) - 1;
