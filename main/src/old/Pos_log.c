/*
 *     Module Name       : POS_LOG.C
 *
 *     Type              : Log invoice (in tdm) to BO via SC
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
 * 05-Jan-2000 Added number of invoice lines, vat and payments to EOJ  P.M.
 * --------------------------------------------------------------------------
 * 30-Oct-2000 Added msam calculation in cre_invoice_vat().            J.D.M.
 * --------------------------------------------------------------------------
 * 10-Jan-2001 Corrected bug in usage of floor_price()                 R.N.B.
 * --------------------------------------------------------------------------
 * 24-Jan-2001 Corrected two bugs in lstrip().                         R.N.B.
 * --------------------------------------------------------------------------
 * 08-Mar-2001 Bugfix: MultiSAM totals should not be sent for deposit  R.N.B.
 *             articles.
 * --------------------------------------------------------------------------
 * 25-Sep-2001 Added Pending Invoice functionality                     M.W.
 * --------------------------------------------------------------------------
 * 30-Oct-2001 Bugfix Pending Invoice.                                 M.W.
 * --------------------------------------------------------------------------
 * 09-Oct-2002 Moved lstrip() to stri_tls.                             J.D.M.
 * --------------------------------------------------------------------------
 * 11-Dec-2002 Pending invoice: Print barcode on invoice i.s.o. art_no M.W.
 * --------------------------------------------------------------------------
 * 16-Dec-2003 Added Donation functionality.                           M.W.
 * --------------------------------------------------------------------------
 * 06-Jul-2005 Bugfix: Rounding of Multisam discounts in case of 
 *             inclusive amounts on invoice                            M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 * 01-May-2007 Added small sequence number to cre_invoice_head()       J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <math.h>

#include "stnetp24.h"                       /* Pos (library) include files   */
#include "comm_tls.h"
#include "stri_tls.h"

#include "tot_mgr.h"                        /* Toolsset include files.       */
#include "prn_mgr.h"
#include "tm_mgr.h"
#include "err_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "sll_mgr.h"
#include "date_tls.h"
#include "stri_tls.h"

#include "pos_recs.h"                       /* Application include files.    */
#include "intrface.h"
#include "pos_tm.h"
#include "pos_tot.h"
#include "pos_log.h"
#include "pos_inp.h"
#include "pos_com.h"
#include "pos_func.h"
#include "pos_errs.h"
#include "WPos_mn.h"
#include "st_main.h"
#include "llist.h"
#include "pos_msam.h"

static int    tot_lines,   tot_void_lines, tot_vat_recs, tot_paym_recs, tot_msam_recs;
static double tot_nm_food, tot_nm_nfood,   tot_mm_food,  tot_mm_nfood;

/*-------------------------------------------------------------------------*/
/*                            cre_invoice_line                             */
/*-------------------------------------------------------------------------*/
static void cre_invoice_line(short voided_invoice)
{
  INVOICE_LINE_DEF il;
  TM_INDX          item_indx;
  short            conv, seq_no;
  _TCHAR           buffer[15];
  short            status;
  int              bar_type;
  /*                                                                       */
  /* Read all not voided invoice lines from tm manager and put it in a     */
  /* structure writing/sending it away via function pos_put_rec().         */
  /*                                                                       */

  memset(&il, 0,sizeof(INVOICE_LINE_DEF ));   // acm - fix

  seq_no         = 0;
  conv           = invline;
  tot_void_lines = 0;
  tot_nm_food    = 0.0;
  tot_nm_nfood   = 0.0;
  tot_mm_food    = 0.0;
  tot_mm_nfood   = 0.0;


  
  if (voided_invoice==VOIDED_INVOICE) {
    return;
  }

  item_indx=tm_frst(TM_ITEM_NAME, (void*)&c_item);
  while (item_indx>=0) {
    if (c_item.voided) {
      tot_void_lines++;
      if (c_item.depo.base.goods_value != 0.0) {  /* Also count deposits   */
        tot_void_lines++;
      }
    }
    else if (c_item.arti.accumulated != TRUE) {
      if (c_item.arti.base.goods_value!=0.0) {
        memcpy((void*)&il.delflg, (void*)&conv, sizeof(short));
        get_invoice_no(buffer);
        il.invoice_no   = (long)_ttol(buffer);
        /*il.invoice_type = GetPrinterSize(selected_printer);   JCP*/
	    il.invoice_type = selected_invoice_printer + 1; 
        il.till_no      = pos_invoice.invoice_till_no;
        il.seq_no       = ++seq_no;
        il.art_no       = c_item.arti.base.art_no;

        bar_type       = get_barcode_type(c_item.arti.base.bar_cd);
        if( bar_type == BARCD_INTERNAL_EAN8 || bar_type == BARCD_INTERNAL_EAN13 ) {
          il.barcode[0] = _T('\0');
        }
        else {
          _tcscpy(il.barcode, lstrip(c_item.arti.base.bar_cd));
        }
        il.art_ind     = c_item.arti.art_ind;
        il.vat_no      = c_item.arti.base.vat_no;
        il.art_grp_no  = c_item.arti.base.art_grp_no;
        il.mmail_no    = c_item.arti.mmail_no;
        il.qty         = c_item.arti.base.qty;

        if (il.qty < 0 && il.art_ind != ART_IND_DEPOSIT) {
          il.art_ind=ART_IND_CREDIT;
        }

        if (genvar.price_incl_vat==INCLUSIVE) {
          il.amount=calc_excl_vat(c_item.arti.base.goods_value, il.vat_no);
        }
        else {
          il.amount=c_item.arti.base.goods_value;
        }

        il.amount = floor_price( il.amount );

        tot_lines++;

        /* Process possible discount on this article.                        */
        if (c_item.disc.base.goods_value != 0.0) {
          if (genvar.price_incl_vat == INCLUSIVE) {
            il.disc_amount=calc_excl_vat(c_item.disc.base.goods_value, il.vat_no);
          }
          else {
            il.disc_amount=c_item.disc.base.goods_value;
          }
          il.disc_amount = floor_price( il.disc_amount );
        }
        else {
          il.disc_amount=0.0;
        }

        /* Multisam */    
        il.msam_disc1       = floor_price(c_item.arti.msam_disc1);
        il.msam_disc2       = floor_price(c_item.arti.msam_disc2);
        il.msam_maction_no1 = c_item.arti.msam_maction_no1;
        il.msam_maction_no2 = c_item.arti.msam_maction_no2;
        il.msam_mmail_no1   = c_item.arti.msam_mmail_no1;
        il.msam_mmail_no2   = c_item.arti.msam_mmail_no2;
        /* end Multisam */

        if (il.mmail_no == 0L) {
          if (c_item.arti.base.dept_cd == DEPT_FOOD) {
            tot_nm_food += il.amount;
            tot_nm_food += il.disc_amount;
            /* Multisam */ 
            tot_nm_food += il.msam_disc1;
            tot_nm_food += il.msam_disc2;
            /* end Multisam */
          }
          else {
            if (c_item.arti.base.dept_cd == DEPT_NFOOD) {
              tot_nm_nfood += il.amount;
              tot_nm_nfood += il.disc_amount;
              /* Multisam */ 
              tot_nm_nfood += il.msam_disc1;
              tot_nm_nfood += il.msam_disc2;
              /* end Multisam */
            }
          }
        }
        else {
          /* makro mail */
          if (c_item.arti.base.dept_cd == DEPT_FOOD) {
            tot_mm_food += il.amount;
            tot_mm_food += il.disc_amount;
            /* Multisam */ 
            tot_mm_food += il.msam_disc1;
            tot_mm_food += il.msam_disc2;
            /* end Multisam */
          }
          else {
            if (c_item.arti.base.dept_cd == DEPT_NFOOD) {
            tot_mm_nfood += il.amount;
            tot_mm_nfood += il.disc_amount;
            /* Multisam */ 
            tot_mm_nfood += il.msam_disc1;
            tot_mm_nfood += il.msam_disc2;
            /* end Multisam */
            }
          }
        }
        il.percep_amount=c_item.arti.base.percep_amount; /// acm - fix2

        status = pos_put_rec(INVO_TYPE, POS_INVL_SIZE, INVOICE_LINE_FNO, (void*)&il);
      } /* if (ART_GROUP / DISCNT_GROUP) */

      if (c_item.depo.base.goods_value!=0.0) {
        memcpy((void*)&il.delflg, (void*)&conv, sizeof(short));
        get_invoice_no(buffer);
        il.invoice_no   = (long)_ttol(buffer);
        /*il.invoice_type = GetPrinterSize(selected_printer);   JCP*/
		il.invoice_type = selected_invoice_printer + 1;
        il.till_no      = pos_invoice.invoice_till_no;
        il.seq_no       = ++seq_no;
        il.art_no       = c_item.depo.base.art_no;
        il.vat_no       = c_item.depo.base.vat_no;
        il.art_grp_no   = c_item.depo.base.art_grp_no;
        il.mmail_no     = c_item.depo.mmail_no;
        il.qty          = c_item.depo.base.qty;
        _tcscpy(il.barcode, lstrip(c_item.arti.base.bar_cd));

        if (il.qty < 0) {
          il.art_ind=ART_IND_CREDIT;
        }
        else {
          il.art_ind=ART_IND_DEPOSIT;
        }

        if (genvar.price_incl_vat==INCLUSIVE) {
          il.amount=calc_excl_vat(c_item.depo.base.goods_value, il.vat_no);
        }
        else {
          il.amount=c_item.depo.base.goods_value;
        }

        il.amount = floor_price( il.amount );

        tot_lines++;
        if (il.mmail_no == 0L) {
          if (c_item.depo.base.dept_cd == DEPT_FOOD) {
            tot_nm_food += il.amount;
          }
          else if (c_item.depo.base.dept_cd == DEPT_NFOOD) {
            tot_nm_nfood += il.amount;
          }
        }
        else {                                                 /* makro mail */
          if (c_item.depo.base.dept_cd == DEPT_FOOD) {
            tot_mm_food += il.amount;
          }
          else if (c_item.depo.base.dept_cd == DEPT_NFOOD) {
            tot_mm_nfood += il.amount;
          }
        }
                               /* No discounts and MultiSAM on deposits.     */
        il.disc_amount      = 0.0;
        il.msam_disc1       = 0.0;
        il.msam_disc2       = 0.0;
        il.msam_maction_no1 = 0;
        il.msam_maction_no2 = 0;
        il.msam_mmail_no1   = 0;
        il.msam_mmail_no2   = 0;
        il.percep_amount    = c_item.depo.base.percep_amount;// acm - fix2
        pos_put_rec(INVO_TYPE, POS_INVL_SIZE, INVOICE_LINE_FNO, (void*)&il);
      }
    }
    item_indx = tm_next(TM_ITEM_NAME, (void*)&c_item);
  }
} /* cre_invoice_line */


/*-------------------------------------------------------------------------*/
/*                            cre_invoice_head                             */
/*-------------------------------------------------------------------------*/
static void cre_invoice_head(short voided_invoice)
{
  INVOICE_DEF id;
  _TCHAR      buffer[15];
  short       conv;
  double      total_sell=0;

  /*                                                                       */
  /* Create the invoice header record and write/send it away via the       */
  /* function pos_put_rec().                                               */
  /*                                                                       */

  memset(&id, 0, sizeof(INVOICE_DEF));

  conv=invhead;
  memcpy((void*)&id.delflg, (void*)&conv, sizeof(short));
  get_invoice_no(buffer);
  id.invoice_no    = (long)_ttol(buffer);
 /* id.invoice_type  = GetPrinterSize(selected_printer);  JCP*/
  id.invoice_type = selected_invoice_printer + 1;  
  id.till_no       = pos_invoice.invoice_till_no;
  id.cashier_no    = c_shft.cashier;
  id.store_no      = cust.store_no;
  id.cust_no       = cust.cust_no;
  id.invoice_date  = pos_invoice.invoice_date;
  id.invoice_time  = pos_invoice.invoice_time;
  id.nbr_lines     = tot_lines;
  id.fee_status    = pos_invoice.invoice_fee_status;
  id.nbr_void_lines= tot_void_lines;
  id.tot_nm_food   = tot_nm_food;
  id.tot_nm_nfood  = tot_nm_nfood;
  id.tot_mm_food   = tot_mm_food;
  id.tot_mm_nfood  = tot_mm_nfood;
  id.fee_amount    = pos_invoice.invoice_fee_amount;
  id.donation      = get_invoice_donation_currmode(/*v3.4.5 acm -*/);
  

  
  strcpy(id.serie, pos_invoice.invoice_serie);  //mlsd FE
  id.correlative   = pos_invoice.invoice_correlative;  //mlsd FE
  
  total_sell       =   id.tot_nm_food   +
                       id.tot_nm_nfood  +
                       id.tot_mm_food   +
                       id.tot_mm_nfood  ;


  if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
    id.sequence_no = pos_invoice.invoice_sequence_no;
  }
  else {
    id.sequence_no = -1;
  }


  //if total_sell
  if (  (voided_invoice==VOIDED_INVOICE) 
      &&(fabs(total_sell)<0.01 ))
  {
      //ticket 
      id.percep_amount           = 0;//pos_invoice.invoice_percep_amount; //v3.6.2 acm -
      strcpy(id.percep_custdoc   , ""); //v3.6.1 acm -
      strcpy(id.percep_custname  , ""); //v3.6.1 acm -
  } else {
    id.percep_amount           = get_invoice_perception_currmode();//pos_invoice.invoice_percep_amount; //v3.6.2 acm -
    strcpy(id.percep_custdoc   , pos_invoice.invoice_percep_custdoc ); //v3.6.1 acm -
    strcpy(id.percep_custname  , pos_invoice.invoice_percep_custname); //v3.6.1 acm -
  } 

  pos_put_rec(INVO_TYPE, POS_INVH_SIZE, INVOICE_FNO,(void*)&id);
} /* cre_invoice_head */


/*-------------------------------------------------------------------------*/
/*                            cre_invoice_payment                          */
/*-------------------------------------------------------------------------*/
static void cre_invoice_payment(void)
{
  INVOICE_PAYMENT_DEF ip;
  PAYMENT_DEF         paym;
  _TCHAR              buffer[19];
  double              extra_amount, amount;
  short               i, conv, extra_paym_cd;

  /*                                                                       */
  /* Create the invoice payment records and write/send it away via the     */
  /* function pos_put_rec().                                               */
  /*                                                                       */

  conv=invpaym;

  extra_paym_cd=(short)tot_ret_double(TOT_CREDIT_PAYM_CD);
  extra_amount=tot_ret_double(TOT_CREDIT_AMNT) +
                 tot_ret_double(TOT_CREDIT_VAT_AMNT);

  for (i = TOT_PAYM_0; i <= TOT_PAYM_9; i++) {
    paym.paym_cd=i%10;
    get_paym(&paym);
    amount = tot_ret_double(i);
    if (amount != 0.0) {
      memcpy((void*)&ip.delflg, (void*)&conv, sizeof(short));
      get_invoice_no(buffer);
      ip.invoice_no   = (long)_ttol(buffer);
      /*ip.invoice_type = GetPrinterSize(selected_printer);   JCP*/
	  ip.invoice_type = selected_invoice_printer + 1;
      ip.till_no      = pos_invoice.invoice_till_no;
      ip.paym_cd      = i - TOT_PAYM_0;
      ip.paym_amount  = amount;
      ip.local_paym   = amount*paym.curr_standard/paym.curr_rate;
      ip.standard     = paym.curr_standard;
      ip.rate         = paym.curr_rate;
      if (i-TOT_PAYM_0==extra_paym_cd && extra_amount>0.0) {
        ip.extra_amount=extra_amount;
      }
      else {
        ip.extra_amount=0.0;
      }
      tot_paym_recs++;
      pos_put_rec(INVO_TYPE, POS_INVP_SIZE, INVOICE_PAYM_FNO, (void*)&ip);
    }
  }
} /* cre_invoice_payment */

/*-------------------------------------------------------------------------*/
/*                            cre_invoice_paym_item                        */
/*-------------------------------------------------------------------------*/
static void cre_invoice_paym_item(void)
{
  INVOICE_PAYM_ITEM_DEF  ii, paym_item;
  _TCHAR                 buffer[19];
  short                  i, conv;
  short                  paym_cd_seq[MAX_PAYM_WAYS] = {1,1,1,1,1,1,1,1,1,1};

  conv=invpitm;
  get_invoice_no(buffer);

  memcpy((void*)&ii.delflg, (void*)&conv, sizeof(short));
  ii.invoice_no      = (long)_ttol(buffer);
  /*ii.invoice_type    = GetPrinterSize(selected_printer);  JCP*/
  ii.invoice_type = selected_invoice_printer + 1;
  ii.till_no         = pos_invoice.invoice_till_no;

  i=0;
  while (sll_read(&payment_items, i, &paym_item) == SUCCEED) {
    ii.paym_amount    = paym_item.paym_amount;
    ii.paym_cd        = paym_item.paym_cd;
    ii.paym_date      = paym_item.paym_date;
    _tcscpy(ii.id, paym_item.id);  

    ii.seq_no         = paym_cd_seq[ii.paym_cd]++;

    pos_put_rec(INVO_TYPE, POS_INVI_SIZE, INVOICE_PITM_FNO, (void*)&ii);
    i++;
  }
} /* cre_invoice_paym_item */

/*-------------------------------------------------------------------------*/
/*                            cre_invoice_vat                              */
/*-------------------------------------------------------------------------*/
static void cre_invoice_vat(void)
{
  INVOICE_VAT_DEF iv;
  _TCHAR          buffer[15];
  double          excl_amnt;
  short           i,
                  conv;

  /*                                                                       */
  /* Create the invoice vat records and write/send it away via the         */
  /* function pos_put_rec().                                               */
  /*                                                                       */
  /* Include customer fee in vat record for vat_no = 0.                    */
  /*                                                                       */

  conv=invvatt;
  get_invoice_no(buffer);

  /* Separate handling of vat_no = 0 to add possible customer fee.         */
  i=0;
  excl_amnt = tot_ret_double((short)(i+TOT_EXCL_0))
              + tot_ret_double((short)(i+MSAM_DISC_TOT_EXCL_0));
  if (excl_amnt != 0.0 || pos_invoice.invoice_fee_amount != 0.0) {
    memcpy((void*)&iv.delflg, (void*)&conv, sizeof(short));
    iv.invoice_no   = (long)_ttol(buffer);
    /*iv.invoice_type = GetPrinterSize(selected_printer);   JCP*/
	iv.invoice_type = selected_invoice_printer + 1;
    iv.till_no      = pos_invoice.invoice_till_no;
    iv.vat_no       = i;
    iv.vat_amount   = tot_ret_double((short)(i+TOT_VAT_0))
                      + tot_ret_double((short)(i+MSAM_DISC_TOT_VAT_0));
    iv.basic_amount = excl_amnt + pos_invoice.invoice_fee_amount;
    tot_vat_recs++;
    pos_put_rec(INVO_TYPE, POS_INVV_SIZE, INVOICE_VAT_FNO, (void*)&iv);
  }

  /* Handle vat_no = 1..9                                                  */
  for (i=1; i <= 9; i++) {
    excl_amnt = tot_ret_double((short)(i+TOT_EXCL_0))
                + tot_ret_double((short)(i+MSAM_DISC_TOT_EXCL_0));
    if (excl_amnt != 0.0) {
      memcpy((void*)&iv.delflg, (void*)&conv, sizeof(short));
      iv.invoice_no   = (long)_ttol(buffer);
      /*iv.invoice_type = GetPrinterSize(selected_printer);   JCP*/
	  iv.invoice_type = selected_invoice_printer + 1;
      iv.till_no      = pos_invoice.invoice_till_no;
      iv.vat_no       = i;
      iv.vat_amount   = tot_ret_double((short)(i+TOT_VAT_0))
                        + tot_ret_double((short)(i+MSAM_DISC_TOT_VAT_0));
      iv.basic_amount = excl_amnt;
      tot_vat_recs++;
      pos_put_rec(INVO_TYPE, POS_INVV_SIZE, INVOICE_VAT_FNO, (void*)&iv);
    }
  }

  return;
} /* cre_invoice_vat */

/*-------------------------------------------------------------------------*/
/*                            cre_invoice_msam                             */
/*-------------------------------------------------------------------------*/
static void cre_invoice_msam(void)
{
  ACTION_RESULT    action_result;
  INVOICE_MSAM_DEF im;
  _TCHAR           buffer[15];
  short            i, conv;

  /*                                                                       */
  /* Create the invoice multisam records and write/send it away via the    */
  /* function pos_put_rec().                                               */
  /*                                                                       */

  conv = invmsam;
  get_invoice_no(buffer);

  i = 0;
  
  while ( sll_read(&action_results,i,&action_result) == SUCCEED) {
    memcpy((void*)&im.delflg, (void*)&conv, sizeof(short));
    im.till_no          = pos_invoice.invoice_till_no;
    im.invoice_no       = (long)_ttol(buffer);
    /*im.invoice_type     = GetPrinterSize(selected_printer);*/
    im.invoice_type = selected_invoice_printer + 1;
    im.mmail_no         = action_result.mmail_no;
    im.maction_no       = action_result.maction_no;
    im.maction_type     = action_result.action_type;
    im.version_no       = action_result.version_no;
    im.threshold_qty    = action_result.threshold_qty;
    im.discount_qty     = action_result.discount_qty;
    im.result_type      = action_result.result_type;
    im.threshold_amount = action_result.threshold_amount;
    im.discount_amount  = action_result.discount_amount;
    _tcscpy(im.result, _T(""));
    i++;
    tot_msam_recs++;
    pos_put_rec(INVO_TYPE, POS_MSAM_SIZE, INVOICE_MSAM_FNO, (void*)&im);
  }
} /* cre_invoice_msam */


/*-------------------------------------------------------------------------*/
/*                            cre_invoice_end                              */
/*-------------------------------------------------------------------------*/
static void cre_invoice_end(void)
{
  INVOICE_EOF_DEF ie;
  _TCHAR          buffer[15];
  short           conv;

  /*                                                                       */
  /* Send the message invoice ended to the function pos_put_rec().         */
  /*                                                                       */

  conv=inveoej;
  memcpy((void*)&ie.delflg, (void*)&conv, sizeof(short));
  get_invoice_no(buffer);
  ie.invoice_no         = (long)_ttol(buffer);
  ie.till_no            = pos_invoice.invoice_till_no;
  ie.nbr_lines          = tot_lines;
  ie.nbr_vat            = tot_vat_recs;
  ie.nbr_payments       = tot_paym_recs;
  ie.nbr_msam_discounts = tot_msam_recs;
  pos_put_rec(INVO_TYPE, POS_INVE_SIZE, INVOICE_EOF_FNO, (void*)&ie);
} /* cre_invoice_end */


/*-------------------------------------------------------------------------*/
/*                            cre_invoice                                  */
/*-------------------------------------------------------------------------*/
void cre_invoice(short voided)
{
  /*                                                                       */
  /* Send/Write complete invoice.                                          */
  /*  If 'voided' == VOIDED_INVOICE then the invoice is voided and only an */
  /*  invoice header (and invoice end record) must be sent to backoffice.  */
  /*                                                                       */

  tot_vat_recs = 0;
  tot_paym_recs = 0;
  tot_lines = 0;
  tot_msam_recs = 0;

  cre_invoice_line(voided);
  cre_invoice_head(voided);                         /* after cre_invoice_line!   */
  if (voided != VOIDED_INVOICE) {
    cre_invoice_payment();
    cre_invoice_paym_item();
    cre_invoice_vat();
    cre_invoice_msam();
  }
  cre_invoice_end();
} /* cre_invoice */


/*--------------------------------------------------------------------*/
/*                         save_article_lines                         */
/*--------------------------------------------------------------------*/
void save_article_lines(void)
{
  TM_INDX             item_indx;
  POS_PEND_CUST_DEF   ppcd;
  POS_PEND_INVL_DEF   ppid;
  short               seq_no;
  long                cust_key;
    
  seq_no   = 0;
  cust_key = (cust.store_no * 1000000L) + cust.cust_no;

  delete_pending_invoice(cust_key);     /* Delete old pending invoice (if exists) */

  item_indx=tm_frst(TM_ITEM_NAME, (void*)&c_item);

  if (item_indx >= 0 || pos_invoice.invoice_fee_amount > 0.0) { /* At least one article record or customer fee */

    ppcd.cust.delflg             = (_TCHAR)0;
    ppcd.cust.cust_key           = cust_key;
    ppcd.cust.store_no           = cust.store_no;
    ppcd.cust.cust_no            = cust.cust_no;
    ppcd.cust.ind_cheques        = cust.ind_cheques;
    ppcd.cust.card_type_no       = cust.card_type_no;
    ppcd.cust.cardholder_no      = cust.cardholder_no;
    ppcd.cust.exp_date           = cust.exp_date;
    ppcd.cust.appr_cheque_amount = cust.appr_cheque_amount;
    ppcd.cust.cust_type_no       = cust.cust_type_no;
    ppcd.cust.cust_grp_no        = cust.cust_grp_no;
    ppcd.fee_amount              = pos_invoice.invoice_fee_amount;
    ppcd.fee_status              = pos_invoice.invoice_fee_status;
    ppcd.invoice_time            = get_current_time((_TCHAR *)NULL);;

    _tcscpy(ppcd.cust.cust_check_dig, cust.cust_check_dig);
    _tcscpy(ppcd.cust.cust_bl_cd,     cust.cust_bl_cd);
    _tcscpy(ppcd.cust.name,           cust.name);
    _tcscpy(ppcd.cust.building,       cust.building);
    _tcscpy(ppcd.cust.address,        cust.address);
    _tcscpy(ppcd.cust.town,           cust.town);
    _tcscpy(ppcd.cust.post_cd_addr,   cust.post_cd_addr);
    _tcscpy(ppcd.cust.fisc_no,        cust.fisc_no);

    // acm7 - {
    ppcd.cust.cust_ret_agent_ind  = cust.cust_ret_agent_ind;
    ppcd.cust.cust_except_ind     = cust.cust_except_ind;
    ppcd.cust.cust_perc_agent_ind = cust.cust_perc_agent_ind;
    // acm7 - }


    pos_put_rec(PEND_TYPE, POS_PEND_CUST_SIZE, PEND_CUST_FNO, (void*)&ppcd);
  }
   
  while (item_indx >= 0) {

    if (c_item.voided==FALSE && c_item.arti.accumulated!=TRUE) {

      ppid.delflg      = (_TCHAR)0;
      ppid.cust_key    = cust_key;
      ppid.seq_no      = ++seq_no;
      ppid.store_no    = cust.store_no;
      ppid.cust_no     = cust.cust_no;
      ppid.art_no      = c_item.arti.base.art_no;
      ppid.qty         = c_item.arti.base.qty;
      ppid.amount      = c_item.arti.base.goods_value;
      ppid.disc_amount = c_item.disc.base.price; // DISCOUNT in price !!! Not goods_value

      _tcscpy(ppid.barcode, c_item.arti.base.bar_cd);
      ppid.pend_price  = c_item.arti.base.price;  //fix recuperar  // acm7 -
      pos_put_rec(PEND_TYPE, POS_PEND_INVL_SIZE, PEND_INVL_FNO, (void*)&ppid);

    }
    item_indx=tm_next(TM_ITEM_NAME, (void*)&c_item);
  }
} /* save_article_lines */

/*--------------------------------------------------------------------*/
/*                     delete_pending_invoice                         */
/*--------------------------------------------------------------------*/
void delete_pending_invoice(long cust_key)
{
  POS_PEND_CUST_DEF      pend_cust_rec;
  POS_PEND_INVL_DEF      pend_invl_rec;

  short status;
  short seq_no=0;

  memset(&pend_cust_rec, 0, POS_PEND_CUST_SIZE);

  pend_cust_rec.cust.cust_key = (cust.store_no * 1000000L) + cust.cust_no;
  status = pos_get_rec(PEND_TYPE, POS_PEND_CUST_SIZE, PEND_CUST_IDX,
                       (void*)&pend_cust_rec, (short)keyEQL);
  
  if (status == SUCCEED) {
  
    pend_invl_rec.cust_key = pend_cust_rec.cust.cust_key;
    pend_invl_rec.seq_no = ++seq_no;

    status = pos_get_rec(PEND_TYPE, POS_PEND_INVL_SIZE, PEND_INVL_IDX,
                         (void*)&pend_invl_rec, (short)keyEQL);
    if(status == SUCCEED) {

      do {
      
        pos_delete_rec(PEND_TYPE, POS_PEND_INVL_SIZE, PEND_INVL_IDX,
                       PEND_INVL_FNO, (void*)&pend_invl_rec);

        memset(&pend_invl_rec, 0, POS_PEND_INVL_SIZE);
        pend_invl_rec.cust_key = pend_cust_rec.cust.cust_key;
        pend_invl_rec.seq_no = ++seq_no;

        status = pos_get_rec(PEND_TYPE, POS_PEND_INVL_SIZE, PEND_INVL_IDX,
                             (void*)&pend_invl_rec, (short)keyEQL);

      } while (status == SUCCEED && pend_invl_rec.cust_key == pend_cust_rec.cust.cust_key);
    }

    pos_delete_rec(PEND_TYPE, POS_PEND_CUST_SIZE, PEND_CUST_IDX,
                   PEND_CUST_FNO, (void*)&pend_cust_rec);
  }
}
