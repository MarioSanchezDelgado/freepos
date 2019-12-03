/*
 *     Module Name       : POS_MSAM.CPP
 *
 *     Type              : Pos Multisam discount processing
 *
 *
 *     Author/Location   : Getronics, Distribution & Retail, Nieuwegein
 *
 *     Copyright Getronics N.V.
 *               Bakenmonde 1
 *               3434KK NIEUWEGEIN
 *               The Netherlands
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 06-Jan-2000 Initial Release WinPOS                                  P.M.
 * --------------------------------------------------------------------------
 * 12-Dec-2000 Solved bugs in action_allowed() (potential endless loop)R.N.B.
 * --------------------------------------------------------------------------
 * 20-Feb-2001 Bug fix: use floor_price in distr_amount for rounding   P.M.
 * --------------------------------------------------------------------------
 * 01-Mar-2001 Bug fix: action_text copies to ansi_action_text when    P.M.
 *                      continueing with the next record.
 * --------------------------------------------------------------------------
 * 05-Mar-2001 Bug fix: action_allowed returns 0 when maximum is       J.H. 
 *                      reached.
 * --------------------------------------------------------------------------
 * 04-May-2001 Bugfix: Articles with manual discount are excluded from P.M.
 *             MultiSave discount
 * --------------------------------------------------------------------------
 * 22-Jan-2002 Use define of MAX_AMOUNT_ON_POS                         M.W.
 * --------------------------------------------------------------------------
 * 20-Mar-2002 Bugfix: Added premium discount as discount type.        R.N.B.
 *             Set discount_qty to art_qty*times.
 * --------------------------------------------------------------------------
 * 02-May-2002 Bug fix: send promotions per 20 to the mml engine
 *             otherwise the engine may run out of stack space. Also
 *             improved the error message when there is an error during
 *             parsing.                                                R.N.B.
 * -------------------------------------------------------------------------- 
 * 07-May-2002 Solved bug in add_article: c_item must be initialised.  R.N.B.
 * -------------------------------------------------------------------------- 
 * 22-Jul-2002 Multisam Phase 3.                                         P.M.
 * --------------------------------------------------------------------------
 * 10-Jun-2003 Changed processing of multisam actions. If an error is
 *             detected the rest of the actions will still be
 *             processed.                                              J.D.M.
 * -------------------------------------------------------------------------- 
 * 27-Aug-2003 Added disc_incl/excl_amnt_per_vat_cd in ACTION_RESULT
 *             to be able to calculate total carried forward totals
 *             correctly.                                              J.D.M.
 * --------------------------------------------------------------------------
 * 05-Sep-2003 Bugfix: Display of wrong line texts corrected.
 *             Action results were partially filled with wrong values. J.D.M.
 * --------------------------------------------------------------------------
 * 14-Jan-2004 Bugfix: Last action could still have a problem with
 *             wrong line texts and wrong action results values. An
 *             extra memcpy to &msam from &msam_parse after the last
 *             action is read from ctree solved this problem.          J.D.M.
 * --------------------------------------------------------------------------
 * 22-Mar-2004 Added vat totals to the action results.                 J.D.M.
 * --------------------------------------------------------------------------
 * 14-Oct-2004 Changed to cpp.                                         J.D.M.
 * --------------------------------------------------------------------------
 * 14-Oct-2004 Put multisam actions in memory for faster performance.  J.D.M.
 * --------------------------------------------------------------------------
 * 22-Oct-2004 Removed strlen(action_text_ansi) in msam_get_char()
 *             from if-statement and put it into a static variable
 *             len_action_text_ansi for performance reasons.
 *             Also performance issue in usr_msg_hndl().               J.D.M.
 * --------------------------------------------------------------------------
 * 23-Jun-2005 Bugfix: c_item initialised in add_article.
 *             Bugfix: Discount on GIFT article i.s.o. article which
 *             activates the multisam action. To prevent difficulties
 *             with different vatrates between these articles          M.W.
 * -------------------------------------------------------------------------- 
 * 06-Jul-2005 Bugfix: Rounding of Multisam discounts in case of 
 *             inclusive amounts on invoice                            M.W.
 * --------------------------------------------------------------------------
 */

/* THIS HERE BELOW COMES FROM MAKRO HOLLAND AND MAYBE SHOULD BE FIXED */
/* IN OTHER COUNTRIES AS WELL!!! SO CHECK IT AND REMOVE THE COMMENT!: */

/* -------------------------------------------------------------------------- 
 * 08-May-2001 Bugfix in MML_TYP_GTCUS: card type was not passed       P.M.
 * -------------------------------------------------------------------------- 
 * 27-Feb-2003 Bugfix: rounding of quantity not anymore in mmldll
 *             (Rmml.c), but in here in distr_amount().                J.D.M.
 * -------------------------------------------------------------------------- 
 */

#pragma warning(push) /* Push current warning level to stack. */
#pragma warning(disable: 4786) /* Disable warning 4786.       */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h"

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "sll_mgr.h"
#include "llist.h"
#include "pos_msam.h"
#include "rmml.h"

#include "date_tls.h"
#include "tm_mgr.h"
#include "tot_mgr.h"
#include "intrface.h"
#include "pos_recs.h"
#include "comm_tls.h"
#include "pos_tm.h"
#include "wpos_mn.h"
#include "inp_mgr.h"
#include "state.h"
#include "stri_tls.h"
#include "pos_inp.h"
#include "st_main.h"
#include "pos_func.h"
#include "pos_tot.h"
#include "pos_com.h"
#include "pos_errs.h"
#include "err_mgr.h"

#define ARTICLE_LVL  1
#define INVOICE_LVL  2
#define DISC_AMNT    1
#define DISC_PERC    2
#define RATIO_AMNT   1
#define RATIO_LINE   2

#define CURRENT_ITEM 1
#define MML_LIST     2

#define USE_MSAM_MEMORY_READ
static short msam_read_action_line();
static short msam_read_action_line_memory();

SLL           action_results;
SLL           crazy_art_hist;
ACTION_RESULT prn_msam_action;

static  short nbr_gifts = 0;
static  ACTION_RESULT action_result;
static  MMML_DEF msam, msam_buf, msam_parse;

static  short  action_text_pos = -1;
static  short  msam_action_count = 0;
static  short  msam_stop_reading = FALSE;
static  short  last_nbr_mml_errors = 0;
static  short  msam_read_first_action = TRUE;
static  time_t start_of_parsing;
#define PARSING_TIME_OUT_PER_LINE  3
#define MAX_NBR_MSAM_ERRORS        10000

/*---------------------------------------------------------------------------*/
/*                              get_value                                    */
/*---------------------------------------------------------------------------*/
double get_value(long art_no)
{
  ARTICLE_DEF article;
  short status;

  article.art_no = art_no;
  status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
                      (void*)&article, (short) keyEQL);
                      
  if(status == SUCCEED) {
    return(article.sell_pr);
  }
  else {
    return(0.0);
  }
} /* get_value */

/*---------------------------------------------------------------------------*/
/*                            add_article                                    */
/*---------------------------------------------------------------------------*/
int add_article(long art_no, short times)
{
  TM_INDX indx;
  ARTICLE_DEF article, deposit;
  short status;

  memset(&c_item, 0, sizeof(TM_ITEM_GROUP));
  c_item.arti.base.art_no= art_no;
  c_item.arti.base.qty   = times;

  /*                                                                   */
  /* Step 2: Get article data from PLU and handle exceptions.          */
  /*                                                                   */
  article.art_no = c_item.arti.base.art_no;
  status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
                         (void*)&article, (short) keyEQL);
  if (status != SUCCEED) {
    return(FAIL);
  }
  else if (article.art_no != c_item.arti.base.art_no) {
    return(FAIL);
  }

  _tcsncpy(c_item.arti.pack_type, article.pack_type, 3);
  c_item.arti.art_ind             = article.type;
  c_item.arti.mmail_no            = article.mmail_no;
  c_item.arti.items_per_pack      = article.cont_sell_unit;
  c_item.arti.base.vat_no         = article.vat_no;
  c_item.arti.base.vat_perc       = get_vat_perc(article.vat_no);
  c_item.arti.base.art_grp_no     = article.art_grp_no;
  c_item.arti.base.art_grp_sub_no = article.art_grp_sub_no;
  c_item.arti.base.dept_cd        = article.dept_cd;

  c_item.arti.base.arti_retention_ind    = article.arti_retention_ind;  //v3.6.1 acm -
  c_item.arti.base.arti_perception_ind   = article.arti_perception_ind; //v3.6.1 acm -
  c_item.arti.base.arti_rule_ind         = article.arti_rule_ind;       //v3.6.1 acm -
  c_item.arti.base.percep_amount         = 0;                           //v3.6.1 acm -



  _tcscpy(c_item.arti.base.descr, article.descr);
  _tcscpy(c_item.arti.reg_no, article.reg_no);

  if (genvar.price_incl_vat == INCLUSIVE) {
    c_item.arti.base.price =
              floor_price(calc_incl_vat(article.sell_pr, article.vat_no));
  }
  else {
    c_item.arti.base.price = floor_price(article.sell_pr);
  }

  /*                                                                   */
  /* Step 2d: If article is ok till now, calculate goods-value.        */
  /*          - If it is a barcode, first decode the barcode and then  */
  /*            calculate the goods-value                              */
  /*                                                                   */
  if (calc_gds_value(ART_GROUP) != SUCCEED) {
    return(FAIL);
  }

  /*                                                                   */
  /* Step 3:  If this article has a deposit, get it and handle         */
  /*          exceptions                                               */
  /*                                                                   */
  if (article.type != ART_IND_DEPOSIT &&
      article.art_no_deposit != 0L) {

    deposit.art_no = article.art_no_deposit;
    status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
                           (void*)&deposit, (short) keyEQL);
    if (status != SUCCEED) {
      return(FAIL);
    }
    else if (deposit.art_no != article.art_no_deposit) {
      return(FAIL);
    }

    /*                                                                   */
    /* Step 3a: If deposit is blocked, ask for S-key.                    */
    /*                                                                   */

    c_item.depo.base.qty = c_item.arti.base.qty;
    _tcsncpy(c_item.depo.pack_type, deposit.pack_type, 3);
    c_item.depo.mmail_no            = deposit.mmail_no;
    c_item.depo.base.art_no         = deposit.art_no;
    c_item.depo.base.vat_no         = deposit.vat_no;
    c_item.depo.base.vat_perc       = get_vat_perc(deposit.vat_no);
    c_item.depo.base.art_grp_no     = deposit.art_grp_no;
    c_item.depo.base.art_grp_sub_no = deposit.art_grp_sub_no;
    c_item.depo.base.dept_cd        = deposit.dept_cd;
    _tcscpy(c_item.depo.base.descr, deposit.descr);
    _tcscpy(c_item.depo.reg_no, deposit.reg_no);

    if (genvar.price_incl_vat == INCLUSIVE) {
      c_item.depo.base.price =
              floor_price(calc_incl_vat(deposit.sell_pr, deposit.vat_no));
    }
    else {
      c_item.depo.base.price = floor_price(deposit.sell_pr);
    }

    /*                                                                   */
    /* Step 3c: If deposit is ok till now, calculate goods-value.        */
    /*                                                                   */
    if (calc_gds_value(DEPOSIT_GROUP) != SUCCEED) {
      return(FAIL);
    }
  } /* if, end of the deposit section. */


  /*                                                                       */
  /* Accept active_item.                                                   */
  /*                                                                       */
  /*  Previously added C_ITEM type scroll-lines are modified to the new    */
  /*  TM_INDX retrieved from the tm_appe() call.                           */
  /*                                                                       */

  if (proc_active_item() == SUCCEED) {

    /*                                                                     */
    /* The approval section in the state-engine for the state Invoicing    */
    /* ensures that not more than TM_ITEM_MAXI-1 items are invoiced.       */
    /* So, do not check the indx for a TM_BOF_EOF error.                   */
    /*                                                                     */

    indx=tm_appe(TM_ITEM_NAME, (void*)&c_item);
    ++nbr_gifts;

    ++nbr_inv_lines;
    if (c_item.depo.base.goods_value != 0.0) {  /* Also count deposits     */
      ++nbr_inv_lines;
    }

    last_item   =indx;
    display_item=indx;
    return(SUCCEED);
  }
  else {                                            /* Price too large.    */
    return(FAIL);
  }
} /* add_article */

/*---------------------------------------------------------------------------*/
/*                            remove_item                                    */
/*---------------------------------------------------------------------------*/
short remove_item(TM_INDX item_to_void)
{
  TM_INDX indx;
  TM_ARTI_BASE *base;
  double item_total;
  short group[] = { ART_GROUP, DEPOSIT_GROUP, DISCNT_GROUP, 0 };
  SUB_ITEM_TOTAL sub_item_totals;
  short i=0, mul_tot = 1;

  /*                                                                       */
  /* Void the item pointed to by item_to_void                              */
  /*                                                                       */

  indx=tm_read_nth(TM_ITEM_NAME, (void*)&c_item, item_to_void);
  if (indx == item_to_void) {

    /* Before voiding, look for total overflow!                            */
    item_total=0;
    for (i=0; group[i]; i++) {
      calc_sub_item_totals(C_ITEM, group[i], &sub_item_totals);
      item_total+=sub_item_totals.incl_gds_val;
    }

    if ( genvar.ind_price_dec == DECIMALS_YES ) {
      mul_tot = 100;
    }

    if ( (double)fabs( mul_tot * (tot_ret_double(TOT_GEN_INCL)-item_total) )
                     <= MAX_AMOUNT_ON_POS ) {
      /* No total overflow, so perform the void.                           */

      c_item.voided=TRUE;

      ++nbr_void_lines;
      --nbr_inv_lines;
      if (c_item.depo.base.goods_value != 0.0) {  /* Also count deposits   */
        ++nbr_void_lines;
        --nbr_inv_lines;
      }

      /* Second, reverse the sign of all sub-items qty's and gds-val's     */
      for (i=0; group[i]; i++) {
        switch (group[i]) {
          case ART_GROUP:
            base=&c_item.arti.base;
            break;
          case DISCNT_GROUP:
            base=&c_item.disc.base;
            break;
          case DEPOSIT_GROUP:
            base=&c_item.depo.base;
            break;
          default:
            break;
        }
        if (base->price!=0.0) {
          base->goods_value*=-1;
          base->qty*=-1;

          /*                                                               */
          /* Recalculate all (sub)totals:                                  */
          /*  - For each sub-item within the active-item, calculate:       */
          /*    . goods-value in- and exclusive,                           */
          /*    . vat amount                                               */
          /*    . vat-code                                                 */
          /*    . number of packs                                          */
          /*                                                               */

          /* Recalc sub-item because now it's values are negative.         */
          if (calc_sub_item_totals(C_ITEM, group[i], &sub_item_totals)
            ==SUCCEED) {
            recalc_totals(&sub_item_totals);
          }
        }
      }

      tm_remv(TM_ITEM_NAME);

      return(SUCCEED);
    }
    else {
      return(FAIL);
    }
  }

  return(FAIL);
} /* remove_item */

/*---------------------------------------------------------------------------*/
/*                            remove_gifts                                   */
/*---------------------------------------------------------------------------*/
int remove_gifts(void)
{
  TM_INDX indx;

  while (nbr_gifts>0) {
    indx = tm_last(TM_ITEM_NAME, (void*)&c_item);
    remove_item(indx);
    nbr_gifts--;
  }

  return(SUCCEED);
} /* remove_gifts */

/*---------------------------------------------------------------------------*/
/*                      init_invoice_msam_disc_totals                        */
/*---------------------------------------------------------------------------*/
void init_invoice_msam_disc_totals(void)
{
  short i;

  for(i=0; i<=9; i++) {
    tot_reset_double((short)(MSAM_DISC_TOT_EXCL_0+i));
    tot_reset_double((short)(MSAM_DISC_TOT_INCL_0+i));
    tot_reset_double((short)(MSAM_DISC_TOT_VAT_0+i));
  }
  tot_reset_double(MSAM_DISC_TOT_GEN_EXCL);
  tot_reset_double(MSAM_DISC_TOT_GEN_VAT);
  tot_reset_double(MSAM_DISC_TOT_GEN_INCL);
  tot_reset_double(MSAM_TOT_DISC1);
  tot_reset_double(MSAM_TOT_DISC2);
} /* init_invoice_msam_disc_totals */

/*---------------------------------------------------------------------------*/
/*                           init_disc_tots                                  */
/*---------------------------------------------------------------------------*/
int init_disc_tots(void)
{
  TM_INDX indx;

  init_invoice_msam_disc_totals();

  indx = tm_frst(TM_ITEM_NAME, (void*)&c_item);
  while ( indx != TM_BOF_EOF ) {
    c_item.arti.msam_disc1       = 0.0;
    c_item.arti.msam_disc2       = 0.0;
    c_item.arti.msam_mmail_no1   = 0;
    c_item.arti.msam_mmail_no2   = 0;
    c_item.arti.msam_maction_no1 = 0;
    c_item.arti.msam_maction_no2 = 0;

    indx = tm_upda(TM_ITEM_NAME, (void*)&c_item);
    indx = tm_next(TM_ITEM_NAME, (void*)&c_item);
  }

  return (SUCCEED);
} /* init_disc_tots */

/*---------------------------------------------------------------------------*/
/*                         reset_multisam_discounts                          */
/*---------------------------------------------------------------------------*/
int reset_multisam_discounts(void)
{
  remove_gifts();
  init_disc_tots();
  sll_remove(&action_results);

  return(SUCCEED);
} /* reset_multisam_discounts */

/*---------------------------------------------------------------------------*/
/*                               distr_amount                                */
/* Returns FAIL when LLIST l is empty.                                       */
/*---------------------------------------------------------------------------*/
int distr_amount(long mmail_no, short maction_no, short mode,
                 double disc, short disc_type, LLIST *l, short item)
{
//  printf_log("distr_amount -> mmail_no:%ld maction_no:%d mode:%d disc:%6.2f disc_type:%d itme:%d",mmail_no, maction_no, mode, disc,disc_type,item);

  double total_value,
         goods_value,
         goods_disc,
         disc_amount,
         remainder;
  short  i, j,
         nbr_lines,
         ratio_type;
  int    index;
  long   total_qty;

  action_result.disc_incl = 0.0;
  action_result.disc_excl = 0.0;
  action_result.disc_vat = 0.0;
  for (j=0; j<10; j++) {
    action_result.disc_incl_amnt_per_vat_cd[j] = 0.0;
    action_result.disc_excl_amnt_per_vat_cd[j] = 0.0;
    action_result.disc_vat_amnt_per_vat_cd[j] = 0.0;
  }
  i = 0;
  total_value = 0.0;
  total_qty   = 0;
  if (item == MML_LIST) {
    while (ll_read_elem(l, i, &index) == SUCCEED) {
      tm_read_nth(TM_ITEM_NAME, (void*)&c_item, (short) mse_invoice[index].seq_no);
      if(genvar.price_incl_vat==INCLUSIVE) {
        total_value += calc_excl_vat(c_item.arti.base.goods_value, c_item.arti.base.vat_no);
      }
      else {
        total_value += c_item.arti.base.goods_value;
      }
      total_value +=       c_item.arti.msam_disc1;
      total_value +=       c_item.arti.msam_disc2;
    	total_qty   += (long)c_item.arti.base.qty;
      i++;
    }
    nbr_lines = i;
  }
  else { /* CURRENT_ITEM */
    if(genvar.price_incl_vat==INCLUSIVE) {
      total_value += calc_excl_vat(c_item.arti.base.goods_value, c_item.arti.base.vat_no);
    }
    else {
      total_value += c_item.arti.base.goods_value;
    }
    total_value +=       c_item.arti.msam_disc1;
    total_value +=       c_item.arti.msam_disc2;
  	total_qty   += (long)c_item.arti.base.qty;
    nbr_lines = 1;
  }
  if ( nbr_lines == 0) {
    return (FAIL);
  }

  action_result.threshold_amount = total_value;
  action_result.threshold_qty    = total_qty;

  assert(disc_type==DISC_PERC || disc_type==DISC_AMNT);

  switch (disc_type) {
    case DISC_PERC:
      disc_amount = disc * total_value / 100;
      break;
    case DISC_AMNT:
      disc_amount = disc;
      break;
    default:
      disc_amount = 0.0;
      break;
  }

  if(total_value == 0.0) {
    ratio_type = RATIO_LINE;
  }
  else {
    ratio_type = RATIO_AMNT;
  }

  action_result.discount_amount = disc_amount;

  assert(mode==ARTICLE_LVL||mode==INVOICE_LVL);

  switch (mode) {
    case ARTICLE_LVL:
      action_result.excluded_amount = 0;
      break;
    case INVOICE_LVL:
      action_result.excluded_amount = tot_ret_double(TOT_GEN_EXCL)
                                    - action_result.threshold_amount;
      break;
    default:
      break;
  }

  remainder = disc_amount;
  i = 0;
  while (i<nbr_lines) {
   
    if (item == MML_LIST) { /* Read List, otherwise current_item already OK */
      ll_read_elem(l, i, &index);
      tm_read_nth(TM_ITEM_NAME, (void*)&c_item, (short) mse_invoice[index].seq_no);
    }
    if(genvar.price_incl_vat==INCLUSIVE) {
      goods_value = calc_excl_vat(c_item.arti.base.goods_value, c_item.arti.base.vat_no);
    }
    else {
      goods_value = c_item.arti.base.goods_value;
    }
    goods_value += c_item.arti.msam_disc1;
    goods_value += c_item.arti.msam_disc2;

    assert(ratio_type==RATIO_AMNT|| ratio_type==RATIO_LINE);

    switch (ratio_type) {
      case RATIO_AMNT:
        if(i<nbr_lines-1) {
          goods_disc = disc_amount/total_value*goods_value;
        }
        else {
          goods_disc = remainder;
        }
        break;
      case RATIO_LINE:
        if(i<nbr_lines-1) {
          goods_disc = disc_amount/nbr_lines;
        }
        else {
          goods_disc = remainder;
        }
        break;
      default:
        goods_disc = 0.0;
        break;
    }

    assert(mode==ARTICLE_LVL||mode==INVOICE_LVL);
    
	/* printf_log("distr_amount ARTICLE_LVL -> art_no:%d ratio:%d goods_disc:%6.2f disc_amount:%6.2f total_value:6.2%f goods_value:%6.2f",
                c_item.arti.base.art_no,
                ratio_type,
                goods_disc ,
                disc_amount,
                total_value,
                goods_value );
    */

    switch (mode) {
      case ARTICLE_LVL:
        c_item.arti.msam_disc1       -=  goods_disc;
        c_item.arti.msam_mmail_no1    =  mmail_no;
        c_item.arti.msam_maction_no1  =  maction_no;
        if (item == MML_LIST) {
          mse_invoice[index].mmail_no_1 =  mmail_no;
        }

/*  printf_log("distr_amount ARTICLE_LVL -> art_no:%d goods_disc:%6.2f msam_disc1:%6.2f msam_mmail_no1:%d msam_maction_no1:%d ",
                    
                    c_item.arti.base.art_no,
                    goods_disc,
                    c_item.arti.msam_disc1,
                    c_item.arti.msam_mmail_no1 ,
                    c_item.arti.msam_maction_no1);*/

        break;
      case INVOICE_LVL:
        c_item.arti.msam_disc2       -=  goods_disc;
        c_item.arti.msam_mmail_no2    =  mmail_no;
        c_item.arti.msam_maction_no2  =  maction_no;

/*	 printf_log("distr_amount INVOICE_LVL -> art_no:%d goods_disc:%6.2f  msam_disc2:%d msam_mmail_no2:%d msam_maction_no2:%d",
                    c_item.arti.base.art_no,
                    goods_disc,
                    c_item.arti.msam_disc2,
                    c_item.arti.msam_mmail_no2 ,
                    c_item.arti.msam_maction_no2);*/

        break;
      default:
        c_item.arti.msam_mmail_no2    =  0;
        c_item.arti.msam_maction_no2  =  0;
        break;
    }

/* TODO: Check if this code below works with genvar.price_incl_vat==INCLUSIVE and genvar.price_incl_vat==EXCLUSIVE */
/*       No rounding differences may occur on the invoice!!                                                        */
/*       After that remove this comment from this source and the source in our basis environment!!                 */

    /* Calculation of the action results */
    /* Exclusive totals */
    action_result.disc_excl_amnt_per_vat_cd[c_item.arti.base.vat_no] += goods_disc;
    action_result.disc_excl += goods_disc;

    /* Inclusive totals */
    action_result.disc_incl_amnt_per_vat_cd[c_item.arti.base.vat_no] =
        calc_incl_vat(action_result.disc_excl_amnt_per_vat_cd[c_item.arti.base.vat_no],
                      c_item.arti.base.vat_no);
    action_result.disc_incl = 0.0;
    for (j=0; j<10; j++) {
      action_result.disc_incl += calc_incl_vat(action_result.disc_excl_amnt_per_vat_cd[j], j);
    }

    /* Vat totals */
    action_result.disc_vat_amnt_per_vat_cd[c_item.arti.base.vat_no] =
                     action_result.disc_incl_amnt_per_vat_cd[c_item.arti.base.vat_no]
                   - action_result.disc_excl_amnt_per_vat_cd[c_item.arti.base.vat_no];
    action_result.disc_vat = 0.0;
    for (j=0; j<10; j++) {
      action_result.disc_vat += action_result.disc_vat_amnt_per_vat_cd[j];
    }

    /* Calculation of the totals */
    /* Exclusive totals */
    tot_add_double(  (short)(MSAM_DISC_TOT_EXCL_0 + c_item.arti.base.vat_no),
                             -1.0 * goods_disc);
    tot_add_double(  (short)(MSAM_DISC_TOT_GEN_EXCL),
                             -1.0 * goods_disc);
    /* Inclusive totals */
    tot_reset_double((short)(MSAM_DISC_TOT_INCL_0 + c_item.arti.base.vat_no));
    tot_add_double(  (short)(MSAM_DISC_TOT_INCL_0 + c_item.arti.base.vat_no),
                             calc_incl_vat(
                             tot_ret_double((short)(MSAM_DISC_TOT_EXCL_0 + c_item.arti.base.vat_no)),
                             c_item.arti.base.vat_no));
    tot_reset_double((short)(MSAM_DISC_TOT_GEN_INCL));
    for ( j = 0 ; j < 10 ; j++ ) {
      tot_add_double((short) MSAM_DISC_TOT_GEN_INCL,
                             calc_incl_vat(
                             tot_ret_double((short)(MSAM_DISC_TOT_EXCL_0 + j)),
                             j));
    }

    /* Vat totals */
    tot_reset_double((short)(MSAM_DISC_TOT_VAT_0 + c_item.arti.base.vat_no));
    tot_add_double(  (short)(MSAM_DISC_TOT_VAT_0 + c_item.arti.base.vat_no),
                             tot_ret_double((short)(MSAM_DISC_TOT_INCL_0 + c_item.arti.base.vat_no))
                             - tot_ret_double((short)(MSAM_DISC_TOT_EXCL_0 + c_item.arti.base.vat_no)));
    tot_reset_double((short)(MSAM_DISC_TOT_GEN_VAT));
    for ( j = 0 ; j < 10 ; j++ ) {
      tot_add_double((short)(MSAM_DISC_TOT_GEN_VAT),
                             tot_ret_double((short)(MSAM_DISC_TOT_VAT_0 + j)));
    }

    /* Total discount amounts */
    switch (mode) {
      case ARTICLE_LVL:
        tot_add_double((short)(MSAM_TOT_DISC1), -1.0 * goods_disc);
        break;
      case INVOICE_LVL:
        tot_add_double((short)(MSAM_TOT_DISC2), -1.0 * goods_disc);
        break;
      default:
        break;
    }
    tot_add_double((short)(MSAM_TOT_DISC1), -1.0 * goods_disc);

    if (item == MML_LIST) {
      tm_upda_nth(TM_ITEM_NAME, (void*)&c_item, (short) mse_invoice[index].seq_no);
      mse_invoice[index].amount -= goods_disc;
    }
    else {
      tm_upda(TM_ITEM_NAME, (void*)&c_item);
    }
    remainder -= goods_disc;
    i++;
  }

  return (SUCCEED);
} /* distr_amount */

/*---------------------------------------------------------------------------*/
/*                              apply_discount                               */
/*---------------------------------------------------------------------------*/
int apply_discount(short disc_type, long art_no, short art_qty, short times, double disc, LLIST *s)
{
  short d_type, d_lvl;
  int   res;
  char aux[4];

  memcpy(aux, &art_no, 4);
  //printf_log("hexa art_no[%X][%X][%X][%X]", (aux[0] & 0xFF), (aux[1] & 0xFF), (aux[2] & 0xFF), (aux[3] & 0xFF));
  memset(&action_result, 0, sizeof(ACTION_RESULT));
  switch (disc_type) {
    case T_DISC_ART:
      disc   = get_value(art_no);
      disc  *= art_qty;
	    disc  *= times;
      d_type = DISC_AMNT;
      break;
    case T_DISC_GIFT:
      disc   = get_value(art_no);
      disc  *= art_qty;
  	  disc  *= times;
      d_type = DISC_AMNT;
      break;
    case T_DISC_PERC:
      d_type = DISC_PERC;
      break;
    case T_DISC_AMNT:
      d_type = DISC_AMNT;
      break;
    case T_DISC_PREMIUM:
      d_type = DISC_AMNT;
      break;
    default:
      return(FAIL);
      break;
  }

  if(msam.priority == 1) {
    d_lvl = ARTICLE_LVL;
  }
  else {
    d_lvl = INVOICE_LVL;
  }

/*  printf_log("apply_discount disc_type:%d art_no:%ld art_qty:%d times:%d  disc:%6.2f",
              disc_type, art_no, art_qty, times,disc);*/



  if (disc_type == T_DISC_GIFT) {
    add_article(art_no, (short)(art_qty * times));
    /* Discount on this article. To prevent problems with different Vat rates */
    res = distr_amount(msam.mmail_no, msam.maction_no, d_lvl, disc, d_type, s, CURRENT_ITEM);
  }
  else {
    res = distr_amount(msam.mmail_no, msam.maction_no, d_lvl, disc, d_type, s, MML_LIST);
  }
  if (res != SUCCEED) {
    err_invoke(MSAM_DISTR_ERR);
    return (FAIL);
  }

  /*  The values for threshold_qty, treshold_amount, disc_excl, disc_incl    */
  /*  and disc_amount are filled in the function distr_amount                */

  action_result.mmail_no     = msam.mmail_no;
  action_result.maction_no   = msam.maction_no;
  action_result.version_no   = msam.version_no;
  action_result.times        = (d_type != DISC_PERC) ? times : 1;
  action_result.action_type  = msam.action_type;
  action_result.discount_qty = art_qty * times;
  action_result.result_type  = disc_type;
  _tcscpy(action_result.line_text, msam.line_text);

  sll_add(&action_results, &action_result);

  return (SUCCEED);
} /* apply_discount */

/*---------------------------------------------------------------------------*/
/*                              read_invoice                                 */
/*---------------------------------------------------------------------------*/
void read_invoice(void)
{
  TM_INDX item_indx;

  int i=0;

  item_indx=tm_frst(TM_ITEM_NAME, (void*)&c_item);

  while (item_indx>=0) {
    if (c_item.voided) { /* TODO */
      ;
    }
    else if (c_item.arti.accumulated != TRUE) {
      if (c_item.arti.base.goods_value>0.0 && c_item.disc.base.goods_value == 0.0) {
        mse_invoice[i].seq_no     = item_indx;
        mse_invoice[i].art_no     = c_item.arti.base.art_no;
        mse_invoice[i].art_grp_no = c_item.arti.base.art_grp_no;
        mse_invoice[i].sub_grp_no = c_item.arti.base.art_grp_sub_no;
        mse_invoice[i].qty        = c_item.arti.base.qty;
        mse_invoice[i].suppl_no   = c_item.arti.base.suppl_no;
        mse_invoice[i].mmail_no   = c_item.arti.mmail_no;
        mse_invoice[i].mmail_no_1 = 0;

        if (genvar.price_incl_vat==INCLUSIVE) {
          mse_invoice[i].amount=calc_excl_vat(c_item.arti.base.goods_value, c_item.arti.base.vat_no);
        }
        else {
          mse_invoice[i].amount=c_item.arti.base.goods_value;
        }
        mse_invoice[i].dept_no    = c_item.arti.base.dept_cd;
        mse_invoice[i].ind_fdd    = (c_item.arti.base.dept_cd == DEPT_FOOD);
        i++;
      }
    }
    item_indx = tm_next(TM_ITEM_NAME, (void*)&c_item);
  }
  mse_nbr_lines = i;
} /* read_invoice */

/*---------------------------------------------------------------------------*/
/*                              msam_get_char                                */
/*---------------------------------------------------------------------------*/
short msam_get_char(BYTE *c)
{
  short       status;
  short       start_of_next_action = FALSE;
  static BYTE action_text_ansi[SIZE(MMML_DEF,action_text)/sizeof(_TCHAR)]={0};
  static short len_action_text_ansi=0;
#define PARSER_LEAVE_COUNT  20
  static short leave_parser_temporary = FALSE;

  if(msam_stop_reading == TRUE) { /* End of data, stop the parser. */
    return FALSE;
  }

  if(leave_parser_temporary == TRUE) {
    /* Maximum of PARSER_LEAVE_COUNT actions allowed */
    /* at once to prevent stack overflow of parser.  */
    leave_parser_temporary = FALSE;
    return FALSE;
  }

  if(last_nbr_mml_errors != mml_errors) { /* Error detected, try to recover. */
    last_nbr_mml_errors = mml_errors;
    do {
#ifdef USE_MSAM_MEMORY_READ
      status = msam_read_action_line_memory();
#else
      status = msam_read_action_line();
#endif
    } while (status==SUCCEED &&
             msam_parse.mmail_no == msam_buf.mmail_no &&
             msam_parse.maction_no == msam_buf.maction_no &&
             msam_parse.version_no == msam_buf.version_no);
    if(status != SUCCEED) {
      goto msam_get_char_FAIL;
    }
    action_text_pos = 0;
    if(msam_buf.action_text==NULL) { /* error */
      goto msam_get_char_FAIL;
    }
    /* Parsing of previous action has finished, copy the data to &msam to be processed. */
    memcpy((void*)&msam, (void*)&msam_parse, sizeof(MMML_DEF));
    /* Copy new data to &msam_parse for parsing in MmlDll. */
    memcpy((void*)&msam_parse, (void*)&msam_buf, sizeof(MMML_DEF));
    strcpy((char*)action_text_ansi, UnicodeToAnsi(msam_parse.action_text));
    len_action_text_ansi = (short)strlen((char*)action_text_ansi);
  }

  /* Test if we need to read a line. */
  if(action_text_pos == -1 || action_text_pos >= len_action_text_ansi) {
#ifdef USE_MSAM_MEMORY_READ
    status = msam_read_action_line_memory();
#else
    status = msam_read_action_line();
#endif
    if(status != SUCCEED) {
      goto msam_get_char_FAIL;
    }
    if((msam_parse.mmail_no   != msam_buf.mmail_no ||
        msam_parse.maction_no != msam_buf.maction_no ||
        msam_parse.version_no != msam_buf.version_no) &&
        action_text_pos != -1) {
      start_of_next_action = TRUE;
    }
    action_text_pos = 0;
    if(msam_buf.action_text==NULL) { /* error */
      goto msam_get_char_FAIL;
    }
    /* Parsing of previous action has finished, copy the data to &msam to be processed. */
    memcpy((void*)&msam, (void*)&msam_parse, sizeof(MMML_DEF));
    /* Copy new data to &msam_parse for parsing in MmlDll. */
    memcpy((void*)&msam_parse, (void*)&msam_buf, sizeof(MMML_DEF));
    strcpy((char*)action_text_ansi, UnicodeToAnsi(msam_parse.action_text));
    len_action_text_ansi = (short)strlen((char*)action_text_ansi);
  }

  if(start_of_next_action == TRUE) {
    *c = '\n';
    ++msam_action_count;
    if(!(msam_action_count % PARSER_LEAVE_COUNT)) {
      leave_parser_temporary = TRUE;
    }
    time(&start_of_parsing); /* Ready for next line, reset time out. */
    return TRUE;
  }
  else {
    *c = action_text_ansi[action_text_pos++];
  }

  return TRUE;

msam_get_char_FAIL:
  if((status == ICUR_ERR || status == INOT_ERR) && action_text_pos > 0) { /* End of data */
    *c = '\n'; /* Return this as the last character of the data. */
    /* Parsing will finish after this, already copy the data to &msam to be processed. */
    memcpy((void*)&msam, (void*)&msam_parse, sizeof(MMML_DEF));
    ++msam_action_count;
    if(!(msam_action_count % PARSER_LEAVE_COUNT)) {
      leave_parser_temporary = TRUE;
    }
    time(&start_of_parsing); /* Ready for next line, reset time out. */
    return TRUE;
  }

  /* In this case there has been no data at all, stop the parser. */
  return FALSE;
} /* msam_get_char */

/*---------------------------------------------------------------------------*/
/*                        msam_read_action_line                              */
/*---------------------------------------------------------------------------*/
static short msam_read_action_line(void)
{
  short status;

  if(msam_read_first_action==TRUE) {
    memset(&msam_buf, 0, sizeof(MMML_DEF));
    status = pos_first_rec(MMML_TYPE, (short)POS_MMML_SIZE,
                           (short)MML_PRIORITY_IDX, (void*)&msam_buf);
    if(status != SUCCEED) {
      goto msam_read_action_line_FAIL;
    }
    msam_read_first_action=FALSE;
  }
  else {
    status = pos_next_rec(MMML_TYPE, (short)POS_MMML_SIZE,
                          (short)MML_PRIORITY_IDX, (void*)&msam_buf);
    if(status != SUCCEED) {
      goto msam_read_action_line_FAIL;
    }
  }

  while(msam_buf.start_date > pos_system.run_date ||
        msam_buf.end_date < pos_system.run_date) {
    status = pos_next_rec(MMML_TYPE, (short)POS_MMML_SIZE,
                          (short)MML_PRIORITY_IDX, (void*)&msam_buf);
    if(status != SUCCEED) {
      goto msam_read_action_line_FAIL;
    }
  }

  /* The action is read */
  return SUCCEED;

msam_read_action_line_FAIL:
  msam_stop_reading = TRUE;
  return status;
} /* msam_read_action_line */

/*---------------------------------------------------------------------------*/
/*                        msam_read_action_line_memory                       */
/*---------------------------------------------------------------------------*/
static short msam_read_action_line_memory(void)
{
  short  status;
  static multimap<MMML_KEY, MMML_DEF>::iterator i=multisam_definitions.begin();

  if(msam_read_first_action==TRUE) {
    msam_read_first_action=FALSE;
    i = multisam_definitions.begin();
  }

  if(i == multisam_definitions.end()) {
    msam_stop_reading = TRUE;
    status = INOT_ERR;
  }
  else {
    memcpy((void*)&msam_buf, (void*)&i->second, sizeof(MMML_DEF));
    ++i;
    status = SUCCEED;
  }

  return status;
} /* msam_read_action_line_memory */

/*--------------------------------------------------------------------------*/
/*                        usr_msg_hndl                                      */
/*--------------------------------------------------------------------------*/
/* This function is the main entry between the MML interpreter and the user */
/* program. Messages are handled by user defined functions.                 */
/*--------------------------------------------------------------------------*/
int usr_msg_hndl(MMLMESG *message)
{
  int status;
  MML_RESULT *mml_res;
  MML_CART *mml_cart;
  MML_CUST *mml_cust;
  MML_TIME *mml_time;
  time_t current_time_of_parsing;
  static short Counter = 0;
  static char  lbuffer[10000];
  static int   lbuffer_idx=0;
  
  switch (message->type) {
    case MML_TYP_DISCT:
      mml_res = (MML_RESULT*)message->data;
      apply_discount((short)mml_res->disc_type, mml_res->art_no, mml_res->art_qty,
                     mml_res->times, mml_res->disc, mml_res->sel);
 
/*      printf_log("MML_TYP_DISCT -> disc_type:%d art_no:%ld   art_qty:%d times:%d disc:%6.2f sel:%d",
                        mml_res->disc_type, mml_res->art_no,
                        mml_res->art_qty,   mml_res->times,
                        mml_res->disc,      mml_res->sel);*/

      status = 0;

      break;
    case MML_TYP_GTCAR:
      mml_cart = (MML_CART*)message->data;
      mml_cart->max_allowed = action_allowed(mml_cart->action_max);
      status = 0;

      break;
    case MML_TYP_GTCUS:
      mml_cust = (MML_CUST*)message->data;
      mml_cust->cust_grp_no  = cust.cust_grp_no;
      mml_cust->cust_type_no = cust.cust_type_no;
      mml_cust->card_type_no = cust.card_type_no;
      status = 0;

/*      printf_log("MML_TYP_GTCUS -> cust_grp_no:%d cust_type_no:%d card_type_no:%d",cust.cust_grp_no,cust.cust_type_no, cust.card_type_no);*/

      break;
    case MML_TYP_GTTME:
      mml_time = (MML_TIME*)message->data;
      mml_time->date = pos_system.run_date;
      mml_time->time = pos_invoice.invoice_time;
      mml_time->weekday = get_current_weekday(pos_system.run_date);
      status = 0;
      break;
    case MML_TYP_GTINP:
      status = msam_get_char((BYTE *)message->data);
      /*
      lbuffer[lbuffer_idx++]=*((char *)message->data);
      if (status==FALSE)
      {
          lbuffer[lbuffer_idx++]=0;
          printf_log("MML_TYP_GTINP -> %s\n",lbuffer);
          //memset(buffer, 0,sizeof(buffer));
          lbuffer_idx=0;
      }*/
      if(Counter++ > 1000) { /* Performance Issue */
        time(&current_time_of_parsing);
        if(difftime(current_time_of_parsing, start_of_parsing) > PARSING_TIME_OUT_PER_LINE) {
          msam_stop_reading = TRUE;
          status = FALSE;
        }
        Counter = 0;
      }
      break;
    case MML_TYP_1STAC:
      status = TRUE;
      break;
    default:
      status = -1;
      break;
  }
  return(status);
} /* usr_msg_hndl */

/*---------------------------------------------------------------------------*/
/*                            multisam_discounts                             */
/*---------------------------------------------------------------------------*/
/* This is the entry point of Multisam being called after the Total key has  */
/* been pressed in the invoice screen                                        */
/*---------------------------------------------------------------------------*/
void multisam_discounts(short error_check)
{
  _TCHAR dummy[2000];
  long   last_mmail_no = -1;
  short  last_maction_no = -1;
  long   last_version_no = -1;
  short  last_line_no = -1;
  time_t current_time_of_parsing;

  sll_init(&action_results, sizeof(ACTION_RESULT));
  init_disc_tots();
  read_invoice();
  init_inv();

  mml_errors = 0;
  last_nbr_mml_errors = mml_errors;
  msam_action_count = 0;
  msam_stop_reading = FALSE;
  action_text_pos = -1;

  msam_read_first_action=TRUE;

  update_multisam_definitions();
  time(&start_of_parsing); /* Don't put it in the do/while loop!! */
  do {
    mml_engine(usr_msg_hndl);

    time(&current_time_of_parsing);
    if(difftime(current_time_of_parsing, start_of_parsing) > PARSING_TIME_OUT_PER_LINE) {
      /* Bug: infinite loop, that's why we are going to end it. */
      /* This one should never occur!                           */
      msam_stop_reading = TRUE;
      if(error_check /* Allowed to show errors */) {
        err_invoke(MSAM_PARSING_TIME_OUT);
      }
    }
    else if(last_nbr_mml_errors != mml_errors) { /* Error detected */
      if(error_check /* Allowed to show errors */
         && (   last_mmail_no    != msam_parse.mmail_no /* show only 1 error per line */
             || last_maction_no  != msam_parse.maction_no
             || last_version_no  != msam_parse.version_no
             || last_line_no     != msam_parse.line_no)) {

        last_mmail_no   = msam_parse.mmail_no;
        last_maction_no = msam_parse.maction_no;
        last_version_no = msam_parse.version_no;
        last_line_no    = msam_parse.line_no;

        if(mml_errors == 1) { /* Only as for supervisor approval the first time. */
          err_invoke(MSAM_SYNTAX_ERR);
        }

        if(action_text_pos == -1) {
          action_text_pos = 0;
        }
        _stprintf(dummy, _T("Please make a note of the following data:\n")
                         _T("Error message\t: %s\n")
                         _T("Mmail no\t\t: %ld\n")
                         _T("Maction no\t: %d\n")
                         _T("Version no\t: %ld\n")
                         _T("Line no\t\t: %d\n")
                         _T("Error at position\t: %d\n\n")
                         _T("Line text:\n")
                         _T("\"%s\"\n\n")
                         _T("Action text (error at last position):\n")
                         _T("\"%*.*s\"\n"),
                         mml_error_txt,
                         msam_parse.mmail_no,
                         msam_parse.maction_no,
                         msam_parse.version_no,
                         msam_parse.line_no,
                         action_text_pos,
                         msam_parse.line_text,
                         action_text_pos, action_text_pos, msam_parse.action_text);
        MessageBox(NULL, (LPTSTR) dummy, _T("MULTISAM ERROR"), MB_OK|MB_SETFOREGROUND);

        /* Reset time out, because it could take a while before approval was done. */
        time(&start_of_parsing);
      }
    }

    if(mml_errors >= MAX_NBR_MSAM_ERRORS) {
      /* Bug: probably an infinite loop, that's why we are going to end it. */
      /* This one should never occur!                                       */
      msam_stop_reading = TRUE;
      if(error_check /* Allowed to show errors */) {
        err_invoke(MSAM_TOO_MANY_ERR);
      }
    }

  } while (msam_stop_reading == FALSE);
} /* multisam_discounts */

/*--------------------------------------------------------------------------*/
/*                         action_allowed                                   */
/*--------------------------------------------------------------------------*/
double action_allowed(short max_allow)
{
  short i, number_allowed=0;
  CRAZY_ART_DEF cart_rec;
  short counter=0;

  wait_for_cart();

  /* After waiting for the requests the time out could been reached. */
  /* This means that we have to set it again.                        */
  time(&start_of_parsing);

  /* Process will take place during parsing, already copy the data to &msam to be processed. */
  memcpy((void*)&msam, (void*)&msam_parse, sizeof(MMML_DEF));
  while (sll_read(&crazy_art_hist, counter, &cart_rec) == SUCCEED) {
    i = 0;
    while (i < cart_rec.rec_num) {
      if (cart_rec.maction_no[i] == msam.maction_no && cart_rec.mmail_no[i] == msam.mmail_no) {
        number_allowed=(max_allow - (short)cart_rec.total_qty[i]);
        if (number_allowed < 0) {
          number_allowed = 0;
        }
        return (double)number_allowed;
      }
      i++;
    }
    counter++;
  }
  return((double)max_allow);
} /* action_allowed */

#pragma warning(pop) /* Pop old warning level from stack. */
