/*
 *     Module Name       : ST_INV.C
 *
 *     Type              : States Invoicing, InvoicingQty, NPriceArt,
 *                         NPWeightArt, NWWeightArt, VoidLastItem
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
 * 06-Jan-2000 Art_grp_sub_no is also kept in TM (Multisam change)     P.M.
 * --------------------------------------------------------------------------
 * 11-Dec-2000 Check if crazy article request has finished before      R.N.B.
 *             calculating MSAM discounts
 * --------------------------------------------------------------------------
 * 29-Mar-2001 Added OCIA2_DATA.                                       R.N.B.
 * --------------------------------------------------------------------------
 * 17-May-2001 Don't ask for price in case of a deposit article        R.N.B.
 * --------------------------------------------------------------------------
 * 21-Sep-2001 Added Pending Invoice functionality.                      M.W.
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Added Article Finder.                                     M.W.
 * --------------------------------------------------------------------------
 * 22-Jan-2002 Use define of MAX_AMOUNT_ON_POS                           M.W.
 * --------------------------------------------------------------------------
 * 26-Feb-2002 Bugfix: Check fee_amount instead of fee_status in 
 *                     vfy_total_key                                     M.W.
 * --------------------------------------------------------------------------
 * 15-May-2002 Weight barcodes adapted to Colombia Dos situation.        M.W.
 * --------------------------------------------------------------------------
 * 22-Jul-2002 Multisam Phase 3.                                         P.M.
 * --------------------------------------------------------------------------
 * 27-Aug-2003 Added calc_incl_excl_vat_tcf() and adapted
 *             recalc_total_carried_forward() to do a correct
 *             calculation of the total carried forward totals.        J.D.M.
 * --------------------------------------------------------------------------
 * 17-Feb-2004 A Separate 'carried forward' total is used to store           
 *             MultiSam discounts in order to avoid rounding diffs.      P.M.
 * --------------------------------------------------------------------------
 * 06-Jul-2005 Bugfix: Rounding of Multisam discounts in case of 
 *             inclusive amounts on invoice                            M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 * 16-Aug-2005 Select customer if searched by fisc_no                    M.W.
 * --------------------------------------------------------------------------
 * 12-Ago-2011 Change for sales Turkey						                     ACM -
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>


#pragma hdrstop


#include "Template.h" /* Every source should use this one! */
#include "appbase.h"
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "stnetp24.h"                       /* Pos (library) include files   */
#include "comm_tls.h"
#include "intrface.h"
#include "date_tls.h"
#include "misc_tls.h"
#include "stri_tls.h"

#include "inp_mgr.h"
#include "prn_mgr.h"
#include "state.h"
#include "tm_mgr.h"
#include "tot_mgr.h"
#include "bp_mgr.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "err_mgr.h"
#include "mapkeys.h"
#include "sll_mgr.h"
#include "llist.h"

#include "pos_tm.h"                         /* Application include files.    */
#include "pos_recs.h"
#include "WPos_mn.h"
#include "pos_errs.h"
#include "pos_txt.h"
#include "pos_inp.h"
#include "pos_func.h"
#include "pos_keys.h"
#include "pos_vfy.h"
#include "pos_dflt.h"
#include "pos_st.h"
#include "pos_scrl.h"
#include "pos_tot.h"
#include "write.h"
#include "pos_bp1.h"
#include "pos_com.h"
#include "st_main.h"
#include "pos_msam.h"
#include "condll.h"
#include "pos_func.h"

/* 25-Set-2012 acm - { */
#include "QBSocket.h"
/* 25-Set-2012 acm - }*/

/* 27-Jan-2012 acm - { */
#define REGEDIT_ITEM_LEN    50 
#define REGEDIT_ITEM_N      20 


long prom_anniv_price_cupon=0;
long prom_anniv_price_gift =0;
long prom_anniv_date_begin =0;
long prom_anniv_date_end   =0;

/* 27-Jan-2012 acm - } */

long promotion_horeca_date_begin =0;
long promotion_horeca_date_end   =0;

extern int isvigente_horeca();

/* 12-Ago-2011 acm -{ */

/* header */
#include "pos_turkey.h" 
#include "registry.h"
#define LEN_TURKEY_CODE     100
#define VALE_ARTICLE_COUNT  10

/* variables */
TM_ITEM_GROUP c_item_valeturkey;
int valeturkey_press_key=0; /* static init value */

/* v3.4.8 acm - {*/
int horeca_press_key=0; 
extern short vfy_empty_field_Horeca(_TCHAR *data, short key);

extern STATE_OBJ Discount_Horeca_ST; 

extern int is_cust_prom_horeca(long cust_no, long store_no);
/* v3.4.8 acm - }*/

char reg_vale_turkey   [VALE_ARTICLE_COUNT][LEN_TURKEY_CODE]; /* acepta 10 articulos como maximo en el regedit */
char reg_article_turkey[VALE_ARTICLE_COUNT][LEN_TURKEY_CODE]; /* acepta 10 articulos como maximo en el regedit */

int  reg_turkey_load__=0;

/* extern variables */
extern STATE_OBJ ValeTurkey_ST; 

/* 12-Ago-2012 acm - } */

/* 25-Set-2012 acm - {*/

extern short vfy_empty_field_QueueBusting(_TCHAR *data, short key);

typedef struct INVOICE_LINE_ITEM
{
    //++TM_ITEM_GROUP c_item;
    int barcd_type;
    ARTICLE_DEF   article;
    ARTICLE_DEF   deposit;
} INVOICE_LINE_ITEM;


extern STATE_OBJ QueueBusting_ST; 



int     prescan_no	=0;


/*
{
    "7501065906762",	154128,	1.000,	
    "7506195145456",	61386,	1.000,	
    "7750520000204",	24219,	1.000,	
    "7750520000273",	24245,	1.000,	
    "7702011034717",	16445,	1.000,	
    "7750885002653",	94120,	2.000,	
    "7862100600411",	21463,	1.000,	
    "",    0,      0
};
*/



/* 25-Set-2012 acm -} */

/* static funciones  */
static void ValeTurkey_VW(void);
static short turkey_start_proc(_TCHAR *data, short key);
static short decode_internal_ean13_barcode_turkeyV2(void);
extern short decode_internal_ean13_barcode_V2(TM_ITEM_GROUP * p_item);


/* extern funciones */
extern short vfy_key_disable(_TCHAR *data, short key);
extern short ValeTurkey_vfy_and_start_artno(_TCHAR *data, short key);
extern short ValeTurkey_vfy_and_start_artno_detalle(_TCHAR *data, short key);
extern short vfy_paymentway(_TCHAR *data, short key);
extern void  init_item_valeturkey(void);
extern short vfy_weight_valeturkey(_TCHAR *data, short key);
extern int is_vale_article(_TCHAR *vale_turkey);
extern int is_vale_article_i(long vale_turkey);
extern int is_turkey_article_i(long article_turkey);
extern int is_turkey_article(_TCHAR *article_turkey);
extern short accept_active_item_valeturkey(_TCHAR *data, short key) ;
extern double get_base_qty( TM_ITEM_GROUP * c_item , int * p_nerror);
extern short vfy_empty_field_ValeTurkey(_TCHAR *data, short key);

extern short fn_turkey_key(_TCHAR *data, short key);
extern char* substring(char *nuevo,  char* cadena, int comienzo, int longitud);

char* substring(char *nuevo,  char* cadena, int comienzo, int longitud)
{
    if (longitud == 0)
        longitud = strlen(cadena)-comienzo;
   
    nuevo[longitud] = '\0';
    strncpy(nuevo, cadena + comienzo, longitud);
   
    return nuevo;
}

/* 12-Ago-2011 acm -}*/



static short decode_internal_ean13_barcode(void);
static short get_scale_barcode_type(_TCHAR *);
static short SC_get_barcode(_TCHAR *, _TCHAR *);
static void  Invoicing_VW(void);
static void  InvoicingQty_VW(void);
static void  NPriceArt_VW(void);
static void  NPWeightArt_VW(void);
static void  NWWeightArt_VW(void);
static void  VoidLastItem_VW(void);
static short vfy_last_item_present(_TCHAR *, short);
static short repeat_last_item(_TCHAR *, short);
static short toggle_subt_display(_TCHAR *, short);
extern short vfy_total_key(_TCHAR *, short);
static short vfy_item_present(_TCHAR *, short);
static short void_last_item(_TCHAR *, short);
#ifdef PRN_IMM
static short proc_new_page(_TCHAR *, short);
#endif
static short reverse_invoice_line_mode(_TCHAR *, short);
static short recalc_active_item(void);
static short accept_repeated_item(_TCHAR *, short);
extern short process_invoice(_TCHAR *, short);



/*---------------------------------------------------------------------------*/
/*                             Input_UVW                                     */
/*---------------------------------------------------------------------------*/
void Input_UVW(void)
{
  /* An unview-element used in various states.                             */
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  SetShowCursor(FALSE);
} /* Input_UVW */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   INVOICING                                               */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* This is the central state of the invoice process. In this state the       */
/* normal handling of articles is performed. Special functions such          */
/* as crediting an article are initiated in this state but handled in other  */
/* states.                                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/


void InvoicingScrl_VW(void)
{
  /*                                                                       */
  /* Draw the scrolling header line, if one does not exist, create it.     */
  /*                                                                       */
  if (scrl_update_header_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_HEADER, 0)
      == SCRL_UNKNOWN_LINE) {
    scrl_add_header_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_HEADER, 0);
  }
  scrl_redraw_lines(INV_ART_LIST_WINDOW);

  return;
} /* InvoicingScrl_VW */


void InvoicingHead_VW(void)
{
  _TCHAR buffer[16], buffer1[16];
  _TCHAR *cust_no;

  /*                                                                       */
  /* Draw all invoice-header information: till-no, cash-no, cust-no        */
  /* sys-date and the invoice-number.                                      */
  /*                                                                       */
  scrn_select_window(INV_HEADER_WINDOW);

  /* Checkout nbr / Cashier nbr */
  _tcscpy(buffer, _T("   "));
  _tcscpy(buffer1, _T("   "));
  _itot(pos_system.till_no, buffer+2, DECIMAL);
  _itot(c_shft.cashier, buffer1+2, DECIMAL);
  _tcscat(buffer, buffer1+(_tcslen(buffer1)-3));
  format_display(&dsp_tillno_cashno6, buffer+(_tcslen(buffer)-6));

  /* Customer number, Customer name */
  *buffer=_T('\0');
  *buffer1=_T('\0');
  cust_no=buffer1;
  if (cust.cust_no >= 0) {
    ch_memset(buffer1, _T('0'), 7*sizeof(_TCHAR));
    _itot(cust.store_no, buffer, DECIMAL);
    ftoa((double)cust.cust_no, 7, buffer1+6);
    cust_no=buffer1+_tcslen(buffer1)-6;
  }
  if (state_get() != &CustomerMode_ST) {
    format_display(&dsp_cust2, buffer);
    format_display(&dsp_cust6, cust_no);
    format_display(&dsp_cust30, cust.name);
  }

  /* Invoice number: in case of a previous invoice, show previous number.  */
//  if (cust.cust_no >= 0) {
//    dec_invoice_no();
//    get_invoice_no(buffer);
//    scrn_string_out(buffer,0,57);
//    inc_invoice_no();
//  }
//  else {
//    get_invoice_no(buffer);
//    scrn_string_out(buffer,0,57);
//  }

  get_invoice_no(buffer);
  scrn_string_out(buffer,0,57);

  /* System date */
  prn_fmt_date(pos_system.run_date, genvar.date_format, mon_names, buffer);
  scrn_string_out(buffer,0,68);
} /* InvoicingHead_VW */


void InvoicingDescr_VW(void)
{
  TM_ITEM_GROUP h_item;

  /*                                                                       */
  /* Clear article description and goods-value fields.                     */
  /*                                                                       */
  if (display_item!=TM_ROOT) {
    if (display_item!=C_ITEM) {
      memcpy(&h_item, &c_item, sizeof(TM_ITEM_GROUP));
      tm_read_nth(TM_ITEM_NAME, (void*)&c_item, display_item);
      if (state_get() != &CustomerMode_ST) {
        view_item(CUST_SCRN);
      }
      memcpy(&c_item, &h_item, sizeof(TM_ITEM_GROUP));
    }
    else {
      if (state_get() != &CustomerMode_ST) {
        view_item(CUST_SCRN);
      }
    }
  }
} /* InvoicingDescr_VW */


/*---------------------------------------------------------------------------*/
/*                              view_total                                   */
/*---------------------------------------------------------------------------*/
void view_total(short screen)
{
  _TCHAR buffer[32];
  unsigned short curr_scrn = scrn_get_current_window();

  /*                                                                       */
  /* Display total amount on operator screen and display (sub)total        */
  /* on customer screen. 'Screen' determines the screens which are         */
  /* updated.                                                              */
  /*                                                                       */

  if (state_get() == &CustomerMode_ST) {
    *buffer = _T('\0');
  }
  else {
    ftoa_price(tot_ret_double(TOT_GEN_INCL), TOTAL_BUF_SIZE, buffer);
    if (STR_ZERO(buffer)) {
      *buffer=(_TCHAR)0;
    }
  }
  if (screen & OPER_SCRN) {
    scrn_select_window(INV_TOT_WINDOW);
    scrn_string_out(scrn_inv_TXT[7],0,0);       /* TOTAL         */
    format_string(&string_price11,buffer);
    scrn_string_out(string_price11.result,0,8);
  }

  if ((screen & CUST_SCRN) && subt_display) {
    cdsp_clear();
    cdsp_write_string(cdsp_TXT[4],0,0);
    format_string(&string_price11, buffer);
    cdsp_write_string(string_price11.result,1,CDSP_RIGHT_JUSTIFY);
  }
  scrn_select_window(curr_scrn);
} /* view_total */


static void Invoicing_VW(void)
{
  unsigned short save_window;

  static short cls_states[]={
     ST_DO_TOTAL
    ,ST_SHOW_CHANGE
    ,ST_CUSTOMER_MODE
    ,ST_CUSTOMER_FEE
    ,ST_CALCULADORA
    ,ST_START_ART_FIND
    ,ST_ART_FIND_RESULT
    ,ST_SELECT_CUST
    ,ST_SELECT_DOCUMENT //v3.6.1 acm -
    ,ST_PASSDAY700 //mname
    ,ST_PREV_DO_TOTAL
    ,ST_NAME //mname
    ,ST_PERCEPTION_NAME
    ,ST_PREV_AMNT_ENOUGH
    ,0
  };


  if (called_by_state(cls_states)==SUCCEED) {
    cdsp_clear();
    cls();
    InvoicingHead_VW();
    InvoicingDescr_VW();
    InvoicingScrl_VW();
  }
  else {
    save_window = scrn_get_current_window();
    scrn_clear_window(ERROR_WINDOW_ROW1);
    scrn_clear_window(ERROR_WINDOW_ROW2);
    scrn_select_window(save_window);
  }

  view_total( OPER_SCRN | CUST_SCRN );
  InvoicingSubt_VW();

  invoice_line_mode=invoice_mode;                 /* Reset to default mode.*/
#ifndef NO_VIEW_POS_STATE
  view_pos_state();
#endif
  init_item();
  init_item_valeturkey(); /* 12-Ago-2011 acm - initialization of var c_item_valeturkey */
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[3],0,16); /* ENTER ITEM / QTY              */
} /* Invoicing_VW */


void InvoicingSubt_VW(void)
{
  _TCHAR buffer[32];
  unsigned short curr_scrn = scrn_get_current_window();

  /*                                                                      */
  /* Display the goods-value exclusive vat, the vat-value and customer    */
  /* fee amount.                                                          */
  /*                                                                      */
  /* Right justify some strings, supporting translation                   */
  /* word length differences                                              */
  /*                                                                      */

  scrn_select_window(INV_SUBT_WINDOW);
  scrn_string_out(scrn_inv_TXT[9],0,0);      /* VAT               */


  if (state_get() == &CustomerMode_ST) {
    *buffer = _T('\0');
  }
  else {
    ftoa_price(tot_ret_double(TOT_GEN_VAT), TOTAL_BUF_SIZE, buffer);
    if (STR_ZERO(buffer)) {
      *buffer=(_TCHAR)0;
    }
  }

  format_string(&string_price11,buffer);
  scrn_string_out(string_price11.result,0,4);

  scrn_string_out(scrn_inv_TXT[10],1,0);     /* GDS               */

  if (state_get() == &CustomerMode_ST) {
    *buffer = _T('\0');
  }
  else {
    ftoa_price(tot_ret_double(TOT_GEN_EXCL), TOTAL_BUF_SIZE, buffer);
    if (STR_ZERO(buffer)) {
      *buffer=(_TCHAR)0;
    }
  }

  format_string(&string_price11,buffer);
  scrn_string_out(string_price11.result,1,4);

  if (pos_invoice.invoice_fee_status != FEE_STAT_NONE) {
    scrn_string_out(scrn_inv_TXT[13],1,20);  /* FEE               */
    if (state_get() == &CustomerMode_ST) {
      *buffer = _T('\0');
    }
    else {
      ftoa_price(tot_ret_double(TOT_FEE_AMOUNT * 100), TOTAL_BUF_SIZE, buffer);
      if (STR_ZERO(buffer)) {
       *buffer=(_TCHAR)0;
      }
    }
    format_string(&string_price9_2,buffer);
    scrn_string_out(string_price9_2.result,1,24);
  }

  scrn_select_window(curr_scrn);
} /* InvoicingSubt_VW */


extern INPUT_CONTROLLER Dartno14KO14n =
{
  (INPUT_DISPLAY *)&dsp_artno14,
  KEYBOARD_MASK | OCIA1_MASK | OCIA2_MASK,
  17, 50,
  (VERIFY_KEY *)&numeric_dnull
};




/*                                                                           */
/* The keys listed below are also checked in the approval part of the        */
/* state engine. If a key is pressed, the number of invoice lines are        */
/* tested to be less 999 and the customer data is checked for a blocked      */
/* customer (due to the asynchron arival of customer date).                  */
/*                                                                           */


static VERIFY_ELEMENT Invoicing_VFY[] =
{
  NEW_PAGE_KEY,            (void *)NULL,           /* calculator */
  LINE_UP_KEY,             handle_scroll_key,
  LINE_DOWN_KEY,           handle_scroll_key,
  PAGE_UP_KEY,             handle_scroll_key,
  PAGE_DOWN_KEY,           handle_scroll_key,
  TIMES_KEY,               vfy_art_qty,
  OCIA1_DATA,              vfy_and_start_artno,    /* Returns ARTICLE_OK,   */
  OCIA2_DATA,              vfy_and_start_artno,    /* Returns ARTICLE_OK,   */
  ENTER_KEY,               vfy_and_start_artno,    /* PRICE_ART, PWEIGHT_ART*/
  VOID_LAST_ITEM_KEY,      vfy_last_item_present,  /* ARTICLE_NOT_OK if     */
  REPEAT_LAST_ITEM_KEY,    vfy_last_item_present,  /* illegal art UNKNOWN_K */
  VOID_LINE_KEY,           vfy_item_present,
  VOID_INVOICE_KEY,        (void *)NULL,
  SAVE_INVOICE_KEY,        vfy_total_key,
  CREDIT_KEY,              vfy_and_clear_field,
  DISCOUNT_KEY,            vfy_empty_field,
  SUB_TOTAL_KEY,           toggle_subt_display,
  TOTAL_KEY,               vfy_total_key,
#ifdef PRN_IMM
  NEW_PAGE_KEY,            proc_new_page,
#endif
  PRINTER_UP_KEY,          proc_new_line,
  CLEAR_KEY,               vfy_clear_key,
  OPEN_DRAWER_KEY,         open_and_close_drawer,  /* Approval in state-engine. */
  ART_FINDER_KEY,          (void *)NULL,
  TURKEY_KEY,              vfy_empty_field_ValeTurkey, /* FMa - Vale Pavos */ /* 12-Ago-2011 acm - modified*/
  QUEUEBUSTING_KEY,        vfy_empty_field_QueueBusting,                      /* 25-Set-2012 acm - modified*/
  HORECA_KEY,              vfy_empty_field_Horeca, /*  v3.4.8 acm -*/
  UNKNOWN_KEY,             illegal_fn_key
};


static PROCESS_ELEMENT Invoicing_PROC[] =
{
  ARTICLE_OK,              accept_active_item,     /* PRICE_TOO_LARGE_KEY   */
                                                   /* if overflow           */
  REPEAT_LAST_ITEM_KEY,    repeat_last_item,
  VOID_INVOICE_KEY,        close_and_log_invoice,  /* See pos_appr.c for    */
                                                   /* Superv. approval.     */
  SAVE_INVOICE_KEY,        save_invoice,           /* See pos_appr.c for    */
  CREDIT_KEY,              reverse_invoice_line_mode,
  
  TOTAL_KEY,               process_invoice,
  NEW_PAGE_KEY,            inic_calc,              /* calculator */
  ART_FINDER_KEY,          (void *)NULL,
  UNKNOWN_KEY, (void *)NULL
};


static CONTROL_ELEMENT Invoicing_CTL[] =
{
  ARTICLE_OK,              &Invoicing_ST,      /* Accepted art, next art.   */
  ARTICLE_NOT_OK,          &Invoicing_ST,
  TIMES_KEY,               &InvoicingQty_ST,   /* Qty entered,get artno/qty */
  PRICE_ART,               &NPriceArt_ST,      /* Price-art : get art-price */
  WWEIGHT_ART,             &NWWeightArt_ST,    /* Weight-art: get art-weight*/
  PWEIGHT_ART,             &NPWeightArt_ST,    /* Weight-art: get art-price */
  TOTAL_KEY,               &DoTotal_ST,        /* Start tendering           */
  PASSDAY700_KEY,          &Passday700_ST,      //mlsd
  PREV_TOTAL_KEY,          &Prev_DoTotal_ST,        /* Start tendering           */
  PREV_AMNT_ENOUGH,        &Prev_Amnt_Enough_ST,    /* Start tendering           */

  AMNT_ENOUGH,             &ShowChange_ST,     /* Start tender,total is neg.*/
  VOID_LAST_ITEM_KEY,      &VoidLastItem_ST,
  VOID_LINE_KEY,           &Correction_ST,
  VOID_INVOICE_KEY,        &CustomerMode_ST,
  SAVE_INVOICE_KEY,        &CustomerMode_ST,
  REPEAT_LAST_ITEM_KEY,    &Invoicing_ST,      /* Repeat last item          */
  CREDIT_KEY,              &Credit_ST,
  DISCOUNT_KEY,            &Discount_ST,
  PRICE_TOO_LARGE_KEY,     &Invoicing_ST,      /* Item is deleted, back to inv. */
  NEW_PAGE_KEY,            &Calculadora_ST,    /* calculator */
  ART_FINDER_KEY,          &StartArtFind_ST,
//  TURKEY_KEY,              &Turkey_ST,       /* FMa - Vale Pavos */   /* 12-Ago-2011 acm - remove */
  TURKEY_KEY,              &ValeTurkey_ST,                              /* 12-Ago-2011 acm - add    */
  QUEUEBUSTING_KEY,        &QueueBusting_ST,   /* 25-Set-2012 acm - add    */
  HORECA_KEY,              &Discount_Horeca_ST,   /* v3.4.8 acm - add    */
  UNKNOWN_KEY,             &Invoicing_ST
};


extern STATE_OBJ NWWeightArt_valeturkey_ST ;
static CONTROL_ELEMENT	ValeTurkey_CTL[] =
{
  ARTICLE_OK,           &ValeTurkey_ST,
  ARTICLE_NOT_OK,       &ValeTurkey_ST,
   
  //PRICE_ART,            &NPriceArt_ST,
  WWEIGHT_ART,          &NWWeightArt_valeturkey_ST,
  //PWEIGHT_ART,          &NPWeightArt_ST,

  ART_FINDER_KEY,       &StartArtFind_ST,

  NO_KEY,               &Invoicing_ST,
  ENTER_KEY,            &Invoicing_ST,

  /*
  TIMES_KEY,            &DiscountQty_ST,
  PRICE_ART,            &NPriceArt_ST,
  WWEIGHT_ART,          &NWWeightArt_ST,
  PWEIGHT_ART,          &NPWeightArt_ST,
  NO_KEY,               &Invoicing_ST,
  ART_FINDER_KEY,       &StartArtFind_ST,
  UNKNOWN_KEY,          &StartArtFind_ST //Discount_ST
  */
};
extern STATE_OBJ Invoicing_ST =
{
  ST_INVOICING,
  Invoicing_VW,
  art_no_DFLT,
  &Dartno14KO14n,
  Invoicing_VFY,
  Input_UVW,
  Invoicing_PROC,
  Invoicing_CTL
};

/* 25-Set-2012 acm -{ */
extern short barcd_to_artno(_TCHAR *artno, _TCHAR *barcd);


extern STATE_OBJ NWWeightArt_QueueBusting_ST ;
extern VERIFY_ELEMENT QueueBusting_VFY[] ;

static CONTROL_ELEMENT	QueueBusting_CTL[] =
{
  ARTICLE_OK,           &QueueBusting_ST,
  ARTICLE_NOT_OK,       &QueueBusting_ST,
   
  //PRICE_ART,            &NPriceArt_ST,
  //---WWEIGHT_ART,          &NWWeightArt_QueueBusting_ST,
  WWEIGHT_ART,          &NWWeightArt_ST,
  //PWEIGHT_ART,          &NPWeightArt_ST,

  ART_FINDER_KEY,       &StartArtFind_ST,

  NO_KEY,               &Invoicing_ST,
  ENTER_KEY,            &Invoicing_ST,

  /*
  TIMES_KEY,            &DiscountQty_ST,
  PRICE_ART,            &NPriceArt_ST,
  WWEIGHT_ART,          &NWWeightArt_ST,
  PWEIGHT_ART,          &NPWeightArt_ST,
  NO_KEY,               &Invoicing_ST,
  ART_FINDER_KEY,       &StartArtFind_ST,
  UNKNOWN_KEY,          &StartArtFind_ST //Discount_ST
  */
};

extern VERIFY_ELEMENT ValeTurkey_VFY[];



/*---------------------------------------------------------------------------*/
/*                           vfy_empty_field_QueueBusting                    */
/*---------------------------------------------------------------------------*/
short vfy_empty_field_QueueBusting(_TCHAR *data, short key)
{
  ///valeturkey_press_key = 1; /* 12-Ago-2011 acm - flag vale turkey key is pressed*/
  return vfy_empty_field(data, key);
} /* vfy_empty_field_ValeTurkey*/

short fn_QueueBusting(_TCHAR *data, short key)
{
  return key;
}


/*-------------------------------------------------------------------------*/
/*                       QueueBusting_VW								   */
/*-------------------------------------------------------------------------*/
void QueueBusting_VW(void)	/* Se ejecuta al presionar el boton pavos */
{
    /*
  if (c_item_valeturkey.arti.display_status ==   ARTICLE_OK){
		scrn_clear_window(INV_ART_INPUT_WINDOW);
	  scrn_string_out(input_TXT[55], 0, 16);      //0055 _T("INGRESE ART-PAVO / CANT.")    

	}else
	{*/
		scrn_clear_window(INV_ART_INPUT_WINDOW);
		scrn_string_out(input_TXT[57], 0, 14+2+3);//fix2      // 0056 _T("SCANEE EL QUEUE TICKET")   
	//}
} 

/*-------------------------------------------------------------------------*/
/*                       QueueBusting_proc_cancel                          */
/*-------------------------------------------------------------------------*/
short QueueBusting_proc_cancel(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Reset display_item so we get an empty description/cust-screen.        */
  /*                                                                       */

  display_item=TM_ROOT;

  return(key);
} 


/*---------------------------------------------------------------------------*/
/*                ValeTurkey_is_val_art_no                                   */
/*---------------------------------------------------------------------------*/


int  QueueBusting_try_init_item(
                _TCHAR * data,
                //double qty,
                TM_ITEM_GROUP  * c_item,
                INVOICE_LINE_ITEM *additional_val // c_item_retornado
                )
{
  /*                                                                       */
  /* Get article and a possible deposit from the PLU.                      */
  /* - data can be an article number or a barcode.                         */
  /* - a possible qty is already in c_item.arti.base.qty.                  */
  /*                                                                       */
  /* Returns:                                                              */
  /*   ARTICLE_NOT_OK if any error.                                        */
  /*    UNKNOWN_KEY   if any error                                         */ 
  /* else                                                                  */
  /*   ARTICLE_OK     item is complete, can be accepted                    */

  _TCHAR  artno[16];
///  _TCHAR  buff_weight[100];
  double qty_ena13=0;

   int * barcd_type;
   ARTICLE_DEF   *article;
   ARTICLE_DEF   *deposit;
 

  short  status;

   barcd_type   =&additional_val->barcd_type;
   article      =&additional_val->article;
   deposit      =&additional_val->deposit;
  // initializacion values --------------

  	if (STR_ZERO(data)) 
	{
		*data = _T('\0');
		err_invoke(INVALID_ARTNO);
		delete_active_item(empty,0);
		return(ARTICLE_NOT_OK);
	}
    /*                                                                     */
    /* Step 1: Convert data to an article number.                          */
    /*  																   */

      if (get_barcode_type(data)==BARCD_INTERNAL_EAN13) {
	      
        *data=_T('\0');
        err_invoke(NO_DISC_ON_R2C_BARCD);
        return(UNKNOWN_KEY);
      }

      *barcd_type = get_barcode_type(data);
	  
      if (*barcd_type != BARCD_ILLEGAL) 
	  {  /* It's a barcode..    */
        if (barcd_to_artno(artno, data) == SUCCEED) 
		{
          if (*barcd_type == BARCD_EXTERNAL_EAN128) 		  
		  {
            _tcsncpy(c_item->arti.base.bar_cd, (data+3), 13);
            c_item->arti.base.bar_cd[14] = _T('\0');
          }else  
		  {
            if (*barcd_type == BARCD_EXTERNAL_DUMP14) 
			{
              _tcsncpy(c_item->arti.base.bar_cd, (data+1), 14);
              c_item->arti.base.bar_cd[15] = _T('\0');
            }else 
			{
              _tcscpy(c_item->arti.base.bar_cd, data);
            }
          }
          c_item->arti.base.art_no = _ttol(artno);
        }else 
		{
          delete_active_item(empty,0);
          return(ARTICLE_NOT_OK);
        }
      }
      else {
	    /* It's an article no. */
        c_item->arti.base.art_no=_ttol(data);
      }


    // validar barcode and art_no

    /*                                                                   */
    /* Step 2: Get article data from PLU and handle exceptions.          */
    /*                                                                   */
    article->art_no = c_item->arti.base.art_no;
    status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
								(void*)article, (short) keyEQL);
    if (status != SUCCEED) {
      err_invoke(INVALID_ARTNO);
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
    }
    else if (article->art_no != c_item->arti.base.art_no) {
      err_invoke(DATA_CORRUPT);                /* article file corrupted */
      delete_active_item(empty,0);             /*  rebuild with stnetp85 */
      return(ARTICLE_NOT_OK);
    }

    /*                                                                   */
    /* Step 2a: No discount allowed on a sold-deposit article.           */
    /*                                                                   */
    if (article->type == ART_IND_DEPOSIT &&
        (state_number() == ST_DISCOUNT ||
         state_number() == ST_DISCOUNT_QTY) ) {
      err_invoke(NO_DISCOUNT_ON_DEPOSIT);
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
    }

    /*                                                                   */
    /* Step 2b: If article is blocked, ask for S-key.                    */
    /*                                                                   */
    if (article->block_ind == YES) {
      if (err_invoke(ARTICLE_BLOCKED) != SUCCEED) {
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
      else {
        c_item->approved = TRUE;
      }
    }

    
    /*                                                                   */
    /* Step 2c: Put all article information in the current item.         */
    /*                                                                   */
    if (c_item->arti.base.qty == 0.0) 
    {
          if (invoice_line_mode == SALES) {
            c_item->arti.base.qty = 1;
          }
          else {
            c_item->arti.base.qty = -1;
          }
    }

/*    
        // fix2 acm -
    ///if (1){
        c_item->arti.base.qty = qty;

    //}else{
       // c_item->arti.base.qty = qty;

        if (c_item->arti.base.qty == 0.0) {
          if (invoice_line_mode == SALES) {
            c_item->arti.base.qty = 1;
          }
          else {
            c_item->arti.base.qty = -1;
          }
        }

        if (*barcd_type == BARCD_VALE_TURKEY_EAN13)
        {
          
            //c_item->arti.base.qty = _ttol(buff_weight);
            qty_ena13=_ttol(buff_weight);
            if (fabs(qty_ena13-c_item->arti.base.qty )>0.0001){
                //QTY DEL EAN NO ES IGUAL A QTY LEIDO EN QUEUSBUSTING
                return(ARTICLE_NOT_OK);

            }
        }
        
    //}
    */

    _tcsncpy(c_item->arti.pack_type, article->pack_type, 3);
    c_item->arti.art_ind             = article->type;
    c_item->arti.mmail_no            = article->mmail_no;
    c_item->arti.items_per_pack      = article->cont_sell_unit;
    c_item->arti.base.vat_no         = article->vat_no;
    c_item->arti.base.vat_perc       = get_vat_perc(article->vat_no);
    c_item->arti.base.art_grp_no     = article->art_grp_no;
    c_item->arti.base.art_grp_sub_no = article->art_grp_sub_no;
    c_item->arti.base.dept_cd        = article->dept_cd;

    c_item->arti.base.arti_retention_ind  = article->arti_retention_ind ;  //v3.6.1 acm -
    c_item->arti.base.arti_perception_ind = article->arti_perception_ind;  //v3.6.1 acm -
    c_item->arti.base.arti_rule_ind       = article->arti_rule_ind      ;  //v3.6.1 acm -
    c_item->arti.base.percep_amount       = 0;                            //v3.6.1 acm -
    c_item->arti.base.arti_detraccion_ind          = article->arti_detraccion_ind;                            ; //v3.6.2 wjm -
    _tcscpy(c_item->arti.base.descr, article->descr);
    _tcscpy(c_item->arti.reg_no, article->reg_no);
    c_item->arti.base.suppl_no       = article->suppl_no;

    if (genvar.price_incl_vat == INCLUSIVE) {
   
	    c_item->arti.base.price =
                floor_price(calc_incl_vat(article->sell_pr, article->vat_no));

    }
    else {
      c_item->arti.base.price = floor_price(article->sell_pr);
    }

    /*                                                                   */
    /* Step 2d: If article is ok till now, calculate goods-value.        */
    /*          - If it is a barcode, first decode the barcode and then  */
    /*            calculate the goods-value                              */
    /*                                                                   */

    if (*barcd_type == BARCD_INTERNAL_EAN13 ||
          *barcd_type == BARCD_SCALE_EAN13    ||
          *barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) 
    { 
        /* treated as internal */
        status = decode_internal_ean13_barcode_V2(c_item);

        if ( status!= SUCCEED) { //lectura del peso dentro del barcode EAN13 
                           /* returns FAIL / SUCCEED / PRICE_TOO_LARGE or  */
                           /* DISCNT_AMNT_TOO_LARGE  /WEIGHT_TURKEY_MUSTBE7              */
          //descontar el peso del vale
      
          delete_active_item(empty,0);
          return(ARTICLE_NOT_OK);
        }
      }
      else if (calc_gds_value(ART_GROUP) != SUCCEED) {
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
    }
    /*                                                                   */
    /* Step 3:  If this article has a deposit, get it and handle         */
    /*          exceptions                                               */
    /*                                                                   */
    if (article->type != ART_IND_DEPOSIT &&
        article->art_no_deposit != 0L) {

		  deposit->art_no = article->art_no_deposit;
		  status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
								 (void*)deposit, (short) keyEQL);
		  if (status != SUCCEED) {
			  err_invoke(INVALID_ARTNO);
			  delete_active_item(empty,0);
			  return(ARTICLE_NOT_OK);
		  }
		  else if (deposit->art_no != article->art_no_deposit) {
			  err_invoke(DATA_CORRUPT);               /* article file corrupted  */
			  delete_active_item(empty,0);            /*  rebuild with stnetp85  */
			  return(ARTICLE_NOT_OK);
		  }

		  /*                                                                   */
		  /* Step 3a: If deposit is blocked, ask for S-key.                    */
		  /*                                                                   */
		  if (deposit->block_ind == YES) {
			  if (err_invoke(ARTICLE_BLOCKED) != SUCCEED) {
			    delete_active_item(empty,0);
			    return(ARTICLE_NOT_OK);
			  }
			  else {
			    c_item->approved = TRUE;
			  }
		  }

		  /*                                                                   */
		  /* Step 3b: Put all deposit information in the current item.         */
		  /*                                                                   */
		  if (article->type == ART_IND_WEIGHT) {
			  if (invoice_line_mode == SALES) {
			    c_item->depo.base.qty = 1;
			  }
			  else {
			    c_item->depo.base.qty = -1;
			  }
		  }
		  else {
			  c_item->depo.base.qty = c_item->arti.base.qty;
		  }


		  _tcsncpy(c_item->depo.pack_type, deposit->pack_type, 3);
		  c_item->depo.mmail_no            = deposit->mmail_no;
		  c_item->depo.base.art_no         = deposit->art_no;
		  c_item->depo.base.vat_no         = deposit->vat_no;
		  c_item->depo.base.vat_perc       = get_vat_perc(deposit->vat_no);
		  c_item->depo.base.art_grp_no     = deposit->art_grp_no;
		  c_item->depo.base.art_grp_sub_no = deposit->art_grp_sub_no;
		  c_item->depo.base.dept_cd        = deposit->dept_cd;
		  _tcscpy(c_item->depo.base.descr, deposit->descr);
		  _tcscpy(c_item->depo.reg_no,	    deposit->reg_no);
		  c_item->depo.base.suppl_no       = deposit->suppl_no;

		  if (genvar.price_incl_vat == INCLUSIVE) {
			  c_item->depo.base.price =
					floor_price(calc_incl_vat(deposit->sell_pr, deposit->vat_no));
		  }
		  else {
			  c_item->depo.base.price = floor_price(deposit->sell_pr);
		  }

		  /*                                                                   */
		  /* Step 3c: If deposit is ok till now, calculate goods-value.        */
		  /*                                                                   */
		  if (calc_gds_value(DEPOSIT_GROUP) != SUCCEED) {
			delete_active_item(empty,0);
			return(ARTICLE_NOT_OK);
		  }
    } /* if, end of the deposit section. */
	
	return (ARTICLE_OK); // las validacions son conformes entonces continuar con el flujo.
}



int QueueBusting_loop_articles( char * queue_busting_id ,
                                    int process, 
                                    char * buffer_article, int buffer_article_len)
{
    short         status;
    double        data_value_fix=0;
    char data[100];

    _TCHAR dataQueue[100];
    _TCHAR art_no_str[100];
    INVOICE_LINE_ITEM additional_val; 

    int index_queue=0;
    status=  ARTICLE_OK;

    
    strcpy(data,"OK");

    while ((article_queue[index_queue].art_no)&&(status==ARTICLE_OK))
    {
        if (strlen(article_queue[index_queue].barcode)>0)
            strcpy(dataQueue, article_queue[index_queue].barcode);
        else{
            itoa(article_queue[index_queue].art_no, art_no_str,10);
            strcpy(dataQueue, art_no_str);
        }
        status = QueueBusting_try_init_item(dataQueue, &c_item, &additional_val);

        if (status!=ARTICLE_OK) {
              break;
        }
        
        //Verificar que el barcode leido en QB sea el grabado en el articulo 
        if (strlen(article_queue[index_queue].barcode)>0)
        {
            if (strcmp(article_queue[index_queue].barcode,c_item.arti.base.bar_cd)!=0)
            {
                status = ARTICLE_NOT_OK;
                if (status!=ARTICLE_OK) break;
            }
        }
        
        //Verificar que el articulo leido en QB sea el grabado en el articulo 
        if (article_queue[index_queue].art_no!=c_item.arti.base.art_no)
        {
            status =  ARTICLE_NOT_OK;
            if (status!=ARTICLE_OK) break;
        }
        //
        status=c_item.arti.display_status;
		switch (additional_val.article.type) 
        {
		    case ART_IND_NORMAL:
                c_item.arti.base.qty=article_queue[index_queue].qty;   //
			    status =  ARTICLE_OK;
			  break;

		    case ART_IND_DEPOSIT:  
			    status= ARTICLE_NOT_OK; 
			  break;

		    case ART_IND_PRICE:
			  if (additional_val.barcd_type == BARCD_INTERNAL_EAN13 ||
				  additional_val.barcd_type == BARCD_SCALE_EAN13    ||
				  additional_val.barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) 
              {
				    c_item.arti.base.qty=article_queue[index_queue].qty;   //  
                    status= ARTICLE_OK;
			  }
			  else 
              {
			        status = PRICE_ART;
			  }
			  break;

		    case ART_IND_WEIGHT:
			  if (additional_val.barcd_type == BARCD_INTERNAL_EAN13 ||
				  additional_val.barcd_type == BARCD_SCALE_EAN13    ||
				  additional_val.barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) 
              {
                 c_item.arti.base.qty=article_queue[index_queue].qty;
                  status =  ARTICLE_OK;
                 /*
                  if (fabs(article_queue[index_queue].qty - c_item.arti.base.qty) <=0.0001)
                {
       			    c_item.arti.base.qty=article_queue[index_queue].qty;

                    status =  ARTICLE_OK;
                    //fix2

                }else
                {
                    //QTY DEL EAN NO ES IGUAL A QTY LEIDO EN QUEUSBUSTING
                    status =  ARTICLE_NOT_OK;
                }
                */
/*              fix2
                if ((article_queue[index_queue].qty- c_item.arti.base.qty) >0.0001)
                {
                    //QTY DEL EAN NO ES IGUAL A QTY LEIDO EN QUEUSBUSTING
                    status =  ARTICLE_NOT_OK;
                }*/
			  }
			  else {
			    if (genvar.price_weight == WEIGHT) 
                {
                    if (article_queue[index_queue].qty>0){
                        c_item.arti.base.qty=article_queue[index_queue].qty;   
                        status =  ARTICLE_OK;
                    }else{
				        status = WWEIGHT_ART;
                    }
			    }
			    else {
				  status = PWEIGHT_ART;
			    }
			  }
			  break;
		    default:
			  status = ARTICLE_NOT_OK; 
			  break;
		}
        if (calc_gds_value(ART_GROUP)==PRICE_TOO_LARGE) 
        {
              status=UNKNOWN_KEY;   /* Value overflow. */
        }


        if (status==ARTICLE_OK) 
        {
            if (process)
            {
                c_item.arti.display_status=ARTICLE_OK;
                if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
			              == SCRL_UNKNOWN_LINE) {
			              scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
	             }
                view_item(OPER_SCRN | CUST_SCRN);
                status = accept_active_item(data, ARTICLE_OK);
            }

        }else
        {
            break;// todos los articulos deben retornar ARTICLE_OK caso contrario no continuar
        }
        index_queue++;
    }

    if ((buffer_article)&&(buffer_article_len>0))
    {
       buffer_article[0]=0;// inicializar
       if ((status!=ARTICLE_OK)&&(article_queue[index_queue].art_no>0))
       {
            char buff[2000];
            sprintf(buff,
                    format_qb_msg ,
                    article_queue[index_queue].barcode, 
                    article_queue[index_queue].art_no, 
                    article_queue[index_queue].qty);
            strncpy(buffer_article,buff,buffer_article_len);
            buffer_article[buffer_article_len-1]=0;
       }
    }
    //if (status ==ARTICLE_OK)
            ///status =ENTER_KEY;
   return status;
}

/*---------------------------------------------------------------------------*/
/*                QueueBusting_vfy_and_start_artno_detalle                   */
/*---------------------------------------------------------------------------*/
short QueueBusting_vfy_and_start_artno_detalle(_TCHAR *data, short key)
{
  short         status;

  /*                                                                       */
  /* Get article and a possible deposit from the PLU.                      */
  /* - data can be an article number or a barcode.                         */
  /* - a possible qty is already in c_item.arti.base.qty.                  */
  /*                                                                       */
  /* Returns:                                                              */
  /*   ARTICLE_NOT_OK if any error.                                        */
  /*   UNKNOWN_KEY    if any error.                                        */
  /* else                                                                  */
  /*   ARTICLE_OK     item is complete, can be accepted                    */
  /*   PRICE_ART      price must be entered                                */
  /*   WWEIGHT_ART    weight article, enter weight                         */
  /*   PWEIGHT_ART    weight article, enter price                          */
  /*                                                                       */
  
  char  queue_busting_id[100];
  int rval=0;
  char buffer_article   [500];


  //verificar que no este en blanco la informacion 
  if (STR_ZERO(data)) {
    *data = _T('\0');
    //err_invoke(INVALID_ARTNO);
    err_invoke(QB_CODE_QUEUE_BUSTING_INVALID);
    delete_active_item(empty,0);
    return(ARTICLE_NOT_OK);
  }


  status           =   ARTICLE_OK;
  strcpy(queue_busting_id ,data);

  rval=start_client(5010,"backoffi",queue_busting_id, 1, pos_system.till_no, cust.cust_no, pos_system.store_no,
      buffer_article,sizeof(buffer_article));
  if (rval==0)
  {
      status=QueueBusting_loop_articles(queue_busting_id,0, buffer_article,sizeof(buffer_article));
      if (status == ARTICLE_OK )
      {
         prescan_no  = atoi(data);
      }else{
          status=UNKNOWN_KEY;
          //sprintf(error_qb_msg,"Barcode:%s art_no:%s Qty:%d",barcode, art_no, qty);
          error_extra_msg=buffer_article;
          err_invoke(QB_ARTICLE_QUEUE_BUSTING_INVALID);
          *data = _T('\0');
      }
  }else
  {
    status=UNKNOWN_KEY;

    if(rval==QB_CONECTION_QUEUE_BUSTING_ERROR)  err_invoke(QB_CONECTION_QUEUE_BUSTING_ERROR);
    else if(rval==QB_ARTICLE_QUEUE_BUSTING_INVALID)  {
        error_extra_msg=buffer_article;
        err_invoke(QB_ARTICLE_QUEUE_BUSTING_INVALID);
    }
    else if(rval==QB_FORMAT_QUEUE_BUSTING_INVALID)  {
        error_extra_msg= buffer_article;
        err_invoke(QB_FORMAT_QUEUE_BUSTING_INVALID);
    }
    else if(rval==QB_ROWS_BIG_QUEUE_BUSTING_INVALID)  err_invoke(QB_ROWS_BIG_QUEUE_BUSTING_INVALID);
    else if(rval==QB_ROWS_QUEUE_BUSTING_INVALID)  err_invoke(QB_ROWS_QUEUE_BUSTING_INVALID);
    else if(rval==QB_SOCKET_CREATED_QUEUE_BUSTING_INVALID)  err_invoke(QB_SOCKET_CREATED_QUEUE_BUSTING_INVALID);
    else if(rval==QB_HOSTNAME_QUEUE_BUSTING_INVALID)  err_invoke(QB_HOSTNAME_QUEUE_BUSTING_INVALID);
    else if(rval==QB_QUEUE_BUSTING_NOTFOUND)  err_invoke(QB_QUEUE_BUSTING_NOTFOUND);
    else if(rval==QB_QUEUE_BUSTING_SQL_INVALID)  err_invoke(QB_QUEUE_BUSTING_SQL_INVALID);
    else err_invoke(INVALID_ARTNO);

    delete_active_item(empty,0);

    *data = _T('\0');
  }

   return status;
} 

/*-------------------------------------------------------------------------*/
/*                       QueueBusting_vfy_and_start_artno                  */
/*-------------------------------------------------------------------------*/
short QueueBusting_vfy_and_start_artno(_TCHAR *data, short key)
{
  short status;
  /*                                                                       */
  /* Get article and possible deposit data from PLU.                       */
  /*                                                                       */
  /* Before calling vfy_and_start_artno(), check wether data is an         */
  /* reduced-to-clear barcode. If it is, give an error and return          */
  /* UNKNOWN_KEY because no discount alouwed on reduced to clear art's.    */
  /*                                                                       */
  /* Also inititalise the discount part of this item.                      */
  /*                                                                       */

 
  status=QueueBusting_vfy_and_start_artno_detalle(data, key); 
  if (status==  ARTICLE_NOT_OK) {
      *data=_T('\0');
      //++err_invoke(INVALID_ARTNO); //acm - 11-oct-2011
      return status;
  }
  if (status==  UNKNOWN_KEY ) {
      *data=_T('\0');
      return status;
  }
  
  if (status==ARTICLE_OK) 
  {
    /*
    if (c_item_valeturkey.arti.base.art_no==0)// si no se ha leido el  pavo vale
    {
        // Leer los datos del pavo vale
        c_item.arti.display_status=status;
        memcpy_item(&c_item_valeturkey, &c_item);

        //inicializar los datos del invoice item
        delete_active_item(empty,0);
    }else
    {
        if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
          == SCRL_UNKNOWN_LINE) {
          scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
        }
        status=key;
    }
    */
    status=key;
  }
  /*
  else if (status==WWEIGHT_ART)
  {
        if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
          == SCRL_UNKNOWN_LINE) {
          scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
        }
  }
  */
  view_item(OPER_SCRN | CUST_SCRN); 
  return(status);

} 

short accept_active_item_QueueBusting(_TCHAR *data, short key) // acm ---
{
  
  int status=0;
  char queue_busting_id[100];
  strcpy(queue_busting_id ,data);

  status = QueueBusting_loop_articles(queue_busting_id,1,NULL,0);

  if (status==ARTICLE_OK)
  {
      status=ENTER_KEY;
      prescan_no	=0 ; //25-Set-2012 acm - Inicilizar el numero de prescan 
                         //                  para que no pueda ser utilizado nuevamente en la misma factura
  }
  return status;
} 


static PROCESS_ELEMENT QueueBusting_PROC[] = /* Solo acepta numero o cancelar. */
{
  ENTER_KEY,            accept_active_item_QueueBusting, /* 12-Ago-2011 acm -++ Se ejecuta al aceptar la factura*/
  NO_KEY,               QueueBusting_proc_cancel,        /* 12-Ago-2011 acm -++ Se ejecuta al aceptar la factura*/
  UNKNOWN_KEY,          (void *)NULL,
  ART_FINDER_KEY,       (void *)NULL,
};


static VERIFY_ELEMENT QueueBusting_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  TIMES_KEY,            vfy_art_qty,
  OCIA1_DATA,           QueueBusting_vfy_and_start_artno,
  OCIA2_DATA,           QueueBusting_vfy_and_start_artno,
  ENTER_KEY,            QueueBusting_vfy_and_start_artno, 
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  ART_FINDER_KEY,       vfy_key_disable,
  UNKNOWN_KEY,          illegal_fn_key,
  QUEUEBUSTING_KEY,     fn_QueueBusting

};

extern INPUT_CONTROLLER Dartno15KO14n =
{
  (INPUT_DISPLAY *)&dsp_artno15,
  KEYBOARD_MASK | OCIA1_MASK | OCIA2_MASK,
  17, 50,
  (VERIFY_KEY *)&numeric_dnull
};

extern STATE_OBJ QueueBusting_ST =
{
  ST_QUEUEBUSTING,
  QueueBusting_VW,
  art_no_DFLT,
  &Dartno15KO14n,
  //QueueBusting_VFY,
  QueueBusting_VFY,		     /* Teclas permitidas?!?           */
  Input_UVW,
  QueueBusting_PROC,
  QueueBusting_CTL
};


/* 25-Set-2012 acm -} */

/* 12-Ago-2011 acm -{ */

/*-------------------------------------------------------------------------*/
/*                       ValeTurkey_VW									                   */
/*-------------------------------------------------------------------------*/
static void ValeTurkey_VW(void)	/* Se ejecuta al presionar el boton pavos  */

{
  if (c_item_valeturkey.arti.display_status ==   ARTICLE_OK){
		scrn_clear_window(INV_ART_INPUT_WINDOW);
	  scrn_string_out(input_TXT[55], 0, 16);      //0055 _T("INGRESE ART-PAVO / CANT.")    

	}else
	{
		scrn_clear_window(INV_ART_INPUT_WINDOW);
		scrn_string_out(input_TXT[56], 0, 16);      // 0056 _T("SCANEE EL !VALE! DEL PAVO")   
	}
} 

/*-------------------------------------------------------------------------*/
/*                       ValeTurkey_proc_cancel                            */
/*-------------------------------------------------------------------------*/
static short ValeTurkey_proc_cancel(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Reset display_item so we get an empty description/cust-screen.        */
  /*                                                                       */

  display_item=TM_ROOT;

  return(key);
} 


static PROCESS_ELEMENT ValeTurkey_PROC[] = /* Solo acepta numero o cancelar. */
{
  ENTER_KEY,            accept_active_item_valeturkey, /* 12-Ago-2011 acm -++ Se ejecuta al aceptar la factura*/
  NO_KEY,               ValeTurkey_proc_cancel,        /* 12-Ago-2011 acm -++ Se ejecuta al aceptar la factura*/
  UNKNOWN_KEY,          (void *)NULL,
  ART_FINDER_KEY,       (void *)NULL,
};


static VERIFY_ELEMENT ValeTurkey_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  TIMES_KEY,            vfy_art_qty,
  OCIA1_DATA,           ValeTurkey_vfy_and_start_artno,
  OCIA2_DATA,           ValeTurkey_vfy_and_start_artno,
  ENTER_KEY,            ValeTurkey_vfy_and_start_artno, 
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  ART_FINDER_KEY,       vfy_key_disable,
  UNKNOWN_KEY,          illegal_fn_key,
  TURKEY_KEY,           fn_turkey_key

};


extern STATE_OBJ ValeTurkey_ST =
{
  ST_VALE_ARTICLE,
  ValeTurkey_VW,         /* Imprime "scanee el pavos..."   */
  art_no_DFLT,           /* Prepara recepcion de datos.    */
  &Dartno14KO14n,        /* Recepciona los datos.          */
  ValeTurkey_VFY,		     /* Teclas permitidas?!?           */
  Input_UVW,			       /* Limpia pantalla?!?             */
  ValeTurkey_PROC,		   /* Solo acepta numero o cancelar. */
  ValeTurkey_CTL		     /* Funciones de la pantalla.      */
};

/*-------------------------------------------------------------------------*/
/*                       ValeTurkey_vfy_and_start_artno                    */
/*-------------------------------------------------------------------------*/
short ValeTurkey_vfy_and_start_artno(_TCHAR *data, short key)
{
  short status;
  /*                                                                       */
  /* Get article and possible deposit data from PLU.                       */
  /*                                                                       */
  /* Before calling vfy_and_start_artno(), check wether data is an         */
  /* reduced-to-clear barcode. If it is, give an error and return          */
  /* UNKNOWN_KEY because no discount alouwed on reduced to clear art's.    */
  /*                                                                       */
  /* Also inititalise the discount part of this item.                      */
  /*                                                                       */


  status=ValeTurkey_vfy_and_start_artno_detalle(data, key); 
  if (status==  ARTICLE_NOT_OK) {
      *data=_T('\0');
      //++err_invoke(INVALID_ARTNO); //acm - 11-oct-2011
      return status;
  }
  if (status==  UNKNOWN_KEY ) {
      *data=_T('\0');
      return status;
  }
  
  if (status==ARTICLE_OK) 
  {
    if (c_item_valeturkey.arti.base.art_no==0)// si no se ha leido el  pavo vale
    {
        // Leer los datos del pavo vale
        c_item.arti.display_status=status;
        memcpy_item(&c_item_valeturkey, &c_item);

        //inicializar los datos del invoice item
        delete_active_item(empty,0);
    }else
    {
        if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
          == SCRL_UNKNOWN_LINE) {
          scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
        }
        status=key;
    }
  }else if (status==WWEIGHT_ART)
  {
        if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
          == SCRL_UNKNOWN_LINE) {
          scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
        }
  }
  view_item(OPER_SCRN | CUST_SCRN); 
  return(status);

} 


/*---------------------------------------------------------------------------*/
/*                           init_item_copy                                  */
/*---------------------------------------------------------------------------*/
void memcpy_item(TM_ITEM_GROUP *c_item, TM_ITEM_GROUP *c_item_src)
{
  /*                                                                         */
  /* Initialise item turkey.                                                 */
  /*                                                                         */
  
  memcpy(c_item, c_item_src, sizeof(TM_ITEM_GROUP));
}


/*---------------------------------------------------------------------------*/
/*                           init_item_valeturkey                            */
/*---------------------------------------------------------------------------*/
void init_item_valeturkey(void)
{
  /*                                                                         */
  /* Initialise item vale turkey.                                                 */
  /*                                                                         */
  
  memset(&c_item_valeturkey, 0,  sizeof(TM_ITEM_GROUP));
  c_item_valeturkey.voided       = FALSE;
  c_item_valeturkey.approved     = FALSE;
  c_item_valeturkey.arti.art_ind = ART_IND_NORMAL;

  c_item_valeturkey.arti.base.art_no = 0;
  c_item_valeturkey.arti.display_status = 0;
} 

/*---------------------------------------------------------------------------*/
/*                           get_valeturkey_count                            */
/*---------------------------------------------------------------------------*/
double get_valeturkey_count(void)
{
  TM_INDX indx;
  TM_ITEM_GROUP c_item_tmp;
  int  valeturkey_count=0;

  indx = tm_frst(TM_ITEM_NAME, (void*)&c_item_tmp);
  while ( indx != TM_BOF_EOF ) {
    if (!c_item_tmp.voided){ // 20-12-2011 acm -* no considerar las facturas eliminadas
      if (is_vale_article_i(c_item_tmp.arti.base.art_no)){
        valeturkey_count++;
      }
    }
    indx = tm_next(TM_ITEM_NAME, (void*)&c_item_tmp);
  }
  return valeturkey_count;
} 

/*---------------------------------------------------------------------------*/
/*                           get_valeturkey_amount                           */
/*---------------------------------------------------------------------------*/
double get_valeturkey_amount(void)
{
  TM_INDX indx;
  TM_ITEM_GROUP c_item_tmp;
  double goods_value_amount=0;

  indx = tm_frst(TM_ITEM_NAME, (void*)&c_item_tmp);
  while ( indx != TM_BOF_EOF ) {
    if (!c_item_tmp.voided){ // 20-12-2011 acm -* no considerar las facturas eliminadas
      if (is_vale_article_i(c_item_tmp.arti.base.art_no)){
        goods_value_amount+=c_item_tmp.arti.base.goods_value;
      }
    }
    indx = tm_next(TM_ITEM_NAME, (void*)&c_item_tmp);
  }
  return goods_value_amount;
} 

/*---------------------------------------------------------------------------*/
/*                           get_base_qty                                    */
/*---------------------------------------------------------------------------*/
double get_base_qty( TM_ITEM_GROUP * c_item, int * p_nerror)
{
  double data_value_fix=0;  
  *p_nerror =0;/* init the code error to zero*/

  /* Inicializar la cantidad al valor incial*/
  data_value_fix = c_item->arti.base.qty;

  /* Si el vale pavo esta inicializado descontar el peso del vale pavo al articulo pavo */
  if (c_item_valeturkey.arti.base.art_no!=0)
  {
    /*  Si es barcode 13 descontar el peso del vale en el articulo pavo           */
    /*  tomar como "pavo kilo" el valor de "pavo kilo digitado" -"peso vale pavo" */
    data_value_fix = c_item->arti.base.qty - c_item_valeturkey.arti.base.qty;

    /* Si la diferencia del"pavo kilo"("kilo digitado" -"peso vale pavo") es menor que cero   */
    /* se debe tomar como pavo kilo el valor de cero.                                         */
    if (data_value_fix== 0)    {
        data_value_fix=0.0001;
    }else if (data_value_fix< 0)    { /*  Si la diferencia es negativa debe retornar error*/
        data_value_fix=0.0001;
        *p_nerror =WEIGHT_TURKEY_MUSTBE7;
    }  
  }
  return data_value_fix;
}

/*---------------------------------------------------------------------------*/
/*                 decode_internal_ean13_barcode_tukeryV2                    */
/*---------------------------------------------------------------------------*/
static short decode_internal_ean13_barcode_turkeyV2(void)
{
  
  _TCHAR ticket_pwd[19];                        /* price/weight/discnt      */
  _TCHAR buffer[19];
  short status;
  int   lnum_error = 0 ;

  /*                                                                       */
  /* Decode internal EAN13 barcode:                                        */
  /*                                                                       */
  /*  Scale price art:  price is in EAN13.                                 */
  /*                    If article_type == PRICE:                          */
  /*                          price in barcode is price_per_item (ppi)     */
  /*                          gds_value = ppi * qty                        */
  /*                    If article_type == WEIGHT:                         */
  /*                          price in barcode is goodsvalue               */
  /*                          gds_value = price in barcode                 */
  /*                          qty = gds-value / ppi                        */
  /*                                                                       */
  /*  Scale weight art: weight is in EAN13.                                */
  /*                          qty = weight from EAN13                      */
  /*                          gds_value = ppi * weight from EAN13          */
  /*                                                                       */
  /*  Reduced to clear: get percentage, calculate discount amount, create  */
  /*                    discount part of the current item and insert       */
  /*                    discount values.                                   */
  /*                                                                       */

  status=SUCCEED;

  if (get_barcode_type(c_item.arti.base.bar_cd)==BARCD_SCALE_EAN13) {
    _tcscpy(buffer,(c_item.arti.base.bar_cd+7));      /* Scale             */
    buffer[5]=(_TCHAR)0;                              /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));     /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      if (get_scale_barcode_type(c_item.arti.base.bar_cd)==BARCD_SCALE_WEIGHT) {
        /*                                                                 */
        /* Weight is in the barcode.                                       */
        /*     qty = weight from EAN13                                     */
        /*     gds_value = ppi * weight from EAN13                         */
        /*                                                                 */

        c_item.arti.base.qty = _tcstod(ticket_pwd, NULL)/1000.0;
        /* 12-Ago-2011 acm -+++ { fix turkey vale qty */
        c_item.arti.base.qty = get_base_qty( &c_item,&lnum_error); 

        if (lnum_error){
            err_invoke(WEIGHT_TURKEY_MUSTBE7);             
            status=WEIGHT_TURKEY_MUSTBE7;
        } else  {
        /* 12-Ago-2011 acm - } */
          if (invoice_line_mode==RETURN) {
            c_item.arti.base.qty*=-1;
          }
          status=calc_gds_value(ART_GROUP);  /* SUCCEED/FAIL/PRICE_TOO_LARGE */
        }   /* 12-Ago-2011 acm - add  */
      }
      else {                                      /* BARCD_SCALE_PRICE     */
        /*                                                                 */
        /* Price is in the barcode, determine the article-type:            */
        /*                                                                 */
        /*            If article_type == PRICE:                            */
        /*                  price in barcode is price_per_item (ppi)       */
        /*                  gds_value = ppi * qty                          */
        /*            If article_type == WEIGHT:                           */
        /*                  price in barcode is goodsvalue                 */
        /*                  gds_value = price in barcode                   */
        /*                  qty = gds-value / ppi                          */
        /*                                                                 */
        if (c_item.arti.art_ind==ART_IND_PRICE) {         /* ART_IND_PRICE */
           c_item.arti.base.price
                        = floor_price(atof_price(ticket_pwd));
           status=calc_gds_value(ART_GROUP); /* SUCCEED/FAIL/PRICE_TOO_LAR */
        }
        else {                                           /* ART_IND_WEIGHT */
          c_item.arti.base.goods_value
                        = floor_price(atof_price(ticket_pwd));
          if (invoice_line_mode==RETURN) {
            c_item.arti.base.goods_value*=-1;
          }

          c_item.arti.base.qty=c_item.arti.base.goods_value/c_item.arti.base.price;
          if( (double)fabs(c_item.arti.base.qty) > 9999.999 ) {
            err_invoke(PRICE_TOO_LARGE);              /* Quantity overflow.*/
            status=PRICE_TOO_LARGE;
          }
        }
      }
    }
    else {
      status=FAIL;                                    /* Zero not legal.   */
    }
  }
  else if (get_barcode_type(c_item.arti.base.bar_cd)==BARCD_EXTERNAL_WEIGHT_EAN13) {
    _tcscpy(buffer,(c_item.arti.base.bar_cd+7));      /* Scale             */
    buffer[5]=(_TCHAR)0;                              /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));     /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      /*                                                                   */
      /* Weight is in the barcode.                                         */
      /*     qty = weight from EAN13                                       */
      /*     gds_value = ppi * weight from EAN13                           */
      /*                                                                   */
      c_item.arti.base.qty = strtod(ticket_pwd, NULL)/1000.0;
      
      /* 12-Ago-2011 acm - { fix turkey vale qty */
      c_item.arti.base.qty = get_base_qty( &c_item, &lnum_error); 
      if (lnum_error){
            err_invoke(WEIGHT_TURKEY_MUSTBE7);             
            status=WEIGHT_TURKEY_MUSTBE7;
      } else  {
          if (invoice_line_mode == RETURN) {
            c_item.arti.base.qty*=-1;
          }
          status=calc_gds_value(ART_GROUP);     /* SUCCEED/FAIL/PRICE_TOO_LARGE */
      } /* 12-Ago-2011 acm - add */
    }
  }
  else {                                       /* Reduced to clear barcode. */
    _tcscpy(buffer,(c_item.arti.base.bar_cd+8));       /* Article no gone   */
    buffer[4]=(_TCHAR)0;                               /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));      /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      if (c_item.arti.art_ind==ART_IND_WEIGHT) {/* Qty for discount allways */
        c_item.disc.base.qty=1.0;               /* positive!                */
      }
      else {
        c_item.disc.base.qty=fabs(c_item.arti.base.qty);
      }
			
      _tcsncpy(c_item.disc.base.descr, scrn_inv_TXT[25], 33);
      c_item.disc.base.vat_no         = c_item.arti.base.vat_no;
      c_item.disc.base.vat_perc       = c_item.arti.base.vat_perc;
      c_item.disc.base.art_grp_no     = c_item.arti.base.art_grp_no;
      c_item.disc.base.art_grp_sub_no = c_item.arti.base.art_grp_sub_no;
      c_item.disc.base.dept_cd        = c_item.arti.base.dept_cd;
      c_item.disc.base.price = floor_price((c_item.arti.base.price/100) *
                                             (_tcstod(ticket_pwd, NULL)/100))*100;
      if (invoice_line_mode==SALES) {
        c_item.disc.base.price*=-1;
      }
      status=calc_gds_value(ART_GROUP);
      if (status==SUCCEED) {
        status=calc_gds_value(DISCNT_GROUP); /* SUCCEED/FAIL/PRICE_TOO_LARGE */
      }
    }
    else {
      status=FAIL;                                /* Zero not legal.       */
    }
  }

  return(status);
} /* decode_internal_ean13_barcode_turkeyV2 */

/*---------------------------------------------------------------------------*/
/*                           vfy_empty_field_ValeTurkey                      */
/*---------------------------------------------------------------------------*/
short vfy_empty_field_ValeTurkey(_TCHAR *data, short key)
{
  valeturkey_press_key = 1; /* 12-Ago-2011 acm - flag vale turkey key is pressed*/
  return vfy_empty_field(data, key);
} /* vfy_empty_field_ValeTurkey*/

short fn_turkey_key(_TCHAR *data, short key)
{
  return key;
}

/* 12-Ago-2011 acm -}*/


/* v3.4.8 acm -{*/
/*---------------------------------------------------------------------------*/
/*                           vfy_empty_field_Horeca                         */
/*---------------------------------------------------------------------------*/

short vfy_empty_field_Horeca(_TCHAR *data, short key)
{
    if (!isvigente_horeca())
    {
      *data=_T('\0');
      err_invoke(VIGENCIA_NOT_HORECA);
      return(UNKNOWN_KEY);
    }
     return vfy_empty_field(data, key);
    if (is_cust_prom_horeca(cust.cust_no,cust.store_no))
    {
      horeca_press_key = 1; /* 12-Ago-2011 acm - flag vale turkey key is pressed*/
      return vfy_empty_field(data, key);
    
    } else{
      *data=_T('\0');
      err_invoke(FIELD_NOT_HORECA);
      return(UNKNOWN_KEY);
    }

} /* vfy_empty_field_ValeTurkey*/

/* v3.4.8 acm -}*/

/*---------------------------------------------------------------------------*/
/*                           init_item                                       */
/*---------------------------------------------------------------------------*/
void init_item(void)
{
  /*                                                                         */
  /* Initialise item.                                                        */
  /*                                                                         */
  memset(&c_item, 0, sizeof(TM_ITEM_GROUP));
  c_item.voided       = FALSE;
  c_item.approved     = FALSE;
  c_item.arti.art_ind = ART_IND_NORMAL;
} /* init_item */

/*---------------------------------------------------------------------------*/
/*                          recalc_totals                                    */
/*---------------------------------------------------------------------------*/
short recalc_totals(SUB_ITEM_TOTAL *sit)
{
  short i;

  /*                                                                         */
  /* Recalculate all totals after a sub-item has been accepted.              */
  /*                                                                         */

  if (genvar.price_incl_vat==INCLUSIVE) {
    /* Total amount inclusive vat per vat-code.                              */
    tot_add_double((short)(TOT_INCL_0     + sit->vat_no), sit->incl_gds_val);
    tot_add_double((short)(SUB_TOT_INCL_0 + sit->vat_no), sit->incl_gds_val);
    tot_add_double((short)(TOT_GEN_INCL),                 sit->incl_gds_val);
    tot_add_double((short)(SUB_TOT_GEN_INCL),             sit->incl_gds_val);

    /* Total amount exclusive vat per vat-code.                              */

    tot_reset_double((short)(TOT_EXCL_0 + sit->vat_no));
    tot_add_double((short)(TOT_EXCL_0 + sit->vat_no), floor_price(calc_excl_vat(
                        tot_ret_double((short)(TOT_INCL_0 + sit->vat_no)),sit->vat_no)));

    tot_reset_double((short)(SUB_TOT_EXCL_0 + sit->vat_no));
    tot_add_double((short)(SUB_TOT_EXCL_0 + sit->vat_no),floor_price(calc_excl_vat(
                    tot_ret_double((short)(SUB_TOT_INCL_0 + sit->vat_no)),sit->vat_no)));

    tot_reset_double(TOT_GEN_EXCL);
    for ( i = 0 ; i < 10 ; i++ ) {
      tot_add_double(TOT_GEN_EXCL,tot_ret_double((short)(TOT_EXCL_0 + i)));
    }
    /* Adding customer-fee */
//    tot_add_double(TOT_GEN_EXCL, tot_ret_double(TOT_FEE_AMOUNT));
  }
  else {                                                        /* EXCLUSIVE */
    /* Total amount exclusive vat per vat-code.                              */
    tot_add_double((short)(TOT_EXCL_0     + sit->vat_no), sit->excl_gds_val);
    tot_add_double((short)(SUB_TOT_EXCL_0 + sit->vat_no), sit->excl_gds_val);
    tot_add_double(TOT_GEN_EXCL,                          sit->excl_gds_val);

    /* Total amount inclusive vat per vat-code.                              */

    tot_reset_double((short)(TOT_INCL_0 + sit->vat_no));
    tot_add_double((short)(TOT_INCL_0 + sit->vat_no), floor_price(calc_incl_vat(
                        tot_ret_double((short)(TOT_EXCL_0 + sit->vat_no)),sit->vat_no)));

    tot_reset_double((short)(SUB_TOT_INCL_0 + sit->vat_no));
    tot_add_double((short)(SUB_TOT_INCL_0 + sit->vat_no), floor_price( calc_incl_vat(
                    tot_ret_double((short)(SUB_TOT_EXCL_0 + sit->vat_no)),sit->vat_no)));

    tot_reset_double(SUB_TOT_GEN_INCL);
    for ( i = 0 ; i < 10 ; i++ ) {
      tot_add_double(SUB_TOT_GEN_INCL,tot_ret_double((short)(SUB_TOT_INCL_0 + i)));
    }

    tot_reset_double(TOT_GEN_INCL);
    for ( i = 0 ; i < 10 ; i++ ) {
      tot_add_double(TOT_GEN_INCL,tot_ret_double((short)(TOT_INCL_0 + i)));
    }
    /* Adding customer-fee */
//    tot_add_double(TOT_GEN_INCL, tot_ret_double(TOT_FEE_AMOUNT));
  }

  /* Total vat amount.                                                     */
  tot_reset_double((short)(SUB_TOT_VAT_0 + sit->vat_no));
  tot_add_double((short)(SUB_TOT_VAT_0 + sit->vat_no),
                 tot_ret_double((short)(SUB_TOT_INCL_0 + sit->vat_no)) -
                 tot_ret_double((short)(SUB_TOT_EXCL_0 + sit->vat_no)));
  tot_reset_double((short)(TOT_VAT_0 + sit->vat_no));
  tot_add_double((short)(TOT_VAT_0 + sit->vat_no),
                   tot_ret_double((short)(TOT_INCL_0+sit->vat_no))
                    - tot_ret_double((short)(TOT_EXCL_0 + sit->vat_no)));
  tot_reset_double(TOT_GEN_VAT);
  for ( i = 0 ; i < 10 ; i++ ) {
    tot_add_double(TOT_GEN_VAT,tot_ret_double((short)(TOT_VAT_0 + i)));
  }


  if ( (sit->packs > 0 && !sit->voided) ||    /* count: credits no,        */
       (sit->packs < 0 /* &&  sit->voided */)) {    /* voided yes                */
    tot_add_double(TOT_PACKS,     (double) sit->packs);
    tot_add_double(TOT_SUB_PACKS, (double) sit->packs);
  }

  return(SUCCEED);
} /* recalc_totals */

/*---------------------------------------------------------------------------*/
/*                          calc_sub_item_totals                             */
/*---------------------------------------------------------------------------*/
short calc_sub_item_totals(TM_INDX item_indx, ITEM_SNAM name,
                           SUB_ITEM_TOTAL *sub_item_totals)
{
  TM_ARTI_BASE *base;
  TM_ITEM_GROUP h_item;
  short art_ind;
  double qty;

  /*                                                                       */
  /* Retreive goods-value (in- exclusive vat), vat-code and the number     */
  /* off packs to be used in recalc_totals().                              */
  /*                                                                       */

  memset(sub_item_totals, 0, sizeof(SUB_ITEM_TOTAL));

  if (item_indx==C_ITEM)
    memcpy(&h_item, &c_item, sizeof(TM_ITEM_GROUP));
  else
    tm_read_nth(TM_ITEM_NAME, (void*)&h_item, item_indx);

  switch (name) {
    case ART_GROUP:
      art_ind=h_item.arti.art_ind;
      base=&h_item.arti.base;
      break;
    case DISCNT_GROUP:
      art_ind=ART_IND_NORMAL;
      base=&h_item.disc.base;
      break;
    case DEPOSIT_GROUP:
      art_ind=ART_IND_NORMAL;
      base=&h_item.depo.base;
      break;
    default:
      break;
  }

  if (fabs(base->goods_value) + fabs(base->qty) + fabs(base->price) != 0.0) {
    sub_item_totals->voided  =h_item.voided;
    sub_item_totals->vat_no  =base->vat_no;

    qty=base->qty;

    /*                                                                     */
    /* Determine number of packs:                                          */
    /*                                                                     */
    /*   art_ind    group-name     packs                                   */
    /*   --------   ----------     ------------------                      */
    /*     x        DEPOSIT_GROUP  0                                       */
    /*     x        DISCNT_GROUP   0                                       */
    /*   NORMAL         x          qty                                     */
    /*   PRICE          x          qty                                     */
    /*   WEIGHT         x          qty<0 ? -1 : 1                          */
    /*   DEPOSIT    ART_GROUP      0                                       */
    /*                                                                     */
    if ( name==DEPOSIT_GROUP || name==DISCNT_GROUP ||
      (name==ART_GROUP && art_ind==ART_IND_DEPOSIT)) {
      sub_item_totals->packs=0;
    }
    else if (art_ind==ART_IND_NORMAL || art_ind==ART_IND_PRICE) {
      sub_item_totals->packs=(short)qty;
    }
    else if (art_ind==ART_IND_WEIGHT) {
      sub_item_totals->packs=(qty<0.0)?-1:1;
    }

    /*                                                                     */
    /* Calculate goods-value in- and exclusive vat.                        */
    /*                                                                     */
    if (genvar.price_incl_vat==INCLUSIVE) {
      sub_item_totals->incl_gds_val=floor_price(base->goods_value);
      sub_item_totals->excl_gds_val=
                floor_price(calc_excl_vat(base->goods_value, base->vat_no));
    }
    else {                              /* goods-value is exclusive vat. */
      sub_item_totals->excl_gds_val= floor_price(base->goods_value);
      sub_item_totals->incl_gds_val=
                  floor_price(calc_incl_vat(base->goods_value, base->vat_no));
    }
    return(SUCCEED);
  }

  return(FAIL);
} /* calc_sub_item_totals */


/*---------------------------------------------------------------------------*/
/*                              calc_gds_value                               */
/*---------------------------------------------------------------------------*/
short calc_gds_value(ITEM_SNAM name)
{
  TM_ARTI_BASE *base;
  double goods_value;
  short art_ind;

  /*                                                                       */
  /* Calculate goods_value = price * qty                                   */
  /*  Also check overflow-situations.                                      */
  /*  Return PRICE_TOO_LARGE in case of an overflow.                       */
  /*                                                                       */

  switch (name) {
    case ART_GROUP:
      art_ind=c_item.arti.art_ind;
      base=&c_item.arti.base;
      break;
    case DISCNT_GROUP:
      art_ind=ART_IND_NORMAL;
      base=&c_item.disc.base;
      break;
    case DEPOSIT_GROUP:
      art_ind=ART_IND_NORMAL;
      base=&c_item.depo.base;
      break;
    default:
      break;
  }

  if (base->qty == 0.0) {
    if (invoice_line_mode == SALES) {
      base->qty=1;
    }
    else {
      base->qty=-1;
    }
  }

  if (art_ind == ART_IND_DEPOSIT && name == ART_GROUP) {
    if (invoice_line_mode == SALES && base->qty > 0.0) {
      base->qty*=-1;                             /* Qty must be negative. */
    }
    if (invoice_line_mode == RETURN && base->qty < 0.0) {
      base->qty*=-1;                             /* Qty must be positive. */
    }
  }
  base->goods_value = floor_price(base->price * base->qty);

  if (genvar.ind_price_dec == DECIMALS_YES) {
    goods_value=base->goods_value*100;
  }
  else {
    goods_value=base->goods_value;
  }

  if (goods_value<=999999999.0) {
    return(SUCCEED);
  }
  else {
    err_invoke(PRICE_TOO_LARGE);
    base->goods_value=0;
    return(PRICE_TOO_LARGE);
  }
} /* calc_gds_value */

/*---------------------------------------------------------------------------*/
/*                 decode_internal_ean13_barcode                             */
/*---------------------------------------------------------------------------*/
static short decode_internal_ean13_barcode_turkey(void)
{
  _TCHAR ticket_pwd[19];                        /* price/weight/discnt      */
  _TCHAR buffer[19];
  short status;

  /*                                                                       */
  /* Decode internal EAN13 barcode:                                        */
  /*                                                                       */
  /*  Scale price art:  price is in EAN13.                                 */
  /*                    If article_type == PRICE:                          */
  /*                          price in barcode is price_per_item (ppi)     */
  /*                          gds_value = ppi * qty                        */
  /*                    If article_type == WEIGHT:                         */
  /*                          price in barcode is goodsvalue               */
  /*                          gds_value = price in barcode                 */
  /*                          qty = gds-value / ppi                        */
  /*                                                                       */
  /*  Scale weight art: weight is in EAN13.                                */
  /*                          qty = weight from EAN13                      */
  /*                          gds_value = ppi * weight from EAN13          */
  /*                                                                       */
  /*  Reduced to clear: get percentage, calculate discount amount, create  */
  /*                    discount part of the current item and insert       */
  /*                    discount values.                                   */
  /*                                                                       */

  status=SUCCEED;

  if (get_barcode_type(c_item.arti.base.bar_cd)==BARCD_SCALE_EAN13) {
    _tcscpy(buffer,(c_item.arti.base.bar_cd+7));      /* Scale             */
    buffer[5]=(_TCHAR)0;                              /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));     /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      if (get_scale_barcode_type(c_item.arti.base.bar_cd)==BARCD_SCALE_WEIGHT) {
        /*                                                                 */
        /* Weight is in the barcode.                                       */
        /*     qty = weight from EAN13                                     */
        /*     gds_value = ppi * weight from EAN13                         */
        /*                                                                 */

        c_item.arti.base.qty=_tcstod(ticket_pwd, NULL)/1000.0;
        if (invoice_line_mode==RETURN) {
          c_item.arti.base.qty*=-1;
        }
        status=calc_gds_value(ART_GROUP);  /* SUCCEED/FAIL/PRICE_TOO_LARGE */
      }
      else {                                      /* BARCD_SCALE_PRICE     */
        /*                                                                 */
        /* Price is in the barcode, determine the article-type:            */
        /*                                                                 */
        /*            If article_type == PRICE:                            */
        /*                  price in barcode is price_per_item (ppi)       */
        /*                  gds_value = ppi * qty                          */
        /*            If article_type == WEIGHT:                           */
        /*                  price in barcode is goodsvalue                 */
        /*                  gds_value = price in barcode                   */
        /*                  qty = gds-value / ppi                          */
        /*                                                                 */
        if (c_item.arti.art_ind==ART_IND_PRICE) {         /* ART_IND_PRICE */
           c_item.arti.base.price
                        = floor_price(atof_price(ticket_pwd));
           status=calc_gds_value(ART_GROUP); /* SUCCEED/FAIL/PRICE_TOO_LAR */
        }
        else {                                           /* ART_IND_WEIGHT */
          c_item.arti.base.goods_value
                        = floor_price(atof_price(ticket_pwd));
          if (invoice_line_mode==RETURN) {
            c_item.arti.base.goods_value*=-1;
          }

          c_item.arti.base.qty=c_item.arti.base.goods_value/c_item.arti.base.price;
          if( (double)fabs(c_item.arti.base.qty) > 9999.999 ) {
            err_invoke(PRICE_TOO_LARGE);              /* Quantity overflow.*/
            status=PRICE_TOO_LARGE;
          }
        }
      }
    }
    else {
      status=FAIL;                                    /* Zero not legal.   */
    }
  }
  else if (get_barcode_type(c_item.arti.base.bar_cd)==BARCD_EXTERNAL_WEIGHT_EAN13) {
    _tcscpy(buffer,(c_item.arti.base.bar_cd+7));      /* Scale             */
    buffer[5]=(_TCHAR)0;                              /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));     /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      /*                                                                   */
      /* Weight is in the barcode.                                         */
      /*     qty = weight from EAN13                                       */
      /*     gds_value = ppi * weight from EAN13                           */
      /*                                                                   */
      c_item.arti.base.qty = strtod(ticket_pwd, NULL)/1000.0;
      if (invoice_line_mode == RETURN) {
        c_item.arti.base.qty*=-1;
      }
      status=calc_gds_value(ART_GROUP);     /* SUCCEED/FAIL/PRICE_TOO_LARGE */
    }
  }
  else {                                       /* Reduced to clear barcode. */
    _tcscpy(buffer,(c_item.arti.base.bar_cd+8));       /* Article no gone   */
    buffer[4]=(_TCHAR)0;                               /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));      /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      if (c_item.arti.art_ind==ART_IND_WEIGHT) {/* Qty for discount allways */
        c_item.disc.base.qty=1.0;               /* positive!                */
      }
      else {
        c_item.disc.base.qty=fabs(c_item.arti.base.qty);
      }
      _tcsncpy(c_item.disc.base.descr, scrn_inv_TXT[38], 33);
      c_item.disc.base.vat_no         = c_item.arti.base.vat_no;
      c_item.disc.base.vat_perc       = c_item.arti.base.vat_perc;
      c_item.disc.base.art_grp_no     = c_item.arti.base.art_grp_no;
      c_item.disc.base.art_grp_sub_no = c_item.arti.base.art_grp_sub_no;
      c_item.disc.base.dept_cd        = c_item.arti.base.dept_cd;
      c_item.disc.base.price = floor_price((c_item.arti.base.price/100) *
                                             (_tcstod(ticket_pwd, NULL)/100))*100;
      if (invoice_line_mode==SALES) {
        c_item.disc.base.price*=-1;
      }
      status=calc_gds_value(ART_GROUP);
      if (status==SUCCEED) {
        status=calc_gds_value(DISCNT_GROUP); /* SUCCEED/FAIL/PRICE_TOO_LARGE */
      }
    }
    else {
      status=FAIL;                                /* Zero not legal.       */
    }
  }

  return(status);
} /* decode_internal_ean13_barcode_turkey */




/*---------------------------------------------------------------------------*/
/*                 decode_internal_ean13_barcode_V2                          */
/*---------------------------------------------------------------------------*/
short decode_internal_ean13_barcode_V2(TM_ITEM_GROUP * p_item)
{
  _TCHAR ticket_pwd[19];                        /* price/weight/discnt      */
  _TCHAR buffer[19];
  short status;

  /*                                                                       */
  /* Decode internal EAN13 barcode:                                        */
  /*                                                                       */
  /*  Scale price art:  price is in EAN13.                                 */
  /*                    If article_type == PRICE:                          */
  /*                          price in barcode is price_per_item (ppi)     */
  /*                          gds_value = ppi * qty                        */
  /*                    If article_type == WEIGHT:                         */
  /*                          price in barcode is goodsvalue               */
  /*                          gds_value = price in barcode                 */
  /*                          qty = gds-value / ppi                        */
  /*                                                                       */
  /*  Scale weight art: weight is in EAN13.                                */
  /*                          qty = weight from EAN13                      */
  /*                          gds_value = ppi * weight from EAN13          */
  /*                                                                       */
  /*  Reduced to clear: get percentage, calculate discount amount, create  */
  /*                    discount part of the current item and insert       */
  /*                    discount values.                                   */
  /*                                                                       */

  status=SUCCEED;

  if (get_barcode_type(p_item->arti.base.bar_cd)==BARCD_SCALE_EAN13) {
    _tcscpy(buffer,(p_item->arti.base.bar_cd+7));      /* Scale             */
    buffer[5]=(_TCHAR)0;                              /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));     /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      if (get_scale_barcode_type(p_item->arti.base.bar_cd)==BARCD_SCALE_WEIGHT) {
        /*                                                                 */
        /* Weight is in the barcode.                                       */
        /*     qty = weight from EAN13                                     */
        /*     gds_value = ppi * weight from EAN13                         */
        /*                                                                 */

        p_item->arti.base.qty=_tcstod(ticket_pwd, NULL)/1000.0;
        if (invoice_line_mode==RETURN) {
          p_item->arti.base.qty*=-1;
        }
        status=calc_gds_value(ART_GROUP);  /* SUCCEED/FAIL/PRICE_TOO_LARGE */
      }
      else {                                      /* BARCD_SCALE_PRICE     */
        /*                                                                 */
        /* Price is in the barcode, determine the article-type:            */
        /*                                                                 */
        /*            If article_type == PRICE:                            */
        /*                  price in barcode is price_per_item (ppi)       */
        /*                  gds_value = ppi * qty                          */
        /*            If article_type == WEIGHT:                           */
        /*                  price in barcode is goodsvalue                 */
        /*                  gds_value = price in barcode                   */
        /*                  qty = gds-value / ppi                          */
        /*                                                                 */
        if (p_item->arti.art_ind==ART_IND_PRICE) {         /* ART_IND_PRICE */
           p_item->arti.base.price
                        = floor_price(atof_price(ticket_pwd));
           status=calc_gds_value(ART_GROUP); /* SUCCEED/FAIL/PRICE_TOO_LAR */
        }
        else            
        {                                           /* ART_IND_WEIGHT */
          /*26-Jul-2012 3.4.4 acm - { Ejecutar solo si es un articulo distinto de Normal*/
          if (p_item->arti.art_ind==ART_IND_NORMAL) {
              err_invoke(ILLEGAL_BARCODE);
              status=FAIL;     
          }else {
          /*26-Jul-2012 3.4.4 acm - } */ 
              p_item->arti.base.goods_value
                            = floor_price(atof_price(ticket_pwd));
              if (invoice_line_mode==RETURN) {
                p_item->arti.base.goods_value*=-1;
              }

              p_item->arti.base.qty=p_item->arti.base.goods_value/p_item->arti.base.price;
              if( (double)fabs(p_item->arti.base.qty) > 9999.999 ) {
                err_invoke(PRICE_TOO_LARGE);              /* Quantity overflow.*/
                status=PRICE_TOO_LARGE;
              }
          } /* 26-Jul-2012 3.4.4 acm -  */
        }
      }
    }
    else { 
      /* Zero not legal.   */
      status=FAIL;
      /*  
      if (get_scale_barcode_type(p_item->arti.base.bar_cd)==BARCD_SCALE_WEIGHT) {// patch for QueueBusting
        // Weight is in the barcode.                                       
        //     qty = weight from EAN13                                     
        //     gds_value = ppi * weight from EAN13                         
        //                                                                 

        p_item->arti.base.qty=0;
        if (invoice_line_mode==RETURN) {
          p_item->arti.base.qty*=-1;
        }
        status=calc_gds_value(ART_GROUP);  // SUCCEED/FAIL/PRICE_TOO_LARGE 
      }else
        status=FAIL;
      */ 
    }
  }
  else if (get_barcode_type(p_item->arti.base.bar_cd)==BARCD_EXTERNAL_WEIGHT_EAN13) {
    _tcscpy(buffer,(p_item->arti.base.bar_cd+7));      /* Scale             */
    buffer[5]=(_TCHAR)0;                               /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));     /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      /*                                                                   */
      /* Weight is in the barcode.                                         */
      /*     qty = weight from EAN13                                       */
      /*     gds_value = ppi * weight from EAN13                           */
      /*                                                                   */
      p_item->arti.base.qty = strtod(ticket_pwd, NULL)/1000.0;
      if (invoice_line_mode == RETURN) {
        p_item->arti.base.qty*=-1;
      }
      status=calc_gds_value(ART_GROUP);     /* SUCCEED/FAIL/PRICE_TOO_LARGE */
    }
  }
  else {                                       /* Reduced to clear barcode. */
    _tcscpy(buffer,(p_item->arti.base.bar_cd+8));       /* Article no gone   */
    buffer[4]=(_TCHAR)0;                               /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));      /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      if (p_item->arti.art_ind==ART_IND_WEIGHT) {/* Qty for discount allways */
        p_item->disc.base.qty=1.0;               /* positive!                */
      }
      else {
        p_item->disc.base.qty=fabs(p_item->arti.base.qty);
      }

      _tcsncpy(p_item->disc.base.descr, scrn_inv_TXT[25], 33);
      p_item->disc.base.vat_no         = p_item->arti.base.vat_no;
      p_item->disc.base.vat_perc       = p_item->arti.base.vat_perc;
      p_item->disc.base.art_grp_no     = p_item->arti.base.art_grp_no;
      p_item->disc.base.art_grp_sub_no = p_item->arti.base.art_grp_sub_no;
      p_item->disc.base.dept_cd        = p_item->arti.base.dept_cd;
      p_item->disc.base.price = floor_price((p_item->arti.base.price/100) *
                                             (_tcstod(ticket_pwd, NULL)/100))*100;
      if (invoice_line_mode==SALES) {
        p_item->disc.base.price*=-1;
      }
      status=calc_gds_value(ART_GROUP);
      if (status==SUCCEED) {
        status=calc_gds_value(DISCNT_GROUP); /* SUCCEED/FAIL/PRICE_TOO_LARGE */
      }
    }
    else {
      status=FAIL;                                /* Zero not legal.       */
    }
  }

  return(status);
} /* decode_internal_ean13_barcode_V2*/


/*---------------------------------------------------------------------------*/
/*                 decode_internal_ean13_barcode                             */
/*---------------------------------------------------------------------------*/
static short decode_internal_ean13_barcode(void)
{
  _TCHAR ticket_pwd[19];                        /* price/weight/discnt      */
  _TCHAR buffer[19];
  short status;

  /*                                                                       */
  /* Decode internal EAN13 barcode:                                        */
  /*                                                                       */
  /*  Scale price art:  price is in EAN13.                                 */
  /*                    If article_type == PRICE:                          */
  /*                          price in barcode is price_per_item (ppi)     */
  /*                          gds_value = ppi * qty                        */
  /*                    If article_type == WEIGHT:                         */
  /*                          price in barcode is goodsvalue               */
  /*                          gds_value = price in barcode                 */
  /*                          qty = gds-value / ppi                        */
  /*                                                                       */
  /*  Scale weight art: weight is in EAN13.                                */
  /*                          qty = weight from EAN13                      */
  /*                          gds_value = ppi * weight from EAN13          */
  /*                                                                       */
  /*  Reduced to clear: get percentage, calculate discount amount, create  */
  /*                    discount part of the current item and insert       */
  /*                    discount values.                                   */
  /*                                                                       */

  status=SUCCEED;

  if (get_barcode_type(c_item.arti.base.bar_cd)==BARCD_SCALE_EAN13) {
    _tcscpy(buffer,(c_item.arti.base.bar_cd+7));      /* Scale             */
    buffer[5]=(_TCHAR)0;                              /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));     /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      if (get_scale_barcode_type(c_item.arti.base.bar_cd)==BARCD_SCALE_WEIGHT) {
        /*                                                                 */
        /* Weight is in the barcode.                                       */
        /*     qty = weight from EAN13                                     */
        /*     gds_value = ppi * weight from EAN13                         */
        /*                                                                 */

        c_item.arti.base.qty=_tcstod(ticket_pwd, NULL)/1000.0;
        if (invoice_line_mode==RETURN) {
          c_item.arti.base.qty*=-1;
        }
        status=calc_gds_value(ART_GROUP);  /* SUCCEED/FAIL/PRICE_TOO_LARGE */
      }
      else {                                      /* BARCD_SCALE_PRICE     */
        /*                                                                 */
        /* Price is in the barcode, determine the article-type:            */
        /*                                                                 */
        /*            If article_type == PRICE:                            */
        /*                  price in barcode is price_per_item (ppi)       */
        /*                  gds_value = ppi * qty                          */
        /*            If article_type == WEIGHT:                           */
        /*                  price in barcode is goodsvalue                 */
        /*                  gds_value = price in barcode                   */
        /*                  qty = gds-value / ppi                          */
        /*                                                                 */
        if (c_item.arti.art_ind==ART_IND_PRICE) {         /* ART_IND_PRICE */
           c_item.arti.base.price
                        = floor_price(atof_price(ticket_pwd));
           status=calc_gds_value(ART_GROUP); /* SUCCEED/FAIL/PRICE_TOO_LAR */
        }
        else            
        {                                           /* ART_IND_WEIGHT */
          /*26-Jul-2012 3.4.4 acm - { Ejecutar solo si es un articulo distinto de Normal*/
          if (c_item.arti.art_ind==ART_IND_NORMAL) {
              err_invoke(ILLEGAL_BARCODE);
              status=FAIL;     
          }else {
          /*26-Jul-2012 3.4.4 acm - } */ 
              c_item.arti.base.goods_value
                            = floor_price(atof_price(ticket_pwd));
              if (invoice_line_mode==RETURN) {
                c_item.arti.base.goods_value*=-1;
              }

              c_item.arti.base.qty=c_item.arti.base.goods_value/c_item.arti.base.price;
              if( (double)fabs(c_item.arti.base.qty) > 9999.999 ) {
                err_invoke(PRICE_TOO_LARGE);              /* Quantity overflow.*/
                status=PRICE_TOO_LARGE;
              }
          } /* 26-Jul-2012 3.4.4 acm -  */
        }
      }
    }
    else {
      status=FAIL;                                    /* Zero not legal.   */
    }
  }
  else if (get_barcode_type(c_item.arti.base.bar_cd)==BARCD_EXTERNAL_WEIGHT_EAN13) {
    _tcscpy(buffer,(c_item.arti.base.bar_cd+7));      /* Scale             */
    buffer[5]=(_TCHAR)0;                               /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));     /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      /*                                                                   */
      /* Weight is in the barcode.                                         */
      /*     qty = weight from EAN13                                       */
      /*     gds_value = ppi * weight from EAN13                           */
      /*                                                                   */
      c_item.arti.base.qty = strtod(ticket_pwd, NULL)/1000.0;
      if (invoice_line_mode == RETURN) {
        c_item.arti.base.qty*=-1;
      }
      status=calc_gds_value(ART_GROUP);     /* SUCCEED/FAIL/PRICE_TOO_LARGE */
    }
  }
  else {                                       /* Reduced to clear barcode. */
    _tcscpy(buffer,(c_item.arti.base.bar_cd+8));       /* Article no gone   */
    buffer[4]=(_TCHAR)0;                               /* Checkdigit gone   */
    _tcscpy(ticket_pwd,no_leading_zeros(buffer));      /* leading 0s gone   */

    if (!STR_ZERO(ticket_pwd)) {
      if (c_item.arti.art_ind==ART_IND_WEIGHT) {/* Qty for discount allways */
        c_item.disc.base.qty=1.0;               /* positive!                */
      }
      else {
        c_item.disc.base.qty=fabs(c_item.arti.base.qty);
      }

      _tcsncpy(c_item.disc.base.descr, scrn_inv_TXT[25], 33);
      c_item.disc.base.vat_no         = c_item.arti.base.vat_no;
      c_item.disc.base.vat_perc       = c_item.arti.base.vat_perc;
      c_item.disc.base.art_grp_no     = c_item.arti.base.art_grp_no;
      c_item.disc.base.art_grp_sub_no = c_item.arti.base.art_grp_sub_no;
      c_item.disc.base.dept_cd        = c_item.arti.base.dept_cd;
      c_item.disc.base.price = floor_price((c_item.arti.base.price/100) *
                                             (_tcstod(ticket_pwd, NULL)/100))*100;
      if (invoice_line_mode==SALES) {
        c_item.disc.base.price*=-1;
      }
      status=calc_gds_value(ART_GROUP);
      if (status==SUCCEED) {
        status=calc_gds_value(DISCNT_GROUP); /* SUCCEED/FAIL/PRICE_TOO_LARGE */
      }
    }
    else {
      status=FAIL;                                /* Zero not legal.       */
    }
  }

  return(status);
} /* decode_internal_ean13_barcode */


/*---------------------------------------------------------------------------*/
/*                           SC_get_barcode                                  */
/*---------------------------------------------------------------------------*/
static short SC_get_barcode(_TCHAR *barcd, _TCHAR *artno)
{
  short status;
  BARCODE_DEF barcode;

  /*                                                                       */
  /* Locate an article in the barcode table.                               */
  /*   Returns SUCCEED if found and article number is copied into artno.   */
  /*                                                                       */

  memset(&barcode, 0, sizeof(BARCODE_DEF));
  if (get_barcode_type(barcd)==BARCD_SCALE_EAN13) {
    /*                                                                     */
    /* Scale barcodes: 2Xaaaa0000000                                       */
    /*                                                                     */
    /*  Only the 4(!) digit scale PLU number is used to locate a barcode.  */
    /*  Also the keyrelationtype keyGTE is used to hit any barcode         */
    /*  starting with '2Xaaaa0000000'                                      */
    /*                                                                     */
    /* After retrieving a barcode successfully, compare first 6 digits     */
    /* due to the fact that keyGTE is used as a KeyRelationType:           */
    /*                                                                     */
    /*  get 2112340000000  -> retrieved 2112340000009    = OK              */
    /*  get 2812340000000  -> retrieved 2812340000009    = OK              */
    /*  get 2100010000000  -> retrieved 2100020000009    = ERROR           */
    /*  get 2600010000000  -> retrieved 2600010000009    = ERROR           */
    /*                                                                     */
                                /* Retrieve scale-barcode using keyGTE.    */
    _tcscpy(barcode.barcode, barcd);
    memset(barcode.barcode+6, 0, 7);
    status= pos_get_rec(BARC_TYPE, POS_BARC_SIZE, BAR_BARC_IDX,
                       (void*)&barcode, (short) keyGTE);
    if (status==SUCCEED) {
      status=_tcsncmp(barcode.barcode, barcd, 6);
    }
    _tcscpy(barcode.barcode, barcd);           /* Put back actual barcode.  */
  }
  else if (get_barcode_type(barcd)==BARCD_EXTERNAL_WEIGHT_EAN13) {
    /* Este solemente por Colombia (S.T. 09/09/1995)                       */
    /* External weight barcodes: 29aaaaa000000                             */
    /*                                                                     */
    /*  Only the 5(!) digit aaaaa is used to locate a barcode.             */
    /*  Also the keyrelationtype keyGTE is used to hit any barcode         */
    /*  starting with '2Xaaaaa000000'                                      */
    /*                                                                     */
    /* After retrieving a barcode successfully, compare first 7 digits     */
    /* due to the fact that keyGTE is used as a KeyRelationType:           */
    /*                                                                     */
    /*                                                                     */
                                /* Retrieve scale-barcode using keyGTE.    */
    _tcscpy(barcode.barcode, barcd);
    memset(barcode.barcode+7, 0, 6);
    status= pos_get_rec(BARC_TYPE, POS_BARC_SIZE, BAR_BARC_IDX,
                       (_TCHAR *)&barcode, (short) keyGTE);
    if (status==SUCCEED) {
      status=_tcsncmp(barcode.barcode, barcd, 7);
    }
    _tcscpy(barcode.barcode, barcd);           /* Put back actual barcode.  */
  }
  else {
                                /* Retrieve a normal barcode using keyEQL  */
    if (get_barcode_type(barcd)==BARCD_EXTERNAL_EAN128) {
      _tcsncpy(barcode.barcode, (barcd+3), 13);
    }
    else {
      if (get_barcode_type(barcd)==BARCD_EXTERNAL_DUMP14) {
         _tcsncpy(barcode.barcode, (barcd+1), 14);
      }
      else {
        _tcsncpy(barcode.barcode, barcd, 14);
      }
    }
    status=pos_get_rec(BARC_TYPE, POS_BARC_SIZE, BAR_BARC_IDX,
		(void*)&barcode, (short) keyEQL);
  }

  if (status==SUCCEED) {               /* Found barcode, copy artno to arg */
    ftoa((double)barcode.art_no, 10, artno);
  }

  return(status);
} /* SC_get_barcode */


/*---------------------------------------------------------------------------*/
/*                              get_barcode_type                             */
/*---------------------------------------------------------------------------*/
short get_barcode_type(_TCHAR *barcd)
{
  short scale_type, l;

  l=_tcslen(barcd);

  if (l<8) {
    return(BARCD_ILLEGAL);
  }

  if (l==8 && *barcd==_T('2')) {
    return(BARCD_INTERNAL_EAN8);
  }

  if (l==13 && *barcd==_T('2') && *(barcd+1)==_T('0')) { /* 20  reduced to clear   */
    return(BARCD_INTERNAL_EAN13);
  }

  if (l==13 && *barcd==_T('2') && *(barcd+1)==_T('9')) { /* 29  EXT. WEIGHT BARCODE */
    return(BARCD_EXTERNAL_WEIGHT_EAN13);
  }

  scale_type=get_scale_barcode_type(barcd);
  if (scale_type!=BARCD_ILLEGAL) {               /* Scale price or weight  */
    return(BARCD_SCALE_EAN13);
  }

  if (l>15 && *barcd==_T('0') && *(barcd+1)==_T('1')) {  /* EAN128 barcode         */
    return(BARCD_EXTERNAL_EAN128);
  }

  if (l>14 && *barcd==_T('2')) {                         /* DUMP14 barcode         */
    return(BARCD_EXTERNAL_DUMP14);
  }


  return(BARCD_EXTERNAL);
} /* get_barcode_type */


/*---------------------------------------------------------------------------*/
/*                              get_scale_barcode_type                       */
/*---------------------------------------------------------------------------*/
static short get_scale_barcode_type(_TCHAR *barcd)
{
  short l=_tcslen(barcd);
  _TCHAR scale_price[] =_T("5637");
  _TCHAR scale_weight[]=_T("4");

  if (l==13 && *barcd==_T('2')) {
    if (_tcschr(scale_price, *(barcd+1))) {
      return(BARCD_SCALE_PRICE);
    }

    if (_tcschr(scale_weight, *(barcd+1))) {
      return(BARCD_SCALE_WEIGHT);
    }
  }

  return(BARCD_ILLEGAL);
} /* get_scale_barcode_type */


/*---------------------------------------------------------------------------*/
/*                           barcd_to_artno                                  */
/*---------------------------------------------------------------------------*/
static short barcd_to_artno(_TCHAR *artno, _TCHAR *barcd)
{
  short status;

  /*                                                                       */
  /* Converts a barcode to an article number. Optionally uses the barcode  */
  /* table to get the article number.                                      */
  /*                                                                       */

  status=FAIL;

  switch (get_barcode_type(barcd)) {
    case BARCD_INTERNAL_EAN13 :
      if (check_barcode_ean(barcd)==SUCCEED) {
        _tcscpy(artno, (barcd+2));
        *(artno+6)=(_TCHAR)0;
        status=SUCCEED;
      }
      break;
    case BARCD_INTERNAL_EAN8  :
      if (check_barcode_ean(barcd)==SUCCEED) {
        _tcscpy(artno, (barcd+1));
        *(artno+6)=(_TCHAR)0;
        status=SUCCEED;
      }
      break;
    case BARCD_EXTERNAL_WEIGHT_EAN13    :
      if (check_barcode_ean(barcd)==SUCCEED) {
        status=SC_get_barcode(barcd, artno);
      }
      break;
    case BARCD_SCALE_EAN13    :
      if (check_barcode_ean(barcd)==SUCCEED) {
        status=SC_get_barcode(barcd, artno);
      }
      break;
    case BARCD_EXTERNAL_EAN128:
      status=SC_get_barcode(barcd, artno);
      break;
    case BARCD_EXTERNAL       :
      status=SC_get_barcode(barcd, artno);
      break;
    default:
      break;
  }
  if (status != SUCCEED) {
    err_invoke(ILLEGAL_BARCODE);
    return(FAIL);
  }
  else {
    return(SUCCEED);
  }
} /* barcd_to_artno */


/*---------------------------------------------------------------------------*/
/*                          reverse_invoice_line_mode                        */
/*---------------------------------------------------------------------------*/
static short reverse_invoice_line_mode(_TCHAR *data, short key)
{
  if (invoice_line_mode==SALES) {
    invoice_line_mode=RETURN;
  }
  else {
    invoice_line_mode=SALES;
  }

#ifndef NO_VIEW_POS_STATE
  view_pos_state();
#endif

  return(key);
} /* reverse_invoice_line_mode */


/*---------------------------------------------------------------------------*/
/*                          handle_scroll_key                                */
/*---------------------------------------------------------------------------*/
short handle_scroll_key(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Handles the scrolling of the article-list-window.                     */
  /*                                                                       */

  switch(key) {
    case LINE_UP_KEY:
      scrl_line_up(INV_ART_LIST_WINDOW);
      break;
    case LINE_DOWN_KEY:
      scrl_line_down(INV_ART_LIST_WINDOW);
      break;
    case PAGE_UP_KEY:
      scrl_page_up(INV_ART_LIST_WINDOW);
      break;
    case PAGE_DOWN_KEY:
      scrl_page_down(INV_ART_LIST_WINDOW);
      break;
    default:
      break;
  }

  return(UNKNOWN_KEY);
} /* handle_scroll_key */



/*---------------------------------------------------------------------------*/
/*                             vfy_art_qty                                   */
/*---------------------------------------------------------------------------*/
short vfy_art_qty(_TCHAR *data, short key)
{
  short status;

  /*                                                                       */
  /* Perform some checks before accepting the entered quantity.            */
  /*                                                                       */

  no_leading_zeros(data);
  if (!STR_ZERO(data) && _tcslen(data)<=4) {
    c_item.arti.base.qty=(double)_tcstod(data, NULL);
    if (invoice_line_mode!=SALES) {
		printf_log("Entro vfy_art_qty");
		c_item.arti.base.qty*=-1;
    }

    /*                                                                     */
    /* Add the current item to the scroll-list using C_ITEM as an id.      */
    /* These scroll-lines are temporarily. On acceptance, the lines are    */
    /* replaced with lines with a TM_INDX id. On canceling, the lines      */
    /* are deleted.                                                        */
    /*                                                                     */
    if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
      == SCRL_UNKNOWN_LINE) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
    }

    if ( (c_item.depo.base.price != 0.0) &&
         (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM)
         == SCRL_UNKNOWN_LINE) ) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM);
    }

    view_item(OPER_SCRN | CUST_SCRN);
    status=key;
  }
  else {
    if (STR_ZERO(data)) {
      err_invoke(ZERO_NOT_LEGAL_ERROR);
    }
    else {
      err_invoke(QTY_TOO_LARGE);
    }
    status=UNKNOWN_KEY;
  }
  *data=_T('\0');

  return(status);
} /* vfy_art_qty */


/* 12-Ago-2011 acm -{*/

/*---------------------------------------------------------------------------*/
/*                is_numeric                                                 */
/*---------------------------------------------------------------------------*/
int is_numeric (const char * s)
{
  char * p;
  if (s == NULL || *s == '\0' || isspace(*s))
     return 0;
   strtol(s, &p,10);
   return (*p == 0);
}

/*---------------------------------------------------------------------------*/
/*                strsplit                                                   */
/*---------------------------------------------------------------------------*/

int strsplit(char* str, char   splitstr[][LEN_TURKEY_CODE])
{
     char* p;
     char splitbuf[100];
     char buffer  [1000];
     int i=0;

     strcpy(buffer, str);

     p = strtok(buffer,",");
     while(p!= NULL)
     {
               strcpy(splitbuf,p);
               strcpy(splitstr[i],splitbuf);
               //strtrim(splitstr[i]);
               i++;
               p = strtok (NULL, ",");
     }
     
     return i;
}


/*---------------------------------------------------------------------------*/
/*                check_number                                               */
/*---------------------------------------------------------------------------*/
int check_number(char   reg_values[][LEN_TURKEY_CODE])
{

  int i;
  for (i=0;i<VALE_ARTICLE_COUNT; i++)
  {
    if (reg_values[i][0]=='\0') break;
    if (!is_numeric(reg_values[i])) return 0;

  }
  return 1;

}

/* 27-Jan-2012 acm - { */
/*---------------------------------------------------------------------------*/
/*                strnsplit                                                   */
/*---------------------------------------------------------------------------*/
#define REGEDIT_ITEM_LEN          50 
#define REGEDIT_ITEM_MAX_ITEMS    20 

int strnsplit(char* str, char   splitstr[][REGEDIT_ITEM_LEN], int num_items) 
{
     char* p;
     char splitbuf[REGEDIT_ITEM_LEN*2];
     char buffer  [REGEDIT_ITEM_MAX_ITEMS*REGEDIT_ITEM_LEN];
     int i=0;

     strcpy(buffer, str);

     p = strtok(buffer,",");
     while(p!= NULL)
     {
               strcpy(splitbuf,p);
               strcpy(splitstr[i],splitbuf);
               //strtrim(splitstr[i]);
               i++;
               p = strtok (NULL, ",");
               if (i>=num_items) break; /* 27-Jan-2012 acm - se agrego condicional*/
     }
     return i;
}

/*---------------------------------------------------------------------------*/
/*                reg_aditional_regedit_load                                 */
/*---------------------------------------------------------------------------*/
char reg_promotion_anniversary[4][REGEDIT_ITEM_LEN]; /* acepta 20 articulos  de longitud 100*/
char reg_promotion_horeca     [2][REGEDIT_ITEM_LEN]; /* acepta 2  items  de longitud 100    */

extern  void configuration_regedit_all();

int reg_aditional_regedit_load()
{
  _TCHAR reg_buffer[REGEDIT_ITEM_MAX_ITEMS*REGEDIT_ITEM_LEN];
  int    reg_buffer_num_elem=0;
  int    reg_buffer_load__=0;

  static int reg_aditional_load__=0;

  if (reg_aditional_load__==0)
  {

    ///1. reg_promotion_anniversary
    //{
    memset(reg_promotion_anniversary, sizeof(reg_promotion_anniversary),0);

    memset(reg_buffer,                sizeof(reg_buffer),               0);
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("PROMOCION_ANIVERSARIO"), reg_buffer, sizeof(reg_buffer)-1);
    if (!reg_buffer[0]){
        de_init(_T("")); 
        exit(1);      
        return 0;
    }

    strcat(reg_buffer,",");
    reg_buffer_num_elem = strnsplit(reg_buffer, reg_promotion_anniversary,REGEDIT_ITEM_MAX_ITEMS);

    prom_anniv_price_cupon= atol(reg_promotion_anniversary[0]);
    prom_anniv_price_gift = atol(reg_promotion_anniversary[1]);
    
    if (reg_promotion_anniversary[2][0]&&reg_promotion_anniversary[3][0])
    {
      prom_anniv_date_begin = atol(reg_promotion_anniversary[2]);
      prom_anniv_date_end   = atol(reg_promotion_anniversary[3]);
    }else {
      prom_anniv_date_begin = 701;
      prom_anniv_date_end   = 731;
    }
    //}


    ///2. reg_promotion_horeca
    //{
    memset(reg_promotion_horeca, sizeof(reg_promotion_horeca),0);

    memset(reg_buffer,                sizeof(reg_buffer),         0);
    SetRegistryEcho(FALSE);
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("PROMOCION_HORECA"), reg_buffer, sizeof(reg_buffer)-1);
    /*
    if (!reg_buffer[0]){
        de_init(_T("")); 
        exit(1);      
        return 0;
    }
    */
    SetRegistryEcho(TRUE);

    strcat(reg_buffer,",");
    reg_buffer_num_elem = strnsplit(reg_buffer, reg_promotion_horeca,REGEDIT_ITEM_MAX_ITEMS);

    if (reg_promotion_horeca[0][0]&&reg_promotion_horeca[1][0])
    {
      promotion_horeca_date_begin = atol(reg_promotion_horeca[0]);
      promotion_horeca_date_end   = atol(reg_promotion_horeca[1]);
    }else {
      promotion_horeca_date_begin = 607;
      promotion_horeca_date_end   = 621;
    }
    //}

    configuration_regedit_all();
    ///--------------------------------------------------------------------------------------------
    ///--------------------------------------------------------------------------------------------
    reg_aditional_load__=1;
  }
  return 1;
}

/* 27-Jan-2012 acm - } */


/*---------------------------------------------------------------------------*/
/*                reg_turkey_load                                            */
/*---------------------------------------------------------------------------*/
int reg_turkey_load()
{
	_TCHAR reg_turkey[VALE_ARTICLE_COUNT*LEN_TURKEY_CODE+1];

  int reg_vale_turkey_n=0;
  int reg_article_turkey_n=0;
  if (reg_turkey_load__==0)
  {
    //inicializar variables
    memset(reg_vale_turkey,   0,sizeof(reg_vale_turkey)   );
    memset(reg_article_turkey,0,sizeof(reg_article_turkey));
    
    //leer del regedit los codigos de los articulos "vale pavo"
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("VALE_TURKEY_CODE"), reg_turkey, LEN_TURKEY_CODE);
    strcat(reg_turkey,",");
    reg_vale_turkey_n = strsplit(reg_turkey, reg_vale_turkey);
    
    //leer del regedit los codigos de los articulos "articulo pavo"
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("TURKEY_CODE"),      reg_turkey, LEN_TURKEY_CODE);
    strcat(reg_turkey,",");
    reg_article_turkey_n=strsplit(reg_turkey, reg_article_turkey);


    // verificar que los articulo "vale pavo" sean numericos
    if (!check_number(reg_vale_turkey))
    {
          MessageBox(NULL,(LPTSTR) _T("Error en Regedit/el código de vale pavo debe ser numérico"), 
                                       (LPTSTR) _T("Error"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);                 
          de_init(_T("")); 
          exit(1);      
          return 0;
    }
    // verificar que los articulos "articulo pavo" sean numericos
    if (!check_number(reg_article_turkey))
    {
          MessageBox(NULL,(LPTSTR) _T("Error en Regedit/el código de article pavo debe ser numérico"), 
                                       (LPTSTR) _T("Error"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);                 
          de_init(_T("")); 
          exit(1);         
          return 0;
    }
    // comparar que la cantidad de articulos "vale de pavos" y "articulos pavos" sean iguales
    if (reg_vale_turkey_n!=reg_article_turkey_n){

          MessageBox(NULL,(LPTSTR) _T("Error en Regedit/el número de elementos en el 'vale de pavo' y 'article pavo'"), 
                                       (LPTSTR) _T("Error"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);                 
          de_init(_T("")); 
          exit(1);       
          return 0;
    }
    reg_turkey_load__=1;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/*                get_turkey_article_index                                   */
/*---------------------------------------------------------------------------*/

int get_turkey_article_index(_TCHAR *data)
{
  int i;
  if (data==NULL) return -1;
  if (*data==0)   return -1;
  if (strlen(data)==0) return -1;
  
  for (i=0;i<VALE_ARTICLE_COUNT; i++)
  {
    if (reg_vale_turkey[i][0]=='\0') break;
    if (strcmp(reg_article_turkey[i],data)==0) return i;
  }
  return -1;
}

/*---------------------------------------------------------------------------*/
/*                get_vale_article_index                                     */
/*---------------------------------------------------------------------------*/
int get_vale_article_index(_TCHAR *data)
{
  int i;

  if (data==NULL) return -1;
  if (*data==0) return -1;
  if (strlen(data)==0) return -1;


  for (i=0;i<VALE_ARTICLE_COUNT; i++)
  {
    if (reg_vale_turkey[i][0]=='\0') break;
    if (strcmp(reg_vale_turkey[i],data)==0) return i; // verificar los 6 primeros caracteres el cual contiene el codigo del articulo vale
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
/*                is_turkey_article                                          */
/*---------------------------------------------------------------------------*/
int turkey_article_valid(_TCHAR *article_turkey, _TCHAR *vale_turkey)
{

  int article_turkey_index=-1;
  int vale_turkey_index   =-1;
  int encontrado          = 0;

  vale_turkey_index       = get_vale_article_index(vale_turkey);
  if (vale_turkey_index!=-1){
    if (strncmp(reg_article_turkey[vale_turkey_index], article_turkey,6)==0)
      encontrado=1;
    else 
      encontrado=0;
  }else  {
    encontrado=0;
  }
  return encontrado;
}


/*---------------------------------------------------------------------------*/
/*                is_vale_article                                            */
/*---------------------------------------------------------------------------*/

int is_vale_article(_TCHAR *vale_turkey)
{
  return (get_vale_article_index(vale_turkey)>=0);
}

/*---------------------------------------------------------------------------*/
/*                is_vale_article_i                                          */
/*---------------------------------------------------------------------------*/
int is_vale_article_i(long vale_turkey)
{
  char art_code_tmp[100];
  itoa(vale_turkey,art_code_tmp,10);
  return is_vale_article(art_code_tmp);
}

/*---------------------------------------------------------------------------*/
/*                is_turkey_article                                          */
/*---------------------------------------------------------------------------*/

int is_turkey_article(_TCHAR *article_turkey)
{
  return (get_turkey_article_index(article_turkey)>=0);
}

/*---------------------------------------------------------------------------*/
/*                is_turkey_article_i                                        */
/*---------------------------------------------------------------------------*/
int is_turkey_article_i(long article_turkey)
{
  char art_code_tmp[100];
  itoa(article_turkey,art_code_tmp,10);
  return is_turkey_article(art_code_tmp);
}
/*---------------------------------------------------------------------------*/
/*                ValeTurkey_is_val_art_no                                   */
/*---------------------------------------------------------------------------*/
int  ValeTurkey_is_val_art_no(_TCHAR *data, short key, 
							  ARTICLE_DEF * article, ARTICLE_DEF *deposit,
							  short * barcd_type, 
                TM_ITEM_GROUP * c_item,
                //size_t data_length,
                int p_barcd_type)
{

  
  _TCHAR  artno[16];
  _TCHAR  buffer[100];
  _TCHAR  buff_weight[100];
  _TCHAR  buff_valeno[100];
  _TCHAR  buff_valetype[100];
  
  short  status;
  double precio_vale;
  
  //, status;

  /*                                                                       */
  /* Get article and a possible deposit from the PLU.                      */
  /* - data can be an article number or a barcode.                         */
  /* - a possible qty is already in c_item.arti.base.qty.                  */
  /*                                                                       */
  /* Returns:                                                              */
  /*   ARTICLE_NOT_OK if any error.                                        */
  /*    UNKNOWN_KEY   if any error                                         */ 
  /* else                                                                  */
  /*   ARTICLE_OK     item is complete, can be accepted                    */

  

	if (STR_ZERO(data)) 
	{
		*data = _T('\0');
		err_invoke(INVALID_ARTNO);
		delete_active_item(empty,0);
		return(ARTICLE_NOT_OK);
	}
    /*                                                                     */
    /* Step 1: Convert data to an article number.                          */
    /*                                                                     */
  if (p_barcd_type == BARCD_VALE_TURKEY_EAN13){ // correlativo (5 digitos) + articulo(6 digitos) +peso
     if (strlen(data)!=12){
		    *data = _T('\0');
		    err_invoke(INVALID_ARTNO);
		    delete_active_item(empty,0);
		    return(ARTICLE_NOT_OK);
     }
     //cccccAAAAAAPV
     //0123456789012
     
     //substring(buffer,      data, 5,6);  //article
     //substring(buff_weight, data,11,1);  //weight

     substring(buffer,      data, 6,6);  //article
     strcpy   (buff_weight, "7");
     substring(buff_valeno,  data, 0,6);  //ValeNo
     substring(buff_valetype,data, 0,1);  //ValeNo
     


     if (!isValeTurkey_Valid(_ttol(buff_valeno), _ttol(buff_valetype)))
     {
		*data = _T('\0');
		err_invoke(INVALID_VALEPAVO);
		delete_active_item(empty,0);
		return(ARTICLE_NOT_OK);
     }

     
     //AAAAAAPV
     //0123456789012
     /* EAN8
     substring(buffer,      data, 0,6);  //article
     substring(buff_weight, data, 6,1);  //weight
     */

     if (!is_vale_article(buffer))
     {
		    *data = _T('\0');
		    err_invoke(INVALID_ARTNO);
		    delete_active_item(empty,0);
		    return(ARTICLE_NOT_OK);
     }

     c_item->arti.base.art_no = _ttol(buffer);
     
     _tcscpy(c_item->arti.base.bar_cd, data);


     *barcd_type = BARCD_VALE_TURKEY_EAN13; 
  }else{

      if (get_barcode_type(data)==BARCD_INTERNAL_EAN13) {
	      
        err_invoke(NO_DISC_ON_R2C_BARCD);
        //*data=_T('\0');
        return(UNKNOWN_KEY);
      }

      *barcd_type = get_barcode_type(data);
      if (*barcd_type != BARCD_ILLEGAL) {              /* It's a barcode..    */
        if (barcd_to_artno(artno, data) == SUCCEED) {
          if (*barcd_type == BARCD_EXTERNAL_EAN128) {
            _tcsncpy(c_item->arti.base.bar_cd, (data+3), 13);
            c_item->arti.base.bar_cd[14] = _T('\0');
          }
          else {
            if (*barcd_type == BARCD_EXTERNAL_DUMP14) {
              _tcsncpy(c_item->arti.base.bar_cd, (data+1), 14);
              c_item->arti.base.bar_cd[15] = _T('\0');
            }
            else {
              _tcscpy(c_item->arti.base.bar_cd, data);
            }
          }
          c_item->arti.base.art_no = _ttol(artno);
        }
        else {
          delete_active_item(empty,0);
          return(ARTICLE_NOT_OK);
        }
      }
      else {                                        /* It's an article no. */
        c_item->arti.base.art_no=_ttol(data);
      }
  }

    /*                                                                   */
    /* Step 2: Get article data from PLU and handle exceptions.          */
    /*                                                                   */
    article->art_no = c_item->arti.base.art_no;
    status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
								(void*)article, (short) keyEQL);
    if (status != SUCCEED) {
      err_invoke(INVALID_ARTNO);
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
    }
    else if (article->art_no != c_item->arti.base.art_no) {
      err_invoke(DATA_CORRUPT);                /* article file corrupted */
      delete_active_item(empty,0);             /*  rebuild with stnetp85 */
      return(ARTICLE_NOT_OK);
    }

    /*                                                                   */
    /* Step 2a: No discount allowed on a sold-deposit article.           */
    /*                                                                   */
    if (article->type == ART_IND_DEPOSIT &&
        (state_number() == ST_DISCOUNT ||
         state_number() == ST_DISCOUNT_QTY) ) {
      err_invoke(NO_DISCOUNT_ON_DEPOSIT);
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
    }

    /*                                                                   */
    /* Step 2b: If article is blocked, ask for S-key.                    */
    /*                                                                   */
    if (article->block_ind == YES) {
      if (err_invoke(ARTICLE_BLOCKED) != SUCCEED) {
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
      else {
        c_item->approved = TRUE;
      }
    }

    /*                                                                   */
    /* Step 2c: Put all article information in the current item.         */
    /*                                                                   */
    if (c_item->arti.base.qty == 0.0) {
      if (invoice_line_mode == SALES) {
        c_item->arti.base.qty = 1;
      }
      else {
        c_item->arti.base.qty = -1;
      }
    }

    if (*barcd_type == BARCD_VALE_TURKEY_EAN13){
      c_item->arti.base.qty = _ttol(buff_weight);
    }

    _tcsncpy(c_item->arti.pack_type, article->pack_type, 3);
    c_item->arti.art_ind             = article->type;
    c_item->arti.mmail_no            = article->mmail_no;
    c_item->arti.items_per_pack      = article->cont_sell_unit;
    c_item->arti.base.vat_no         = article->vat_no;
    c_item->arti.base.vat_perc       = get_vat_perc(article->vat_no);
    c_item->arti.base.art_grp_no     = article->art_grp_no;
    c_item->arti.base.art_grp_sub_no = article->art_grp_sub_no;
    c_item->arti.base.dept_cd        = article->dept_cd;

    c_item->arti.base.arti_retention_ind    = article->arti_retention_ind   ; //v3.6.1 acm -
    c_item->arti.base.arti_perception_ind   = article->arti_perception_ind  ; //v3.6.1 acm -
    c_item->arti.base.arti_rule_ind         = article->arti_rule_ind        ; //v3.6.1 acm -
    c_item->arti.base.percep_amount         = 0;                            ;  //v3.6.1 acm -
    c_item->arti.base.arti_detraccion_ind          = article->arti_detraccion_ind;                            ; //v3.6.2 wjm -
    
    _tcscpy(c_item->arti.base.descr, article->descr);
    _tcscpy(c_item->arti.reg_no, article->reg_no);
    c_item->arti.base.suppl_no       = article->suppl_no;
/*
    if (genvar.price_incl_vat == INCLUSIVE) {
	    c_item->arti.base.price =
                floor_price(calc_incl_vat(article->sell_pr, article->vat_no));

    }
    else {
      c_item->arti.base.price = floor_price(article->sell_pr);
    }

*/
   if (*barcd_type == BARCD_VALE_TURKEY_EAN13)
   {
       precio_vale =atof(cur_valepavo.sell_pr )/c_item->arti.base.qty;
       if (genvar.price_incl_vat == INCLUSIVE) {
	        c_item->arti.base.price =
                    floor_price(calc_incl_vat( precio_vale /*article->sell_pr*/, article->vat_no));
        }
        else 
        {
          c_item->arti.base.price = floor_price(precio_vale);
        }
   }else
   {
        if (genvar.price_incl_vat == INCLUSIVE) {
	        c_item->arti.base.price =
                    floor_price(calc_incl_vat(article->sell_pr, article->vat_no));
        }
        else 
        {
          c_item->arti.base.price = floor_price(article->sell_pr);
        }
   }



    /*                                                                   */
    /* Step 2d: If article is ok till now, calculate goods-value.        */
    /*          - If it is a barcode, first decode the barcode and then  */
    /*            calculate the goods-value                              */
    /*                                                                   */

    if (p_barcd_type == BARCD_VALE_TURKEY_EAN13){ 

    }else{
      if (*barcd_type == BARCD_INTERNAL_EAN13 ||
          *barcd_type == BARCD_SCALE_EAN13    ||
          *barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) { /* treated as internal */
        status =decode_internal_ean13_barcode_turkeyV2();

        if ( status!= SUCCEED) { //lectura del peso dentro del barcode EAN13 
                           /* returns FAIL / SUCCEED / PRICE_TOO_LARGE or  */
                           /* DISCNT_AMNT_TOO_LARGE  /WEIGHT_TURKEY_MUSTBE7              */
          //descontar el peso del vale
          
          delete_active_item(empty,0);
          return(ARTICLE_NOT_OK);
        }
      }
      else if (calc_gds_value(ART_GROUP) != SUCCEED) {
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
    }
    /*                                                                   */
    /* Step 3:  If this article has a deposit, get it and handle         */
    /*          exceptions                                               */
    /*                                                                   */
    if (article->type != ART_IND_DEPOSIT &&
        article->art_no_deposit != 0L) {

		  deposit->art_no = article->art_no_deposit;
		  status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
								 (void*)deposit, (short) keyEQL);
		  if (status != SUCCEED) {
			  err_invoke(INVALID_ARTNO);
			  delete_active_item(empty,0);
			  return(ARTICLE_NOT_OK);
		  }
		  else if (deposit->art_no != article->art_no_deposit) {
			  err_invoke(DATA_CORRUPT);               /* article file corrupted  */
			  delete_active_item(empty,0);            /*  rebuild with stnetp85  */
			  return(ARTICLE_NOT_OK);
		  }

		  /*                                                                   */
		  /* Step 3a: If deposit is blocked, ask for S-key.                    */
		  /*                                                                   */
		  if (deposit->block_ind == YES) {
			  if (err_invoke(ARTICLE_BLOCKED) != SUCCEED) {
			    delete_active_item(empty,0);
			    return(ARTICLE_NOT_OK);
			  }
			  else {
			    c_item->approved = TRUE;
			  }
		  }

		  /*                                                                   */
		  /* Step 3b: Put all deposit information in the current item.         */
		  /*                                                                   */
		  if (article->type == ART_IND_WEIGHT) {
			  if (invoice_line_mode == SALES) {
			    c_item->depo.base.qty = 1;
			  }
			  else {
			    c_item->depo.base.qty = -1;
			  }
		  }
		  else {
			  c_item->depo.base.qty = c_item->arti.base.qty;
		  }


		  _tcsncpy(c_item->depo.pack_type, deposit->pack_type, 3);
		  c_item->depo.mmail_no            = deposit->mmail_no;
		  c_item->depo.base.art_no         = deposit->art_no;
		  c_item->depo.base.vat_no         = deposit->vat_no;
		  c_item->depo.base.vat_perc       = get_vat_perc(deposit->vat_no);
		  c_item->depo.base.art_grp_no     = deposit->art_grp_no;
		  c_item->depo.base.art_grp_sub_no = deposit->art_grp_sub_no;
		  c_item->depo.base.dept_cd        = deposit->dept_cd;
		  _tcscpy(c_item->depo.base.descr, deposit->descr);
		  _tcscpy(c_item->depo.reg_no,	deposit->reg_no);
		  c_item->depo.base.suppl_no       = deposit->suppl_no;

		  if (genvar.price_incl_vat == INCLUSIVE) {
			  c_item->depo.base.price =
					floor_price(calc_incl_vat(deposit->sell_pr, deposit->vat_no));
		  }
		  else {
			  c_item->depo.base.price = floor_price(deposit->sell_pr);
		  }

		  /*                                                                   */
		  /* Step 3c: If deposit is ok till now, calculate goods-value.        */
		  /*                                                                   */
		  if (calc_gds_value(DEPOSIT_GROUP) != SUCCEED) {
			delete_active_item(empty,0);
			return(ARTICLE_NOT_OK);
		  }
    } /* if, end of the deposit section. */
	
	return (ARTICLE_OK); // las validacions son conformes entonces continuar con el flujo.
}


/*---------------------------------------------------------------------------*/
/*                ValeTurkey_vfy_and_start_artno_detalle                     */
/*---------------------------------------------------------------------------*/
short ValeTurkey_vfy_and_start_artno_detalle(_TCHAR *data, short key)
{
  ARTICLE_DEF article, deposit;
  short barcd_type, status;
  char art_no_vale_turkey[100];
  char art_code_base_tmp[100];
  double data_value_fix=0;

  /*                                                                       */
  /* Get article and a possible deposit from the PLU.                      */
  /* - data can be an article number or a barcode.                         */
  /* - a possible qty is already in c_item.arti.base.qty.                  */
  /*                                                                       */
  /* Returns:                                                              */
  /*   ARTICLE_NOT_OK if any error.                                        */
  /*   UNKNOWN_KEY    if any error.                                        */
  /* else                                                                  */
  /*   ARTICLE_OK     item is complete, can be accepted                    */
  /*   PRICE_ART      price must be entered                                */
  /*   WWEIGHT_ART    weight article, enter weight                         */
  /*   PWEIGHT_ART    weight article, enter price                          */
  /*                                                                       */

  //if (is_vale_article(data))
  status=  ARTICLE_NOT_OK;//inicializo

  if (c_item_valeturkey.arti.base.art_no==0)/* acm -++ Si no se ha leido el  pavo vale*/
  {
    //leer el pavo vale
    status=ValeTurkey_is_val_art_no(data,key, 
                                    &article,     &deposit,
                                    &barcd_type,  
                                    &c_item,BARCD_VALE_TURKEY_EAN13);
    if (status==  ARTICLE_NOT_OK ||
        status==  UNKNOWN_KEY ) return status;

    /* Validar si no se ha ingresado otro vale de pavo */
    if ( get_valeturkey_count()>=1 ) { //acm-++ 12-oct-2011
      err_invoke(ONE_VALE_BY_INVOICE);
      return ARTICLE_NOT_OK ;
    }

    status=c_item.arti.display_status ;
		switch (article.type)
		{
		  case ART_IND_WEIGHT:
			if (barcd_type == BARCD_INTERNAL_EAN13 ||
				  barcd_type == BARCD_SCALE_EAN13    ||
				  barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13||
          barcd_type == BARCD_VALE_TURKEY_EAN13) {

          status=ARTICLE_OK;
			}
			else {
			  if (genvar.price_weight == WEIGHT) {
          status=ARTICLE_OK;
				  c_item.arti.display_status =   ARTICLE_OK;
				//c_item.arti.display_status = WWEIGHT_ART;
			  }
			  else {
          status=ARTICLE_OK;
         // c_item.arti.base.qty=
				//c_item.arti.display_status = PWEIGHT_ART;
			  }
      } 
			break;

		  default:
			  //c_item.arti.display_status = VALE_ARTICLE_NOT_OK; //ARTICLE_NOT_OK;
        status=ARTICLE_OK; //ARTICLE_NOT_OK;
			break;
		}
    c_item.arti.display_status = status;

    //ValeTurkey_VW();
  }else if (c_item_valeturkey.arti.display_status== ARTICLE_OK)
  {
      

      art_code_base_tmp[0]='\0';
      itoa(c_item_valeturkey.arti.base.art_no,  art_no_vale_turkey,10);

      /*
      if (data[0]){
          if (strlen(data)<=6)
          {
            // copiar el codigo del articulo ingresado
            strcpy(art_code_base_tmp,data);
            if (!is_turkey_article(art_code_base_tmp, art_no_vale_turkey)) 
              return ARTICLE_NOT_OK;
          }
          else
          {
            // extraer los 6 primeros digitos del codigo barcode ingresado  
            strncpy(art_code_base_tmp,data,6);
            art_code_base_tmp[6]='\0';
          }
      }    
      */
      /**/
	    status=ValeTurkey_is_val_art_no(data,key, 
                                      &article,     &deposit,
                                      &barcd_type,  //&status,
                                      &c_item,0);

      if (status==  ARTICLE_NOT_OK ||
          status==  UNKNOWN_KEY ) return status;

      /*Validate article with 13 digit */
      itoa(c_item.arti.base.art_no,  art_code_base_tmp,10);
      //strcpy(art_code_base_tmp,data);
      if (is_turkey_article(art_code_base_tmp))
      {
        if (!turkey_article_valid(art_code_base_tmp, art_no_vale_turkey)) {
          err_invoke(TURKEY_NOT_APPLICABLE);
          return ARTICLE_NOT_OK;
        }    
      }else{
        err_invoke(ARTICLE_MUSTBE_TURKEY);
        return ARTICLE_NOT_OK;
      }



      /**/
      status=c_item.arti.display_status ;
		  switch (article.type) {
		    case ART_IND_NORMAL:
			    status =  ARTICLE_OK;
			  break;

		    case ART_IND_DEPOSIT:  // 12-Ago-2011 acm ---
			    status= ARTICLE_NOT_OK; // ARTICLE_OK;
	  /* Don't ask price, just take the price from the database.
			  if (barcd_type == BARCD_INTERNAL_EAN13) {
			    c_item.arti.display_status = ARTICLE_OK;
			  }
			  else {
			   c_item.arti.display_status = PRICE_ART;
			  }
	  */
			  break;
		    case ART_IND_PRICE:
			  if (barcd_type == BARCD_INTERNAL_EAN13 ||
				  barcd_type == BARCD_SCALE_EAN13    ||
				  barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) {
				    status= ARTICLE_OK;
			  }
			  else {
			    status = PRICE_ART;
			  }
			  break;
		    case ART_IND_WEIGHT:
			  if (barcd_type == BARCD_INTERNAL_EAN13 ||
				  barcd_type == BARCD_SCALE_EAN13    ||
				  barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) {
			    status =  ARTICLE_OK;
			  }
			  else {
			    if (genvar.price_weight == WEIGHT) {
				  status = WWEIGHT_ART;
			    }
			    else {
				  status = PWEIGHT_ART;
			    }
			  }
			  break;
		    default:
			  status = ARTICLE_NOT_OK; //ARTICLE_NOT_OK;
			  break;
		  }
      c_item.arti.display_status = status;
      //ENTER_KEY
    }else{
      //c_item.arti.display_status = ARTICLE_NOT_OK; //ARTICLE_NOT_OK;
      err_invoke(NOT_VALID_TURKEY_VALE);
      return(UNKNOWN_KEY);
    }
   return status;
   //     return(c_item.arti.display_status);
} /*  */

/* 12-Ago-2011 acm -}*/

/*---------------------------------------------------------------------------*/
/*                             vfy_and_start_artno                           */
/*---------------------------------------------------------------------------*/
short vfy_and_start_artno_turkey(_TCHAR *data, short key)
{
  ARTICLE_DEF article, deposit;
  _TCHAR  artno[16];

  short barcd_type, status;

  /*                                                                       */
  /* Get article and a possible deposit from the PLU.                      */
  /* - data can be an article number or a barcode.                         */
  /* - a possible qty is already in c_item.arti.base.qty.                  */
  /*                                                                       */
  /* Returns:                                                              */
  /*   ARTICLE_NOT_OK if any error.                                        */
  /* else                                                                  */
  /*   ARTICLE_OK     item is complete, can be accepted                    */
  /*   PRICE_ART      price must be entered                                */
  /*   WWEIGHT_ART    weight article, enter weight                         */
  /*   PWEIGHT_ART    weight article, enter price                          */
  /*                                                                       */

  if (STR_ZERO(data)) {
    *data = _T('\0');
    err_invoke(INVALID_ARTNO);
    delete_active_item(empty,0);
    return(ARTICLE_NOT_OK);
  }
  else {
    /*                                                                     */
    /* Step 1: Convert data to an article number.                          */
    /*                                                                     */

    barcd_type = get_barcode_type(data);
    if (barcd_type != BARCD_ILLEGAL) {              /* It's a barcode..    */
      if (barcd_to_artno(artno, data) == SUCCEED) {
        if (barcd_type == BARCD_EXTERNAL_EAN128) {
          _tcsncpy(c_item.arti.base.bar_cd, (data+3), 13);
          c_item.arti.base.bar_cd[14] = _T('\0');
        }
        else {
          if (barcd_type == BARCD_EXTERNAL_DUMP14) {
            _tcsncpy(c_item.arti.base.bar_cd, (data+1), 14);
            c_item.arti.base.bar_cd[15] = _T('\0');
          }
          else {
            _tcscpy(c_item.arti.base.bar_cd, data);
          }
        }
        c_item.arti.base.art_no = _ttol(artno);
      }
      else {
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
    }
    else {                                        /* It's an article no. */
      c_item.arti.base.art_no=_ttol(data);
    }

    /*                                                                   */
    /* Step 2: Get article data from PLU and handle exceptions.          */
    /*                                                                   */
    article.art_no = c_item.arti.base.art_no;
    status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
                           (void*)&article, (short) keyEQL);
    if (status != SUCCEED) {
      err_invoke(INVALID_ARTNO);
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
    }
    else if (article.art_no != c_item.arti.base.art_no) {
      err_invoke(DATA_CORRUPT);                /* article file corrupted */
      delete_active_item(empty,0);             /*  rebuild with stnetp85 */
      return(ARTICLE_NOT_OK);
    }

    /*                                                                   */
    /* Step 2a: No discount allowed on a sold-deposit article.           */
    /*                                                                   */
    if (article.type == ART_IND_DEPOSIT &&
        (state_number() == ST_DISCOUNT ||
         state_number() == ST_DISCOUNT_QTY) ) {
      err_invoke(NO_DISCOUNT_ON_DEPOSIT);
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
    }

    /*                                                                   */
    /* Step 2b: If article is blocked, ask for S-key.                    */
    /*                                                                   */
    if (article.block_ind == YES) {
      if (err_invoke(ARTICLE_BLOCKED) != SUCCEED) {
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
      else {
        c_item.approved = TRUE;
      }
    }

    /*                                                                   */
    /* Step 2c: Put all article information in the current item.         */
    /*                                                                   */
    if (c_item.arti.base.qty == 0.0) {
      if (invoice_line_mode == SALES) {
        c_item.arti.base.qty = 1;
      }
      else {
        c_item.arti.base.qty = -1;
      }
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

    c_item.arti.base.arti_retention_ind     = article.arti_retention_ind    ; //v3.6.1 acm -
    c_item.arti.base.arti_perception_ind    = article.arti_perception_ind   ; //v3.6.1 acm -
    c_item.arti.base.arti_rule_ind          = article.arti_rule_ind         ; //v3.6.1 acm -
    c_item.arti.base.percep_amount          = 0;                            ; //v3.6.1 acm -
    c_item.arti.base.arti_detraccion_ind          = article.arti_detraccion_ind;                            ; //v3.6.2 wjm -
    
    _tcscpy(c_item.arti.base.descr, article.descr);
    _tcscpy(c_item.arti.reg_no, article.reg_no);
    c_item.arti.base.suppl_no       = article.suppl_no;

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
    if (barcd_type == BARCD_INTERNAL_EAN13 ||
        barcd_type == BARCD_SCALE_EAN13    ||
        barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) { /* treated as internal */
      if (decode_internal_ean13_barcode_turkey() != SUCCEED) {
                         /* returns FAIL / SUCCEED / PRICE_TOO_LARGE or  */
                         /* DISCNT_AMNT_TOO_LARGE                        */
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
    }
    else if (calc_gds_value(ART_GROUP) != SUCCEED) {
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
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
        err_invoke(INVALID_ARTNO);
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
      else if (deposit.art_no != article.art_no_deposit) {
        err_invoke(DATA_CORRUPT);               /* article file corrupted  */
        delete_active_item(empty,0);            /*  rebuild with stnetp85  */
        return(ARTICLE_NOT_OK);
      }

      /*                                                                   */
      /* Step 3a: If deposit is blocked, ask for S-key.                    */
      /*                                                                   */
      if (deposit.block_ind == YES) {
        if (err_invoke(ARTICLE_BLOCKED) != SUCCEED) {
          delete_active_item(empty,0);
          return(ARTICLE_NOT_OK);
        }
        else {
          c_item.approved = TRUE;
        }
      }

      /*                                                                   */
      /* Step 3b: Put all deposit information in the current item.         */
      /*                                                                   */
      if (article.type == ART_IND_WEIGHT) {
        if (invoice_line_mode == SALES) {
          c_item.depo.base.qty = 1;
        }
        else {
          c_item.depo.base.qty = -1;
        }
      }
      else {
        c_item.depo.base.qty = c_item.arti.base.qty;
      }

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
      c_item.depo.base.suppl_no       = deposit.suppl_no;

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
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
    } /* if, end of the deposit section. */

    /*                                                                   */
    /* Step 4:  Determine the next state by looking at the article-type  */
    /*          and barcode-type of the article (not the deposit!).      */
    /*                                                                   */
    switch (article.type) {
      case ART_IND_NORMAL:
        c_item.arti.display_status = ARTICLE_OK;
        break;
      case ART_IND_DEPOSIT:
        c_item.arti.display_status = ARTICLE_OK;
/* Don't ask price, just take the price from the database.
        if (barcd_type == BARCD_INTERNAL_EAN13) {
          c_item.arti.display_status = ARTICLE_OK;
        }
        else {
         c_item.arti.display_status = PRICE_ART;
        }
*/
        break;
      case ART_IND_PRICE:
        if (barcd_type == BARCD_INTERNAL_EAN13 ||
            barcd_type == BARCD_SCALE_EAN13    ||
            barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) {
          c_item.arti.display_status = ARTICLE_OK;
        }
        else {
          c_item.arti.display_status = PRICE_ART;
        }
        break;
      case ART_IND_WEIGHT:
        if (barcd_type == BARCD_INTERNAL_EAN13 ||
            barcd_type == BARCD_SCALE_EAN13    ||
            barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) {
          c_item.arti.display_status = ARTICLE_OK;
        }
        else {
          if (genvar.price_weight == WEIGHT) {
            c_item.arti.display_status = WWEIGHT_ART;
          }
          else {
            c_item.arti.display_status = PWEIGHT_ART;
          }
        }
        break;
      default:
        c_item.arti.display_status = ARTICLE_NOT_OK;
        break;
    }

    /*                                                                   */
    /* Step 5: Add the current item to the scroll-list using C_ITEM as   */
    /*         an id.                                                    */
    /*         (These scroll-lines are temporarily. On acceptance, the   */
    /*          lines are replaced with lines with a TM_INDX id. On      */
    /*          canceling, the lines are deleted. Because a possible     */
    /*          call to vfy_qty() already added the scroll-lines, the    */
    /*          scroll-lines are only added if not yet present.)         */
    /*                                                                   */

    if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
      == SCRL_UNKNOWN_LINE) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
    }
    if ((c_item.depo.base.price != 0.0) &&
        (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM)
        == SCRL_UNKNOWN_LINE) ) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM);
    }
    if ((c_item.disc.base.price != 0.0) &&
        (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM)
        == SCRL_UNKNOWN_LINE) ) {             /* Reduced to clear art.   */
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
    }

    view_item(OPER_SCRN | CUST_SCRN);
    return(c_item.arti.display_status);
  }
} /* vfy_and_start_artno_turkey */


/*---------------------------------------------------------------------------*/
/*                             vfy_and_start_artno                           */
/*---------------------------------------------------------------------------*/
short vfy_and_start_artno(_TCHAR *data, short key)
{
  ARTICLE_DEF article, deposit;
  _TCHAR  artno[16];

  short barcd_type, status;

  /*                                                                       */
  /* Get article and a possible deposit from the PLU.                      */
  /* - data can be an article number or a barcode.                         */
  /* - a possible qty is already in c_item.arti.base.qty.                  */
  /*                                                                       */
  /* Returns:                                                              */
  /*   ARTICLE_NOT_OK if any error.                                        */
  /* else                                                                  */
  /*   ARTICLE_OK     item is complete, can be accepted                    */
  /*   PRICE_ART      price must be entered                                */
  /*   WWEIGHT_ART    weight article, enter weight                         */
  /*   PWEIGHT_ART    weight article, enter price                          */
  /*                                                                       */

  if (STR_ZERO(data)) {
    *data = _T('\0');
    err_invoke(INVALID_ARTNO);
    delete_active_item(empty,0);
    return(ARTICLE_NOT_OK);
  }
  else {
    /*                                                                     */
    /* Step 1: Convert data to an article number.                          */
    /*                                                                     */

    barcd_type = get_barcode_type(data);
    if (barcd_type != BARCD_ILLEGAL) {              /* It's a barcode..    */
      if (barcd_to_artno(artno, data) == SUCCEED) {
        if (barcd_type == BARCD_EXTERNAL_EAN128) {
          _tcsncpy(c_item.arti.base.bar_cd, (data+3), 13);
          c_item.arti.base.bar_cd[14] = _T('\0');
        }
        else {
          if (barcd_type == BARCD_EXTERNAL_DUMP14) {
            _tcsncpy(c_item.arti.base.bar_cd, (data+1), 14);
            c_item.arti.base.bar_cd[15] = _T('\0');
          }
          else {
            _tcscpy(c_item.arti.base.bar_cd, data);
          }
        }
        c_item.arti.base.art_no = _ttol(artno);
      }
      else {
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
    }
    else {                                        /* It's an article no. */
      c_item.arti.base.art_no=_ttol(data);
    }

    /*                                                                   */
    /* Step 2: Get article data from PLU and handle exceptions.          */
    /*                                                                   */
    article.art_no = c_item.arti.base.art_no;
    status = pos_get_rec(ARTI_TYPE, POS_ARTI_SIZE, ARTICLE_IDX,
                           (void*)&article, (short) keyEQL);
    if (status != SUCCEED) {
      err_invoke(INVALID_ARTNO);
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
    }
    else if (article.art_no != c_item.arti.base.art_no) {
      err_invoke(DATA_CORRUPT);                /* article file corrupted */
      delete_active_item(empty,0);             /*  rebuild with stnetp85 */
      return(ARTICLE_NOT_OK);
    }

    /*                                                                   */
    /* Step 2a: No discount allowed on a sold-deposit article.           */
    /*                                                                   */
    if (article.type == ART_IND_DEPOSIT &&
        (state_number() == ST_DISCOUNT ||
         state_number() == ST_DISCOUNT_QTY) ) {
      err_invoke(NO_DISCOUNT_ON_DEPOSIT);
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
    }

    /*                                                                   */
    /* Step 2b: If article is blocked, ask for S-key.                    */
    /*                                                                   */
    if (article.block_ind == YES) {
      if (err_invoke(ARTICLE_BLOCKED) != SUCCEED) {
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
      else {
        c_item.approved = TRUE;
      }
    }

    /*                                                                   */
    /* Step 2c: Put all article information in the current item.         */
    /*                                                                   */
    if (c_item.arti.base.qty == 0.0) {
      if (invoice_line_mode == SALES) {
        c_item.arti.base.qty = 1;
      }
      else {
        c_item.arti.base.qty = -1;
      }
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

    c_item.arti.base.arti_retention_ind     = article.arti_retention_ind    ; //v3.6.1 acm -
    c_item.arti.base.arti_perception_ind    = article.arti_perception_ind   ; //v3.6.1 acm -
    c_item.arti.base.arti_rule_ind          = article.arti_rule_ind         ; //v3.6.1 acm -
    c_item.arti.base.percep_amount          = 0;                            ; //v3.6.1 acm -
    
    c_item.arti.base.arti_detraccion_ind          = article.arti_detraccion_ind;                            ; //v3.6.2 wjm -
    
    _tcscpy(c_item.arti.base.descr, article.descr);
    _tcscpy(c_item.arti.reg_no, article.reg_no);
    c_item.arti.base.suppl_no       = article.suppl_no;

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
    if (barcd_type == BARCD_INTERNAL_EAN13 ||
        barcd_type == BARCD_SCALE_EAN13    ||
        barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) { /* treated as internal */
      if (decode_internal_ean13_barcode() != SUCCEED) {
                         /* returns FAIL / SUCCEED / PRICE_TOO_LARGE or  */
                         /* DISCNT_AMNT_TOO_LARGE                        */
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
    }
    else if (calc_gds_value(ART_GROUP) != SUCCEED) {
      delete_active_item(empty,0);
      return(ARTICLE_NOT_OK);
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
        err_invoke(INVALID_ARTNO);
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
      else if (deposit.art_no != article.art_no_deposit) {
        err_invoke(DATA_CORRUPT);               /* article file corrupted  */
        delete_active_item(empty,0);            /*  rebuild with stnetp85  */
        return(ARTICLE_NOT_OK);
      }

      /*                                                                   */
      /* Step 3a: If deposit is blocked, ask for S-key.                    */
      /*                                                                   */
      if (deposit.block_ind == YES) {
        if (err_invoke(ARTICLE_BLOCKED) != SUCCEED) {
          delete_active_item(empty,0);
          return(ARTICLE_NOT_OK);
        }
        else {
          c_item.approved = TRUE;
        }
      }

      /*                                                                   */
      /* Step 3b: Put all deposit information in the current item.         */
      /*                                                                   */
      if (article.type == ART_IND_WEIGHT) {
        if (invoice_line_mode == SALES) {
          c_item.depo.base.qty = 1;
        }
        else {
          c_item.depo.base.qty = -1;
        }
      }
      else {
        c_item.depo.base.qty = c_item.arti.base.qty;
      }

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
      c_item.depo.base.suppl_no       = deposit.suppl_no;

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
        delete_active_item(empty,0);
        return(ARTICLE_NOT_OK);
      }
    } /* if, end of the deposit section. */

    /*                                                                   */
    /* Step 4:  Determine the next state by looking at the article-type  */
    /*          and barcode-type of the article (not the deposit!).      */
    /*                                                                   */
    switch (article.type) {
      case ART_IND_NORMAL:
        c_item.arti.display_status = ARTICLE_OK;
        break;
      case ART_IND_DEPOSIT:
        c_item.arti.display_status = ARTICLE_OK;
/* Don't ask price, just take the price from the database.
        if (barcd_type == BARCD_INTERNAL_EAN13) {
          c_item.arti.display_status = ARTICLE_OK;
        }
        else {
         c_item.arti.display_status = PRICE_ART;
        }
*/
        break;
      case ART_IND_PRICE:
        if (barcd_type == BARCD_INTERNAL_EAN13 ||
            barcd_type == BARCD_SCALE_EAN13    ||
            barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) {
          c_item.arti.display_status = ARTICLE_OK;
        }
        else {
          c_item.arti.display_status = PRICE_ART;
        }
        break;
      case ART_IND_WEIGHT:
        if (barcd_type == BARCD_INTERNAL_EAN13 ||
            barcd_type == BARCD_SCALE_EAN13    ||
            barcd_type == BARCD_EXTERNAL_WEIGHT_EAN13) {
          c_item.arti.display_status = ARTICLE_OK;
        }
        else {
          if (genvar.price_weight == WEIGHT) {
            c_item.arti.display_status = WWEIGHT_ART;
          }
          else {
            c_item.arti.display_status = PWEIGHT_ART;
          }
        }
        break;
      default:
        c_item.arti.display_status = ARTICLE_NOT_OK;
        break;
    }

    /*                                                                   */
    /* Step 5: Add the current item to the scroll-list using C_ITEM as   */
    /*         an id.                                                    */
    /*         (These scroll-lines are temporarily. On acceptance, the   */
    /*          lines are replaced with lines with a TM_INDX id. On      */
    /*          canceling, the lines are deleted. Because a possible     */
    /*          call to vfy_qty() already added the scroll-lines, the    */
    /*          scroll-lines are only added if not yet present.)         */
    /*                                                                   */

    if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
      == SCRL_UNKNOWN_LINE) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
    }
    if ((c_item.depo.base.price != 0.0) &&
        (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM)
        == SCRL_UNKNOWN_LINE) ) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM);
    }
    if ((c_item.disc.base.price != 0.0) &&
        (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM)
        == SCRL_UNKNOWN_LINE) ) {             /* Reduced to clear art.   */
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
    }

    view_item(OPER_SCRN | CUST_SCRN);
    return(c_item.arti.display_status);
  }
} /* vfy_and_start_artno */


#ifdef PRN_IMM
/*---------------------------------------------------------------------------*/
/*                                print_item                                 */
/*---------------------------------------------------------------------------*/
void print_item(void)
{
  short i;
  short group[] = { ART_GROUP, DEPOSIT_GROUP, DISCNT_GROUP, 0 };

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
} /* print_item */
#endif

/*---------------------------------------------------------------------------*/
/*                       recalc_total_carried_forward                        */
/*---------------------------------------------------------------------------*/
short recalc_total_carried_forward(ITEM_SNAM group_name)
{
  SUB_ITEM_TOTAL sub_item_totals;

  /*                                                                       */
  /* Recalculate total-carried forward after printing the line. Allways    */
  /* use the inclusive amount!                                             */
  /*                                                                       */

  calc_sub_item_totals(C_ITEM, group_name, &sub_item_totals);
  if(genvar.price_incl_vat == INCLUSIVE) {
    tot_add_double((short)(TOT_CARR_FORWD_INCL_0+sub_item_totals.vat_no), sub_item_totals.incl_gds_val);
  }
  else {
    tot_add_double((short)(TOT_CARR_FORWD_EXCL_0+sub_item_totals.vat_no), sub_item_totals.excl_gds_val);
  }

  /* Now all the inclusive or exclusive totals are correct. */
  /* We have to recalc all other totals.                    */
  calc_incl_excl_vat_tcf();

  return(SUCCEED);
} /* recalc_total_carried_forward */

/*---------------------------------------------------------------------------*/
/*                          calc_incl_excl_vat_tcf                           */
/*---------------------------------------------------------------------------*/
void calc_incl_excl_vat_tcf(void)
{
  short i;

  tot_reset_double(TOT_CARR_FORWD_INCL);
  tot_reset_double(TOT_CARR_FORWD_EXCL);
  tot_reset_double(TOT_CARR_FORWD_VAT);
  tot_reset_double(TOT_CARR_FMSAM_INCL);
  tot_reset_double(TOT_CARR_FMSAM_EXCL);
  tot_reset_double(TOT_CARR_FMSAM_VAT);

  if (genvar.price_incl_vat==INCLUSIVE) {
    for (i=0; i<=9; i++) {
      tot_add_double(TOT_CARR_FORWD_INCL, tot_ret_double((short)(TOT_CARR_FORWD_INCL_0+i)));
      tot_add_double(TOT_CARR_FMSAM_INCL, tot_ret_double((short)(TOT_CARR_FMSAM_INCL_0+i)));
      tot_reset_double((short)(TOT_CARR_FORWD_EXCL_0+i));
      tot_add_double((short)(TOT_CARR_FORWD_EXCL_0+i), floor_price(calc_excl_vat(
                          tot_ret_double((short)(TOT_CARR_FORWD_INCL_0+i)), i)));
      tot_add_double((short)(TOT_CARR_FORWD_EXCL), tot_ret_double((short)(TOT_CARR_FORWD_EXCL_0+i)));
      tot_reset_double((short)(TOT_CARR_FMSAM_EXCL_0+i));
      tot_add_double((short)(TOT_CARR_FMSAM_EXCL_0+i), floor_price(calc_excl_vat(
                          tot_ret_double((short)(TOT_CARR_FMSAM_INCL_0+i)), i)));
      tot_add_double((short)(TOT_CARR_FMSAM_EXCL), tot_ret_double((short)(TOT_CARR_FMSAM_EXCL_0+i)));
    }
  }
  else {
    for (i=0; i<=9; i++) {
      tot_add_double(TOT_CARR_FORWD_EXCL, tot_ret_double((short)(TOT_CARR_FORWD_EXCL_0+i)));
      tot_add_double(TOT_CARR_FMSAM_EXCL, tot_ret_double((short)(TOT_CARR_FMSAM_EXCL_0+i)));
      tot_reset_double((short)(TOT_CARR_FORWD_INCL_0+i));
      tot_add_double((short)(TOT_CARR_FORWD_INCL_0+i), floor_price(calc_incl_vat(
                          tot_ret_double((short)(TOT_CARR_FORWD_EXCL_0+i)), i)));
      tot_add_double((short)(TOT_CARR_FORWD_INCL), tot_ret_double((short)(TOT_CARR_FORWD_INCL_0+i)));
      tot_reset_double((short)(TOT_CARR_FMSAM_INCL_0+i));
      tot_add_double((short)(TOT_CARR_FMSAM_INCL_0+i), floor_price(calc_incl_vat(
                          tot_ret_double((short)(TOT_CARR_FMSAM_EXCL_0+i)), i)));
      tot_add_double((short)(TOT_CARR_FMSAM_INCL), tot_ret_double((short)(TOT_CARR_FMSAM_INCL_0+i)));
    }
  }

  for(i=0; i<=9; i++) {
    tot_reset_double((short)(TOT_CARR_FORWD_VAT_0+i));
    tot_add_double((short)(TOT_CARR_FORWD_VAT_0+i),
                     tot_ret_double((short)(TOT_CARR_FORWD_INCL_0+i))
                      - tot_ret_double((short)(TOT_CARR_FORWD_EXCL_0+i)));
    tot_add_double((short)(TOT_CARR_FORWD_VAT), tot_ret_double((short)(TOT_CARR_FORWD_VAT_0+i)));
    tot_reset_double((short)(TOT_CARR_FMSAM_VAT_0+i));
    tot_add_double((short)(TOT_CARR_FMSAM_VAT_0+i),
                     tot_ret_double((short)(TOT_CARR_FMSAM_INCL_0+i))
                      - tot_ret_double((short)(TOT_CARR_FMSAM_EXCL_0+i)));
    tot_add_double((short)(TOT_CARR_FMSAM_VAT), tot_ret_double((short)(TOT_CARR_FMSAM_VAT_0+i)));

  }
} /* calc_incl_excl_vat_tcf */

/*---------------------------------------------------------------------------*/
/*                             view_item                                     */
/*---------------------------------------------------------------------------*/
void view_item(short screen)
{
  TM_ARTI_BASE *base;
  short art_ind, display_status;
  unsigned short curr_scrn;
  _TCHAR buffer[36], descr_gds_val[41], qty[10], price[19], gds_value[19];

  /*                                                                       */
  /* Display 'item_indx' on 'screen'.                                      */
  /*                                                                       */

  if (c_item.arti.base.price!=0.0) {

    /* If there is discount information, display the discount              */
    /* instead of the article.                                             */

    base=&c_item.arti.base;
    art_ind=c_item.arti.art_ind;
    display_status=c_item.arti.display_status;
    if (display_status==0) {
      display_status=ARTICLE_OK;
    }
    if (c_item.disc.base.price!=0.0) {
      base=&c_item.disc.base;
      art_ind=ART_IND_NORMAL;
      display_status=ARTICLE_OK;
    }

    /* Build a description + goods value string to display.                */
    ch_memset(descr_gds_val, _T(' '), sizeof(descr_gds_val));
    strcpy_no_null(descr_gds_val, base->descr);
    descr_gds_val[39] = _T('\0');

    if (display_status==WWEIGHT_ART || display_status==PWEIGHT_ART) {
      _tcscpy(qty,empty);
    }
    else {
      ftoa(base->qty, 9, qty);
    }

    if (display_status!=ARTICLE_OK) {
      _tcscpy(gds_value,empty);
    }
    else {
      ftoa_price(base->goods_value, 18, gds_value);
    }

    if (display_status==PRICE_ART) {
      _tcscpy(price,empty);
    }
    else {
      ftoa_price(base->price, 18, price);
    }

    format_string(&string_price11_2, gds_value);
    *buffer=_T(' ');                  /* Space between description and price.  */
    _tcscpy(buffer+1, string_price11_2.result);
    _tcscpy(&descr_gds_val[40-_tcslen(buffer)], buffer);
    curr_scrn=scrn_get_current_window();
    scrn_select_window(INV_ART_DESCR_WINDOW);
    scrn_string_out(descr_gds_val,0,0);
    scrn_select_window(curr_scrn);

    if ((screen & CUST_SCRN) && (!subt_display) ) {
      descr_gds_val[19]=_T('\0');
      cdsp_write_string(descr_gds_val,0,0);
      ch_memset(descr_gds_val, _T(' '), 20*sizeof(_TCHAR));
      if (art_ind == ART_IND_WEIGHT) {
        format_string(&string_price11, gds_value);
        _tcscpy(&descr_gds_val[20-_tcslen(string_price11.result)], string_price11.result);
        cdsp_write_string(descr_gds_val,1,0);
      }
      else {
        format_string(&string_qty4,qty);
        strcpy_no_null(descr_gds_val, string_qty4.result);
        strcpy_no_null(descr_gds_val+5, cdsp_TXT[10]);      /* 'x'         */
        if (STR_LESS_ZERO(qty)) {
          reverse_sign(price);
        }
        format_string(&string_price8,price);
        _tcscpy(&descr_gds_val[20-_tcslen(string_price8.result)], string_price8.result);
        cdsp_write_string(descr_gds_val,1,0);
      }
    }
  }
  else if ((screen & CUST_SCRN) && (!subt_display) ) {
    cdsp_clear();
  }
} /* view_item */


/*---------------------------------------------------------------------------*/
/*                             proc_active_item                              */
/*---------------------------------------------------------------------------*/
short proc_active_item(void)
{
  double item_total;
  SUB_ITEM_TOTAL sub_item_totals[3];
  short group[] = { ART_GROUP, DEPOSIT_GROUP, DISCNT_GROUP, 0 };
  short i, mul_tot=1;

  /*                                                                       */
  /* Recalculate totals using the current item.                            */
  /*                                                                       */
  /*  - For each sub-item within the active-item, calculate:               */
  /*    . goods-value in- and exclusive,                                   */
  /*    . vat amount                                                       */
  /*    . vat-code                                                         */
  /*    . number of packs                                                  */
  /*                                                                       */

  /*                                                                       */
  /* First look for price/total overflow!                                  */
  /*                                                                       */
  item_total=0.0;
  for (i=0; group[i]; i++) {
    calc_sub_item_totals(C_ITEM, group[i], &sub_item_totals[i]);
    item_total+=sub_item_totals[i].incl_gds_val;
  }
  if ( genvar.ind_price_dec == DECIMALS_YES ) {
    mul_tot = 100;
  }

  if ((double)fabs( mul_tot * (item_total + tot_ret_double(TOT_GEN_INCL)) )
                    <= MAX_AMOUNT_ON_POS) {
    for (i=0; group[i]; i++) {
      if (sub_item_totals[i].incl_gds_val != 0.0) {
        recalc_totals(&sub_item_totals[i]);
      }
    }
    return(SUCCEED);
  }
  else {
    err_invoke(PRICE_TOO_LARGE);
    delete_active_item(empty,0);
    return(PRICE_TOO_LARGE);
  }
} /* proc_active_item */


/*---------------------------------------------------------------------------*/
/*                           accept_active_item                              */
/*---------------------------------------------------------------------------*/
short accept_active_item(_TCHAR *data, short key)
{
  TM_INDX indx;

  /*                                                                       */
  /* Accept active_item.                                                   */
  /*                                                                       */
  /*  Previously added C_ITEM type scroll-lines are modified to the new    */
  /*  TM_INDX retrieved from the tm_appe() call.                           */
  /*                                                                       */
printf_log("accept_active_item");
  if (proc_active_item() == SUCCEED) {

    /*                                                                     */
    /* The approval section in the state-engine for the state Invoicing    */
    /* ensures that not more than TM_ITEM_MAXI-1 items are invoiced.       */
    /* So, do not check the indx for a TM_BOF_EOF error.                   */
    /*                                                                     */

    indx=tm_appe(TM_ITEM_NAME, (void*)&c_item);

    scrl_modify_id(SCRL_INV_DISCNT_LINE,  C_ITEM, indx);
    scrl_modify_id(SCRL_INV_DEPOSIT_LINE, C_ITEM, indx);
    scrl_modify_id(SCRL_INV_ART_LINE,     C_ITEM, indx);

    view_item(OPER_SCRN | CUST_SCRN);

#ifdef PRN_IMM
    print_item();
#endif

    ++nbr_inv_lines;
    ++nbr_log_lines;
    if (c_item.depo.base.goods_value != 0.0) {  /* Also count deposits     */
      ++nbr_log_lines;
    }

    last_item   =indx;
    display_item=indx;
    return(key);
  }
  else {                                            /* Price too large.    */
    *data=_T('\0');
    return(PRICE_TOO_LARGE_KEY);
  }
} /* accept_active_item */

/* 12-Ago-2011 acm -{*/
short accept_active_item_valeturkey(_TCHAR *data, short key) // acm ---
{
  
  int status=0;

  status = accept_active_item(data, key);
  if (key==status)
  {
    init_item();
    memcpy_item(&c_item, &c_item_valeturkey);
    c_item.arti.display_status=ARTICLE_OK;



    if (calc_gds_value(ART_GROUP)==PRICE_TOO_LARGE) {
      status=UNKNOWN_KEY;                     /* Value overflow.           */
    }
    else {
      status=key;
      if (fabs(fabs(c_item.arti.base.goods_value)-fabs(atof(cur_valepavo.sell_pr)))<1.0)
      {  
          if   (c_item.arti.base.goods_value>0 )
                c_item.arti.base.goods_value= atof(cur_valepavo.sell_pr);  // fix valepavo
          else
             c_item.arti.base.goods_value= -1.0*atof(cur_valepavo.sell_pr);  // fix valepavo
      }
      //c_item.  
      //;

      c_item.arti.display_status=ARTICLE_OK;

	    if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
			  == SCRL_UNKNOWN_LINE) {
			  scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
		  }
      view_item(OPER_SCRN | CUST_SCRN);
    }
    status = accept_active_item(data, key);
  }
  return status;

} /* accept_active_item_valeturkey */

/* 12-Ago-2011 acm -  }*/


/*---------------------------------------------------------------------------*/
/*                           delete_active_item                              */
/*---------------------------------------------------------------------------*/
short delete_active_item(_TCHAR *data, short key)
{
  /*                                                                       */
  /* The active item is deleted from the screens.                          */
  /*                                                                       */
  /*   Totals etc. are not recalculated(!) because the active_item is not  */
  /*   yet accepted!                                                       */
  /*                                                                       */

  scrl_delete_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
  scrl_delete_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM);
  scrl_delete_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
  scrn_clear_window(INV_ART_DESCR_WINDOW);
  cdsp_clear();
  init_item();
  display_item= TM_ROOT;
  last_item   = TM_ROOT;

  return(key);
} /* delete_active_item */


/*---------------------------------------------------------------------------*/
/*                           vfy_last_item_present                              */
/*---------------------------------------------------------------------------*/
static short vfy_last_item_present(_TCHAR *data, short key)
{
  /* Check to see if a last item is present. The global 'last_item' must   */
  /* point to an item-group and not TM_ROOT.                               */
  /*                                                                       */
  *data=_T('\0');
  if (last_item==TM_ROOT) {
    err_invoke(NO_LAST_ITEM);
    return(UNKNOWN_KEY);
  }

  return(key);
} /* vfy_last_item_present */


/*---------------------------------------------------------------------------*/
/*                           repeat_last_item                                */
/*---------------------------------------------------------------------------*/
static short repeat_last_item(_TCHAR *data, short key)
{
  /*                                                                         */
  /* Repeat the last entered item.                                           */
  /*                                                                         */
  /* The function vfy_void_last_item() has checked if there is a last        */
  /* item which can be repeated, now repeat the last item                    */
  /*                                                                         */

  *data=_T('\0');

  if (last_item >= 0) {
    tm_read_nth(TM_ITEM_NAME, (void*)&c_item, last_item);
    if (c_item.approved == TRUE && err_invoke(APPROVED_ITEM) != SUCCEED) {
      return(UNKNOWN_KEY);                        /* Cancel the repeat.    */
    }
    if (c_item.arti.base.price != 0.0) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
    }
    if (c_item.depo.base.price != 0.0) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM);
    }
    if (c_item.disc.base.price != 0.0) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
    }
    accept_active_item(data, key);
  }

  return(UNKNOWN_KEY);
} /* repeat_last_item */


/*---------------------------------------------------------------------------*/
/*                           toggle_subt_display                             */
/*---------------------------------------------------------------------------*/
static short toggle_subt_display(_TCHAR *data, short key)
{
  TM_ITEM_GROUP h_item;

  subt_display=!subt_display;
  if (subt_display) {
    view_total(CUST_SCRN);                /* Display subtotal.             */
  }
  else {
    if (display_item != TM_ROOT && display_item != C_ITEM) {
      memcpy(&h_item, &c_item, sizeof(TM_ITEM_GROUP));
      tm_read_nth(TM_ITEM_NAME, (void*)&c_item, display_item);
      view_item(CUST_SCRN);
      memcpy(&c_item, &h_item, sizeof(TM_ITEM_GROUP));
    }
    else {
      view_item(CUST_SCRN);
    }
  }

  return(UNKNOWN_KEY);
} /* toggle_subt_display */


/*---------------------------------------------------------------------------*/
/*                           vfy_total_key                                   */
/*---------------------------------------------------------------------------*/
 short vfy_total_key(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Check for any invoiced article.                                       */
  /*                                                                       */

  *data=_T('\0');

  if (nbr_inv_lines == 0 || tot_ret_double(TOT_GEN_INCL) == 0.0) { /*02-dec-92 ASu*/
    if( pos_invoice.invoice_fee_amount == 0.0) {
      err_invoke(TOTAL_NOT_POSSIBLE);
      return(UNKNOWN_KEY);
    }
  }

  return(key);
} /* vfy_total_key */

/*---------------------------------------------------------------------------*/
/*                          process_invoice                                  */
/*---------------------------------------------------------------------------*/
short process_invoice(_TCHAR *data, short key)
{
  /* 12-Ago-2011 acm -{*/
  int valeturkey_amount=0;
  int status=0;
  _TCHAR data_payment[100];
  int vigencia_date;
  double amount__=0;
  /* Read amount from vale turkey */

  amount__=get_valeturkey_amount()*100.00;
  valeturkey_amount = (int)(amount__+0.001); /*(int)*/ /* 27-April-2012 acm - fix */  /* 16-Dic-2012 acm - fix */
  itoa(valeturkey_amount,data_payment,10); 
  /* 12-Ago-2011 acm -}*/

  multisam_discounts(FALSE);

 /* v3.6.1 acm - {*/ 
   pos_invoice.invoice_percep_amount = 0;
   pos_invoice.invoice_percep_custdoc[0]  = 0;    /* v3.6.1 acm - */
   pos_invoice.invoice_percep_custname[0] = 0;    /* v3.6.1 acm - */
   
   pos_invoice.invoice_serie[0] = 0;  		//mlsd FE
   pos_invoice.invoice_correlative = 0;		//mlsd FE

   IS_GEN_PERCEPTION=is_PERCEPCION( &vigencia_date, &TOT_GEN_PERCEPTION, &PERC_GEN_PERCEPTION) ;
   if (IS_GEN_PERCEPTION)
   {
        TOT_GEN_PERCEPTION                  = floor_price(TOT_GEN_PERCEPTION) ;
        pos_invoice.invoice_percep_amount   = TOT_GEN_PERCEPTION;
   }
  /* v3.6.1 acm - }*/ 

  if ( tot_ret_double(TOT_GEN_INCL)
       + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL)) <= 0.0 ) {

      return Prev_Amnt_Enought_init(data,AMNT_ENOUGH);

    //return(AMNT_ENOUGH);                                   /* Subtotal <= 0  */
  }

  sll_init(&voucher_anticipo_hist, sizeof(VOUCHER_ANTICIPO_DEF));            /* initialize vouchers history */
  sll_init(&voucher_hist, sizeof(VOUCHER_DEF));            /* initialize vouchers history */
  sll_init(&payment_items, sizeof(INVOICE_PAYM_ITEM_DEF)); /* initialize payment_item     */

 
  /* 12-Ago-2011 acm -{*/
  if (valeturkey_press_key&&(valeturkey_amount>0))
  {       
    
    //fixxxx fixxxx fix2014  
    setpayment_curr_type(3);//acm -++++
  	copy_invoice    = NO;                    /* Reset copy_invoice.           */ //acm -++++
	status = vfy_paymentway(data_payment, PAYMENT_WAY_3_KEY); 
    if (status==AMNT_ENOUGH)
       return Prev_Amnt_Enought_init(data,AMNT_ENOUGH);
  }
 /* 12-Ago-2011 acm -}*/
  key=Prev_DoTotal_ST_init(data,key);
  return(key);                                             /* Subtotal > 0   */
} /* process_invoice */

/*---------------------------------------------------------------------------*/
/*                           vfy_item_present                                */
/*---------------------------------------------------------------------------*/
static short vfy_item_present(_TCHAR *data, short key)
{
  if (nbr_inv_lines > 0) {
    return(key);
  }

  err_invoke(NO_ITEMS_TO_VOID);
  return(UNKNOWN_KEY);
} /* vfy_item_present */


/*---------------------------------------------------------------------------*/
/*                          recalc_subtotals                                 */
/*---------------------------------------------------------------------------*/
short recalc_subtotals(void)
{
  short i;
  double tot_excl, tot_incl, tot_vat, vat_perc;
  double tot_gen_excl=0, tot_gen_vat=0;

  /*                                                                       */
  /* Recalculate (sub)totals before printing (sub)totals. This is done     */
  /* to avoid incorrect roundings.                                         */
  /*                                                                       */

  if (genvar.price_incl_vat==INCLUSIVE) {
    for (i=0; i<=9; i++) {
      tot_incl     = tot_ret_double((short)(TOT_INCL_0+i));
      vat_perc     = get_vat_perc(i);
      tot_vat      = vat_perc / (100 + vat_perc) * tot_incl;
      tot_vat      = floor_price(tot_vat);
      tot_gen_vat +=tot_vat;
      tot_excl     = tot_incl - tot_vat;
      tot_gen_excl+=tot_excl;
      tot_reset_double((short)(TOT_EXCL_0+i));
      tot_add_double((short)(TOT_EXCL_0+i), tot_excl);
      tot_reset_double((short)(TOT_VAT_0+i));
      tot_add_double((short)(TOT_VAT_0+i), tot_vat);
    }

    tot_reset_double(TOT_GEN_EXCL);
    tot_add_double(TOT_GEN_EXCL, tot_gen_excl);

    tot_reset_double(TOT_GEN_VAT);
    tot_add_double(TOT_GEN_VAT, tot_gen_vat);

    for (i=0; i<=9; i++) {
      tot_incl = tot_ret_double((short)(SUB_TOT_INCL_0+i));
      vat_perc = get_vat_perc(i);
      tot_vat  = vat_perc / (100 + vat_perc) * tot_incl;
      tot_vat  = floor_price(tot_vat);
      tot_excl = tot_incl - tot_vat;
      tot_reset_double((short)(SUB_TOT_EXCL_0+i));
      tot_add_double((short)(SUB_TOT_EXCL_0+i), tot_excl);
      tot_reset_double((short)(SUB_TOT_VAT_0+i));
      tot_add_double((short)(SUB_TOT_VAT_0+i), tot_vat);
    }
  }

  return(SUCCEED);
} /* recalc_subtotals */


#ifdef PRN_IMM
/*---------------------------------------------------------------------------*/
/*                             proc_new_page                                 */
/*---------------------------------------------------------------------------*/
static short proc_new_page(_TCHAR *data, short key)
{
  short i;

  /*                                                                       */
  /* Recalculate VAT's, print and clear subtotals.                         */
  /*                                                                       */

  recalc_subtotals();
  if (GetPrinterSize(selected_printer)!=PRINTER_SIZE_SMALL) {
    bp_now(BP_INVOICE, BP_SUBTOTAL_LINES, 0);
  }

  /* And reset the sub-totals.                                             */
  for(i=0; i<=9; i++) {
    tot_reset_double(SUB_TOT_EXCL_0+i);
    tot_reset_double(SUB_TOT_INCL_0+i);
    tot_reset_double(SUB_TOT_VAT_0 +i);
  }
  tot_reset_double(SUB_TOT_GEN_INCL);
  tot_reset_double(TOT_SUB_PACKS);

  return(UNKNOWN_KEY);
} /* proc_new_page */
#endif


/*---------------------------------------------------------------------------*/
/*                             proc_new_line                                 */
/*---------------------------------------------------------------------------*/
short proc_new_line(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Skip one line on the invoice, print trailer- and header if page-skip  */
  /* is encountered.                                                       */
  /*                                                                       */

  if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
    bp_now(BP_SMALL_INVOICE, BP_SKIP_ONE_LINE, 0);
  }
  else {
    bp_now(BP_INVOICE, BP_SKIP_ONE_LINE, 0);
  }

  return(UNKNOWN_KEY);
} /* proc_new_line */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   INVOICING QTY                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  At this point, an item is created and the QTY_LEAF is filled.            */
/*  In the scrolling window of OPER-SCRN the QTY_LEAF is displayed.          */
/*  In this state, the quantity can be changed, the item can                 */
/*  be deleted and an article-number or barcode can be entered.              */
/*  It is not possible to enter the Credit (30) or Discount (40) state.      */
/*                                                                           */
/*---------------------------------------------------------------------------*/


static void InvoicingQty_VW(void)
{
  static short cls_states[]={
     ST_START_ART_FIND
    ,ST_ART_FIND_RESULT
    ,0
  };

  if (called_by_state(cls_states)==SUCCEED) {
    cls();
    InvoicingHead_VW();
    InvoicingDescr_VW();
    InvoicingScrl_VW();
    view_total( OPER_SCRN | CUST_SCRN );
    InvoicingSubt_VW();
  }

  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[4],0,16); /* ENTER ITEM / QTY OR PRESS<NO> */
} /* InvoicingQty_VW */


static VERIFY_ELEMENT InvoicingQty_VFY[] =
{
  LINE_UP_KEY,     handle_scroll_key,
  LINE_DOWN_KEY,   handle_scroll_key,
  PAGE_UP_KEY,     handle_scroll_key,
  PAGE_DOWN_KEY,   handle_scroll_key,
  NO_KEY,          (void *)NULL,
  TIMES_KEY,       vfy_art_qty,
  ENTER_KEY,       vfy_and_start_artno,
  OCIA1_DATA,      vfy_and_start_artno,
  OCIA2_DATA,      vfy_and_start_artno,
  CLEAR_KEY,       vfy_clear_key,
  OPEN_DRAWER_KEY, open_and_close_drawer,  /* Approval in state-engine.     */
  ART_FINDER_KEY,  (void *)NULL,
  UNKNOWN_KEY,     illegal_fn_key
};


/* extern INPUT_CONTROLLER Dartno14KO14n, defined in previous state          */


static PROCESS_ELEMENT InvoicingQty_PROC[] =
{
  ARTICLE_OK,   accept_active_item,
  NO_KEY,       delete_active_item,
  UNKNOWN_KEY,  (void *)NULL
};


static CONTROL_ELEMENT InvoicingQty_CTL[] =
{
  ARTICLE_OK,          &Invoicing_ST,
  ARTICLE_NOT_OK,      &Invoicing_ST,
  NO_KEY,              &Invoicing_ST,
  TIMES_KEY,           &InvoicingQty_ST,
  PRICE_ART,           &NPriceArt_ST,
  WWEIGHT_ART,         &NWWeightArt_ST,
  PWEIGHT_ART,         &NPWeightArt_ST,
  PRICE_TOO_LARGE_KEY, &Invoicing_ST,
  ART_FINDER_KEY,      &StartArtFind_ST,
  UNKNOWN_KEY,         &InvoicingQty_ST
};


extern STATE_OBJ InvoicingQty_ST =
{
  ST_INVOICING_QTY,
  InvoicingQty_VW,
  art_no_DFLT,
  &Dartno14KO14n,
  InvoicingQty_VFY,
  Input_UVW,
  InvoicingQty_PROC,
  InvoicingQty_CTL
};


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   NORMAL PRICE ARTICLE                                    */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  At this point, an item is created and the ARTICLE_GROUP is filled with   */
/*  data from the Store Controler.                                           */
/*  In the scrolling window of OPER-SCRN the article data is displayed.      */
/*  In this state, price articles are handled.                               */
/*  Pressing NO will cause an item-deletion from screen and tdm.             */
/*                                                                           */
/*---------------------------------------------------------------------------*/


static void NPriceArt_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[5],0,12); /* ENTER PRICE 1X                */
} /* NPriceArt_VW */


/*---------------------------------------------------------------------------*/
/*                               vfy_art_price                               */
/*---------------------------------------------------------------------------*/
short vfy_art_price(_TCHAR *data, short key)
{
  no_leading_zeros(data);

  if (!STR_ZERO(data)) {
    /*                                                                     */
    /* Price is a non-zero value, the price entered is in- or exclusive    */
    /* vat depending on genvar.price_incl_vat.                             */
    /*                                                                     */

    c_item.arti.base.price=atof_price(data);

    if (calc_gds_value(ART_GROUP)==PRICE_TOO_LARGE) {
      *data=_T('\0');
      return(UNKNOWN_KEY);                        /* Value overflow        */
    }
    else if (genvar.ind_price_dec!=DECIMALS_YES && _tcslen(data) > 7) {
      err_invoke(VALUE_TOO_LARGE);
      *data=_T('\0');
      return(UNKNOWN_KEY);
    }
    c_item.arti.display_status=ARTICLE_OK;
    scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
    view_item(OPER_SCRN | CUST_SCRN);
    return(key);
  }
  err_invoke(ZERO_NOT_LEGAL_ERROR);

  return(UNKNOWN_KEY);
} /* vfy_art_price */


static VERIFY_ELEMENT NPriceArt_VFY[] =
{
  LINE_UP_KEY,     handle_scroll_key,
  LINE_DOWN_KEY,   handle_scroll_key,
  PAGE_UP_KEY,     handle_scroll_key,
  PAGE_DOWN_KEY,   handle_scroll_key,
  NO_KEY,          (void *)NULL,           /* Delete current item, back to  */
  ENTER_KEY,       vfy_art_price,          /* returns UNKNOWN_KEY or ENTER  */
  OCIA1_DATA,      vfy_ocia_not_legal,     /* err_invoke() + UNKNOWN_KEY    */
  OCIA2_DATA,      vfy_ocia_not_legal,     /* err_invoke() + UNKNOWN_KEY    */
  CLEAR_KEY,       vfy_clear_key,
  OPEN_DRAWER_KEY, open_and_close_drawer,  /* Approval in state-engine.     */
  UNKNOWN_KEY,     illegal_fn_key
};


extern INPUT_CONTROLLER Dprice10K10nd =
{
  (INPUT_DISPLAY *)&dsp_price10,
  KEYBOARD_MASK | OCIA1_MASK | OCIA2_MASK,
  10, 10,
  (VERIFY_KEY *)&numeric_dnull
};


static PROCESS_ELEMENT NPriceArt_PROC[] =
{
  ENTER_KEY,       accept_active_item,
  NO_KEY,          delete_active_item,    /* Delete current item, back to   */
  UNKNOWN_KEY,     (void *)NULL
};


static CONTROL_ELEMENT NPriceArt_CTL[] =
{
  ENTER_KEY,           &Invoicing_ST,
  NO_KEY,              &Invoicing_ST,
  PRICE_TOO_LARGE_KEY, &Invoicing_ST,
  UNKNOWN_KEY,         &NPriceArt_ST
};


extern STATE_OBJ NPriceArt_ST =
{
  ST_NPRICE_ART,
  NPriceArt_VW,
  no_DFLT,
  &Dprice10K10nd,
  NPriceArt_VFY,
  Input_UVW,
  NPriceArt_PROC,
  NPriceArt_CTL
};



/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   NORMAL WEIGHT ARTICLE AND                               */
/*                   GENVAR.PRICE_WEIGHT == PRICE                            */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  At this point, an item is created the ARTICLE_GROUP is filled with data  */
/*  from the Store Controler.                                                */
/*  In the scrolling window of OPER-SCRN the article data is displayed.      */
/*  In this state, weight articles are handled and because                   */
/*  general.price_weight is equal to price, the price is retreived from      */
/*  the cashier.                                                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/


static void NPWeightArt_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[5],0,12);
} /* NPWeightArt_VW */



/*---------------------------------------------------------------------------*/
/*                               vfy_weight_price                            */
/*---------------------------------------------------------------------------*/
short vfy_weight_price(_TCHAR *data, short key)
{
  double gds_value;

  /*                                                                       */
  /* Put price in c_item and return key if data contains a number > 0      */    /* Invoke an error and return UNKNOWN_KEY if data is 0 or empty          */
  /*                                                                       */

  no_leading_zeros(data);

  if (!STR_ZERO(data)) {
    /*                                                                     */
    /* Price is a non-zero value, put it in goods_value field and          */
    /* calculate weight.                                                   */
    /*                                                                     */
    /*  - The price in c_item is in- or exclusive vat depending            */
    /*    on genvar. The goods-value just entered is also in- or           */
    /*    exclusive vat depending on genvar. So, nothing to                */
    /*    worry about, just put everything in the current item and calc    */
    /*    weight.                                                          */
    /*                                                                     */

    if (genvar.price_incl_vat==EXCLUSIVE) {
      gds_value=floor_price(calc_incl_vat(_tcstod(data, NULL),c_item.arti.base.vat_no));
    }
    else {
      gds_value=_tcstod(data, NULL);
    }

    if (gds_value > 999999999.0) {
      err_invoke(PRICE_TOO_LARGE);
      *data=_T('\0');
      return(UNKNOWN_KEY);
    }

    if (invoice_line_mode != SALES) {
      reverse_sign(data);
    }

    c_item.arti.base.goods_value=floor_price(atof_price(data));

    c_item.arti.base.qty=c_item.arti.base.goods_value/c_item.arti.base.price;
    if ((double)fabs(c_item.arti.base.qty) <= 9999.999) {
      c_item.arti.display_status=ARTICLE_OK;
      scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
      view_item(OPER_SCRN | CUST_SCRN);
      return(key);
    }
    else {
      err_invoke(PRICE_TOO_LARGE);            /* Quantity overflow.        */
      *data=_T('\0');
      return(UNKNOWN_KEY);
    }
  }
  err_invoke(ZERO_NOT_LEGAL_ERROR);

  return(UNKNOWN_KEY);
} /* vfy_weight_price */



static VERIFY_ELEMENT NPWeightArt_VFY[] =
{
  LINE_UP_KEY,     handle_scroll_key,
  LINE_DOWN_KEY,   handle_scroll_key,
  PAGE_UP_KEY,     handle_scroll_key,
  PAGE_DOWN_KEY,   handle_scroll_key,
  NO_KEY,          (void *)NULL,           /* Delete current item, back to  */
  ENTER_KEY,       vfy_weight_price,       /* returns UNKNOWN_KEY or ENTER  */
  OCIA1_DATA,      vfy_ocia_not_legal,     /* err_invoke() + UNKNOWN_KEY    */
  OCIA2_DATA,      vfy_ocia_not_legal,     /* err_invoke() + UNKNOWN_KEY    */
  CLEAR_KEY,       vfy_clear_key,
  OPEN_DRAWER_KEY, open_and_close_drawer,  /* Approval in state-engine.     */
  UNKNOWN_KEY,     illegal_fn_key
};


/* extern INPUT_CONTROLLER Dprice10K10nd,  defined in previous state         */


static PROCESS_ELEMENT NPWeightArt_PROC[] =
{
  ENTER_KEY,       accept_active_item,
  NO_KEY,          delete_active_item,
  UNKNOWN_KEY,     (void *)NULL
};


static CONTROL_ELEMENT NPWeightArt_CTL[] =
{
  ENTER_KEY,           &Invoicing_ST,
  NO_KEY,              &Invoicing_ST,
  PRICE_TOO_LARGE_KEY, &Invoicing_ST,
  UNKNOWN_KEY,         &NPWeightArt_ST
};


extern STATE_OBJ NPWeightArt_ST =
{
  ST_NPWEIGHT_ART,
  NPWeightArt_VW,
  no_DFLT,
  &Dprice10K10nd,
  NPWeightArt_VFY,
  Input_UVW,
  NPWeightArt_PROC,
  NPWeightArt_CTL
};


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   NORMAL WEIGHT ARTICLE AND                               */
/*                   GENVAR.PRICE_WEIGHT == WEIGHT                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  At this point, an item is created the ARTICLE_GROUP is filled with data  */
/*  from the Store Controler.                                                */
/*  In the scrolling window of OPER-SCRN the article data is displayed.      */
/*  In this state, weight articles are handled and because                   */
/*  general.price_weight is equal to weight, the weight is retreived from    */
/*  the cashier.                                                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/


static void NWWeightArt_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[6],0,10);     /* WEIGHT REQUIRED           */
} /* NWWeightArt_VW */



/*---------------------------------------------------------------------------*/
/*                               vfy_weight                                  */
/*---------------------------------------------------------------------------*/
short vfy_weight(_TCHAR *data, short key)
{
  short status;

  /*                                                                       */
  /* A weight is entered, put it in c_item if the weight is correct and    */
  /* calculate goods-value.                                                */
  /*                                                                       */

  no_leading_zeros(data);

  if (!STR_ZERO(data)) {
    if (invoice_line_mode!=SALES) {
      reverse_sign(data);
    }
    c_item.arti.base.qty=_tcstod(data, NULL)/1000;
    if (calc_gds_value(ART_GROUP)==PRICE_TOO_LARGE) {
      status=UNKNOWN_KEY;                     /* Value overflow.           */
    }
    else {
      status=key;
      c_item.arti.display_status=ARTICLE_OK;
      scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
      view_item(OPER_SCRN | CUST_SCRN);
    }
  }
  else {
    err_invoke(ZERO_NOT_LEGAL_ERROR);         /* No legal QTY-amount       */
    status=UNKNOWN_KEY;                       /* Loop to default           */
  }
  *data=_T('\0');

  return(status);
} /* vfy_weight */


short vfy_key_disable(_TCHAR *data, short key) /* acm -+++*/
{
  int status;
  *data=_T('\0');
  err_invoke(INVALID_KEY_ERROR);            /* show message              */
  status=UNKNOWN_KEY;                       /* Loop to default           */
  return(status);
}
short vfy_weight_valeturkey(_TCHAR *data, short key)
{
  short status;
  int lnum_error;

  /*                                                                       */
  /* A weight is entered, put it in c_item if the weight is correct and    */
  /* calculate goods-value.                                                */
  /*                                                                       */

  no_leading_zeros(data);

  if (!STR_ZERO(data)) {
    /* 12-Ago-2011 acm -++ Reverse invoice is not valid
    if (invoice_line_mode!=SALES) {
      reverse_sign(data);
    }*/

    c_item.arti.base.qty = _tcstod(data, NULL)/1000.0;
    c_item.arti.base.qty = get_base_qty( &c_item,&lnum_error); // 12-Ago-2011 acm - fix turkey vale qty
    if (lnum_error){
        err_invoke(WEIGHT_TURKEY_MUSTBE7);             
        status=UNKNOWN_KEY;
    }else {
        if (calc_gds_value(ART_GROUP)==PRICE_TOO_LARGE) {
          status=UNKNOWN_KEY;                     /* Value overflow.           */
        }
        else {
          status=key;
          c_item.arti.display_status=ARTICLE_OK;
          scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
          view_item(OPER_SCRN | CUST_SCRN);
        }
    }
  }
  else {
    err_invoke(ZERO_NOT_LEGAL_ERROR);         /* No legal QTY-amount       */
    status=UNKNOWN_KEY;                       /* Loop to default           */
  }
  *data=_T('\0');

  return(status);
} /* vfy_weight */


static VERIFY_ELEMENT NWWeightArt_VFY[] =
{
  LINE_UP_KEY,     handle_scroll_key,
  LINE_DOWN_KEY,   handle_scroll_key,
  PAGE_UP_KEY,     handle_scroll_key,
  PAGE_DOWN_KEY,   handle_scroll_key,
  NO_KEY,          (void *)NULL,           /* Delete current item, back to  */
  ENTER_KEY,       vfy_weight,             /* returns UNKNOWN_KEY or ENTER  */
  OCIA1_DATA,      vfy_ocia_not_legal,     /* err_invoke() + UNKNOWN_KEY    */
  OCIA2_DATA,      vfy_ocia_not_legal,     /* err_invoke() + UNKNOWN_KEY    */
  CLEAR_KEY,       vfy_clear_key,
  OPEN_DRAWER_KEY, open_and_close_drawer,  /* Approval in state-engine.     */
  UNKNOWN_KEY,     illegal_fn_key
};


extern INPUT_CONTROLLER Dqty6K6nd =
{
  (INPUT_DISPLAY *)&dsp_qty6,
  KEYBOARD_MASK | OCIA1_MASK | OCIA2_MASK,
  7, 7,
  (VERIFY_KEY *)&numeric_dnull
};


static PROCESS_ELEMENT NWWeightArt_PROC[] =
{
  ENTER_KEY,       accept_active_item,
  NO_KEY,          delete_active_item,
  UNKNOWN_KEY,     (void *)NULL
};


static CONTROL_ELEMENT NWWeightArt_CTL[] =
{
  ENTER_KEY,           &Invoicing_ST,
  NO_KEY,              &Invoicing_ST,
  PRICE_TOO_LARGE_KEY, &Invoicing_ST,
  UNKNOWN_KEY,         &NWWeightArt_ST
};

extern STATE_OBJ NWWeightArt_ST =
{
  ST_NWWEIGHT_ART,
  NWWeightArt_VW,
  no_DFLT,
  &Dqty6K6nd,
  NWWeightArt_VFY,
  Input_UVW,
  NWWeightArt_PROC,
  NWWeightArt_CTL
};

/* 12-Ago-2011 acm -{ */

static PROCESS_ELEMENT NWWeightArt_valeturkey_PROC[] =
{
  ENTER_KEY,       accept_active_item_valeturkey, /* 12-Ago-2011 acm -++ */
  NO_KEY,          delete_active_item,
  UNKNOWN_KEY,     (void *)NULL
};

static VERIFY_ELEMENT NWWeightArt_valeturkey_VFY[] =
{
  LINE_UP_KEY,     handle_scroll_key,
  LINE_DOWN_KEY,   handle_scroll_key,
  PAGE_UP_KEY,     handle_scroll_key,
  PAGE_DOWN_KEY,   handle_scroll_key,
  NO_KEY,          (void *)NULL,           /* Delete current item, back to  */
  ENTER_KEY,       vfy_weight_valeturkey,             /* returns UNKNOWN_KEY or ENTER  */
  OCIA1_DATA,      vfy_ocia_not_legal,     /* err_invoke() + UNKNOWN_KEY    */
  OCIA2_DATA,      vfy_ocia_not_legal,     /* err_invoke() + UNKNOWN_KEY    */
  CLEAR_KEY,       vfy_clear_key,
  OPEN_DRAWER_KEY, open_and_close_drawer,  /* Approval in state-engine.     */
  UNKNOWN_KEY,     illegal_fn_key
};



extern STATE_OBJ NWWeightArt_valeturkey_ST =
{
  ST_NWWEIGHT_ART,
  NWWeightArt_VW,
  no_DFLT,
  &Dqty6K6nd,
  NWWeightArt_valeturkey_VFY,
  Input_UVW,
  NWWeightArt_valeturkey_PROC,
  NWWeightArt_CTL
};

/* 12-Ago-2011 acm -} */

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE    VOID LAST ITEM                                         */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* At this point active_item points to TDM_ROOT and last_item points to      */
/* a previous entered item.                                                  */
/* This state handles the voiding of the previous entered item(!), this      */
/* includes the printing of that item in opposite way, unview the item       */
/* in the scroll window and recalculate shift- and subtotals.                */
/*                                                                           */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/*                           void_last_item                                  */
/*---------------------------------------------------------------------------*/
static short void_last_item(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Void the item pointed to by last_item.                                */
  /*                                                                       */

  *data=_T('\0');
  do_void_item(last_item);
  last_item=TM_ROOT;

  return(key);
} /* void_last_item */


/*---------------------------------------------------------------------------*/
/*                           do_void_item                                    */
/*---------------------------------------------------------------------------*/
short do_void_item(TM_INDX item_to_void)
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
    /* Approved item can be voided only with Supervisor approval.          */
    if (c_item.approved == TRUE && err_invoke(APPROVED_ITEM) != SUCCEED) {
      return(FAIL);                               /* Cancel the void.      */
    }

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
      --nbr_log_lines;
      if (c_item.depo.base.goods_value != 0.0) {  /* Also count deposits   */
        --nbr_log_lines;
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

      tm_upda(TM_ITEM_NAME, (void*)&c_item);

      /* Remove scroll lines from OPER-SCRN and clear CUST-SCRN            */
      scrl_delete_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, item_to_void);
      scrl_delete_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, item_to_void);
      scrl_delete_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, item_to_void);
      cdsp_clear();
      scrn_clear_window(INV_ART_DESCR_WINDOW);

      /* View the voided item only on the customer screen.                 */
      view_item(CUST_SCRN);

#ifdef PRN_IMM
      print_item();
#endif

      display_item = item_to_void;
      return(SUCCEED);
    }
    else {
      /* Total - item_to_void exceeds MAX_AMOUNT_ON_POS, so do not void the */
      /* article.                                                           */
      err_invoke(PRICE_TOO_LARGE);
      return(PRICE_TOO_LARGE);
    }
  }

  return(FAIL);
} /* do_void_item */


static void VoidLastItem_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[7],0,3); /* VOID LAST ITEM Y/N?           */
} /* NWWeightArt_VW */


static VERIFY_ELEMENT VoidLastItem_VFY[] =
{
  LINE_UP_KEY,     handle_scroll_key,
  LINE_DOWN_KEY,   handle_scroll_key,
  PAGE_UP_KEY,     handle_scroll_key,
  PAGE_DOWN_KEY,   handle_scroll_key,
  NO_KEY,          (void *)NULL,               /* Cancel voiding            */
  ENTER_KEY,       void_last_item,
  OPEN_DRAWER_KEY, open_and_close_drawer,      /* Approval in state-engine. */
  UNKNOWN_KEY,     illegal_fn_key
};


extern INPUT_CONTROLLER DYN1K1n =
{
  (INPUT_DISPLAY *)&dsp_YN1,
  KEYBOARD_MASK,
  2, 2,
  (VERIFY_KEY *)&numeric
};


static CONTROL_ELEMENT VoidLastItem_CTL[] =
{
  ENTER_KEY,       &Invoicing_ST,
  NO_KEY,          &Invoicing_ST,
  UNKNOWN_KEY,     &VoidLastItem_ST
};


extern STATE_OBJ VoidLastItem_ST =
{
  ST_VOID_LAST_ITEM,
  VoidLastItem_VW,
  no_DFLT,
  &DYN1K1n,
  VoidLastItem_VFY,
  Input_UVW,
  (void *)NULL,
  VoidLastItem_CTL
};
