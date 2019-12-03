/*
 *     Module Name       : ST_MAIN.H
 *
 *     Type              : General Functions Definitions
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
 * 11-Jan-1999 Added some st_inv.c prototypes                            P.M.
 * --------------------------------------------------------------------------
 * 21-Sep-2001 Added Pending Invoice functionality.                      M.W.
 * --------------------------------------------------------------------------
 * 27-Aug-2003 Added calc_incl_excl_vat_tcf() to do a correct
 *             calculation of the total carried forward totals.        J.D.M.
 * --------------------------------------------------------------------------
 * 28-Jul-2005 Summarize tax codes on total screen.                      M.W.
 * --------------------------------------------------------------------------
 * 01-Aug-2005 Detail payment on invoice.                                M.W.
 * --------------------------------------------------------------------------
 * 02-Aug-2005 Search customer by Fiscal number.                         M.W.
 * --------------------------------------------------------------------------
 * 16-Aug-2005 Select customer if searched by fisc_no                    M.W.
 * --------------------------------------------------------------------------
 * 12-Ago-2011 ACM - Change for sales Turkey						    ACM -
 * -------------------------------------------------------------------------- 
 */

#ifndef __ST_MAIN_H__
#define __ST_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {                /* Struct to pass total-info of an sub-item. */
    double incl_gds_val;        /* Goods value inclusive vat                 */
    double excl_gds_val;        /* Goods value exclusive vat                 */
    short  vat_no;              /* Vat number                                */
    short  packs;               /* Number off packs                          */
    short  voided;              /* Used for pack-count. TRUE means item void */
} SUB_ITEM_TOTAL;

extern INPUT_CONTROLLER Dwallamnt13n;
extern INPUT_CONTROLLER Dwallno3n;
extern INPUT_CONTROLLER DMode1K1;
extern INPUT_CONTROLLER Dopermnu1K1n;
extern INPUT_CONTROLLER XRcashid3K3n;
extern INPUT_CONTROLLER Dartno14KO14n;
extern INPUT_CONTROLLER Dprice10K10nd;
extern INPUT_CONTROLLER Dprice10K10ndm;
extern INPUT_CONTROLLER Dqty6K6nd;
extern INPUT_CONTROLLER DYN1K1n;
extern INPUT_CONTROLLER DYN1K1n2;
extern INPUT_CONTROLLER DpinK4nLln;

extern VERIFY_ELEMENT  OperatorMode_VFY[];
extern VERIFY_ELEMENT  OpTillWallet_VFY[];
extern VERIFY_ELEMENT  OpTillRefill_VFY[];
extern VERIFY_ELEMENT  OpTillLift_VFY[];
extern VERIFY_ELEMENT  OpXRCashno_VFY[];
extern VERIFY_ELEMENT  OpTillRefillOk_VFY[];

extern PROCESS_ELEMENT OpTillWallet_PROC[];
extern PROCESS_ELEMENT OperatorMode_PROC[];
extern PROCESS_ELEMENT OpTillRefill_PROC[];
extern PROCESS_ELEMENT OpTillRefillOk_PROC[];
extern PROCESS_ELEMENT OpXRCashno_PROC[];
extern PROCESS_ELEMENT OpTillLift_PROC[];

/*
 * ST_STBY.C
 */
extern void  Basic_VW(void);
extern void  PosStandBy_VW(void);
extern void  PosStandBy_UVW(void);
extern short vfy_s_key(_TCHAR *data, short key);
extern short proc_init_environment_records(_TCHAR *, short);
extern short discnt_vfy_and_start_artno(_TCHAR *, short);
extern short discnt_vfy_and_start_artno_Turkey(_TCHAR *, short);
extern short vfy_discount_amount(_TCHAR *, short);

#define FORCED_UPDATE     1
#define NO_FORCED_UPDATE  2
extern void  view_lan_state(short);
#ifndef NO_VIEW_POS_STATE
extern void  view_pos_state(void);
#endif

/*
 * ST_INV.C
 */
extern void InvoicingScrl_VW(void);
extern void InvoicingHead_VW(void);
extern void InvoicingDescr_VW(void);
extern void view_total(short screen);
extern void InvoicingSubt_VW(void);
extern void Input_UVW(void);
extern short delete_active_item(_TCHAR *data, short key);
extern short do_void_item(TM_INDX item_to_void);
extern short get_barcode_type(_TCHAR *barcd);
extern void  print_item(void);
extern short delete_active_item(_TCHAR *data, short key);
extern short do_void_item(TM_INDX item_to_void);
extern short vfy_art_qty(_TCHAR *data, short key);
extern short vfy_weight(_TCHAR *data, short key);
extern void  init_item(void);
extern short recalc_totals(SUB_ITEM_TOTAL *sit);
extern short calc_sub_item_totals(TM_INDX, ITEM_SNAM, SUB_ITEM_TOTAL *);
extern short handle_scroll_key(_TCHAR *data, short key);
extern short vfy_and_start_artno(_TCHAR *data, short key);
extern void  view_item(short screen);
extern short accept_active_item(_TCHAR *data, short key);
extern short vfy_art_price(_TCHAR *data, short key);
extern short vfy_weight_price(_TCHAR *data, short key);
extern short proc_new_line(_TCHAR *, short);
extern short recalc_subtotals(void);
extern short recalc_total_carried_forward(ITEM_SNAM);
extern void  calc_incl_excl_vat_tcf(void);
extern short calc_gds_value(ITEM_SNAM);
extern short proc_active_item(void);

extern int reg_turkey_load();           /* 12-Ago-2011 acm -*/
extern int reg_aditional_regedit_load();/* 27-Jan-2012 acm -*/
extern void memcpy_item(TM_ITEM_GROUP *c_item, TM_ITEM_GROUP *c_item_valeturkey); /* 12-Ago-2011 acm -*/
extern void setpayment_curr_type( int value); /* 12-Ago-2011 acm -*/
extern short vfy_and_start_artno_turkey(_TCHAR *data, short key);  /* 27-April-2012 acm - fix */

extern long prom_anniv_price_cupon;
extern long prom_anniv_price_gift ;
extern long prom_anniv_date_begin ;
extern long prom_anniv_date_end   ;

extern long promotion_horeca_date_begin;
extern long promotion_horeca_date_end  ;

/*
 * ST_TOTAL.C
 */
extern void  reset_paymentways(void);
extern short close_and_log_invoice(_TCHAR *data, short key);
extern short save_invoice(_TCHAR *data, short key);
extern short reprint_invoice(_TCHAR *data, short key);
extern short proc_till_lift_refill(_TCHAR *data, short key);
extern short vfy_wallet_amnt(_TCHAR *data, short key);
extern short start_till_lift(_TCHAR *data, short key);
extern short do_close_drawer(_TCHAR *data, short key);
extern void  print_invoice_type_unsorted(void);
extern void  reset_voucher_items(void);
extern double amount_due(void);

/*
 * ST_CUST.C
 */
extern short do_get_invoice_no(_TCHAR *data, short from);
extern void  inc_invoice_no(void);
extern void  dec_invoice_no(void);
extern short init_invoice(_TCHAR *data, short key);
extern short get_invoice_no(_TCHAR *);
extern short get_system_invoice_no(_TCHAR *);
extern short get_use_fisc_no(void);
extern short check_pending_invoice(POS_PEND_CUST_DEF *);

/* calculator */
extern short view_calc_scrn(_TCHAR *, short);
extern short vfy_calc(_TCHAR *, short);   
extern short inic_calc(_TCHAR *, short);   
extern  void tela_calc(void);            
extern  void inic_sinal(short);          
extern short entra_decimal(_TCHAR *, short);

/*
 * ST_OPMO.C
 */
extern short EndOfDay(_TCHAR *data, short key);
extern short EndOfShift(_TCHAR *data, short key);
extern short XRead(_TCHAR *data, short key);
extern short vfy_menu_option5(_TCHAR *data, short key);
extern short proc_clr_scrn(_TCHAR *data, short key);
extern short start_till_refill(_TCHAR *data, short key);
extern void  view_menu(short, short, short *, _TCHAR *);

/*
 * ST_OPER.C
 */
extern short supervisor_menu(_TCHAR *, short);
extern void  OperatorMode_UVW(void);
extern void  OperatorMode_VW(void);
extern void  OpTillRefillOk_UVW(void);
extern void  OpTillRefillOk_VW(void);
extern void  OpTillRefill_VW(void);
extern void  OpTillLift_VW(void);
extern void  OpXRCashno_VW(void);
extern short proc_set_date(_TCHAR *, short);

/*
 * ST_BOPER.C
 */

/*
 * ST_COPER.C
 */
extern short start_till_refill(_TCHAR *data, short key);
extern short bcoperator_menu(_TCHAR *data, short key);
extern short proc_x_read(short requested_cashno);

/*
 * ST_VLINE.C
 */

/*
 * ST_CASH.C
 */
extern short close_shift(_TCHAR *data, short key);
extern short setup_shift_on(SHIFT_ON_DEF *shift_on);
extern short setup_shift_off(SHIFT_OFF_DEF *shift_off);
extern short setup_shift_lift(SHIFT_LIFT_DEF *shift_lift);

#ifdef __cplusplus
}
#endif

#endif

