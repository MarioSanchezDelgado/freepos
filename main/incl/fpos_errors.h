/*
 *     Module Name       : POS_ERRS.H
 *
 *     Type              : Include file application error structures
 *                         and functions
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

#ifndef __FPOS_ERRORS_H__
#define __FPOS_ERRORS_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
#define PRN_POWER_FAILURE           -1201
#define PRN_NOT_CONFIGURED          -1202
#define PRN_JOURNAL_PAPER_OUT       -1203
#define PRN_MOTOR_JAM               -1204
#define PRN_UNDEFINED_ERROR         -1205
#define PRN_SLIP_OUT                -1206

#define PRN_FATAL_ERROR             -1250                                    

#define PRN_REMOVE_CHEQUE           -1254
#define PRN_INSERT_CHEQUE           -1255
#define PRN_INVOICE_FATAL_ERROR     -1256

#define PRN_NOT_OPEN                -1297
#define PRN_NOT_QUEUED              -1298
#define PRN_COMMAND_ABORTED         -1299
#endif

                           /* Add new errors here (count down) !! */
#define PRN_SMALL_SEQ_FINISHED         -24986
#define CUST_FISC_NO_ERROR             -24987
#define EXCEED_MAX_AMOUNT_DONATION     -24988
#define EXCEED_CHANGE_VALUE_DONATION   -24989

#define PRN_UNKNOWN_ERR                -24990
#define PRN_INIT_ERROR                 -24991
#define PRN_METHOD_ERR                 -24992
#define PRN_OFFLINE                    -24993
#define PRN_TIMEOUT                    -24994
#define PRN_BUSY                       -24995
#define PRN_COVER_OPEN                 -24996
#define PRN_NO_PAPER                   -24997
#define PRN_NOT_EMPTY                  -24998

#define PRN_REMOVE_SLIP                -24999
#define PRN_INSERT_SLIP                -25000

#define MSAM_TOO_MANY_ERR              -31909
#define MSAM_PARSING_TIME_OUT          -31910
#define SCAN_KEYB_CONFLICT             -31911
#define OPOS_SCANNER_READ_TIME_OUT     -31912

#define CFEE_RETURN                    -31913
#define DOUBLE_LINE_ON_INVOICE         -31914
#define BACKLOG_CORRUPT                -31915

#define EXCEED_MAX_AMOUNT_CASHDRAWER   -31916
#define EXCEED_MAX_START_FLOAT         -31917
#define NET_CFG_NO_SHIFT               -31918
#define NET_CONFIG_ERROR               -31919

#define UNBLOCK_VOUCHER_ERROR          -31920
#define VOUCHER_LENGTH                 -31921
#define NO_CONNECTION                  -31922
#define VOUCHER_NOT_KNOWN              -31923
#define VOUCHER_BLOCKED                -31924

#define QUERY_PARSE_ERROR              -31925
#define QUERY_TOO_MANY                 -31926
#define QUERY_ZERO_RECORDS             -31927
#define OPOS_KEYB_ERROR                -31928

#define GET_PENDING_INVOICE            -31929
#define OVERWRITE_SAVED_INVOICE        -31930
#define VALIDATE_SAVE_INVOICE          -31931
#define CUST_DELETE_PASSPORT           -31932

/********/
#define WARN_NO_CHEQUES_ALLOWED        -31933
#define VALIDATE_CHEQUE                -31934
/********/

#define MSAM_DISTR_ERR                 -31935
#define MSAM_SYNTAX_ERR                -31936
#define INVALID_GENV_CD                -31937
#define CFEE_OVERRULE_ONCE             -31938
#define CFEE_OVERRULE_ONE_YEAR         -31939

#define TO_LESS_FOREIGN                -31940
#define PAYTYPE_NOT_SAME_SELECT        -31941
#define DATA_CORRUPT                   -31942

#define BEFORE_COPY_INVOICE            -31943
#define AFTER_COPY_INVOICE             -31944
#define NO_OTHER_PAYMENTS              -31945

#define APPROVED_ITEM                  -31946

#define CASHIER2FAST                   -31947
#define TOO_MANY_SHIFTS                -31948

#define ACCEPT_CREDIT_CARD             -31949
#define NO_AMOUNT_ON_CREDIT_CARDS      -31950

#define NO_DISC_ON_R2C_BARCD           -31951
#define NO_DISCOUNT_ON_DEPOSIT         -31952

#define VALUE_TOO_LARGE                -31953
#define SHIFT_RECOVERED_NOT_CLOSED     -31954

#define CANCEL_TILL_LIFT_REFILL        -31955

#define REMOVE_INVOICE_MSG             -31956
#define NO_TRAINING                    -31957
#define ILLEGAL_SYSTEM_DATE            -31958

#define CUST_NO_FACTURA				   -31900   /*JCP*/
#define CUST_ILLEGAL_PASSPORT_NUMBER   -31959
#define CUST_PASSPORT_INVALID          -31960
#define CUST_PASSPORT_EXPIRED          -31961

#define INIT_ENVIRONMENT_ERROR         -31962
#define INP_OCIA_NOT_LEGAL             -31963

#define EXCEEDS_CHEQUE_LIMIT           -31964
#define CUST_UNKNOWN                   -31965
#define DISCNT_AMNT_TOO_LARGE          -31966
#define ILLEGAL_TIME_VALUE             -31967
#define ILLEGAL_DATE_VALUE             -31968

#define CASHIER_NOT_AVAILABLE          -31970
#define TOO_MANY_INVOICE_LINES         -31971
#define ARTICLE_BLOCKED                -31972
#define CUST_BLOCKED                   -31973
#define PRICE_TOO_LARGE                -31974
#define ILLEGAL_BARCODE                -31975

#define LIFT_AMNT_EXCEED_TILL_AMNT     -31977
#define CASHIER_ALREADY_LOGON          -31978
#define CASHIER_NOT_ALLOWED            -31979
#define UNKNOWN_CASHIERNO              -31980
#define CASH_PINCD_MISTAKE3            -31982
#define CASH_PINCD_MISTAKE2            -31983
#define CASH_PINCD_MISTAKE1            -31984
#define NO_ITEMS_TO_VOID               -31985
#define CUST_NO_CHEQUES_ALLOWED        -31986
#define AMNT_PAID_EXCEED_LIMIT         -31987
#define FIRST_ENTER_AMOUNT             -31988
#define PAYMENT_NOT_PRESENT            -31989
#define TOTAL_NOT_POSSIBLE             -31990  /* subtotal is zero */
#define NO_LAST_ITEM                   -31991
#define INVALID_ARTNO                  -31992
#define QTY_TOO_LARGE                  -31993
#define FIELD_NOT_EMPTY                -31994  /* key legal on empty field */
#define INVALID_DISCNO                 -31995
#define INVALID_MENU_OPTION            -31998
#define TEST_KEY                       -31999
#define INVALID_KEY_ERROR              -32000
#define NOT_RUC_CUSTOMER_ERROR         -32001   /*jcp*/ /*25-Set-2012 acm*/
#define ILLEGAL_FUNCTION_KEY_ERROR     -32006
#define POS_TOO_MANY_KEYS              -32023
#define ZERO_NOT_LEGAL_ERROR           -32038
#define NOT_VALID_VOUCHER_TURKEY       -32036   /*jcp*/ /*25-Set-2012 acm*/
#define NOT_VALID_TURKEY			   -32037  /*jcp*/  /*25-Set-2012 acm*/
#define NOT_VALID_TURKEY_VALE		       -32041   /* 12-Ago-2011 acm - */
#define WEIGHT_TURKEY_MUSTBE7          -32042   /* 12-Ago-2011 acm - */

#define ONE_VALE_BY_INVOICE            -32043   /* 12-Ago-2011 acm - */
#define TURKEY_NOT_APPLICABLE          -32044   /* 12-Ago-2011 acm - */
#define ARTICLE_MUSTBE_TURKEY          -32045   /* 12-Ago-2011 acm - */

#define QB_CODE_QUEUE_BUSTING_INVALID       -32046   /* 25-Set-2012 acm - */
#define QB_CONECTION_QUEUE_BUSTING_ERROR  -32047   /* 25-Set-2012 acm - */
#define QB_ARTICLE_QUEUE_BUSTING_INVALID    -32048   /* 25-Set-2012 acm - */

#define QB_FORMAT_QUEUE_BUSTING_INVALID         -32049   /* 25-Set-2012 acm - */
#define QB_ROWS_BIG_QUEUE_BUSTING_INVALID       -32050   /* 25-Set-2012 acm - */
#define QB_ROWS_QUEUE_BUSTING_INVALID           -32051   /* 25-Set-2012 acm - */
#define QB_SOCKET_CREATED_QUEUE_BUSTING_INVALID -32052   /* 25-Set-2012 acm - */
#define QB_HOSTNAME_QUEUE_BUSTING_INVALID       -32053   /* 25-Set-2012 acm - */

#define QB_QUEUE_BUSTING_NOTFOUND           -32054   /* 25-Set-2012 acm - */
#define QB_QUEUE_BUSTING_SQL_INVALID        -32055   /* 25-Set-2012 acm - */
#define FIELD_NOT_HORECA                    -32056    /* V3.4.8 acm -*/
#define ARTICLE_NOT_HORECA                  -32057    /* V3.4.8 acm -*/
#define VIGENCIA_NOT_HORECA                 -32058    /* V3.4.8 acm -*/
#define INVALID_VALEPAVO                    -32059

#define VALEPAVO_CONECTION_ERROR            -32060
#define VALEPAVO_USED                       -32061
#define VALEPAVO_NOTFOUND                   -32062
#define VALEPAVO_UNKNOWN                    -32063
#define VALEPAVO_BD_ERR                     -32064

#define DOCUMENT_DOC_ERR                    -32065
#define ILLEGAL_CLIENAME_PERCEPTION         -32066

#define ANTICIPO_CONECTION_ERROR            -32067
#define ANTICIPO_USED                       -32068
#define ANTICIPO_NOTFOUND                   -32069
#define ANTICIPO_UNKNOWN                    -32070
#define ANTICIPO_BD_ERR                     -32071

#define ANTICIPO_PERCEPCION_ERR             -32072
#define ANTICIPO_TOTAL_ERR                  -32073
#define ANTICIPO_IGV_ERR                    -32074
#define ANTICIPO_TOTAL2_ERR                 -32075
#define ANTICIPO_DOC_TYPE_FAC_ERR           -32076
#define ANTICIPO_DOC_TYPE_BOL_ERR           -32077
#define ANTICIPO_FIS_NO_ERR                 -32078

#define EPOS_CONNECT_ERROR                  -32079

/* Add new errors at the top !! */

/*
 *   First Error Structure  - Return status of the wait structure's function
 */

typedef struct error1
{
  short (*fn)(struct error1 *);                         /* FUNCTION TO USE */
  char **message1;                                      /* MESSAGE LINE 1 */
  char **message2;                                      /* MESSAGE LINE 2 */
  short (*wait)(void);                               /* ACTION AFTER ERROR MSG */
} ERROR1;

typedef struct simple_errors
{
  short (*fn)(struct simple_errors *);
}ERROR2;


/*
 *    Functions that handle Errors through the Pop-Up Error Window
 */

extern short err_and_status(ERROR1 *);
extern short err_and_SUCCEED(ERROR1 *);

extern void pos_init_errors(void);
extern void pos_reinit_errors(void);

/*
 *    Extra error message, is to be inserted in error->message1
 */
extern char * error_extra_msg;

#ifdef __cplusplus
}
#endif

#endif
