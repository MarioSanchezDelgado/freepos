/*
 *     Module Name       : WPOS_MN.H
 *
 *     Type              : POS Application System Constants
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

#ifndef _FPOS_MAIN_H_
#define _FPOS_MAIN_H_

//#include "Sll_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The includes wctree.h (ctreep.h) and wpos_mn.h interfere with eachother, so   */
/* instead of including the extra ctree includes for INOT_ERR we define it here. */
/* Thanks to the #define EXCLUSIVE ...                                           */
#ifndef ICUR_ERR
#define ICUR_ERR 100
#endif
#ifndef INOT_ERR
#define INOT_ERR 101
#endif


#define FREEPOSENV "FREEPOS"

/*
 *    WINDOW DEFINITIONS
 */

enum WINDOWS {
   STORE_LOGO_WINDOW
  ,INV_HEADER_WINDOW
  ,INV_ART_DESCR_WINDOW
  ,INV_ART_LIST_WINDOW
  ,INV_ART_INPUT_WINDOW
  ,INV_TOT_WINDOW
  ,INV_SUBT_WINDOW
  ,ERROR_WINDOW_ROW1
  ,ERROR_WINDOW_ROW2
  ,LAN_STATUS_WINDOW
  ,FISC_NO_WINDOW
  ,SELECT_PRINTER_WINDOW
  ,SELECT_CUST_WINDOW
  ,CASHIER_LOGON_WINDOW
  ,TOTAL_INPUT_WINDOW
  ,OPERATOR_WINDOW
  ,PAYMENT_1_WINDOW
  ,PAYMENT_2_WINDOW
  ,ART_FIND_HEADER_WINDOW
  ,ART_FIND_LIST_WINDOW
#ifndef NO_VIEW_POS_STATE
  ,POS_STATUS_WINDOW
#endif
  ,TOTAL_INPUT_WINDOW_PERCEPTION
  ,LAST_WINDOW_MARKER    /* always last */
};

#define NUM_WINDOWS   LAST_WINDOW_MARKER

/*
 * POS SALES/RETURN MODE
 */

#define SALES                       1
#define RETURN                      0


/*
 * CASHIER TYPE
 */

#define CASH_NORMAL                 0
#define CASH_TRAINING               1



/*
 *    BARCODE DEFINITIONS
 */

#define BARCD_ILLEGAL               100
#define BARCD_INTERNAL_EAN8         110
#define BARCD_INTERNAL_EAN13        115
#define BARCD_CUSTOMER              116
#define BARCD_SCALE_EAN13           117
#define BARCD_SCALE_PRICE           118
#define BARCD_SCALE_WEIGHT          119
#define BARCD_EXTERNAL              120
#define BARCD_EXTERNAL_EAN128       121
#define BARCD_EXTERNAL_DUMP14       122
#define BARCD_EXTERNAL_WEIGHT_EAN13 123
#define BARCD_VALE_TURKEY_EAN13     124 /* acm -*/

/*
 *    ARTICLE TYPES
 */

#define ART_IND_NORMAL              0
#define ART_IND_PRICE               1
#define ART_IND_WEIGHT              2
#define ART_IND_DEPOSIT             3
#define ART_IND_CREDIT              4


/*
 *    FEE STATUS TYPES
 */
#define FEE_STAT_NONE              0
#define FEE_STAT_PAY               1
#define FEE_STAT_CANCEL            2
#define FEE_STAT_CANCEL_YEAR       3

/*
 *    DEPARTMENT TYPES
 */

#define DEPT_NONE                   0
#define DEPT_FOOD                   1
#define DEPT_NFOOD                  2


/*
 *    PAYMENT TYPES
 */

#define PAYM_TYPE_NORMAL            _T('0')
#define PAYM_TYPE_CASH              _T('1')
#define PAYM_TYPE_CHEQUE            _T('2')
#define PAYM_TYPE_INTERNAL          _T('3')
#define PAYM_TYPE_VOUCHER           _T('4')


/*
 *    SCREEN NUMBERS
 */

#define CUST_SCRN               0x0001
#define OPER_SCRN               0x0002


/*
 *    VIEW TENDERING DEFINITIONS
 */

#define VIEW_AMNT_DUE               0
#define VIEW_AMNT_CHANGE            1


/*
 * VAT DEFINITIONS
 */

#define INCLUSIVE               YES
#define EXCLUSIVE               NO


/*
 *    IN- EXCLUSIVE VAT CALCULATIONS
 */

#define EXCLUSIVE_TO_INCLUSIVE  0
#define INCLUSIVE_TO_EXCLUSIVE  1

/*
 *   WEIGHT ARTICLES DEFINED AS PRICE OR WEIGHT
 */

#define WEIGHT                  1
#define PRICE                   0


/*
 *    NUMBER TO STRING CONVERSION DEFINITIONS
 */

#define DECIMAL                 10
#define HEXDEC                  16

/*
 * DECIMAL FORMAT DEFINITIONS
 */

#define DECIMALS_YES  1                /* Use 2 decimals in price-layout     */
#define DECIMALS_NO   0                /* Use no decimals in price-layout    */


/*
 * SHIFT WALLET NUMBER DEFINITIONS
 */

#define WALLET_NOT_USED     -1

/*
 *    SOME MACROS
 */


extern short prn_on[];                  /* See explanation in Wpos_mn.c      */
extern short print_to_file;             /* See explanation in Wpos_mn.c      */
extern short selected_printer;
extern short selected_invoice_printer;  /*JCP*/

extern char prn_fname[80];

extern short invoice_mode;              /* Current (active) invoice mode.    */
extern short invoice_line_mode;         /* Current (active) inv-line mode.   */
extern short next_invoice_mode;         /* Invoice mode for next customer.   */ /* S.T. reprint */

extern short train_mode;
extern short closed_till;
extern short status_of_pos;
extern short assign_price;              /* Used in StartFloat_ST.            */
extern short assign_minus;              /* Used in DiscAmount state.         */
extern short system_type;               /* See main() for details.           */
extern short copy_invoice;              /* If YES, print invoice twice.      */
extern short copy_invoice_active;       /* If YES, special text is being pr. */
extern short bot_copy_invoice_active;   /* If YES, special text is being pr. */
extern short voided_invoice;            /* If YES, special text is being pr. */
extern short reverse_invoice_active;    /* if YES , si la caja esta en modo  */
extern short err_init_environment;      /* init_environment err (TRUE/FALSE) */

extern long  nbr_log_lines;         /* Nr. of lines logged to BO (nor+dep) . */
extern long  nbr_inv_lines;         /* Number of items (norm/dep/disc).      */
extern long  nbr_void_lines;        /* Number of voided invoice lines.       */
extern short wallet_seq_no;         /* Wallet number sequence number/shift.  */

//extern TM_INDX last_item;               /* Last legal item in invoice        */
//extern TM_INDX corr_item;               /* Item to correct in invoice        */
//extern TM_INDX display_item;            /* Item currently displayed          */
//
//extern TM_ITEM_GROUP c_item;            /* Current item                      */
//extern TM_SHFT_GROUP c_shft;            /* Current shift                     */
//extern SHIFT_TDM_DEF c_day;             /* Current day                       */
//
extern short subt_display;              /* Display subtotal on cust-scrn     */

extern short cash_pincd_mistakes;       /* Number of times the cashier pincd */
                                        /* was incorrect.                    */
extern short send_logoff_cashier;       /* Send or do not send a logoff at   */
                                        /* end of shift.                     */
extern double cheque_amount;            /* Cheque amount to be printed       */

extern short pos_main(void);            /* Program Entry Point               */
extern void  de_init(char *);         /* Program Exit point                */

extern char last_condition[];         /* used by article finder            */
extern long   art_no_from_finder;       /* art_no returned by art finder     */

extern char  sinal[];                 /* calculator                        */

//extern SLL voucher_hist;                /* History of vouchers               */
//extern SLL voucher_anticipo_hist;                /* History of vouchers               */
//
//extern SLL payment_items;               /* Payment items                     */
//extern SLL tax_sum_codes;               /* Tax summarize codes               */
//extern SLL customers_by_fiscno;         /* Found customers by fiscal number  */

//extern TILL_DEF          till;

/*mlsd*/
//extern FILE * pLogFile;
/*mlsd*/

#ifdef __cplusplus
}
#endif

#endif
