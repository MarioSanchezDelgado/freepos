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
 */

#include <stdio.h>
#include <ctype.h>

#include "fpos_defs.h"
#include "fpos_main.h"
#include "fpos_string_tools.h"
#include "fpos_input_mgr.h"
#include "fpos_input.h"
#include "fpos_func.h"
#include "fpos_errors.h"
#include "MapKeys.h"


char *empty = 0;

/* Format strings used for displaying by the input-manager                   */

static char star1[]        = "*";
static char uline0[]       = "";
static char uline1[]       = "_";
static char uline2[]       = "__";
static char uline3[]       = "___";
static char uline4[]       = "____";
static char uline6[]       = "______";
static char uline8[]       = "________";
static char uline10[]      = "__________";
static char uline10m[]     = "__________-";
static char uline11[]      = "___________"; 
static char uline14[]      = "______________";
static char uline15[]      = "_______________";
static char uline18[]      = "__________________";
static char uline28[]      = "____________________________";
static char uline30[]      = "______________________________";
static char uline32[]      = "________________________________";
static char uline40[]      = "________________________________________";

static char tillno_cashno6[] = "___/___";

static char date8[]        = "__-__-____";
static char cover_date8[]  = "_";
static char time4[]        = "00:00";
static char cover_time4[]  = "0";

/* Number of decimals in the qty-field may not be changed! This is because   */
/* in the source, the default qty-value is '1000'. If one change the number  */
/* of decimals of the qty-field, one must check sources for '1000' and       */
/* change this also!                                                         */
static char perc3p2[]      = "___.__%";
static char qty3p3[]       = "___.___";
static char cover_qty6[]   = "_";

static char price7p2[]     = "_______.__";
static char price7p2m[]    = "_______.__-";
static char cover_price10[]= "_";

static char price13p0k4[]  = "_,___,___,___,___";
static char price11p2k3[]  = "__,___,___,___.__";
static char price13p0k[]   = "_,___,___,___,___";
static char price13p2k[]   = "_,___,___,___,___.__";
static char price15p0k[]   = "   _,___,___,___,___";
static char price8p0k2[]   = "__,___,___";
static char price8p2k2[]   = "___,___.__";
static char cover_price13[]= "_";

static char price9p2k2[]   = "___,___,___.__";
static char price11p0k3[]  = "__,___,___,___";
static char cover_price11[]= "_";


/* Format strings used to format prices and strings for output to            */
/* screen, printer etc.                                                      */

static char nperc3p2[]        = "  0.00%";

static char nqty3p3m[]        = "  0.000-";
static char nqty3m[]          = "  0-";
static char nqty4p3m[]        = "   0.000-";
static char nqty4m[]          = "   0-";


static char npacks5m[]        = "    0-";

static char ndep5m[]          = " x  0-";
static char nprice6p2k1m[]    = "   ,  0.00-";
static char nprice8p0k2m[]    = " ,   ,  0-";
static char nprice9p0k2m[]    = "  ,   ,  0-";
static char nprice10k3m[]     = " ,   ,   ,  0-";
static char nprice9p2k2m[]    = "   ,   ,  0.00-";
static char nprice11p0k3m[]   = "  ,   ,   ,  0-";
static char nprice12p0k3m[]   = "   ,   ,   ,  0-";
static char nprice13p2[]      = "            0,00";    
static char nprice13p2k4[]    = " ,   ,   ,   ,  0.00";
static char nprice13p0k4[]    = ",   ,   ,   ,   ,  0";
static char nprice13p2m[]     = "            0.00-";
static char nprice11p2m[]     = "          0.00-";
static char nprice10p2m[]     = "         0.00-";
static char nprice9p2m[]      = "        0.00-";
static char nprice8p2k0m[]    = "       0.00-";
static char nprice7p2k0m[]    = "      0.00-";
static char nprice6p2[]       = "     0.00";
static char nprice16m[]       = "               0-";
static char nprice14m[]       = "             0-";
static char nprice13m[]       = "            0-";
static char nprice12m[]       = "           0-";
static char nprice10m[]       = "         0-";
static char nprice9[]         = "        0";

/*********/
static char cheque9p2k2m[]    = "   .   .  0,00-"; /* special for cheque dots */
static char cheque11p0k3m[]   = "  .   .   .  0-"; /* special for cheques dot */
/*********/

static char ncover_s0[]       = " 0";
static char ncover_su0[]      = " _0";
static char ncover_0[]        = "0";

static char nreplace_ks[]      = ", ";
static char nreplace_ps[]      = ". ";
static char nreplace_us[]      = "_ ";
static char nreplace_ksus[]    = ", _ ";
static char nreplace_none[]    = "";
static char nreplace_star[]    = " *";    /* Fill up spaces " " with stars "*" */

/* Keys allowed while entering a field                                               */
static char all_and_keys[] = "0123456789";
static char all_anp_keys[] = ".0123456789";

#define all_an_keys  all_anp_keys + 1


short is_function_key(short key)
{
  /* All function keys must be mapped to a code larger than NO_MAPPED_KEY.*/
  return (key<NO_MAPPED_KEY ? NO : YES);
} /* is_function_key() */

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
//    string_price8.format         = nprice8p0k2m;
//    string_price9.format         = nprice9p0k2m;
//    string_price11.format        = nprice11p0k3m;
//    string_price17.format        = nprice16m;
//    string_price15.format        = nprice14m;
//    string_price14.format        = nprice13m;
//    string_price13.format        = nprice12m;
//    string_price13_star.format   = nprice12m;
//    string_price13_2.format      = nprice10k3m;
//    string_price12.format        = nprice12p0k3m;
//    string_price11_star.format   = nprice10m;
//    string_price11_2.format      = nprice10m;
//    string_price11_2_star.format = nprice10m;
//    string_price9_2.format       = nprice6p2;
//    string_price19.format        = nprice13p0k4;
//    string_chequ11.format        = cheque11p0k3m; /* special for cheque dot */

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
//    string_price8.format         = nprice7p2k0m;
//    string_price9.format         = nprice8p2k0m;
//    string_price11.format        = nprice9p2k2m;
//    string_price17.format        = nprice13p2m;
//    string_price15.format        = nprice11p2m;
//    string_price14.format        = nprice10p2m;
//    string_price13.format        = nprice9p2m;
//    string_price13_star.format   = nprice9p2m;
//    string_price13_2.format      = nprice9p2m;
//    string_price12.format        = nprice9p2k2m;
//    string_price11_star.format   = nprice7p2k0m;
//    string_price11_2.format      = nprice7p2k0m;
//    string_price11_2_star.format = nprice7p2k0m;
//    string_price9_2.format       = nprice6p2;
//    string_price19.format        = nprice13p2k4;
///********/
//    string_chequ11.format      = cheque9p2k2m; /* special for cheque dot */
/********/
  }
  dsp_vr_calc11.format       = uline11;
  //string_preco11.format      = nprice13p2;
} /* setup_inp_mgr_price_fmt */

                             
/*
 *    VERIFY KEY FUNCTION - is keypress in a set of keys
 */


extern short is_key_in_set(SET_VERIFY *x, char *data, short key)
{
  char *keyptr = x->set;
  if (x->convert) {                    /* if there is a convert function    */
    key = x->convert(key);             /* execute it                        */
  }
  if (is_function_key(key)) {         /* Function keys ok                   */
    return key;
  }
  while (*keyptr) {                   /* check for Key in key-set           */
    if ( *keyptr == (char)((key) & 0x00FF) ) {
      return key;                     /* Key is in the x->set               */
    }
    else {
      keyptr++;
    }
  }
  err_invoke(INVALID_KEY_ERROR);      /* Illegal Keypress                   */

  return 0;
} /* is_key_in_set */

extern short is_digit(SET_VERIFY *x, char *data, short key)
{
  if (is_function_key(key)) {         /* Function keys ok                   */
    return key;
  }

  if ( isdigit((int)((key) & 0x00FF)) ) {
    return key;                     /* Key is in the x->set               */
  }

  return 0;
} /* is_digit */

extern short is_key_printing_char(SET_VERIFY *x, char *data, short key)
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
    key = save | (toupper(key) << 8);   /* Convert to upper-case            */
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

//static short dot_to_double_null_convert(short key)
//{
//  if(key == DOT_KEY) {
//    return DOUBLE_NULL_KEY;              /* is (% << 8)                   */
//  }
//  else {
//    return key;
//  }
//} /* dot_to_double_null_convert */

                             
extern SET_VERIFY numeric =
{
  is_key_in_set,
  all_an_keys,
  0                    /* Dot does not work here.               */
};

extern SET_VERIFY numeric_no_err =
{
  is_digit,
  "",
  0                    /* Dot does not work here.               */
};

extern SET_VERIFY numeric_punct =
{
  is_key_in_set,
  all_anp_keys,
  0
};

//
//extern SET_VERIFY numeric_dnull =
//{
//  is_key_in_set,
//  all_and_keys,
//  dot_to_double_null_convert      /* Used to enter double-nulls.           */
//};

extern SET_VERIFY printing_char_upr =
{
  is_key_printing_char,
  "",
  char_toupper_convert
};

extern SET_VERIFY no_data =
{
  is_key_in_set,              /* fn                                        */
  "",                     /* set                                       */
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
  '*'                                 /* character to show instead of key  */
};


extern TEMPLATE_PASSWORD dsp_1pincd4 =
{
  r2l_password,
  uline4,
  uline1,
  CASHIER_LOGON_WINDOW,
  7, 66,    /* was 54 */
  '*'
};


extern TEMPLATE_PASSWORD dsp_manpincd4 =
{
  r2l_password,
  uline4,
  uline1,
  ERROR_WINDOW_ROW2,
  0, 26,
  '*'
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

//extern TEMPLATE_STRING string_perc =
//{
//  r2l_display,                    /* Format/replace function           */
//  nperc3p2,                             /* Default, initial. in init_formats */
//  ncover_s0,                            /* Characters in format to cover     */
//  nreplace_none,                        /* Replace char's in format not cov. */
//  (char *)0                        /* Pointer to result after format    */
//};


//extern TEMPLATE_STRING string_price8 =
//{
//  r2l_display,
//  nprice7p2k0m,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//
//extern TEMPLATE_STRING string_price9 =
//{
//  r2l_display,
//  nprice9p0k2m,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//
//extern TEMPLATE_STRING string_price11 =
//{
//  r2l_display,
//  nprice9p2k2m,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//
//extern TEMPLATE_STRING string_artno14 =
//{
//  r2l_display,
//  uline14,
//  uline1,
//  nreplace_us,
//  (char *)0
//};
//
//
//extern TEMPLATE_STRING string_packs6 =
//{
//  r2l_display,
//  npacks5m,
//  ncover_s0,
//  nreplace_none,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_packs6_star =
//{
//  r2l_display,
//  npacks5m,
//  ncover_s0,
//  nreplace_star,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_time5 =
//{
//  r2l_display,
//  time4,
//  ncover_s0,
//  nreplace_none,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_qty8 =
//{
//  r2l_display,
//  nqty3p3m,
//  ncover_s0,
//  nreplace_none,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_qty8_star =
//{
//  r2l_display,
//  nqty3p3m,
//  ncover_s0,
//  nreplace_star,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_qty9 =
//{
//  r2l_display,
//  nqty4p3m,
//  ncover_s0,
//  nreplace_none,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_qty5 =
//{
//  r2l_display,
//  nqty4m,
//  ncover_s0,
//  nreplace_none,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_qty5_star =   
//{
//  r2l_display,
//  nqty4m,
//  ncover_s0,
//  nreplace_star,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_qty4 =
//{
//  r2l_display,
//  nqty3m,
//  ncover_s0,
//  nreplace_none,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_qty4_star =
//{
//  r2l_display,
//  nqty3m,
//  ncover_s0,
//  nreplace_star,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_dep_qty6 =
//{
//  r2l_display,
//  ndep5m,
//  ncover_s0,
//  nreplace_none,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price17 =
//{
//  r2l_display,
//  nprice13p2m,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price15 =
//{
//  r2l_display,
//  nprice11p2m,
//  ncover_s0,
//  nreplace_none,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price14 =
//{
//  r2l_display,
//  nprice10p2m,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price13 =
//{
//  r2l_display,
//  nprice9p2m,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price13_star =
//{
//  r2l_display,
//  nprice9p2m,
//  ncover_s0,
//  nreplace_star,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price13_2 =
//{
//  r2l_display,
//  nprice9p2m,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price12 =
//{
//  r2l_display,
//  nprice9p2k2m,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price11_star =
//{
//  r2l_display,
//  nprice7p2k0m,
//  ncover_s0,
//  nreplace_star,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price11_2 =
//{
//  r2l_display,
//  nprice7p2k0m,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price11_2_star =
//{
//  r2l_display,
//  nprice7p2k0m,
//  ncover_s0,
//  nreplace_star,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price9_2 =
//{
//  r2l_display,
//  nprice6p2,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_price19 =
//{
//  r2l_display,
//  nprice13p2k4,
//  ncover_s0,
//  nreplace_ks,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_chequ11 =
//{
//  r2l_display,
//  cheque9p2k2m,
//  ncover_s0,
//  nreplace_ps,
//  (char *)0
//};
//
//extern TEMPLATE_STRING string_preco11 = /* calculator */
//{
//  r2l_display,      
//  nprice13p2,             // 9999999999999,99
//  ncover_s0,
//  nreplace_ks,            
//  (char *)0            /* result */
//};
//
//
//extern TEMPLATE_DISPLAY1 dsp_artno15 =
//{
//  r2l_display,
//  uline15,
//  uline1,
//  INV_ART_INPUT_WINDOW,
//  0,0
//};
//
