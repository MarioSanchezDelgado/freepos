/*
 *     Module Name       : POS_INP.C
 *
 *     Type              : Application Input Structures
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
 * 31-Oct-2001 Added star (***amount) for amount and quantity            M.W.
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Changes for Article Finder.                               M.W.
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Added Spanish keys for Article Finder.                    M.W.
 * --------------------------------------------------------------------------
 * 22-Jan-2002 Removed Spanish keys for Article Finder (NOT USED).       M.W.
 * --------------------------------------------------------------------------
 * 23-Jan-2002 Changes for Credit note. Ask for amount if offline.       M.W.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 06-May-2004 Added vat amounts on total section.                       M.W.
 * --------------------------------------------------------------------------
 * 28-Jul-2005 Summarize tax codes on total screen.                      M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 * 16-Aug-2005 Select customer if searched by fisc_no                    M.W.
 * --------------------------------------------------------------------------
 * 23-Feb-2006 Added string_price11_star.                              J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
                                            /* Pos (library) include files   */
#include "misc_tls.h"
#include "stri_tls.h"

                                            /* Toolsset include files.       */
#include "inp_mgr.h"   
#include "tm_mgr.h"
#include "err_mgr.h"

#include "pos_keys.h"                       /* Application include files.    */
#include "pos_recs.h"
#include "pos_tm.h"
#include "WPos_mn.h"
#include "pos_inp.h"
#include "pos_errs.h"
#include "pos_func.h"

#include "mapkeys.h"

_TCHAR *empty = _T("");

/* Format strings used for displaying by the input-manager                   */

static _TCHAR star1[]        = _T("*");
static _TCHAR uline0[]       = _T("");
static _TCHAR uline1[]       = _T("_");
static _TCHAR uline2[]       = _T("__");
static _TCHAR uline3[]       = _T("___");
static _TCHAR uline4[]       = _T("____");
static _TCHAR uline6[]       = _T("______");
static _TCHAR uline8[]       = _T("________");
static _TCHAR uline10[]      = _T("__________");
static _TCHAR uline10m[]     = _T("__________-");
static _TCHAR uline11[]      = _T("___________"); 
static _TCHAR uline14[]      = _T("______________");
static _TCHAR uline15[]      = _T("_______________");
static _TCHAR uline18[]      = _T("__________________");
static _TCHAR uline28[]      = _T("____________________________");
static _TCHAR uline30[]      = _T("______________________________");
static _TCHAR uline32[]      = _T("________________________________");
static _TCHAR uline40[]      = _T("________________________________________");

static _TCHAR tillno_cashno6[] = _T("___/___");

static _TCHAR date8[]        = _T("__-__-____");
static _TCHAR cover_date8[]  = _T("_");
static _TCHAR time4[]        = _T("00:00");
static _TCHAR cover_time4[]  = _T("0");

/* Number of decimals in the qty-field may not be changed! This is because   */
/* in the source, the default qty-value is '1000'. If one change the number  */
/* of decimals of the qty-field, one must check sources for '1000' and       */
/* change this also!                                                         */
static _TCHAR perc3p2[]      = _T("___.__%");
static _TCHAR qty3p3[]       = _T("___.___");
static _TCHAR cover_qty6[]   = _T("_");

static _TCHAR price7p2[]     = _T("_______.__");
static _TCHAR price7p2m[]    = _T("_______.__-");
static _TCHAR cover_price10[]= _T("_");

static _TCHAR price13p0k4[]  = _T("_,___,___,___,___");
static _TCHAR price11p2k3[]  = _T("__,___,___,___.__");
static _TCHAR price13p0k[]   = _T("_,___,___,___,___");
static _TCHAR price13p2k[]   = _T("_,___,___,___,___.__");
static _TCHAR price15p0k[]   = _T("   _,___,___,___,___");
static _TCHAR price8p0k2[]   = _T("__,___,___");
static _TCHAR price8p2k2[]   = _T("___,___.__");
static _TCHAR cover_price13[]= _T("_");

static _TCHAR price9p2k2[]   = _T("___,___,___.__");
static _TCHAR price11p0k3[]  = _T("__,___,___,___");
static _TCHAR cover_price11[]= _T("_");


/* Format strings used to format prices and strings for output to            */
/* screen, printer etc.                                                      */

static _TCHAR nperc3p2[]        = _T("  0.00%");

static _TCHAR nqty3p3m[]        = _T("  0.000-");
static _TCHAR nqty3m[]          = _T("  0-");
static _TCHAR nqty4p3m[]        = _T("   0.000-");
static _TCHAR nqty4m[]          = _T("   0-");


static _TCHAR npacks5m[]        = _T("    0-");

static _TCHAR ndep5m[]          = _T(" x  0-");
static _TCHAR nprice6p2k1m[]    = _T("   ,  0.00-");
static _TCHAR nprice8p0k2m[]    = _T(" ,   ,  0-");
static _TCHAR nprice9p0k2m[]    = _T("  ,   ,  0-");
static _TCHAR nprice10k3m[]     = _T(" ,   ,   ,  0-");
static _TCHAR nprice9p2k2m[]    = _T("   ,   ,  0.00-");
static _TCHAR nprice11p0k3m[]   = _T("  ,   ,   ,  0-");
static _TCHAR nprice12p0k3m[]   = _T("   ,   ,   ,  0-");
static _TCHAR nprice13p2[]      = _T("            0,00");    
static _TCHAR nprice13p2k4[]    = _T(" ,   ,   ,   ,  0.00");
static _TCHAR nprice13p0k4[]    = _T(",   ,   ,   ,   ,  0");
static _TCHAR nprice13p2m[]     = _T("            0.00-");
static _TCHAR nprice11p2m[]     = _T("          0.00-");
static _TCHAR nprice10p2m[]     = _T("         0.00-");
static _TCHAR nprice9p2m[]      = _T("        0.00-");
static _TCHAR nprice8p2k0m[]    = _T("       0.00-");
static _TCHAR nprice7p2k0m[]    = _T("      0.00-");
static _TCHAR nprice6p2[]       = _T("     0.00");
static _TCHAR nprice16m[]       = _T("               0-");
static _TCHAR nprice14m[]       = _T("             0-");
static _TCHAR nprice13m[]       = _T("            0-");
static _TCHAR nprice12m[]       = _T("           0-");
static _TCHAR nprice10m[]       = _T("         0-");
static _TCHAR nprice9[]         = _T("        0");

/*********/
static _TCHAR cheque9p2k2m[]    = _T("   .   .  0,00-"); /* special for cheque dots */
static _TCHAR cheque11p0k3m[]   = _T("  .   .   .  0-"); /* special for cheques dot */
/*********/

static _TCHAR ncover_s0[]       = _T(" 0");
static _TCHAR ncover_su0[]      = _T(" _0");
static _TCHAR ncover_0[]        = _T("0");

static _TCHAR nreplace_ks[]      = _T(", ");
static _TCHAR nreplace_ps[]      = _T(". ");
static _TCHAR nreplace_us[]      = _T("_ ");
static _TCHAR nreplace_ksus[]    = _T(", _ ");
static _TCHAR nreplace_none[]    = _T("");
static _TCHAR nreplace_star[]    = _T(" *");    /* Fill up spaces " " with stars "*" */

/* Keys allowed while entering a field                                               */
static _TCHAR all_and_keys[] = _T("0123456789");
static _TCHAR all_anp_keys[] = _T(".0123456789");

#define all_an_keys  all_anp_keys + 1

/*---------------------------------------------------------------------------*/
/*                       setup_inp_mgr_price_fmt                             */
/*---------------------------------------------------------------------------*/
void setup_inp_mgr_price_fmt(short price_dec)
{
  if( price_dec != DECIMALS_YES ) {
    /* All price-fields have NO decimal point */
    dsp_min_extra.format         = price8p0k2;
    dsp_price10.format           = uline10;
    dsp_price10m.format          = uline10m;
    dsp_start_float13.format     = price13p0k;
    dsp_payd_amnt11.format       = price11p0k3;
    dsp_wallamnt15.format        = price13p0k;
    dsp_float13p2k2.format       = price15p0k;
    dsp_float13p2k3.format       = price13p0k;
    string_price8.format         = nprice8p0k2m;
    string_price9.format         = nprice9p0k2m;
    string_price11.format        = nprice11p0k3m;
    string_price17.format        = nprice16m;
    string_price15.format        = nprice14m;
    string_price14.format        = nprice13m;
    string_price13.format        = nprice12m;
    string_price13_star.format   = nprice12m;
    string_price13_2.format      = nprice10k3m;
    string_price12.format        = nprice12p0k3m;
    string_price11_star.format   = nprice10m;
    string_price11_2.format      = nprice10m;
    string_price11_2_star.format = nprice10m;
    string_price9_2.format       = nprice6p2;
    string_price19.format        = nprice13p0k4;
    string_chequ11.format        = cheque11p0k3m; /* special for cheque dot */

  }
  else {
    /* all price-fields have a decimal point */
    dsp_min_extra.format         = price8p2k2;
    dsp_price10.format           = price7p2;
    dsp_price10m.format          = price7p2m;
    dsp_start_float13.format     = price11p2k3;
    dsp_payd_amnt11.format       = price9p2k2;
    dsp_wallamnt15.format        = price13p2k;
    dsp_float13p2k2.format       = price13p2k;
    dsp_float13p2k3.format       = price13p2k;
    string_price8.format         = nprice7p2k0m;
    string_price9.format         = nprice8p2k0m;
    string_price11.format        = nprice9p2k2m;
    string_price17.format        = nprice13p2m;
    string_price15.format        = nprice11p2m;
    string_price14.format        = nprice10p2m;
    string_price13.format        = nprice9p2m;
    string_price13_star.format   = nprice9p2m;
    string_price13_2.format      = nprice9p2m;
    string_price12.format        = nprice9p2k2m;
    string_price11_star.format   = nprice7p2k0m;
    string_price11_2.format      = nprice7p2k0m;
    string_price11_2_star.format = nprice7p2k0m;
    string_price9_2.format       = nprice6p2;
    string_price19.format        = nprice13p2k4;
/********/
    string_chequ11.format      = cheque9p2k2m; /* special for cheque dot */
/********/
  }
  dsp_vr_calc11.format       = uline11;
  string_preco11.format      = nprice13p2;
} /* setup_inp_mgr_price_fmt */

                             
/*
 *    VERIFY KEY FUNCTION - is keypress in a set of keys
 */


extern short is_key_in_set(SET_VERIFY *x, _TCHAR *data, short key)
{
  _TCHAR *keyptr = x->set;
  if (x->convert) {                    /* if there is a convert function    */
    key = x->convert(key);             /* execute it                        */
  }
  if (is_function_key(key)) {         /* Function keys ok                   */
    return key;
  }
  while (*keyptr) {                   /* check for Key in key-set           */
    if ( *keyptr == (_TCHAR)((key) & 0x00FF) ) {
      return key;                     /* Key is in the x->set               */
    }
    else {
      keyptr++;
    }
  }
  err_invoke(INVALID_KEY_ERROR);      /* Illegal Keypress                   */

  return 0;
} /* is_key_in_set */

extern short is_digit(SET_VERIFY *x, _TCHAR *data, short key)
{
  if (is_function_key(key)) {         /* Function keys ok                   */
    return key;
  }

  if ( isdigit((int)((key) & 0x00FF)) ) {
    return key;                     /* Key is in the x->set               */
  }

  return 0;
} /* is_digit */

extern short is_key_printing_char(SET_VERIFY *x, _TCHAR *data, short key)
{
  if (is_function_key(key)) {         /* Function keys ok                   */
    return key;
  }

  if (x->convert) {                    /* if there is a convert function    */
    key = x->convert(key);             /* execute it                        */
  }

  if ( isprint((int)((key) & 0x00FF)) ) {
      return key;                     /* Key is in the x->set               */
  }

  err_invoke(INVALID_KEY_ERROR);      /* Illegal Keypress                   */

  return 0;
} /* is_key_printing_char */

/*
 *    Keypress Verification Structures
 */

static short inp_toupper(short key)
{
  short save = key & 0x00FF;

  if (key >> 8) {
    key = (key >> 8) & 0x00FF;
    key = save | (_totupper(key) << 8);   /* Convert to upper-case            */
  }

  return key;
} /* inp_toupper */


static short return_bad_fn_key(short key)
{
  if (is_function_key(key)) {         /* Function keys ok                 */
    return key;
  }

  return BAD_FN_KEY;
} /* return_bad_fn_key */

static short char_toupper_convert(short key)
{
  if (islower(key)) {
    key = toupper(key);               /* Convert to upper-case            */
  }

  return key;
} /* char_toupper_convert */

static short dot_to_double_null_convert(short key)
{
  if(key == DOT_KEY) {
    return DOUBLE_NULL_KEY;              /* is (% << 8)                   */
  }
  else {
    return key;
  }
} /* dot_to_double_null_convert */

                             
extern SET_VERIFY numeric =
{
  is_key_in_set,
  all_an_keys,
  (void *)NULL                    /* Dot does not work here.               */
};

extern SET_VERIFY numeric_no_err =
{
  is_digit,
  _T(""),
  (void *)NULL                    /* Dot does not work here.               */
};

extern SET_VERIFY numeric_punct =
{
  is_key_in_set,
  all_anp_keys,
  (void *)NULL
};


extern SET_VERIFY numeric_dnull =
{
  is_key_in_set,
  all_and_keys,
  dot_to_double_null_convert      /* Used to enter double-nulls.           */
};

extern SET_VERIFY printing_char_upr =
{
  is_key_printing_char,
  _T(""),
  char_toupper_convert
};

extern SET_VERIFY no_data =
{
  is_key_in_set,              /* fn                                        */
  _T(""),                     /* set                                       */
  return_bad_fn_key           /* convert                                   */
};


/* INPUT DISPLAY STRUCTURES **************************************************/

extern TEMPLATE_DISPLAY1 dsp_type =
{
  r2l_display,
  uline1,
  uline1,
  OPERATOR_WINDOW,
  18, 71
};


extern TEMPLATE_DISPLAY1 dsp_vat =
{
  r2l_display,
  uline1,
  uline1,
  OPERATOR_WINDOW,
  18, 68
};


extern TEMPLATE_DISPLAY1 dsp_min_extra =
{
  r2l_display,
  price8p2k2,
  uline1,
  OPERATOR_WINDOW,
  18, 54
};


extern TEMPLATE_DISPLAY1 dsp_perc =
{
  r2l_display,
  perc3p2,
  uline1,
  OPERATOR_WINDOW,
  18, 44
};


extern TEMPLATE_DISPLAY1 dsp_cash_id3 =
{
  r2l_display,
  uline3,
  uline1,
  CASHIER_LOGON_WINDOW,
  7,37
};


extern TEMPLATE_DISPLAY1 XRdsp_cash_id3 =
{
  r2l_display,
  uline3,
  uline1,
  OPERATOR_WINDOW,
  16,22
};

extern TEMPLATE_DISPLAY1 dsp_cashier3 =
{
  r2l_display,
  uline3,
  uline1,
  OPERATOR_WINDOW,
  15, 37
};


extern TEMPLATE_DISPLAY1 dsp_cash_nm30=
{
  l2r_display,
  uline30,
  uline1,
  CASHIER_LOGON_WINDOW,
  7,41
};



extern TEMPLATE_PASSWORD dsp_pincd4 =
{
  r2l_password,
  uline4,
  uline1,
  CASHIER_LOGON_WINDOW,
  8,37,
  _T('*')                                   /* character to show instead of key  */
};


extern TEMPLATE_PASSWORD dsp_1pincd4 =
{
  r2l_password,
  uline4,
  uline1,
  CASHIER_LOGON_WINDOW,
  7, 66,    /* was 54 */
  _T('*')
};


extern TEMPLATE_PASSWORD dsp_manpincd4 =
{
  r2l_password,
  uline4,
  uline1,
  ERROR_WINDOW_ROW2,
  0, 26,
  _T('*')
};


extern TEMPLATE_DISPLAY1 dsp_start_float13=
{
  r2l_display,
  price11p2k3,
  cover_price13,
  CASHIER_LOGON_WINDOW,
  10,37
};


extern TEMPLATE_DISPLAY1 dsp_cust6 =
{
  r2l_display,
  uline6,
  uline1,
  INV_HEADER_WINDOW,
  0,17
};


extern TEMPLATE_DISPLAY1 dsp_cust2 =
{
  r2l_display,
  uline2,
  uline1,
  INV_HEADER_WINDOW,
  0,14
};


extern TEMPLATE_DISPLAY1 dsp_cust30 =
{
  l2r_display,
  uline30,
  uline1,
  INV_HEADER_WINDOW,
  0,24
};


extern TEMPLATE_DISPLAY1 dsp_artno14 =
{
  r2l_display,
  uline14,
  uline1,
  INV_ART_INPUT_WINDOW,
  0,0
};


extern TEMPLATE_DISPLAY1 dsp_qty6 =
{
  r2l_display,
  qty3p3,
  cover_qty6,
  INV_ART_INPUT_WINDOW,
  0,0
};


extern TEMPLATE_DISPLAY1 oper_menu1 =
{
  r2l_display,
  uline1,
  uline1,
  OPERATOR_WINDOW,
  13,38
};

extern TEMPLATE_DISPLAY1 oper_menu2 =
{ 
  r2l_display,
  uline2,
  uline1,
  OPERATOR_WINDOW,
  16, 38
};

extern TEMPLATE_DISPLAY1 oper_menu3 =
{
  r2l_display,
  uline1,
  uline1,
  OPERATOR_WINDOW,
  15,38
};

extern TEMPLATE_DISPLAY1 cfee_menu1 =
{
  r2l_display,
  uline1,
  uline1,
  OPERATOR_WINDOW,
  15,32
};

extern TEMPLATE_DISPLAY1 dsp_price10 =
{
  r2l_display,
  price7p2,
  cover_price10,
  INV_ART_INPUT_WINDOW,
  0,0
};


extern TEMPLATE_DISPLAY1 dsp_price10m =
{
  r2l_display,
  price7p2m,
  cover_price10,
  INV_ART_INPUT_WINDOW,
  0,0
};


extern TEMPLATE_DISPLAY1 dsp_YN1 =
{
  r2l_display,
  uline1,
  uline1,
  INV_ART_INPUT_WINDOW,
  0,0
};
                    
extern TEMPLATE_DISPLAY1 dsp_YN2 =
{
  r2l_display,
  uline1,
  uline1,
  TOTAL_INPUT_WINDOW,
  0,0
};

extern TEMPLATE_DISPLAY1 dsp_payd_amnt11 =
{
  r2l_display,
  price9p2k2,
  cover_price11,
  TOTAL_INPUT_WINDOW,
  0,0
};

                    
extern TEMPLATE_DISPLAY1 dsp_wallamnt15 =
{ 
  r2l_display,
  price13p2k,
  uline1,
  OPERATOR_WINDOW,
  16, 39
};


extern TEMPLATE_DISPLAY1 dsp_wallet_no3 =
{
  r2l_display,
  uline3,
  uline1,
  OPERATOR_WINDOW,
  17,32
};

extern TEMPLATE_DISPLAY1 dsp_wallno3 =
{ 
  r2l_display,
  uline3,
  uline1,
  OPERATOR_WINDOW,
  18, 39
};


extern TEMPLATE_DISPLAY1 dsp_tillno_cashno6 =
{
  r2l_display,
  tillno_cashno6,
  uline1,
  INV_HEADER_WINDOW,
  0,0
};

extern TEMPLATE_DISPLAY1 dsp_1mode1 =
{
  r2l_display,
  uline1,
  uline1,
  OPERATOR_WINDOW,
  18, 53
};

extern TEMPLATE_DISPLAY1 dsp_float13p2k2 =
{
  r2l_display,
  price13p2k,
  uline1,
  OPERATOR_WINDOW,
  18, 23
};

extern TEMPLATE_DISPLAY1 dsp_float13p2k3 =
{
  r2l_display,
  price13p2k,
  uline1,
  OPERATOR_WINDOW,
  17, 44
};


extern TEMPLATE_DISPLAY1 dsp_dateU =
{
  r2l_display,
  uline8,
  uline1,
  OPERATOR_WINDOW,
  17, 39
};

extern TEMPLATE_DISPLAY1 dsp_time4 =
{
  r2l_display,
  time4,
  cover_time4,
  OPERATOR_WINDOW,
  17, 39
};

extern TEMPLATE_DISPLAY1 dsp_keypos =
{ 
  r2l_display,
  uline0,
  uline0,
  CASHIER_LOGON_WINDOW,
  0, 0
};

extern TEMPLATE_DISPLAY1 dsp_exit =
{ 
  r2l_display,
  uline1,
  uline1,
  CASHIER_LOGON_WINDOW,
  7, 48
};

extern TEMPLATE_DISPLAY1 dsp_vr_calc11 = /* calculator */
{ 
  r2l_display,
  uline11,
  uline1,
  OPERATOR_WINDOW,
  18, 37
};

extern TEMPLATE_DISPLAY1 dsp_invoiceno6 =
{
  r2l_display,
  uline6,
  uline1,
  OPERATOR_WINDOW,
  16, 37
};

extern TEMPLATE_DISPLAY1 dsp_art_descr =
{ 
  l2r_display,
  uline32,
  uline1,
  ART_FIND_LIST_WINDOW,
  0, 0
};

extern TEMPLATE_DISPLAY1 dsp_voucher8 =
{
  r2l_display,
  uline15,//uline8,
  uline15,//uline8,
  TOTAL_INPUT_WINDOW,
  0, 9
};

extern TEMPLATE_DISPLAY1 dsp_select_printer =
{ 
  r2l_display,
  uline1,
  uline1,
  SELECT_PRINTER_WINDOW,
  5, 48
};

extern TEMPLATE_DISPLAY1 dsp_select_cust =
{ 
  r2l_display,
  uline1,
  uline1,
  SELECT_CUST_WINDOW,
  14, 54
};

extern TEMPLATE_DISPLAY1 dsp_document=
{
  l2r_display, 
  uline8,
  uline1,
  PAYMENT_2_WINDOW,// TOTAL_INPUT_WINDOW_PERCEPTION,
  14,23
};

extern TEMPLATE_DISPLAY1 dsp_name=
{
  l2r_display,
  uline40,
  uline1,
  PAYMENT_2_WINDOW, ///TOTAL_INPUT_WINDOW_PERCEPTION,
  15,23
};


extern TEMPLATE_DISPLAY1 dsp_perception_document=
{
  l2r_display, 
  uline8,
  uline1,
  PAYMENT_2_WINDOW,// TOTAL_INPUT_WINDOW_PERCEPTION,
  14,23
};

extern TEMPLATE_DISPLAY1 dsp_perception_name=
{
  l2r_display,
  uline40,
  uline1,
  PAYMENT_2_WINDOW, ///TOTAL_INPUT_WINDOW_PERCEPTION,
  15,23
};




/*---------------------------------------------------------------------------*/
/* Format string definitions                                                 */
/*---------------------------------------------------------------------------*/

extern TEMPLATE_STRING string_perc =
{
  r2l_create_string,                    /* Format/replace function           */
  nperc3p2,                             /* Default, initial. in init_formats */
  ncover_s0,                            /* Characters in format to cover     */
  nreplace_none,                        /* Replace char's in format not cov. */
  (_TCHAR *)NULL                        /* Pointer to result after format    */
};


extern TEMPLATE_STRING string_price8 =
{
  r2l_create_string,
  nprice7p2k0m,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};


extern TEMPLATE_STRING string_price9 =
{
  r2l_create_string,
  nprice9p0k2m,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};


extern TEMPLATE_STRING string_price11 =
{
  r2l_create_string,
  nprice9p2k2m,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};


extern TEMPLATE_STRING string_artno14 =
{
  r2l_create_string,
  uline14,
  uline1,
  nreplace_us,
  (_TCHAR *)NULL
};


extern TEMPLATE_STRING string_packs6 =
{
  r2l_create_string,
  npacks5m,
  ncover_s0,
  nreplace_none,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_packs6_star =
{
  r2l_create_string,
  npacks5m,
  ncover_s0,
  nreplace_star,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_time5 =
{
  r2l_create_string,
  time4,
  ncover_s0,
  nreplace_none,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_qty8 =
{
  r2l_create_string,
  nqty3p3m,
  ncover_s0,
  nreplace_none,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_qty8_star =
{
  r2l_create_string,
  nqty3p3m,
  ncover_s0,
  nreplace_star,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_qty9 =
{
  r2l_create_string,
  nqty4p3m,
  ncover_s0,
  nreplace_none,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_qty5 =
{
  r2l_create_string,
  nqty4m,
  ncover_s0,
  nreplace_none,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_qty5_star =   
{
  r2l_create_string,
  nqty4m,
  ncover_s0,
  nreplace_star,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_qty4 =
{
  r2l_create_string,
  nqty3m,
  ncover_s0,
  nreplace_none,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_qty4_star =
{
  r2l_create_string,
  nqty3m,
  ncover_s0,
  nreplace_star,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_dep_qty6 =
{
  r2l_create_string,
  ndep5m,
  ncover_s0,
  nreplace_none,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price17 =
{
  r2l_create_string,
  nprice13p2m,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price15 =
{
  r2l_create_string,
  nprice11p2m,
  ncover_s0,
  nreplace_none,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price14 =
{
  r2l_create_string,
  nprice10p2m,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price13 =
{
  r2l_create_string,
  nprice9p2m,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price13_star =
{
  r2l_create_string,
  nprice9p2m,
  ncover_s0,
  nreplace_star,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price13_2 =
{
  r2l_create_string,
  nprice9p2m,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price12 =
{
  r2l_create_string,
  nprice9p2k2m,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price11_star =
{
  r2l_create_string,
  nprice7p2k0m,
  ncover_s0,
  nreplace_star,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price11_2 =
{
  r2l_create_string,
  nprice7p2k0m,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price11_2_star =
{
  r2l_create_string,
  nprice7p2k0m,
  ncover_s0,
  nreplace_star,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price9_2 =
{
  r2l_create_string,
  nprice6p2,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_price19 =
{
  r2l_create_string,
  nprice13p2k4,
  ncover_s0,
  nreplace_ks,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_chequ11 =
{
  r2l_create_string,
  cheque9p2k2m,
  ncover_s0,
  nreplace_ps,
  (_TCHAR *)NULL
};

extern TEMPLATE_STRING string_preco11 = /* calculator */
{
  r2l_create_string,      
  nprice13p2,             // 9999999999999,99
  ncover_s0,
  nreplace_ks,            
  (_TCHAR *)NULL            /* result */
};


/* 25-Set-2012 acm - { */

extern TEMPLATE_DISPLAY1 dsp_artno15 =
{
  r2l_display,
  uline15,
  uline1,
  INV_ART_INPUT_WINDOW,
  0,0
};

/* 25-Set-2012 acm - } */
