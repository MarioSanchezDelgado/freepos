/*
 *     Module Name       : POS_COM.CPP
 *
 *     Type              : Pos ctree & communication Functions
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
 * 15-Nov-2000 Replaced localtime by GetLocalTime                      R.N.B.
 * --------------------------------------------------------------------------
 * 11-Dec-2000 Added functions for asynchornous requests for crazy     R.N.B.
 *             articles. Added pwdn function called by p12     
 * --------------------------------------------------------------------------
 * 19-Jan-2001 Added check in init_paym if paym type 1 is present.     R.N.B.
 * 06-Jun-2001 Set global error status in init_environment_records.    R.N.B.
 * --------------------------------------------------------------------------
 * 23-Jan-2002 Solved bug concerning waiting for the result of a crazy
 *             article history request. If there is no result of that
 *             request the POS waited for 10 seconds after pressing
 *             the total key, but actually the 10 seconds should be
 *             counted from the moment that the last request was
 *             done. The bug gave some serious problems when stnetp20
 *             was busy doing something else than producing a crazy
 *             article answer for the POS.                             J.D.M.
 * 31-Jan-2002 Bugfix get_cust_cr(): if customer is not found we
 *             should not wait until the time out.                     J.D.M.
 * --------------------------------------------------------------------------
 * 23-Jan-2002 Changes for Credit note. Ask for amount if offline.       M.W.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 22-Jul-2002 Multisam Phase 3.                                         P.M.
 * --------------------------------------------------------------------------
 * 04-Oct-2002 Bugfix get_cust_cr(): while loop was wrong.             J.D.M.
 * --------------------------------------------------------------------------
 * 14-Oct-2004 Put multisam actions in memory for faster performance
 *             and changed to cpp.                                     J.D.M.
 * --------------------------------------------------------------------------
 * 28-Jul-2005 Summarize tax codes on total screen.                      M.W.
 * --------------------------------------------------------------------------
 * 02-Aug-2005 Search customer by Fiscal number.                         M.W.
 * --------------------------------------------------------------------------
 * 16-Aug-2005 Select customer if searched by fisc_no                    M.W.
 * --------------------------------------------------------------------------
 * 01-May-2007 Added init_till_table() and update_till_callback()      J.D.M.
 * --------------------------------------------------------------------------
 */

#pragma warning(push) /* Push current warning level to stack. */
#pragma warning(disable: 4786) /* Disable warning 4786.       */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include <map>
#include <deque>
using namespace std;

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>
#include <time.h>
#include <process.h>

#include "appbase.h"
#include "stnetp24.h"                       /* Pos (library) include files   */
#include "comm_tls.h"
#include "misc_tls.h"
#include "stri_tls.h"
#include "storerec.h"

                                            /* Toolsset include files.       */
#include "scrn_mgr.h"
#include "tm_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "llist.h"
#include "sll_mgr.h"
#include "err_mgr.h"

#include "intrface.h"                       /* Application include files.    */
#include "pos_inp.h"
#include "pos_recs.h"
#include "pos_tm.h"
#include "pos_com.h"
#include "pos_func.h"
#include "st_main.h"
#include "WPos_mn.h"
#include "pos_msam.h"
#include "pos_tot.h"
#include "pos_errs.h"
#include "pos_txt.h"

/* contains the vat information. MAX_VAT elements */
static  struct
{
  double vat_perc;
  double summarize_cd;
} tax_table[MAX_VAT];
                                /* vars used for asynchronous CART requests */
static short last_rqst_store_no;
static long  last_rqst_cust_no;
static short accept_cart_data;
static time_t start_of_most_recent_cart_request; 
static deque<MMML_DEF>  multisam_updates;
multimap<MMML_KEY, MMML_DEF>  multisam_definitions;

static short check_cust_no(short, _TCHAR *, _TCHAR *);
static short init_multisam_definitions(void);

/*--------------------------------------------------------------------------*/
/*                            INIT_CUST_REC                                 */
/*--------------------------------------------------------------------------*/
void init_cust_rec(POS_CUST_DEF *cust)                 /* a customer record */
{
   CRAZY_ART_DEF cart_dummy;

   if (sll_read(&crazy_art_hist, 0, &cart_dummy) == SUCCEED) {
     sll_remove(&crazy_art_hist);
   }

   memset(cust, 0, sizeof(POS_CUST_DEF));
   cust->cust_no = -1;
   cust->ind_cheques = TRUE;
   cust->exp_date = pos_system.run_date;
   cust->appr_cheque_amount = 0.0;
   return;
}  /* init_cust_rec */


/*--------------------------------------------------------------------------*/
/*                            INIT_CUST_FISC_REC                            */
/*--------------------------------------------------------------------------*/
void init_cust_fisc_rec(POS_CUST_FISC_DEF *custfisc)
{
   CRAZY_ART_DEF cart_dummy;
   short         i;

   if (sll_read(&crazy_art_hist, 0, &cart_dummy) == SUCCEED) {
     sll_remove(&crazy_art_hist);
   }

   memset(custfisc, 0, sizeof(POS_CUST_FISC_DEF));
   for (i=0; i<MAX_NBR_OF_CUST; i++) {
     custfisc->cust[i].cust_no = -1;
     custfisc->cust[i].ind_cheques = TRUE;
     custfisc->cust[i].exp_date = pos_system.run_date;
     custfisc->cust[i].appr_cheque_amount = 0.0;
   }
   return;
}  /* init_cust_fisc_rec */


/*--------------------------------------------------------------------------*/
/*                             GET_CUST_DATA                                */
/*--------------------------------------------------------------------------*/
short get_cust_data(_TCHAR *cust_no_alpha, POS_PEND_CUST_DEF *pend_cust_rec)
{
   short             status = SUCCEED;
   _TCHAR            buffer[20];
   _TCHAR            buffer1[20];
   _TCHAR            *p;
   long              cust_no;
   long              cust_key;
   _TCHAR            cust_check_dig[20];
   short             store_no;
   short             len, found;
   POS_CUST_DEF      *custrec = &pend_cust_rec->cust;
   POS_CUST_FISC_DEF custfiscrec;

   memset(pend_cust_rec, 0, POS_PEND_CUST_SIZE);
   memset(cust_check_dig, 0, sizeof(cust_check_dig));

   if (get_use_fisc_no()==FALSE) {

     _tcscpy(buffer, cust_no_alpha);
     p=buffer;

     if (*p==_T('2') && _tcslen(p)==13) {
       /* Customer number is scanned, contents of data:                       */
       /*                                                                     */
       /* 2XSSCCCCCCDDZ with:                                                 */
       /*   SS          : store no                                            */
       /*     CCCCCC    : customer no                                         */
       /*  X        DD  : check digit, if X is '1' the first digit is '.'     */
       /*               : else X is '0'                                       */
       /*             Z : check digit                                         */
       if (*(p+1)==_T('1'))  {            /* Handle the dot.                  */
         *(p+10)=_T('.');
       }
       *(p+12)=_T('\0');                  /* Remove the check-digit.          */
       p+=2;                              /* Resulting in p -> SSCCCCCCDD     */
     } 

     /*                                                                       */
     /* At this point a legal customer number looks like:                     */
     /*                                                                       */
     /*   [SS][C]CCCCCDD -> SSCCCCCCDD or                                     */
     /*                       CCCCCCDD or                                     */
     /*                        CCCCCDD or                                     */
     /*                      SSCCCCCDD                                        */
     /*                                                                       */

     len=_tcslen(p);
     if (len>(POS_STNO_SIZE+POS_CSTN_SIZE+POS_CHDG_SIZE)) {
       return(CUST_NUM_WRONG);             /* Illegal barcode or whatever..   */
     }

     store_no = pos_system.store_no;
     if (len>POS_CSTN_SIZE+POS_CHDG_SIZE) {            /* There is a STORE_NO */
       _tcscpy(buffer1, p);
       buffer1[POS_STNO_SIZE] = _T('\0');
       store_no = (short)_ttoi(buffer1);
       p+=POS_STNO_SIZE;
     } 

     /*                                                                       */
     /* At this point a legal customer number looks like:                     */
     /*                                                                       */
     /*   [C]CCCCCDD -> CCCCCCDD or                                           */
     /*                  CCCCCDD                                              */
     /*                                                                       */

     len=_tcslen(p);
     _tcscpy(cust_check_dig, p+(len-POS_CHDG_SIZE));          /* get DD       */

     *(p+len-POS_CHDG_SIZE)=_T('\0');                         /* get [C]CCCCC */
     cust_no = _ttol(p);
     cust_key = (store_no * 1000000) + cust_no;

     if (cust_no==0 || store_no==0) {
       return (CUST_NUM_WRONG);
     }

     if (check_cust_no(store_no, p, cust_check_dig)==FAIL) {
       return (CUST_NUM_WRONG);
     }

     init_cust_rec(&pend_cust_rec->cust);
     pend_cust_rec->cust.cust_key = cust_key;
     if (check_pending_invoice(pend_cust_rec)==TRUE) {
       return(PEND_INV_FOUND);
     }
     init_cust_rec(custrec);
   }
   else {
     init_cust_fisc_rec(&custfiscrec);
   }

                   /* retrieve customer data from CNS or posctree depending */
                   /* on the genvar variable cust_on_pos                    */
   switch (genvar.cust_on_pos) {
     case NONE:                                     /* all customers on CNS */
       if (get_use_fisc_no()==FALSE) {
         status = get_cust_cr(cust_key, custrec);
       }
       else {
         _tcscpy(custfiscrec.fisc_no, cust_no_alpha);
         status = get_cust_fisc_cr(CUST_BY_FISC_NO, &custfiscrec);
         for (found=0; found < custfiscrec.nbr_of_cust; found++) {
           sll_add(&customers_by_fiscno, &custfiscrec.cust[found]);
         }
       }
       break;
     case LOCAL_ONLY:
       if (get_use_fisc_no()==TRUE) {           /* search by fisc_no on CNS */
         _tcscpy(custfiscrec.fisc_no, cust_no_alpha);
         status = get_cust_fisc_cr(CUST_BY_FISC_NO, &custfiscrec);
         for (found=0; found < custfiscrec.nbr_of_cust; found++) {
           sll_add(&customers_by_fiscno, &custfiscrec.cust[found]);
         }
       }
       else {
         if (store_no == pos_system.store_no) {   /* local customers on pos */
           custrec->cust_key = cust_key;
           status = pos_read_customer(POS_CUST_SIZE, CUSTOMER_IDX, custrec);
         }
         else {                               /* non-local customers on CNS */
           status = get_cust_cr(cust_key, custrec);
         }
       }
       break;
     case ALL:                                 /* all customers on posctree */
       if (get_use_fisc_no()==FALSE) {
         custrec->cust_key = cust_key;
         status = pos_read_customer(POS_CUST_SIZE, CUSTOMER_IDX, custrec);
       }
       else {
         _tcscpy(custrec->fisc_no, cust_no_alpha);
         status = pos_get_rec(CUST_TYPE, POS_CUST_SIZE, CUST_FISCNO_IDX, custrec, (short) keyGTE);
         found=0;
         while (status == SUCCEED && (_tcscmp(custrec->fisc_no, cust_no_alpha)==0) && found<MAX_NBR_OF_CUST) {
           sll_add(&customers_by_fiscno, custrec);
           found++;
           status = pos_next_rec_no_key(CUST_TYPE, POS_CUST_SIZE, CUST_FISCNO_IDX, custrec);
         }
       }
       break;
     default:
       status = INVALID_CUST_ON_POS;
       break;
   }

   if (get_use_fisc_no()==TRUE) {
     switch (found) {
       case 0: /* Not found, or record of other customer                */
         status=FISC_NO_NOT_FOUND;
         break;
       case 1: /* Found, one record to far, so read correct record again    */
         sll_read(&customers_by_fiscno, 0, custrec);
         status=SUCCEED;
       break;
       default:
         if (found>1) {
           return(FISC_NO_MANY_FOUND);
         }
       break;
     }
   }

   if (status==SUCCEED) {

     if (get_use_fisc_no()==FALSE) {
        /* check digit different, there is a newer customer sequence number */
       if (_tcscmp(custrec->cust_check_dig,cust_check_dig)!=0) {
         status = CUST_CHKD_WRONG;
       }
     }
     else {
       if (check_pending_invoice(pend_cust_rec)==TRUE) {
         return(PEND_INV_FOUND);
       }   
     }
   }
                              /* error getting customer; initialize custrec */
   if ( status != SUCCEED ) { /* because supervisor may override with S-key */
     init_cust_rec(custrec);
     if (get_use_fisc_no()==FALSE) {
       custrec->store_no = store_no;
       custrec->cust_no = cust_no;
       _tcscpy(custrec->cust_check_dig,cust_check_dig);
     }
     else {
       status = FISC_NO_NOT_FOUND;
     }
   }

   return(status);
} /* get_cust_data */

/*--------------------------------------------------------------------------*/
/*                             GET_CUST_CR                                  */
/*--------------------------------------------------------------------------*/
short get_cust_cr(long cust_key, POS_CUST_DEF *custrec)
{

   short           status  = SUCCEED;
   REQUEST_STRUCT  request = {rqtcust,          /* sub request_type      */
                              cust_key};

   status = cr_request(CUST_TYPE,(MAXWAIT * 4),&request,(void*)custrec);

   while(((status == SUCCEED && cust_key != custrec->cust_key) || /* Not the requested customer */
          (status == FAIL && cust_key != -custrec->cust_key)) &&  /* BO still did not return 'customer not found' */
           status != TIME_OUT /* There is still no time out */) {
     status=cr_again(CUST_TYPE, (MAXWAIT * 3), (void*)custrec);
   }

   return(status);
} /* get_cust_cr */

/*--------------------------------------------------------------------------*/
/*                             GET_CUST_FICS_CR                             */
/*--------------------------------------------------------------------------*/
short get_cust_fisc_cr(long cust_key, POS_CUST_FISC_DEF *custfiscrec)
{
   _TCHAR          fisc_no[16];
   short           status  = SUCCEED;
   REQUEST_STRUCT  request = {rqtcufi,          /* sub request_type         */
                              cust_key};

   _tcscpy(fisc_no, custfiscrec->fisc_no);
   memcpy(&(request.extra_request_info), &(custfiscrec->fisc_no), RFNO_SIZE);
   status = cr_request(CUFI_TYPE,(MAXWAIT * 4),&request,(void*)custfiscrec);

   while((_tcscmp(fisc_no, custfiscrec->fisc_no)!=0) &&
           status != TIME_OUT) {              /* There is still no time out */
     status=cr_again(CUFI_TYPE, (MAXWAIT * 3), (void*)custfiscrec);
   }

   return(status);
} /* get_cust_fisc_cr */

/*--------------------------------------------------------------------*/
/*                            check_cust_no                           */
/*--------------------------------------------------------------------*/
static short check_cust_no(short store_no, _TCHAR *s_cust_no, _TCHAR *s_check_dig)
{
  /* Checks the customer number. If the check digit is wrong or a     */
  /* '.' is found on the wrong place FAIL is returned else SUCCEED.   */
  /* The customer number has the following layout: SS CCCCCC DQ with: */
  /* SS    : 2 digits store number                                    */
  /* CCCCCC: 6 digits customer number                                 */
  /* D     : 1 digit check digit                                      */
  /* Q     : 1 digit sequence number.                                 */
  /* The check digit is calculated in the following way (C3 is the    */
  /* third digit of CCCCCC):                                          */
  /* D = (10 * S1 + 9 * S2 + 8 * C1 + 7 * C2 + 6 * C3 + 5 * C4 +      */
  /*      4 * C5 + 3 * C6 + 2 * Q                                     */
  /*     ) % 11        -- Modulo 11.                                  */
  /* If the remainder is 10 then the check digit must be a '.'.       */
  short  i;
  long   cust_no;
  _TCHAR *s;

  for (s = s_cust_no; *s; s++) {
    if (*s == _T('.')) {
      return FAIL;
    }
  }

  cust_no = _ttol(s_cust_no);
  if (s_check_dig[1] == _T('.')) {
    return FAIL;
  }

  i = 10 * (store_no / 10) + 9 * (store_no % 10);
  i += 8 * (short)(cust_no / 100000L);
  cust_no %= 100000L;
  i += 7 * (short)(cust_no / 10000L);
  cust_no %= 10000L;
  i += 6 * (short)(cust_no / 1000L);
  cust_no %= 1000L;
  i += 5 * (short)(cust_no / 100L);
  cust_no %= 100L;
  i += 4 * (short)(cust_no / 10L) + 3 * (short)(cust_no % 10L) +
       2 * ((s_check_dig[1] - _T('0')) % 10);
  i %= 11;

  return (((i == 10 && s_check_dig[0] == _T('.')) ||
           (i < 10 && s_check_dig[0] == (_TCHAR)(i + _T('0'))) )? SUCCEED: FAIL);
} /* check_cust_no */


/*-------------------------------------------------------------------------*/
/*                         get_cash_bo                                     */
/*-------------------------------------------------------------------------*/
short get_cash_bo(short c_no, CASH_DEF *cashrec)
{
  short          status;
  REQUEST_STRUCT request;

  cashrec->empl_no = c_no;
  _tcscpy(cashrec->empl_name,_T(""));
  cashrec->reg_status = CASH_NOT_ALLOWED;   /* 0 not autorised             */
                                            /* 1 autorised                 */
                                            /* 2 already working on a till */
  cashrec->cur_date = pos_system.run_date;
  request.subtype = rqtcshn;                           /* sub request_type */
  request.number  = (long)c_no;
  status = bo_request(CASH_TYPE, 3*MAXWAIT, &request, (void*)cashrec);
  while (status == SUCCEED) {                /* So there is a CASH_TYPE    */
    if (cashrec->empl_no == c_no) {          /* and it's the requested one */
      status = SUCCEED;
      break;
    }
    status = bo_again(CASH_TYPE, 3*MAXWAIT, (void*)cashrec);
  }
  return(status);
} /* get_cash_bo */

/*-------------------------------------------------------------------------*/
/*                         get_cart_bo                                     */
/* - intiates an asynchronous crazy article request                        */
/*-------------------------------------------------------------------------*/
short get_cart_bo(short rqst_store_no, long rqst_cust_no,
                  CRAZY_ART_DEF *cartrec, long mmail_no, short maction_no)
{
  short            status;
  REQUEST_STRUCT   request;
  struct REQ_CRAZY req_crazy;
                                         /* save for the callback function */
  accept_cart_data = YES;
  last_rqst_store_no = rqst_store_no;
  last_rqst_cust_no = rqst_cust_no;

  request.subtype = rqtcart;                           /* sub request_type */
  request.number  = (rqst_store_no * 1000000) + rqst_cust_no;
  req_crazy.mmail_no   =       mmail_no;
  req_crazy.maction_no =       maction_no;
  req_crazy.store_no   = (long)rqst_store_no;
  req_crazy.cust_no    =       rqst_cust_no;
  memcpy(&(request.extra_request_info), &(req_crazy.mmail_no), RCRA_SIZE);

  time(&start_of_most_recent_cart_request);
  status = bo_request(CART_TYPE, DONTWAIT, &request, (void*)cartrec);

  return(status==TIME_OUT ? SUCCEED : status);     /* TIME_OUT is expected */
} /* get_cart_bo */

/*-------------------------------------------------------------------------*/
/*                         get_voucher_bo                                  */
/*-------------------------------------------------------------------------*/
short get_voucher_bo(long req_seq_no, long prefix, long voucher_status,
                     short store_no, long cust_no, VOUCHER_DEF *vouchrec)
{
  short          status;
  REQUEST_STRUCT request;
  struct REQ_VOUCHER req_voucher;

  request.subtype = rqtvouc;                           /* sub request_type    */
  request.number  = (long)req_seq_no;
  req_voucher.seq_no = req_seq_no;
  req_voucher.prefix = prefix;
  req_voucher.store_no = store_no;
  req_voucher.cust_no  = cust_no;
  req_voucher.status = voucher_status;
  memcpy(&(request.extra_request_info), &(req_voucher.seq_no), RVOU_SIZE);
  /* With hexidecimal codes you can check the contents of extra_request_info */
  status = bo_request(VOUC_TYPE, 3*MAXWAIT, &request, (void*)vouchrec);
  return(status);
} /* get_voucher_bo */

/*-------------------------------------------------------------------------*/
/*                      get_cart_callback                                  */
/*-------------------------------------------------------------------------*/
short get_cart_callback(void *data)
{
  CRAZY_ART_DEF *cartrec = (CRAZY_ART_DEF*)data;
  short num_elem = SIZE(CRAZY_ART_DEF, mmail_no)/sizeof(long);
  short status = SUCCEED,
        i;

  if (cartrec->cust_no!=last_rqst_cust_no || accept_cart_data==NO) {
    return -1;                                   /* This one is not for me */
  }                                              /* or it came too late    */

  if (cartrec->rec_num > 0) {
    sll_add(&crazy_art_hist, cartrec);
  }

  if (cartrec->ind_last==1) { /* Are there any more crazy article records? */
                            /* Search for the last filled element in array */
    for (i=num_elem-1; i>=0 && cartrec->mmail_no[i]==0; i--) {
      ;
    }
                                            /* send a new request for more */
    status = get_cart_bo(last_rqst_store_no, last_rqst_cust_no, NULL,
                         cartrec->mmail_no[i], cartrec->maction_no[i]);
  }
  else {
    accept_cart_data = NO;                    /* don't accept data anymore */
  }

  return status;
} /* get_cart_callback */

/*-------------------------------------------------------------------------*/
/*                      wait_for_cart                                      */
/*-------------------------------------------------------------------------*/
short wait_for_cart(void)
{
  short  status = SUCCEED;
  time_t now;
  short  SaveWindow;

  SaveWindow = scrn_get_current_window();
  scrn_clear_window(ERROR_WINDOW_ROW1);
  if(accept_cart_data == YES) {
    scrn_string_out(prompt_TXT[11], 0, 0);
  }
  scrn_select_window(SaveWindow);
  while (accept_cart_data == YES) {
    time(&now);
    if ((short)(now-start_of_most_recent_cart_request) > 2*MAXWAIT
        || get_bo_status()==GONE) {
      status = TIME_OUT;
      break;
    }
    comm_idle();
  }

  accept_cart_data = NO;

  SaveWindow = scrn_get_current_window();
  scrn_clear_window(ERROR_WINDOW_ROW1);
  scrn_select_window(SaveWindow);

  return status;
} /* wait_for_cart */


/*-------------------------------------------------------------------------*/
/*                         logoff_cash_bo                                  */
/*-------------------------------------------------------------------------*/
short logoff_cash_bo(short c_no)
{
  short            status = FAIL;
  REQUEST_STRUCT   request;

  request.subtype = rqtcshf;
  request.number  = (long)c_no;
  status = bo_request(CASH_TYPE, 0, &request, NULL);      /* nothing back */
  return(status);
} /* logoff_cash_bo */


/*--------------------------------------------------------------------------*/
/*                       init_tax_table                                     */
/*--------------------------------------------------------------------------*/
short init_tax_table(void)
{
  short   status;
  short   i;
  TAX_DEF taxrec;

  for(i=0; i<MAX_VAT; i++) {
    tax_table[i].vat_perc = 0.0;
  }
  status = pos_first_rec(TAXX_TYPE, POS_TAXX_SIZE, TAX_IDX, (void*)&taxrec);
  while (status == SUCCEED ) {
    if (taxrec.start_date <= pos_system.run_date) {
      tax_table[taxrec.vat_no].vat_perc     = taxrec.vat_perc;
      tax_table[taxrec.vat_no].summarize_cd = taxrec.summarize_cd;
    }
    status = pos_next_rec(TAXX_TYPE, POS_TAXX_SIZE, TAX_IDX, (void*)&taxrec);
  }                   /* 100 ICUR_ERR  101 INOT_ERR */
  if (( (( status == 100 ) || (status == 101)) && ( i > 0 ) )) {
    return(SUCCEED);
  }
  else {
    return(status);
  }
} /* init_tax_table */

/*--------------------------------------------------------------------------*/
/*                        get_vat_perc                                      */
/*--------------------------------------------------------------------------*/
double get_vat_perc(short vat_no)
{
  if ((vat_no >= 0) && (vat_no < MAX_VAT)) {
    return tax_table[vat_no].vat_perc;
  }
  return (double)0.0;
} /* get_vat_perc */


/*--------------------------------------------------------------------------*/
/*                         get_vat_sum_cd                                   */
/*--------------------------------------------------------------------------*/
short get_vat_sum_cd(short vat_no)
{
  if ((vat_no >= 0) && (vat_no < MAX_VAT)) {
    return tax_table[vat_no].summarize_cd;
  }
  return 0;
} /* get_vat_sum_cd */

/*--------------------------------------------------------------------------*/
/*                        init_vat_sum_cd_table                             */
/*--------------------------------------------------------------------------*/
static short init_vat_sum_cd_table(void)
{
  short            status;
  TAX_SUM_CD_DEF   tsco_record;

  sll_init(&tax_sum_codes, sizeof(TAX_SUM_CD_DEF));
  status = pos_first_rec(TSCO_TYPE, (short)POS_TSCO_SIZE, (short)TAX_SUM_CD_IDX, (void*)&tsco_record);

  while (status == SUCCEED) {
    sll_add(&tax_sum_codes, &tsco_record);
    status = pos_next_rec(TSCO_TYPE, (short)POS_TSCO_SIZE, (short)TAX_SUM_CD_IDX, (void*)&tsco_record);
  }

  if (( status == ICUR_ERR ) || (status == INOT_ERR)) {
    status = SUCCEED;
  }

  return status;
} /* init_vat_sum_cd_table */

/*--------------------------------------------------------------------------*/
/*                           init_till_table                                */
/*--------------------------------------------------------------------------*/
static short init_till_table(void)
{
  short    status;

  memset(&till, 0, sizeof(till));
  till.till_no = pos_system.till_no;
  status = pos_get_rec(TILL_TYPE, POS_TILL_SIZE, TILL_IDX,
                           (void*)&till, (short) keyEQL);

  return status;
} /* init_till_table */

static PAYMENT_DEF payment[MAX_PAYM_WAYS];
/*--------------------------------------------------------------------------*/
/*                            get_paym                                      */
/*--------------------------------------------------------------------------*/
short get_paym(PAYMENT_DEF *request)
{
  if ((request->paym_cd >= PAYM_WAY_0) && (request->paym_cd < MAX_PAYM_WAYS)) {
    memcpy(request,&payment[request->paym_cd],POS_PAYM_SIZE);
    if (payment[request->paym_cd].paym_limit != 0.0) {
      return(SUCCEED);
    }
    else {
      return(FAIL);
    }
 }

  return(FAIL);
} /* get_paym */


/*--------------------------------------------------------------------------*/
/*                            put_paym                                      */
/*--------------------------------------------------------------------------*/
short put_paym(PAYMENT_DEF *request)
{
  short status;

  /*                                                                       */
  /* Update payment record on disk and in static array (for get_paym())    */
  /*                                                                       */
  status=pos_update_rec(PAYM_TYPE, POS_PAYM_SIZE, PAYMENT_IDX, PAYMENT_FNO,(void*)request);
  if( status==SUCCEED ) {
    if ((request->paym_cd >= PAYM_WAY_0) && (request->paym_cd < MAX_PAYM_WAYS)) {
      memcpy(&payment[request->paym_cd],request,POS_PAYM_SIZE);
    }
  }

  return(status);
} /* put_paym */

/*--------------------------------------------------------------------------*/
/*                           init_paym                                      */
/*--------------------------------------------------------------------------*/
short init_paym(void)
{
  short   status, i;
  PAYMENT_DEF payrec;
  CURR_DEF curr;
  short   b_type_1_fnd = FALSE;

  for(i=PAYM_WAY_0; i<MAX_PAYM_WAYS; i++) {
    payment[i].paym_cd = i;
    _tcscpy(payment[i].paym_descr, _T(""));
    payment[i].paym_limit = 0.0;
    payment[i].extra_perc = 0.0;
    payment[i].min_extra_amount = 0.0;
    payment[i].vat_no = 0;
    _tcscpy(payment[i].paym_type, _T("0"));
    payment[i].currency_no = 0;
    _tcscpy(payment[i].currency_cd, _T("0"));
    payment[i].curr_standard = 1;
    payment[i].curr_rate = 1.0;
  }

  i = 0;
  status = pos_first_rec(PAYM_TYPE, POS_PAYM_SIZE, PAYMENT_IDX, (void*)&payrec);
  while (status == SUCCEED ) {
    payment[payrec.paym_cd].paym_cd = payrec.paym_cd;
    _tcscpy(payment[payrec.paym_cd].paym_descr,payrec.paym_descr);
    payment[payrec.paym_cd].paym_limit = payrec.paym_limit;
    payment[payrec.paym_cd].extra_perc = payrec.extra_perc;
    payment[payrec.paym_cd].min_extra_amount = payrec.min_extra_amount;
    payment[payrec.paym_cd].vat_no = payrec.vat_no;
    if (isdigit(*payrec.paym_type)) {
      _tcscpy(payment[payrec.paym_cd].paym_type, payrec.paym_type);
      if (_tcscmp(payrec.paym_type, _T("1")) == 0) {
        if (b_type_1_fnd == TRUE) {
          return !SUCCEED;           /* there can be only one payment type 1 */
        }
        b_type_1_fnd = TRUE;
      }
    }

    if (get_stand_rate(payrec.currency_no, &curr) == SUCCEED ) {
      payment[payrec.paym_cd].currency_no = curr.currency_no;
      _tcscpy(payment[payrec.paym_cd].currency_cd, curr.currency_cd);
      payment[payrec.paym_cd].curr_standard = curr.standard;
      payment[payrec.paym_cd].curr_rate = curr.rate;
    }
    i++;
    status = pos_next_rec(PAYM_TYPE, POS_PAYM_SIZE, PAYMENT_IDX, (void*)&payrec);
  }
                                          /* 100 ICUR_ERR  101 INOT_ERR */
  if ((status == 100 || status == 101) && i>0) {
    status = SUCCEED;
  }

  if (b_type_1_fnd == FALSE) {
    status = !SUCCEED;    /* payment type 1 not found ! (used for cash change) */
  }

  return(status);
} /* init_paym */

/*--------------------------------------------------------------------------*/
/*                         get_paym_cd_cash                                 */
/*--------------------------------------------------------------------------*/
short get_paym_cd_cash(void)
{
  short i;

  for(i=PAYM_WAY_0; i<MAX_PAYM_WAYS; i++) {
    if (_tcscmp(payment[i].paym_type, _T("1")) == 0) {
      return(i);
    }
  }

  return(1);                   /* Never reached because init_paym will fail */
} /* get_paym */

/*--------------------------------------------------------------------------*/
/*                       get_stand_rate                                     */
/*--------------------------------------------------------------------------*/
/* This function was modified to solve an Y2k bug.                          */
/* Ctree reads its records sorted. This means that currency records are     */
/* read as following: - 000101                                              */
/*                    - 000229                                              */
/*                    - 970101                                              */
/*                    - 991231                                              */
/* If a 99xxxx record is OK compared to the pos_system.run_date it          */
/* will be preferred over a 00xxxx record which is from a later date!       */
/* The code is changed in a way that if a record is found and it's from a   */
/* later date than 20000101 the 19th century dates are skipped.             */
/*--------------------------------------------------------------------------*/
short get_stand_rate(short currency_no, CURR_DEF *request)
{
  CURR_DEF currec;
  short    status, found, century;
  long     last_start_date; /* Added to solve Y2k bug */

  currec.curr_key    = (long)(currency_no*1000000);
  currec.currency_no = 0;
  _tcscpy(currec.currency_cd, _T(""));
  currec.standard    = 0.0;
  currec.rate        = 0.0;

  last_start_date = 0; /* Added to solve Y2k bug */
  found = FAIL;
  status = pos_get_rec(CURR_TYPE, POS_CURR_SIZE, CURR_RATE_IDX, (void*)&currec, (short) keyGTE);
  while (status == SUCCEED && currency_no == currec.currency_no) {
    if (currec.start_date >= 800101) {               /* between 1980 - 2000 */
      if(found == SUCCEED && last_start_date >= 20000101) { /* Added to solve Y2k bug */
        return(found);                                      /* Added to solve Y2k bug */
      }
      century = 19;
    }
    else {
      century = 20;                                  /* year 2000 or later. */
    }
    if ((currec.start_date+century*1000000) <= pos_system.run_date) {
      request->currency_no = currec.currency_no;
      _tcscpy(request->currency_cd, currec.currency_cd);
      request->standard    = currec.standard;
      request->rate        = currec.rate;
      last_start_date      = currec.start_date+century*1000000; /* Added to solve Y2k bug */
      if ( (request->standard > 0.0) && (request->rate > 0.0) ) {
        found = SUCCEED;
      }
      else  {
        found = FAIL;
      }
    }
    status = pos_next_rec(CURR_TYPE, POS_CURR_SIZE, CURR_RATE_IDX, (void*)&currec);
  }

  return(found);
} /* get_stand_rate */

/*--------------------------------------------------------------------------*/
/*                       get_first_stand_rate                               */
/*--------------------------------------------------------------------------*/
short get_first_stand_rate(CURR_DEF *request)
{
  short status;

  request->curr_key    = 0;
  request->currency_no = 0;
  _tcscpy(request->currency_cd, _T(""));
  request->standard    = 0.0;
  request->rate        = 0.0;

  status = pos_first_rec(CURR_TYPE, POS_CURR_SIZE, CURR_RATE_IDX, (void*)request);

  if ( (request->standard > 0.0) && (request->rate > 0.0) ) {
    return (SUCCEED);
  }
  else {
    return (FAIL);
  }

} /* get_first_stand_rate */

/*--------------------------------------------------------------------------*/
/*                      init_multisam_definitions                           */
/*--------------------------------------------------------------------------*/
static short init_multisam_definitions(void)
{
  short    status;
  MMML_DEF mmml_record;
  MMML_KEY mmml_key;

  multisam_updates.clear();
  multisam_definitions.clear();

  status = pos_first_rec(MMML_TYPE, (short)POS_MMML_SIZE,
                         (short)MML_PRIORITY_IDX, (void*)&mmml_record);
  while (status == SUCCEED) {
    if (mmml_record.start_date <= pos_system.run_date &&
        mmml_record.end_date >= pos_system.run_date) {
      mmml_key.priority = mmml_record.priority;
      mmml_key.mmail_no = mmml_record.mmail_no;
      mmml_key.maction_no = mmml_record.maction_no;
      mmml_key.line_no = mmml_record.line_no;
      multisam_definitions.insert(make_pair(mmml_key, mmml_record));
    }
    status = pos_next_rec(MMML_TYPE, (short)POS_MMML_SIZE,
                          (short)MML_PRIORITY_IDX, (void*)&mmml_record);
  }

  if (( status == ICUR_ERR ) || (status == INOT_ERR)) {
    status = SUCCEED;
  }
  update_multisam_definitions();

  return status;
} /* init_multisam_definitions */

/*-------------------------------------------------------------------------*/
/*                 update_multisam_definitions_callback                    */
/*-------------------------------------------------------------------------*/
short update_multisam_definitions_callback(void *mmml_info)
{
  MMML_DEF sc_mmml;

  memcpy(&sc_mmml, mmml_info, sizeof(MMML_DEF));
  multisam_updates.push_back(sc_mmml);

  return 0;
} /* update_multisam_definitions_callback */

/*-------------------------------------------------------------------------*/
/*                     update_multisam_definitions                         */
/*-------------------------------------------------------------------------*/
void update_multisam_definitions(void)
{
  MMML_DEF mmml_info;
  MMML_DEF sc_mmml;
  MMML_KEY mk;
  multimap<MMML_KEY, MMML_DEF>::iterator i;

  while(multisam_updates.size()) {
    mmml_info = multisam_updates.back();
    memcpy(&sc_mmml, &mmml_info, sizeof(MMML_DEF));
    multisam_updates.pop_back();
    if(sc_mmml.line_no != -1) { /* Insert or update definition */
      mk.priority = sc_mmml.priority;
      mk.mmail_no = sc_mmml.mmail_no;
      mk.maction_no = sc_mmml.maction_no;
      mk.line_no = sc_mmml.line_no;

      /* Check if the action line already exists */
      i=multisam_definitions.find(mk);
      if(i!=multisam_definitions.end()) { /* Delete it so we can insert the new one later */
        multisam_definitions.erase(i);
      }

      /* Delete older versions first with same mmail_no and maction_no */
      for(i=multisam_definitions.begin(); i!=multisam_definitions.end();) {
        if(i->first.mmail_no == sc_mmml.mmail_no &&
           i->first.maction_no == sc_mmml.maction_no &&
           i->second.version_no != sc_mmml.version_no) {
          i=multisam_definitions.erase(i);
        }
        else {
          ++i;
        }
      }

      /* Finally insert the received record into the multisam definitions */
      multisam_definitions.insert(make_pair(mk, sc_mmml));
    }
    else { /* Delete definition */
      /* We have to walk all the actions, because the key is not correct with line_no and priority */
      for(i=multisam_definitions.begin(); i!=multisam_definitions.end();) {
        if(i->first.mmail_no == sc_mmml.mmail_no &&
           i->first.maction_no == sc_mmml.maction_no) {
          i=multisam_definitions.erase(i);
        }
        else {
          ++i;
        }
      }
    }
  }
} /* update_multisam_definitions */

/*-------------------------------------------------------------------------*/
/*                         update_till_callback                            */
/*-------------------------------------------------------------------------*/
short update_till_callback(void *till_info)
{
  TILL sc_till;

  memcpy(&sc_till, till_info, sizeof(TILL));
  pos_system.small_inv_seq = sc_till.set_sequence_no;
  till.start_sequence_no = sc_till.start_sequence_no;
  till.end_sequence_no = sc_till.end_sequence_no;

  return 0;
} /* update_till_callback */

/*--------------------------------------------------------------------------*/
/*                    init_environment_records                              */
/*--------------------------------------------------------------------------*/
short init_environment_records(short scrn_type, void (*err_func)(void))
{
#define DISPLAY_TIME 300 /* To make everything visible */

  _TCHAR  dummy[100];
  short   status = SUCCEED;
  _TCHAR *init_error = NULL;

                                                  /* reset global variable */
  err_init_environment =  FALSE;

  /* POS SYSTEM */
  if( scrn_type != NO_WINDOW ) {
    display_prompt_right(_T("Loading pos_system. "), ERROR_WINDOW_ROW1);
    Sleep(DISPLAY_TIME);
  }
  status = pos_read_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
  if (status != SUCCEED) {
    init_error = _T("system");
    goto end_init;
  }

  /* DATE SYSTEM */
  if( scrn_type != NO_WINDOW ) {
    display_prompt_right(_T("  Init date_system. "), ERROR_WINDOW_ROW1);
    Sleep(DISPLAY_TIME);
  }
  status = init_date_system();
  if (status != SUCCEED) {
    init_error = _T("date");
    goto end_init;
  }

  /* GENVAR */
  if( scrn_type != NO_WINDOW ) {
    display_prompt_right(_T("    Loading genvar. "), ERROR_WINDOW_ROW1);
    Sleep(DISPLAY_TIME);
  }
  status = pos_read_genvar(POS_GENV_SIZE, GENVAR_FNO, &genvar);
  if (status != SUCCEED) {
    init_error = _T("genvar");
    goto end_init;
  }

  /* PAYMENT */
  if( scrn_type != NO_WINDOW ) {
    display_prompt_right(_T("Init payment types. "), ERROR_WINDOW_ROW1);
    Sleep(DISPLAY_TIME);
  }
  status = init_paym();
  if (status != SUCCEED) {
    init_error = _T("payment");
    goto end_init;
  }

  /* TAX */
  if( scrn_type != NO_WINDOW ) {
    display_prompt_right(_T("    Init tax table. "), ERROR_WINDOW_ROW1);
    Sleep(DISPLAY_TIME);
  }
  status = (short) init_tax_table();
  if (status != SUCCEED) {
    init_error = _T("vat");
    goto end_init;
  }

  /* SUMMARIZE_CD */
  if( scrn_type != NO_WINDOW ) {
    display_prompt_right(_T(" Init sum cd table. "), ERROR_WINDOW_ROW1);
    Sleep(DISPLAY_TIME);
  }
  status = (short) init_vat_sum_cd_table();
  if (status != SUCCEED) {
    init_error = _T("sum_cd");
    goto end_init;
  }

  /* MULTISAM */
  if( scrn_type != NO_WINDOW ) {
    display_prompt_right(_T(" Init multisam table "), ERROR_WINDOW_ROW1);
    Sleep(DISPLAY_TIME);
  }
  status = (short) init_multisam_definitions();
  if (status != SUCCEED) {
    init_error = _T("msam");
    goto end_init;
  }

  /* TILL */
  if( scrn_type != NO_WINDOW ) {
    display_prompt_right(_T("   Init till table. "), ERROR_WINDOW_ROW1);
    Sleep(DISPLAY_TIME);
  }
  status = (short) init_till_table();
  if (status != SUCCEED) {
    init_error = _T("till");
    goto end_init;
  }

end_init:
  if (scrn_type != NO_WINDOW) {
    display_prompt_right(_T("                    "), ERROR_WINDOW_ROW1);
  }

  if (init_error) {                      /* there was an error   */
    err_init_environment =  TRUE;
    if (scrn_type == NO_WINDOW) {
      if (err_func) {
        err_func();
      }
      _stprintf(dummy, _T("Error during init_environment_records() on init %s!"), init_error);
      MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
    }
  }

  return(status);
} /* init_environment_records */


/*--------------------------------------------------------------------------*/
/*                      init_date_system                                    */
/*--------------------------------------------------------------------------*/
short init_date_system(void)
{
  SYSTEMTIME st;

  GetLocalTime(&st);
                                                                   /* Convert to YYYYMMDD */
  pos_system.run_date  = (long)(st.wYear * 10000);
  pos_system.run_date += (st.wMonth * 100);
  pos_system.run_date += st.wDay;

  return(pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system));
} /* init_date_system */


/*-------------------------------------------------------------------------*/
/*                      activate_pwdn_callback                             */
/*-------------------------------------------------------------------------*/
short activate_pwdn_callback(void *not_used)
{
  int     cc;
  FILE   *fptr;
  _TCHAR  str_till[5];
  _TCHAR  buf[500];

  fptr = _tfopen(PWDN_CMD, _T("r"));
  if (!fptr) {
    return -1;                                     /* PWDN file not found */
  }

  fclose(fptr);

  de_init(_T("Powerdown command received from backoffice.")); /* Main de-init of application */
                                      /* Start sofware distribution script */
                     /* This should be the last action in this function.   */
                     /* If it fails, WinPOS will have to exit nevertheless.*/
  cc = _tspawnl( _P_NOWAIT, PWDN_CMD, PWDN_CMD, _itot(pos_system.till_no,str_till,10), NULL );
  if (cc < 0) {
    _stprintf(buf, _T("Error while starting powerdown script.\n\nScript\t: %s\nError\t: %s\n"), PWDN_CMD, AnsiToUnicode(strerror(errno)) );
    MessageBox(NULL, buf, (LPTSTR) _T("WinPOS Powerdown error"), MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_TASKMODAL);
  }

  exit(cc<0 ? 1 : 999);
} /* activate_pwdn_callback */

/*-------------------------------------------------------------------------*/
/*                         return_pos_status                               */
/*-------------------------------------------------------------------------*/
short return_pos_status(struct POS_STATUS *psd)
{
  SYSTEMTIME       st;
  
  psd->till_no = pos_system.till_no;
  psd->status  = status_of_pos;

  GetLocalTime(&st);

  _stprintf((_TCHAR *)psd->status_date, "%04.4hu%02.2hu%02.2hu%02.2hu%02.2hu%02.2hu",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

  return(SUCCEED);
} /* return_pos_status */

/*-------------------------------------------------------------------------*/
/*                         get_voucher_bo                                  */
/*-------------------------------------------------------------------------*/
short get_voucher_bo_turkey(long req_vale_no,   long req_vale_type, long req_voucher_status,
                            
                            short store_no,     long inv_cust_no, 
                            long inv_till_no,   long inv_invoice_type,  
                            long inv_invoice_no,long inv_invoice_date,
                            long inv_sequence_no,
                            VOUCHER_VALE_DEF *vouchrec, int time_wait)
{
  short          status;
  REQUEST_STRUCT request;
  struct REQ_VOUCHER_VALE req_voucher;

  VOUCHER_VALE_DEF vouchrec_base;
  
  if (vouchrec)
  {
    memcpy(&vouchrec_base,vouchrec, sizeof(VOUCHER_VALE_DEF));
  }

  request.subtype = rqtvouc_vale;                       /* sub request_type    */
  request.number  = (long)req_vale_no;

  req_voucher.vale_no = req_vale_no;
  req_voucher.vale_type = req_vale_type;
  req_voucher.status = req_voucher_status;

  req_voucher.store_no = store_no;
  req_voucher.inv_cust_no   = inv_cust_no;
  req_voucher.inv_till_no       = inv_till_no;
  req_voucher.inv_invoice_no    = inv_invoice_no;
  req_voucher.inv_invoice_date  = inv_invoice_date;
  req_voucher.inv_invoice_type  = inv_invoice_type;
  req_voucher.inv_sequence_no   = inv_sequence_no; 


  display_working(YES);

  memcpy(&(request.extra_request_info), &(req_voucher.vale_no), RVOU_VALE_SIZE);
  /* With hexidecimal codes you can check the contents of extra_request_info */

  status = bo_request(VOUC_VALE_TYPE, time_wait*MAXWAIT, &request,&vouchrec_base);


  if (time_wait==0)
    status=(status==TIME_OUT ? SUCCEED : status);    
  
  display_working(NO);


  if (vouchrec)
  {
    memcpy(vouchrec, &vouchrec_base, sizeof(VOUCHER_VALE_DEF));
  }

  return(status);
} /* get_voucher_bo */





/*-------------------------------------------------------------------------*/
/*                         get_voucher_bo                                  */
/*-------------------------------------------------------------------------*/
short get_voucher_bo_anticipo(long req_vale_no,   long req_vale_type, long req_voucher_status,
                            short store_no,     long inv_cust_no, 
                            long inv_till_no,   long inv_invoice_type,  
                            long inv_invoice_no,long inv_invoice_date,
                            long inv_sequence_no,
                            VOUCHER_ANTICIPO_DEF *vouchrec, int time_wait)
{
  short          status;
  REQUEST_STRUCT request;
  struct REQ_VOUCHER_ANTICIPO req_voucher;

  VOUCHER_ANTICIPO_DEF vouchrec_base;
  
  if (vouchrec)
  {
    memcpy(&vouchrec_base,vouchrec, sizeof(VOUCHER_ANTICIPO_DEF));
  }

  request.subtype = rqtvouc_anticipo;                       /* sub request_type    */
  request.number  = (long)req_vale_no;

  req_voucher.seq_no = req_vale_no;
  req_voucher.seq_type = req_vale_type;
  req_voucher.status = req_voucher_status;

  req_voucher.store_no = store_no;
  req_voucher.inv_cust_no   = inv_cust_no;
  req_voucher.inv_till_no       = inv_till_no;
  req_voucher.inv_invoice_no    = inv_invoice_no;
  req_voucher.inv_invoice_date  = inv_invoice_date;
  req_voucher.inv_invoice_type  = inv_invoice_type;
  req_voucher.inv_sequence_no   = inv_sequence_no; 


  display_working(YES);

  memcpy(&(request.extra_request_info), &(req_voucher.seq_no), RVOU_ANTICIPO_SIZE);
  /* With hexidecimal codes you can check the contents of extra_request_info */

  status = bo_request(ANTICIPO_PROM_TYPE, time_wait*MAXWAIT, &request,&vouchrec_base);


  if (time_wait==0)
    status=(status==TIME_OUT ? SUCCEED : status);    
  
  display_working(NO);


  if (vouchrec)
  {
    memcpy(vouchrec, &vouchrec_base, sizeof(VOUCHER_ANTICIPO_DEF));
  }

  return(status);
} /* get_voucher_bo */
#pragma warning(pop) /* Pop old warning level from stack. */




