/*
 *     Module Name       : POS_TM.H
 *
 *     Type              : Transaction Manager structure definitions
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
 * 06-Jan-2000 Added extra fields to transaction manager structure     P.M
 *             to store additional article information and the         
 *             discounts obtained from the multisam discount actions 
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Changes for Article Finder.                               M.W.
 * --------------------------------------------------------------------------
 * 22-Jul-2002 Multisam Phase 3.                                         P.M.
 * --------------------------------------------------------------------------
 * 16-Dec-2003 Added Donation functionality.                             M.W.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_TM_H__
#define __POS_TM_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TM_ROOT                -1

#define C_ITEM             -31000                /* Must be unique....      */

#define TM_ITEM_NAME            1
#define TM_SHFT_NAME            2
#define TM_ARTF_NAME            3


#define TM_ITEM_MAXI         1000                /* seq number is 3 pos.    */
#define TM_SHFT_MAXI          100
#define TM_ARTF_MAXI          500

#define ART_GROUP              11                /* Indicates TM_ARTI_LINE  */
#define DISCNT_GROUP           12                /* Indicates TM_DISC_LINE  */
#define DEPOSIT_GROUP          13                /* Indicates TM_DEPO_LINE  */

typedef short ITEM_SNAM;                      /* ART_ DISCNT- DEPOSIT_GROUP */

/*--------------------------------------------------------------------------*/
/*                        TM_ARTI_BASE                                      */
/* Basic information struct wich will be in each invoive line               */
/*--------------------------------------------------------------------------*/
typedef struct {
  _TCHAR bar_cd[16];
  long   art_no;
  _TCHAR descr[34];
  short  vat_no;
  double vat_perc;
  double qty;
  double price;
  double goods_value;
  short  art_grp_no;
  short  art_grp_sub_no;
  short  dept_cd;
  long   suppl_no;

  long    arti_retention_ind    ; //v3.6.1 acm -
  long    arti_perception_ind   ; //v3.6.1 acm -
  long    arti_rule_ind         ; //v3.6.1 acm -
  double  percep_amount            ; //v3.6.1 acm -
  
  long    arti_detraccion_ind ; //v3.6.2 wjm -
} TM_ARTI_BASE;

//  remark *) VAT_PERC  WAS (!!) vat * 1000   (18.500 -> 18500)
//                      NOW                    18.500 -> 18.500

/*--------------------------------------------------------------------------*/
/*                        TM_ARTI_LINE                                      */
/* Article line                                                             */
/*--------------------------------------------------------------------------*/
typedef struct {
  TM_ARTI_BASE base;
  short        art_ind;
  long         mmail_no;
  _TCHAR       pack_type[3];
  short        items_per_pack;
  short        accumulated;
  short        display_status;
  _TCHAR       reg_no[12];
  double       msam_disc1;
  long         msam_mmail_no1;
  short        msam_maction_no1;
  double       msam_disc2;
  long         msam_mmail_no2;
  short        msam_maction_no2;
} TM_ARTI_LINE;

/*--------------------------------------------------------------------------*/
/*                        TM_DISC_LINE                                      */
/* Discount line                                                            */
/*--------------------------------------------------------------------------*/
typedef struct {
  TM_ARTI_BASE base;
} TM_DISC_LINE;

/*--------------------------------------------------------------------------*/
/*                        TM_DEPO_LINE                                      */
/* Deposit line                                                             */
/*--------------------------------------------------------------------------*/
typedef struct {
  TM_ARTI_BASE base;
  short        art_ind;
  long         mmail_no;
  _TCHAR       pack_type[3];
  short        accumulated;
  short        display_status;
  _TCHAR       reg_no[12];
} TM_DEPO_LINE;

/*--------------------------------------------------------------------------*/
/*                        TM_ITEM_GROUP                                     */
/*--------------------------------------------------------------------------*/
typedef struct {
  short        voided;
  short        doc_type;
  short        approved;
  TM_ARTI_LINE arti;
  TM_DISC_LINE disc;
  TM_DEPO_LINE depo;
} TM_ITEM_GROUP;

/*--------------------------------------------------------------------------*/
/*                               TM_SHFT_GROUP                              */
/*--------------------------------------------------------------------------*/
typedef struct {
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

  long    cupon;                /* AC2012-003 acm -*/
  double  rounded;             //v3.4.7 acm -
  long    cupon_cine;          /* v3.5 acm -          */
  long    vale_pavo;          /* v3.5 acm -          */
  long    feria_escolar;        /* v3.6.0 acm -          */
  double  percep_amount;           //v3.6.1 acm -
  long    fiesta_futbol;

  long    cupon_global[CUPON_GLOBAL_MAX];
} TM_SHFT_GROUP;

/*--------------------------------------------------------------------------*/
/*                        TM_ARTF_GROUP                                     */
/*--------------------------------------------------------------------------*/
typedef struct {
  long    art_no;               /* article number                        */
  _TCHAR  descr[34];            /* description                           */
  short   cont_sell_unit;       /* selling (makro) unit                  */
  _TCHAR  pack_type[3];         /* pack type                             */
  double  sell_pr;              /* selling price per makro unit          */
  long    mmail_no;             /* makro mail number                     */
} TM_ARTF_GROUP;                /* ANSI SIZEOF ? | UNICODE SIZEOF ?      */

#ifdef __cplusplus
}
#endif

#endif
