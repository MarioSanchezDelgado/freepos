/*
 *     Module Name       : ST_CRED.C
 *
 *     Type              : States Credit, CreditQty, CPriceArt (Credit on price art.),
 *                         CPWeightArt, CWWeightArt (Credit on weight art),
 *                         (GENVAR == Price or == Weight), CPriceCorr
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
 * 29-Mar-2001 Added OCIA2_DATA.                                       R.N.B.
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Added Article Finder.                                     M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
                                            /* Pos (library) include files   */
#include "appbase.h"
#include "stri_tls.h"
#include "misc_tls.h"


#include "tm_mgr.h"                         /* Toolsset include files.       */
#include "scrn_mgr.h"
#include "inp_mgr.h"
#include "state.h"
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

/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   Credit_ST                                             */
/*-------------------------------------------------------------------------*/

static void Credit_VW(void)
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
  scrn_string_out(input_TXT[42],0,16);  /* CREDIT ITEM / QTY               */
} /* Credit_VW */

static VERIFY_ELEMENT Credit_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  TIMES_KEY,            vfy_art_qty,
  OCIA1_DATA,           vfy_and_start_artno,
  OCIA2_DATA,           vfy_and_start_artno,
  ENTER_KEY,            vfy_and_start_artno,
  CLEAR_KEY,            vfy_clear_key,
  NO_KEY,               (void *)NULL,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  ART_FINDER_KEY,       (void *)NULL,
  UNKNOWN_KEY,          illegal_fn_key 
};

static PROCESS_ELEMENT Credit_PROC[] =
{
  ARTICLE_OK,           accept_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT Credit_CTL[] =
{
  ARTICLE_OK,           &Invoicing_ST,
  ARTICLE_NOT_OK,       &Credit_ST,
  TIMES_KEY,            &CreditQty_ST,
  PRICE_ART,            &CPriceArt_ST,  
  WWEIGHT_ART,          &CWWeightArt_ST,
  PWEIGHT_ART,          &CPWeightArt_ST,
  NO_KEY,               &Invoicing_ST,
  PRICE_TOO_LARGE_KEY,  &Invoicing_ST,      /* Item is deleted, back to inv. */
  ART_FINDER_KEY,       &StartArtFind_ST,
  UNKNOWN_KEY,          &Credit_ST   
};

extern STATE_OBJ Credit_ST =
{
  ST_CREDIT,
  Credit_VW,
  art_no_DFLT,
  &Dartno14KO14n,
  Credit_VFY,
  Input_UVW,
  Credit_PROC,
  Credit_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   CreditQTY_ST                                          */
/*-------------------------------------------------------------------------*/

static void CreditQty_VW(void)
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
    InvoicingSubt_VW();
    view_total(OPER_SCRN | CUST_SCRN);
  }

  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[4], 0, 16);     /* ENTER ITEM / QTY OR PR <NO>   */
} /* CreditQty_VW */


static VERIFY_ELEMENT CreditQty_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  TIMES_KEY,            vfy_art_qty,
  ENTER_KEY,            vfy_and_start_artno,
  OCIA1_DATA,           vfy_and_start_artno,
  OCIA2_DATA,           vfy_and_start_artno,
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  ART_FINDER_KEY,       (void *)NULL,
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT CreditQty_PROC[] =
{
  ARTICLE_OK,           accept_active_item,
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT CreditQty_CTL[] =
{
  ARTICLE_OK,           &Invoicing_ST,
  ARTICLE_NOT_OK,       &Credit_ST,     /* Illegal article, cancel QTY       */
  NO_KEY,               &Invoicing_ST,
  TIMES_KEY,            &CreditQty_ST,
  PRICE_ART,            &CPriceArt_ST,
  WWEIGHT_ART,          &CWWeightArt_ST,
  PWEIGHT_ART,          &CPWeightArt_ST,
  PRICE_TOO_LARGE_KEY,  &Invoicing_ST,      /* Item is deleted, back to inv. */
  ART_FINDER_KEY,       &StartArtFind_ST,
  UNKNOWN_KEY,          &CreditQty_ST
};

extern STATE_OBJ CreditQty_ST =
{
  ST_CREDIT_QTY,
  CreditQty_VW,
  art_no_DFLT,
  &Dartno14KO14n,
  CreditQty_VFY,
  Input_UVW,
  CreditQty_PROC,
  CreditQty_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   CPriceArt_ST                                          */
/*-------------------------------------------------------------------------*/

static void CPriceArt_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[33], 0, 16);    /* ENTER CREDIT PRICE 1X         */
} /* CPriceArt_VW */

static VERIFY_ELEMENT CPriceArt_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  ENTER_KEY,            vfy_art_price,
  OCIA1_DATA,           vfy_ocia_not_legal,  /* err_invoke() + UNKNOWN_KEY    */
  OCIA2_DATA,           vfy_ocia_not_legal,  /* err_invoke() + UNKNOWN_KEY    */
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT CPriceArt_PROC[] =
{
  ENTER_KEY,            (void *)NULL,
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT CPriceArt_CTL[] =
{  
  ENTER_KEY,            &CPriceCorr_ST,
  NO_KEY,               &Invoicing_ST,
  UNKNOWN_KEY,          &CPriceArt_ST
};

extern STATE_OBJ CPriceArt_ST =
{
  ST_CPRICE_ART,
  CPriceArt_VW,
  no_DFLT,
  &Dprice10K10nd,
  CPriceArt_VFY,
  Input_UVW,
  CPriceArt_PROC,
  CPriceArt_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   CPWeightArt_ST                                        */
/*-------------------------------------------------------------------------*/

static void CPWeightArt_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[33], 0, 16);    /* ENTER CREDIT PRICE 1X         */
} /* CPWeightArt_VW */

static VERIFY_ELEMENT CPWeightArt_VFY[] =
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

static PROCESS_ELEMENT CPWeightArt_PROC[] =
{
  ENTER_KEY,            (void *)NULL,
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT CPWeightArt_CTL[] =
{
  ENTER_KEY,            &CPriceCorr_ST,
  NO_KEY,               &Invoicing_ST,
  UNKNOWN_KEY,          &CPWeightArt_ST
};

extern STATE_OBJ CPWeightArt_ST =
{
  ST_CPWEIGHT_ART,
  CPWeightArt_VW,
  no_DFLT,
  &Dprice10K10nd,
  CPWeightArt_VFY,
  Input_UVW,
  CPWeightArt_PROC,
  CPWeightArt_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   CWWeightArt_ST                                        */
/*-------------------------------------------------------------------------*/

static void CWWeightArt_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[6], 0, 16);     /* WEIGHT REQUIRED               */
} /* CWWeightArt_VW */

static VERIFY_ELEMENT CWWeightArt_VFY[] =
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

static PROCESS_ELEMENT CWWeightArt_PROC[] =
{
  ENTER_KEY,            (void *)NULL,
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT CWWeightArt_CTL[] =
{
  ENTER_KEY,            &CPriceCorr_ST,
  NO_KEY,               &Invoicing_ST,
  UNKNOWN_KEY,          &CWWeightArt_ST
};

extern STATE_OBJ CWWeightArt_ST =
{
  ST_CWWEIGHT_ART,
  CWWeightArt_VW,
  no_DFLT,
  &Dqty6K6nd,
  CWWeightArt_VFY,
  Input_UVW,
  CWWeightArt_PROC,
  CWWeightArt_CTL
};


/*-------------------------------------------------------------------------*/
/* STATE-STRUCTURE   CPriceCorr_ST                                         */
/*-------------------------------------------------------------------------*/
static void CPriceCorr_VW(void)
{
  scrn_clear_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[43], 0, 3);     /* OK (Y/N)                      */
} /* CPriceCorr_VW */


static VERIFY_ELEMENT CPriceCorr_VFY[] =
{
  LINE_UP_KEY,          handle_scroll_key,
  LINE_DOWN_KEY,        handle_scroll_key,
  PAGE_UP_KEY,          handle_scroll_key,
  PAGE_DOWN_KEY,        handle_scroll_key,
  NO_KEY,               (void *)NULL,
  ENTER_KEY,            (void *)NULL,
  CLEAR_KEY,            vfy_clear_key,
  OPEN_DRAWER_KEY,      open_and_close_drawer,  /* Approval in state-engine. */
  UNKNOWN_KEY,          illegal_fn_key
};

static PROCESS_ELEMENT CPriceCorr_PROC[] =
{
  ENTER_KEY,            accept_active_item,
  NO_KEY,               delete_active_item,
  UNKNOWN_KEY,          (void *)NULL
};

static CONTROL_ELEMENT CPriceCorr_CTL[] =
{
  ENTER_KEY,            &Invoicing_ST,
  NO_KEY,               &Invoicing_ST,
  PRICE_TOO_LARGE_KEY,  &Invoicing_ST,      /* Item is deleted, back to inv. */
  UNKNOWN_KEY,          &CPriceCorr_ST
};

extern STATE_OBJ CPriceCorr_ST =
{
  ST_CPRICE_CORR,
  CPriceCorr_VW,
  no_DFLT,
  &DYN1K1n,
  CPriceCorr_VFY,
  Input_UVW,
  CPriceCorr_PROC,
  CPriceCorr_CTL
};

