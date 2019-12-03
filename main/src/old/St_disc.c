/*
 *     Module Name       : ST_DISC.C
 *
 *     Type              : States Discount, DiscountQty, DPriceArt  (Discnt on price art.),
 *                         DPWeightArt, DWWeightArt (Discnt on weight art.),
 *                         (GENVAR == Price or == Weight), DiscAmount
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
 * 06-Jan-2000 Art_grp_sub_no is also kept in TM (Multisam change)       P.M.
 * --------------------------------------------------------------------------
 * 29-Mar-2001 Added OCIA2_DATA.                                       R.N.B.
 * --------------------------------------------------------------------------
 * 21-Sep-2001 Changes for Pending Invoice.                              M.W.
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Added Article Finder.                                     M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "appbase.h"
#include <math.h>
                                            /* Pos (library) include files   */
#include "stri_tls.h"
#include "misc_tls.h"

#include "tm_mgr.h"                         /* Toolsset include files.       */
#include "tot_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "err_mgr.h"
#include "mapkeys.h"

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
#include "st_main.h"
#include "pos_turkey.h"
#include "registry.h"
#include <stdio.h>    /* 27-April-2012 acm - fix */

// 2013-05-30 acm -{
extern short vfy_total_key(_TCHAR *, short);
extern short process_invoice(_TCHAR *, short);

// 2013-05-30 acm -}

static short proc_cancel_discnt(_TCHAR *, short);
static short vfy_toggle_sign(_TCHAR *, short);


float turkey_weight = 0;
/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   Discount_ST                                           */
/*-------------------------------------------------------------------------*/

static void Discount_VW(void)
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
  }

  /* Re-display totals.                                                    */
  InvoicingSubt_VW();
  view_total(OPER_SCRN | CUST_SCRN);
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[31],0,16);  /* DISCOUNT ITEM / QTY               */
} /* Discount_VW */

/*JCP Pavos*/
static void Discount_VW_Turkey(void)
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
  }

  /* Re-display totals.                                                    */
  InvoicingSubt_VW();
  view_total(OPER_SCRN | CUST_SCRN);
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[53],0,16);  /* DISCOUNT ITEM / QTY               */
  turkey_weight = 0;
} /* Discount_VW */


static VERIFY_ELEMENT Discount_VFY_Turkey[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  TIMES_KEY,            vfy_art_qty,
  OCIA1_DATA,           discnt_vfy_and_start_artno_Turkey,
  OCIA2_DATA,           discnt_vfy_and_start_artno_Turkey,
  ENTER_KEY,            discnt_vfy_and_start_artno_Turkey,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  ART_FINDER_KEY,       (void *)NULL,
  UNKNOWN_KEY,          illegal_fn_key
};

static VERIFY_ELEMENT Discount_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  TIMES_KEY,            vfy_art_qty,
  OCIA1_DATA,           discnt_vfy_and_start_artno,
  OCIA2_DATA,           discnt_vfy_and_start_artno,
  ENTER_KEY,            discnt_vfy_and_start_artno,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  ART_FINDER_KEY,       (void *)NULL,
  UNKNOWN_KEY,          illegal_fn_key
};


static PROCESS_ELEMENT Discount_PROC[] =
{
  NO_KEY,               proc_cancel_discnt,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT Discount_CTL[] =
{
  ARTICLE_OK,           &DiscAmount_ST,
  ARTICLE_NOT_OK,       &Discount_ST,
  TIMES_KEY,            &DiscountQty_ST,
  PRICE_ART,            &DPriceArt_ST,
  WWEIGHT_ART,          &DWWeightArt_ST,
  PWEIGHT_ART,          &DPWeightArt_ST,
  NO_KEY,               &Invoicing_ST,
  ART_FINDER_KEY,       &StartArtFind_ST,
  UNKNOWN_KEY,          &Discount_ST
};

static CONTROL_ELEMENT Discount_CTL_Turkey[] =
{
  ARTICLE_OK,           &DiscAmount_ST_Turkey,
  ARTICLE_NOT_OK,       &Discount_ST,
  TIMES_KEY,            &DiscountQty_ST,
  PRICE_ART,            &DPriceArt_ST,
  WWEIGHT_ART,          &DWWeightArt_ST,
  PWEIGHT_ART,          &DPWeightArt_ST,
  NO_KEY,               &Invoicing_ST,
  ART_FINDER_KEY,       &StartArtFind_ST,
  UNKNOWN_KEY,          &Discount_ST
};


extern STATE_OBJ Discount_ST =
{
  ST_DISCOUNT,
  Discount_VW,
  art_no_DFLT,
  &Dartno14KO14n,
  Discount_VFY,
  Input_UVW,
  Discount_PROC,
  Discount_CTL
};

/* FMa - Vale Pavos */

extern STATE_OBJ Turkey_ST =
{
  ST_DISCOUNT,
  Discount_VW_Turkey,    /* FMa: Imprime "scanee el pavos..."  */
  art_no_DFLT,           /* FMa: Prepara recepcion de datos.    */
  &Dartno14KO14n,        /* FMa: Recepciona los datos.          */
  Discount_VFY_Turkey,		     /* FMa: Teclas permitidas?!?           */
  Input_UVW,			 /* FMa: Limpia pantalla?!?             */
  Discount_PROC,		 /* FMa: Solo acepta numero o cancelar. */
  Discount_CTL_Turkey			 /* FMa: Funciones de la pantalla.      */
};


/*-------------------------------------------------------------------------*/
/*                       proc_cancel_discnt                                */
/*-------------------------------------------------------------------------*/
static short proc_cancel_discnt(_TCHAR *data, short key)
{
  /*                                                                       */
  /* Reset display_item so we get an empty description/cust-screen.        */
  /*                                                                       */

  display_item=TM_ROOT;

  return(key);
} /* proc_cancel_discnt */


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   DiscountQty_ST                                        */
/*-------------------------------------------------------------------------*/

static void DiscountQty_VW(void)
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
  scrn_string_out(input_TXT[32], 0, 16);  /* ENTER DISCOUNT ITEM / QTY OR PR */
} /* DiscountQty_VW */


static VERIFY_ELEMENT DiscountQty_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  TIMES_KEY,            vfy_art_qty,
  ENTER_KEY,            discnt_vfy_and_start_artno,
  OCIA1_DATA,           discnt_vfy_and_start_artno,
  OCIA2_DATA,           discnt_vfy_and_start_artno,
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  ART_FINDER_KEY,       (void *)NULL,
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT DiscountQty_PROC[] =
{
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT DiscountQty_CTL[] =
{
  ARTICLE_OK,           &DiscAmount_ST,
  ARTICLE_NOT_OK,       &Discount_ST,       /* Illegal article, cancel QTY   */
  NO_KEY,               &Invoicing_ST,
  TIMES_KEY,            &DiscountQty_ST,
  PRICE_ART,            &DPriceArt_ST,
  WWEIGHT_ART,          &DWWeightArt_ST,
  PWEIGHT_ART,          &DPWeightArt_ST,
  ART_FINDER_KEY,       &StartArtFind_ST,
  UNKNOWN_KEY,          &DiscountQty_ST
};

extern STATE_OBJ DiscountQty_ST =
{
  ST_DISCOUNT_QTY,
  DiscountQty_VW,
  art_no_DFLT,
  &Dartno14KO14n,
  DiscountQty_VFY,
  Input_UVW,
  DiscountQty_PROC,
  DiscountQty_CTL
};


/*-------------------------------------------------------------------------*/
/*                       discnt_vfy_and_start_artno                        */
/*-------------------------------------------------------------------------*/
short discnt_vfy_and_start_artno(_TCHAR *data, short key)
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

  if (get_barcode_type(data)==BARCD_INTERNAL_EAN13) {
    err_invoke(NO_DISC_ON_R2C_BARCD);
    *data=_T('\0');
    return(UNKNOWN_KEY);
  }
  status=vfy_and_start_artno(data, key);
  if (status != ARTICLE_NOT_OK) {
    if (c_item.arti.art_ind==ART_IND_WEIGHT) {/* Qty for discount allways  */
      c_item.disc.base.qty=1.0;               /* positive!                 */
    }
    else {
      c_item.disc.base.qty=fabs(c_item.arti.base.qty);
    }
/*    turkey_weight = 0;*/
/*m*/printf_log("Aqui realiza el descuento manual");
    _tcsncpy(c_item.disc.base.descr, scrn_inv_TXT[25], 33);
    c_item.disc.base.vat_no         = c_item.arti.base.vat_no;
    c_item.disc.base.vat_perc       = c_item.arti.base.vat_perc;
    c_item.disc.base.art_grp_no     = c_item.arti.base.art_grp_no;
    c_item.disc.base.art_grp_sub_no = c_item.arti.base.art_grp_sub_no;
    c_item.disc.base.dept_cd        = c_item.arti.base.dept_cd;
	/*turkey_weight = c_item.arti.base.price;*/
	
	
    if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM)
      == SCRL_UNKNOWN_LINE) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
    }
  }

  return(status);
} /* discnt_vfy_and_start_artno */


extern int get_discount_prom_horeca(long art_no, double * valor);

/*-------------------------------------------------------------------------*/
/*                       discnt_vfy_and_start_artno                        */
/*-------------------------------------------------------------------------*/
short discnt_vfy_and_start_artno_Horeca(_TCHAR *data, short key)
{
    short status;
    double descuento      =0;
    int   is_art_horeca   =0;

    /*                                                                       */
    /* Get article and possible deposit data from PLU.                       */
    /*                                                                       */
    /* Before calling vfy_and_start_artno(), check wether data is an         */
    /* reduced-to-clear barcode. If it is, give an error and return          */
    /* UNKNOWN_KEY because no discount alouwed on reduced to clear art's.    */
    /*                                                                       */
    /* Also inititalise the discount part of this item.                      */
    /*                                                                       */
    if (get_barcode_type(data)==BARCD_INTERNAL_EAN13) {
        err_invoke(NO_DISC_ON_R2C_BARCD);
        *data=_T('\0');
        return(UNKNOWN_KEY);
    }
    //++init_item();

    /*
    c_item.disc.base.price          = 0;
    c_item.disc.base.qty            = 0;
    */
    status=vfy_and_start_artno(data, key);
    if (status != ARTICLE_NOT_OK) 
    {
        /*
        if (c_item.arti.art_ind==ART_IND_WEIGHT) {/
          c_item.disc.base.qty=0;               // si es pesable se inicializa a cero
        }
        else {
          c_item.disc.base.qty=fabs(c_item.arti.base.qty);
        }
        */
    /*    turkey_weight = 0;*/
        is_art_horeca = get_discount_prom_horeca(c_item.arti.base.art_no,&descuento);

        if (is_art_horeca)
        {
            _tcsncpy(c_item.disc.base.descr, scrn_inv_TXT[25], 33);
            c_item.disc.base.vat_no         = c_item.arti.base.vat_no;
            c_item.disc.base.vat_perc       = c_item.arti.base.vat_perc;
            c_item.disc.base.art_grp_no     = c_item.arti.base.art_grp_no;
            c_item.disc.base.art_grp_sub_no = c_item.arti.base.art_grp_sub_no;
            c_item.disc.base.dept_cd        = c_item.arti.base.dept_cd;

            c_item.disc.base.price          = descuento;
            c_item.disc.base.qty            = fabs(c_item.arti.base.qty);

            /*
            c_item.disc.base.goods_value=floor_price(
                    c_item.disc.base.price * c_item.disc.base.qty;
            );
            */
            // fix----
            c_item.disc.base.goods_value    = c_item.disc.base.price * c_item.disc.base.qty;

            if ((c_item.disc.base.price != 0.0) &&
                (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM)
                == SCRL_UNKNOWN_LINE) ) {             /* Reduced to clear art.   */
              scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
            }

            /*
           if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM)
              == SCRL_UNKNOWN_LINE) {
              scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, C_ITEM);
            }
            */
           /*
            if ((c_item.depo.base.price != 0.0) &&
                (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM)
                == SCRL_UNKNOWN_LINE) ) {
              scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, C_ITEM);
            }
            */
        }
        else
        {
            *data=_T('\0');
            err_invoke(ARTICLE_NOT_HORECA);
            delete_active_item(empty,0);
            return(UNKNOWN_KEY);
        }
  }

  return(status);
} /* discnt_vfy_and_start_artno */



/*-------------------------------------------------------------------------*/
/*                       discnt_vfy_and_start_artno                        */
/*-------------------------------------------------------------------------*/
short discnt_vfy_and_start_artno_Turkey(_TCHAR *data, short key)
{
  short status;
  _TCHAR reg_turkey_code[7];
  #define LEN_TURKEY_CODE_NO     6
  _TCHAR* param;
  _TCHAR strT[6];
  /*                                                                       */
  /* Get article and possible deposit data from PLU.                       */
  /*                                                                       */
  /* Before calling vfy_and_start_artno(), check wether data is an         */
  /* reduced-to-clear barcode. If it is, give an error and return          */
  /* UNKNOWN_KEY because no discount alouwed on reduced to clear art's.    */
  /*                                                                       */
  /* Also inititalise the discount part of this item.                      */
  /*                                                                       */



  /*Verifico que los primeros digitos sean 24 con esto me aseguro sea un pesable*/

  
  /* TurkeyCode */
  /* CHECK TILL NUMBER */
  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("TURKEY_CODE"), reg_turkey_code, LEN_TURKEY_CODE_NO);
  param = _tcsupr(reg_turkey_code);

  *(param+6) = _T('\0');
  _tcsncpy(strT, data, 6);   /*saco los primeros 5 digitos de la data de ingreso*/
  *(strT+6) = _T('\0');

  if (_tcscmp(reg_turkey_code,strT))
  {/*MessageBox(NULL, (LPTSTR) strT, NULL, MB_OK|MB_SETFOREGROUND);*/
        err_invoke(NOT_VALID_TURKEY);
	      *data=_T('\0');
        return(UNKNOWN_KEY);
  }

/*
  if (*data==_T('2'))
  {
	  if( *(data+1)!=_T('4')) 
	  {   / 29  EXT. WEIGHT BARCODE /
          err_invoke(NOT_VALID_TURKEY);
	      *data=_T('\0');
          return(UNKNOWN_KEY);
	  }
  }else 
  {
	  err_invoke(NOT_VALID_TURKEY);
	  *data=_T('\0');
      return(UNKNOWN_KEY);
  }

  
  /Verifico el codigo del producto /
  if (*(data+2)==_T('1'))
  {
	  if( *(data+3)==_T('6')) 
	  {   
		  if( *(data+4)==_T('8'))
		  {
			  if( *(data+5)!=_T('6'))
			  {
				  err_invoke(NOT_VALID_TURKEY);
				  *data=_T('\0');
				  return(UNKNOWN_KEY);
			  }
		  }else{
			  err_invoke(NOT_VALID_TURKEY);
			  *data=_T('\0');
			  return(UNKNOWN_KEY);
		  }
		  
	  }else
	  {
		  err_invoke(NOT_VALID_TURKEY);
		  *data=_T('\0');
		  return(UNKNOWN_KEY);
	  }
  }else 
  {
	  err_invoke(NOT_VALID_TURKEY);
	  *data=_T('\0');
      return(UNKNOWN_KEY);
  }
  */

  if (get_barcode_type(data)==BARCD_INTERNAL_EAN13) {
	  
    err_invoke(NO_DISC_ON_R2C_BARCD);
    *data=_T('\0');
    return(UNKNOWN_KEY);
  }
  
  status=vfy_and_start_artno_turkey(data, key); /*JCP Veri*/
  
  
  if (status != ARTICLE_NOT_OK) {
	  /*MessageBox(NULL, (LPTSTR) strT, NULL, MB_OK|MB_SETFOREGROUND);*/
    if (c_item.arti.art_ind==ART_IND_WEIGHT) {/* Qty for discount allways  */
      c_item.disc.base.qty=1.0;               /* positive!                 */
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

    turkey_weight = (float)c_item.arti.base.price; 


    if (scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM)
      == SCRL_UNKNOWN_LINE) {
      scrl_add_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
    }
     return(status);
  }
/*MessageBox(NULL, (LPTSTR) strT, NULL, MB_OK|MB_SETFOREGROUND);*/
  //return(status);
  return(UNKNOWN_KEY);
} /* discnt_vfy_and_start_artno */



/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   DPriceArt_ST                                          */
/*-------------------------------------------------------------------------*/

static void DPriceArt_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[5], 0, 12);    /* ENTER PRICE 1X               */
} /* DPriceArt_VW */

static VERIFY_ELEMENT DPriceArt_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  ENTER_KEY,            vfy_art_price,
  OCIA1_DATA,           vfy_ocia_not_legal, /* err_invoke() + UNKNOWN_KEY    */
  OCIA2_DATA,           vfy_ocia_not_legal, /* err_invoke() + UNKNOWN_KEY    */
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT DPriceArt_PROC[] =
{
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT DPriceArt_CTL[] =
{
  ENTER_KEY,            &DiscAmount_ST,
  NO_KEY,               &Invoicing_ST,
  UNKNOWN_KEY,          &DPriceArt_ST
};

extern STATE_OBJ DPriceArt_ST =
{
  ST_DPRICE_ART,
  DPriceArt_VW,
  no_DFLT,
  &Dprice10K10nd,
  DPriceArt_VFY,
  Input_UVW,
  DPriceArt_PROC,
  DPriceArt_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   DPWeightArt_ST                                        */
/*-------------------------------------------------------------------------*/

static void DPWeightArt_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[5], 0, 16);    /* ENTER PRICE 1X               */
} /* DPWeightArt_VW */


static VERIFY_ELEMENT DPWeightArt_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  ENTER_KEY,            vfy_weight_price,
  OCIA1_DATA,           vfy_ocia_not_legal, /* err_invoke() + UNKNOWN_KEY    */
  OCIA2_DATA,           vfy_ocia_not_legal, /* err_invoke() + UNKNOWN_KEY    */
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};


static PROCESS_ELEMENT DPWeightArt_PROC[] =
{
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT DPWeightArt_CTL[] =
{
  ENTER_KEY,            &DiscAmount_ST,
  NO_KEY,               &Invoicing_ST,
  UNKNOWN_KEY,          &DPWeightArt_ST
};

extern STATE_OBJ DPWeightArt_ST =
{
  ST_DPWEIGHT_ART,
  DPWeightArt_VW,
  no_DFLT,
  &Dprice10K10nd,
  DPWeightArt_VFY,
  Input_UVW,
  DPWeightArt_PROC,
  DPWeightArt_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   DWWeightArt_ST                                        */
/*-------------------------------------------------------------------------*/

static void DWWeightArt_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[6], 0, 10);
} /* DWWeightArt_VW */

static VERIFY_ELEMENT DWWeightArt_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  ENTER_KEY,            vfy_weight,
  OCIA1_DATA,           vfy_ocia_not_legal, /* err_invoke() + UNKNOWN_KEY    */
  OCIA2_DATA,           vfy_ocia_not_legal, /* err_invoke() + UNKNOWN_KEY    */
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT DWWeightArt_PROC[] =
{
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT DWWeightArt_CTL[] =
{
  ENTER_KEY,            &DiscAmount_ST,
  NO_KEY,               &Invoicing_ST,
  UNKNOWN_KEY,          &DWWeightArt_ST
};

extern STATE_OBJ DWWeightArt_ST =
{
  ST_DWWEIGHT_ART,
  DWWeightArt_VW,
  no_DFLT,
  &Dqty6K6nd,
  DWWeightArt_VFY,
  Input_UVW,
  DWWeightArt_PROC,
  DWWeightArt_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   DiscAmount_ST                                         */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                          vfy_discount_amount                            */
/*-------------------------------------------------------------------------*/
short vfy_discount_amount(_TCHAR *data, short key)
{

  /*                                                                       */
  /* Perform several checks before accepting the discount amount.          */
  /*                                                                       */

  no_leading_zeros(data);

  if (STR_ZERO(data)) {
    err_invoke(ZERO_NOT_LEGAL_ERROR);
    if (*data==_T('-')) {
      *(data+1)=_T('\0');
    }
    else {
      *data = _T('\0');
    }
    key=UNKNOWN_KEY;
  }
  else if (genvar.ind_price_dec!=DECIMALS_YES && _tcslen(data) > 7) {
    err_invoke(VALUE_TOO_LARGE);
    if( *data==_T('-')) {
      *(data+1) = _T('\0');
    }
    else {
      *data = _T('\0');
    }
    key=UNKNOWN_KEY;
  }
  else if ((double)fabs(atof_price(data)) * fabs(c_item.disc.base.qty) >
           (double)fabs(c_item.arti.base.goods_value)) {
    /* Amount exceeds the article goods-value.                             */
    err_invoke(DISCNT_AMNT_TOO_LARGE);
    if (*data==_T('-')) {
      *(data+1)=_T('\0');
    }
    else {
      *data=_T('\0');
    }
    key=UNKNOWN_KEY;
  }
  else {
    
	c_item.disc.base.price=atof_price(data);
    c_item.disc.base.goods_value=c_item.disc.base.price * c_item.disc.base.qty;
    scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
  }
   


  return(key);
} /* vfy_discount_amount */


/*-------------------------------------------------------------------------*/
/*                          vfy_discount_amount_turkey                     */
/*-------------------------------------------------------------------------*/
short vfy_discount_amount_turkey(_TCHAR *data, short key)
{
   _TCHAR strF[40];
   _TCHAR strT[6];
   _TCHAR newdata[4];
   double peso=0;
   double tmp=0;

   //_TCHAR voucher_num[6];
  /*                                                                       */
  /* Perform several checks before accepting the discount amount.          */
  /*                                                                       */
  ch_memset(strF, 0, sizeof(strF));
  ch_memset(strT, 0, sizeof(strT));
  
  _tcsncpy(strT, data, 5);   /*saco los primeros 5 digitos de la data de ingreso*/
  _tcsncpy(newdata, (data+5), 3);   /*saco los ultimos 5 digitos de la data de ingreso*/
  

  _tcscpy(data, newdata);   /*igualo la nueva data es decir los 5 ultimos digitos*/
  //no_leading_zeros(data);
  //peso = atof_price(data)/10;
  peso = (double)_tcstod(data, NULL) / 10;

  tmp =  peso;

  *data=_T('\0');

  _stprintf(data, _T("%f"), ((peso)*(turkey_weight*100))*-1);

  no_leading_zeros(data);

  

  if (STR_ZERO(data)) {
    err_invoke(ZERO_NOT_LEGAL_ERROR);
    if (*data==_T('-')) {
      *(data+1)=_T('\0');
    }
    else {
      *data = _T('\0');
    }
    key=UNKNOWN_KEY;
  }
  else if (genvar.ind_price_dec!=DECIMALS_YES && _tcslen(data) > 7) {
    err_invoke(VALUE_TOO_LARGE);
    if( *data==_T('-')) {
      *(data+1) = _T('\0');
    }
    else {
      *data = _T('\0');
    }
    key=UNKNOWN_KEY;
  }
  else if ((double)fabs(atof_price(data)) * fabs(c_item.disc.base.qty) >
           (double)fabs(c_item.arti.base.goods_value)) {
    /* Amount exceeds the article goods-value.   */                          
    err_invoke(DISCNT_AMNT_TOO_LARGE);
    if (*data==_T('-')) {
      *(data+1)=_T('\0');
    }
    else {
      *data=_T('\0');
    }
    key=UNKNOWN_KEY;
  }
  else {
	  _tcscat(strF,c_item.disc.base.descr);
      
	  _tcscat(strF,strT);

    _tcscpy(c_item.disc.base.descr, strF);
	   
    c_item.disc.base.price=atof_price(data);
    c_item.disc.base.goods_value=c_item.disc.base.price * c_item.disc.base.qty;
    scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
  }
   
  /*if (_tcslen(data) > 2)
  {
	  err_invoke(NOT_VALID_VOUCHER_TURKEY);
	  key=UNKNOWN_KEY;
  }*/

  return(key);
} /* vfy_discount_amount_turkey */



static void DiscAmount_VW_Turkey(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[54], 0, 16);      

} /* DiscAmount_VW */

static void DiscAmount_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[35], 0, 13);    /* DISCOUNT AMOUNT 1X            */
  assign_minus=0;
  format_display(&dsp_price10m, empty);
} /* DiscAmount_VW */

static VERIFY_ELEMENT DiscAmount_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  ENTER_KEY,            vfy_discount_amount,
  OCIA1_DATA,           vfy_ocia_not_legal,     /* err_invoke()+UNKNOWN_KEY  */
  OCIA2_DATA,           vfy_ocia_not_legal,     /* err_invoke()+UNKNOWN_KEY  */
  TOGGLE_SIGN_KEY,      vfy_toggle_sign,        /* returns UNKNOWN_KEY       */
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

static VERIFY_ELEMENT DiscAmount_VFY_Turkey[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  ENTER_KEY,            vfy_discount_amount_turkey,
  OCIA1_DATA,           vfy_ocia_not_legal,     /* err_invoke()+UNKNOWN_KEY  */
  OCIA2_DATA,           vfy_ocia_not_legal,     /* err_invoke()+UNKNOWN_KEY  */
  TOGGLE_SIGN_KEY,      vfy_toggle_sign,        /* returns UNKNOWN_KEY       */
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

extern INPUT_CONTROLLER Dprice10K10ndm =
{
  (INPUT_DISPLAY *)&dsp_price10m,
  KEYBOARD_MASK | OCIA1_MASK | OCIA2_MASK,  
  10, 10,
  (VERIFY_KEY *)&numeric_dnull
};

static PROCESS_ELEMENT DiscAmount_PROC[] =
{
  ENTER_KEY,            accept_active_item,
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT DiscAmount_CTL[] =
{
  ENTER_KEY,            &Invoicing_ST,
  NO_KEY,               &Invoicing_ST,
  UNKNOWN_KEY,          &DiscAmount_ST
};

extern STATE_OBJ DiscAmount_ST =
{
  ST_DISC_AMOUNT,
  DiscAmount_VW,
  minus_DFLT,
  &Dprice10K10ndm,
  DiscAmount_VFY,
  Input_UVW,
  DiscAmount_PROC,
  DiscAmount_CTL
};


extern STATE_OBJ DiscAmount_ST_Turkey =
{
  ST_DISC_AMOUNT,
  DiscAmount_VW_Turkey,
  /*minus_DFLT,*/
  no_DFLT,
  &Dartno14KO14n,
  DiscAmount_VFY_Turkey,
  Input_UVW,
  DiscAmount_PROC,
  DiscAmount_CTL
};



/*-------------------------------------------------------------------------*/
/*                          vfy_toggle_sign                                */
/*-------------------------------------------------------------------------*/
static short vfy_toggle_sign(_TCHAR *data, short key)
{
  if (invoice_line_mode == RETURN) {
    toggle_sign(data);                    /* Handles empty strings also. */
  }
  else {
    err_invoke(ILLEGAL_FUNCTION_KEY_ERROR);
  }

  return(UNKNOWN_KEY);
} /* vfy_toggle_sign */

//v3.4.8 acm -{


 void Discount_VW_Horeca(void)
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
  }

  /* Re-display totals.                                                    */
  InvoicingSubt_VW();
  view_total( OPER_SCRN | CUST_SCRN );


//++  invoice_line_mode=invoice_mode;                 /* Reset to default mode.*/
  #ifndef NO_VIEW_POS_STATE
      view_pos_state();
  #endif
  init_item();

  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[58],0,16);  /* ENTER ITEM / QTY OR PRESS<NO> HORECA   */

} /* Discount_VW_Horeca */


/*-------------------------------------------------------------------------*/
/*                          vfy_discount_amount                            */
/*-------------------------------------------------------------------------*/
short vfy_discount_amount_Horeca(_TCHAR *data, short key)
{

  /*                                                                       */
  /* Perform several checks before accepting the discount amount.          */
  /*                                                                       */

  no_leading_zeros(data);

  if (STR_ZERO(data)) {
    err_invoke(ZERO_NOT_LEGAL_ERROR);
    if (*data==_T('-')) {
      *(data+1)=_T('\0');
    }
    else {
      *data = _T('\0');
    }
    key=UNKNOWN_KEY;
  }
  else if (genvar.ind_price_dec!=DECIMALS_YES && _tcslen(data) > 7) {
    err_invoke(VALUE_TOO_LARGE);
    if( *data==_T('-')) {
      *(data+1) = _T('\0');
    }
    else {
      *data = _T('\0');
    }
    key=UNKNOWN_KEY;
  }
  else if ((double)fabs(atof_price(data)) * fabs(c_item.disc.base.qty) >
           (double)fabs(c_item.arti.base.goods_value)) {
    /* Amount exceeds the article goods-value.                             */
    err_invoke(DISCNT_AMNT_TOO_LARGE);
    if (*data==_T('-')) {
      *(data+1)=_T('\0');
    }
    else {
      *data=_T('\0');
    }
    key=UNKNOWN_KEY;
  }
  else {
    
	c_item.disc.base.price=atof_price(data);
/*
    c_item.disc.base.goods_value=floor_price(
            calc_incl_vat(c_item.disc.base.price * c_item.disc.base.qty,c_item.arti.base.vat_no)
    );
*/
    //fix---
    c_item.disc.base.goods_value=c_item.disc.base.price * c_item.disc.base.qty;
    //acm - 
    scrl_update_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, C_ITEM);
  }
   
  return(key);
} /* vfy_discount_amount_Horeca */


extern STATE_OBJ DWWeightArt_Horeca_ST;
extern STATE_OBJ DiscountQty_Horeca_ST; 


extern VERIFY_ELEMENT Discount_Horeca_VFY[];
extern PROCESS_ELEMENT Discount_Horeca_PROC[];
extern CONTROL_ELEMENT Discount_Horeca_CTL[] ;

extern STATE_OBJ Discount_Horeca_ST =
{
  ST_DISCOUNT,
  Discount_VW_Horeca,    /* FMa: Imprime "scanee el pavos..."  */
  art_no_DFLT,           /* FMa: Prepara recepcion de datos.    */
  &Dartno14KO14n,        /* FMa: Recepciona los datos.          */
  Discount_Horeca_VFY,		     /* FMa: Teclas permitidas?!?           */
  Input_UVW,			 /* FMa: Limpia pantalla?!?             */
  Discount_Horeca_PROC,		 /* FMa: Solo acepta numero o cancelar. */
  Discount_Horeca_CTL			 /* FMa: Funciones de la pantalla.      */
};

VERIFY_ELEMENT Discount_Horeca_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  TIMES_KEY,            vfy_art_qty,
  OCIA1_DATA,           discnt_vfy_and_start_artno_Horeca,
  OCIA2_DATA,           discnt_vfy_and_start_artno_Horeca,
  ENTER_KEY,            discnt_vfy_and_start_artno_Horeca,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  ART_FINDER_KEY,       (void *)NULL,
  UNKNOWN_KEY,          illegal_fn_key,
  TOTAL_KEY,            vfy_total_key
};

PROCESS_ELEMENT Discount_Horeca_PROC[] =
{
  ARTICLE_OK,           accept_active_item, 
//  ENTER_KEY,            accept_active_item, 
  NO_KEY,               proc_cancel_discnt,
  UNKNOWN_KEY,          (void *)NULL,
  TOTAL_KEY,               process_invoice
};

CONTROL_ELEMENT Discount_Horeca_CTL[] =
{
  ARTICLE_OK,           &Discount_Horeca_ST, //&Invoicing_ST,//&Discount_Horeca_ST, //cambiar de status
  ARTICLE_NOT_OK,       &Discount_Horeca_ST,
  TIMES_KEY,            &DiscountQty_Horeca_ST, /// &InvoicingQty_ST,   /* Qty entered,get artno/qty */
//++PRICE_ART,            &DPriceArt_ST,
  WWEIGHT_ART,          &DWWeightArt_Horeca_ST,
  //++PWEIGHT_ART,          &DPWeightArt_ST,
  //NO_KEY,               &Invoicing_ST, // fix 2013-05-31 acm - 
  NO_KEY,               &Invoicing_ST, // fix 2013-05-31 acm - 
//  ENTER_KEY,            &Invoicing_ST,
//  ART_FINDER_KEY,       &StartArtFind_ST,
  UNKNOWN_KEY,          &Discount_Horeca_ST,
  ///++TOTAL_KEY,            &DoTotal_ST        /* Start tendering           */
};


extern VERIFY_ELEMENT  DWWeightArt_Horeca_VFY[];
extern PROCESS_ELEMENT DWWeightArt_Horeca_PROC[];
extern CONTROL_ELEMENT DWWeightArt_Horeca_CTL[] ;

STATE_OBJ DWWeightArt_Horeca_ST =
{
  ST_DWWEIGHT_ART,
  DWWeightArt_VW,
  no_DFLT,
  &Dqty6K6nd,
  DWWeightArt_Horeca_VFY,
  Input_UVW,
  DWWeightArt_Horeca_PROC,
  DWWeightArt_Horeca_CTL
};

VERIFY_ELEMENT DWWeightArt_Horeca_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  ENTER_KEY,            vfy_weight,
  OCIA1_DATA,           vfy_ocia_not_legal, /* err_invoke() + UNKNOWN_KEY    */
  OCIA2_DATA,           vfy_ocia_not_legal, /* err_invoke() + UNKNOWN_KEY    */
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

PROCESS_ELEMENT DWWeightArt_Horeca_PROC[] =
{
  ENTER_KEY,            accept_active_item,
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

CONTROL_ELEMENT DWWeightArt_Horeca_CTL[] =
{
  ENTER_KEY,            &Discount_Horeca_ST,
  NO_KEY,               &Invoicing_ST,
  UNKNOWN_KEY,          &DWWeightArt_Horeca_ST
};


static void DiscountQty_Horeca_VW(void)
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
  scrn_string_out(input_TXT[58],0,16); /* ENTER ITEM / QTY OR PRESS<NO> HORECA*/
} /* InvoicingQty_VW */


/* extern INPUT_CONTROLLER Dartno14KO14n, defined in previous state          */

static VERIFY_ELEMENT DiscountQty_Horeca_VFY[] =
{
  LINE_UP_KEY,     handle_scroll_key,
  LINE_DOWN_KEY,   handle_scroll_key,
  PAGE_UP_KEY,     handle_scroll_key,
  PAGE_DOWN_KEY,   handle_scroll_key,
  NO_KEY,          (void *)NULL,
  TIMES_KEY,       vfy_art_qty,
  ENTER_KEY,       discnt_vfy_and_start_artno_Horeca,
  OCIA1_DATA,      discnt_vfy_and_start_artno_Horeca,
  OCIA2_DATA,      discnt_vfy_and_start_artno_Horeca,
  CLEAR_KEY,       vfy_clear_key,
  OPEN_DRAWER_KEY, open_and_close_drawer,  /* Approval in state-engine.     */
  ART_FINDER_KEY,  (void *)NULL,
  UNKNOWN_KEY,     illegal_fn_key
};


static PROCESS_ELEMENT DiscountQty_Horeca_PROC[] =
{
  ARTICLE_OK,   accept_active_item,
  NO_KEY,       delete_active_item,
  UNKNOWN_KEY,  (void *)NULL
};


static CONTROL_ELEMENT DiscountQty_Horeca_CTL[] =
{
  ARTICLE_OK,          &Discount_Horeca_ST,
  ARTICLE_NOT_OK,      &Discount_Horeca_ST,
  NO_KEY,              &Discount_Horeca_ST,
  TIMES_KEY,           &DiscountQty_Horeca_ST,
//++  PRICE_ART,           &NPriceArt_ST,
  WWEIGHT_ART,         &DWWeightArt_Horeca_ST, //&NWWeightArt_ST,
//++  PWEIGHT_ART,         &NPWeightArt_ST,
  PRICE_TOO_LARGE_KEY, &Discount_Horeca_ST,
  ART_FINDER_KEY,      &StartArtFind_ST,
  UNKNOWN_KEY,         &DiscountQty_Horeca_ST
};


extern STATE_OBJ DiscountQty_Horeca_ST =
{
  ST_INVOICING_QTY,
  DiscountQty_Horeca_VW,
  art_no_DFLT,
  &Dartno14KO14n,
  DiscountQty_Horeca_VFY,
  Input_UVW,
  DiscountQty_Horeca_PROC,
  DiscountQty_Horeca_CTL
};

