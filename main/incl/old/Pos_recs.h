/*
 *     Module Name       : POS_RECS.H
 *
 *     Type              : Include file record structures in POS application
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
 * 07-Dec-2000 Bug: added CASH_DEF & CRAZY_ART_DEF to MAX_BO_ENT_SIZE  R.N.B.
 * --------------------------------------------------------------------------
 * 21-Sep-2001 Pending Invoice structures added.                         M.W.
 * --------------------------------------------------------------------------
 * 23-Jan-2002 Changes for Credit note. Ask for amount if offline.       M.W.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 29-Apr-2002 Added daypass_card_type_no, cfee_vat_no in genvar.        M.W.
 * --------------------------------------------------------------------------
 * 22-Jul-2002 Multisam Phase 3.                                         P.M.
 * --------------------------------------------------------------------------
 * 26-Sep-2002 Made barcode flexible depending on LEN_BARC.            J.D.M.
 * --------------------------------------------------------------------------
 * 11-Dec-2002 Pending invoice: Print barcode on invoice i.s.o. art_no   M.W.
 * --------------------------------------------------------------------------
 * 16-Dec-2003 Added Donation functionality.                             M.W.
 * --------------------------------------------------------------------------
 * 14-Oct-2004 Added MMML_KEY for use with multisam actions in memory
 *             for faster performance.                                 J.D.M.
 * --------------------------------------------------------------------------
 * 31-May-2005 Added max_amnt_donation in genvar.                        M.W.
 * --------------------------------------------------------------------------
 * 21-Jul-2005 Added donation_txt in genvar + extra fields for reduced
 *             invoice. Added summarize_cd to tax. Added tax_sum_codes 
 *             table.                                                    M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 * 13-Mar-2006 Added small_inv_seq to SYSTEM_DEF.                      J.D.M.
 * --------------------------------------------------------------------------
 * 01-May-2007 Added till entity.                                      J.D.M.
 * --------------------------------------------------------------------------
 */

/*******************************************************************************/
/*                     Basic information                                       */
/*******************************************************************************/
/* Do not alter the sequence of the fields otherwise ctree writes to disk      */
/* will be corrupted.                                                          */
/*******************************************************************************/
/* The size of the common buffer between the application and stnetp24 is       */
/* COMMBUF_SIZE.                                                               */
/* The size depends on the size of the largest entity which is sent via the    */
/* structure MESSAGE_STRUCT. The structure MESSAGE_STRUCT can be found in      */
/* stnetp24.h and is 10 bytes larger than the largest entity structure (when   */
/* calculated with 2 bytes alignment), which is sent between POS and stnetp24. */
/*******************************************************************************/
/* When a new entitity is added, then also add it to the calculation of        */
/* MAX_POS_ENT_SIZE in this file.                                              */
/*******************************************************************************/

#ifndef _POS_RECS_H_
#define _POS_RECS_H_

#include "Storerec.h"
#include "appbase.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LEN_PROM_TXT_CUST 161  /* For use in pos_func.c                  */
typedef struct {
  _TCHAR  delflg;              /* delete flag                            */
  _TCHAR  prom_txt_cust[LEN_PROM_TXT_CUST];
                               /* promotional text on cust. display      */
  _TCHAR  prom_txt_top1[97];   /* promotext top, row 1                   */
  _TCHAR  prom_txt_top2[97];   /* promotext top, row 2                   */
  _TCHAR  prom_txt_top3[97];   /* promotext top, row 3                   */
  _TCHAR  prom_txt_top4[97];   /* promotext top, row 4                   */
  _TCHAR  prom_txt_bot1[61];   /* promotext bottom, row 1                */
  _TCHAR  prom_txt_bot2[61];   /* promotext bottom, row 2                */
  _TCHAR  date_format[12];     /* date format                            */
  _TCHAR  fee_descr[34];       /* description of customer fee            */
  short   ind_price_dec;       /* 0: no decimals, 1: 2 decimals          */
  short   pst_print_cd;        /* addr.layout: 0: addr/pstc 1: pstc/addr */
  double  def_fl_nm;           /* default float amount in sales mode     */
  double  def_fl_rt;           /* default float amount in return mode    */
  double  max_amnt_drawer;     /* max amount in cash drawer              */
  double  max_amnt_donation;   /* max allowed amount for donation        */
  _TCHAR  donation_txt[31];    /* donation txt                           */
  short   super_pin;           /* supervisor pincode                     */
  short   price_weight;        /* price or weight in barcode             */
  short   price_incl_vat;      /* print incl (1) or excl (0) vat         */
                               /* on backoffice price_incl_vat_tick      */
  short   invoice_history;     /* number of days that to save bklg info  */
  double  low_curr;            /* miminum currency to round to           */
  short   cust_on_pos;         /* indicator for customers on POS         */
  _TCHAR  town[26];            /* store town                             */
  short   daypass_card_type_no;/* card type no that specifies daypass    */
  short   cfee_vat_no;         /* vat no on customer fee                 */
  _TCHAR  name_comp[31];       /* name company         (reduced invoice) */
  _TCHAR  name[36];            /* name store           (reduced invoice) */
  _TCHAR  address[31];         /* address store        (reduced invoice) */
  _TCHAR  fisc_no_comp[21];    /* fisc_no company      (reduced invoice) */
  _TCHAR  tax_txt1[61];        /* tax text 1           (reduced invoice) */
  _TCHAR  tax_txt2[61];        /* tax text 2           (reduced invoice) */
  _TCHAR  tax_txt3[61];        /* tax text 3           (reduced invoice) */
  _TCHAR  tax_txt4[61];        /* tax text 4           (reduced invoice) */
  _TCHAR  tax_txt5[61];        /* tax text 5           (reduced invoice) */
  _TCHAR  tax_txt6[61];        /* tax text 6           (reduced invoice) */
  _TCHAR  prom_txt_bot3[61];   /* promotext bot, row 3 (reduced invoice) */
  _TCHAR  prom_txt_bot4[61];   /* promotext bot, row 4 (reduced invoice) */
  _TCHAR  prom_txt_bot5[61];   /* promotext bot, row 5 (reduced invoice) */

  /* v3.4.9
  _TCHAR  extra_1[61];   /* promotext bot, row 5 (reduced invoice)   //v3.4.8 acm -
  _TCHAR  extra_2[61];   /* promotext bot, row 5 (reduced invoice)   //v3.4.8 acm -
  _TCHAR  extra_3[61];   /* promotext bot, row 5 (reduced invoice)   //v3.4.8 acm -
  */
} GENVAR_DEF;

typedef struct {
  short   prefix;               /* prefix number of voucher              */
  long    seq_no;               /* sequence number of voucher            */
  _TCHAR  amount[17];           /* amount of voucher                     */
  short   paym_cd;              /* payment code of voucher               */
  short   status;               /* status of voucher 0=used, 1=not used  */
  short   store_no;             /* cust.store_no                         */
  long    cust_no;              /* cust.cust_no                          */
} VOUCHER_DEF;


//v3.5.1 acm -{
typedef struct {
  short   prefix;               /* prefix number of voucher              */
  long    seq_no;               /* sequence number of voucher            */
  _TCHAR  amount[17];           /* amount of voucher                     */
  short   paym_cd;              /* payment code of voucher               */
  short   status;               /* status of voucher 0=used, 1=not used  */
  short   store_no;             /* cust.store_no                         */
  long    cust_no;              /* cust.cust_no                          */
} VOUCHER_VALE_DEF;

//v3.5.1 acm -}


//v3.6.1 acm -{
typedef struct {
  short   prefix;               /* prefix number of voucher              */
  long    seq_no;               /* sequence number of voucher            */
  _TCHAR  amount[17];           /* amount of voucher                     */
  short   paym_cd;              /* payment code of voucher               */
  short   status;               /* status of voucher 0=used, 1=not used  */
  short   store_no;             /* cust.store_no                         */
  long    cust_no;              /* cust.cust_no                          */
} VOUCHER_ANTICIPO_DEF;

//v3.6.1 acm -}

typedef struct {
  _TCHAR  delflg;              /* delete flag                            */
  short   default_mode;        /* mode at startup: 0: return 1: sales    */
  short   current_mode;        /* current mode: 0: return 1: sales       */
  short   invoice_no;          /* current invoice sequence number        */
  long    run_date;            /* actual rundate YYYYMMDD                */
  short   store_no;            /* store number                           */
  short   till_no;             /* actual till number                     */
  long    small_inv_seq;       /* small invoice sequence number          */
  
  _TCHAR last_serie_fac[5];  //FE
  long   last_corr_fac;      //FE
  _TCHAR last_serie_bol[5];  //FE
  long   last_corr_bol;      //FE
  
} SYSTEM_DEF;


typedef struct {
  _TCHAR  delflg;              /* delete flag                            */
  long    tax_key;             /* composed key, from BO                  */
  short   vat_no;              /* vat code                               */
  short   summarize_cd;        /* summarize code                         */
  long    start_date;          /* date a vat_no should become active     */
  double  vat_perc;            /* vat percentage                         */
} TAX_DEF;


typedef struct {
  _TCHAR  delflg;              /* delete flag                            */
  short   summarize_cd;        /* summarize code                         */
  _TCHAR  descr[34];           /* description                            */
} TAX_SUM_CD_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    art_no;               /* article number                        */
  short   art_grp_no;           /* article group number                  */
  short   art_grp_sub_no;       /* article group sub number              */
  _TCHAR  descr[34];            /* description                           */
  short   type;                 /* type                                  */
  short   vat_no;               /* vat code                              */
  short   cont_sell_unit;       /* selling (makro) unit                  */
  double  deposit_unit;         /* deposit price (not used anymore)      */
  short   vat_no_deposit;       /* vat code deposit (not used anymore)   */
  _TCHAR  pack_type[3];         /* pack type                             */
  double  sell_pr;              /* selling price per makro unit          */
  short   art_status;           /* article status                        */
  short   block_ind;            /* blocked for sales? 0: not 1: yes      */
  long    mmail_no;             /* makro mail number                     */
  long    art_no_deposit;       /* deposit article number                */
  short   dept_cd;              /* indicates food or non-food            */
  _TCHAR  reg_no[12];           /* extra registration number             */
  long    suppl_no;             /* supplier number                       */

  long    arti_retention_ind  ; //v3.6.1 acm -
  long    arti_perception_ind ; //v3.6.1 acm -
  long    arti_rule_ind       ; //v3.6.1 acm -

  long    arti_detraccion_ind       ; //v3.6.2 wjm -
} ARTICLE_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    art_no;               /* article number                        */
  _TCHAR  barcode[LEN_BARC];    /* barcode                               */
} BARCODE_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  short   paym_cd;              /* payment type                          */
  _TCHAR  paym_descr[16];       /* payment description                   */
  double  paym_limit;           /* payment limit                         */
  double  extra_perc;           /* extra percentage on credit cards      */
  double  min_extra_amount;     /* minimum extra amount on credit cards  */
  short   vat_no;               /* vat_no for extra amount               */
  _TCHAR  paym_type[2];         /* special actions if filled             */
  short   currency_no;          /* number of a currency type             */
  _TCHAR  currency_cd[4];       /* ISO standard currency codes           */
  double  curr_standard;        /* rate is specified per standard (1,10) */
  double  curr_rate;            /* rate is 114,37 per 100 Dm             */
} PAYMENT_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    curr_key;             /* composed key from BO                  */
  short   currency_no;          /* number of a currency type             */
  _TCHAR  currency_cd[4];       /* ISO standard currency codes           */
  long    start_date;           /* date that new rate will be active     */
  double  standard;             /* rate is specified per standard (1,10) */
  double  rate;                 /* rate is 114,37 per 100 Dm             */
} CURR_DEF;


typedef struct {
  SYSTEM_DEF syst;
  GENVAR_DEF genv;
} STATUS_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    cfee_key;             /* composed key from BO                  */
  short   card_type_no;         /* number of card_type                   */
  short   nbr_card_from;        /* Range number card from                */
  short   nbr_card_to;          /* Range end card-number to              */
  double  fee_amount;           /* Customer fee                          */
} CFEE_DEF;

#ifdef __cplusplus
class MMML_KEY {
  public:
    short   priority;
    long    mmail_no;
    short   maction_no;
    short   line_no;

    bool operator<(const MMML_KEY& mk) const {
      double Temp1;
      double Temp2;

      Temp1 = priority * 10000000000000.0
            + mmail_no * 10000000.0
            + maction_no * 1000.0
            + line_no * 1.0;
      Temp2 = mk.priority * 10000000000000.0
            + mk.mmail_no * 10000000.0
            + mk.maction_no * 1000.0
            + mk.line_no * 1.0;

      return Temp1<Temp2;
    }
};
#endif

typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    mmail_no;             /* mmail number                          */
  short   maction_no;           /* makro action number                   */
  short   line_no;              /* line number                           */
  long    version_no;           /* version number                        */
  long    start_date;           /* multisam start date                   */
  long    end_date;             /* multisam end date                     */
  short   action_type;          /* action type                           */
  short   priority;             /* priority                              */
  _TCHAR  line_text[81];        /* invoice text for promotion            */
  _TCHAR  action_text[801];     /* mml code for WinPOS                   */
} MMML_DEF;

typedef struct {
  _TCHAR  delflg;
  long    till_no;
  long    till_mode;
  long    start_sequence_no;
  long    end_sequence_no;
  long    last_proc_sequence_no;
  _TCHAR  till_id[41];
} TILL_DEF;

/****************************************************************************/
/*                                 INVOICING                                */
/****************************************************************************/

typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    invoice_no;           /* invoice number                        */
  short   till_no;              /* till number                           */
  short   invoice_type;         /* Printed on normal or small printer    */
  short   cashier_no;           /* cashier number                        */
  short   store_no;             /* store number                          */
  long    cust_no;              /* customer number                       */
  long    invoice_date;         /* date invoice started                  */
  short   invoice_time;         /* time invoice started                  */
  short   nbr_lines;            /* number of lines in invoice            */
  short   nbr_void_lines;       /* number of voide lines in invoice      */
  short   fee_status;           /* Flag if fee is cancelled              */
  double  tot_nm_food;          /* tot normal soled food ex vat          */
  double  tot_nm_nfood;         /* tot normal soled non-food ex vat      */
  double  tot_mm_food;          /* tot makro mail food soled ex vat      */
  double  tot_mm_nfood;         /* tot makro mail non-food soled ex vat  */
  double  fee_amount;           /* fee_amount paid                       */
  double  donation;             /* donation amount                       */
  long    sequence_no;          /* small invoice sequence number         */
  
  double  percep_amount;           //v3.6.1 acm - 
  _TCHAR  percep_custdoc      [14+1];       //v3.6.1 acm -
  _TCHAR  percep_custname     [45+1];        //v3.6.1 acm -
  
  _TCHAR  serie			      [4+1]; 	//mlsd ctree FE
  long	  correlative;					  //mlsd ctree FE
  
} INVOICE_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    invoice_no;           /* invoice number                        */
  short   till_no;              /* till number                           */
  short   seq_no;               /* article line sequence number          */
  short   invoice_type;         /* Printed on normal or small printer    */
  long    art_no;               /* article number                        */
  short   art_ind;              /* 0=normal | 1=price | 2=weight         */
                                /* 3=deposit | 4=credit/discount         */
                                /* 5=return                              */
  short   vat_no;               /* tax number                            */
  short   art_grp_no;           /* article grp number of article         */
  long    mmail_no;             /* makro mail number                     */
  double  qty;                  /* quantity weight                       */
  double  amount;               /* amount total ex. vat                  */
  double  disc_amount;          /* discount amount for this article      */
  _TCHAR  barcode[LEN_BARC];    /* Scanned barcode                       */
  long    msam_mmail_no1;       /* Multisam makro mail number 1          */
  long    msam_mmail_no2;       /* Multisam makro mail number 2          */
  short   msam_maction_no1;     /* Multisam makro action number 1        */
  short   msam_maction_no2;     /* Multisam makro action number 2        */
  double  msam_disc1;           /* Multisam discount 1                   */
  double  msam_disc2;           /* Multisam discount 2                   */
  double  percep_amount;           //v3.6.1 acm -
} INVOICE_LINE_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    invoice_no;           /* invoice number                        */
  short   till_no;              /* till number                           */
  short   paym_cd;              /* payment type                          */
  short   invoice_type;         /* Printed on normal or small printer    */
  double  standard;             /* standard foreign (1..100000)          */
  double  paym_amount;          /* payment amount                        */
  double  local_paym;           /* calc. foreign*standard/rate           */
  double  extra_amount;         /* extra charge for credit cards         */
  double  rate;                 /* rate foreign (1.14)                   */
} INVOICE_PAYMENT_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  short   till_no;              /* till number                           */
  long    invoice_no;           /* invoice number                        */
  short   paym_cd;              /* paym_cd                               */
  short   seq_no;               /* sequence number                       */
  short   invoice_type;         /* Printed on normal or small printer    */
  long    paym_date;            /* Payment date                          */
  double  paym_amount;          /* paym_amount                           */
  _TCHAR  id[31];               /* id                                    */
} INVOICE_PAYM_ITEM_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    invoice_no;           /* invoice number                        */
  short   till_no;              /* till number                           */
  short   vat_no;               /* vat code                              */
  short   invoice_type;         /* Printed on normal or small printer    */
  double  vat_amount;           /* total vat-amount paid by the customer */
  double  basic_amount;         /* goods value (ex. vat) for this VAT_NO */
} INVOICE_VAT_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    invoice_no;           /* invoice number                        */
  short   till_no;              /* till number                           */
  short   invoice_type;         /* Printed on normal or small printer    */
  long    mmail_no;             /* makro mail number                     */
  short   maction_no;           /* makro action number                   */
  short   maction_type;         /* makro action type                     */
  long    version_no;           /* makro action version number           */
  long    threshold_qty;        /* threshold quantity                    */
  long    discount_qty;         /* discount quantity                     */
  short   result_type;          /* result type                           */
  double  threshold_amount;     /* threshold amount                      */
  double  discount_amount;      /* discount amount                       */
  _TCHAR  result[15];           /* not used                              */
} INVOICE_MSAM_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    invoice_no;           /* invoice number                        */
  short   till_no;              /* till number                           */
  short   nbr_lines;            /* number of invoice line records        */
  short   nbr_vat;              /* number of invoice vat records         */
  short   nbr_payments;         /* number of invoice payment records     */
  short   nbr_msam_discounts;   /* number of multisam discount records   */
} INVOICE_EOF_DEF;

typedef struct {
  INVOICE_EOF_DEF eof;
  short tag;
} INVOICE_EOF_CTREE_DEF;

/****************************************************************************/
/*                               SHIFT                                      */
/****************************************************************************/

typedef struct {
  _TCHAR delflg;
  short  shift_no;
  short  cashier;
  short  till_no;
  long   invoice_on;
  long   invoice_off;
  long   date_on;
  short  time_on;
  short  time_off;
  long   nbr_inv_lines;
  short  nbr_void_inv;
  long   nbr_void_lines;
  double start_float;
  double end_float;
  double lift_refill_float;
  short  wallet_no[3];
  double paym_amnt[10];
  double donation;

  long    cupon;               /* AC2012-003 acm - totalcupones         */
  double  rounded;             // v3.4.7 acm - 
  long    cupon_cine;          /* v3.5 acm -          */  
  long    vale_pavo;          /* v3.5 acm -          */  
  long    feria_escolar;      /* v3.6.0 acm -          */  
  double  percep_amount;         //v3.6.1 acm -
  long    fiesta_futbol;

   long  cupon_global[CUPON_GLOBAL_MAX];
} SHIFT_TDM_DEF;

typedef struct {
  SHIFT_TDM_DEF tdm;
  short tag;
} XZ_READ_CTREE_DEF;

typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    date_on;              /* date shift started                    */
  short   time_on;              /* time shift started                    */
  short   till_no;              /* till number                           */
  short   cashier_no;           /* cashier number                        */
  long    invoice_on;           /* invoice number at shift on            */
  double  start_float;          /* pos-float at shift on                 */
} SHIFT_ON_DEF;

typedef struct {
  SHIFT_ON_DEF on;
  short tag;
} SHIFT_ON_CTREE_DEF;

typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    date_on;              /* date shift started                    */
  short   time_on;              /* time shift started                    */
  short   till_no;              /* till number                           */
  short   cashier_no;           /* cashier number                        */
  short   time_off;             /* time shift ended                      */
  long    invoice_off;          /* invoice number at end of shift        */
} SHIFT_OFF_DEF;

typedef struct {
  SHIFT_OFF_DEF off;
  long  date_off;               /* date shift finished                   */
  short tag;
} SHIFT_OFF_CTREE_DEF;

typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    date_on;              /* date shift started                    */
  short   time_on;              /* time shift started                    */
  short   till_no;              /* till number                           */
  short   cashier_no;           /* cashier number                        */
  short   seq_no;               /* shift lift sequence number            */
  short   wallet_no;            /* wallet number                         */
  double  wallet_amount;        /* wallet amount                         */
} SHIFT_LIFT_DEF;

typedef struct {
  SHIFT_LIFT_DEF lift;
  short tag;
} SHIFT_LIFT_CTREE_DEF;

/****************************************************************************/
/*                   GLOBAL STRUCT DEFINITIONS                              */
/****************************************************************************/

typedef struct {
  short  invoice_till_no;      /* invoice till number                   */
  short  invoice_no;           /* invoice sequence number               */
  short  invoice_cashier_no;   /* invoice cashier number                */
  long   invoice_date;         /* invoice rundate YYYYMMDD              */
  short  invoice_time;         /* invoice runtime HHMM                  */
  short  invoice_fee_status;   /* invoice fee cancelled flag            */
  double invoice_fee_amount;   /* invoice fee amount                    */
  double invoice_donation;     /* invoice donation                      */
  long   invoice_sequence_no;  /* small invoice sequence number         */

  long   invoice_cupon;         /* AC2012-003 acm -                     */
  long   invoice_gift ;         /* AC2012-003 acm -                     */
  double invoice_rounded;       // v3.4.7 acm -                     
  long   invoice_cupon_cine;    /* v3.5 acm -                            */
  long   invoice_vale_pavo;     /* v3.5 acm -                            */
  long   invoice_feria_escolar; /* v3.6.0 acm -                            */
  double invoice_percep_amount; /* invoice perception                    */ //v3.6.1 acm -


  _TCHAR invoice_percep_custdoc [14+1];  /*                     */   //v3.6.1 acm -
  _TCHAR invoice_percep_custname[45+1];  /*                     */   //v3.6.1 acm -

  long   invoice_fiesta_futbol; /* v3.6.0 acm -                            */

  long   invoice_cupon_global[CUPON_GLOBAL_MAX];
  
  _TCHAR invoice_serie [4+1]; //mlsd FE
  long   invoice_correlative;	//mlsd FE
  
} POS_INVOICE_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    cust_key;             /* composed key from BO                  */
  short   store_no;             /* store number of customer              */
  long    cust_no;              /* customer number                       */
  _TCHAR  cust_check_dig[3];    /* seq. no + check digit                 */
  _TCHAR  cust_bl_cd[2];        /* customer bl code                      */
  _TCHAR  name[31];             /* name                                  */
  _TCHAR  building[31];         /* building                              */
  _TCHAR  address[31];          /* address                               */
  _TCHAR  town[21];             /* town                                  */
  _TCHAR  post_cd_addr[12];     /* postal code                           */
  _TCHAR  fisc_no[15];          /* fiscal number                         */
  short   ind_cheques;          /* cheques allowed 0: no 1: yes          */
  long    card_type_no;         /* Card type number                      */
  short   cardholder_no;        /* Cardholder number                     */
  long    exp_date;             /* expiration date (YYYYMMDD)            */
  double  appr_cheque_amount;   /* approved cheque amount                */
  short   cust_type_no;         /* Customer type number                  */
  short   cust_grp_no;          /* Customer group number                 */

  long    cust_ret_agent_ind    ;  //v3.6.1 acm - 
  long    cust_perc_agent_ind   ;  //v3.6.1 acm - 
  long    cust_except_ind       ;  //v3.6.1 acm - 
} POS_CUST_DEF;

typedef struct {
  short          nbr_of_cust;
  _TCHAR         fisc_no[15];
  POS_CUST_DEF   cust[MAX_NBR_OF_CUST];
} POS_CUST_FISC_DEF;


typedef struct {
  POS_CUST_DEF cust;
  double  fee_amount;           /* fee_amount paid                       */
  short   fee_status;           /* Flag if fee is cancelled              */
  short   invoice_time;         /* time invoice started                  */
} POS_PEND_CUST_DEF;


typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    cust_key;             /* composed key from BO                  */
  short   seq_no;               /* article line sequence number          */
  short   store_no;             /* store number of customer              */
  long    cust_no;              /* customer number                       */
  long    art_no;               /* article number                        */
  double  qty;                  /* quantity weight                       */
  double  amount;               /* amount total ex. vat                  */
  double  disc_amount;          /* discount amount for this article      */
  _TCHAR  barcode[LEN_BARC];    /* Scanned barcode                       */
  double  pend_price;           /* price                                 */ // acm7 -

} POS_PEND_INVL_DEF;


typedef struct {
  short   empl_no;              /* cashier number                        */
  _TCHAR  empl_name[31];        /* name                                  */
  short   reg_status;           /* 0 - NOT authorized, 1 - authorized    */
                                /* 2 - already signed on                 */
  long    cur_date;             /* current bo date (YYYYMMDD)            */
} CASH_DEF;


typedef struct {
  short   store_no;             /* store number of customer              */
  long    cust_no;              /* customer number                       */
  short   rec_num;              /* number of records read                */
  short   ind_last;             /* indicator for last record             */
  long    mmail_no[40];         /* makro mail number                     */
  short   maction_no[40];       /* makro action number                   */
  double  total_qty[40];        /* qty allready bought by customer       */
} CRAZY_ART_DEF;


typedef struct {
  short   till_no;
  short   pos_status;
  _TCHAR  status_date[15];   /* format YYYYMMDDHHMMSS */
} POS_STATUS_DEF;




/* 27-Jan-2012 acm - { */

typedef struct {
  _TCHAR  delflg;               /* delete flag                           */  
  long    art_no;               /* article number                        */
  long    prom_type;          
  long    art_status;           /* article status                        */
  double  prom_dvalue;          /* v3.4.8 acm - */
} ARTICLE_PROM_DEF;

#define ARTICLE_PROM_SIZE     sizeof(ARTICLE_PROM_DEF)

/* 27-Jan-2012 acm - } */


/* v3.4.8 acm - { */

typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    cust_no;  
  long    store_no;
  long    prom_type;
  long    cust_status;           /* cust prom status  */
} CUST_PROM_DEF;

#define CUST_PROM_SIZE     sizeof(CUST_PROM_DEF)

/* v3.4.8 acm - } */


typedef struct    {  /* 1st @ longs, then chars; needed for alignment */
  _TCHAR  delflg;               /* delete flag                           */
  long    seq_no;  
  _TCHAR  fisc_no     [15+1];
  
  _TCHAR  fisc_no_type[1+1];
  _TCHAR  cust_name   [45+1];
}CUST_PERC_DEF;
#define DEF_CUST_PERC_SIZE     sizeof(CUST_PERC_DEF)

typedef struct  {  /* 1st @ longs, then chars; needed for alignment */
  _TCHAR  delflg;               /* delete flag                           */
  long    sequence_no;
  long    cust_key;             /* composed key from BO                  */
  _TCHAR  doc_no    [15+1];
  _TCHAR  doc_code  [5+1];
  _TCHAR  name      [30+1];
} CARDHOLDER_DEF;

#define DEF_CARDHOLDER_SIZE     sizeof(CARDHOLDER_DEF)

/* v3.5.1 acm - { */

typedef struct {
  _TCHAR  delflg;               /* delete flag                           */
  long    store_no;
  long    vale_no;  
  long    vale_type;
  long    invoice_no;
  TCHAR    fisc_no  [14+1];
  TCHAR    name_cust[45+1];
  long    tag;

  long    status;
  long    cust_no;
  long    inv_till_no;
  long    inv_invoice_type;
  long    inv_invoice_no;
  long    inv_invoice_date;

  BYTE    fact_no[21+1];
  BYTE    sell_pr[12+1];   

} VALEPAVO_PROM_DEF;

#define VALEPAVO_PROM_SIZE     sizeof(VALEPAVO_PROM_DEF)

/* v3.5.1 acm - } */


/* v3.5.1 acm - { */

typedef struct 
{
    _TCHAR  delflg;               /* delete flag                           */

    long    nro_seq	        ;
    long    store_no	        ;
    long    tipo_doc	        ;
    long    ind_uso	        ;

    TCHAR   fac_no		    [20+1];
    TCHAR   fisc_no	        [11+1];
    TCHAR   razon_social	[50+1];
  //  TCHAR   cust_name       [50+1];

    double  base_imp;
    double  base_exo;
    double  igv	 ;
    double  percepcion;
    double  monto_total;
    long    tag;
} 
ANTICIPO_PROM_DEF;

#define ANTICIPO_PROM_SIZE     sizeof(ANTICIPO_PROM_DEF)

/* v3.5.1 acm - } */






/****************************************************************************/
/*                       SIZE DEFINITIONS                                   */
/****************************************************************************/

/*------------------------------------------------------------------------------*/
/* Below, the size of the common buffer between the application and stnetp24    */
/* is defined.                                                                  */
/* It depends on the size of the largest entity which is sent via the           */
/* structure MESSAGE_STRUCT. The structure MESSAGE_STRUCT can be found in       */
/* stnetp24.h and is 16 bytes larger than the largest entity structure (when    */
/* calculated with 8 bytes alignment), which is sent between POS and stnetp24.  */
/*------------------------------------------------------------------------------*/
/* The initial size was 1536 bytes.                                             */
/*------------------------------------------------------------------------------*/
union ALL_ENTITIES {
    ARTICLE_DEF           a;
    GENVAR_DEF            b;
    STATUS_DEF            c;
    TAX_DEF               d;
    BARCODE_DEF           e;
    PAYMENT_DEF           f;
    CURR_DEF              g;
    CFEE_DEF              h;
    SYSTEM_DEF            i;
    MMML_DEF              j;
    INVOICE_DEF           k;
    INVOICE_LINE_DEF      l;
    INVOICE_PAYMENT_DEF   m;
    INVOICE_VAT_DEF       n;
    INVOICE_EOF_CTREE_DEF o;
    INVOICE_MSAM_DEF      p;
    SHIFT_ON_CTREE_DEF    q;
    SHIFT_OFF_CTREE_DEF   r;
    SHIFT_LIFT_CTREE_DEF  s;
    XZ_READ_CTREE_DEF     t;
    SHIFT_TDM_DEF         u;
    POS_CUST_DEF          v;
    CRAZY_ART_DEF         w;
    CASH_DEF              x;
    POS_PEND_CUST_DEF     y;
    POS_PEND_INVL_DEF     z;
    INVOICE_PAYM_ITEM_DEF A;
    VOUCHER_DEF           B;
    TAX_SUM_CD_DEF        C;
    POS_CUST_FISC_DEF     D;
    TILL_DEF              E;
    ARTICLE_PROM_DEF      F; /* 27-Jan-2012 acm - */
    CUST_PROM_DEF         G; /* v3.4.8 acm - */
    VALEPAVO_PROM_DEF     H; /* v3.5.1 acm - */
    VOUCHER_VALE_DEF      I;
    CUST_PERC_DEF         J;
    CARDHOLDER_DEF        K;
    ANTICIPO_PROM_DEF     L; /* v3.5.1 acm - */
};

#define MAX_POS_ENT_SIZE  sizeof(union ALL_ENTITIES)

#define POS_CSTN_SIZE     6                /* CUST_NO length alpha numeric  */
#define POS_CHDG_SIZE     2                /* length of cust check digit    */
#define POS_STNO_SIZE     2                /* STORE_NO length alpha numeric */

#define POS_GENV_SIZE     sizeof(GENVAR_DEF)
#define POS_SYST_SIZE     sizeof(SYSTEM_DEF)
#define POS_STAT_SIZE     sizeof(STATUS_DEF)
#define POS_TAXX_SIZE     sizeof(TAX_DEF)
#define POS_TSCO_SIZE     sizeof(TAX_SUM_CD_DEF)
#define POS_CUST_SIZE     sizeof(POS_CUST_DEF)
#define POS_ARTI_SIZE     sizeof(ARTICLE_DEF)
#define POS_BARC_SIZE     sizeof(BARCODE_DEF)
#define POS_PAYM_SIZE     sizeof(PAYMENT_DEF)
#define POS_CURR_SIZE     sizeof(CURR_DEF)
#define POS_CFEE_SIZE     sizeof(CFEE_DEF)
#define POS_MMML_SIZE     sizeof(MMML_DEF)
#define POS_TILL_SIZE     sizeof(TILL_DEF)
#define POS_INVH_SIZE     sizeof(INVOICE_DEF)
#define POS_INVL_SIZE     sizeof(INVOICE_LINE_DEF)
#define POS_INVP_SIZE     sizeof(INVOICE_PAYMENT_DEF)
#define POS_INVI_SIZE     sizeof(INVOICE_PAYM_ITEM_DEF)
#define POS_INVV_SIZE     sizeof(INVOICE_VAT_DEF)
#define POS_MSAM_SIZE     sizeof(INVOICE_MSAM_DEF)
#define POS_INVE_SIZE     sizeof(INVOICE_EOF_DEF)
#define POS_CASH_SIZE     sizeof(CASH_DEF)
#define POS_CART_SIZE     sizeof(CRAZY_ART_DEF)

#define POS_RECO_SIZE     sizeof(SHIFT_TDM_DEF)
#define POS_SHFO_SIZE     sizeof(SHIFT_ON_DEF)
#define POS_SHFL_SIZE     sizeof(SHIFT_LIFT_DEF)
#define POS_SHFF_SIZE     sizeof(SHIFT_OFF_DEF)

#define CTREE_INVE_SIZE   sizeof(INVOICE_EOF_CTREE_DEF)
#define CTREE_SHFO_SIZE   sizeof(SHIFT_ON_CTREE_DEF)
#define CTREE_SHFL_SIZE   sizeof(SHIFT_LIFT_CTREE_DEF)
#define CTREE_SHFF_SIZE   sizeof(SHIFT_OFF_CTREE_DEF)
#define CTREE_XZREAD_SIZE sizeof(XZ_READ_CTREE_DEF)

#define POS_PEND_CUST_SIZE sizeof(POS_PEND_CUST_DEF)
#define POS_PEND_INVL_SIZE sizeof(POS_PEND_INVL_DEF)

#define POS_CUST_FISC_SIZE sizeof(POS_CUST_FISC_DEF)

#define POS_VOUC_SIZE           sizeof(VOUCHER_DEF)
#define POS_VOUC_VALE_SIZE     sizeof(VOUCHER_VALE_DEF)

#define POS_VOUC_ANTICIPO_SIZE     sizeof(VOUCHER_ANTICIPO_DEF)


extern SYSTEM_DEF       pos_system;
extern POS_INVOICE_DEF  pos_invoice;
extern GENVAR_DEF       genvar;
extern POS_CUST_DEF     cust;
extern ARTICLE_DEF      article;
extern CASH_DEF         cash;
extern SHIFT_LIFT_DEF   till_lift;

/*****/
extern double tot_fee_pay;
extern double get_invoice_donation_currmode(); /*  v3.4.5 acm -*/
extern double get_invoice_perception_currmode();
/*****/




#ifdef __cplusplus
}
#endif

#endif
