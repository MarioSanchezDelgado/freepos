/*
 *     Module Name       : POS_BP3.C
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
 * 18-Apr-2001 Added print_total_msam for Immediate printing in 
 *             st_total.c                                                J.H.
 * --------------------------------------------------------------------------
 * 26-Sep-2002 Made barcode flexible depending on LEN_BARC.            J.D.M.
 * --------------------------------------------------------------------------
 * 25-Oct-2002 Added some forward skips and reverse skips.             J.D.M.
 * --------------------------------------------------------------------------
 * 27-Aug-2003 Changed calculation of total carried forward in
 *             print_total_msam() to prevent rounding differences.
 *             Reset of all total carried forward totals added in
 *             several functions.                                      J.D.M.
 * --------------------------------------------------------------------------
 * 17-Feb-2004 A Separate 'carried forward' total is used to store           
 *             MultiSam discounts in order to avoid rounding diffs.      P.M.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include "appbase.h"
#include <stdio.h>                          /* System include files.         */
#include <stdlib.h>
                                            /* POS (library) include files.  */
#include "misc_tls.h"
                                            /* Toolsset include files.       */
#include "bp_mgr.h"
#include "prn_mgr.h"
#include "inp_mgr.h"
#include "scrl_mgr.h"
#include "state.h"
#include "tm_mgr.h"
#include "tot_mgr.h"
#include "mem_mgr.h"
#include "llist.h"
#include "sll_mgr.h"
                                            /* Application include files.    */
#include "pos_recs.h"
#include "pos_tm.h"
#include "write.h"
#include "pos_bp1.h"
#include "pos_bp3.h"
#include "pos_func.h"
#include "pos_tot.h"
#include "pos_txt.h"
#include "WPos_mn.h"
#include "st_main.h"
#include "pos_msam.h"
#include "MapKeys.h"
#include "Pos_edoc.h"

#include "intrface.h"
#include "stnetp24.h"
#include "DllMnP24.h"
#include "comm_tls.h"
#include "stri_tls.h"


#ifndef PRN_IMM

short is_detraction = FALSE;
int desc_items = 0;

typedef struct {
  short   status;                 /* 0 is ok, 9 is moved out          */
  long    art_no;
  _TCHAR  barcode[LEN_BARC];
  short   switches;               /* 1:negative, 2:discount, 4:weight */
  TM_INDX indx;
} SORT_ELEM;

SORT_ELEM * sort_arr = NULL,          /* accessible for all functions */
          * sort_elm;

short   sorted_invoice = 0;
_TCHAR  sort_string[] = _T("+#");

int sort_compare(const void *, const void *);

/*--------------------------------------------------------------------*/
/*                         sort_compare                               */
/*--------------------------------------------------------------------*/
int sort_compare(const void *e1, const void *e2)
{
  _TCHAR   k1[28], k2[28];
  _TCHAR   prompt[20];
  static short count=0, pos=0;

  if (count == 141) {
    count = 0;
    _stprintf(prompt, _T("%s %c"), prompt_TXT[7], (_TCHAR)sort_string[pos]);
    display_prompt(prompt, ERROR_WINDOW_ROW1);
    pos++;
    if (pos > 1) {
      pos = 0;
    }
  }
  count++;
  _stprintf(k1, _T("%1d%07ld%14s%1d"), ((SORT_ELEM *)e1)->status,
           ((SORT_ELEM *)e1)->art_no, ((SORT_ELEM *)e1)->barcode, ((SORT_ELEM *)e1)->switches);

  _stprintf(k2, _T("%1d%07ld%14s%1d"), ((SORT_ELEM *)e2)->status,
           ((SORT_ELEM *)e2)->art_no, ((SORT_ELEM *)e2)->barcode, ((SORT_ELEM *)e2)->switches);

  return(_tcscmp(k1, k2));               /* taking the simple way out */
} /* sort_compare */


/*--------------------------------------------------------------------*/
/*                         sort_addelem                               */
/*--------------------------------------------------------------------*/
void sort_addelem(TM_INDX item_indx)
{
  int bar_type;
  sort_elm->status   = 0;
  sort_elm->indx     = item_indx;              /* Article found       */
  sort_elm->art_no   = c_item.arti.base.art_no;
  bar_type           = get_barcode_type(c_item.arti.base.bar_cd);
  if( bar_type == BARCD_INTERNAL_EAN8 || bar_type == BARCD_INTERNAL_EAN13 ) {
    sort_elm->barcode[0] = _T('\0');
  }
  else {
    _tcscpy(sort_elm->barcode, c_item.arti.base.bar_cd); // acm -+++fix barcode
  }
  sort_elm->switches = 0;
  if (c_item.arti.base.qty < 0.0) {            /* only cum. same sign */
    sort_elm->switches += 1;
  }
  if (c_item.disc.base.goods_value != 0.0) {   /* is there a DISCOUNT */
    sort_elm->switches += 2;
  }
  if (c_item.arti.art_ind == ART_IND_WEIGHT) { /* weight art no acc.  */
    sort_elm->switches += 4;
  }
  if (c_item.depo.base.goods_value != 0.0) {   /* is there a DEPOSIT  */
    sort_elm->switches += 8;
  }
  sort_elm++;                                  /* next entry in array */
} /* sort_addelem */


/*--------------------------------------------------------------------*/
/*                         prepare_sort                               */
/*--------------------------------------------------------------------*/
short prepare_sort(void)
{
  TM_INDX     item_indx;
  short       rle_groups;
  _TCHAR      prompt[30];

  sort_elm = sort_arr;                         /* pos. on 0th entry   */
  rle_groups = 0;
  item_indx = tm_frst(TM_ITEM_NAME, (void*)&c_item);
  while (item_indx >= 0) {
    if (rle_groups % 47 == 0) {
      _stprintf(prompt, _T("%s %d"), prompt_TXT[6], rle_groups);
      display_prompt(prompt, ERROR_WINDOW_ROW1);
    }
    if (!c_item.voided) {                      /* forget voided lines */
      if (c_item.arti.base.goods_value != 0.0) {
        rle_groups++;
        sort_addelem(item_indx);
      }
    }
    item_indx=tm_next(TM_ITEM_NAME, (void*)&c_item);
  }
  memset(sort_elm, 0, sizeof(SORT_ELEM));      /* mark end-of-array   */
  sort_elm->status = 9;                        /* mark end-of-array   */
  rle_groups++;

  return(rle_groups);
} /* prepare_sort */


/*mlsd*/
void de_get_articles(void)
{
  _TCHAR dummy[16];
  
  sort_elm = sort_arr;                           /* pos. on 0th entry */
  while (sort_elm->status == 0) {
    tm_read_nth(TM_ITEM_NAME, (void*)&c_item, sort_elm->indx);
    
    memset(dummy, 0, sizeof(dummy));
    ftoa((double)c_item.arti.base.art_no, 15, dummy);
    memcpy(edoc_item_array[n_items].art_no, dummy, strlen(dummy));

    memset(dummy, 0, sizeof(dummy));
    ftoa((double)c_item.arti.base.art_grp_no, 15, dummy);
    memcpy(edoc_item_array[n_items].art_grp_no, dummy, strlen(dummy));
    printf_log("art_grp_no[%s]", edoc_item_array[n_items].art_grp_no);

    edoc_item_array[n_items].price = c_item.arti.base.goods_value/c_item.arti.base.qty;
    edoc_item_array[n_items].art_ind = c_item.arti.art_ind;
    edoc_item_array[n_items].qty = c_item.arti.base.qty;
    edoc_item_array[n_items].goods_value = c_item.arti.base.goods_value;
    memcpy(edoc_item_array[n_items].descr, c_item.arti.base.descr, (int)strlen(c_item.arti.base.descr));
    edoc_item_array[n_items].vat_no = c_item.arti.base.vat_no;
    edoc_item_array[n_items].vat_perc = c_item.arti.base.vat_perc;

    if (c_item.arti.base.percep_amount >0.0001)
        edoc_item_array[n_items].flag_percep = TRUE;
        
	
    if (sort_elm->switches & 2) //Manual Discount by unity
      edoc_item_array[n_items].disc_manual = c_item.disc.base.price;
	  
    printf_log("disc_manual[%.2f]", edoc_item_array[n_items].disc_manual);
 
    memcpy(edoc_item_array[n_items].disc_base_descr, c_item.disc.base.descr, (int)strlen(c_item.disc.base.descr));
    edoc_item_array[n_items].disc_base_qty = c_item.disc.base.qty;
    edoc_item_array[n_items].disc_base_price = c_item.disc.base.price;
    edoc_item_array[n_items].disc_base_vat_no = c_item.disc.base.vat_no;  
    edoc_item_array[n_items].disc_base_goods_value = c_item.disc.base.goods_value;

    printf_log("disc_base_goods_value[%.2f]", edoc_item_array[n_items].disc_base_goods_value);
	
    n_items++;
    sort_elm++;
  }  
  return;
}

void de_get_articles_unsorted(void)
{
  TM_INDX  c_indx;
  _TCHAR dummy[16];
  
  c_indx = tm_frst(TM_ITEM_NAME, (void*)&c_item);
  while (c_indx >= 0) {
    if (!c_item.voided) {
      tm_read_nth(TM_ITEM_NAME, (void*)&c_item, sort_elm->indx);
      
      memset(dummy, 0, sizeof(dummy));
      ftoa((double)c_item.arti.base.art_no, 15, dummy);
      memcpy(edoc_item_array[n_items].art_no, dummy, strlen(dummy));

      memset(dummy, 0, sizeof(dummy));
      ftoa((double)c_item.arti.base.art_grp_no, 15, dummy);
      memcpy(edoc_item_array[n_items].art_grp_no, dummy, strlen(dummy));
      printf_log("art_grp_no[%s]", edoc_item_array[n_items].art_grp_no);

      edoc_item_array[n_items].price = c_item.arti.base.goods_value/c_item.arti.base.qty;
      edoc_item_array[n_items].goods_value = c_item.arti.base.goods_value;
      edoc_item_array[n_items].art_ind = c_item.arti.art_ind;
      edoc_item_array[n_items].qty = c_item.arti.base.qty;
      printf_log("aaa edoc_item_array[n_items].qty[%.2f]", edoc_item_array[n_items].qty);
      memcpy(edoc_item_array[n_items].descr, c_item.arti.base.descr, (int)strlen(c_item.arti.base.descr));
      edoc_item_array[n_items].vat_no = c_item.arti.base.vat_no;
      edoc_item_array[n_items].vat_perc = c_item.arti.base.vat_perc;
      
      if (c_item.arti.base.percep_amount >0.0001)
          edoc_item_array[n_items].flag_percep = TRUE;
          
      if (c_item.disc.base.goods_value != 0.0) //Manual Discount
        edoc_item_array[n_items].disc_manual = c_item.disc.base.price;
      
      memcpy(edoc_item_array[n_items].disc_base_descr, c_item.disc.base.descr, (int)strlen(c_item.disc.base.descr));
      edoc_item_array[n_items].disc_base_qty = c_item.disc.base.qty;
      edoc_item_array[n_items].disc_base_price = c_item.disc.base.price;
      edoc_item_array[n_items].disc_base_vat_no = c_item.disc.base.vat_no;  
      edoc_item_array[n_items].disc_base_goods_value = c_item.disc.base.goods_value;
      
      n_items++;
    }
    c_indx = tm_next(TM_ITEM_NAME, (void*)&c_item);
  }
  return;
}


void de_get_msam(void)
{
  short msam_indx = 0;
  short i = 0;
  short j;
  double disc_amount;
  ART_DESC_MACTMML disc_array[128];
  int n = 0;
  long aux[128];
  
  memset(&disc_array, 0, sizeof(disc_array));
  memset(&aux, 0, sizeof(aux));

  if (sll_read(&action_results, msam_indx, &prn_msam_action) == SUCCEED) {
    do {
      if(search_discounts_mactmml(&disc_array[n], prn_msam_action.mmail_no, prn_msam_action.maction_no) == 0){
        n++;
      }

      disc_amount = (genvar.price_incl_vat == INCLUSIVE) ? prn_msam_action.disc_incl : prn_msam_action.disc_excl;
      desc_item[desc_items].desc = disc_amount;
      desc_item[desc_items].times = prn_msam_action.times;
      memcpy(desc_item[desc_items].descr, prn_msam_action.line_text, (int)strlen(prn_msam_action.line_text));
      printf_log("bp3 descr[%s]", desc_item[desc_items].descr);
      desc_items++;
      msam_indx++;
    } while (sll_read(&action_results, msam_indx, &prn_msam_action) == SUCCEED);
  }

  for(i = 0; i < n_items; i++){
    for(j = 0; j < n; j++)
      acc_discounts(&edoc_item_array[i], &disc_array[j]);
  }
  
  for(i = 0; i < n_items; i++){
    for(j = 0; j < n; j++)
      match_discounts(&edoc_item_array[i], &disc_array[j]);
  }

  return;
}
/*mlsd*/

/*--------------------------------------------------------------------*/
/*                         print_invoice_type                         */
/*--------------------------------------------------------------------*/
void print_invoice_type(void)
{

  if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
    bp_now(BP_SMALL_INVOICE, BP_INV_INIT, 0);
    bp_now(BP_SMALL_INVOICE, BP_SMALL_HEADER, 0);
  }
  else {
    bp_now(BP_INVOICE, BP_INV_INIT, 0);
    bp_now(BP_INVOICE, BP_CUSTOMER_LINES, 0);
  }
  /* Print Customer fee line only in case
     -  fee_status != FEE_STAT_NONE
  */
  if(pos_invoice.invoice_fee_status != FEE_STAT_NONE ) {
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_CUST_FEE, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_CUST_FEE_LINE, 0);   /* Print cust-fee-data       */
    }
  }

  sort_elm = sort_arr;                           /* pos. on 0th entry */
  while (sort_elm->status == 0) {
    tm_read_nth(TM_ITEM_NAME, (void*)&c_item, sort_elm->indx);
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_ARTICLE, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_ARTICLE_LINE, 0);
    }
    recalc_total_carried_forward(ART_GROUP);
    if (sort_elm->switches & 8) {                   /* DEPOSIT        */
      if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
        bp_now(BP_SMALL_INVOICE, BP_SMALL_DEPOSIT, 0);
      }
      else {
        bp_now(BP_INVOICE, BP_DEPOSIT_LINE, 0);
      }
      recalc_total_carried_forward(DEPOSIT_GROUP);
    }
    if (sort_elm->switches & 2) {                   /* DISCOUNT       */
      if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
        bp_now(BP_SMALL_INVOICE, BP_SMALL_DISCOUNT, 0);
      }
      else {
        bp_now(BP_INVOICE, BP_DISCOUNT_LINE, 0);
      }
      recalc_total_carried_forward(DISCNT_GROUP);
    }
    sort_elm++;
  }

  print_total_msam();

  if (voided_invoice == NO) {
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_TOTAL, 0);
      bp_now(BP_SMALL_INVOICE, BP_SMALL_TAX, 0);
      bp_now(BP_SMALL_INVOICE, BP_SMALL_PROMO, 0); /* 27-Jan-2012 acm -  mark */
      bp_now(BP_SMALL_INVOICE, BP_SMALL_FOOTER, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_TOTAL_LINES, 0);
    }
  }
  else {
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_VOID, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_VOID_INV, 0);
    }
  }
} /* print_invoice_type */

/*--------------------------------------------------------------------*/
/*                         accumulate_invoice                         */
/*--------------------------------------------------------------------*/
short accumulate_invoice(void)
{
  double      art_uprice, art_qty, art_gds_value, msam_disc1, msam_disc2;
  double      dep_uprice, dep_qty, dep_gds_value;
  double      art_percep_amount =0;

  short       entries, processed, removed;
  SORT_ELEM  *sort_cum;

  scrl_delete(INV_ART_LIST_WINDOW);          /* delete scroll list    */
  sorted_invoice = 0;                        /* not sorted            */
  entries = tm_coun(TM_ITEM_NAME) + 1;
  sort_arr = (SORT_ELEM *)mem_allocate(entries * sizeof(SORT_ELEM));
  if (NULL == sort_arr) {                    /* UNABLE to sort        */
    return(FAIL);
  }
  sorted_invoice = 1;
  entries = prepare_sort();
  sort_arr = (SORT_ELEM *)mem_reallocate((void *)sort_arr, entries * sizeof(SORT_ELEM));
  qsort((void *)sort_arr, (size_t)entries, sizeof(SORT_ELEM), sort_compare);
  removed = 0;
  processed = 1;
  display_prompt(prompt_TXT[8], ERROR_WINDOW_ROW1);
  display_percentage(entries, processed, ERROR_WINDOW_ROW1);
  sort_elm = sort_arr;                       /* pos. on 0th entry     */
  while (sort_elm->status != 9) {            /* end of sort_array     */
    sort_cum = sort_elm;
    tm_read_nth(TM_ITEM_NAME, (void*)&c_item, sort_cum->indx);
    art_uprice          = c_item.arti.base.price;
    art_qty             = c_item.arti.base.qty;
    art_gds_value       = c_item.arti.base.goods_value;
    art_percep_amount   = c_item.arti.base.percep_amount; // acm -

    msam_disc1    = c_item.arti.msam_disc1;
    msam_disc2    = c_item.arti.msam_disc2;
    dep_uprice    = c_item.depo.base.price;
    dep_qty       = c_item.depo.base.qty;
    dep_gds_value = c_item.depo.base.goods_value;


    sort_elm++;
    processed++;

    while (sort_elm->status   != 9 &&
           sort_elm->art_no   == sort_cum->art_no &&
           !_tcscmp(sort_elm->barcode,sort_cum->barcode) &&
           sort_elm->switches == sort_cum->switches) {
      /* NOTE: there is never a discount on deposit article, so it*/
      /*       doesn't hurt folding the structures together       */
      if (sort_elm->switches & 2) {             /* DISCOUNT       */
        break;                                  /* no accumulation*/
      }
      if (sort_elm->switches & 4) {             /* WEIGHT         */
        break;                                  /* no accumulation*/
      }
      tm_read_nth(TM_ITEM_NAME, (void*)&c_item, sort_elm->indx);
      if (c_item.arti.base.price != art_uprice ||
          c_item.depo.base.price != dep_uprice) {/* should be same*/
        break;
      }
      art_qty           += c_item.arti.base.qty;
      art_gds_value     += c_item.arti.base.goods_value;
      art_percep_amount += c_item.arti.base.percep_amount;// acm -

      msam_disc1      += c_item.arti.msam_disc1;
      msam_disc2      += c_item.arti.msam_disc2;
      dep_qty         += c_item.depo.base.qty;
      dep_gds_value   += c_item.depo.base.goods_value;


      sort_elm->status = 9;                     /* move out       */
      c_item.arti.accumulated = TRUE;           /* let log know   */
      c_item.depo.accumulated = TRUE;
      tm_upda(TM_ITEM_NAME, (void*)&c_item);
      --nbr_inv_lines;                  /* keep this value valid! */
      if (c_item.depo.base.goods_value > 0.0) {
        --nbr_inv_lines;
      }
      removed++;
      sort_elm++;
      processed++;
    }
    tm_read_nth(TM_ITEM_NAME, (void*)&c_item, sort_cum->indx);
    c_item.arti.base.qty            = art_qty;
    c_item.arti.base.goods_value    = art_gds_value;
    c_item.arti.base.percep_amount  = art_percep_amount; //acm -

    c_item.arti.msam_disc1       = msam_disc1;
    c_item.arti.msam_disc2       = msam_disc2;
    c_item.depo.base.qty         = dep_qty;
    c_item.depo.base.goods_value = dep_gds_value;

    
    tm_upda(TM_ITEM_NAME, (void*)&c_item);
    display_percentage(entries, processed, ERROR_WINDOW_ROW1);
  }
  /* Sort the array a second time. The NON-relevant items are moved   */
  /* to the back of the array, so the (re-)printing of the invoice and*/
  /* receipt is faster                                                */
  /* Only the items with accumulated == FALSE need to be written to   */
  /* the invoice backlog file                                         */
  qsort((void *)sort_arr, (size_t)entries, sizeof(SORT_ELEM), sort_compare);
  entries -= removed;
  sort_arr = (SORT_ELEM *)mem_reallocate((void *)sort_arr, entries * sizeof(SORT_ELEM));
  display_percentage(0, 0, ERROR_WINDOW_ROW1);

  return(SUCCEED);
} /* accumulate_invoice */

/*--------------------------------------------------------------------*/
/*                         print_invoice                              */
/*--------------------------------------------------------------------*/
void print_invoice(void)
{
	short i;
  
  memset(&edoc_item_array, 0, sizeof(edoc_item_array));
  memset(serie_corr, 0, sizeof(serie_corr)); 
  memset(de_serie, 0, sizeof(de_serie));
  n_items = 0;
  desc_items = 0;
  de_correlative = 0;
  is_detraction = 0;
  
  if (sorted_invoice) {
    tot_reset_double(TOT_CARR_FORWD_EXCL);
    tot_reset_double(TOT_CARR_FORWD_VAT);
    tot_reset_double(TOT_CARR_FORWD_INCL);
    tot_reset_double(TOT_CARR_FMSAM_EXCL);
    tot_reset_double(TOT_CARR_FMSAM_VAT);
    tot_reset_double(TOT_CARR_FMSAM_INCL); 
    for (i=0; i<=9; i++) {
      tot_reset_double((short)(TOT_CARR_FORWD_EXCL_0+i));
      tot_reset_double((short)(TOT_CARR_FORWD_INCL_0+i));
      tot_reset_double((short)(TOT_CARR_FORWD_VAT_0+i));
      tot_reset_double((short)(TOT_CARR_FMSAM_EXCL_0+i));
      tot_reset_double((short)(TOT_CARR_FMSAM_INCL_0+i));
      tot_reset_double((short)(TOT_CARR_FMSAM_VAT_0+i));
    }
	
    de_get_articles();
    de_get_msam();
    de_totales();
    
		if(gen_edoc && (de_reverse_mode != YES) && (train_mode == CASH_NORMAL) && (copy_invoice_active != YES) && (bot_copy_invoice_active != YES))
			edoc_invoice();

		print_invoice_type();
	}
	else {     /* not enough memory to sort the invoice/receipt */
    de_get_articles_unsorted();
    de_get_msam();
    de_totales();
    
		if(gen_edoc && (de_reverse_mode != YES) && (train_mode == CASH_NORMAL) && (copy_invoice_active != YES) && (bot_copy_invoice_active != YES))
			edoc_invoice();

		print_invoice_type_unsorted();
	}
	de_reverse_mode = NO;
	return;
} /* print_invoice */

/*--------------------------------------------------------------------*/
/*                         clean_invoice                              */
/*--------------------------------------------------------------------*/
void clean_invoice(void)
{
  if (sort_arr != NULL) {
    mem_free(sort_arr);
    sort_arr=NULL;
  }
} /* clean_invoice */

#endif /* PRN_IMM*/

/*--------------------------------------------------------------------*/
/* Functions used in both printing modes.                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                         print_invoice_type_unsorted                */
/*--------------------------------------------------------------------*/
void print_invoice_type_unsorted(void)
{
  short    i;
  short    group[] = { ART_GROUP, DEPOSIT_GROUP, DISCNT_GROUP, 0 };
  TM_INDX  c_indx;

  tot_reset_double(TOT_CARR_FORWD_EXCL);
  tot_reset_double(TOT_CARR_FORWD_VAT);
  tot_reset_double(TOT_CARR_FORWD_INCL);
  tot_reset_double(TOT_CARR_FMSAM_EXCL);
  tot_reset_double(TOT_CARR_FMSAM_VAT);
  tot_reset_double(TOT_CARR_FMSAM_INCL); 

  for (i=0; i<=9; i++) {
    tot_reset_double((short)(TOT_CARR_FORWD_EXCL_0+i));
    tot_reset_double((short)(TOT_CARR_FORWD_INCL_0+i));
    tot_reset_double((short)(TOT_CARR_FORWD_VAT_0+i));
    tot_reset_double((short)(TOT_CARR_FMSAM_EXCL_0+i));
    tot_reset_double((short)(TOT_CARR_FMSAM_INCL_0+i));
    tot_reset_double((short)(TOT_CARR_FMSAM_VAT_0+i)); 
  }

  if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
    bp_now(BP_SMALL_INVOICE, BP_INV_INIT, 0);
    bp_now(BP_SMALL_INVOICE, BP_SMALL_HEADER, 0);
  }
  else {
    bp_now(BP_INVOICE, BP_INV_INIT, 0);
    bp_now(BP_INVOICE, BP_CUSTOMER_LINES, 0);
  }

  /* Print Customer fee line only in case
     -  fee_status != FEE_STAT_NONE
  */
  if(pos_invoice.invoice_fee_status != FEE_STAT_NONE) {
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_CUST_FEE, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_CUST_FEE_LINE, 0);   /* Print cust-fee-data       */
    }
  }
  c_indx = tm_frst(TM_ITEM_NAME, (void*)&c_item);
  while (c_indx >= 0) {
    if (!c_item.voided) {
      if (c_item.arti.art_ind != ART_IND_DEPOSIT) {
        if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
          bp_now(BP_SMALL_INVOICE, BP_SMALL_ARTICLE, 0);
        }
        else {
          bp_now(BP_INVOICE, BP_ARTICLE_LINE, 0);
        }
        if (c_item.depo.base.goods_value != 0.0) {
          if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
            bp_now(BP_SMALL_INVOICE, BP_SMALL_DEPOSIT, 0);
          }
          else {
            bp_now(BP_INVOICE, BP_DEPOSIT_LINE, 0);
          }
        }
        if (c_item.disc.base.goods_value != 0.0) {
          if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
            bp_now(BP_SMALL_INVOICE, BP_SMALL_DISCOUNT, 0);
          }
          else {
            bp_now(BP_INVOICE, BP_DISCOUNT_LINE, 0);
          }
        }
      }
      else {
        if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
          bp_now(BP_SMALL_INVOICE, BP_SMALL_ARTICLE, 0);
        }
        else {
          bp_now(BP_INVOICE, BP_ARTICLE_LINE, 0);
        }
      }
      for (i=0; group[i]; i++) {
        recalc_total_carried_forward(group[i]);
      }
    }
    c_indx = tm_next(TM_ITEM_NAME, (void*)&c_item);
  }

  print_total_msam();

  if (voided_invoice == NO) {
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_TOTAL, 0);
      bp_now(BP_SMALL_INVOICE, BP_SMALL_TAX, 0);
      bp_now(BP_SMALL_INVOICE, BP_SMALL_PROMO, 0);
      bp_now(BP_SMALL_INVOICE, BP_SMALL_FOOTER, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_TOTAL_LINES, 0);
    }
  }
  else {
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_VOID, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_VOID_INV, 0);
    }
  }
  return;
} /* print_invoice_type_unsorted */

/*--------------------------------------------------------------------*/
/*                         print_total_msam                           */
/*--------------------------------------------------------------------*/
void print_total_msam(void) 
{
  short msam_indx;
  short i;

  msam_indx = 0;

  if (sll_read(&action_results, msam_indx, &prn_msam_action) == SUCCEED) {
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_MSAM_HEADER, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_MSAM_HEADER, 0);
    }
    do {
      if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
        bp_now(BP_SMALL_INVOICE, BP_SMALL_MSAM_LINE, 0);
      }
      else {
        bp_now(BP_INVOICE, BP_MSAM_LINE, 0);
      }
      if(genvar.price_incl_vat == INCLUSIVE) {
        for(i=0; i<10; i++) {
          tot_add_double((short)(TOT_CARR_FMSAM_INCL_0+i), -prn_msam_action.disc_incl_amnt_per_vat_cd[i]);
        }
      }
      else {
        for(i=0; i<10; i++) {
          tot_add_double((short)(TOT_CARR_FMSAM_EXCL_0+i), -prn_msam_action.disc_excl_amnt_per_vat_cd[i]);
        }
      }
      msam_indx++;
    } while (sll_read(&action_results, msam_indx, &prn_msam_action) == SUCCEED);

    /* We did a recalculation of some of the total carried forward totals.  */
    /* This means that we have to recalculate all totals again of the total */
    /* carried forward!                                                     */
    calc_incl_excl_vat_tcf();

    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_MSAM_TOTAL, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_MSAM_TOTAL, 0);
    }
  }
} /* print_total_msam */
