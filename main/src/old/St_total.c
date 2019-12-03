/*
 *     Module Name       : ST_TOTAL.C
 *
 *     Type              : States DoTotal, ShowChange
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
 * 11-Jan-2000 Integrated multisam discounts                           P.M.
 * --------------------------------------------------------------------------
 * 30-Jan-2001 Removed reset multisam from close_and_log_invoice       R.N.B.
 *             because MultiSAM totals must also be on a reprinted inv.
 * --------------------------------------------------------------------------
 * 25-Sep-2001 Added Pending Invoice functionality                     M.W.
 * --------------------------------------------------------------------------
 * 22-Jan-2002 Use define of MAX_AMOUNT_ON_POS                         M.W.
 * --------------------------------------------------------------------------
 * 23-Jan-2002 Changes for Credit note. Ask for amount if offline.     M.W.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality       M.W.
 * --------------------------------------------------------------------------
 * 21-Jan-2003 Abort data in case of cancel tendering.                 M.W.
 * --------------------------------------------------------------------------
 * 16-Dec-2003 Added Donation functionality.                           M.W.
 * --------------------------------------------------------------------------
 * 06-May-2004 Added vat amounts on total section.                     M.W.
 * --------------------------------------------------------------------------
 * 06-Jul-2005 Bugfix: Rounding of Multisam discounts in case of 
 *             inclusive amounts on invoice                            M.W.
 * --------------------------------------------------------------------------
 * 28-Jul-2005 Summarize tax codes on total screen.                    M.W.
 * --------------------------------------------------------------------------
 * 01-Aug-2005 Detail payment on invoice.                              M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 * 23-Feb-2006 Added extra functionality for selecting printer. This
 *             is needed for testing purposes.                         J.D.M.
 * --------------------------------------------------------------------------
 * 12-Ago-2011 ACM - Change for sales Turkey						    ACM -
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <math.h>
#include <stdio.h>

#include "stnetp24.h"                       /* Pos (library) include files   */
#include "misc_tls.h"
#include "stri_tls.h"
#include "intrface.h"

#include "inp_mgr.h"                        /* Toolsset include files.       */
#include "prn_mgr.h"
#include "state.h"
#include "tm_mgr.h"
#include "bp_mgr.h"
#include "tot_mgr.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "err_mgr.h"
#include "mapkeys.h"
#include "sll_mgr.h"
#include "llist.h"
#include "comm_tls.h"

#include "pos_tm.h"                         /* Application include files.    */
#include "pos_errs.h"
#include "pos_recs.h"
#include "pos_txt.h"
#include "pos_inp.h"
#include "pos_func.h"
#include "pos_keys.h"
#include "pos_vfy.h"
#include "pos_dflt.h"
#include "pos_st.h"
#include "pos_tot.h"
#include "write.h"
#include "pos_bp1.h"
#include "pos_com.h"
#include "pos_log.h"
#include "pos_scrl.h"
#include "WPos_mn.h"
#include "st_main.h"
#include "pos_bp3.h"
#include "pos_msam.h"
#include "condll.h"
#include "Pos_chq.h"
#include "OPrn_mgr.h"
#include "registry.h"

#include "Pos_edoc.h"

/*

#define   NOT_USED   0
#define   USED       1

#define   VOUCHER_PREFIX   1
  */

short used_curr_type = 999; /* indicated curr_type.
                                      999 means NOT identified curr_type */
static short  calc_donation_amount;
static _TCHAR amount_donation[20];
 
static void   DoTotal_VW(void);
static short  reset_to_local_curr(void);
static short  payment_cd(short);
static void   ShowChange_VW(void);
static void   ShowChange_UVW(void);
static short  cancel_tendering(_TCHAR *, short);
static short  Choose_Chng(_TCHAR *, short);
static short  view_tender_scrn(short, short);
extern short  vfy_amount_due(void);

/* 12-Ago-2011 acm -{ */
/*static */  short  vfy_paymentway(_TCHAR *, short); /* 12-Ago-2011 acm - comment:static to normal funtion */

extern short  cancel_tendering_perception(_TCHAR *, short);

int     paymentway_bono_pressed=0;
short   vfy_paymentway_bono(_TCHAR *, short);
short   vfy_illegal_fn_key_bono(_TCHAR *data, short key);

extern void print_perception_document();
extern void print_perception_name();

/* 12-Ago-2011 acm -} */

extern void print_document();
extern void print_name();

static double calc_tot_paid(void);
extern short  vfy_voucher(_TCHAR *, short);
extern short  vfy_voucher_nc(_TCHAR * data, short key, int  * handled);

static short  vfy_voucher_amount(_TCHAR *, short);
extern short  book_voucher(short, double, short);
static short  cancel_voucher(short);
static void   DFLT_donation(INPUT_DISPLAY *, _TCHAR *);
static short  vfy_choice(_TCHAR *, short);
static short  vfy_donation_amount(_TCHAR *, short);
static void   view_select_printer_scrn(void);
static short  vfy_select_printer(_TCHAR *data, short key);

/* Variables needed for payment_items */
SLL    voucher_hist;
SLL    voucher_anticipo_hist;
SLL    payment_items;
INVOICE_PAYM_ITEM_DEF voucher_items;

SLL    tax_sum_codes;

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   DO TOTAL (HANDLE PAYMENT)                               */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* At this point it is certain the subtotal not equals zero.                 */
/* There are two posible modes;                                              */
/*    1.  The subtotal is positive, so we collect the amount from the        */
/*        customer, or                                                       */
/*    2.  the subtotal is negative, so we have to pay the customer.          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

extern INPUT_CONTROLLER Ddsp_perception_document =
{
  (INPUT_DISPLAY *)&dsp_perception_document,
  KEYBOARD_MASK,
  8, 9,
  (VERIFY_KEY *)&numeric
};

static void DoTotal_VW(void)
{
  static short no_cls_states[]={
       ST_DO_TOTAL
      ,ST_PASSDAY700 //mlsd FE
      ,ST_NAME //mlsd FE
      ,ST_PREV_DO_TOTAL
      ,ST_PERCEPTION_NAME
      ,0
  };

  if (called_by_state(no_cls_states)==FAIL) {
    cls();
  }
  view_tender_scrn(VIEW_AMNT_DUE, (OPER_SCRN | CUST_SCRN) );
  scrn_clear_window(TOTAL_INPUT_WINDOW);
  scrn_string_out(input_TXT[30],0,16);    /* ENTER PAID AMOUNT AND PRESS P */
} /* DoTotal_VW */


static VERIFY_ELEMENT DoTotal_VFY[] =
{
  NO_KEY,             (void *)NULL,
  VOID_INVOICE_KEY,   (void *)NULL,
  SAVE_INVOICE_KEY,   (void *)NULL,
  CLEAR_KEY,          vfy_clear_key,
  PAYMENT_WAY_0_KEY,  vfy_paymentway,   /* returns AMNT_(NOT)ENOUGH or       */
  PAYMENT_WAY_1_KEY,  vfy_paymentway,   /* UNKNOWN_KEY in case of errors.    */
  PAYMENT_WAY_2_KEY,  vfy_paymentway,
  PAYMENT_WAY_3_KEY,  vfy_paymentway,
  PAYMENT_WAY_4_KEY,  vfy_paymentway_bono, /* 12-Ago-2011 acm -+++{*/
  PAYMENT_WAY_5_KEY,  vfy_paymentway,
  PAYMENT_WAY_6_KEY,  vfy_paymentway,
  PAYMENT_WAY_7_KEY,  vfy_paymentway,
  PAYMENT_WAY_8_KEY,  vfy_paymentway,
  PAYMENT_WAY_9_KEY,  vfy_paymentway,
  OPEN_DRAWER_KEY,    open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,        vfy_illegal_fn_key_bono,    /* 12-Ago-2011 acm -+++{*/
};


extern INPUT_CONTROLLER Ddsp_payd_amnt11K11np =
{
  (INPUT_DISPLAY *)&dsp_payd_amnt11,
  KEYBOARD_MASK,
  12, 12,
  (VERIFY_KEY *)&numeric_dnull
};


static PROCESS_ELEMENT DoTotal_PROC[] =
{
  NO_KEY,           cancel_tendering,
  VOID_INVOICE_KEY, close_and_log_invoice,
  SAVE_INVOICE_KEY, save_invoice,
  UNKNOWN_KEY,      (void *)NULL
};


static CONTROL_ELEMENT DoTotal_CTL[] =
{
  NO_KEY,           &Invoicing_ST,
  VOID_INVOICE_KEY, &CustomerMode_ST,
  SAVE_INVOICE_KEY, &CustomerMode_ST,
  AMNT_ENOUGH,      &ShowChange_ST,
  AMNT_NOT_ENOUGH,  &DoTotal_ST,
  PAY_BY_VOUCHER,   &DoVoucher_ST,
  UNKNOWN_KEY,      &DoTotal_ST
};


extern STATE_OBJ DoTotal_ST =
{
  ST_DO_TOTAL,
  DoTotal_VW,
  no_DFLT,
  //&Ddsp_perception_document,
  &Ddsp_payd_amnt11K11np,
  DoTotal_VFY,
  Input_UVW,
  DoTotal_PROC,
  DoTotal_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   PerceptionInput                                       */
/*-------------------------------------------------------------------------*/

/*

extern INPUT_CONTROLLER Dartdescr33K33 =
{
  (INPUT_DISPLAY *)&dsp_art_descr,
  KEYBOARD_MASK,
  ART_DESC_INPUT_LENGTH, // key length    
  ART_DESC_INPUT_LENGTH, // buffer length 
  (VERIFY_KEY *)&printing_char_upr
};

*/

short  cancel_tendering_passday700(_TCHAR * data, short key)
{
  key=cancel_tendering(data,key);

  SetKeyMapping(NORMAL_KEYS);
  return key;

}

short  cancel_tendering_perception(_TCHAR * data, short key)
{
  key=cancel_tendering(data,key);

  SetKeyMapping(NORMAL_KEYS);
  return key;

}

short Prev_Amnt_Enought_init(_TCHAR *data, short key)
{
  if (IS_GEN_PERCEPTION)
  {
      if(is_client_pay_FACTURA())
      {
          sprintf( usr_perception_document,   cust.fisc_no );
          sprintf( usr_perception_name,       cust.name );
      }else
      {
          // BOLETA
          ////SetKeyMapping(ASCII_KEYS);

          if (   ispassday() || isFiscalClient() )
          {
              return PREV_TOTAL_KEY;
              //return PREV_AMNT_ENOUGH;
          }
          else 
          {
              sprintf( usr_perception_document,   cust.fisc_no );
              sprintf( usr_perception_name,       cust.name );
          }
      }
  }
  else
  { /* isFiscalClient() */
    printf_log("Entro al else state");
    if(!is_client_pay_FACTURA() && ispassday() && (tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))> 700))
    {
      return PASSDAY700_KEY;
    }
  }
  
  return key;
}

short Prev_DoTotal_ST_init(_TCHAR *data, short key)
{
    
    if (IS_GEN_PERCEPTION)
    {
        if(is_client_pay_FACTURA())
        {
            sprintf( usr_perception_document,   cust.fisc_no );
            sprintf( usr_perception_name,       cust.name );
        }else
        {
            // BOLETA
            ////SetKeyMapping(ASCII_KEYS);

            if (   ispassday() 
                || isFiscalClient() )
            {
                return PREV_TOTAL_KEY;
            }
            else 
            {
                sprintf( usr_perception_document,   cust.fisc_no );
                sprintf( usr_perception_name,       cust.name );
            }
        }
    }
    else
    { /* isFiscalClient() */
       printf_log("Entro al else state2[%d][%d][%.2f][%d]", ispassday(), !(is_client_pay_FACTURA()), tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL)), (tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL)) > 700));
      if(!(is_client_pay_FACTURA()) && ispassday() && (tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL)) > 700.0))
      {
        printf_log("Entro else state2 if");
        return PASSDAY700_KEY;
      }
    }
       
    return key;
}

short  vfy_PerceptionInput(_TCHAR *data, short key)
{
    int   ret_key=UNKNOWN_KEY;

    if (pe_val_dni(data))
    {
        ret_key= key;
    } 
    else
    {
        err_invoke(DOCUMENT_DOC_ERR);
        ret_key= UNKNOWN_KEY;
    } 
    return ret_key;
}

short prc_PerceptionInput(_TCHAR *data, short key)
{
    CUST_PERC_DEF cust_perc;
    CARDHOLDER_DEF cardholder;
    char old_document[500];

    strcpy(old_document,"");
    if (usr_perception_document[0]!=NULL)
        strcpy(old_document,usr_perception_document);

    strcpy(usr_perception_document,data);

    if ( ispassday() )
    {
        // Si es PassDay Buscar el Nombre en la tabla CUST_PERC
        if (getNameFromCust_Perc(data, &cust_perc))
        {
            return prc_PerceptionInput_name(cust_perc.cust_name,DOCID_FOUND);
        }else
        {
            if (strcmp(old_document,usr_perception_document)!=0)
            {
                //si cambio el DNI cambiamos el nombre
                strcpy(usr_perception_name,"");                
            }
        }
    }

    if ( isFiscalClient() ) 
    {
        if ( !is_client_pay_FACTURA() )
        {
            // Si es Cliente con RUC y pide boleta Buscar el Nombre en la tabla CARDHOLDER
            if (getNameFromCardHolder(data, &cardholder))
            {
                return prc_PerceptionInput_name(cardholder.name,DOCID_FOUND);
            }else
            {
                if (strcmp(old_document,usr_perception_document)!=0)
                {
                    //si cambio el DNI cambiamos el nombre
                    strcpy(usr_perception_name,"");                
                }
            }
        }
    }

    //SetKeyMapping(ASCII_KEYS);
//    usr_perception_name    [500];

    
    /*
    if (key==ENTER_KEY)
    {
        if (perception_current_field==1) // procesa dni
        {
           
            Ddsp_perception_document.display=&dsp_perception_name;
            perception_current_field=2;
        }else if (perception_current_field==2) // procesa nombre
        {
            key=ENTER_KEY_TOTAL;    
        }

    }
    */

    return key;
}

static void PerceptionInput_UVW(void)
{
  //ShiftPosCaret((short)(user_shift_pos_caret-1));

  //SetKeyMapping(NORMAL_KEYS);

}

static void PerceptionInput_VW(void)
{

  static short no_cls_states[]={
     ST_PREV_DO_TOTAL
    ,ST_PREV_AMNT_ENOUGH
    ,0
  };

  if (called_by_state(no_cls_states)==FAIL) {
    cls();
  }
  view_tender_scrn(VIEW_AMNT_DUE, (OPER_SCRN | CUST_SCRN) );
  scrn_clear_window(TOTAL_INPUT_WINDOW);
  ///scrn_string_out(input_TXT[30],0,16);    /* ENTER PAID AMOUNT AND PRESS P */

  format_display(&dsp_perception_document, usr_perception_document);
  strcpy(last_perception_document, usr_perception_document);

  print_perception_document();

  //SetKeyMapping(ASCII_KEYS);

  //SetShowCursor(TRUE);
  
} /* LogOn_VW */


static VERIFY_ELEMENT PerceptionInput_VFY[] =
{
  
  NO_KEY,             (void *)NULL,
  ENTER_KEY,          (void *)vfy_PerceptionInput,  // v3.5.1 acm -
  //VOID_INVOICE_KEY,   (void *)NULL,
  SAVE_INVOICE_KEY,   (void *)NULL,
  //CLEAR_KEY,          vfy_clear_key,
  //OPEN_DRAWER_KEY,    open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,        (void *)NULL, ///vfy_illegal_fn_key_bono,    /* 12-Ago-2011 acm -+++{*/
};



static PROCESS_ELEMENT PerceptionInput_PROC[] =
{
    NO_KEY,           cancel_tendering_perception,
    ENTER_KEY,        (void *)prc_PerceptionInput,  // v3.5.1 acm -
    UNKNOWN_KEY,      (void *)NULL
};

static CONTROL_ELEMENT PerceptionInput_CTL[] =
{
    NO_KEY,           &Invoicing_ST,
    ENTER_KEY,        &PerceptionInput_name_ST,  // v3.5.1 acm -
    DOCID_FOUND,      &DoTotal_ST,              // v3.5.1 acm -
    AMNT_ENOUGH,      &ShowChange_ST,
    UNKNOWN_KEY,      &Prev_DoTotal_ST
};

//mname

extern STATE_OBJ Prev_DoTotal_ST =
{                         
  ST_PREV_DO_TOTAL,
  PerceptionInput_VW,
  perception_document_DFLT,
  &Ddsp_perception_document,
  PerceptionInput_VFY,
  PerceptionInput_UVW,
  PerceptionInput_PROC,
  PerceptionInput_CTL
};

extern STATE_OBJ Prev_Amnt_Enough_ST =
{                         
  ST_PREV_AMNT_ENOUGH,
  PerceptionInput_VW,
  perception_document_DFLT,
  &Ddsp_perception_document,
  PerceptionInput_VFY,
  PerceptionInput_UVW,
  PerceptionInput_PROC,
  PerceptionInput_CTL
};



/*
int user_shift_pos_caret=-1;
_TCHAR reg_cursor[11] = _T("");
*/



/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   PerceptionInput                                       */
/*-------------------------------------------------------------------------*/


extern INPUT_CONTROLLER Ddsp_perception_name =
{
  (INPUT_DISPLAY *)&dsp_perception_name,
  KEYBOARD_MASK,
  40, 41,
  (VERIFY_KEY *)&printing_char_upr
};

short  vfy_PerceptionInput_name(_TCHAR *data, short key)
{
  if ((data==NULL) ||(strlen(data)<5))
  {
      err_invoke(ILLEGAL_CLIENAME_PERCEPTION);
      return(UNKNOWN_KEY);
  }
  return key;
}

short prc_PerceptionInput_name(_TCHAR *data, short key)
{
    strcpy(usr_perception_name,data);

    if ((key==ENTER_KEY) || (key==DOCID_FOUND) )
    {
        if ( tot_ret_double(TOT_GEN_INCL)
           + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL)) <= 0.0 ) 
        {

          //REVERSA      
          key=AMNT_ENOUGH;
        }        
    }
//    SetKeyMapping(NORMAL_KEYS); 
    return key;
}

static void PerceptionInput_name_UVW(void)
{
  //ShiftPosCaret((short)(user_shift_pos_caret-1));

  SetKeyMapping(NORMAL_KEYS);
    
}

static void PerceptionInput_name_VW(void)
{

  static short no_cls_states[]={
       ST_PREV_DO_TOTAL
      ,ST_PREV_AMNT_ENOUGH
      ,ST_PERCEPTION_NAME
      ,0
  };

  if (called_by_state(no_cls_states)==FAIL) {
    cls();
  }

  /*
  if (*reg_cursor == _T('\0')) {
    ReadEnvironmentValue(TREE_SCRN_SETTINGS, _T("ShowCursor"), reg_cursor, 10);
    if (_tcsicmp(reg_cursor, _T("vertical")) != 0) {
      user_shift_pos_caret = 0;
    }
    else {
      user_shift_pos_caret = 1;     // shift caret 1 position with respect to cursor     }
  }

  ShiftPosCaret(user_shift_pos_caret);
    */

  view_tender_scrn(VIEW_AMNT_DUE, (OPER_SCRN | CUST_SCRN) );
  scrn_clear_window(TOTAL_INPUT_WINDOW);


  format_display(&dsp_perception_name, usr_perception_name);
  strcpy(last_perception_name, usr_perception_name);
  
  print_perception_name();
  //SetCaretPosition(10, 10);

  SetKeyMapping(ASCII_KEYS);
  
} /* LogOn_VW */




static VERIFY_ELEMENT PerceptionInput_name_VFY[] =
{
  
  NO_KEY,             (void *)NULL,
  ENTER_KEY,          (void *)vfy_PerceptionInput_name,  // v3.5.1 acm -
///  VOID_INVOICE_KEY,   (void *)NULL,
//  SAVE_INVOICE_KEY,   (void *)NULL,
  //CLEAR_KEY,          vfy_clear_key,
//  OPEN_DRAWER_KEY,    open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,        (void *)NULL, ///vfy_illegal_fn_key_bono,    /* 12-Ago-2011 acm -+++{*/
};




static PROCESS_ELEMENT PerceptionInput_name_PROC[] =
{
    NO_KEY,           cancel_tendering_perception,
    ENTER_KEY,        (void *)prc_PerceptionInput_name,  // v3.5.1 acm -
    UNKNOWN_KEY,      (void *)NULL
};


static CONTROL_ELEMENT PerceptionInput_name_CTL[] =
{
    NO_KEY,           &Invoicing_ST,
    ENTER_KEY,        &DoTotal_ST,  // v3.5.1 acm -
    AMNT_ENOUGH,      &ShowChange_ST,
    UNKNOWN_KEY,      &PerceptionInput_name_ST
};

extern STATE_OBJ PerceptionInput_name_ST =
{                         
  ST_PERCEPTION_NAME,
  PerceptionInput_name_VW,
  perception_name_DFLT,
  &Ddsp_perception_name,
  PerceptionInput_name_VFY,
  PerceptionInput_name_UVW,
  PerceptionInput_name_PROC,
  PerceptionInput_name_CTL
};



/*---------------------------------------------------------------------------*/
/*                              reset_extra_amount                           */
/*---------------------------------------------------------------------------*/
void reset_extra_amount(void)
{
  tot_reset_double(TOT_CREDIT_AMNT);
  tot_reset_double(TOT_CREDIT_VAT_AMNT);
  tot_reset_double(TOT_CREDIT_VAT_NO);
  tot_reset_double(TOT_CREDIT_PAYM_CD);

  tot_reset_double(TOT_LCREDIT_AMNT);
  tot_reset_double(TOT_LCREDIT_VAT_AMNT);
} /* reset_extra_amount */


/*---------------------------------------------------------------------------*/
/*                              reset_paymentways                            */
/*---------------------------------------------------------------------------*/
void reset_paymentways(void)
{
  short i;

  /*                                                                       */
  /* Reset all paymentway-totals                                           */
  /*                                                                       */

  for(i=TOT_PAYM_0; i<=TOT_PAYM_9; i++) {
    tot_reset_double(i);
  }
  tot_reset_double(TOT_PAID);

  tot_reset_double(TOT_CHANGE);
  tot_reset_double(TOT_CHANGE_CD);

  reset_extra_amount();
} /* reset_paymentways */


/*---------------------------------------------------------------------------*/
/*                          cancel_tendering                                 */
/*---------------------------------------------------------------------------*/
static short cancel_tendering(_TCHAR *data, short key)
{
  double dl, dl1;
  short vat_no;

  /*                                                                       */
  /* The tendering (totalising) is canceled; the invoice-screen is re-     */
  /* painted and the payment's until now are cleared!                      */
  /*                                                                       */

  /*                                                                       */
  /* If an extra amount is charged for credit-cards, the extra amount +    */
  /* extra vat-amount must be substracted from the already updated totals. */
  /*                                                                       */

  dl=tot_ret_double(TOT_LCREDIT_AMNT);
  if (dl > (double)0 ) {
    dl1    =tot_ret_double(TOT_LCREDIT_VAT_AMNT);
    vat_no=(short)tot_ret_double(TOT_CREDIT_VAT_NO);

    tot_add_double(TOT_GEN_INCL,      -1*(dl+dl1));
    tot_add_double(TOT_GEN_EXCL,      -1*dl);
    tot_add_double((short)(TOT_EXCL_0+vat_no), -1*dl);
    tot_add_double((short)(TOT_VAT_0 +vat_no), -1*dl1);
    tot_add_double((short)(TOT_INCL_0+vat_no), -1*dl+dl1);
    tot_add_double(TOT_GEN_VAT,       -1*dl1);
  }

  reset_paymentways();
  reset_multisam_discounts();

  key = cancel_voucher(key);
  reset_voucher_items();

  inp_abort_data();


  return(key);
} /* cancel_tendering */


/*---------------------------------------------------------------------------*/
/*                       view_tender_scrn                                    */
/*---------------------------------------------------------------------------*/
static short view_tender_scrn(short mode, short screen)
{
  short i, row, credit_used;
  _TCHAR gen_incl_str[19], tot_paid_str[19], due_str[19], d_currency_cd[4];
  double due, d_gen_incl, d_tot_paid, d_rate, d_standard;
  PAYMENT_DEF paym;
  short  nbr_paym_ways, nbr_vat_codes, used_vat_cd, start_row_vat, column;
  _TCHAR buffer[81];
  _TCHAR tot_vat[TOTAL_BUF_SIZE +1];
  _TCHAR tot_excl[TOTAL_BUF_SIZE +1];
  _TCHAR perc[7 + 1];

  double           tot_tax_sum_cd, tot_vat_tax_sum_cd;
  short            count, start_row_tax_sum_codes;
  TAX_SUM_CD_DEF   tsco_record;

  scrn_select_window(PAYMENT_1_WINDOW);

  credit_used = FALSE;
  if (tot_ret_double(TOT_CREDIT_AMNT) > (double)0) {
    credit_used = TRUE;
    i=(short) tot_ret_double(TOT_CREDIT_PAYM_CD);
    due = tot_ret_double((short)(TOT_PAYM_0+i));
  }

  d_gen_incl = tot_ret_double(TOT_GEN_INCL) +
               floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))
               // v3.6.1 acm - {
               + get_invoice_perception_currmode(); /*floor_price(TOT_GEN_PERCEPTION);*/ //v3.6.2 acm -
               /// v3.6.1 acm - }

  d_tot_paid = tot_ret_double(TOT_PAID);

  if (used_curr_type == 999) {
    reset_to_local_curr();
    paym.paym_cd=used_curr_type;
    if (get_paym(&paym) == SUCCEED) {
      if (_tcscmp(paym.paym_type,_T("1")) == 0 ) {
        used_curr_type = paym.paym_cd; /* Selects correct currency  */
        d_rate = paym.curr_rate;       /* where type is 1 = local   */
        d_standard = paym.curr_standard;
        _tcscpy(d_currency_cd, _T("   "));
        strcpy_no_null(d_currency_cd, paym.currency_cd);
      }
    }
    if (used_curr_type == 999) { /* not-found any local/cash currency */
      paym.paym_cd = 1;
      get_paym(&paym);
    }
  }
  else {
    paym.paym_cd = used_curr_type;
    if (get_paym(&paym) != SUCCEED) {
      err_invoke(PAYMENT_NOT_PRESENT);
      return(UNKNOWN_KEY);
    }
    else {
      used_curr_type = paym.paym_cd;
      d_rate = paym.curr_rate;
      d_standard = paym.curr_standard;
      _tcscpy(d_currency_cd, _T("   "));
      strcpy_no_null(d_currency_cd, paym.currency_cd);
    }
  }

  if (screen & OPER_SCRN) {
    if (credit_used) {
      /* In case of credit-cards, display allways 'DUE' and the            */
      /* amount to pay with the credit-card.                               */
      scrn_string_out(scrn_inv_TXT[11],0,0);                     /* DUE    */
    }
    else {
      due = floor_price(d_gen_incl - d_tot_paid);             /* Local due */
      due = (due*d_rate)/d_standard;
      if (due <= (double)0 ) {
        scrn_string_out(scrn_inv_TXT[8],0,0);                    /* CHANGE */
      }
      else {
        scrn_string_out(scrn_inv_TXT[11],0,0);                   /* DUE    */
      }
    }
    ftoa_price(due, TOTAL_BUF_SIZE, due_str);
    format_string(&string_price12, due_str);
    scrn_string_out(string_price12.result,0,8);
    scrn_string_out(d_currency_cd,0,24);               /* Actual CURR_TYPE */
  }

  if (screen & CUST_SCRN) {
    cdsp_clear();
    /* By more currency types d_rate and d_standard necessary */
    ftoa_price((d_gen_incl*d_rate)/d_standard, TOTAL_BUF_SIZE, gen_incl_str);
    ftoa_price((d_tot_paid*d_rate)/d_standard, TOTAL_BUF_SIZE, tot_paid_str);
    if (floor_price(d_gen_incl) <= (double)0) {
      /* TOTAL: total is negative, pay customer.                           */
      format_string(&string_price13_2, gen_incl_str);
      cdsp_write_string(cdsp_TXT[5],0,0);
      cdsp_write_string(string_price13_2.result,0,CDSP_RIGHT_JUSTIFY);
    }
    else {
      if (mode == VIEW_AMNT_DUE) {
        /* Display tendering results, TOTAL: / PAID:                       */
        cdsp_write_string(cdsp_TXT[6],0,0); /* CUST.DISPLAY TEXT : TOTAL   */

        /* In case of a credit amount, the amount TOTAL and PAID           */
        /* should be the same. Use a trick to do this...                   */
        if (credit_used) {
          format_string(&string_price13_2, tot_paid_str);     /* PAID      */
        }
        else {
          format_string(&string_price13_2, gen_incl_str);      /* TOTAL GEN */
        }

        cdsp_write_string(string_price13_2.result,0,CDSP_RIGHT_JUSTIFY);
        Sleep(20); /* This is done to prevent timing problems with the customer display */
        cdsp_write_string(cdsp_TXT[7],1,0); /* CUST.DISPLAY TEXT : PAID    */
        format_string(&string_price13_2, tot_paid_str);
        cdsp_write_string(string_price13_2.result,1,CDSP_RIGHT_JUSTIFY);
      }
      else {
        /* Paid enough, display CHANGE: -> customer display 6              */
        cdsp_write_string(cdsp_TXT[8],0,0);
        ftoa_price(d_gen_incl - d_tot_paid,TOTAL_BUF_SIZE, due_str);
        format_string(&string_price13, due_str);
        cdsp_write_string(string_price13.result,0,CDSP_RIGHT_JUSTIFY);
        Sleep(20); /* This is done to prevent timing problems with the customer display */
        cdsp_write_string(cdsp_TXT[9],1,0);                  /* THANK YOU  */
      }
    }
  }

  
  if (screen & OPER_SCRN) {
    /*                                                                     */
    /* Display available paymentways.                                      */
    /*                                                                     */

    /* Determine Number of Payment ways                                    */
    nbr_paym_ways = 0;
    for(i=PAYM_WAY_0; i<MAX_PAYM_WAYS; i++) {
      paym.paym_cd = i;
      if (get_paym(&paym) == SUCCEED) {
        if (paym.paym_limit != (double)0 ) {
          nbr_paym_ways++;
        }
      }
    }

    scrn_select_window(PAYMENT_2_WINDOW);
    scrn_clear_window(PAYMENT_2_WINDOW);
    row=1; column=0;
    for(i=PAYM_WAY_1; i<=MAX_PAYM_WAYS; i++) {
      paym.paym_cd = (i==MAX_PAYM_WAYS) ? PAYM_WAY_0 : i;
      if (get_paym(&paym) == SUCCEED) {
        if (paym.paym_limit != (double)0 ) {
          _tcscpy(gen_incl_str,_T("  "));
          _itot(i,(gen_incl_str+1),DECIMAL);
          scrn_string_out((gen_incl_str+_tcslen(gen_incl_str)-2), row, (short)(0 + column));
          scrn_string_out(paym.paym_descr, row, (short)(3 + column));
          ftoa_price(tot_ret_double((short)(TOT_PAYM_0 + paym.paym_cd)),
                     TOTAL_BUF_SIZE, gen_incl_str);
          format_string(&string_price11, gen_incl_str);
          scrn_string_out(string_price11.result, row, (short)(19 + column));
          scrn_string_out(paym.currency_cd,row, (short)(34 + column)); /* CURR_TYPE FL/DM  */
          ++row;
        }
        // 2 balanced columns of payment ways
        if (row > nbr_paym_ways/2 + (short)(nbr_paym_ways%2==0 ? 0 : 1)) {
          start_row_vat = row+1;
          row = 1;
          column = 40;
        }
      }
    }
 
    /* Determine Number of Vat codes                                    */
    nbr_vat_codes = 0;
    for (used_vat_cd=TOT_EXCL_0; used_vat_cd<=TOT_EXCL_9; used_vat_cd++) {
      if (tot_ret_double(used_vat_cd) != 0.0) {
        nbr_vat_codes++;
      }
    }

    row = start_row_vat;
    column=0;

    if (nbr_vat_codes==1) {
      scrn_string_out(scrn_inv_TXT[29], row++, (short)(1 + column));
    }
    else {
      scrn_string_out(scrn_inv_TXT[30], row++, (short)(1 + column));
    }

    start_row_vat = row;

    for (used_vat_cd=TOT_EXCL_0; used_vat_cd<=TOT_EXCL_9; used_vat_cd++) {

      if (tot_ret_double(used_vat_cd) != 0.0) {

        ftoa_price(tot_ret_double(used_vat_cd) +
                   tot_ret_double((short)(MSAM_DISC_TOT_EXCL_0+used_vat_cd)),
                   TOTAL_BUF_SIZE, tot_excl);
        format_string(&string_price13_2, tot_excl);
        _tcscpy(tot_excl, string_price13_2.result);
 
        ftoa_price(tot_ret_double((short)(used_vat_cd + TOT_VAT_0)) +
                   tot_ret_double((short)(MSAM_DISC_TOT_VAT_0+used_vat_cd)),
                   TOTAL_BUF_SIZE, tot_vat);
        format_string(&string_price9, tot_vat);
        _tcscpy(tot_vat, string_price9.result);

        *perc = _T('\0');
        fmt_vat_perc(get_vat_perc((short)(used_vat_cd % 10)), perc);
  
        _stprintf(buffer, scrn_inv_TXT[31], used_vat_cd % (short)10, perc,
                                           tot_excl, tot_vat);

        scrn_string_out(buffer, row, (short)(1 + column));
        ++row;
      }
 
      // 2 balanced columns of vatcodes
      if (row - (start_row_vat-1)> nbr_vat_codes/2 + (short)(nbr_vat_codes%2==0 ? 0 : 1)) {
          start_row_tax_sum_codes = row;
          row = start_row_vat;
          column = 40;
      }
    }

    i   = 0;
    row = start_row_tax_sum_codes;

    while (sll_read(&tax_sum_codes,i++,&tsco_record) == SUCCEED)
    {
      count              = 0;
      tot_tax_sum_cd     = 0.0;
      tot_vat_tax_sum_cd = 0.0;

      for (used_vat_cd=TOT_EXCL_0; used_vat_cd<=TOT_EXCL_9; used_vat_cd++) {

        if (get_vat_sum_cd(used_vat_cd) == tsco_record.summarize_cd &&
                                           tot_ret_double(used_vat_cd) != 0.0) {
          count++;
          tot_tax_sum_cd     += tot_ret_double(used_vat_cd) +
                                tot_ret_double((short)(MSAM_DISC_TOT_EXCL_0+used_vat_cd));
          tot_vat_tax_sum_cd += tot_ret_double((short)(used_vat_cd + TOT_VAT_0)) +
                                tot_ret_double((short)(MSAM_DISC_TOT_VAT_0+used_vat_cd));
        }
      }
      if (count > 1) {

        ftoa_price(tot_tax_sum_cd, TOTAL_BUF_SIZE, tot_excl);
        format_string(&string_price13_2, tot_excl);
        _tcscpy(tot_excl, string_price13_2.result);
   
        ftoa_price(tot_vat_tax_sum_cd, TOTAL_BUF_SIZE, tot_vat);
        format_string(&string_price9, tot_vat);
        _tcscpy(tot_vat, string_price9.result);

        _stprintf(buffer, scrn_inv_TXT[32], tsco_record.summarize_cd,
                                            tsco_record.descr, tot_excl, tot_vat);
        scrn_string_out(buffer, ++row, 0);
      }
    }
  }

	if (IS_GEN_PERCEPTION )
	{
	  {
		char buff0[100];
		user_ftoa_porcentaje(PERC_GEN_PERCEPTION, buff0);
		

		if (IS_ANTICIPO())
			sprintf(buffer, prn_inv_TXT[91], buff0, 
					get_invoice_perception_currmode()
					- cur_anticipo.percepcion);//3.6.2 acm -
		else
			sprintf(buffer, prn_inv_TXT[91], buff0, 
					get_invoice_perception_currmode());//3.6.2 acm -
			
		scrn_string_out(buffer, 12, 1);
		
		if (!is_client_pay_FACTURA())// solo imprimir dni  y nombre en caso que NO SEA FACTURA
		{
			print_perception_document();
			print_perception_name();
		}

		//format_display_passwrd(&dsp_perception_name, usr_perception_name);
	  }
	}  
  
	if ((ispassday()) && (d_gen_incl > 700.0))
	{
		print_document();
		print_name();
	}
  
  //scrn_string_out("PERCEPCION ", row++, (short)(1 + column));

  return(SUCCEED);
} /* view_tender_scrn */

void print_perception_name()
{
  _TCHAR buffer[81];

  sprintf(buffer, prn_inv_TXT[93], usr_perception_name);  //nombre
  scrn_string_out(buffer, 15, 1);
}

void print_perception_document()
{
  _TCHAR buffer[81];        

  sprintf(buffer, prn_inv_TXT[92], usr_perception_document);  //dni
  scrn_string_out(buffer, 14, 1);
  //format_display_passwrd(&dsp_perception_document, usr_perception_document /*empty*/);
}


void print_name()
{
  _TCHAR buffer[81];

  sprintf(buffer, prn_inv_TXT[93],usr_name);  //nombre
  scrn_string_out(buffer, 15, 1);
}

void print_document()
{
  _TCHAR buffer[81];        

  sprintf(buffer, prn_inv_TXT[92], usr_document);  //dni
  scrn_string_out(buffer, 14, 1);
  //format_display_passwrd(&dsp_perception_document, usr_perception_document /*empty*/);
}




/*---------------------------------------------------------------------------*/
/*                            payment_cd                                     */
/*---------------------------------------------------------------------------*/
static short payment_cd(short key)
{
  short i=0;
  short paym_key_to_cd[][2] = {      /* payment-code */
       PAYMENT_WAY_1_KEY, 1
      ,PAYMENT_WAY_2_KEY, 2
      ,PAYMENT_WAY_3_KEY, 3
      ,PAYMENT_WAY_4_KEY, 4
      ,PAYMENT_WAY_5_KEY, 5
      ,PAYMENT_WAY_6_KEY, 6
      ,PAYMENT_WAY_7_KEY, 7
      ,PAYMENT_WAY_8_KEY, 8
      ,PAYMENT_WAY_9_KEY, 9
      ,PAYMENT_WAY_0_KEY, 0
      ,key              ,-1    /* last item; key not present */
  };

  /*                                                                       */
  /* Determine the payment code belonging to paymentway-key 'key'.         */
  /*                                                                       */
  while (paym_key_to_cd[i][0] != key) {
    i++;
  }

  return(paym_key_to_cd[i][1]);
} /* payment_cd */


/*---------------------------------------------------------------------------*/
/*                          reset_to_local_curr()                            */
/*---------------------------------------------------------------------------*/
static short reset_to_local_curr(void)
{
  PAYMENT_DEF paym;
  short i;

  for(i=PAYM_WAY_0+1; i<=MAX_PAYM_WAYS; i++) {
    paym.paym_cd=i%10;
    if (get_paym(&paym) == SUCCEED) {
      if (_tcscmp(paym.paym_type,_T("1")) == 0 ) {
        used_curr_type = paym.paym_cd; /* Marks local currency  */
      }
    }
  }

  return(1);
} /* reset_to_local_curr() */


/*---------------------------------------------------------------------------*/
/*                           vfy_amount_due                                  */
/*---------------------------------------------------------------------------*/
short vfy_amount_due(void)
{
  double min_diff;

  /* Calculates if the amount-paid >= total-amount. */

  if (genvar.ind_price_dec == DECIMALS_YES) {
    min_diff = 0.005;
  }
  else {
    min_diff = 0.5;
  }
  if( amount_due() < min_diff) {
    return(AMNT_ENOUGH);
  }
  else {
    return(AMNT_NOT_ENOUGH);
  }
} /* vfy_amount_due */

/* 12-Ago-2011 acm -{ */
/*---------------------------------------------------------------------------*/
/*                           setpayment_curr_type                            */
/*---------------------------------------------------------------------------*/
void setpayment_curr_type(int value)
{
  used_curr_type= value; 
}

/*---------------------------------------------------------------------------*/
/*                           vfy_paymentway_bono                             */
/*---------------------------------------------------------------------------*/


short  vfy_paymentway_bono(_TCHAR *data, short key)
{
  int status;
  status=vfy_paymentway(data,key);

  if (used_curr_type==1){
    paymentway_bono_pressed=1;
  }  
  
  return status;
}

/*---------------------------------------------------------------------------*/
/*                           vfy_illegal_fn_key_bono                         */
/*---------------------------------------------------------------------------*/
short vfy_illegal_fn_key_bono(_TCHAR *data, short key)
{
  if ((key==ENTER_KEY)&&(paymentway_bono_pressed==1)&&(*data == _T('\0'))){
    *data = _T('\0');
    return(UNKNOWN_KEY);
  }
  paymentway_bono_pressed = 0;
  err_invoke(ILLEGAL_FUNCTION_KEY_ERROR);
  return(UNKNOWN_KEY);
} /* vfy_illegal_fn_key_bono */


/* 12-Ago-2011 acm -} */

/*---------------------------------------------------------------------------*/
/*                           vfy_paymentway                                  */
/*---------------------------------------------------------------------------*/
/*static */ short vfy_paymentway(_TCHAR *data, short key)  /* 12-Ago-2011 acm - comment:static to normal funtion */
{
  short status, paymcd, i, mul = 1;
  PAYMENT_DEF paym;
  double amount, d, d1, dl, dl1;
  STATE_OBJ *curr_st;

  if ((paymcd=payment_cd(key)) != -1) {   /* A payment-key is pressed      */
    paym.paym_cd=paymcd;
    if (get_paym(&paym) != SUCCEED) {     /* Payment-type does not exist   */
      err_invoke(PAYMENT_NOT_PRESENT);
      return(UNKNOWN_KEY);
    }
    else {
      if (*data == _T('\0')) {
        used_curr_type = paym.paym_cd;
      }
    }

    copy_invoice = NO;                    /* Reset copy_invoice.           */

    /*                                                                     */
    /* No amount on paymenttype 'INTERNAL' and 'VOUCHER', mandatory        */
    /* amount on non-credit-cards.                                         */
    /*                                                                     */

    if (*paym.paym_type == PAYM_TYPE_VOUCHER) {
      if (*data == _T('\0')) {            /* Amount on the voucher.        */
        return(PAY_BY_VOUCHER);
      }
    } 

    if (paym.extra_perc != 0 || paym.min_extra_amount != 0 ||
        *paym.paym_type == PAYM_TYPE_INTERNAL ||
        *paym.paym_type == PAYM_TYPE_VOUCHER) {
      if (*data != _T('\0')) {            /* No amount on these types.     */
        error_extra_msg=paym.paym_descr;  /* Displayed in error message.   */
        err_invoke(NO_AMOUNT_ON_CREDIT_CARDS);
        *data=_T('\0');
        return(UNKNOWN_KEY);
      }
      else {
        /* Amount == 0, so display due in (pressed) currency type */
        view_tender_scrn(VIEW_AMNT_DUE, 0);
      }
    }
    else if (*data == _T('\0')) {
      /* Amount == 0, so display due in (pressed) currency type */
      view_tender_scrn(VIEW_AMNT_DUE, (CUST_SCRN | OPER_SCRN) );
      return(UNKNOWN_KEY);
    }

    if (used_curr_type != paym.paym_cd) {
      err_invoke(PAYTYPE_NOT_SAME_SELECT); /* SELECT PAYMENT_CD != AMOUNT+PAYMENT_CD */
      return(UNKNOWN_KEY);
    }

    if (*paym.paym_type == PAYM_TYPE_INTERNAL) {
      if (tot_ret_double(TOT_PAID) > 0.0) {
        error_extra_msg=paym.paym_descr;  /* Displayed in error message.   */
        err_invoke(NO_OTHER_PAYMENTS);    /* Only use if no others used    */
        *data=_T('\0');
        return(UNKNOWN_KEY);
      }                                   /* Supervisor approval needed    */
      display_prompt(paym.paym_descr, ERROR_WINDOW_ROW1);
      if (wait_for_super_appr_or_NO() == SUCCEED) {
        scrn_clear_window(ERROR_WINDOW_ROW1);
        copy_invoice = YES;               /* Copy invoice is printed       */
        ftoa_price((amount_due()*paym.curr_rate)/paym.curr_standard, TOTAL_BUF_SIZE, data);
        curr_st = state_get();
        if (curr_st->input) {
          curr_st->input->display->fn(curr_st->input->display, data);
        }
      }                                   /* Cancel payment type           */
      else {
        scrn_clear_window(ERROR_WINDOW_ROW1);
        *data=_T('\0');
        return(UNKNOWN_KEY);
      }
    }

    if (paym.extra_perc != 0 || paym.min_extra_amount != 0) {
      reset_extra_amount();

      /* dl and dl1 are values in < LOCAL > currency.                      */
      dl =floor_price((paym.extra_perc/100) * amount_due());/* Extra amount*/
      dl1=floor_price((get_vat_perc(paym.vat_no)/100)*dl);  /* Extra VAT   */

      /* d and d1 are values in < FOREIGN > currency.                      */
      d =floor_price((paym.extra_perc/100) *
         ((amount_due()*paym.curr_rate)/paym.curr_standard));/* Extra amount */
      d1=floor_price((get_vat_perc(paym.vat_no)/100)*d);   /* Extra VAT    */

      if (d+d1 < paym.min_extra_amount) {
        d  =paym.min_extra_amount;     /* extra_amount in foreign currency */
        d1 =floor_price((get_vat_perc(paym.vat_no)/100)*d);
                                       /* recalc to local currency         */
        dl =floor_price(((paym.min_extra_amount * paym.curr_standard)/
                             paym.curr_rate));
        dl1=floor_price((get_vat_perc(paym.vat_no)/100)*dl);
      }
      tot_add_double(TOT_CREDIT_VAT_NO, (double)paym.vat_no);
      tot_add_double(TOT_CREDIT_PAYM_CD, (double)paym.paym_cd);

      tot_add_double(TOT_CREDIT_AMNT, d);
      tot_add_double(TOT_CREDIT_VAT_AMNT, d1);

      tot_add_double(TOT_LCREDIT_AMNT, dl);
      tot_add_double(TOT_LCREDIT_VAT_AMNT, dl1);

      /* Create data and display it. */
      ftoa_price((amount_due()*paym.curr_rate)/paym.curr_standard +
                 (dl * paym.curr_rate)/paym.curr_standard         +
                 (dl1* paym.curr_rate)/paym.curr_standard, 19, data);

      curr_st = state_get();
      if (curr_st->input) {
        curr_st->input->display->fn(curr_st->input->display, data);
      }
    }

    if (*paym.paym_type == PAYM_TYPE_CHEQUE && cust.ind_cheques == FALSE ) {
      if ((status=err_invoke(CUST_NO_CHEQUES_ALLOWED))==SUCCEED) {
        cust.ind_cheques=TRUE;
      }
      else {
        *data=_T('\0');
        reset_extra_amount();
        return(UNKNOWN_KEY);
      }
    }

    amount = atof_price(data);

    if (genvar.ind_price_dec == DECIMALS_YES) {
      mul = 100;
    }

    /*                                                                     */
    /* Check for overflow already paid amount for requested payment        */
    /* way plus entered amount.                                            */
    /*                                                                     */
    if (mul*fabs(tot_ret_double((short)(TOT_PAYM_0+paymcd))+amount) > MAX_AMOUNT_ON_POS) {
      err_invoke(VALUE_TOO_LARGE);
      *data=_T('\0');
      reset_extra_amount();
      return(UNKNOWN_KEY);
    }

    /*                                                                     */
    /* Check for overflow sum of paymentways plus current amount.          */
    /*                                                                     */
    d=0.0;
    for(i=TOT_PAYM_0; i<=TOT_PAYM_9; i++) {
      d+=tot_ret_double(i);
    }
    if ( mul*fabs( d+amount ) > MAX_AMOUNT_ON_POS ) {
      err_invoke(VALUE_TOO_LARGE);
      *data=_T('\0');
      reset_extra_amount();
      return(UNKNOWN_KEY);
    }


    /* Begin Optional Cheque validation.                                   */
    if (paymcd==PAYM_WAY_2 && oprn_is_initialised(SLIP_PRINTER1)==TRUE) {
      if (( amount > cust.appr_cheque_amount) ||
          ( amount > paym.paym_limit)) {
        if (err_invoke(VALIDATE_CHEQUE)==SUCCEED) {
          cheque_amount = amount;
          print_ch_side_1(SLIP_PRINTER1);
        } 
        else {                                        /* key NO pressed */
          *data=0;
          reset_extra_amount();
          return(UNKNOWN_KEY);
        } /* if err */
      } 
      else {                                  /* No bank approval needed */
        cheque_amount = amount;
        print_ch_side_1(SLIP_PRINTER1);
      } /* if amnt */
      if ( cheque_amount == 0.0 ) {  /* Something went wrong with printing */
        *data=0;
        reset_extra_amount();
        return(UNKNOWN_KEY);
      } /* if che */
    } /* if paym */
    /* End Optional Cheque validation.                                     */

    /*                                                                     */
    /* Entered amount is less-equal the limit for the chosen paymentway    */
    /* or in case of a cheque it might be allowed by the Supervisor to     */
    /* exceed the limit.                                                   */
    /*                                                                     */

    tot_add_double((short)(TOT_PAYM_0 + paymcd), amount);  /* This is foreign money */
    tot_reset_double(TOT_PAID);
                  /* First reset your TOT_PAID and then calculate it again */
    tot_add_double(TOT_PAID,(double) calc_tot_paid() );

    /*                                                                     */
    /* In case of credit-cards                                             */
    /*                                                                     */
    if (paym.extra_perc != 0 || paym.min_extra_amount != 0) {
      d =tot_ret_double(TOT_LCREDIT_AMNT);
      d1=tot_ret_double(TOT_LCREDIT_VAT_AMNT);
      view_tender_scrn(VIEW_AMNT_DUE, (OPER_SCRN | CUST_SCRN) );
      if (err_invoke(ACCEPT_CREDIT_CARD)==SUCCEED) {
        tot_add_double(TOT_GEN_INCL,           dl+dl1);
        tot_add_double(TOT_GEN_EXCL,           dl);
        tot_add_double((short)(TOT_EXCL_0+paym.vat_no), dl);
        tot_add_double((short)(TOT_VAT_0 +paym.vat_no), dl1);
        tot_add_double((short)(TOT_INCL_0+paym.vat_no), dl+dl1);
        tot_add_double(TOT_GEN_VAT,            d1);
      }
      else {
        tot_add_double((short)(TOT_PAYM_0+paymcd), amount*-1.0);
        tot_reset_double(TOT_PAID);
        tot_add_double(TOT_PAID, (double)calc_tot_paid() );
        reset_extra_amount();
        *data=_T('\0');
        reset_to_local_curr();
        view_tender_scrn(VIEW_AMNT_DUE, (OPER_SCRN | CUST_SCRN) );
        return(UNKNOWN_KEY);
      } /* if */
    } /* if */
                        /* No reset to local currency by paym type cheques */
    if (*paym.paym_type != PAYM_TYPE_CHEQUE) {
      reset_to_local_curr();
    }
    *data=_T('\0');
    status=vfy_amount_due();            /* returns AMNT_(NOT_)ENOUGH       */
  }
  else {
    reset_extra_amount();
    status=UNKNOWN_KEY;
  }

  return(status);
} /* vfy_paymentway */

/*---------------------------------------------------------------------------*/
/*                             book_voucher                                  */
/*---------------------------------------------------------------------------*/
short book_voucher(short paymcd, double paym_amount, short amnt_eq_due)
{
  short       i, mul;
  PAYMENT_DEF paym;
  double      d_tmp;

  paym.paym_cd = paymcd;

  if (get_paym(&paym) != SUCCEED) {
    err_invoke(PAYMENT_NOT_PRESENT);
    return(UNKNOWN_KEY);
  }

    /* If paym_amount should equal the due amount, it must now be recalculated */
  if (amnt_eq_due) 
  {
    paym_amount = floor_price(amount_due()*paym.curr_rate/paym.curr_standard);
  }

    /*                                                                     */
    /* Check for overflow already paid amount for requested payment        */
    /* way plus entered amount.                                            */
    /*                                                                     */
  mul = 1;
  if (genvar.ind_price_dec == DECIMALS_YES) {
    mul = 100;
  }
  if (mul*fabs(tot_ret_double((short)(TOT_PAYM_0+paymcd))+paym_amount) > MAX_AMOUNT_ON_POS) {

    err_invoke(VALUE_TOO_LARGE);
    return(UNKNOWN_KEY);
  }

    /*                                                                     */
    /* Check for overflow sum of paymentways plus current amount.          */
    /*                                                                     */
  d_tmp = 0.0;
  for (i=TOT_PAYM_0; i<=TOT_PAYM_9; i++) {
    d_tmp += tot_ret_double(i);
  }
  if ( mul*fabs( d_tmp+paym_amount ) > MAX_AMOUNT_ON_POS ) {
    err_invoke(VALUE_TOO_LARGE);
    return(UNKNOWN_KEY);
  }

    /*                                                                     */
    /* Finally book payment.                                               */
    /*                                                                     */
  tot_add_double((short)(TOT_PAYM_0 + paymcd), paym_amount);  /* This is foreign money */
  tot_reset_double(TOT_PAID);
  tot_add_double(TOT_PAID, calc_tot_paid() );

  reset_to_local_curr();

  return(vfy_amount_due());               /* returns AMNT_(NOT_)ENOUGH       */
} /* book_voucher */

/*---------------------------------------------------------------------------*/
/*                                amount_due                                 */
/*---------------------------------------------------------------------------*/
/* v3.4.5 acm -{
double amount_due(void)
{  
  return ((tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))
                                       - tot_ret_double(TOT_PAID))+pos_invoice.invoice_donation);
}*/
/* amount_due */

double get_invoice_perception_currmode()
{
 
    short  mode         = invoice_mode;
    double perception   = 0.0;

    mode = pos_system.current_mode;
    mode = invoice_mode;

    ///floor_price(TOT_GEN_PERCEPTION)
    perception=pos_invoice.invoice_percep_amount;  //pos_invoice.invoice_donation;
/*
    if ( mode == RETURN )
    {
        perception=-perception;
    }*/
    return perception;
}


double get_invoice_donation_currmode()
{
 
    double donation=pos_invoice.invoice_donation;

    if (pos_system.current_mode==RETURN)
    {
        donation=-donation;
    }
    return donation;
}

double amount_due(void)
{  
    //v3.4.5 acm - Add floor_price in function amount_due for Fixed amount_due calculation 
  return (
      floor_price(tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))
      ///+ floor_price(TOT_GEN_PERCEPTION) // v3.6.1 acm -
      +get_invoice_perception_currmode() - tot_ret_double(TOT_PAID))
      + get_invoice_donation_currmode()
         );
}
/* v3.4.5 acm -}

/*---------------------------------------------------------------------------*/
/*                              calc_tot_paid                                */
/*---------------------------------------------------------------------------*/
static double calc_tot_paid(void)
{
  short i;
  double Tot_sub, Part;
  PAYMENT_DEF paym;

  Tot_sub = 0.0;
  for(i=PAYM_WAY_0; i<MAX_PAYM_WAYS; i++) {
    paym.paym_cd=i;
    if (get_paym(&paym) == SUCCEED) {
      if (paym.paym_limit != (double)0 ) {
        Part = tot_ret_double((short)(TOT_PAYM_0+i));
        Tot_sub = Tot_sub + ((Part * paym.curr_standard)/paym.curr_rate);
      }
    }
  }

  return ( (double) Tot_sub );
} /* calc_tot_paid */


extern int get_num_cupon_cine(); //v3.5 acm -
extern int get_num_vale_pavo(); //v3.5 acm -
/*---------------------------------------------------------------------------*/
/*                             close_and_log_invoice                         */
/*---------------------------------------------------------------------------*/
extern short close_and_log_invoice(_TCHAR *data, short key)
{
  PAYMENT_DEF paym;
  _TCHAR   buffer[19];
  double d, dd;
  short  i;
  double min_diff;
  double tot_incl_for_calcupon=0; /* AC2012-003 acm - */
  double amount_due__=0;  // v3.4.7 acm -
  int ix;

  /*                                                                       */
  /* Close and log the current invoice.                                    */
  /* Key is the last key entered (VOID_INVOICE_KEY or other).              */
  /*                                                                       */
  /*   Normal (not voided) invoice:                                        */
  /*   Open drawer, recalculate totals, print lines, update shift-totals   */
  /*   and save the shift-data to disk.                                    */
  /*   Log invoice lines to BO via SC.                                     */
  /*                                                                       */
  /*   Voided invoice:                                                     */
  /*   Print void-string, update shift-totals and save the shift-data to   */
  /*   disk.                                                               */
  /*                                                                       */

  INVOICE_VALID=0; /* AC2012-003 acm - */

  if (key!=VOID_INVOICE_KEY) {              /* Not a voided invoice        */

    INVOICE_VALID=1; /* AC2012-003 acm - */

    display_working(YES);
    rs_open_cash_drawer(CASH_DRAWER1);
    recalc_subtotals();

    /*                                                                     */
    /* Copy invoice handling:                                              */
    /*                                                                     */
    /*   The special copy invoice text is printed where normaly the promo- */
    /*   tional text is printed. Because the promo text for the NEXT       */
    /*   invoice is printed in the CURRENT invoice. So special arangements */
    /*   are needed to print the copy invoice text after the total lines   */
    /*   of the normal invoice, and print normal promotional text after    */
    /*   the total lines of the copy invoice.                              */
    /*   The indicator copy_invoice_active (YES/NO) indicates if the       */
    /*   special copy invoice text must be printed. This indicator is      */
    /*   toggled in pos_bp1.c function bp_i_total_lines().                 */
    /*                                                                     */


#ifdef PRN_IMM
    print_total_msam();

    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_TOTAL, 0);
      bp_now(BP_SMALL_INVOICE, BP_SMALL_TAX, 0);
      bp_now(BP_SMALL_INVOICE, BP_SMALL_PROMO, 0);
      bp_now(BP_SMALL_INVOICE, BP_SMALL_FOOTER, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_TOTAL_LINES, 0);
      bp_now(BP_INVOICE, BP_FORWARD_SKIP, 0);
    }

    if (copy_invoice == YES) {
      err_invoke(BEFORE_COPY_INVOICE);
      copy_invoice_active = YES;
      print_invoice_type_unsorted();
      err_invoke(AFTER_COPY_INVOICE);
      copy_invoice_active = NO;
      copy_invoice = NO;
    }
#else
    printf_log("antes accumulate_invoice[%s][%d]", __FILE__, __LINE__);
    accumulate_invoice();
    display_prompt(prompt_TXT[4], ERROR_WINDOW_ROW1);
    display_prompt_right(prompt_TXT[5], ERROR_WINDOW_ROW1);

    /* AC2012-003 acm - { set values before cre_invoice */
    if (isvigente_promotion())
    {
        tot_incl_for_calcupon     = tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL));
        pos_invoice.invoice_cupon = get_num_cupon(tot_incl_for_calcupon);
        pos_invoice.invoice_gift  = get_num_gift(tot_incl_for_calcupon);
    }
	
	//mlsd FE
	strcpy(pos_invoice.invoice_serie, de_serie);
	pos_invoice.invoice_correlative = de_correlative;
     /* AC2012-003 acm - }*/ 

    /* v3.6.1 acm - {*/ 
    //pos_invoice.invoice_percep_amount = 0;
    pos_invoice.invoice_percep_custdoc[0]  = 0;    /* v3.6.1 acm - */
    pos_invoice.invoice_percep_custname[0] = 0;    /* v3.6.1 acm - */


    if (IS_GEN_PERCEPTION)
    {
//++        pos_invoice.invoice_percep_amount = TOT_GEN_PERCEPTION;
      strcpy(pos_invoice.invoice_percep_custdoc,  usr_perception_document );  //dni
      strcpy(pos_invoice.invoice_percep_custname, usr_perception_name);  //Nombre
    }
    else if((ispassday() && (tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL)) > 700.0))){
      strcpy(pos_invoice.invoice_percep_custdoc, usr_document );  //dni
      strcpy(pos_invoice.invoice_percep_custname, usr_name);
    }
    /* v3.6.1 acm - }*/ 

    /* v3.4.7 acm - {*/ 
    pos_invoice.invoice_cupon_cine = get_num_cupon_cine();
    /* v3.4.7 acm - }*/ 

    /* v3.4.7 acm - {*/ 
    if (get_num_vale_pavo())
    {
       //v3.5.2 acm -
	    pos_invoice.invoice_vale_pavo = get_valeturkey_count();
    }
    /* v3.4.7 acm - }*/ 

    /* v3.6.0 acm - {*/ 
    { 
      int num_cupon_feria_escolar=get_num_cupon_FERIA_ESCOLAR();

      if (num_cupon_feria_escolar)
      {
        pos_invoice.invoice_feria_escolar = num_cupon_feria_escolar;
      }
    }
    /* v3.6.0 acm - }*/ 


    /* v3.6.0 acm - {*/ 
    { 
      int num_cupon_fiesta_futbol=get_num_cupon_FIESTA_FUTBOL();

      if (num_cupon_fiesta_futbol)
      {
        pos_invoice.invoice_fiesta_futbol = num_cupon_fiesta_futbol;
      }
    }
    /* v3.6.0 acm - }*/ 

    { 
           for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
           {  
				int num_cupon_global=0;
				if (cupon_global_vigente(ix))
                {
                    num_cupon_global=get_num_cupon_CUPON_GLOBAL(ix);
                }else {
					pos_invoice.invoice_cupon_global[ix] = 0;
				}
               pos_invoice.invoice_cupon_global[ix] = num_cupon_global;
            }
    }


    /* v3.4.7 acm - {*/ 
    amount_due__=amount_due();
    if (amount_due__!=0.0) 
    {
        pos_invoice.invoice_rounded = get_invoice_rounded(amount_due__);
    }
    /* v3.4.7 acm - }*/ 

    
    if (copy_invoice == YES) {
      copy_invoice_active = NO;         /* No optional 'copy' text printed */
      print_invoice();                  /*   print the normal invoice.     */
      err_invoke(BEFORE_COPY_INVOICE);
      copy_invoice_active = YES;        /* Optional 'copy' text printed    */
      print_invoice();                  /*   print the copy invoice.       */
      err_invoke(AFTER_COPY_INVOICE);
      copy_invoice_active = NO;
      copy_invoice = NO;
    }
    else {
      print_invoice();
    }

    display_working(YES);
#endif

    view_tender_scrn(VIEW_AMNT_CHANGE, (OPER_SCRN | CUST_SCRN) );

    /* If amount due is negative (change) substract change from            */
    /* cash payment-type.                                                  */

    if ((d=amount_due()) < (double)0 ) {
      used_curr_type = get_paym_cd_cash();
      paym.paym_cd = used_curr_type;
      get_paym(&paym);

      /* Calculates if the amount-paid >= total-amount.                    */

      if (genvar.ind_price_dec == DECIMALS_YES) {
        min_diff = -0.005;
      }
      else {
        min_diff = -0.5;
      }

      if ( d <= min_diff )  {       /* 04/94 Look for change. If 1.123     */
        tot_add_double((short)(TOT_PAYM_0+used_curr_type), (d*paym.curr_rate)/paym.curr_standard);
        tot_add_double(TOT_CHANGE, d);
        tot_add_double(TOT_CHANGE_CD, (double)used_curr_type);
      }
    }

    reset_to_local_curr();


    /* Log invoice to BO.                                                  */
    if (train_mode == CASH_NORMAL) {      /* don't send in training mode   */
       cre_invoice(NORMAL_INVOICE);
    }



    /* Update shift-totals                                                 */
    c_shft.nbr_inv_lines    +=nbr_inv_lines;
    c_shft.nbr_void_lines   +=nbr_void_lines;
    c_shft.donation         +=get_invoice_donation_currmode(/*v3.4.5 acm -*/);

    c_shft.cupon+=pos_invoice.invoice_cupon;           /* AC2012-003 acm - */
    c_shft.rounded+=pos_invoice.invoice_rounded;       /* v3.4.7 acm - */
    c_shft.cupon_cine+=pos_invoice.invoice_cupon_cine;           /* AC2012-003 acm - */

    c_shft.vale_pavo +=pos_invoice.invoice_vale_pavo;           /* AC2012-003 acm - */

    c_shft.feria_escolar +=pos_invoice.invoice_feria_escolar;           /* AC2012-003 acm - */

    if (IS_ANTICIPO()){
        c_shft.percep_amount +=
            (get_invoice_perception_currmode()
            - cur_anticipo.percepcion
            ); ///pos_invoice.invoice_percep_amount;          //v3.6.1 acm - //3.6.2 acm -
            
    }else {
        c_shft.percep_amount +=get_invoice_perception_currmode(); ///pos_invoice.invoice_percep_amount;          //v3.6.1 acm - //3.6.2 acm -
    
    }
    



    c_shft.fiesta_futbol +=pos_invoice.invoice_fiesta_futbol;           /* AC2012-003 acm - */

    for (ix=0; ix<CUPON_GLOBAL_MAX; ix++)  
    {
            c_shft.cupon_global[ix] +=pos_invoice.invoice_cupon_global[ix];
    }
    /* Here you calculated your end_total and end_totals per amount        */
    for (i=0; i<=9; i++) {
      dd = tot_ret_double((short)(TOT_PAYM_0+i));
      c_shft.end_float+=dd;  /* dit is een bug JR */
      c_shft.paym_amnt[i]+=dd;
    }

    tot_add_double(AMOUNT_IN_DRAWER, tot_ret_double((short)(TOT_PAYM_0 + get_paym_cd_cash())));

    /* Increment and save invoice number.                                  */
    /*                                                                     */
    /* Example:   shift-on =  1                                            */
    /*            invoice  =  1..3                                         */
    /*            shift-off=  4                                            */

    inc_invoice_no();
    get_invoice_no(buffer);
    c_shft.invoice_off=_ttol(buffer);

    tm_upda(TM_SHFT_NAME, (void*)&c_shft);
    recover_shift_updt();

    display_working(NO);
    wait_for_closed_drawer(input_TXT[29], (_TCHAR *)NULL);

    if (tot_ret_double(AMOUNT_IN_DRAWER) > genvar.max_amnt_drawer) {
      err_invoke(EXCEED_MAX_AMOUNT_CASHDRAWER);
    }
  }
  else {
    /*                                                                     */
    /* Voided invoice.                                                     */
    /*                                                                     */

    voided_invoice = YES;
    display_working(YES);

#ifdef PRN_IMM
    if (GetPrinterSize(selected_printer)==PRINTER_SIZE_SMALL) {
      bp_now(BP_SMALL_INVOICE, BP_SMALL_VOID, 0);
    }
    else {
      bp_now(BP_INVOICE, BP_VOID_INV, 0);
      bp_now(BP_INVOICE, BP_FORWARD_SKIP, 0);
    }
#else
    printf_log("antes accumulate_invoice[%s][%d]", __FILE__, __LINE__);
    accumulate_invoice();
    display_prompt(prompt_TXT[4], ERROR_WINDOW_ROW1);
    display_prompt_right(prompt_TXT[5], ERROR_WINDOW_ROW1);

    if (copy_invoice == YES) {
      copy_invoice_active = NO;         /* No optional 'copy' text printed */
      print_invoice();                  /*   print the normal invoice.     */
      err_invoke(BEFORE_COPY_INVOICE);
      copy_invoice_active = YES;        /* Optional 'copy' text printed    */
      print_invoice();                  /*   print the copy invoice.       */
      err_invoke(AFTER_COPY_INVOICE);
      copy_invoice_active = NO;
      copy_invoice = NO;
    }
    else {
      print_invoice();
    }

    display_working(YES);
#endif

    /* Reset fee_amount and fee_status */
    pos_invoice.invoice_fee_amount = 0.0;
    pos_invoice.invoice_fee_status = FEE_STAT_NONE;
    if (train_mode == CASH_NORMAL) {        /* don't send in training mode */
      cre_invoice(VOIDED_INVOICE);
    }

    /* Update shift-totals                                                 */

    c_shft.nbr_void_inv++;

    /* Increment and save invoice number.                                  */
    /*                                                                     */
    /* Example:   shift-on =  1                                            */
    /*            invoice  =  1..3                                         */
    /*            shift-off=  4                                            */

    inc_invoice_no();
    get_invoice_no(buffer);
    c_shft.invoice_off=_ttol(buffer);

    tm_upda(TM_SHFT_NAME, (void*)&c_shft);
    recover_shift_updt();

    init_invoice(empty,0);
    init_cust_rec(&cust);

    display_working(NO);

#ifdef PRN_IMM
    err_invoke(REMOVE_INVOICE_MSG);
#endif
  }
  
  /* Removing scrolling lines from scrl-mgr (does not clear window!)       */
  scrl_delete(INV_ART_LIST_WINDOW);
  scrn_clear_window(INV_ART_LIST_WINDOW);

  voided_invoice=NO;
  subt_display=NO;

  cdsp_clear();
  cls();

  return(key);
} /* close_and_log_invoice */

/*---------------------------------------------------------------------------*/
/*                                save_invoice                               */
/*---------------------------------------------------------------------------*/
extern short save_invoice(_TCHAR *data, short key)
{
  short status, save;
  POS_PEND_CUST_DEF pend_cust_rec;

  /*                                                                       */
  /* Save the current invoice.                                             */
  /* Key is the last key entered (SAVE_INVOICE_KEY).                       */
  /*                                                                       */
  /*   Save invoice:                                                       */
  /*   save the article lines to disk.                                     */
  /*                                                                       */

  memset(&pend_cust_rec, 0, POS_PEND_CUST_SIZE);
  pend_cust_rec.cust.cust_key = (cust.store_no * 1000000L) + cust.cust_no;

  save = FALSE;

  status = pos_get_rec(PEND_TYPE, POS_PEND_CUST_SIZE, PEND_CUST_IDX,
                       (void*)&pend_cust_rec, (short)keyEQL);
  if (status == SUCCEED) {
    if (err_invoke(OVERWRITE_SAVED_INVOICE)==SUCCEED) {
      save = TRUE;
    }
  }
  else {
    if (err_invoke(VALIDATE_SAVE_INVOICE)==SUCCEED) {
      save = TRUE;
    }
  }

  if (save) {

    display_working(YES);

    save_article_lines();

    /* Reset fee_amount and fee_status */
    pos_invoice.invoice_fee_amount = 0.0;
    pos_invoice.invoice_fee_status = FEE_STAT_NONE;

    init_invoice(empty,0);
    init_cust_rec(&cust);

    display_working(NO);

    /* Removing scrolling lines from scrl-mgr (does not clear window!)       */
    scrl_delete(INV_ART_LIST_WINDOW);
    scrn_clear_window(INV_ART_LIST_WINDOW);

    subt_display=NO;
    cdsp_clear();
    cls();

    return(key);
  }

  return(UNKNOWN_KEY);
} /* save_invoice */

/*---------------------------------------------------------------------------*/
/*                              reprint_invoice                              */
/*---------------------------------------------------------------------------*/
short reprint_invoice(_TCHAR *data, short key)
{
  PAYMENT_DEF paym;
  short i;

  /* Reprint the current invoice. In case a PAYM_TYPE_INTERNAL is used, a    */
  /* copy invoice must be printed.                                           */

  copy_invoice = NO;
  for (i=0; i <= 9 && copy_invoice == NO; i++) {
    if (tot_ret_double((short)(TOT_PAYM_0 + i)) != 0.0) {
      paym.paym_cd = i;
      if (get_paym(&paym) == SUCCEED) {
        if (*paym.paym_type == PAYM_TYPE_INTERNAL) {
          copy_invoice = YES;
        }
      }
    }
  }
  dec_invoice_no();
#ifdef PRN_IMM
  copy_invoice_active = NO;           /* No optional 'copy' text printed */
  print_invoice_type_unsorted();      /*   print the normal invoice.     */
  if (copy_invoice == YES) {
    err_invoke(BEFORE_COPY_INVOICE);
    copy_invoice_active = YES;        /* Optional 'copy' text printed    */
    print_invoice_type_unsorted();    /*   print the copy invoice.       */
    err_invoke(AFTER_COPY_INVOICE);
    copy_invoice_active = NO;
    copy_invoice = NO;
  }
#else
  if (copy_invoice == YES) {
    copy_invoice_active = NO;         /* No optional 'copy' text printed */
    print_invoice();                  /*   print the normal invoice.     */
    err_invoke(BEFORE_COPY_INVOICE);
    copy_invoice_active = YES;        /* Optional 'copy' text printed    */
    print_invoice();                  /*   print the copy invoice.       */
    err_invoke(AFTER_COPY_INVOICE);
    copy_invoice_active = NO;
    copy_invoice = NO;
  }
  else {
    bot_copy_invoice_active = YES;
    print_invoice();
    bot_copy_invoice_active = NO;
  }
#endif
  inc_invoice_no();

  return(key);
} /* reprint_invoice */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   SHOW CHANGE                                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* At this point the customer paid enough or in case of return mode          */
/* the customer gets money back.                                             */
/* First ask for confirmation of closing the invoice.                        */
/* The drawer is opened, and the change amount is displayed.                 */
/* After closing the drawer the invoice is closed, logged and cleared. After */
/* that the customer mode is entered (state 12).                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static short Choose_Chng(_TCHAR *data, short key)
{
  short paymcd;
  PAYMENT_DEF paym;

  if ((paymcd=payment_cd(key)) != -1) {   /* A payment-key is pressed      */
    paym.paym_cd=paymcd;
    if (get_paym(&paym) == SUCCEED) {
      /* Check if you received more foreign money than you would like to sell !! */
      if ( (((amount_due()*paym.curr_standard)/paym.curr_rate +
            tot_ret_double((short)(TOT_PAYM_0+paymcd))) >= 0.0) ||
           (_tcscmp(paym.paym_type,_T("1")) == 0) ) {
        used_curr_type = paymcd;
        SetShowCursor(FALSE);
        view_tender_scrn(VIEW_AMNT_DUE, (CUST_SCRN | OPER_SCRN) );
        scrn_clear_window(TOTAL_INPUT_WINDOW);
        scrn_string_out(input_TXT[26],0,3); /* CLOSE INVOICE Y/N?  */
        SetShowCursor(TRUE);
        return(UNKNOWN_KEY);
      }
      else {
        err_invoke(TO_LESS_FOREIGN);
        reset_to_local_curr();
        return(UNKNOWN_KEY);
      }

    }
    else {
      err_invoke(PAYMENT_NOT_PRESENT);
      reset_to_local_curr();
      return(UNKNOWN_KEY);
    }
  }

  return (UNKNOWN_KEY);
} /* Choose_Chng() */


static void ShowChange_VW(void)
{
  static short no_cls_states[]={
       ST_DO_TOTAL
      ,0
  };

  if (called_by_state(no_cls_states)==FAIL) {
    cls();
  }

  view_tender_scrn(VIEW_AMNT_DUE, (CUST_SCRN | OPER_SCRN) );
  scrn_clear_window(TOTAL_INPUT_WINDOW);
  scrn_string_out(input_TXT[26],0,3);     /* CLOSE INVOICE Y/N?            */
} /* ShowChange_VW */


static void ShowChange_UVW(void)
{
  SetShowCursor(FALSE);
  scrn_clear_window(TOTAL_INPUT_WINDOW);
} /* ShowChange_UVW */


static VERIFY_ELEMENT ShowChange_VFY[] =
{
  NO_KEY,            (void *)NULL,                   /* Cancel tendering     */
  ENTER_KEY,         (void *)vfy_valepavo_save,  // v3.5.1 acm -
  //ENTER_KEY,         (void *)NULL,  
  VOID_INVOICE_KEY,  (void *)NULL,
  SAVE_INVOICE_KEY,  (void *)NULL,
  PAYMENT_WAY_0_KEY, Choose_Chng,
  PAYMENT_WAY_1_KEY, Choose_Chng,
  PAYMENT_WAY_2_KEY, Choose_Chng,
  PAYMENT_WAY_3_KEY, Choose_Chng,
  PAYMENT_WAY_4_KEY, Choose_Chng,
  PAYMENT_WAY_5_KEY, Choose_Chng,
  PAYMENT_WAY_6_KEY, Choose_Chng,
  PAYMENT_WAY_7_KEY, Choose_Chng,
  PAYMENT_WAY_8_KEY, Choose_Chng,
  PAYMENT_WAY_9_KEY, Choose_Chng,
  OPEN_DRAWER_KEY,   open_and_close_drawer,     /* Approval in state-engine. */
  UNKNOWN_KEY,       illegal_fn_key
};


static PROCESS_ELEMENT ShowChange_PROC[] =
{
  NO_KEY,           cancel_tendering,
  ENTER_KEY,        (void *)NULL,
  VOID_INVOICE_KEY, close_and_log_invoice,
  SAVE_INVOICE_KEY, save_invoice,
  UNKNOWN_KEY,      (void *)NULL
};


static CONTROL_ELEMENT ShowChange_CTL[] =
{
  NO_KEY,           &Invoicing_ST,
  ENTER_KEY,        &AskForDonation_ST,
  VOID_INVOICE_KEY, &CustomerMode_ST,
  SAVE_INVOICE_KEY, &CustomerMode_ST,
  UNKNOWN_KEY,      &ShowChange_ST
};


extern STATE_OBJ ShowChange_ST =
{
  ST_SHOW_CHANGE,
  ShowChange_VW,
  no_DFLT,
  &DYN1K1n2,
  ShowChange_VFY,
  ShowChange_UVW,
  ShowChange_PROC,
  ShowChange_CTL
};

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   VOUCHER                                                 */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* At this point the customer wants to pay with a voucher.                   */
/* The cashier has to enter the voucher number in format:                    */
/*   TTIIIIII                                                                */
/* T= Till number     2 positions                                            */
/* I= Invoice number  6 positions                                            */
/* This number will be checked at backoffice.                                */
/*---------------------------------------------------------------------------*/

static void DoVoucher_VW(void)
{
  static short no_cls_states[]={
       ST_DO_VOUCHER
      ,0
  };

  if (called_by_state(no_cls_states)==FAIL) {
    cls();
  }

  view_tender_scrn(VIEW_AMNT_DUE, (CUST_SCRN | OPER_SCRN) );
  scrn_clear_window(TOTAL_INPUT_WINDOW);
  format_display(&dsp_voucher8, empty);
  scrn_string_out(input_TXT[46],0,19+6);     /* ENTER VOUCHER NUMBER  */
} /* Voucher_VW */


static void DoVoucher_UVW(void)
{
  SetShowCursor(FALSE);
  scrn_clear_window(TOTAL_INPUT_WINDOW);
} /* Voucher_UVW */


static VERIFY_ELEMENT DoVoucher_VFY[] =
{
  NO_KEY,            (void *)NULL,                   /* Cancel tendering     */
  ENTER_KEY,         vfy_voucher,
  VOID_INVOICE_KEY,  (void *)NULL,
  CLEAR_KEY,         vfy_clear_key,
  UNKNOWN_KEY,       illegal_fn_key
};

extern INPUT_CONTROLLER Ddsp_voucher8KOKl8n =
 {
  (INPUT_DISPLAY *)&dsp_voucher8,
  KEYBOARD_MASK,
  16, 16,//9, 9,
  //(VERIFY_KEY *)&numeric_punct
  (VERIFY_KEY *)&printing_char_upr // (VERIFY_KEY *)&numeric_punct
};

static PROCESS_ELEMENT DoVoucher_PROC[] =
{
  UNKNOWN_KEY,      (void *)NULL
};


static CONTROL_ELEMENT DoVoucher_CTL[] =
{
  NO_KEY,           &DoTotal_ST,
  AMNT_ENOUGH,      &ShowChange_ST,
  AMNT_NOT_ENOUGH,  &DoTotal_ST,
  VOUCHER_AMOUNT,   &DoVoucherAmount_ST,
  UNKNOWN_KEY,      &DoVoucher_ST
};


extern STATE_OBJ DoVoucher_ST =
{
  ST_DO_VOUCHER,
  DoVoucher_VW,
  no_DFLT,
  &Ddsp_voucher8KOKl8n,
  DoVoucher_VFY,
  DoVoucher_UVW,
  DoVoucher_PROC,
  DoVoucher_CTL
};


/*---------------------------------------------------------------------------*/
/*                                vfy_voucher                                */
/*---------------------------------------------------------------------------*/
short vfy_voucher(_TCHAR *data, short key)
{
  long             voucher_no;
  VOUCHER_DEF      voucher;
  int  handled;

/* The voucher number is entered in the folowing format TTIIIIII
 *
 *  T till number       2 positions
 *  I invoice number    6 positions
 *
 */


  /* Check length, must be 8, TTIIIIII */
  if (_tcslen(data) != 8) 
  {
    key=vfy_voucher_nc(data, key, &handled);
    if (handled) 
        return key;
   
    err_invoke(VOUCHER_LENGTH);
    *data = _T('\0');
    return (UNKNOWN_KEY);
  }

/*
 *  We use a voucher prefix to be partly compatible with Argentina, it is fixed to 1.
 */

  voucher_no                = _ttol(data);

  voucher_items.seq_no      = VOUCHER_PREFIX;
  voucher_items.paym_cd     = used_curr_type;
  _tcscpy(voucher_items.id, data);

  /* Connect to backoffice -> send  prefix, sequence number, amount             */
  /*                       the status is checked and changed on backoffice      */
  /*                       <-  payment type is returned                         */
   
  if (get_voucher_bo(voucher_no, (long)voucher_items.seq_no, NOT_USED, cust.store_no,
                     cust.cust_no, &voucher) != SUCCEED) {

    if (err_invoke(NO_CONNECTION) == SUCCEED) {
      *data = _T('\0');
      return(VOUCHER_AMOUNT);                 /* Goto voucher amount state */
    }
    else {
      *data = _T('\0');
      return(NO_KEY);                                   /* simulate NO-key */
    }
  }

  if (voucher.status != NOT_USED) {
    if (voucher.status == USED ){
      err_invoke(VOUCHER_BLOCKED); 
    }
    else {        /* something went wrong (-1 = not found), end processing */
      err_invoke(VOUCHER_NOT_KNOWN); 
    }
    *data = _T('\0');
    return(NO_KEY);                                     /* simulate NO-key */
  }

  if (voucher.paym_cd != used_curr_type) {
    err_invoke(PAYTYPE_NOT_SAME_SELECT); /* SELECT PAYMENT_CD != AMOUNT+PAYMENT_CD */
    if (get_voucher_bo(voucher.seq_no, (long)voucher_items.seq_no, USED, cust.store_no,
                       cust.cust_no, &voucher) != SUCCEED) {
      err_invoke(NO_CONNECTION);
    }
    *data = _T('\0');
    return(NO_KEY);                                       /* simulate NO-key */
  }
                                           /* VOUCHER IS OKAY, SO PROCESS IT */
            /* Status changed from NOT_USED -> USED on backoffice.           */
            /* Book the amount on the returned paymenttype                   */
  key = book_voucher(voucher.paym_cd, _tcstod(voucher.amount,NULL), 0);
  if (key == UNKNOWN_KEY) {
    if (get_voucher_bo(voucher.seq_no, (long)voucher_items.seq_no, USED, cust.store_no,
                       cust.cust_no, &voucher) != SUCCEED) {
      err_invoke(NO_CONNECTION);
    }
    *data = _T('\0');
    return(NO_KEY);                                   /* simulate NO-key */
  }

  voucher_items.paym_date   = pos_system.run_date;
  voucher_items.paym_amount = _tcstod(voucher.amount, NULL);
  voucher.status = USED;

  sll_add(&payment_items, &voucher_items);/* Add to payment item.            */
  sll_add(&voucher_hist, &voucher);       /* Add to hist for possible cancel */

  return(key);
} /* vfy_voucher */

/*---------------------------------------------------------------------------*/
/*                             cancel_voucher                                */
/*---------------------------------------------------------------------------*/
static short cancel_voucher(short key) 
{
  VOUCHER_DEF voucher;
  VOUCHER_DEF dummy_voucher;
  short i=0;

  while (sll_read(&voucher_hist,i,&voucher) == SUCCEED) {
    if (get_voucher_bo(voucher.seq_no, (long)voucher.prefix, USED, cust.store_no,
                       cust.cust_no, &voucher) != SUCCEED) {
      err_invoke(UNBLOCK_VOUCHER_ERROR);
      break;
    }
    i++;
  }
  if (sll_read(&voucher_hist, 0, &dummy_voucher) == SUCCEED) {
    sll_remove(&voucher_hist);
  }

  return key;
} /* cancel_voucher */

/*---------------------------------------------------------------------------*/
/*                             reset_voucher_items                           */
/*---------------------------------------------------------------------------*/
void reset_voucher_items(void)
{
  INVOICE_PAYM_ITEM_DEF dummy_paym_item;
  VOUCHER_DEF           dummy_voucher;
  VOUCHER_ANTICIPO_DEF  dummy_voucher_anticipo;

  if (sll_read(&payment_items, 0, &dummy_paym_item) == SUCCEED) {
    sll_remove(&payment_items);
  }

  if (sll_read(&voucher_hist, 0, &dummy_voucher) == SUCCEED) {
    sll_remove(&voucher_hist);
  }


  return;
} /* reset_voucher_items */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   VOUCHER_AMOUNT                                          */
/*---------------------------------------------------------------------------*/

static void DoVoucherAmount_VW(void)
{
  scrn_clear_window(TOTAL_INPUT_WINDOW);
  format_display(&dsp_payd_amnt11, empty);
  scrn_string_out(input_TXT[47],0,19);     /* ENTER VOUCHER AMOUNT           */
} /* VoucherAmount_VW */

static void DoVoucherAmount_UVW(void)
{
  SetShowCursor(FALSE);
  scrn_clear_window(TOTAL_INPUT_WINDOW);
} /* VoucherAmount_UVW */

static VERIFY_ELEMENT DoVoucherAmount_VFY[] =
{
  NO_KEY,            (void *)NULL,                   /* Cancel tendering     */
  VOID_INVOICE_KEY,  (void *)NULL,
  CLEAR_KEY,         vfy_clear_key,
  ENTER_KEY,         vfy_voucher_amount,
  UNKNOWN_KEY,       illegal_fn_key
};

static PROCESS_ELEMENT DoVoucherAmount_PROC[] =
{
  UNKNOWN_KEY,      (void *)NULL
};

static CONTROL_ELEMENT DoVoucherAmount_CTL[] =
{
  NO_KEY,           &DoTotal_ST,
  AMNT_ENOUGH,      &ShowChange_ST,
  AMNT_NOT_ENOUGH,  &DoTotal_ST,
  UNKNOWN_KEY,      &DoVoucherAmount_ST
};


extern STATE_OBJ DoVoucherAmount_ST =
{
  ST_DO_VOUCHER_AMOUNT,
  DoVoucherAmount_VW,
  no_DFLT,
  &Ddsp_payd_amnt11K11np,
  DoVoucherAmount_VFY,
  DoVoucherAmount_UVW,
  DoVoucherAmount_PROC,
  DoVoucherAmount_CTL
};

/*---------------------------------------------------------------------------*/
/*                            vfy_voucher_amount                             */
/*---------------------------------------------------------------------------*/
static short vfy_voucher_amount(_TCHAR *data, short key)
{
  VOUCHER_DEF voucher;

  if (*data == _T('\0')) {
    return(NO_KEY);
  }
                /* Process amount. used_curr_type contains used payment code */
  key = book_voucher(used_curr_type, _tcstod(data,NULL), 0);

  if (key == UNKNOWN_KEY) {
    *data = _T('\0');
    return(NO_KEY);                                       /* simulate NO-key */
  }

  voucher_items.paym_date   = pos_system.run_date;
  voucher_items.paym_amount = _tcstod(data, NULL);

  voucher.prefix   = voucher_items.seq_no;
  voucher.paym_cd  = voucher_items.paym_cd;
  voucher.cust_no  = cust.cust_no;
  voucher.store_no = cust.store_no;
  voucher.status   = USED;
  voucher.seq_no   = (long)_tcstol(voucher_items.id, NULL, 10);
  _tcscpy(voucher.amount, data);

  sll_add(&payment_items, &voucher_items);/* Add to payment item.               */
  sll_add(&voucher_hist, &voucher);       /* Add to hist for possible canceling */
  return(key);

} /* vfy_voucher_amount */

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   ASK_FOR_DONATION                                        */
/*---------------------------------------------------------------------------*/

static void AskForDonation_VW(void)
{
  view_tender_scrn(VIEW_AMNT_DUE, (CUST_SCRN | OPER_SCRN) );
  scrn_clear_window(TOTAL_INPUT_WINDOW);
  scrn_string_out(input_TXT[48],0,3);     /* DO YOU WISH TO DONATE Y/N?      */
} /* Donation_VW */


static void AskForDonation_UVW(void)
{
  SetShowCursor(FALSE);
  scrn_clear_window(TOTAL_INPUT_WINDOW);
} /* Donation_UVW */

static VERIFY_ELEMENT AskForDonation_VFY[] =
{
  NO_KEY,            vfy_choice,
  ENTER_KEY,         (void *)NULL,
  OPEN_DRAWER_KEY,   open_and_close_drawer,     /* Approval in state-engine. */
  ///SEL_PRINTER_KEY,  (void *)vfy_valepavo_save,
  UNKNOWN_KEY,       illegal_fn_key
};

static PROCESS_ELEMENT AskForDonation_PROC[] =
{
  /*NO_KEY,            close_and_log_invoice, */              //v3.6.1 acm -
  SEL_PRINTER_KEY,  /*(void *)NULL, */ close_and_log_invoice, //v3.6.1 acm -
  ENTER_KEY,        (void *)NULL,
  UNKNOWN_KEY,      (void *)NULL
};

static CONTROL_ELEMENT AskForDonation_CTL[] =
{
  NO_KEY,           &CustomerMode_ST,
  SEL_PRINTER_KEY,  /*&SelectPrinter_ST,*/  &CustomerMode_ST, //v3.6.1 acm -
  ENTER_KEY,        &Donation_ST,
  UNKNOWN_KEY,      &AskForDonation_ST
};

extern STATE_OBJ AskForDonation_ST =
{
  ST_ASK_FOR_DONATION,
  AskForDonation_VW,
  no_DFLT,
  &DYN1K1n2,
  AskForDonation_VFY,
  AskForDonation_UVW,
  AskForDonation_PROC,
  AskForDonation_CTL
};

/*---------------------------------------------------------------------------*/
/*                            vfy_choice                                     */
/*---------------------------------------------------------------------------*/
static short vfy_choice(_TCHAR *data, short key)
{
  short i;

  if (printers_attached == (PRINTER_SIZE_SMALL | PRINTER_SIZE_NORMAL)) {
    key = SEL_PRINTER_KEY;
  }
  else if (!(printers_attached & PRINTER_SIZE_NORMAL) && (printers_attached & PRINTER_SIZE_SMALL)) {
    for (i=DEV_PRINTER1; i<NUMBER_OF_PRINTERS; i++) {
      if (pos_printer_info[i].attached == YES || print_to_file==YES) {
        if (pos_printer_info[i].printersize == PRINTER_SIZE_SMALL) {
          selected_printer = i;
        }
      }
    }
  }
  else if ((printers_attached & PRINTER_SIZE_NORMAL) && !(printers_attached & PRINTER_SIZE_SMALL)) {
    for (i=DEV_PRINTER1; i<NUMBER_OF_PRINTERS; i++) {
      if (pos_printer_info[i].attached == YES || print_to_file==YES) {
        if (pos_printer_info[i].printersize == PRINTER_SIZE_NORMAL) {
          selected_printer = i;
        }
      }
    }
  }


  return(key);

} /* vfy_choice */

/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   DONATION                                                */
/*---------------------------------------------------------------------------*/

static void Donation_VW(void)
{
  static short no_cls_states[]={
       ST_DONATION
      ,0
  };

  if (called_by_state(no_cls_states)==FAIL) {
   calc_donation_amount = TRUE;             /* Only if called by other state */
   cls();
  }

  view_tender_scrn(VIEW_AMNT_DUE, (CUST_SCRN | OPER_SCRN) );
  scrn_clear_window(TOTAL_INPUT_WINDOW);
  scrn_string_out(input_TXT[49],0,19);     /* ENTER DONATION AMOUNT          */

  *amount_donation = 0;

} /* Donation_VW */


static void Donation_UVW(void)
{
  SetShowCursor(FALSE);
  scrn_clear_window(TOTAL_INPUT_WINDOW);
} /* Donation_UVW */


static VERIFY_ELEMENT Donation_VFY[] =
{
  NO_KEY,            vfy_donation_amount,
  ENTER_KEY,         vfy_donation_amount,
  CLEAR_KEY,         vfy_clear_key,
  UNKNOWN_KEY,       illegal_fn_key
};

extern INPUT_CONTROLLER DYN1K1n2 =
{
  (INPUT_DISPLAY *)&dsp_YN2,
  KEYBOARD_MASK,
  2, 2,
  (VERIFY_KEY *)&numeric
};

static PROCESS_ELEMENT Donation_PROC[] =
{
  /*NO_KEY,            close_and_log_invoice, */                        //v3.6.1 acm -
  ENTER_KEY,        /* close_and_log_invoice, */ (void *)NULL,          //v3.6.1 acm -
  SEL_PRINTER_KEY,  /* (void *)NULL,          */  close_and_log_invoice,//v3.6.1 acm -
  UNKNOWN_KEY,      (void *)NULL
};


static CONTROL_ELEMENT Donation_CTL[] =
{
  /*NO_KEY,           &CustomerMode_ST, */                      //v3.6.1 acm - 
  ENTER_KEY,       /* &CustomerMode_ST, */  (void *)NULL,       //v3.6.1 acm -
  SEL_PRINTER_KEY, /* &SelectPrinter_ST,*/  &CustomerMode_ST,   //v3.6.1 acm -
  UNKNOWN_KEY,      &Donation_ST
};


extern STATE_OBJ Donation_ST =
{
  ST_DONATION,
  Donation_VW,
  DFLT_donation,
  &Ddsp_payd_amnt11K11np,
  Donation_VFY,
  Donation_UVW,
  Donation_PROC,
  Donation_CTL
};

/*---------------------------------------------------------------------------*/
/*                              DFLT_donation                                */
/*---------------------------------------------------------------------------*/
void DFLT_donation(INPUT_DISPLAY *x, _TCHAR *data)
{
  if (x) {                          /* If the display structure is not NULL  */
    if(*amount_donation && calc_donation_amount == TRUE) {
      x->fn(x, amount_donation);
    }
    else {
      *amount_donation = 0;
      x->fn(x, empty);                        /* Call display function NO data */
    }
  }
  else {
    *amount_donation = 0;
  }
  calc_donation_amount = FALSE;
  SetShowCursor(TRUE);
  return;
} /* DFLT_donation */


/*---------------------------------------------------------------------------*/
/*                            vfy_donation_amount                            */
/*---------------------------------------------------------------------------*/
static short vfy_donation_amount(_TCHAR *data, short key)
{
   _TCHAR   error_message[30];
   short    i;

   if (*amount_donation && !*data) {  /* if amount_donation is calculated and no data is entered */
    _tcscpy(data, amount_donation);   /* take the default value                                  */
  }
  else {
    *amount_donation = _T('\0');
  }
  
  pos_invoice.invoice_donation = 0.0;

  switch(key) {

    case NO_KEY:
      *data = 0;
      break;

    case ENTER_KEY:

      /* Check the donation amount against the lowest value of change_value  */
      /* and max_amnt_donation.                                              */

      if (Dbl_GT((amount_due() * -1), floor_price(genvar.max_amnt_donation), -1)) {

        if (Dbl_GT((double)atof_price(data), floor_price(genvar.max_amnt_donation), -1)) {
          ftoa_price(floor_price(genvar.max_amnt_donation), 18, error_message);
          error_extra_msg=error_message;
          err_invoke(EXCEED_MAX_AMOUNT_DONATION);
          return(UNKNOWN_KEY);
        }
      }
      else {
        if (Dbl_GT((double)atof_price(data), (amount_due() * -1), -1)) {
          err_invoke(EXCEED_CHANGE_VALUE_DONATION);
          return(UNKNOWN_KEY);
        }
      }
   
      if (Dbl_LT((double)atof_price(data), 0.0, -1)) {
        key = UNKNOWN_KEY;
      }
      else {
        pos_invoice.invoice_donation = atof_price(data);
      }

      break;
  }
  
  if (printers_attached == (PRINTER_SIZE_SMALL | PRINTER_SIZE_NORMAL)) { /*FredM*/
    key = SEL_PRINTER_KEY;
  }
  else if (!(printers_attached & PRINTER_SIZE_NORMAL) && (printers_attached & PRINTER_SIZE_SMALL)) {
    for (i=DEV_PRINTER1; i<NUMBER_OF_PRINTERS; i++) {
      if (pos_printer_info[i].attached == YES || print_to_file==YES) {
        if (pos_printer_info[i].printersize == PRINTER_SIZE_SMALL) {
          selected_printer = i;
        }
      }
    }
  }
  else if ((printers_attached & PRINTER_SIZE_NORMAL) && !(printers_attached & PRINTER_SIZE_SMALL)) {
    for (i=DEV_PRINTER1; i<NUMBER_OF_PRINTERS; i++) {
      if (pos_printer_info[i].attached == YES || print_to_file==YES) {
        if (pos_printer_info[i].printersize == PRINTER_SIZE_NORMAL) {
          selected_printer = i;
        }
      }
    }
  }

   // v3.5.1 acm -{
  /*
  if (key == SEL_PRINTER_KEY)
     key=vfy_valepavo_save(data, key);
*/
  // v3.5.1 acm -}

  return(key);

} /* vfy_donation_amount */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   SELECT_PRINTER                                          */
/*---------------------------------------------------------------------------*/

static void SelectPrinter_VW(void)
{
  view_select_printer_scrn();
  view_lan_state(FORCED_UPDATE);        /* update ON- OFFLINE indicator.     */
} /* SelectPrinter_VW */

static void SelectPrinter_UVW(void)
{
  SetShowCursor(FALSE);
} /* SelectPrinter_UVW */

static VERIFY_ELEMENT SelectPrinter_VFY[] =
{                              
  ENTER_KEY,            vfy_select_printer,
  CLEAR_KEY,            vfy_clear_key,
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT SelectPrinter_PROC[] =
{
  SELECT_PRINTER_OK,    close_and_log_invoice,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT SelectPrinter_CTL[] =
{
  SELECT_PRINTER_OK,   &CustomerMode_ST,
  UNKNOWN_KEY,         &SelectPrinter_ST
};

extern INPUT_CONTROLLER Dscanerr1n =
 {
  (INPUT_DISPLAY *)&dsp_select_printer,
  KEYBOARD_MASK,
  1, 2,
  (VERIFY_KEY *)&numeric
};

extern STATE_OBJ SelectPrinter_ST =
{                         
  ST_SELECT_PRINTER,
  SelectPrinter_VW,
  no_DFLT,
  &Dscanerr1n,
  SelectPrinter_VFY,
  SelectPrinter_UVW,
  SelectPrinter_PROC,
  SelectPrinter_CTL
};



//v3.6.1 acm - {
extern short start_invoice(_TCHAR *, short);

static void SelectDocument_VW(void)
{
  view_select_printer_scrn();
  view_lan_state(FORCED_UPDATE);        /* update ON- OFFLINE indicator.     */
} /* SelectDocument_VW */

static void SelectDocument_UVW(void)
{
  //SetShowCursor(FALSE);

  scrn_clear_window(SELECT_PRINTER_WINDOW);
 // scrn_select_window(INV_HEADER_WINDOW);
  SetShowCursor(FALSE);

} /* SelectDocument_UVW */



/*
static PROCESS_ELEMENT CustomerMode_PROC[] =
{
  //CUSTNO_OK,     start_invoice, //v3.6.1 acm  -
  CUSTNO_OK,       start_document, //v3.6.1 acm  -
  NO_KEY,          cancel_cust,
  SELECT_CUST_KEY, (void *)NULL,
  UNKNOWN_KEY,     (void *)NULL
};


static CONTROL_ELEMENT CustomerMode_CTL[] =
{
  
  //CUSTNO_OK,       &Invoicing_ST, //v3.6.1 acm -

*/
static VERIFY_ELEMENT SelectDocument_VFY[] =
{                              
  ENTER_KEY,            vfy_select_printer,
  CLEAR_KEY,            vfy_clear_key,
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT SelectDocument_PROC[] =
{
  SELECT_PRINTER_OK,   /* close_and_log_invoice,*/ start_invoice, //v3.6.1 acm -
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT SelectDocument_CTL[] =
{
  SELECT_PRINTER_OK,  /*&CustomerMode_ST, */ &Invoicing_ST, //v3.6.1 acm -
  UNKNOWN_KEY,         &SelectDocument_ST
};


extern STATE_OBJ SelectDocument_ST =
{                         
  ST_SELECT_DOCUMENT,
  SelectPrinter_VW,
  no_DFLT,
  &Dscanerr1n,
  SelectDocument_VFY,
  SelectDocument_UVW,
  SelectDocument_PROC,
  SelectDocument_CTL
};

//v3.6.1 acm - }

/*---------------------------------------------------------------------------*/
/*                       view_select_printer_scrn                            */
/*---------------------------------------------------------------------------*/
static void view_select_printer_scrn()
{
  short            row;
  _TCHAR           buffer[65];

  scrn_clear_window(SELECT_PRINTER_WINDOW);
  scrn_select_window(SELECT_PRINTER_WINDOW);

  row = 1;
  /*for (i=DEV_PRINTER1; i<NUMBER_OF_PRINTERS; i++) {
    if (pos_printer_info[i].attached == YES || print_to_file==YES) {
      if (pos_printer_info[i].printersize != PRINTER_SIZE_SMALL) {
        _stprintf(buffer, _T("%1d  %-25.25s %-20.20s"), i+1, pos_printer_info[i].printer, scrn_inv_TXT[34]);
      }
      else {
        _stprintf(buffer, _T("%1d  %-25.25s %-20.20s"), i+1, pos_printer_info[i].printer, scrn_inv_TXT[35]);
      }
      scrn_string_out(buffer, row, 5);
      row++;
    }
  }*/

	_stprintf(buffer, _T("%1d  %s"), 1,"FACTURA");
	scrn_string_out(buffer, row++, 5);
	_stprintf(buffer, _T("%1d  %s"), 2,"BOLETA");
    scrn_string_out(buffer, row++, 5);

  row++;
  scrn_string_out(input_TXT[50], 5, 19); /* SELECT PRINTER FOR INVOICE      */
} /* view_select_printer_scrn */

/*---------------------------------------------------------------------------*/
/*                       view_select_invtype_scrn                            */
/*---------------------------------------------------------------------------*/
static void view_select_invtype_scrn()
{
  short             row;
  _TCHAR           buffer[65];

  scrn_clear_window(SELECT_PRINTER_WINDOW);
  scrn_select_window(SELECT_PRINTER_WINDOW);

  row = 1;
  /*for (i=DEV_PRINTER1; i<NUMBER_OF_PRINTERS; i++) {
    if (pos_printer_info[i].attached == YES || print_to_file==YES) {
      if (pos_printer_info[i].printersize != PRINTER_SIZE_SMALL) {
        _stprintf(buffer, _T("%1d  %-25.25s %-20.20s"), i+1, pos_printer_info[i].printer, scrn_inv_TXT[34]);
      }
      else {
        _stprintf(buffer, _T("%1d  %-25.25s %-20.20s"), i+1, pos_printer_info[i].printer, scrn_inv_TXT[35]);
      }
      scrn_string_out(buffer, row, 5);
      row++;
    }
  }*/

	_stprintf(buffer, _T("%1d  %s"), 1,"FACTURA");
	scrn_string_out(buffer, row++, 5);
	_stprintf(buffer, _T("%1d  %s"), 2,"BOLETA");
    scrn_string_out(buffer, row++, 5);

  row++;
  scrn_string_out(input_TXT[50], 5, 19); /* SELECT PRINTER FOR INVOICE      */
} /* view_select_invtype_scrn */


short vfy_valepavo_save(_TCHAR *data, short key)
{
  _TCHAR   linvoice_buffer[100];
   long    linvoice_no; 
   int cc;
  VOUCHER_VALE_DEF   voucher;
  int status;

  

    if ((get_num_vale_pavo()==1)&&
        (get_valeturkey_count()>=1)// v3.5.2 acm -
        )// TIENE ARTICULO VALEPAVO
      {
            get_invoice_no(linvoice_buffer);
            linvoice_no=atol(linvoice_buffer);

            cc=get_voucher_bo_turkey(cur_valepavo.vale_no, 
                                  cur_valepavo.vale_type, 
                                  NOT_USED, 
                                  pos_system.store_no,
                                  cust.cust_no,
                                  pos_invoice.invoice_till_no , //till_no
                                  0, //selected_invoice_printer +1,  //invoice_type
                                  linvoice_no ,                 //invoice_no
                                  pos_invoice.invoice_date,     //invoice_date
                                  pos_invoice.invoice_sequence_no,
                                  &voucher,3 );
            if (cc!=SUCCEED)
            {
                err_invoke(VALEPAVO_CONECTION_ERROR);
                *data = _T('\0');
                return(UNKNOWN_KEY);
            }
            if (voucher.status != NOT_USED) 
            {
                if (voucher.status == USED )
                {
                  err_invoke(VALEPAVO_USED); 
                }
                else if (voucher.status == 98)
                {  
                  err_invoke(VALEPAVO_NOTFOUND); 
                }else if (voucher.status == 99)
                {
                   err_invoke(VALEPAVO_BD_ERR); 
                }else
                    err_invoke(VALEPAVO_UNKNOWN); 

                *data = _T('\0');
                return(UNKNOWN_KEY);                                 /* simulate NO-key */
            }

          /*                                    */
          /* Update cur_valepavo                */
          /*                                    */
                    
                    
          cur_valepavo.status=1;
          status=pos_update_rec(VALEPAVO_PROM_TYPE, VALEPAVO_PROM_SIZE, 
              VALEPAVO_PROM_IDX, VALEPAVO_PROM_FNO,(void*)&cur_valepavo);
          /*
          if( status==SUCCEED ) {
            if ((request->paym_cd >= PAYM_WAY_0) && (request->paym_cd < MAX_PAYM_WAYS)) {
              memcpy(&payment[request->paym_cd],request,POS_PAYM_SIZE);
            }
          }
          */
      }
      return key;
}

/*---------------------------------------------------------------------------*/
/*                          vfy_select_printer                               */
/*---------------------------------------------------------------------------*/
short vfy_select_printer(_TCHAR *data, short key)
{
  short   printer_choice, invoice_type_choice;
  //_TCHAR linvoice_buffer[100];
  //long linvoice_no;
//  VOUCHER_VALE_DEF   voucher;
//  int cc;

  if (!*data) {
    err_invoke(INVALID_KEY_ERROR);
    return UNKNOWN_KEY;
  }



  invoice_type_choice = _ttoi(data)-1;

  printer_choice = 1;

  /*
  if (printer_choice<DEV_PRINTER1 || printer_choice>=NUMBER_OF_PRINTERS) {
    err_invoke(INVALID_KEY_ERROR);
    return UNKNOWN_KEY;
  }
  if (pos_printer_info[printer_choice].attached!=YES) {
    err_invoke(INVALID_KEY_ERROR);
    return UNKNOWN_KEY;
  }
  */

  if (invoice_type_choice<0 || invoice_type_choice>=2) {
    err_invoke(INVALID_KEY_ERROR);
    return UNKNOWN_KEY;
  }
  if(strlen(cust.fisc_no) != 11 && invoice_type_choice == 0) {
    err_invoke(NOT_RUC_CUSTOMER_ERROR);
    return UNKNOWN_KEY;
  }

  /*
      // v3.5.1 acm -{
  if (get_num_vale_pavo()==1)
  {
        
        get_invoice_no(linvoice_buffer);
        linvoice_no=atol(linvoice_buffer);

        cc=get_voucher_bo_turkey(cur_valepavo.vale_no, 
                              cur_valepavo.vale_type, 
                              NOT_USED, 
                              pos_system.store_no,
                              cust.cust_no,
                              pos_invoice.invoice_till_no , //till_no
                              selected_invoice_printer +1,  //invoice_type
                              linvoice_no ,                 //invoice_no
                              pos_invoice.invoice_date,     //invoice_date
                              &voucher);
        if (cc!=SUCCEED)
        {
            err_invoke(VALEPAVO_CONECTION_ERROR);
            *data = _T('\0');
            return(UNKNOWN_KEY);
        }

        if (voucher.status != NOT_USED) {
            if (voucher.status == USED )
            {
              err_invoke(VALEPAVO_USED); 
            }
            else if (voucher.status == 98)
            {  
              err_invoke(VALEPAVO_NOTFOUND); 
            }else if (voucher.status == 99)
            {
               err_invoke(VALEPAVO_BD_ERR); 
            }else
                err_invoke(VALEPAVO_UNKNOWN); 

            *data = _T('\0');
            return(UNKNOWN_KEY);                                 // simulate NO-key 
        }
  }
   */
  
  selected_printer = printer_choice;
  selected_invoice_printer = invoice_type_choice; /*Tipo de pago*/

  return SELECT_PRINTER_OK;
} /* vfy_select_printer */


//mname
short  vfy_Input(_TCHAR *data, short key)
{
    int   ret_key=UNKNOWN_KEY;

    if (pe_val_dni(data))
    {
        ret_key= key;
    } 
    else
    {
        err_invoke(DOCUMENT_DOC_ERR);
        ret_key= UNKNOWN_KEY;
    } 
    return ret_key;
}
short prc_Input(_TCHAR *data, short key)
{
/*    CUST_PERC_DEF cust_perc;
    CARDHOLDER_DEF cardholder;
    char old_document[500];

    strcpy(old_document,"");
    if (usr_document[0] != NULL)
        strcpy(old_document, usr_document);*/

    strcpy(usr_document, data);
/*
    if ( ispassday() )
    {
        // Si es PassDay Buscar el Nombre en la tabla CUST_PERC
        if (getNameFromCust_Perc(data, &cust_perc))
        {
           return prc_Input_name(cust_perc.cust_name,DOCID_FOUND);
        }
        else
        {
            if (strcmp(old_document,usr_document)!=0)
            {
                //si cambio el DNI cambiamos el nombre
                strcpy(usr_name,"");
            }
        }
    }

    if ( isFiscalClient() ) 
    {
        if ( !is_client_pay_FACTURA() )
        {
            // Si es Cliente con RUC y pide boleta Buscar el Nombre en la tabla CARDHOLDER
            if (getNameFromCardHolder(data, &cardholder))
            {
                return prc_Input_name(cardholder.name,DOCID_FOUND);
            }else
            {
                if (strcmp(old_document,usr_document)!=0)
                {
                    //si cambio el DNI cambiamos el nombre
                    strcpy(usr_name,"");                
                }
            }
        }
    }
*/
    //SetKeyMapping(ASCII_KEYS);
//    usr_perception_name    [500];

    
    /*
    if (key==ENTER_KEY)
    {
        if (perception_current_field==1) // procesa dni
        {
           
            Ddsp_perception_document.display=&dsp_perception_name;
            perception_current_field=2;
        }else if (perception_current_field==2) // procesa nombre
        {
            key=ENTER_KEY_TOTAL;    
        }

    }
    */

  return key;
}
static void Passday700Input_VW(void)
{
  static short no_cls_states[]={
       ST_PASSDAY700
      ,0
  };

  if (called_by_state(no_cls_states)==FAIL) {
    cls();
  }
  view_tender_scrn(VIEW_AMNT_DUE, (OPER_SCRN | CUST_SCRN) );
  scrn_clear_window(TOTAL_INPUT_WINDOW);
  
  format_display(&dsp_document, usr_document);
  strcpy(last_document, usr_document);

  print_document();
} /* LogOn_VW */
extern INPUT_CONTROLLER Ddsp_passday700_document =
{
  (INPUT_DISPLAY *)&dsp_document,
  KEYBOARD_MASK,
  8,
  9,
  (VERIFY_KEY *)&numeric
};
static VERIFY_ELEMENT Passday700Input_VFY[] =
{
  
  NO_KEY,             (void *)NULL,
  ENTER_KEY,          (void *)vfy_Input, 
  SAVE_INVOICE_KEY,   (void *)NULL,
  UNKNOWN_KEY,        (void *)NULL,
};
static void Passday700Input_UVW(void)
{
  //ShiftPosCaret((short)(user_shift_pos_caret-1));
  //SetKeyMapping(NORMAL_KEYS);
}
static PROCESS_ELEMENT Passday700Input_PROC[] =
{
    NO_KEY,           cancel_tendering_passday700,
    ENTER_KEY,        (void *)prc_Input,
    UNKNOWN_KEY,      (void *)NULL
};

static CONTROL_ELEMENT Passday700Input_CTL[] =
{
    NO_KEY,           &Invoicing_ST,
    ENTER_KEY,        &Input_name_ST, 
    DOCID_FOUND,      &DoTotal_ST,     
    AMNT_ENOUGH,      &ShowChange_ST,
    UNKNOWN_KEY,      &Passday700_ST
};

extern STATE_OBJ Passday700_ST =
{                         
  ST_PASSDAY700,  //
  Passday700Input_VW,
  passday700_document_DFLT,
  &Ddsp_passday700_document,
  Passday700Input_VFY,
  Passday700Input_UVW,
  Passday700Input_PROC,
  Passday700Input_CTL
};
//mname
short  vfy_Input_name(_TCHAR *data, short key)
{
  if ((data==NULL) ||(strlen(data)<5))
  {
      err_invoke(ILLEGAL_CLIENAME_PERCEPTION);
      return(UNKNOWN_KEY);
  }
  return key;
}
short prc_Input_name(_TCHAR *data, short key)
{
    strcpy(usr_name, data);

    if ((key==ENTER_KEY) || (key==DOCID_FOUND) )
    {
        if ( tot_ret_double(TOT_GEN_INCL)
           + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL)) <= 0.0 ) 
        {
          //REVERSA      
          key=AMNT_ENOUGH;
        }        
    }
//    SetKeyMapping(NORMAL_KEYS); 
    return key;
}
static void Input_name_VW(void)
{
  static short no_cls_states[]={
       ST_PASSDAY700
      ,ST_NAME
      ,0
  };

  if (called_by_state(no_cls_states)==FAIL) {
    cls();
  }

  view_tender_scrn(VIEW_AMNT_DUE, (OPER_SCRN | CUST_SCRN) );
  scrn_clear_window(TOTAL_INPUT_WINDOW);

  format_display(&dsp_name, usr_name);
  strcpy(last_name, usr_name);
  
  print_name();

  SetKeyMapping(ASCII_KEYS);
} /* LogOn_VW */
extern INPUT_CONTROLLER Ddsp_name =
{
  (INPUT_DISPLAY *)&dsp_name,
  KEYBOARD_MASK,
  40,
  41,
  (VERIFY_KEY *)&printing_char_upr
};
static VERIFY_ELEMENT Input_name_VFY[] =
{
  NO_KEY,             (void *)NULL,
  ENTER_KEY,          (void *)vfy_Input_name,
  UNKNOWN_KEY,        (void *)NULL,
};
static void Input_name_UVW(void)
{
  SetKeyMapping(NORMAL_KEYS);
}
static PROCESS_ELEMENT Input_name_PROC[] =
{
    NO_KEY,           cancel_tendering_passday700,
    ENTER_KEY,        (void *)prc_Input_name,  
    UNKNOWN_KEY,      (void *)NULL
};
static CONTROL_ELEMENT Input_name_CTL[] =
{
    NO_KEY,           &Invoicing_ST,
    ENTER_KEY,        &DoTotal_ST, 
    AMNT_ENOUGH,      &ShowChange_ST,
    UNKNOWN_KEY,      &Input_name_ST
};
extern STATE_OBJ Input_name_ST =
{
  ST_NAME,
  Input_name_VW,
  name_DFLT,
  &Ddsp_name,
  Input_name_VFY,
  Input_name_UVW,
  Input_name_PROC,
  Input_name_CTL
};