/*
 *     Module Name       : POS_INP.H
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
 * 13-Dec-1999 Initial Release WinPOS                                  E.J.
 * --------------------------------------------------------------------------
 */

#ifndef __FPOS_INPUT_H__
#define __FPOS_INPUT_H__


#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MSR2_SIZE         42
#define MAX_OCIA1_SIZE        16
#define MAX_UNSOLICITED_SIZE  128

/*
 *    INPUT CONTROLS
 */

typedef struct setkeys
   {
   short (*fn)(struct setkeys *,char *,short);
   char *set;                          /* Set string of legal keys         */
   short (*convert)(short);            /* Key Conversion Routine.          */
   } SET_VERIFY;

#define BAD_FN_KEY         0x0300      /* This is CTRL-C... See pos_inp.c  */

extern char *empty;

/*
 *    Display Structures
 */ extern TEMPLATE_DISPLAY1 dsp_type;           /* Special code (Restaurant etc.)   */
extern TEMPLATE_DISPLAY1 dsp_vat;            /* VAT over Extra charge            */
extern TEMPLATE_DISPLAY1 dsp_min_extra;      /* Minimum extra amount             */
extern TEMPLATE_DISPLAY1 dsp_perc;           /* Extra amount percentage          */
extern TEMPLATE_DISPLAY1 dsp_cash_id3;       /* cashier id/no                    */
extern TEMPLATE_DISPLAY1 XRdsp_cash_id3;     /* cashier id/no                    */
extern TEMPLATE_DISPLAY1 dsp_cash_nm30;      /* cashier name                     */
extern TEMPLATE_DISPLAY1 dsp_start_float13;  /* cash-float on logon time         */
extern TEMPLATE_DISPLAY1 dsp_price10;        /* with or without 2 decimals and . */
extern TEMPLATE_DISPLAY1 dsp_price10m;       /* minus extra.                     */
extern TEMPLATE_DISPLAY1 dsp_cust6;          /* customer number                  */
extern TEMPLATE_DISPLAY1 dsp_cust2;          /* customer store number            */
extern TEMPLATE_DISPLAY1 dsp_cust30;         /* customer name                    */
extern TEMPLATE_DISPLAY1 dsp_artno14;        /* article number                   */
extern TEMPLATE_DISPLAY1 dsp_qty6;           /* quantity / weight                */
extern TEMPLATE_DISPLAY1 oper_menu1;         /* get operator menu option         */
extern TEMPLATE_DISPLAY1 dsp_YN1;            /* Enter or No                      */
extern TEMPLATE_DISPLAY1 dsp_YN2;            /* Enter or No in total screen      */
extern TEMPLATE_DISPLAY1 dsp_payd_amnt11;    /* Amount paid                      */
extern TEMPLATE_DISPLAY1 dsp_wallet_no3;     /* Wallet number                    */
extern TEMPLATE_DISPLAY1 dsp_tillno_cashno6; /* Till-no / Cash-no                */
extern TEMPLATE_DISPLAY1 dsp_wallamnt15;
extern TEMPLATE_DISPLAY1 dsp_wallno3;
extern TEMPLATE_DISPLAY1 dsp_1mode1;
extern TEMPLATE_DISPLAY1 dsp_cashier3;
extern TEMPLATE_DISPLAY1 oper_menu2;
extern TEMPLATE_DISPLAY1 dsp_float13p2k2;
extern TEMPLATE_DISPLAY1 dsp_dateU;
extern TEMPLATE_DISPLAY1 dsp_time4;
extern TEMPLATE_DISPLAY1 dsp_float13p2k3;

extern TEMPLATE_DISPLAY1 dsp_keypos; //freepos

extern TEMPLATE_DISPLAY1 dsp_exit;
extern TEMPLATE_DISPLAY1 dsp_vr_calc11;      /* calculator                       */
extern TEMPLATE_DISPLAY1 dsp_art_descr;
extern TEMPLATE_DISPLAY1 dsp_voucher8;       /* voucher number                   */

extern TEMPLATE_PASSWORD dsp_pincd4;         /* DSP_PINCD4 uses DSP1_PINCD4      */
extern TEMPLATE_PASSWORD dsp_1pincd4;
extern TEMPLATE_PASSWORD dsp_manpincd4;

extern TEMPLATE_DISPLAY1 oper_menu3;
extern TEMPLATE_DISPLAY1 cfee_menu1;

extern TEMPLATE_DISPLAY1 dsp_select_printer;
extern TEMPLATE_DISPLAY1 dsp_select_cust;


extern TEMPLATE_DISPLAY1 dsp_perception_document    ;
extern TEMPLATE_DISPLAY1 dsp_perception_name        ;

extern TEMPLATE_DISPLAY1 dsp_document    ;
extern TEMPLATE_DISPLAY1 dsp_name        ;

/*
 * Format string Structures
 */

extern TEMPLATE_STRING string_perc;
extern TEMPLATE_STRING string_artno14;
extern TEMPLATE_STRING string_qty6;
extern TEMPLATE_STRING string_packs6;
extern TEMPLATE_STRING string_packs6_star;
extern TEMPLATE_STRING string_time5;
extern TEMPLATE_STRING string_qty8;
extern TEMPLATE_STRING string_qty8_star;
extern TEMPLATE_STRING string_qty9;
extern TEMPLATE_STRING string_qty5;
extern TEMPLATE_STRING string_qty5_star;
extern TEMPLATE_STRING string_qty4;
extern TEMPLATE_STRING string_qty4_star;
extern TEMPLATE_STRING string_dep_qty6;
extern TEMPLATE_STRING string_price19;
extern TEMPLATE_STRING string_price17;
extern TEMPLATE_STRING string_price15;
extern TEMPLATE_STRING string_price14;
extern TEMPLATE_STRING string_price13;
extern TEMPLATE_STRING string_price13_star;
extern TEMPLATE_STRING string_price13_2;
extern TEMPLATE_STRING string_price12;
extern TEMPLATE_STRING string_price11;
extern TEMPLATE_STRING string_price11_star;
extern TEMPLATE_STRING string_price11_2;
extern TEMPLATE_STRING string_price11_2_star;
extern TEMPLATE_STRING string_price9_2;
extern TEMPLATE_STRING string_price9;
extern TEMPLATE_STRING string_price8;


/* 25-Set-2012 acm - { */
extern TEMPLATE_DISPLAY1 dsp_artno15;        /* article number                   */
/* 25-Set-2012 acm - } */

/********/
 
extern TEMPLATE_STRING string_chequ11;        /* special cheque structure */
 
/********/
extern TEMPLATE_STRING string_preco11;        /* calculator */ 
   


/*
 *    Verify Structures
 */

extern SET_VERIFY numeric;
extern SET_VERIFY numeric_no_err;
extern SET_VERIFY numeric_punct;
extern SET_VERIFY numeric_dnull;
extern SET_VERIFY printing_char_upr;
extern SET_VERIFY no_data;
extern SET_VERIFY nothing;


/*
 * Prototypes
 */
extern void setup_inp_mgr_price_fmt(short);

#ifdef __cplusplus
}
#endif

#endif
