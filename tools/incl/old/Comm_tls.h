/*
 *     Module Name       : COMM_TLS.H
 *
 *     Type              : Include file communication from pos with the
 *                         stnetp24 program
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
 * 11-Dec-2000 added PWDN_TYPE                                         R.N.B.
 * --------------------------------------------------------------------------
 * 21-Sep-2001 Added Pending Invoice.                                    M.W.
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Changes for Article Finder.                               M.W.
 * --------------------------------------------------------------------------
 * 03-May-2002 Added CFGERR                                            J.D.M.
 * --------------------------------------------------------------------------
 * 25-Jul-2005 Added TSCO_TYPE                                           M.W.
 * --------------------------------------------------------------------------
 * 01-May-2007 Added TILL_TYPE                                         J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef _comm_tls_h_
#define _comm_tls_h_

#include "storerec.h"
#include "pos_recs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------*/
/* Definitions to be used in dest_id and signature of structure:          */
/* message_struct. Do NOT leave gaps between the numbers                  */
/* Do NOT change the original sequence of the defines                     */
/*------------------------------------------------------------------------*/
#define PS_ID                100        /* Dest/Src of msg is POS         */
#define SC_ID                101        /* Dest/Src of msg is Store Contr.*/
#define BO_ID                102        /* Dest/Src of msg is Backoffice  */
#define CR_ID                103        /* Dest/Src of msg is Cust. recept*/
#define LN_ID                104        /* Src of msg is LAN              */

/*------------------------------------------------------------------------*/
/* Definitions to be used in message_struct.type                          */
/* Do NOT leave gaps between the numbers.                                 */
/* Do NOT change the original sequence of the defines                     */
/*------------------------------------------------------------------------*/
#define VOID_TYPE            209                            /* dummy type */
#define CUST_TYPE            210
#define ARTI_TYPE            211
#define BARC_TYPE            212
#define GENV_TYPE            213
#define TAXX_TYPE            214
#define PAYM_TYPE            215
#define SYST_TYPE            216
#define RQST_TYPE            217
#define COMM_TYPE            218
#define INVO_TYPE            219
#define SHFT_TYPE            220
#define CASH_TYPE            221
#define RECO_TYPE            222                /* local types start here */
#define CURR_TYPE            223
#define CFEE_TYPE            224
#define MMML_TYPE            225
#define CART_TYPE            226
#define CONF_TYPE            227
#define PWDN_TYPE            228
#define PEND_TYPE            229
#define VOUC_TYPE            230
#define VERS_TYPE            231
#define STAT_TYPE            232
#define TSCO_TYPE            233
#define CUFI_TYPE            234
#define TILL_TYPE            235
#define ARTI_PROM_TYPE       236 /* 27-Jan-2012 acm -  */
#define CUST_PROM_TYPE       237 /* v3.4.8 acm -  */
#define VALEPAVO_PROM_TYPE   238 /* v3.5.1 acm -  */
#define VOUC_VALE_TYPE       239 //v3.5.1 acm -

#define CUST_PERC_TYPE       240 /* v3.4.8 acm -  */
#define CARDHOLDER_TYPE      241 /* v3.4.8 acm -  */
#define ANTICIPO_PROM_TYPE   242 //v3.5.1 acm -



/*------------------------------------------------------------------------*/
/* Definitions to be used in message_struct.action. Do NOT leave gaps     */
/* between the numbers. Do NOT change the original sequence of the defines*/
/*------------------------------------------------------------------------*/
#define READ_ACTION          310        /* Read rec                       */
#define UPDT_ACTION          311        /* Update rec                     */
#define INST_ACTION          312        /* Insert rec                     */
#define UPIN_ACTION          313        /* Update or Insert rec           */
#define DELT_ACTION          314        /* Delete rec                     */
#define FRST_ACTION          315        /* First rec                      */
#define NXTR_ACTION          316        /* Next rec                       */
#define OPEN_ACTION          317        /* Open ctree                     */
#define CLSE_ACTION          318        /* Close ctree                    */
#define FSET_ACTION          319        /* First in set                   */
#define NSET_ACTION          320        /* Next in set                    */
#define NXTR_NO_KEY_ACTION   321        /* Next rec without index scan    */

/*------------------------------------------------------------------------*/
/* Definitions to be used to communicate between pos and sc               */
/*------------------------------------------------------------------------*/
#define CONN                   1                /* socket connected       */
#define GONE                   2                /* socket dis-connected   */
#define EDAY                   3                /* end of day             */
#define PFAL                   4                /* powerfailure           */
#define CFGERR                 5                /* socket disconnected be-*/
                                                /* cause of an config err */

/*------------------------------------------------------------------------*/
/* CTREE Keyrelation types                                                */
/*------------------------------------------------------------------------*/
enum RelationType {keyLTE,                    /* Less than or equal to    */
                   keyEQL,                    /* Equal to                 */
                   keyGTE};                   /* Greater than or equal to */

/*------------------------------------------------------------------------*/
/* Structure SC_CTREE_INFO is a communication structure for the interface */
/* used in the pos application and the store controler.                   */
/* The structure will be encapsulated in the MESSAGE_STRUCT on position   */
/* *data.                                                                 */
/*------------------------------------------------------------------------*/
typedef struct {
   short             indexfilenum;
   enum RelationType keyrel;
   short             status;
   BYTE              data;
} SC_CTREE_INFO;

#define CORRLENGTH (sizeof(SC_CTREE_INFO)-2*sizeof(BYTE)) /* ctree info -/- data */

/*------------------------------------------------------------------------*/
/* Definitions to be used to communicate between pos and network          */
/*------------------------------------------------------------------------*/

typedef struct {
   short subtype;                         /* contains 'sub RQST type'     */
   long  number;                          /* e.q. customer, cashier, etc. */
   BYTE  extra_request_info[LEN_EXTRA_REQ_INFO];
                                          /* extra array for use when     */
                                          /* subtype and number are not   */
                                          /* sufficient.                  */
} REQUEST_STRUCT;

/* defined error numbers */
#define TIME_OUT              -2       /* Network time out or no answer   */

/* other defines */
#define DONTWAIT               0       /* max seconds to wait on stnetp24 */
#define MAXWAIT                5       /* max seconds to wait on stnetp24 */

/* export functions */
extern void  comm_tls_init(void (*)(int));
extern void  comm_idle(void);
extern short pos_first_rec (short, short, short, void *);
extern short pos_start_rec (short, short, short, void *);
extern short pos_next_rec  (short, short, short, void *);
extern short pos_next_rec_no_key(short, short, short, void *);
extern short pos_get_rec   (short, short, short, void *, short);
extern short pos_put_rec   (short, short, short, void *);
extern short pos_update_rec(short, short, short, short, void *);
extern short pos_delete_rec(short, short, short, short, void *);
extern short pos_upda_system(short, short, SYSTEM_DEF *);
extern short pos_read_system(short, short, SYSTEM_DEF *);
extern short pos_upda_genvar(short, short, GENVAR_DEF *);
extern short pos_read_genvar(short, short, GENVAR_DEF *);
extern short pos_read_customer(short, short, POS_CUST_DEF *);
extern short bo_again(short, short, void *);
extern short cr_again(short, short, void *);
extern short bo_request(short, short, REQUEST_STRUCT *, void *);
extern short cr_request(short, short, REQUEST_STRUCT *, void *);
extern short get_bo_status (void);
extern short get_cr_status (void);
extern short end_of_day_p24(void);
extern short powerfail_p24 (void);
extern short cmd2p24 (short);

#ifdef __cplusplus
}
#endif

#endif
