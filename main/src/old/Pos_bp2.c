/* 
 *     Module Name       : POS_BP2.C
 *
 *     Type              : Application Print Functions
 *                         (invoice, x-read, z-read)
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
 * 04-Oct-2000 Changed calculation of print_ln in fn_i_msam_header,
 *             fn_i_msam_line, fn_i_msam_total.                        J.D.M.
 * 09-Oct-2000 Changed prn_on into prn_on[printer]                     J.D.M.
 * 16-May-2001 Bug in printing of price per pack of deposit art        R.N.B.
 * --------------------------------------------------------------------------
 * 31-Oct-2001 Added star (***amount) for amount and quantity            M.W.
 * --------------------------------------------------------------------------
 * 09-Apr-2002 Added Customer Fee on Day Pass                            M.W.
 * --------------------------------------------------------------------------
 * 16-Oct-2002 Added sending of init_codes in several functions to make
 *             sure that the printer will correct if it has forgotten
 *             it's settings.                                          J.D.M.
 * --------------------------------------------------------------------------
 * 27-Aug-2003 Renamed TOT_CARR_FORWD to TOT_CARR_FORWD_INCL.          J.D.M.
 * --------------------------------------------------------------------------
 * 16-Dec-2003 Added Donation functionality.                             M.W.
 * --------------------------------------------------------------------------
 * 10-Feb-2004 Prevent that line text is emptied by
 *             fmt_left_justify_string() in fn_i_msam_line().          J.D.M.
 * --------------------------------------------------------------------------
 * 17-Feb-2004 A Separate 'carried forward' total is used to store           
 *             MultiSam discounts in order to avoid rounding diffs.      P.M.
 * --------------------------------------------------------------------------
 * 06-May-2004 Added vat amounts on total section.                       M.W.
 * --------------------------------------------------------------------------
 * 06-Jul-2005 Bugfix: Rounding of Multisam discounts in case of 
 *             inclusive amounts on invoice                              M.W.
 * --------------------------------------------------------------------------
 * 01-Aug-2005 Detail payment on invoice.                                M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 * 23-Feb-2006 Revision of small invoice layout.                       J.D.M.
 * --------------------------------------------------------------------------
 * 13-Mar-2006 Added printing of small invoice sequence number on small
 *             invoice, xread and zread.                               J.D.M.
 * --------------------------------------------------------------------------
 * 12-Jul-2011 Added initialization of var buffer
 *             this is un fix / patch		                           A.C.M.
 * --------------------------------------------------------------------------
 */

/**/

#include <string.h>
#include <stdlib.h>
/**/

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include "appbase.h"
#include <stdio.h>
#include <math.h>
                                            /* Pos (library) include files   */
#include "comm_tls.h"
#include "stri_tls.h"
#include "date_tls.h"

                                            /* Toolsset include files.       */
#include "fmt_tool.h"
#include "mem_mgr.h"
#include "prn_mgr.h"
#include "tot_mgr.h"
#include "err_mgr.h"
#include "tm_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "llist.h"
#include "sll_mgr.h"

#include "pos_inp.h"                        /* Application include files.    */
#include "pos_recs.h"
#include "pos_txt.h"
#include "pos_tm.h"
#include "pos_tot.h"
#include "WPos_mn.h"
#include "pos_func.h"
#include "pos_com.h"
#include "pos_tot.h"
#include "write.h"
#include "pos_bp2.h"
#include "pos_bp1.h"
#include "st_main.h"
#include "pos_errs.h"
#include "pos_msam.h"
#include "pos_tot.h"
#include "pos_tran.h"
#include "pos_errs.h"
#include "comm_tls.h"
#include "intrface.h" /* 27-Jan-2012 acm -  */
#include <stdarg.h>   /* 27-Jan-2012 acm -  */
#include <stdarg.h>   /* 27-Jan-2012 acm -  */
#include <time.h>

#include "Pos_edoc.h"

extern short print_ln_fmt(short printer_id, char* format, ...); /* 27-Jan-2012 acm -  */
extern short print_ln_fmt_centre(short printer_id, short size, char* format, ...); /* 27-Jan-2012 acm -  */

/* 27-April-2012 acm - { */
/* XPrint dolar*tipocambio  */
#define EV_PRINTX 1
#define EV_PRINTZ 2

int flag_paym_totals_per_cshr=0; 

/* 27-April-2012 acm - }*/


short  on_ln_inv = START_LN_CUST-2;    /* current line on invoice at startup */
short  pg_no;

extern short power_fail;

static void  add_left_margin(_TCHAR *);
static short print_vat_payment_lines(short);


 _TCHAR  first1[75 +1];
 _TCHAR  next1[75 +1];

/*
 * Definition of misc. objects
 */
extern PRN_OBJECT forward_skip         = { fn_forward_skip,       (short)0 };
extern PRN_OBJECT reverse_skip         = { fn_reverse_skip,       (short)0 };
extern PRN_OBJECT init_prn             = { fn_init_prn,           (short)0 };

/*
 * Definition of objects of X-read and Z-read.
 */
extern PRN_OBJECT xr_zr_init              = { fn_xr_zr_init,                (short)0 };
extern PRN_OBJECT xr_header               = { fn_xr_header,                 (short)0 };
extern PRN_OBJECT xr_zr_paym_header       = { fn_xr_zr_paym_header,         (short)0 };
extern PRN_OBJECT xr_paym_amnts           = { fn_xr_paym_amnts,             (short)0 };
extern PRN_OBJECT xr_paym_totals_per_cshr = { fn_xr_paym_totals_per_cshr,   (short)0 };
extern PRN_OBJECT xr_zr_time_header       = { fn_xr_zr_time_header,         (short)0 };
extern PRN_OBJECT xr_zr_shift_per_time    = { fn_xr_zr_shift_per_time,      (short)0 };
extern PRN_OBJECT xr_till_totals          = { fn_xr_till_totals,            (short)0 };
extern PRN_OBJECT zr_header               = { fn_zr_header,                 (short)0 };
extern PRN_OBJECT zr_paym_totals_per_cshr = { fn_zr_paym_totals_per_cshr,   (short)0 };
extern PRN_OBJECT zr_paym_totals_per_till = { fn_zr_paym_totals_per_till,   (short)0 };
extern PRN_OBJECT zr_till_totals          = { fn_zr_till_totals,            (short)0 };
extern PRN_OBJECT zr_connect              = { fn_zr_connect,                (short)0 };


/*
 * Definition of objects of the Invoice.
 */
extern PRN_OBJECT i_total_lines    = { fn_i_total_lines,   (short)0 };
extern PRN_OBJECT i_article_line   = { fn_i_article_line,  (short)0 };
extern PRN_OBJECT i_deposit_line   = { fn_i_deposit_line,  (short)0 };
extern PRN_OBJECT i_discount_line  = { fn_i_discount_line, (short)0 };
extern PRN_OBJECT i_customer       = { fn_i_customer,      (short)0 };
extern PRN_OBJECT i_void_inv       = { fn_i_void_inv,      (short)0 };
extern PRN_OBJECT i_to_end_of_page = { fn_i_to_end_of_page,(short)0 };
extern PRN_OBJECT i_subtotal       = { fn_i_subtotal,      (short)0 };
extern PRN_OBJECT i_skip_one_line  = { fn_i_skip_one_line, (short)0 };
extern PRN_OBJECT i_promo          = { fn_i_promo,         (short)0 };
extern PRN_OBJECT i_to_transport   = { fn_i_to_transport,  (short)0 };
extern PRN_OBJECT i_transported    = { fn_i_transported,   (short)0 };
extern PRN_OBJECT i_empty_bottom   = { fn_i_empty_bottom,  (short)0 };
extern PRN_OBJECT i_cust_fee_line  = { fn_i_cust_fee_line, (short)0 };
extern PRN_OBJECT i_msam_header    = { fn_i_msam_header,   (short)0 };
extern PRN_OBJECT i_msam_line      = { fn_i_msam_line,     (short)0 };
extern PRN_OBJECT i_msam_total     = { fn_i_msam_total,    (short)0 };
extern PRN_OBJECT i_small_header   = { fn_i_small_header,  (short)0 };
extern PRN_OBJECT i_small_article  = { fn_i_small_article, (short)0 };
extern PRN_OBJECT i_small_deposit  = { fn_i_small_deposit, (short)0 };
extern PRN_OBJECT i_small_discount = { fn_i_small_discount,(short)0 };
extern PRN_OBJECT i_small_total    = { fn_i_small_total,   (short)0 };
extern PRN_OBJECT i_small_tax      = { fn_i_small_tax,     (short)0 };
extern PRN_OBJECT i_small_footer   = { fn_i_small_footer,  (short)0 };
extern PRN_OBJECT i_small_void     = { fn_i_small_void,    (short)0 };
extern PRN_OBJECT i_small_promo    = { fn_i_small_promo,   (short)0 };
extern PRN_OBJECT i_small_cust_fee = { fn_i_small_cust_fee,(short)0 };
extern PRN_OBJECT i_small_msam_header = { fn_i_small_msam_header, (short)0 };
extern PRN_OBJECT i_small_msam_line   = { fn_i_small_msam_line,   (short)0 };
extern PRN_OBJECT i_small_msam_total  = { fn_i_small_msam_total,  (short)0 };

char *NoToLet(const char *);  /*funcion que convierte de numero a palabras*/

/*----------------------------------------------------------------------------*/
/*                           M I S C E L L A N E O U S                        */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                           fn_forward_skip                                  */
/*----------------------------------------------------------------------------*/
/* Advances the invoice paper LN_TEAR_OFF lines so the paper can be torn off  */
/*----------------------------------------------------------------------------*/
short fn_forward_skip(PRN_OBJECT *p_object, short action,
                      short offs_grp, short printer)
{
/* TODO */

  short i;

  if (action == CALCULATE_IT) {
    p_object->ln=0;
  }

  if (action == PRINT_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      return(SUCCEED);
    }
    for (i=0; i < LN_TEAR_OFF; i++) {
      ec_prn_lf(printer);
    }
  }


  return(SUCCEED);
} /* fn_forward_skip */

/*----------------------------------------------------------------------------*/
/*                           fn_reverse_skip                                  */
/*----------------------------------------------------------------------------*/
/* Pull back the invoice paper LN_TEAR_OFF lines so the paper is positioned   */
/* the first printable line.                                                  */
/*----------------------------------------------------------------------------*/
short fn_reverse_skip(PRN_OBJECT *p_object, short action,
                      short offs_grp, short printer)
{
  _TUCHAR rev_lf[] =
  { ESCAPE
   ,_T('\x6A')                                         /* By 8 LPI 216/8=27   */
   ,_T('\x1B')                                         /* 27/216 Inch = 1 line*/
  };
  short rev_lf_len = sizeof(rev_lf)/sizeof(_TUCHAR);
  short lines;

  if (action == CALCULATE_IT) {
    p_object->ln=0;
  }

  /* The reversed linefeed for the printer is given in n/216 Inch. We use     */
  /* 8 LPI, so 216/8 is 27 for one reversed linefeed. The variable n has a    */
  /* maximum of 255, so maximal 9 reversed linefeeds (9*27=243) can be send   */
  /* in one command.                                                          */

  if (action == PRINT_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      return(SUCCEED);
    }
    lines = LN_TEAR_OFF;
    while (lines > 0) {
      if (lines > 7) {
        rev_lf[2] = (_TUCHAR)(36 * 9);               /* Maximum of 255      */
      }
      else {
        rev_lf[2] = (_TUCHAR)(36 * lines);
      }
      lines -= 9;
      ec_prn_raw(rev_lf, rev_lf_len, printer);
    }
  }

  return(SUCCEED);
} /* fn_reverse_skip */

/*----------------------------------------------------------------------------*/
/*                           fn_init_prn                                      */
/*----------------------------------------------------------------------------*/
/* Initialize the destination printer and check if ready. If not ready give   */
/* an error message.                                                          */
/*----------------------------------------------------------------------------*/
/* NOTE the init codes are currently not interpreted by ec_prn_init!!         */
/*----------------------------------------------------------------------------*/
short fn_init_prn(PRN_OBJECT *p_object, short action,
                  short offs_grp, short printer)
{
  short prn_status;

  if (print_to_file==YES) {
    /* Do nothing */
  }
  else {
    while (prn_on[printer]==YES &&
		(prn_status = ec_prn_init(init_codes, size_init_codes, printer)) != SUCCEED) { /*FredM*/
      if (prn_status == EC_PRN_NOT_READY) {
        err_invoke(PRN_FATAL_ERROR);
      }
      if(prn_status == EC_PRN_NO_PAPER) {
        err_invoke(PRN_JOURNAL_PAPER_OUT);
      }
    }
  }

  if (!power_fail) {
    pg_no = 1;                       /* Initialization of invoice page number */
  }

  return(SUCCEED);
} /* fn_init_prn */

/*----------------------------------------------------------------------------*/
/*                            X  -  R  E  A  D                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                           fn_xr_zr_init                                    */
/*----------------------------------------------------------------------------*/
/* Prints linefeeds until start line for X-read (or Z-read) is reached.       */
/*----------------------------------------------------------------------------*/
short fn_xr_zr_init(PRN_OBJECT *p_object,
                    short action, short offs_grp, short printer)
{

  if (action == CALCULATE_IT) {
    p_object->ln=0;
  }

  if (action == PRINT_IT) {
    while (on_ln_inv < START_LN_XREAD) {
      print_ln(printer, empty);
    }
  }
  send_init_codes(printer);

  return (SUCCEED);
} /* fn_xr_zr_init */


/*----------------------------------------------------------------------------*/
/*                           fn_xr_header                                     */
/*----------------------------------------------------------------------------*/
/* Prints 1 header line of the X-read.                                        */
/*----------------------------------------------------------------------------*/
short fn_xr_header(PRN_OBJECT *p_object, short action,
                   short offs_grp, short printer)
{
  _TCHAR  str_time[6];
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  str_date[14];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }

  if (action == PRINT_IT) {
    get_current_time(str_time);
    format_string(&string_time5, str_time);
    prn_fmt_date(pos_system.run_date, genvar.date_format, mon_names, str_date);
    _stprintf(buffer, prn_xr_TXT[2], pos_system.till_no, str_date,
                                     c_shft.shift_no, string_time5.result);
    /*add_left_margin(buffer); comentado para soporte a x y z en tirilla*/
    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_xr_header */

/*----------------------------------------------------------------------------*/
/*                           fn_zr_connect()                                  */
/*----------------------------------------------------------------------------*/
/* Imprime 1 linea con el status de la red ON LINE/OFF LINE                   */
/*----------------------------------------------------------------------------*/
short fn_zr_connect(PRN_OBJECT *p_object, short action, 
                    short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }
  if (action == PRINT_IT) {
    switch (get_bo_status()) {
      case CONN:
        _stprintf(buffer, prn_xr_TXT[20], _T("ON LINE"));
        break;
      case GONE:
        _stprintf(buffer, prn_xr_TXT[20], _T("OFF LINE"));
        break;
      case CFGERR:
        _stprintf(buffer, prn_xr_TXT[20], _T("CFG ERR"));
        break;
      default:
        _stprintf(buffer, prn_xr_TXT[20], _T(""));
        break;
    }
    /*add_left_margin(buffer);*/
    print_ln(printer, buffer);
  }
  return (SUCCEED);
} /* fn_zr_connect() */


short fn_zr_connect_TR(PRN_OBJECT *p_object, short action, 
                    short offs_grp, short printer) /*funcion original*/
{
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }
  if (action == PRINT_IT) {
    switch (get_bo_status()) {
      case CONN:
        _stprintf(buffer, prn_xr_TXT[20], _T("ON LINE"));
        break;
      case GONE:
        _stprintf(buffer, prn_xr_TXT[20], _T("OFF LINE"));
        break;
      case CFGERR:
        _stprintf(buffer, prn_xr_TXT[20], _T("CFG ERR"));
        break;
      default:
        _stprintf(buffer, prn_xr_TXT[20], _T(""));
        break;
    }
    add_left_margin(buffer);
    print_ln(printer, buffer);
  }
  return (SUCCEED);
} /* fn_zr_connect() */


/*----------------------------------------------------------------------------*/
/*                           fn_zr_header                                     */
/*----------------------------------------------------------------------------*/
/* Prints 1 header line of the Z-read.                                        */
/*----------------------------------------------------------------------------*/
short fn_zr_header(PRN_OBJECT *p_object, short action,
                   short offs_grp, short printer)
{
  _TCHAR  str_time[6];
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  str_date[14];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }

  if (action == PRINT_IT) {
    get_current_time(str_time);
    format_string(&string_time5, str_time);
    prn_fmt_date(pos_system.run_date, genvar.date_format, mon_names, str_date);
    _stprintf(buffer, prn_xr_TXT[35], pos_system.till_no, str_date,
                                                        string_time5.result);
    add_left_margin(buffer);
    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_zr_header */



short fn_zr_header_TR(PRN_OBJECT *p_object, short action,
                   short offs_grp, short printer)  /*FUNCION ORIGINAL*/
{
  _TCHAR  str_time[6];
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  str_date[14];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }

  if (action == PRINT_IT) {
    get_current_time(str_time);
    format_string(&string_time5, str_time);
    prn_fmt_date(pos_system.run_date, genvar.date_format, mon_names, str_date);
    _stprintf(buffer, prn_xr_TXT[17], pos_system.till_no, str_date,
                                                        string_time5.result);
    add_left_margin(buffer);
    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_zr_header */


/*----------------------------------------------------------------------------*/
/*                           fn_xr_zr_paym_header                             */
/*----------------------------------------------------------------------------*/
/* Prints 1 or 2 lines with the allowed payment way descriptions. 5 payment   */
/* way descriptions will fit on one line.                                     */
/*----------------------------------------------------------------------------*/
short fn_xr_zr_paym_header(PRN_OBJECT *p_object, short action,
                           short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
 /* _TCHAR  first[75 +1];
  _TCHAR  next[75 +1];*/
  short nbr_paym;

  nbr_paym=calc_nbr_paym_ways();
  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      if (nbr_paym > 5) {
        p_object->ln=2;
      }
      else {
        p_object->ln=1;
      }
    }
  }

  if (action == PRINT_IT) {
    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE] = _T('\0');
    str_paym_descr(first1, next1);
	/*En esta linea imprime los tipos de pago las cabeceras*/
    /*if (nbr_paym > 5) {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[6], first, prn_xr_TXT[8]);
      add_left_margin(buffer);
      print_ln(printer, buffer);
      _stprintf(buffer, prn_xr_TXT[5], empty, next, empty );
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }
    else {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[6], first, prn_xr_TXT[8] );
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }*/
  }

  return (SUCCEED);
} /* fn_xr_zr_paym_header */


/*----------------------------------------------------------------------------*/
/*                           calc_nbr_paym_ways                               */
/*----------------------------------------------------------------------------*/
/* Returns the number of payment ways allowed. If the payment way limit is    */
/* greater than zero, the payment way is allowed.                             */
/*----------------------------------------------------------------------------*/
short calc_nbr_paym_ways(void)
{
  PAYMENT_DEF paym;
  short nbr, i;

  nbr=0;
  for (i=PAYM_WAY_0; i<MAX_PAYM_WAYS; i++) {
    paym.paym_cd=i;
    get_paym(&paym);
    if (paym.paym_limit>0) {
      nbr++;
    }
  }

  return(nbr);
} /* calc_nbr_paym_ways */


/*----------------------------------------------------------------------------*/
/*                           str_paym_descr                                   */
/*----------------------------------------------------------------------------*/
/* Composes two strings (first and next) with the descriptions of the allowed */
/* payment ways.                                                              */
/* Max. 10 payment ways are allowed.                                          */
/* The first and next buffer size should at least be 76 characters            */
/*----------------------------------------------------------------------------*/
short str_paym_descr(_TCHAR first[], _TCHAR next[])   /*no se toco*/
{
  PAYMENT_DEF paym;
  short  i = 0;
  short  nbr = 0;
  _TCHAR *p;
  short printer = 1;
//  _TCHAR   buffer[INV_SIZE+LEFT_MARGIN+1];

  *first = _T('\0');
  *next = _T('\0');

  for (i=PAYM_WAY_1; i <= MAX_PAYM_WAYS; i++) {
    if (i==MAX_PAYM_WAYS) {
      paym.paym_cd=0;
    }
    else {
      paym.paym_cd=i;
    }
    get_paym(&paym);
    if (paym.paym_limit > 0) {
      if (nbr < 5) {
        p=first;
        nbr++;
      }
      else {
        p=next;
      }
      while(*p) p++;                            /* to end of string          */
      ch_memset(p, _T(' '), 16*sizeof(_TCHAR));
      *(p+15)=_T('\0');
      fmt_right_justify_string(p,0,14,paym.paym_descr);


    }
  }

  return (SUCCEED);
} /* str_paym_descr */


/*----------------------------------------------------------------------------*/
/*                           str_paym_amnts                                   */
/*----------------------------------------------------------------------------*/
/* Composes two strings (first and next) with the allowed payment amounts.    */
/* Max. 10 payment ways are used.                                             */
/*----------------------------------------------------------------------------*/
short str_paym_amnts(_TCHAR *first, _TCHAR *next, double *p_tot,  short printer)
{
  _TCHAR   buffer[INV_SIZE+LEFT_MARGIN+1];
	
	PAYMENT_DEF paym;
  _TCHAR  paym_tot[TOTAL_BUF_SIZE +1];
   short i, nbr;
   double fix_donation=0; //3.4.7 acm -
   double paym_amnt   =0; //3.4.7 acm -

  i   = 0;
  nbr = 0;
  _tcscpy(first, _T(" "));
  _tcscpy(next,  _T(" "));
  
  fix_donation=c_shft.donation;
  for (i = PAYM_WAY_1; i <= MAX_PAYM_WAYS; i++) {
    if (i == MAX_PAYM_WAYS) {
      paym.paym_cd=0;
    }
    else {
      paym.paym_cd=i;
    }
    get_paym(&paym);
    if (paym.paym_limit > 0) {
        paym_amnt=c_shft.paym_amnt[paym.paym_cd];
        *p_tot+=(paym_amnt*paym.curr_standard)/paym.curr_rate;  //3.4.7 acm -

      //V3.4.7 acm -  { restar la donacion del medio de pago soles
    
      if (i==PAYM_WAY_1){
         paym_amnt-=fix_donation; 
         *p_tot-=fix_donation; 
         fix_donation=0;
      }
      //V3.4.7 acm -  }  


      ftoa_price(paym_amnt, TOTAL_BUF_SIZE, paym_tot);
      format_string(&string_price15, paym_tot);
      if (nbr < 5) {
        _tcscpy(first, string_price15.result);
		/****/
		_stprintf(buffer, prn_xr_TXT[23], paym.paym_descr, first);
        print_ln(printer, buffer);
		ch_memset(first, _T(' '), sizeof(first));
		/****/
        nbr++;
      }
      else {
        _tcscpy(next, string_price15.result);
		/****/
		_stprintf(buffer, prn_xr_TXT[23], paym.paym_descr, next);
        print_ln(printer, buffer);
		ch_memset(next, _T(' '), sizeof(next));
		/****/
      }
    }
  }

  return (SUCCEED);
} /* str_paym_amnts */


/*----------------------------------------------------------------------------*/
/*                           fn_xr_paym_amnts                                 */
/*----------------------------------------------------------------------------*/
/* Prints one or two lines (depending on the number of allowed payment ways)  */
/* with the allowed payment amounts per payment way.                          */
/*----------------------------------------------------------------------------*/
short fn_xr_paym_amnts(PRN_OBJECT *p_object, short action,
                       short offs_grp, short printer)
{
  _TCHAR   buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR   buffer1[TOTAL_BUF_SIZE+1];
  _TCHAR   first[PAYM_DESCR_SIZE * 5 +2];
  _TCHAR   next[PAYM_DESCR_SIZE * 5 +2];
//  _TCHAR   tmp[INV_SIZE+1];
//  _TCHAR  cshr_tot[TOTAL_BUF_SIZE +1];

  short  nbr_paym;
  double p_tot;

  nbr_paym=calc_nbr_paym_ways();

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      if (nbr_paym > 5) {
        p_object->ln=2;
      }
      else {
        p_object->ln=1;
      }
    }
  }

  if (action == PRINT_IT) {
    *first = (_TCHAR)0;
    *next  = (_TCHAR)0;
    p_tot  = 0.0;

   /*imprime el codigo del cajero*/
	_stprintf(buffer, prn_xr_TXT[22], c_shft.cashier);
    print_ln(printer, empty);
    print_ln(printer, buffer);
	
    /*****/

    str_paym_amnts(first, next, &p_tot, printer);   /*funcion modefifcada para tirrilla*/
    ftoa_price(p_tot, TOTAL_BUF_SIZE, buffer1);
    format_string(&string_price15, buffer1);
    _tcscpy(buffer1, string_price15.result);
   
    /*
    //v3.4.7 acm - fix
   paym_amnts_tot_cshr(first, next, cshr_tot);  //imprime el total del cajero
   _stprintf(buffer, prn_xr_TXT[24],cshr_tot);
   */

   _stprintf(buffer, prn_xr_TXT[24],buffer1); //v3.4.7 acm - fix
   print_ln(printer, buffer);//v3.4.7 acm - fix
   //print_ln(printer, buffer1);//v3.4.7 acm - fix

    /*Imprime los montos de los tipos de pago*/

    /* if (nbr_paym > 5) {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[6], first1, prn_xr_TXT[8]);
      add_left_margin(buffer);
      print_ln(printer, buffer);
      _stprintf(buffer, prn_xr_TXT[5], empty, next1, empty );
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }
    else {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[6], first1, prn_xr_TXT[8] );
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }*/



	/*******************************************/



   /* if (nbr_paym > 5) {
      _stprintf(buffer, prn_xr_TXT[4], c_shft.cashier, first, empty);
      add_left_margin(buffer);
      print_ln(printer, buffer);
      *buffer = (_TCHAR)0;
      _stprintf(tmp, prn_xr_TXT[16], next);
      _tcscat(buffer, tmp);
      _tcscat(buffer, _T("  "));
      _tcscat(buffer, buffer1);
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }
    else {
      _stprintf(buffer, prn_xr_TXT[19], c_shft.cashier, first);
      _tcscat(buffer, _T("  "));
      _tcscat(buffer, buffer1);
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }*/


 

  }

  return (SUCCEED);
} /* fn_xr_paym_amnts */

/*----------------------------------------------------------------------------*/
/*                           fn_xr_paym_totals_per_cshr                       */
/*----------------------------------------------------------------------------*/
short fn_xr_paym_totals_per_cshr_TR(PRN_OBJECT *p_object, short action,
                                 short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  first[75 +2];
  _TCHAR  next[75 +2];
  _TCHAR  cshr_tot[TOTAL_BUF_SIZE +1];
  short nbr_paym;

  
  nbr_paym=calc_nbr_paym_ways();

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      if (nbr_paym > 5) {
        p_object->ln=4;
      }
      else {
        p_object->ln=3;
      }
    }
  }

  if (action == PRINT_IT) {
    _tcscpy(buffer,prn_xr_TXT[14]);
    add_left_margin(buffer);
    print_ln(printer, buffer);
    *first    = (_TCHAR)0;
    *next     = (_TCHAR)0;
    *cshr_tot = (_TCHAR)0;
    paym_amnts_tot_cshr(first, next, cshr_tot);
    if (nbr_paym > 5) {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[9], first, empty );
      add_left_margin(buffer);
      print_ln(printer, buffer);
      _stprintf(buffer, prn_xr_TXT[5], empty, next, cshr_tot);
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }
    else {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[9], first, cshr_tot);
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }
    print_ln(printer, empty);
  }

  return (SUCCEED);
} /* fn_xr_paym_totals_per_cshr */


/*----------------------------------------------------------------------------*/
/*                           fn_xr_paym_totals_per_cshr                       */
/*----------------------------------------------------------------------------*/
short fn_xr_paym_totals_per_cshr(PRN_OBJECT *p_object, short action,
                                 short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  first[75 +2];
  _TCHAR  next[75 +2];
  _TCHAR  cshr_tot[TOTAL_BUF_SIZE +1];
  short nbr_paym;


  flag_paym_totals_per_cshr=EV_PRINTX; /* 27-April-2012 acm - XPrint dolar*tipocambio */

  nbr_paym=calc_nbr_paym_ways();

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      if (nbr_paym > 5) {
        p_object->ln=4;
      }
      else {
        p_object->ln=3;
      }
    }
  }

  if (action == PRINT_IT) {
    /*_tcscpy(buffer,prn_xr_TXT[14]);*/
    /*add_left_margin(buffer);*/
    /*print_ln(printer, buffer);*/
    *first    = (_TCHAR)0;
    *next     = (_TCHAR)0;
    *cshr_tot = (_TCHAR)0;
    paym_amnts_tot_cshr_TR(first, next, cshr_tot,printer);
    if (nbr_paym > 5) {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[9], first, empty );
      add_left_margin(buffer);
     /* print_ln(printer, buffer);*/
      _stprintf(buffer, prn_xr_TXT[5], empty, next, cshr_tot);
      add_left_margin(buffer);
      /*print_ln(printer, buffer);*/
    }
    else {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[9], first, cshr_tot);
      add_left_margin(buffer);
      /*print_ln(printer, buffer);*/
    }

  /*imprimo el total de la caja ojo no del cajero*/
  _stprintf(buffer, prn_xr_TXT[24],cshr_tot);
  print_ln(printer, buffer);
  print_ln(printer, empty);

  }

  return (SUCCEED);
} /* fn_xr_paym_totals_per_cshr */


/*----------------------------------------------------------------------------*/
/*                           fn_zr_paym_totals_per_cshr                       */
/*----------------------------------------------------------------------------*/
short fn_zr_paym_totals_per_cshr(PRN_OBJECT *p_object, short action,
                                 short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  cshr_tot[TOTAL_BUF_SIZE +1];
  _TCHAR  first[75 +2];
  _TCHAR  next[75 +2];
  short nbr_paym, cashier;

  flag_paym_totals_per_cshr=EV_PRINTZ; /* 27-April-2012 acm - XPrint dolar*tipocambio */

  nbr_paym=calc_nbr_paym_ways();

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      if (nbr_paym > 5) {
        p_object->ln=2;
      }
      else {
        p_object->ln=1;
      }
    }
  }

  if (action == PRINT_IT) {
    *first    = (_TCHAR)0;
    *next     = (_TCHAR)0;
    *cshr_tot = (_TCHAR)0;
    cashier=c_shft.cashier;     /* paym_amnts..() changes the current shift. */


	/*imprime el codigo del cajero*/
	_stprintf(buffer, prn_xr_TXT[22], c_shft.cashier);
    print_ln(printer, empty);
    print_ln(printer, buffer);

    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    /*****/

    paym_amnts_tot_cshr_TR(first, next, cshr_tot, printer);


   /*paym_amnts_tot_cshr(first, next, cshr_tot); */ /*imprime el total del cajero*/
   _stprintf(buffer, prn_xr_TXT[24],cshr_tot);
   print_ln(printer, buffer);

    /*if (nbr_paym > 5) {
      _stprintf(buffer, prn_xr_TXT[4], cashier, first, empty );
      add_left_margin(buffer);
      print_ln(printer, buffer);
      _stprintf(buffer, prn_xr_TXT[5], empty, next, cshr_tot);
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }
    else {
      _stprintf(buffer, prn_xr_TXT[4], cashier, first, cshr_tot);
      add_left_margin(buffer);
      print_ln(printer, buffer);
    }*/
  }

  return (SUCCEED);
} /* fn_zr_paym_totals_per_cshr */


/*----------------------------------------------------------------------------*/
/*                           fn_zr_paym_totals_per_till                       */
/*----------------------------------------------------------------------------*/
short fn_zr_paym_totals_per_till(PRN_OBJECT *p_object, short action,
                                 short offs_grp, short printer)
{
  _TCHAR  till_tot[TOTAL_BUF_SIZE +1];
  _TCHAR  first[75 +2];
  _TCHAR  next[75 +2];
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  short nbr_paym;

  nbr_paym=calc_nbr_paym_ways();

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      if (nbr_paym > 5) {
        p_object->ln=2;
      }
      else {
        p_object->ln=1;
      }
    }
  }

  if (action == PRINT_IT) {
    *first    = (_TCHAR)0;
    *next     = (_TCHAR)0;
    *till_tot = (_TCHAR)0;
    paym_amnts_tot_till_TR(first, next, till_tot,printer);

   _stprintf(buffer, prn_xr_TXT[24],till_tot);

   print_ln(printer, buffer);

    if (nbr_paym > 5) {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[9], first, empty );
      add_left_margin(buffer);
      /*print_ln(printer, buffer);*/
      _stprintf(buffer, prn_xr_TXT[5], empty, next, till_tot);
      add_left_margin(buffer);
      /*print_ln(printer, buffer);*/
    }
    else {
      _stprintf(buffer, prn_xr_TXT[5], prn_xr_TXT[9], first, till_tot);
      add_left_margin(buffer);
      /*print_ln(printer, buffer);*/
    }
  }

  return (SUCCEED);
} /* fn_zr_paym_totals_per_till */


/*----------------------------------------------------------------------------*/
/*                           paym_amnts_tot_cshr                              */
/*----------------------------------------------------------------------------*/
/* Composes two strings (first and next) with the allowed payment amounts     */
/* totals per cashier.                                                        */
/* Max. 10 payment ways are used.                                             */
/*----------------------------------------------------------------------------*/

short paym_amnts_tot_cshr_TR(_TCHAR *first, _TCHAR *next, _TCHAR *cshr_tot,short printer)
{
  PAYMENT_DEF paym;
  _TCHAR   dummy[TOTAL_BUF_SIZE+1];
  double paym_tot[MAX_PAYM_WAYS];

  _TCHAR first_next_buffer[120];

  /* 27-April-2012 acm - { XPrint dolar*tipocambio */
  double paym_tot_local[MAX_PAYM_WAYS];
  /* 27-April-2012 acm - } */
  double fix_donation;//V3.4.7 acm -

  double grand_tot;
  short  nbr, i, old_cshr;
   _TCHAR  buffer[SMALL_INV_SIZE+LEFT_MARGIN+1];

 

  _tcscpy(first, _T(" "));
  _tcscpy(next,  _T(" "));


  print_ln(printer, empty);
  _stprintf(buffer, prn_xr_TXT[25]);
  print_ln(printer, buffer);

  grand_tot=0.0;
  for (i=0; i<MAX_PAYM_WAYS; i++) {
    paym_tot[i]=0.0;
    paym_tot_local[i]=0.0;/* 27-April-2012 acm - */
  }
  old_cshr=c_shft.cashier;

  do {
    if (c_shft.cashier == old_cshr) {
     fix_donation = c_shft.donation; //V3.4.7 acm -
  
      nbr = 0;
      for (i=PAYM_WAY_0; i<=PAYM_WAY_9; i++) {
        paym.paym_cd = i;
        get_paym(&paym);
        if (paym.paym_limit>0) {

          paym_tot[i]+=c_shft.paym_amnt[i];

          grand_tot  +=((c_shft.paym_amnt[i]*paym.curr_standard)/paym.curr_rate);

          paym_tot_local[i]+=((c_shft.paym_amnt[i]*paym.curr_standard)/paym.curr_rate); /* 27-April-2012 acm - */

          //V3.4.7 acm -  { restar la donacion del medio de pago soles
          if (i==PAYM_WAY_1){
             paym_tot[i]-=fix_donation; 
             grand_tot  -=fix_donation;
             
             fix_donation=0;
          }
          //V3.4.7 acm -  }  

          nbr++;
        }
      }
    }
  } while (tm_next(TM_SHFT_NAME, (void*)&c_shft)>=0);

  nbr = 0;
  for (i=PAYM_WAY_1; i<=MAX_PAYM_WAYS; i++) {
    if (i==MAX_PAYM_WAYS) {
      paym.paym_cd=0;
    }
    else {
      paym.paym_cd=i;
    }
    get_paym(&paym);
    if (paym.paym_limit>0) {
      ftoa_price(paym_tot[paym.paym_cd], TOTAL_BUF_SIZE, dummy);
      format_string(&string_price15, dummy);
      
      /* 27-April-2012 acm -{ add */
      _tcscpy(first_next_buffer, string_price15.result);
      _stprintf(buffer, prn_xr_TXT[23], paym.paym_descr, first_next_buffer);
      print_ln(printer, buffer);

      if ((i==2/*EFECTIVO US$*/) && (flag_paym_totals_per_cshr==EV_PRINTX))
      {
        ftoa_price(paym_tot_local[paym.paym_cd], TOTAL_BUF_SIZE, dummy);
        format_string(&string_price15, dummy);

      //_stprintf(buffer, prn_xr_TXT[23], "   EQUIV S/.", string_price15.result);
        _stprintf(buffer, prn_xr_TXT[23], " - EQUIV S/.", string_price15.result);
        print_ln (printer, buffer);     
      }

      if (nbr < 5) {
        _tcscpy(first, first_next_buffer);
        ch_memset(first, _T(' '), sizeof(first));
        nbr++;
      }
      else {
        _tcscpy(next, first_next_buffer);
		    ch_memset(next, _T(' '), sizeof(next));
      }
      /* 27-April-2012 acm -}*/

      /* 27-April-2012 acm - remove
	   if (nbr < 5) {
        _tcscpy(first, first_next_buffer);
		    _stprintf(buffer, prn_xr_TXT[23], paym.paym_descr, first);
        print_ln(printer, buffer);
        ch_memset(first, _T(' '), sizeof(first));
        nbr++;
      }
      else {
        _tcscpy(next, string_price15.result);
		    _stprintf(buffer, prn_xr_TXT[23], paym.paym_descr, next);
        print_ln(printer, buffer);             
		    ch_memset(next, _T(' '), sizeof(next));
      }
      */
     /* if (nbr < 5) {
        _tcscat(first, string_price15.result);
      }
      else {
        _tcscat(next,  string_price15.result);
      }*/
      /*nbr++;*/ /* 27-April-2012 acm - remove command because it is not necesary */
    }
  }

  ftoa_price(grand_tot, TOTAL_BUF_SIZE, dummy);
  format_string(&string_price15, dummy);
  _tcscpy(cshr_tot, string_price15.result);

  flag_paym_totals_per_cshr=0; /* 27-April-2012 acm - */

  return (SUCCEED);
} /* paym_amnts_tot_cshr */


/*****************************************************/

short paym_amnts_tot_cshr(_TCHAR *first, _TCHAR *next, _TCHAR *cshr_tot)
{
  PAYMENT_DEF paym;
  _TCHAR   dummy[TOTAL_BUF_SIZE+1];

  double fix_donation;//V3.4.7 acm -

  double paym_tot[MAX_PAYM_WAYS];
  double grand_tot;
  short  nbr, i, old_cshr;

  _tcscpy(first, _T(" "));
  _tcscpy(next,  _T(" "));
  grand_tot=0.0;
  for (i=0; i<MAX_PAYM_WAYS; i++) {
    paym_tot[i]=0.0;
  }
  old_cshr=c_shft.cashier;

  do {
    if (c_shft.cashier == old_cshr) {
      nbr = 0;
      fix_donation = c_shft.donation; //V3.4.7 acm -
      for (i=PAYM_WAY_0; i<=PAYM_WAY_9; i++) {
        paym.paym_cd = i;
        get_paym(&paym);
        if (paym.paym_limit>0) {
          paym_tot[i]+=c_shft.paym_amnt[i];
          grand_tot  +=((c_shft.paym_amnt[i]*paym.curr_standard)/paym.curr_rate);

          //V3.4.7 acm -  { restar la donacion del medio de pago soles
          if (i==PAYM_WAY_1){
             paym_tot[i]-=fix_donation; 
             grand_tot  -=fix_donation;
             
             fix_donation=0;
          }
          //V3.4.7 acm -  }  


          nbr++;
        }
      }
    }
  } while (tm_next(TM_SHFT_NAME, (void*)&c_shft)>=0);

  nbr = 0;
  for (i=PAYM_WAY_1; i<=MAX_PAYM_WAYS; i++) {
    if (i==MAX_PAYM_WAYS) {
      paym.paym_cd=0;
    }
    else {
      paym.paym_cd=i;
    }
    get_paym(&paym);
    if (paym.paym_limit>0) {
      ftoa_price(paym_tot[paym.paym_cd], TOTAL_BUF_SIZE, dummy);
      format_string(&string_price15, dummy);
      if (nbr < 5) {
        _tcscat(first, string_price15.result);
      }
      else {
        _tcscat(next,  string_price15.result);
      }
      nbr++;
    }
  }

  ftoa_price(grand_tot, TOTAL_BUF_SIZE, dummy);
  format_string(&string_price15, dummy);
  _tcscpy(cshr_tot, string_price15.result);

  return (SUCCEED);
} 

/* paym_amnts_tot_cshr */


/*----------------------------------------------------------------------------*/
/*                           paym_amnts_tot_till                              */
/*----------------------------------------------------------------------------*/
/* Composes two strings (first and next) with the allowed payment amounts     */
/* totals per till.                                                           */
/* Max. 10 payment ways are used.                                             */
/*----------------------------------------------------------------------------*/
short paym_amnts_tot_till_TR(_TCHAR *first, _TCHAR *next, _TCHAR *till_tot,short printer)  /*FUNCION MODIFICADA*/
{
  PAYMENT_DEF paym;
  _TCHAR   dummy[TOTAL_BUF_SIZE+1];
  short  nbr=0, i;
  double fix_donation;//V3.4.7 acm -

  double paym_tot[MAX_PAYM_WAYS], grand_tot;
  _TCHAR  buffer[SMALL_INV_SIZE+LEFT_MARGIN+1];


  grand_tot=0.0;
  for (i=0; i<MAX_PAYM_WAYS; i++) {
    paym_tot[i]=0.0;
  }

/********************/
	print_ln(printer, empty);
	print_ln(printer, empty);
	print_ln(printer, prn_xr_TXT[36]);
/********************/


  tm_frst(TM_SHFT_NAME, (void*)&c_shft);
  do {
    nbr = 0;  //V3.4.7 acm - fix
    fix_donation = c_shft.donation; //V3.4.7 acm -
    for (i=PAYM_WAY_0; i<=PAYM_WAY_9; i++) {
      paym.paym_cd = i;
      get_paym(&paym);
      if (paym.paym_limit>0) {
        paym_tot[i]+=c_shft.paym_amnt[i];
        grand_tot  +=((c_shft.paym_amnt[i]*paym.curr_standard)/paym.curr_rate);

          //V3.4.7 acm -  { restar la donacion del medio de pago soles
          if (i==PAYM_WAY_1){
             paym_tot[i]-=fix_donation; 
             grand_tot  -=fix_donation;
             
             fix_donation=0;
          }
          //V3.4.7 acm -  }  

        nbr++;
      }
    }
  } while (tm_next(TM_SHFT_NAME, (void*)&c_shft)>=0);
 
  for (i=0; i<MAX_PAYM_WAYS; i++) {
    c_day.paym_amnt[i]=paym_tot[i];
  }

  nbr = 0;
  _tcscpy(first, _T(" "));
  _tcscpy(next,  _T(" "));
  for (i=PAYM_WAY_1; i<=MAX_PAYM_WAYS; i++) {
    if (i == MAX_PAYM_WAYS) {
      paym.paym_cd=0;
    }
    else {
      paym.paym_cd=i;
    }
    get_paym(&paym);
    if (paym.paym_limit > 0) {
      ftoa_price(paym_tot[paym.paym_cd], TOTAL_BUF_SIZE, dummy);
      format_string(&string_price15, dummy);
      /*if (nbr < 5) {
        _tcscat(first, string_price15.result);
      }
      else {
        _tcscat(next,  string_price15.result);
      }*/

/*****************************/
     if (nbr < 5) {
        _tcscpy(first, string_price15.result);
		/****/
		_stprintf(buffer, prn_xr_TXT[23], paym.paym_descr, first);
        print_ln(printer, buffer);
		ch_memset(first, _T(' '), sizeof(first));
		/****/
        nbr++;
      }
      else {
        _tcscpy(next, string_price15.result);
		/****/
		_stprintf(buffer, prn_xr_TXT[23], paym.paym_descr, next);
        print_ln(printer, buffer);
		ch_memset(next, _T(' '), sizeof(next));
		/****/
      }

/*******************************/

      nbr++;
    }
  } /* end for loop */

  ftoa_price(grand_tot, TOTAL_BUF_SIZE, dummy);
  format_string(&string_price15, dummy);
  _tcscpy(till_tot, string_price15.result);



  return (SUCCEED);
} /* paym_amnts_tot_till */


/*----------------------------------------------------------------------------*/
/*                           fn_xr_zr_time_header                             */
/*----------------------------------------------------------------------------*/
/* Prints a header.                                                           */
/*----------------------------------------------------------------------------*/
short fn_xr_zr_time_header(PRN_OBJECT *p_object, short action,
                           short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  buffer[0]= _T('\0'); // 12-Jul-2011 acm - fix bug. initialization

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=2;
    }
  }

  if (action == PRINT_IT) {
/*    _stprintf(buffer, prn_xr_TXT[10]);  comentado JCP tirilla z y x*/
    add_left_margin(buffer);
   /* print_ln(printer, buffer);*/
 /*   _stprintf(buffer, prn_xr_TXT[11], prn_xr_TXT[6]); comentado JCP tirilla z y x*/
    add_left_margin(buffer);
    /*print_ln(printer, buffer);*/
  }

  return (SUCCEED);
} /* fn_xr_zr_time_header */


/*----------------------------------------------------------------------------*/
/*                           fn_xr_zr_shift_per_time                          */
/*----------------------------------------------------------------------------*/
/* Prints shift information per shift ('time from' until 'time to', etc.).    */
/*----------------------------------------------------------------------------*/

short fn_xr_zr_shift_per_time(PRN_OBJECT *p_object, short action,
                              short offs_grp, short printer)
{
  _TCHAR   buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR   dummy[TOTAL_BUF_SIZE+1];

  _TCHAR   time_on[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR   time_off[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR   buffer1[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR   buffer2[INV_SIZE+LEFT_MARGIN+1];

  int ix;
  double result;
  int temp=0;

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }

  if (action == PRINT_IT) {
    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');

    ftoa((double)c_shft.cashier, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 0, 2, dummy);
	
	/*******************/
    print_ln(printer, empty);
	_stprintf(buffer1, prn_xr_TXT[22], c_shft.cashier);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/
    
	ftoa((double)c_shft.invoice_on, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 16, 21, dummy);

    ftoa((double)c_shft.invoice_off, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 23, 28, dummy);

	/*******************/
	_stprintf(buffer1, prn_xr_TXT[28], c_shft.invoice_on,c_shft.invoice_off);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/
    

    ftoa((double)c_shft.nbr_inv_lines, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 36, 40, dummy);

    ftoa((double)c_shft.nbr_void_inv, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 42, 45, dummy);

	/*******************/
/*	_stprintf(buffer1, prn_xr_TXT[29], c_shft.nbr_inv_lines,c_shft.nbr_void_inv);*/
    _stprintf(buffer1, prn_xr_TXT[29], (c_shft.invoice_off-c_shft.invoice_on)-c_shft.nbr_void_inv,c_shft.nbr_inv_lines);	
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/

    ftoa((double)c_shft.nbr_void_lines, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 47, 51, dummy);

    if (c_shft.wallet_no[0] != WALLET_NOT_USED) {
      ftoa((double)c_shft.wallet_no[0], TOTAL_BUF_SIZE, dummy);
      fmt_right_justify_string(buffer, 53, 55, dummy);
	  _stprintf(buffer1, prn_xr_TXT[30], c_shft.nbr_void_lines,c_shft.wallet_no[0]);
	  temp=1;
    }
    if (c_shft.wallet_no[1] != WALLET_NOT_USED) {
      ftoa((double)c_shft.wallet_no[1], TOTAL_BUF_SIZE, dummy);
      fmt_right_justify_string(buffer, 57, 59, dummy);
	  _stprintf(buffer1, prn_xr_TXT[30], c_shft.nbr_void_lines,c_shft.wallet_no[1]);
	  temp=1;
    }
    if (c_shft.wallet_no[2] != WALLET_NOT_USED) {
      ftoa((double)c_shft.wallet_no[2], TOTAL_BUF_SIZE, dummy);
      fmt_right_justify_string(buffer, 61, 63, dummy);
	  _stprintf(buffer1, prn_xr_TXT[30], c_shft.nbr_void_lines,c_shft.wallet_no[2]);
	  temp=1;
    }

	/*******************/
	if (temp == 0)
	{
		_stprintf(buffer1, prn_xr_TXT[30], c_shft.nbr_void_lines,0);
	}

	print_ln(printer, buffer1);
	print_ln(printer, prn_xr_TXT[31]);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/

    result=c_shft.invoice_off-c_shft.invoice_on;
    if (result<0) {
      result+=1000;
    }
    result-=c_shft.nbr_void_inv;
    ftoa(result , TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 30, 34, dummy);

    ftoa((double)c_shft.time_on, 6, dummy);
    format_string(&string_time5, dummy);
    fmt_right_justify_string(time_on, 4, 8, string_time5.result);

    /*Hora de apertura de caja*/
	_stprintf(buffer1, prn_xr_TXT[26],string_time5.result ,time_off);
    /*******************/

    if (c_shft.time_off != 9999) {
      ftoa((double)c_shft.time_off, 6, dummy);
      format_string(&string_time5, dummy);
      fmt_right_justify_string(time_off, 10, 14, string_time5.result);
    }

	/*hora de cierre de caja*/
	_stprintf(buffer2, prn_xr_TXT[27],string_time5.result ,time_off);
	_tcscat(buffer1, buffer2);
    print_ln(printer, buffer1);
    ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
    ch_memset(buffer2, _T(' '), INV_SIZE*sizeof(_TCHAR));


    ftoa_price(c_shft.start_float, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price14, dummy);
    fmt_right_justify_string(buffer, 67, 80, string_price14.result);

	/*******************/
	_stprintf(buffer1, prn_xr_TXT[32], string_price14.result);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/
    

    if (c_shft.lift_refill_float == 0.0) {
      _tcscpy(dummy, _T("0"));
    }
    else {
      ftoa_price(c_shft.lift_refill_float, TOTAL_BUF_SIZE, dummy);
    }
    format_string(&string_price14, dummy);
    fmt_right_justify_string(buffer, 82, 95, string_price14.result);

    
	/*******************/
	_stprintf(buffer1, prn_xr_TXT[33], string_price14.result);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/


    ftoa_price(c_shft.donation, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price14, dummy);
    fmt_right_justify_string(buffer, 97, 110, string_price14.result);
    
	/*******************/
	_stprintf(buffer1, prn_xr_TXT[34], string_price14.result);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/

  /* V3.4.7 acm -{*/

	  _stprintf(buffer1, prn_xr_TXT[38], c_shft.rounded);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* V3.4.7 acm -}*/

  /* v3.6.1 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[42], c_shft.percep_amount);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.6.1 acm -}*/


  /* AC2012-003 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[37], c_shft.cupon);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* AC2012-003 acm -}*/

  /* v3.5.0 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[39], c_shft.cupon_cine);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.5.0 acm -}*/


  /* v3.5.1 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[40], c_shft.vale_pavo);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.5.1 acm -}*/


  /* v3.6 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[41], c_shft.feria_escolar);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.6 acm -}*/


  /* v3.5.0 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[43], c_shft.fiesta_futbol);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.5.0 acm -}*/
     
      for (ix=0;ix< CUPON_GLOBAL_MAX; ix++) 
      {
          if (cupon_global_vigente(ix)) 
          {
              _stprintf(buffer1, cupon_global_xr_TXT[ix], c_shft.cupon_global[ix]);
              print_ln(printer, buffer1);
              ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
          }
      }
    /*add_left_margin(buffer);*/
    /*print_ln(printer, buffer);*/
  }

  return (SUCCEED);
} /* fn_xr_zr_shift_per_time_TR */


/*********************************************************************/

short fn_xr_zr_shift_per_time_GR(PRN_OBJECT *p_object, short action,
                              short offs_grp, short printer)      /*funcion original*/
{
  _TCHAR   buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR   dummy[TOTAL_BUF_SIZE+1];

  double result;

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }

  if (action == PRINT_IT) {
    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');

    ftoa((double)c_shft.cashier, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 0, 2, dummy);

    ftoa((double)c_shft.invoice_on, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 16, 21, dummy);

    ftoa((double)c_shft.invoice_off, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 23, 28, dummy);

    ftoa((double)c_shft.nbr_inv_lines, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 36, 40, dummy);

    ftoa((double)c_shft.nbr_void_inv, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 42, 45, dummy);

    ftoa((double)c_shft.nbr_void_lines, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 47, 51, dummy);

    if (c_shft.wallet_no[0] != WALLET_NOT_USED) {
      ftoa((double)c_shft.wallet_no[0], TOTAL_BUF_SIZE, dummy);
      fmt_right_justify_string(buffer, 53, 55, dummy);
    }
    if (c_shft.wallet_no[1] != WALLET_NOT_USED) {
      ftoa((double)c_shft.wallet_no[1], TOTAL_BUF_SIZE, dummy);
      fmt_right_justify_string(buffer, 57, 59, dummy);
    }
    if (c_shft.wallet_no[2] != WALLET_NOT_USED) {
      ftoa((double)c_shft.wallet_no[2], TOTAL_BUF_SIZE, dummy);
      fmt_right_justify_string(buffer, 61, 63, dummy);
    }

    result=c_shft.invoice_off-c_shft.invoice_on;
    if (result<0) {
      result+=1000;
    }
    result-=c_shft.nbr_void_inv;
    ftoa(result , TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 30, 34, dummy);

    ftoa((double)c_shft.time_on, 6, dummy);
    format_string(&string_time5, dummy);
    fmt_right_justify_string(buffer, 4, 8, string_time5.result);

    if (c_shft.time_off != 9999) {
      ftoa((double)c_shft.time_off, 6, dummy);
      format_string(&string_time5, dummy);
      fmt_right_justify_string(buffer, 10, 14, string_time5.result);
    }

    ftoa_price(c_shft.start_float, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price14, dummy);
    fmt_right_justify_string(buffer, 67, 80, string_price14.result);

    if (c_shft.lift_refill_float == 0.0) {
      _tcscpy(dummy, _T("0"));
    }
    else {
      ftoa_price(c_shft.lift_refill_float, TOTAL_BUF_SIZE, dummy);
    }
    format_string(&string_price14, dummy);
    fmt_right_justify_string(buffer, 82, 95, string_price14.result);

    ftoa_price(c_shft.donation, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price14, dummy);
    fmt_right_justify_string(buffer, 97, 110, string_price14.result);
    
    add_left_margin(buffer);
    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_xr_zr_shift_per_time */


/*----------------------------------------------------------------------------*/
/*                           fn_xr_till_totals                                */
/*----------------------------------------------------------------------------*/
/* Prints final totals per till for the X-read.                               */
/*----------------------------------------------------------------------------*/
short fn_xr_till_totals(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  buffer1[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  dummy[TOTAL_BUF_SIZE+1];
  short   cur_cshr;
  long    tot_inv, tot_inv_ln, tot_void_inv, tot_void_ln, tmp_l;
  double  tot_donation;
  long    tot_cupon=0;   /*AC2012-003 acm -*/
  double  tot_rounded=0;   /*v3.4.7 acm -*/
  long    tot_cupon_cine = 0;
  long    tot_vale_pavo = 0;
  long    tot_feria_escolar=0;   /*v3.6.0 acm -*/
  double  tot_perception=0;   /*v3.6.1 acm -*/
  long    tot_fiesta_futbol=0;
  int ix;
  long    tot_cupon_global[CUPON_GLOBAL_MAX];

  for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
      tot_cupon_global[ix]=0;

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=2;
    }
  }

  if (action == PRINT_IT) {
    ch_memset(buffer,  _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');
    tot_inv      = 0L;
    tot_void_inv = 0;
    tot_void_ln  = 0;
    tot_inv_ln   = 0L;
    tot_donation = 0.0;
    tot_cupon    = 0.0;   /*AC2012-003 acm -*/
    tot_rounded  = 0.0; //v3.4.7 acm - 
    tot_cupon_cine =0.0; //v3.5 acm -
    tot_vale_pavo  =0.0; //v3.5 acm -
    tot_feria_escolar  =0.0; //v3.6.0 acm -
    tot_perception     =0.0; //v3.6.1 acm -
    tot_fiesta_futbol =0.0; //v3.5 acm -

    for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
        tot_cupon_global[ix]=0;
    

    cur_cshr=c_shft.cashier;
    do {
      if (c_shft.cashier==cur_cshr) {
        tmp_l=c_shft.invoice_off-c_shft.invoice_on;
        if (tmp_l < 0L) {
          tmp_l+=1000L;
        }

        tot_inv     +=tmp_l;
        tot_inv_ln  +=c_shft.nbr_inv_lines;
        tot_void_inv+=c_shft.nbr_void_inv;
        tot_void_ln +=c_shft.nbr_void_lines;
        tot_donation+=c_shft.donation;
        
        tot_cupon   +=c_shft.cupon;   /*AC2012-003 acm -*/
        tot_rounded +=c_shft.rounded; // v3.4.7 acm -
		
        tot_cupon_cine  +=c_shft.cupon_cine;   /*v3.5 acm -*/		
        tot_vale_pavo   +=c_shft.vale_pavo;   /*v3.5 acm -*/
        tot_feria_escolar+=c_shft.feria_escolar;

        tot_perception+=c_shft.percep_amount; //v3.6.1 acm -

        tot_fiesta_futbol  +=c_shft.fiesta_futbol;   /*v3.5 acm -*/		

        for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
        {
           tot_cupon_global[ix] +=c_shft.cupon_global[ix];
        }
      }
    } while (tm_next(TM_SHFT_NAME, (void*)&c_shft)>=0);

	print_ln(printer, empty);
	print_ln(printer, empty);
	print_ln(printer, prn_xr_TXT[25]);

    /*fmt_right_justify_string(buffer,  0,  4, prn_xr_TXT[13]);*/

    ftoa((double)tot_inv-tot_void_inv, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 30, 34, dummy);


    ftoa((double)tot_inv_ln, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 36, 40, dummy);

	/*******************/
	_stprintf(buffer1, prn_xr_TXT[29], tot_inv-tot_void_inv,tot_inv_ln);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/

    ftoa((double)tot_void_inv, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 42, 45, dummy);

    ftoa((double)tot_void_ln, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 47, 51, dummy);


    if(tot_void_ln < 0)
	{
		tot_void_ln=0.0;
	}

	/*******************/
	_stprintf(buffer1, prn_xr_TXT[30], tot_void_inv,tot_void_ln);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/

    ftoa(tot_donation*100, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price14, dummy);
    fmt_right_justify_string(buffer, 97, 110, string_price14.result);


	/*******************/
	_stprintf(buffer1, prn_xr_TXT[34], string_price14.result);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/

  /* v3.4.7 acm -{*/

	 _stprintf(buffer1, prn_xr_TXT[38], tot_rounded);
	 print_ln(printer, buffer1);
	 ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.4.7 acm -}*/

  /* v3.6.1 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[42], tot_perception);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.6.1 acm -}*/


  /* AC2012-003 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[37], tot_cupon);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* AC2012-003 acm -}*/

  /* v3.5.0 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[39], tot_cupon_cine);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.5.0 acm -}*/

  /* v3.5.0 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[40], tot_vale_pavo);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.5.0 acm -}*/

  /* v3.6.0 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[41], tot_feria_escolar);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.6.0 acm -}*/

      
  /* v3.6.0 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[43], tot_fiesta_futbol);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.6.0 acm -}*/

      for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
      {  
         if (cupon_global_vigente(ix)) 
         {  
              _stprintf(buffer1, cupon_global_xr_TXT[ix], tot_cupon_global[ix]);
              print_ln(printer, buffer1);
              ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
          }  
      }  
      
    /*add_left_margin(buffer);*/
    /*print_ln(printer, buffer);*/
     ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    //_stprintf(buffer, _T("%20.20s  %02dT%09ld"), prn_xr_TXT[21], pos_system.store_no, pos_system.small_inv_seq);
    /*add_left_margin(buffer);*/
    _stprintf(buffer, _T("%17.17s  %s-%ld"), prn_xr_TXT[44], pos_system.last_serie_fac, pos_system.last_corr_fac);
    print_ln(printer, buffer);
    _stprintf(buffer, _T("%17.17s  %s-%ld"), prn_xr_TXT[45], pos_system.last_serie_bol, pos_system.last_corr_bol);
    print_ln(printer, buffer);
	
	  print_ln(printer, empty);
	  print_ln(printer, empty);

	  print_cut(printer);
  }

  return (SUCCEED);
} /* fn_xr_till_totals */
/******************************************************************************/

short fn_xr_till_totals_TR(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  dummy[TOTAL_BUF_SIZE+1];
  short   cur_cshr;
  long    tot_inv, tot_inv_ln, tot_void_inv, tot_void_ln, tmp_l;
  double  tot_donation;

  long    tot_cupon; /*AC2012-003 acm -*/
  double  tot_rounded; //v3.4.7 acm -
  long    tot_cupon_cine; /*v3.5 acm -*/
  long    tot_vale_pavo; /*v3.5 acm -*/
  long    tot_feria_escolar; /*v3.6.0 acm -*/
  double  tot_perception =0; //v3.6.1 acm -
  long    tot_fiesta_futbol=0;

  int ix;
  long    tot_cupon_global[CUPON_GLOBAL_MAX];

  for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
      tot_cupon_global[ix]=0;

 
  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=2;
    }
  }

  if (action == PRINT_IT) {
    ch_memset(buffer,  _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');
    tot_inv      = 0L;
    tot_void_inv = 0;
    tot_void_ln  = 0;
    tot_inv_ln   = 0L;
    tot_donation = 0.0;
    tot_cupon    = 0.0; /*AC2012-003 acm -*/
    tot_rounded  = 0.0;  //v3.4.7 acm -
    tot_cupon_cine = 0.0; /*v3.5 acm -*/
    tot_vale_pavo  = 0.0; /*v3.5 acm -*/
    tot_feria_escolar  = 0.0; /*v3.6.0 acm -*/
    tot_perception     = 0.0;
    tot_fiesta_futbol  =0;

    for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
        tot_cupon_global[ix]=0;


    cur_cshr           = c_shft.cashier;
    do {
      if (c_shft.cashier==cur_cshr) {
        tmp_l=c_shft.invoice_off-c_shft.invoice_on;
        if (tmp_l < 0L) {
          tmp_l+=1000L;
        }

        tot_inv     +=tmp_l;
        tot_inv_ln  +=c_shft.nbr_inv_lines;
        tot_void_inv+=c_shft.nbr_void_inv;
        tot_void_ln +=c_shft.nbr_void_lines;
        tot_donation+=c_shft.donation;

        tot_cupon   +=c_shft.cupon; /*AC2012-003 acm -*/
        tot_rounded +=c_shft.rounded; /*v3.4.7 acm -*/
		
        tot_cupon_cine   +=c_shft.cupon_cine; /*AC2012-003 acm -*/		
        tot_vale_pavo    +=c_shft.vale_pavo; /*AC2012-003 acm -*/		
        tot_feria_escolar+=c_shft.feria_escolar; /*AC2012-003 acm -*/		

        tot_perception   +=c_shft.percep_amount; /*v3.6.1 acm -*/

        tot_fiesta_futbol+=c_shft.fiesta_futbol; /*AC2012-003 acm -*/		

        for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
        {
           tot_cupon_global[ix] +=c_shft.cupon_global[ix];
        }
      }
    } while (tm_next(TM_SHFT_NAME, (void*)&c_shft)>=0);


    fmt_right_justify_string(buffer,  0,  4, prn_xr_TXT[13]);

    ftoa((double)tot_inv-tot_void_inv, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 30, 34, dummy);

    ftoa((double)tot_inv_ln, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 36, 40, dummy);

    ftoa((double)tot_void_inv, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 42, 45, dummy);

    ftoa((double)tot_void_ln, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 47, 51, dummy);

    ftoa(tot_donation, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price14, dummy);
    fmt_right_justify_string(buffer, 97, 110, string_price14.result);

    add_left_margin(buffer);
    print_ln(printer, buffer);
    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    //_stprintf(buffer, _T("%20.20s  %02dT%09ld"), prn_xr_TXT[21], pos_system.store_no, pos_system.small_inv_seq);
    /*add_left_margin(buffer);*/
    _stprintf(buffer, _T("%17.17s  %s-%ld"), prn_xr_TXT[44], pos_system.last_serie_fac, pos_system.last_corr_fac);
    print_ln(printer, buffer);
    _stprintf(buffer, _T("%17.17s  %s-%ld"), prn_xr_TXT[45], pos_system.last_serie_bol, pos_system.last_corr_bol);
    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_xr_till_totals */




/*----------------------------------------------------------------------------*/
/*                           fn_zr_till_totals                                */
/*----------------------------------------------------------------------------*/
/* Prints final totals per till for the Z-read.                               */
/*----------------------------------------------------------------------------*/
short fn_zr_till_totals(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  _TCHAR  dummy[TOTAL_BUF_SIZE+1];
  _TCHAR  buffer[INV_SIZE+LEFT_MARGIN+1];
  _TCHAR  buffer1[INV_SIZE+LEFT_MARGIN+1];
  TM_INDX shft_indx;
  short   first_time,   tot_void_inv;
  long    first_inv_on, last_inv_off;
  long    tot_void_ln, tot_inv_ln, tmp_l;
  double  tot_donation;
  long    tot_cupon;   /*AC2012-003 acm -*/
  double  tot_rounded; //v3.4.7 acm -
  
  long    tot_cupon_cine;   /*v3.5 acm -*/
  long    tot_vale_pavo;   /*v3.5 acm -*/
  long    tot_feria_escolar;   /*v3.5 acm -*/
  double  tot_perception; //v3.6.1 acm -
  long    tot_fiesta_futbol=0;

  int ix;
  long    tot_cupon_global[CUPON_GLOBAL_MAX];

  for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
      tot_cupon_global[ix]=0;


  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)==PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=2;
    }
  }

  if (action == PRINT_IT) {
    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE] = _T('\0');

/***************************/
	
  	print_ln(printer, empty);
	print_ln(printer, empty);
	print_ln(printer, prn_xr_TXT[36]);

/***************************/
    first_time   = TRUE;
    tot_void_inv = 0;
    tot_inv_ln   = 0L;
    tot_void_ln  = 0L;
    tot_donation = 0.0;
    tot_cupon    = 0.0; /*AC2012-003 acm -*/
    tot_rounded  = 0.0; //v3.4.7 acm - 
    tot_cupon_cine  = 0.0; /*v3.5 acm -*/	
    tot_vale_pavo   = 0.0; /*v3.5 acm -*/	
    tot_feria_escolar = 0.0; /*v3.5 acm -*/	

    
    tot_perception    = 0.0;
    tot_fiesta_futbol = 0;

    for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
        tot_cupon_global[ix]=0;

 
    shft_indx=tm_frst(TM_SHFT_NAME, (void*)&c_shft);
    while (shft_indx>=0) {
      if (first_time==TRUE) {
        first_inv_on=c_shft.invoice_on;
        c_day.invoice_on=c_shft.invoice_on;
        c_day.date_on=c_shft.date_on;
        c_day.time_on=c_shft.time_on;
        c_day.till_no=c_shft.till_no;
        c_day.cashier=0;
        first_time=FALSE;
      }
      c_day.time_off=c_shft.time_off;
      c_day.shift_no++;
      last_inv_off =c_shft.invoice_off;
      tot_inv_ln  +=(long)c_shft.nbr_inv_lines;
      tot_void_inv+=c_shft.nbr_void_inv;
      tot_void_ln +=c_shft.nbr_void_lines;
      tot_donation+=c_shft.donation;

      tot_cupon   +=c_shft.cupon; /*AC2012-003 acm -*/
      tot_rounded +=c_shft.rounded; //v3.4.7 acm -
	  
      tot_cupon_cine   +=c_shft.cupon_cine; /*v3.5 acm -*/	  
      tot_vale_pavo    +=c_shft.vale_pavo; /*v3.5 acm -*/	  
      tot_feria_escolar+=c_shft.feria_escolar; /*v3.6.0 acm -*/	  
	  
      tot_perception   +=c_shft.percep_amount; //v3.6.1 acm - 

      tot_fiesta_futbol+=c_shft.fiesta_futbol; /*v3.5 acm -*/	  

      for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
      {
          tot_cupon_global[ix] +=c_shft.cupon_global[ix];
      }
      shft_indx=tm_next(TM_SHFT_NAME, (void*)&c_shft);
    }

    c_day.invoice_off=last_inv_off;
    c_day.nbr_inv_lines=tot_inv_ln;
    c_day.nbr_void_inv=tot_void_inv;
    c_day.nbr_void_lines=tot_void_ln;
    c_day.donation=tot_donation;
    c_day.rounded =tot_rounded;

    c_day.percep_amount =tot_perception; //v3.6.1 acm -



    fmt_right_justify_string(buffer,  0,  4, prn_xr_TXT[13]);

    ftoa(first_inv_on, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 16, 21, dummy);

    ftoa(last_inv_off, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 23, 28, dummy);

    tmp_l=last_inv_off-first_inv_on;
    if (tmp_l < 0L) {
      tmp_l+=1000L;
    }
    tmp_l-=tot_void_inv;

    ftoa(tmp_l, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 30, 34, dummy);

    ftoa(tot_inv_ln, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 36, 40, dummy);


	/*******************/
	_stprintf(buffer1, prn_xr_TXT[29], tmp_l,tot_inv_ln);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/

    ftoa(tot_void_inv, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 42, 45, dummy);

    ftoa(tot_void_ln, TOTAL_BUF_SIZE, dummy);
    fmt_right_justify_string(buffer, 47, 51, dummy);

    if(tot_void_ln < 0)
	{
		tot_void_ln=0.0;
	}
	/*******************/
	_stprintf(buffer1, prn_xr_TXT[30], tot_void_inv,tot_void_ln);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/


    ftoa(tot_donation*100, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price14, dummy);
    fmt_right_justify_string(buffer, 97, 110, string_price14.result);

	/*********DONATION**********/
	_stprintf(buffer1, prn_xr_TXT[34], string_price14.result);
	print_ln(printer, buffer1);
	ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
	/*******************/

  /* v3.4.7 acm -{*/

	  _stprintf(buffer1, prn_xr_TXT[38], tot_rounded);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.4.7 acm -}*/

  /* v3.6.1 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[42], tot_perception);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.6.1 acm -}*/


  /* AC2012-003 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[37], tot_cupon);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* AC2012-003 acm -}*/

  /* v3.5 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[39], tot_cupon_cine);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.5 acm -}*/


  /* v3.5 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[40], tot_vale_pavo);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.5 acm -}*/

  /* v3.6.0 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[41], tot_feria_escolar);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.6.0 acm -}*/

  /* v3.6.0 acm -{*/
	  _stprintf(buffer1, prn_xr_TXT[43], tot_fiesta_futbol);
	  print_ln(printer, buffer1);
	  ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
  /* v3.6.0 acm -}*/

      for (ix=0; ix<CUPON_GLOBAL_MAX; ix++) 
      {  
         if (cupon_global_vigente(ix)) 
         {  
              _stprintf(buffer1, cupon_global_xr_TXT[ix], tot_cupon_global[ix]);
              print_ln(printer, buffer1);
              ch_memset(buffer1, _T(' '), INV_SIZE*sizeof(_TCHAR));
          }  
      }  

    /*add_left_margin(buffer);*/
   /* print_ln(printer, buffer);*/  /*JCP X Z*/
    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    //_stprintf(buffer, _T("%20.20s  %02dT%09ld"), prn_xr_TXT[44], pos_system.store_no, pos_system.small_inv_seq);
    /*add_left_margin(buffer);*/
    _stprintf(buffer, _T("%17.17s  %s-%ld"), prn_xr_TXT[44], pos_system.last_serie_fac, pos_system.last_corr_fac);
    print_ln(printer, buffer);
    _stprintf(buffer, _T("%17.17s  %s-%ld"), prn_xr_TXT[45], pos_system.last_serie_bol, pos_system.last_corr_bol);
    print_ln(printer, buffer);

	  print_ln(printer, empty);

	  print_cut(printer);

  }

  return (SUCCEED);
} /* fn_zr_till_totals */


/*----------------------------------------------------------------------------*/
/*                            I  N  V  O  I  C  E                             */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                            fn_i_promo                                      */
/*----------------------------------------------------------------------------*/
/* Prints 4 lines of promotional text at the top of each invoice.             */
/*----------------------------------------------------------------------------*/
static short fn_i_promo(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=4;
    }
  }

  if (action == PRINT_IT) {
    while (on_ln_inv < START_LN_PROMO) {

#ifdef DEBUG
      print_ln(printer, _T("fn_i_promo"));
#else
      print_ln(printer, empty);
#endif
    }

    if (train_mode == CASH_NORMAL) {
      if (copy_invoice_active == YES) {
        print_ln(printer, prn_inv_TXT[26]);
        print_ln(printer, prn_inv_TXT[73]);
        print_ln(printer, prn_inv_TXT[26]);
        print_ln(printer, prn_inv_TXT[73]);
      }
      else {
        print_ln(printer, genvar.prom_txt_top1);
        print_ln(printer, genvar.prom_txt_top2);
        print_ln(printer, genvar.prom_txt_top3);
        print_ln(printer, genvar.prom_txt_top4);
      }
    }
    else {
      print_ln(printer, prn_inv_TXT[20]);
      print_ln(printer, prn_inv_TXT[72]);
      if (copy_invoice_active == YES) {
        print_ln(printer, prn_inv_TXT[26]);
        print_ln(printer, prn_inv_TXT[73]);
      }
      else {
        print_ln(printer, prn_inv_TXT[20]);
        print_ln(printer, prn_inv_TXT[72]);
      }
    }
  }

  return (SUCCEED);
} /* fn_i_promo */


/*----------------------------------------------------------------------------*/
/*                            fn_i_customer                                   */
/*----------------------------------------------------------------------------*/
/* Prints 3 lines of the customer header on the invoice.                      */
/* Imprime lo datos del cliente (la cabecera)                                 */
/*----------------------------------------------------------------------------*/
short fn_i_customer(PRN_OBJECT *p_object, short action,
                    short offs_grp, short printer)
{
  _TCHAR buffer[INV_SIZE+1];
  _TCHAR prn_date[12 +1];
  _TCHAR invoice_no[6 +1];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=3;
    }
  }

  if (action == PRINT_IT) {
    while (on_ln_inv < START_LN_CUST) {

#ifdef DEBUG
      print_ln(printer, _T("fn_i_customer"));
#else
      print_ln(printer, empty);
#endif
    }

    prn_fmt_date(pos_system.run_date, genvar.date_format, mon_names, prn_date);
    get_invoice_no(invoice_no);

    _itot(pos_invoice.invoice_time, buffer, DECIMAL);
    format_string(&string_time5, buffer);

    _stprintf(buffer,prn_inv_TXT[2], cust.name,
                                     pos_system.store_no,
                                     pos_invoice.invoice_till_no,
                                     pos_system.run_date/10000,     /* YEAR */
                                     invoice_no,
                                     pg_no++);
   
    print_ln(printer, buffer);
    _stprintf(buffer, prn_inv_TXT[3],  cust.fisc_no,
                                       pos_invoice.invoice_till_no,
                                       pos_invoice.invoice_cashier_no);
    print_ln(printer, buffer);

    _stprintf(buffer, prn_inv_TXT[4], cust.store_no, cust.cust_no,
                                      _T("  "),
                                      prn_date, string_time5.result);
    print_ln(printer, buffer);
  }
  return (SUCCEED);
} /* fn_i_customer */


/*----------------------------------------------------------------------------*/
/*                            fn_i_article_line                               */
/*----------------------------------------------------------------------------*/
/* Prints an article line each time an article is scanned.                    */
/* imprime una linea por producto (es el detalle de la factura)               */
/*----------------------------------------------------------------------------*/
short fn_i_article_line(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+1];
  _TCHAR  dummy[TOTAL_BUF_SIZE+1];

  _TCHAR  perc[7+1];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1+LN_TRANS_INV;
    }
  }

  if (action == PRINT_IT) {
    while (on_ln_inv < START_LN_ART) {

#ifdef DEBUG
      print_ln(printer, _T("fn_i_article_line"));
#else
      print_ln(printer, empty);
#endif
    }

    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');

    if (c_item.arti.art_ind==ART_IND_WEIGHT) {
      ftoa(c_item.arti.base.qty*1000, 9, dummy);
      format_string(&string_qty8, dummy);
      fmt_right_justify_string(buffer, 90, 97, string_qty8.result);
    }
    else {
      ftoa(c_item.arti.base.qty, 9, dummy);
      format_string(&string_qty5_star, dummy);
      fmt_right_justify_string(buffer, 96, 100, string_qty5_star.result); 
    }

    ftoa((double)c_item.arti.base.art_no, 12, dummy);
    fmt_right_justify_string(buffer, 2, 10, dummy);   /*codigo*/

    fmt_left_justify_string(buffer, 15, 73, c_item.arti.base.descr);  /*descripcion*/ 
    fmt_left_justify_string(buffer, 88, 89, c_item.arti.pack_type);   /*tipo de empaque*/

    *perc = _T('\0');
    fmt_vat_perc(c_item.arti.base.vat_perc, perc);
    fmt_right_justify_string(buffer, 104, 110, perc);    /*cantidad*/

    ftoa_price(c_item.arti.base.goods_value, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 117, 127    , string_price11_2.result);  /*valor total*/

    /* Price can be negative, but display allways positive!                  */
    if (c_item.arti.base.price >= 0) {
      ftoa_price(c_item.arti.base.price, TOTAL_BUF_SIZE, dummy);
    }
    else {
      ftoa_price(c_item.arti.base.price * -1, TOTAL_BUF_SIZE, dummy);
    }
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 77, 87, string_price11_2.result); /*Precio*/

    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_i_article_line */


/*----------------------------------------------------------------------------*/
/*                            fn_i_deposit_line                               */
/*----------------------------------------------------------------------------*/
/* Prints a deposit line (this line always belongs to the previous article.   */
/* NOTE: size of article description is decreased !                           */
/*----------------------------------------------------------------------------*/
short fn_i_deposit_line(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+1];
  _TCHAR  qty[TOTAL_BUF_SIZE +1];
  _TCHAR  dummy[TOTAL_BUF_SIZE +1];
  _TCHAR  dep_descr[DESCR_SIZE +1];
  
  _TCHAR  perc[7+1];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1+LN_TRANS_INV;
    }
  }

  if (action == PRINT_IT) {
    while (on_ln_inv < START_LN_ART) {

#ifdef DEBUG
      print_ln(printer, _T("fn_i_deposit_line"));
#else
      print_ln(printer, empty);
#endif
    }

    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');

    if (c_item.depo.art_ind==ART_IND_WEIGHT) {
      _tcscpy(qty, _T("1"));
    }
    else {
      ftoa(c_item.depo.base.qty, 9, qty);
    }

    ftoa((double)c_item.depo.base.art_no, 15, dummy);
    fmt_right_justify_string(buffer, 2, 10, dummy); 

    _tcscpy(dep_descr, c_item.depo.base.descr);
    format_string(&string_dep_qty6, qty);
    strcat_maxlen(dep_descr, string_dep_qty6.result, DESCR_SIZE);

    fmt_left_justify_string(buffer, 15, 73, dep_descr);
    fmt_left_justify_string(buffer, 88, 89, c_item.depo.pack_type);

    *perc = _T('\0');
    fmt_vat_perc(c_item.depo.base.vat_perc, perc);
    fmt_right_justify_string(buffer, 99, 105, perc);

    ftoa_price(c_item.depo.base.goods_value, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 112, 122, string_price11_2.result);

    /* Price can be negative, but display allways positive!                  */
    if (c_item.depo.base.price >= 0) {
      ftoa_price(c_item.depo.base.price, TOTAL_BUF_SIZE, dummy);
    }
    else {
      ftoa_price(c_item.depo.base.price * -1, TOTAL_BUF_SIZE, dummy);
    }
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 77, 87, string_price11_2.result); 

    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_i_deposit_line */


/*----------------------------------------------------------------------------*/
/*                            fn_i_discount_line                              */
/*----------------------------------------------------------------------------*/
/* Prints a discount line (this line always belongs to the previous article.  */
/*----------------------------------------------------------------------------*/
short fn_i_discount_line(PRN_OBJECT *p_object, short action,
                         short offs_grp, short printer)
{
  _TCHAR buffer[INV_SIZE+1];
  _TCHAR discnt_descr[INV_SIZE+1];
  _TCHAR qty[TOTAL_BUF_SIZE +1];
  _TCHAR dummy[TOTAL_BUF_SIZE +1];
  
  _TCHAR  perc[7+1];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1+LN_TRANS_INV;
    }
  }

  if (action == PRINT_IT) {
    while (on_ln_inv < START_LN_ART) {

#ifdef DEBUG
      print_ln(printer, _T("fn_i_discount_line"));
#else
      print_ln(printer, empty);
#endif
    }

    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');

    ftoa(c_item.disc.base.qty, TOTAL_BUF_SIZE, qty);
    _tcscpy(discnt_descr, c_item.disc.base.descr);
    format_string(&string_dep_qty6, qty);
    strcat_maxlen(discnt_descr, string_dep_qty6.result, DESCR_SIZE);
    fmt_left_justify_string(buffer, 15, 73, discnt_descr);

    *perc = _T('\0');
    fmt_vat_perc(c_item.disc.base.vat_perc, perc);
    fmt_right_justify_string(buffer, 99, 105, perc);

    ftoa_price(c_item.disc.base.goods_value, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 112, 122, string_price11_2.result);

    /* Price can be negative, but display allways positive!                  */
    if (c_item.disc.base.price>=0) {
      ftoa_price(c_item.disc.base.price, TOTAL_BUF_SIZE, dummy);
    }
    else {
      ftoa_price(c_item.disc.base.price * -1, TOTAL_BUF_SIZE, dummy);
    }
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 77, 87, string_price11_2.result);

    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_i_discount_line */


/*----------------------------------------------------------------------------*/
/*                           fn_i_total_lines                                 */
/*----------------------------------------------------------------------------*/
short fn_i_total_lines(PRN_OBJECT *p_object, short action,
                       short offs_grp, short printer)
{
  short i, lines_vat, lines_paym;

  lines_vat  = 1; /* Header line */
  lines_paym = 1; /* Header line */

  if (action == CALCULATE_IT) {

    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      for (i=TOT_EXCL_0; i<=TOT_EXCL_9; i++) {
        if (tot_ret_double(i) != 0.0) {
          lines_vat++;
        }
      }

      for (i = PAYM_WAY_0; i < MAX_PAYM_WAYS; i++) {
        if (tot_ret_double((short)(i-PAYM_WAY_0+TOT_PAYM_0)) != 0.0) {
          lines_paym++;
        }

        /* In case of a reprinted invoice, the amount cash must be decreased    */
        /* with the amount change. This could mean that there is not paid       */
        /* with cash at all.                                                    */
        if (tot_ret_double(TOT_CHANGE) != 0.0 &&
          i-PAYM_WAY_0 == (short)tot_ret_double(TOT_CHANGE_CD)) {
          if (tot_ret_double((short)(i - PAYM_WAY_0 + TOT_PAYM_0)) -
            tot_ret_double((short)TOT_CHANGE) == 0.0) {
            lines_paym--;
          }
        }
      }

      if (amount_due()!=0.0) {
        lines_paym++;
      }

      if (lines_paym > lines_vat) {
        p_object->ln= 8 + lines_paym + 1; /* 8 was total section. Now first vat/payment + empty line added. */
      }
      else {
        p_object->ln= 8 + lines_vat + 1;  /* 8 was total section. Now first vat/payment + empty line added. */
      }
    }
  }

  if (action == PRINT_IT) {

    if (GetPrinterSize(printer)!=PRINTER_SIZE_SMALL || GetPrinterSize(printer)!=PRINTER_SIZE_SMALL_INV ) {
			  
      while (on_ln_inv <= (LN_PG_INV - p_object->ln - LN_BOT_INV)) {

			#ifdef DEBUG
					print_ln(printer, _T("fn_i_total_lines"));
			#else
					print_ln(printer, empty);
			#endif
      }
    }

    prnt_totals(printer);
  }

  return (SUCCEED);
} /* fn_i_total_lines */

/*-------------------------------------------------------------------------*/
/*                          prnt_totals                                    */
/*-------------------------------------------------------------------------*/
/* Print all total lines.                                                  */
/*-------------------------------------------------------------------------*/
short prnt_totals(short printer)
{
  double tot_extra, tot_excl, tot_incl, tot_vat;  /****************************/
  _TCHAR buffer[ INV_SIZE + 1 ];
  _TCHAR tmp1[ TOTAL_BUF_SIZE + 1 ];
  _TCHAR tot_packs[ TOTAL_BUF_SIZE +1 ];
  _TCHAR genv_txt[82];

  tot_extra = tot_ret_double(TOT_LCREDIT_AMNT) +
              tot_ret_double(TOT_LCREDIT_VAT_AMNT);

  tot_excl = tot_ret_double(TOT_GEN_EXCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_EXCL));
  tot_incl = tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL));
  tot_vat  = tot_ret_double(TOT_GEN_VAT)  + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_VAT));

  tot_extra = tot_incl - tot_extra;

 
  /* Vat header + vat lines / payment lines -> nbr_of_vat_lines */
  
  print_vat_payment_lines(printer);

  /* Line 1 + nbr_of_vat_lines : empty line */ 
  print_ln(printer, empty);

  /* Line 2 + nbr_of_vat_lines: Donation line */
  if (pos_invoice.invoice_donation > 0.0) {
    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');
    ftoa_price(get_invoice_donation_currmode(/*v3.4.5 acm -*/), TOTAL_BUF_SIZE, tmp1); // acm -
    format_string(&string_price13_star, tmp1);
    fmt_left_justify_string(buffer, 15, 73, genvar.donation_txt);
    fmt_right_justify_string(buffer, 103, 116, string_price13_star.result);
    print_ln(printer, buffer);
  }
  else {
    print_ln(printer, empty); 
  }
  /*print_ln(printer, empty);*/
  /* Line 3 + nbr_of_vat_lines : empty line */
  /* print_ln(printer, empty); PERU Test */
  ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
  buffer[INV_SIZE] = _T('\0');
  ftoa_price(tot_incl, TOTAL_BUF_SIZE, tmp1);
  /*NoToLet(tmp1);*/
  format_string(&string_price13, tmp1);
  /*fmt_left_justify_string(buffer, 10, 116, string_price13_star.result);*/
  print_ln(printer, NoToLet(tmp1));

  print_ln(printer, empty); 

  /* Line 4 + nbr_of_vat_lines : used payment types line 1 + total exclusive VAT */
  ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
  buffer[INV_SIZE]=_T('\0');
  ftoa_price(tot_excl, TOTAL_BUF_SIZE, tmp1);
  format_string(&string_price13_star, tmp1);
  fmt_right_justify_string(buffer, 103, 116, string_price13_star.result);
  print_ln(printer, buffer);

  /* Line 5 + nbr_of_vat_lines : empty line */
  print_ln(printer, empty);

  /* Line 6 + nbr_of_vat_lines : VAT amount */
  ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
  buffer[INV_SIZE]=_T('\0');
  ftoa_price(tot_vat, TOTAL_BUF_SIZE, tmp1);
  format_string(&string_price13_star, tmp1);
  fmt_right_justify_string(buffer, 103, 116, string_price13_star.result);
  print_ln(printer, buffer);

  /* Line 7 + nbr_of_vat_lines : total packs */

  ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
  buffer[INV_SIZE] = _T('\0');
  fmt_left_justify_string(buffer, 63,  68, prn_inv_TXT[8]);
  ftoa(tot_ret_double(TOT_PACKS), TOTAL_BUF_SIZE, tot_packs);
  format_string(&string_packs6_star, tot_packs);
  fmt_left_justify_string(buffer, 70, 75, string_packs6_star.result);
  print_ln(printer, buffer);

  /* Line 8 + nbr_of_vat_lines : promotional text bottom 2 + total including VAT */
  ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
  buffer[INV_SIZE] = _T('\0');
  ftoa_price(tot_incl, TOTAL_BUF_SIZE, tmp1);
  format_string(&string_price13_star, tmp1);
  fmt_right_justify_string(buffer, 103, 116, string_price13_star.result);

  if (bot_copy_invoice_active == YES) {
    fmt_left_justify_string(buffer, 10, 78, prn_inv_TXT[39]);
  }
  else {
    _tcsncpy(genv_txt, genvar.prom_txt_bot1, 73);
    fmt_left_justify_string(buffer, 4, 78, genv_txt);
  }
  print_ln(printer, buffer);

  /* Line 9 + nbr_of_vat_lines : promotional text bottom 2 */
  ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
  buffer[INV_SIZE]=_T('\0');
  
  if (bot_copy_invoice_active == YES) {
    fmt_left_justify_string(buffer, 10, 78, prn_inv_TXT[39]);
  }
  else {
    _tcsncpy(genv_txt, genvar.prom_txt_bot2, 73);
    fmt_left_justify_string(buffer, 4, 78, genv_txt);
  }
  print_ln(printer, buffer);

/**/
  tot_extra=0; tot_excl=0; tot_incl=0; tot_vat=0;
  /**/

  return(SUCCEED);
} /* prnt_totals() */

/*-------------------------------------------------------------------------*/
/*                      print_vat_payment_lines                            */
/*-------------------------------------------------------------------------*/
short print_vat_payment_lines(short printer)
{
  short  l1, l2;
  _TCHAR buffer[INV_SIZE + 1];
  _TCHAR vat_part [100];
  _TCHAR paym_part[100];

  print_ln(printer, prn_inv_TXT[45]);

  l1 = prnt_total_vat (RESET, vat_part);
  l2 = prnt_total_paym(RESET, paym_part);
  do {
    _stprintf(buffer, prn_inv_TXT[19], vat_part, paym_part);
    print_ln(printer, buffer);
    l1 = prnt_total_vat (INCREMENT, vat_part);
    l2 = prnt_total_paym(INCREMENT, paym_part);
  } while (l1 || l2);

  return (SUCCEED);
} /* print_vat_payment_lines */

/*-------------------------------------------------------------------------*/
/*                          prnt_total_vat                                 */
/*-------------------------------------------------------------------------*/
/* Vat part is first filled with the used vat codes and then with the      */
/* total line.                                                             */
/*-------------------------------------------------------------------------*/
short prnt_total_vat(short action, _TCHAR *vat_part)
{
  static short used_vat_cd;
  _TCHAR tot_vat[TOTAL_BUF_SIZE +1];
  _TCHAR tot_excl[TOTAL_BUF_SIZE +1];
  _TCHAR perc[7 + 1];
  short  status;

  if (action == RESET) {
    used_vat_cd=0;
  }

  *vat_part=_T('\0');
  while (used_vat_cd <= TOT_EXCL_9 && tot_ret_double(used_vat_cd) == 0.0) {
    used_vat_cd++;
  }

  status = 1;

  if (used_vat_cd <= TOT_EXCL_9) {            /* Build a vat line.           */

    ftoa_price(floor_price(tot_ret_double(used_vat_cd) +
               tot_ret_double((short)(MSAM_DISC_TOT_EXCL_0+used_vat_cd))),
               TOTAL_BUF_SIZE, tot_excl);
    format_string(&string_price13, tot_excl);
    _tcscpy(tot_excl, string_price13.result);
      
    ftoa_price(floor_price(tot_ret_double((short)(used_vat_cd + TOT_VAT_0)) +
               tot_ret_double((short)(MSAM_DISC_TOT_VAT_0+used_vat_cd))),
               TOTAL_BUF_SIZE, tot_vat);
    format_string(&string_price13, tot_vat);
    _tcscpy(tot_vat, string_price13.result);

    *perc = _T('\0');
    fmt_vat_perc(get_vat_perc((short)(used_vat_cd % 10)), perc);
    _stprintf(vat_part, prn_inv_TXT[46], used_vat_cd % (short)10, perc, tot_excl, tot_vat);
    used_vat_cd++;
  }
  else {
    status = 0;
  }

  return(status);
} /* prnt_total_vat */

/*-------------------------------------------------------------------------*/
/*                          prnt_total_paym                                */
/*-------------------------------------------------------------------------*/
/* Total part is first filled with the used payment types and then with    */
/* the needed total lines.                                                 */
/*-------------------------------------------------------------------------*/
short prnt_total_paym(short action, _TCHAR *paym_part)
{
  static short used_paym_cd;
  static short first = TRUE;
  PAYMENT_DEF paym;
  _TCHAR   buffer[ TOTAL_BUF_SIZE +1 ];
  double extra_amount, tot_paym;
  short  extra_paym_cd, status;


  extra_paym_cd=(short)tot_ret_double(TOT_CREDIT_PAYM_CD);

  extra_amount =floor_price(tot_ret_double(TOT_LCREDIT_VAT_AMNT)) +
                floor_price(tot_ret_double(TOT_LCREDIT_AMNT));

  if (action == RESET) {
    used_paym_cd   = 1;
  }

  *paym_part=_T('\0');

  /* TOT_CHANGE is only valid in case the invoice is reprinted. In this    */
  /* case, the amount change is added to the amount cash, so it will       */
  /* have the correct value as it was on the original invoice.             */
  /* It's even possible there was not paid with cash at all.               */

  if ((short)tot_ret_double(TOT_CHANGE_CD) != 0) {
    tot_add_double((short)(TOT_PAYM_0+(short)tot_ret_double(TOT_CHANGE_CD)),
                   (tot_ret_double(TOT_CHANGE) * -1));
  }

  while (used_paym_cd <= MAX_PAYM_WAYS &&
    tot_ret_double((short)((used_paym_cd%MAX_PAYM_WAYS)+TOT_PAYM_0)) == 0.0) {
    used_paym_cd++;
  }

  status = 1;

  if (used_paym_cd <= MAX_PAYM_WAYS) {  /* Build a payment line.           */

    first  = TRUE;
    paym.paym_cd=used_paym_cd%MAX_PAYM_WAYS;
    get_paym(&paym);

    tot_paym = floor_price((tot_ret_double((short)(TOT_PAYM_0 + paym.paym_cd))
                            *paym.curr_standard)/paym.curr_rate);

    if (used_paym_cd == extra_paym_cd && 0.0 != extra_amount) {
      tot_paym -= extra_amount;
    }

    ftoa_price(tot_paym, TOTAL_BUF_SIZE, buffer);
    format_string(&string_price13_star, buffer);

    //3.4.7 acm -+++
    _stprintf(paym_part, prn_inv_TXT[47], paym.paym_descr, string_price13.result);
    ++used_paym_cd;
  }
  else { /* Change */

    if (first && amount_due()!=0.0) {
      ftoa_price(amount_due(), TOTAL_BUF_SIZE, buffer);
      format_string(&string_price13_star, buffer);
      /*_stprintf(paym_part, prn_inv_TXT[48], string_price13.result);*/
       
	  
	  first = FALSE;
    }
    else {
      status = 0;
    } 
  }

  /* Reset the payment total which was increased with the amount change, in */
  /* the beginning of this function, to it's old value.                    */
  if ((short)tot_ret_double(TOT_CHANGE_CD) != 0) {
    tot_add_double((short)(TOT_PAYM_0+(short)tot_ret_double(TOT_CHANGE_CD)),
                   tot_ret_double(TOT_CHANGE));
  }

  return(status);
} /* prnt_total_paym */

/*----------------------------------------------------------------------------*/
/*                            fn_i_subtotal                                   */
/*----------------------------------------------------------------------------*/
/* This function prints the subtotal at the bottom of the invoice and         */
/* executes a page skip. Consequently the promotional text and the customer   */
/* header are printed on the next page.                                       */
/*----------------------------------------------------------------------------*/
short fn_i_subtotal(PRN_OBJECT *p_object, short action,
                    short offs_grp, short printer)
{
# ifdef PRN_IMM
  _TCHAR   buffer[INV_SIZE+1];
  _TCHAR   dummy[TOTAL_BUF_SIZE+1];
  _TCHAR   packs[6 +1];             /* contains "PACKS" or packs qty (max. 999) */
  short  i, lines_vat, first_vat_ln;

  lines_vat    = 0;
  first_vat_ln = 1;

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      for (i=SUB_TOT_EXCL_0; i < SUB_TOT_EXCL_9 + 1; i++) {
        if (tot_ret_double(i) != 0.0) {
          lines_vat++;
        }
      }

      if (lines_vat == 1) {
        p_object->ln=lines_vat + 2;
      }
      else {
        p_object->ln=lines_vat + 1;
      }
    }
  }

  if (action == PRINT_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      return SUCCEED;
    }

    while (on_ln_inv <= (LN_PG_INV - p_object->ln - LN_BOT_INV)) {

#ifdef DEBUG
      print_ln(printer, _T("fn_i_subtotal"));
#else
      print_ln(printer, empty);
#endif
    }

    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');

    ftoa_price(tot_ret_double(SUB_TOT_GEN_INCL), TOTAL_BUF_SIZE, dummy); 
    format_string(&string_price13_star, dummy);
    _stprintf(buffer, prn_inv_TXT[5], empty,           /* No 'PACKS'   */
                                    prn_inv_TXT[17],   /* SUBTOTAL DUE */
                                    string_price13_star.result);
    print_ln(printer, buffer);

    /* Subtotal lines per vat code                                           */
    /* 9/3/93 F.Gijbels: subtotal packs under 'SUBTOTAL DUE'                 */
    for (i=SUB_TOT_EXCL_0; i <= SUB_TOT_EXCL_9; i++) {
      if (tot_ret_double(i) != 0.0) {
        ftoa_price(tot_ret_double(i), TOTAL_BUF_SIZE, dummy);
        if (first_vat_ln) {
          prnt_subtot_per_vat(RESET    , i, dummy, buffer, SUBTOTAL);
         }
        else {
          prnt_subtot_per_vat(INCREMENT, i, dummy, buffer, SUBTOTAL);
        }
        print_ln(printer, buffer);
        first_vat_ln=0;
      }
    }
    /* If no vat-lines are printed, print only the number of packs.           */
    if (first_vat_ln) {
      ftoa(tot_ret_double(TOT_SUB_PACKS), TOTAL_BUF_SIZE, packs);
      format_string(&string_packs6, packs);
      _stprintf(buffer, prn_inv_TXT[24], _T(' '), empty, empty, empty, empty,
                                       prn_inv_TXT[25], string_packs6.result);
      print_ln(printer, buffer);
    }
  }

#endif
  return (SUCCEED);
} /* fn_i_subtotal */


/*----------------------------------------------------------------------------*/
/*                            fn_i_skip_one_line                              */
/*----------------------------------------------------------------------------*/
/* Skips one line if requested by the cashier.                                */
/*----------------------------------------------------------------------------*/
short fn_i_skip_one_line(PRN_OBJECT *p_object, short action,
                         short offs_grp, short printer)
{
  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1+LN_TRANS_INV;
    }
  }

  if (action == PRINT_IT) {
    print_skip_ln(printer, empty); /* Skip line, do not increment on_ln_inv */
  }

  return (SUCCEED);
} /* fn_i_skip_one_line */


/*----------------------------------------------------------------------------*/
/*                            fn_i_void_inv                                   */
/*----------------------------------------------------------------------------*/
/* Prints at least one 'VOID' lines on the invoice in order to void the       */
/* entire invoice. On the next page the promotional text is printed.          */
/*----------------------------------------------------------------------------*/
short fn_i_void_inv(PRN_OBJECT *p_object, short action,
                    short offs_grp, short printer)
{
  short i, j;

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }

  if (action == PRINT_IT) {                  /* Ensure at least one void line */
    for (i=on_ln_inv; i <= LN_PG_INV-LN_BOT_INV; i+=5) {
      print_ln(printer, prn_inv_TXT[21]);
      for (j=1; j <= 4 && on_ln_inv <= LN_PG_INV-LN_BOT_INV; j++) {

#ifdef DEBUG
        print_ln(printer, _T("fn_i_void_inv"));
#else
        print_ln(printer, empty);
#endif

      }
    }
  }

  return (SUCCEED);
} /* fn_i_void_inv */


/*----------------------------------------------------------------------------*/
/*                            fn_i_to_end_of_page                             */
/*----------------------------------------------------------------------------*/
/* Prints linefeeds in order to go to the end of the invoice.                 */
/*----------------------------------------------------------------------------*/
short fn_i_to_end_of_page(PRN_OBJECT *p_object, short action,
                          short offs_grp, short printer)
{
  if (action == CALCULATE_IT) {
    p_object->ln=0;
  }

  if (action == PRINT_IT) {
    if (GetPrinterSize(printer)!=PRINTER_SIZE_SMALL || GetPrinterSize(printer)!=PRINTER_SIZE_SMALL_INV) {
		while (on_ln_inv <= LN_PG_INV  - LN_BOT_INV) { /*Revi. el +3 no debe quedar asi*/

#ifdef DEBUG
        print_ln(printer, _T("fn_i_to_end_of_page"));
#else
		{ print_ln(printer, empty); (printer, empty);(printer, empty);(printer, empty);}
#endif
      }
    }
  }

  return (SUCCEED);
} /* fn_i_to_end_of_page */


/*----------------------------------------------------------------------------*/
/*                            fn_i_empty_bottom                               */
/*----------------------------------------------------------------------------*/
/* Prints the required empty lines at the bottom of each invoice paper.       */
/*----------------------------------------------------------------------------*/
short fn_i_empty_bottom(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  short i;

  if (action == CALCULATE_IT) {
    p_object->ln=0;
  }

  if (action == PRINT_IT) {
    if (GetPrinterSize(printer)!=PRINTER_SIZE_SMALL || GetPrinterSize(printer)!=PRINTER_SIZE_SMALL_INV) {
      for (i=1; i <= LN_BOT_INV; i++) {
#ifdef DEBUG
        print_ln(printer, _T("fn_i_empty_bottom"));
#else
        print_ln(printer, empty);
#endif
      }
    }
  }

  return (SUCCEED);
} /* fn_i_empty_bottom */


/*----------------------------------------------------------------------------*/
/*                            fn_i_to_transport                               */
/*----------------------------------------------------------------------------*/
/* This function prints the required line(s) with the transported value of    */
/* the invoice. This value (total amount until now) will be transported to    */
/* the next page.                                                             */
/*----------------------------------------------------------------------------*/
short fn_i_to_transport(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  short i;
  _TCHAR  tot_curr[TOTAL_BUF_SIZE+1];
  _TCHAR  buffer[INV_SIZE+1];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL || GetPrinterSize(printer)!=PRINTER_SIZE_SMALL_INV) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=LN_TRANS_INV;
    }
  }

  if (action == PRINT_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      return(SUCCEED);
    }
    *tot_curr=(_TCHAR)0;
    while (on_ln_inv <= (LN_PG_INV-LN_BOT_INV-LN_TRANS_INV)) {
#ifdef DEBUG
      print_ln(printer, _T("fn_i_to_transport (a)"));
#else
      print_ln(printer, empty);
#endif
    }

    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');

    /* Print total carried forward line */
    ftoa_price(floor_price(tot_ret_double(TOT_CARR_FORWD_INCL)+tot_ret_double(TOT_CARR_FMSAM_INCL)), TOTAL_BUF_SIZE, tot_curr);
    format_string(&string_price13_star, tot_curr);
    fmt_right_justify_string(buffer, 103, 116, string_price13_star.result);   /*corte*/
	print_ln(printer, buffer);
    
	/* In case more than one 'Total carried forward line' required...        */
    for (i=1; i <= LN_TRANS_INV; i++) {
#ifdef DEBUG
      print_ln(printer, _T("fn_i_to_transport (b)"));
#else
  #ifdef PRN_IMM
      print_ln(printer, empty);
  #else
      if (voided_invoice == YES && i % 3 == 2) {
        print_ln(printer, prn_inv_TXT[21]);
      }
      else {
        print_ln(printer, empty);
      }
  #endif
#endif
    }
  }

  return (SUCCEED);
} /* fn_i_to_transport */


/*----------------------------------------------------------------------------*/
/*                            fn_i_transported                                */
/*----------------------------------------------------------------------------*/
/* This function prints the required line(s) with the transported value of    */
/* the previous page (total brought forward).                                 */
/*----------------------------------------------------------------------------*/
short fn_i_transported(PRN_OBJECT *p_object, short action,
                       short offs_grp, short printer)
{
  _TCHAR  tot_curr[TOTAL_BUF_SIZE+1];
  _TCHAR  buffer[INV_SIZE+1];

  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }

  if (action == PRINT_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      return(SUCCEED);
    }
    while (on_ln_inv < START_LN_ART) {
#ifdef DEBUG
      print_ln(printer, _T("fn_i_transported"));
#else
      print_ln(printer, empty);
#endif
    }

    ftoa_price(floor_price(tot_ret_double(TOT_CARR_FORWD_INCL)+tot_ret_double(TOT_CARR_FMSAM_INCL)), TOTAL_BUF_SIZE, tot_curr);
    format_string(&string_price13_star, tot_curr);
   /* _stprintf(buffer, prn_inv_TXT[16], string_price13_star.result ); *//*vienen*/
    _stprintf(buffer, prn_inv_TXT[16] );

    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_i_transported */


/*----------------------------------------------------------------------------*/
/*                            fn_i_msam_total                                 */
/*----------------------------------------------------------------------------*/
/* Prints msam-actions discount total                                         */
/*----------------------------------------------------------------------------*/
short fn_i_msam_total(PRN_OBJECT *p_object, short action,
                      short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+1];
  _TCHAR  dummy[TOTAL_BUF_SIZE+1];
  double  tot_disc_amount;

  if (action == CALCULATE_IT) {
#define NBR_PRINT_LN_MSAM_TOTAL  1  /* Number of print_ln calls in this fn_i function */
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln = NBR_PRINT_LN_MSAM_TOTAL + LN_TRANS_INV;
    }
  }
  if (action == PRINT_IT) {
    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');
    fmt_left_justify_string(buffer, 0, 79, prn_inv_TXT[31]); 

    tot_disc_amount = (genvar.price_incl_vat == INCLUSIVE) ? floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))
                                                           : floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_EXCL));
    ftoa_price(floor_price(tot_disc_amount), TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 109, 119, string_price11_2.result);

    print_ln(printer, buffer);
  }

  return (SUCCEED);
}  /* fn_i_msam_total */

  
/*----------------------------------------------------------------------------*/
/*                            fn_i_msam_line                                  */
/*----------------------------------------------------------------------------*/
/* Prints msam-action discount line                                           */
/*----------------------------------------------------------------------------*/
short fn_i_msam_line(PRN_OBJECT *p_object, short action,
                     short offs_grp, short printer)
{ 
#define MSAM_LINE_TEXT_START   20
#define MSAM_LINE_TEXT_END     98

  _TCHAR  buffer[INV_SIZE+1];
  _TCHAR  dummy[INV_SIZE+1];
  double disc_amount;

  if (action == CALCULATE_IT) {
#define NBR_PRINT_LN_MSAM_LINE  1  /* Number of print_ln calls in this fn_i function */
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln = NBR_PRINT_LN_MSAM_LINE + NBR_PRINT_LN_MSAM_TOTAL + LN_TRANS_INV;
    }
  }  


  if (action == PRINT_IT) {
    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');

    if (prn_msam_action.times) {
      ftoa(prn_msam_action.times, 9, dummy);
      format_string(&string_qty4, dummy);
      fmt_right_justify_string(buffer, 15, 18, string_qty4.result);
    } 

    /* Prevent that line text is emptied by fmt_left_justify_string: */
    if(_tcslen(prn_msam_action.line_text)>MSAM_LINE_TEXT_END-MSAM_LINE_TEXT_START+1) {
      _tcsncpy(dummy, prn_msam_action.line_text, MSAM_LINE_TEXT_END-MSAM_LINE_TEXT_START+1);
      dummy[MSAM_LINE_TEXT_END-MSAM_LINE_TEXT_START+1]=_T('\0');
    }
    else {
      _tcscpy(dummy, prn_msam_action.line_text);
    }
    fmt_left_justify_string(buffer, MSAM_LINE_TEXT_START,
                                    MSAM_LINE_TEXT_END,
                                    dummy);
    
    disc_amount = (genvar.price_incl_vat == INCLUSIVE) ? prn_msam_action.disc_incl
		                                                   : prn_msam_action.disc_excl;
    disc_amount *= -1.0;

    ftoa_price(disc_amount, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 112, 122, string_price11_2.result);

    print_ln(printer, buffer);
  }  

  return(SUCCEED);
} /* fn_i_msam_line */


/*----------------------------------------------------------------------------*/
/*                            fn_i_msam_header                                */
/*----------------------------------------------------------------------------*/
/* Prints header of msam discount section                                     */
/*----------------------------------------------------------------------------*/
short fn_i_msam_header(PRN_OBJECT *p_object, short action,
                       short offs_grp, short printer)
{
  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln = 2 + NBR_PRINT_LN_MSAM_LINE + NBR_PRINT_LN_MSAM_TOTAL + LN_TRANS_INV;
    }
  }

  if (action == PRINT_IT) {
    print_ln(printer, empty);
    print_ln(printer, prn_inv_TXT[30]);
  }

  return (SUCCEED);
} /* fn_i_msam_header */


/*----------------------------------------------------------------------------*/
/*                            fn_i_cust_fee_line                              */
/*----------------------------------------------------------------------------*/
/* Prints Customer fee line                                                   */
/*----------------------------------------------------------------------------*/
short fn_i_cust_fee_line(PRN_OBJECT *p_object, short action,
                         short offs_grp, short printer)
{
  _TCHAR  buffer[INV_SIZE+1];
  _TCHAR  dummy[TOTAL_BUF_SIZE+1];
  _TCHAR  extra_dummy[INV_SIZE+1];

  _TCHAR  datebuf[40];
  _TCHAR  perc[7+1];
  long    ldate;
  double  tot_fee_amount;


  if (action == CALCULATE_IT) {
    if (GetPrinterSize(printer)==PRINTER_SIZE_SMALL) {
      p_object->ln = 0; /* Never calculate for smallprinter */
    }
    else {
      p_object->ln=1;
    }
  }

  if (action == PRINT_IT) {
    while (on_ln_inv < START_LN_ART) {

#ifdef DEBUG
      print_ln(printer, _T("fn_i_cust_fee_line"));
#else
      print_ln(printer, empty);
#endif
    }

    ch_memset(buffer, _T(' '), INV_SIZE*sizeof(_TCHAR));
    buffer[INV_SIZE]=_T('\0');
    /*
       Determine now the expiry date for this customer depending on
       the cancelling or paying of the due fee.
    */
    if (pos_invoice.invoice_fee_status == FEE_STAT_CANCEL_YEAR) {
      /* The fee payment is overruled for one year 'exp-date += 1 year' */
      ldate = pos_invoice.invoice_date + 10000;
    }
    else if (pos_invoice.invoice_fee_status == FEE_STAT_CANCEL) {
       /* The fee payment is overruled for this time only */
      ldate = cust.exp_date;
    }
    else {
       /* The fee is paid 'exp-date += 1 year*/
      ldate = pos_invoice.invoice_date + 10000;
    }
    prn_fmt_date(ldate, genvar.date_format, mon_names, datebuf);

    _tcscpy(extra_dummy, genvar.fee_descr);
    fmt_left_justify_string(buffer, 15, (short)(15 + _tcslen(extra_dummy)), extra_dummy);

    if (cust.card_type_no != genvar.daypass_card_type_no) {
      fmt_left_justify_string(buffer, (short)(15 + _tcslen(extra_dummy) + 1),
                              (short)(15 + _tcslen(datebuf) + _tcslen(extra_dummy) + 1),
                              datebuf);
    }
    else { /* DAY PASS on invoice i.s.o date + vat perc. */
      fmt_left_justify_string(buffer, (short)(15 + _tcslen(extra_dummy) + 1),
                              (short)(15 + _tcslen(menu_TXT[43]) + _tcslen(extra_dummy) + 1),
                              menu_TXT[43]);
      *perc = _T('\0');
      fmt_vat_perc(get_vat_perc(genvar.cfee_vat_no), perc);
      fmt_right_justify_string(buffer, 99, 105, perc);
    }

    if (genvar.price_incl_vat == INCLUSIVE) {
      tot_fee_amount = floor_price(calc_incl_vat(tot_ret_double(TOT_FEE_AMOUNT), genvar.cfee_vat_no));
    }
    else {
      tot_fee_amount = tot_ret_double(TOT_FEE_AMOUNT);
    }

    ftoa_price(tot_fee_amount, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 112, 122, string_price11_2.result);

    print_ln(printer, buffer); /* print line */
  }

  return (SUCCEED);
} /* fn_i_cust_fee_line */


void print_distrito( short printer)
{
    char distrito[200];
    _TCHAR   buffer[SMALL_INV_SIZE+1];
    int store_no=pos_system.store_no;
    char * array[11]=
    {
        "",
        "INDEPENDENCIA - LIMA - LIMA", //1
        "CALLAO-PROV. CONST. DEL CALLAO - LIMA", //2
        "SANTA ANITA - LIMA - LIMA", //3
        "SANTIAGO DE SURCO - LIMA - LIMA", //4
        "JOSE LUIS BUSTAMANTE Y RIVERO - AREQUIPA", //5
        "CHICLAYO - CHICLAYO - LAMBAYEQUE", //6
        "SAN JUAN DE LURIGANCHO - LIMA - LIMA", //7
        "TRUJILLO - TRUJILLO - LA LIBERTAD", //8
        "PIURA - PIURA - PIURA", //9
        "COMAS - LIMA - LIMA" //10
    };

    if (store_no>=1 && store_no<=10)
    {
        strcpy(distrito, array[store_no]);
    }else
        return;

    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    fmt_centre_justify_string(buffer,  0, 39, distrito);
    print_ln(printer, buffer);
    if (store_no==5)
    {
        ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
        buffer[SMALL_INV_SIZE]=_T('\0');

        fmt_centre_justify_string(buffer,  0, 39, "AREQUIPA");
        print_ln(printer, buffer);
    }
}

/*----------------------------------------------------------------------------*/
/* SMALL INVOICE LAYOUT                                                       */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                            fn_i_small_header                               */
/*----------------------------------------------------------------------------*/
short fn_i_small_header(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  short    i;
  _TCHAR   prn_date[12 +1];
  _TCHAR   buffer[SMALL_INV_SIZE+1];


  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {
    for (i=0;i<2;i++) {
      print_ln(printer, empty);
    }
    
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
	/* MAKRO SUPERMAYORISTA S.A. */
    if (_tcslen(genvar.name) >= SMALL_INV_SIZE) {
      genvar.name[SMALL_INV_SIZE]=_T('\0');
    }
    fmt_centre_justify_string(buffer, 0, SMALL_INV_SIZE - 1, genvar.name_comp);
    print_ln(printer, buffer);

	/* MAKRO - LIMA 04*/
    /*ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    if (_tcslen(genvar.name) >= 30) {
      genvar.name[30]=_T('\0');
    }
    fmt_centre_justify_string(buffer,  0, SMALL_INV_SIZE - 1, genvar.name);
    print_ln(printer, buffer);*/

	/* AV. JORGE CHAVEZ NRO. 1218 */
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    fmt_centre_justify_string(buffer,  0, SMALL_INV_SIZE - 1, wp_de_data_conf.descr_dir);
    print_ln(printer, buffer);
	
	/* LIMA-LIMA-SANTIAGO DE SURCO */
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    fmt_centre_justify_string(buffer,  0, SMALL_INV_SIZE - 1, wp_de_data_conf.descr_dist);
    print_ln(printer, buffer);
	
	/* RUC 2049... */
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    if (_tcslen(genvar.fisc_no_comp) >= 20) {
      genvar.fisc_no_comp[20]=_T('\0');
    }
    fmt_centre_justify_string(buffer,  0, 39, genvar.fisc_no_comp);
    print_ln(printer, buffer);

	/*Av. Jorge Chavez ...*/
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    if (_tcslen(genvar.address) >= 35) {
      genvar.address[35]=_T('\0');
    }
    fmt_centre_justify_string(buffer,  0, 39, genvar.address);
    print_ln(printer, buffer);
    
	/* SANTIAGO DE SURCO - ...*/
    print_distrito(printer);

	/* TELF: 614-9300 */
    if(_tcslen(genvar.tax_txt1) > 0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      if (_tcslen(genvar.tax_txt1) >= 40) {
        genvar.tax_txt1[40]=_T('\0');
      }
      fmt_centre_justify_string(buffer,  0, 39, genvar.tax_txt1);
      print_ln(printer, buffer);
    }
	
	/*Serial number*/
    _stprintf(buffer, prn_inv_TXT[75], till.till_id);
    print_ln(printer, buffer);

    if(_tcslen(genvar.tax_txt2) > 0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      if (_tcslen(genvar.tax_txt2) >= 40) {
        genvar.tax_txt2[40]=_T('\0');
      }
      fmt_centre_justify_string(buffer,  0, 39, genvar.tax_txt2);
      print_ln(printer, buffer);
    }

    if(_tcslen(genvar.tax_txt3) > 0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      if (_tcslen(genvar.tax_txt3) >= 40) {
        genvar.tax_txt3[40]=_T('\0');
      }
      fmt_centre_justify_string(buffer,  0, 39, genvar.tax_txt3);
      print_ln(printer, buffer);
    }
    
    print_ln(printer, empty);
	
	/* BOLETA VENTA ELECTRONICA */
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
	buffer[SMALL_INV_SIZE]=_T('\0');
	if(is_client_pay_FACTURA()) {
		fmt_centre_justify_string(buffer,  0, SMALL_INV_SIZE - 1, wp_de_data_conf.descr_fac);
	} else {
		fmt_centre_justify_string(buffer,  0, SMALL_INV_SIZE - 1, wp_de_data_conf.descr_bol);
	}
	print_ln(printer, buffer);

	/* SERIE BOLETA O FACTURA */
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
	buffer[SMALL_INV_SIZE]=_T('\0');
	if (_tcslen(serie_corr) >= 30) {
		serie_corr[30]=_T('\0');
	}
	fmt_centre_justify_string(buffer,  0, SMALL_INV_SIZE - 1, serie_corr);
	print_ln(printer, buffer);
	
	print_ln(printer, empty);

	/* FECHA DE EXP... dd-mm-yyyy */
	/* Tienen que cambiar el date_format para dd/mm/yyyy */
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
	buffer[SMALL_INV_SIZE]=_T('\0');
	prn_fmt_date(pos_invoice.invoice_date, wp_de_data_conf.date_format, mon_names, prn_date);
	fmt_left_justify_string(buffer,  0, 19, prn_inv_TXT[49]);
	fmt_right_justify_string(buffer, 20, 39, prn_date);
	print_ln(printer, buffer);
	
	print_ln(printer, prn_inv_TXT[51]); /* ============ */
	print_ln(printer, prn_inv_TXT[54]); /* Cod.	Descripcion */
	print_ln(printer, prn_inv_TXT[68]); /*      Cant 	Valor IGV 	Total */
	print_ln(printer, prn_inv_TXT[51]); /* ============ */
  }

  return (SUCCEED);
} /* fn_i_small_header */


/*----------------------------------------------------------------------------*/
/*                            fn_i_small_article                              */
/*----------------------------------------------------------------------------*/
short fn_i_small_article(PRN_OBJECT *p_object, short action,
                         short offs_grp, short printer)
{
  _TCHAR   dummy[DESCR_SIZE+1];
  _TCHAR   buffer[SMALL_INV_SIZE+1];
  //_TCHAR   buffer2[SMALL_INV_SIZE+1+4];

  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) { //acm - print
    //v3.6.1 acm -{
    /* mlsd P */
    /* Aqui coloca la P de percepcion en el precio de articulo en el ticket */
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');

	/* Article code */
    ftoa((double)c_item.arti.base.art_no, 15, dummy);
    fmt_right_justify_string(buffer, 1, 7, dummy); 

	/* Article description */
    _tcscpy(dummy, c_item.arti.base.descr);
    if (_tcslen(dummy) >= 31) {
      dummy[31]=_T('\0');
    }
    fmt_left_justify_string(buffer, 9, SMALL_INV_SIZE, dummy); 
	
	/* Article with perception */
	if (c_item.arti.base.percep_amount > 0.0001)
		buffer[0]='P';
	
    print_ln(printer, buffer); //v3.6.1 acm -

	/* Article quantity */
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    if (c_item.arti.art_ind==ART_IND_WEIGHT) {
      ftoa(c_item.arti.base.qty*1000, 9, dummy);
      format_string(&string_qty8, dummy);
      fmt_right_justify_string(buffer, 3, 10, string_qty8.result);
    }
    else {
      ftoa(c_item.arti.base.qty, 9, dummy);
      format_string(&string_qty5, dummy);
      fmt_right_justify_string(buffer, 6, 10, string_qty5.result);
    }

    //v3.6.1 acm -}

	/* Article price */
    ftoa_price(c_item.arti.base.price, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 13, 23, string_price11_2.result);

	/* VAT code */
    ftoa(c_item.arti.base.vat_no, 2, dummy);
    fmt_right_justify_string(buffer, 27, 28, dummy);

	/* Total article price */
    ftoa_price(c_item.arti.base.goods_value, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 28, 40, string_price11_2.result);
	buffer[SMALL_INV_SIZE]=_T('\0');
    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_i_small_article */


/*----------------------------------------------------------------------------*/
/*                            fn_i_small_deposit                              */
/*----------------------------------------------------------------------------*/
short fn_i_small_deposit(PRN_OBJECT *p_object, short action,
                         short offs_grp, short printer)
{
  _TCHAR   buffer[SMALL_INV_SIZE+1];
  _TCHAR   qty[TOTAL_BUF_SIZE +1];
  _TCHAR   dummy[DESCR_SIZE +1];

  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {

    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');

    ftoa((double)c_item.depo.base.art_no, 15, dummy);
    fmt_right_justify_string(buffer, 0, 5, dummy); 

    _tcscpy(dummy, c_item.depo.base.descr);
    if (_tcslen(dummy) >= 33) {
      dummy[33]=_T('\0');
    }
    fmt_left_justify_string(buffer, 7, 39, dummy); 

    print_ln(printer, buffer);

    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    if (c_item.depo.art_ind==ART_IND_WEIGHT) {
      _tcscpy(qty, _T("1"));
    }
    else {
      ftoa(c_item.depo.base.qty, TOTAL_BUF_SIZE, qty);
    }
    format_string(&string_qty5_star, qty);
    fmt_right_justify_string(buffer, 7, 11, string_qty5_star.result);

    ftoa_price(c_item.depo.base.price, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 12, 23, string_price11_2.result);

    ftoa(c_item.depo.base.vat_no, 2, dummy);
    fmt_right_justify_string(buffer, 25, 26, dummy);

    ftoa_price(c_item.depo.base.goods_value, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 27, 39, string_price11_2.result);

    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_i_small_deposit */

/*----------------------------------------------------------------------------*/
/*                            fn_i_small_discount                             */
/*----------------------------------------------------------------------------*/
short fn_i_small_discount(PRN_OBJECT *p_object, short action,
                          short offs_grp, short printer)
{
  _TCHAR   buffer[SMALL_INV_SIZE+1];
  _TCHAR   dummy[DESCR_SIZE +1];
  _TCHAR   qty[TOTAL_BUF_SIZE +1];

  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    _tcscpy(dummy, c_item.disc.base.descr);

    if (_tcslen(dummy) >= 33) {
      dummy[33]=_T('\0');
    }
    fmt_left_justify_string(buffer, 9, 39, dummy); 

    print_ln(printer, buffer);

    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    ftoa(c_item.disc.base.qty, TOTAL_BUF_SIZE, qty);
    format_string(&string_qty5, qty);
    fmt_right_justify_string(buffer, 7, 14, string_qty5.result);

    ftoa_price(c_item.disc.base.price, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 17, 27, string_price11_2.result);
    
    ftoa(c_item.disc.base.vat_no, 2, dummy);
    fmt_right_justify_string(buffer, 27, 28, dummy);

    ftoa_price(c_item.disc.base.goods_value, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 28, 39, string_price11_2.result);

    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_i_small_discount */

/*mlsd*/
void fmt_tot_f(_TCHAR * dummy, double num)
{
	memset(dummy, 0, SMALL_INV_SIZE+1);
	sprintf(dummy, "%.2f", num);
	return;
}
/*mlsd*/

/*----------------------------------------------------------------------------*/
/*                            fn_i_small_total                                */
/*----------------------------------------------------------------------------*/
short fn_i_small_total(PRN_OBJECT *p_object, short action,
                       short offs_grp, short printer)
{
   double       tot_incl, extra_amount, tot_paym, tot_disc_amount;
   short        used_paym_cd, extra_paym_cd;
  _TCHAR        buffer[SMALL_INV_SIZE+1];
  _TCHAR        dummy[SMALL_INV_SIZE+1];
  _TCHAR   		dummy_d[256];
  _TCHAR   		aux[256];
  _TCHAR        sbuff_anticipo[SMALL_INV_SIZE+1];
  
  PAYMENT_DEF   paym;
  int has_valepavo=0;
  int has_anticipo=0;
  double tot_incl_perception=0;
  double tot_perception     =0;

  _TCHAR **aux_too_long;
  int length = 0;
  int i;
  int parts; 
  int size = 39;
  
  //double fix_donation;

  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {

    print_ln(printer, prn_inv_TXT[99]); /* ----- */

    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    tot_incl = tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL));
    ftoa_price(tot_incl, TOTAL_BUF_SIZE, dummy);

	//sprintf(buffer,dummy);

    tot_disc_amount = (genvar.price_incl_vat == INCLUSIVE) ? floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))
                                                           : floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_EXCL));
	/* OP. GRAVADAS */
	fmt_tot_f(dummy, floor_price((de_gravadas)/1.18));
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[100]);//acm - print
    fmt_right_justify_string(buffer, 26, 39, dummy);
    print_ln(printer, buffer);

	/* OP. GRATUITAS */
	fmt_tot_f(dummy, de_gratuitas);
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[101]);//acm - print
    fmt_right_justify_string(buffer, 26, 39, dummy);
    print_ln(printer, buffer);

	/* OP. EXONERADAS */
	fmt_tot_f(dummy, de_exoneradas);
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[102]);//acm - print
    fmt_right_justify_string(buffer, 26, 39, dummy);
    print_ln(printer, buffer);

	/* OP. INAFECTAS */
    fmt_tot_f(dummy, de_inafectas);
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[103]);//acm - print
    fmt_right_justify_string(buffer, 26, 39, dummy);
    print_ln(printer, buffer);	
	
	/* TOT. DESCUENTO GLOBAL */
    fmt_tot_f(dummy, 0);
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[104]);//acm - print
    fmt_right_justify_string(buffer, 26, 39, dummy);
    print_ln(printer, buffer);
	
	/* I.S.C. */
    fmt_tot_f(dummy, 0); /* No se tiene nada con ISC */
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[105]);//acm - print
    fmt_right_justify_string(buffer, 26, 39, dummy);
    print_ln(printer, buffer);
	
	/* I.G.V. (18%) */
    fmt_tot_f(dummy, floor_price(de_impuesto_d));
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[106]);//acm - print
    fmt_right_justify_string(buffer, 26, 39, dummy);
    print_ln(printer, buffer);

	/* TOTAL VENTA */
    fmt_tot_f(dummy, de_total_d);
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[108]);//acm - print
    fmt_right_justify_string(buffer, 26, 39, dummy);
    print_ln(printer, buffer);
	
	/* TOTAL A PAGAR CON PERCEPCION */
	if(IS_GEN_PERCEPTION)
		fmt_tot_f(dummy, de_total_d + get_invoice_perception_currmode());
	else 
		fmt_tot_f(dummy, 0);
	
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE * sizeof(_TCHAR));
    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[107]);//acm - print
    fmt_right_justify_string(buffer, 26, 39, dummy);
    print_ln(printer, buffer);	
	
    //v3.6.1 acm -{
    /*tot_perception   = get_invoice_perception_currmode();//floor_price(TOT_GEN_PERCEPTION) ;
    */
    /*if ( tot_perception != 0.0  *//*POS_ZERO_VALUE *//*) */
    /*{
        ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
        buffer[SMALL_INV_SIZE]=_T('\0');
        tot_incl_perception = tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))+ tot_perception;

        ftoa_price(tot_incl_perception, TOTAL_BUF_SIZE, dummy);

	    sprintf(buffer,dummy);

        format_string(&string_price13_star, dummy);*/
        /* mlsd P */
        /* Aqui coloca el total con percepcion*/
    /*    fmt_left_justify_string(buffer, 0,  39, prn_inv_TXT[88]);//acm - print perception
        fmt_right_justify_string(buffer, 26, 39, string_price13_star.result);

		(printer, buffer);
    }*/
    //v3.6.1 acm -}


    extra_paym_cd = (short)tot_ret_double(TOT_CREDIT_PAYM_CD);

    extra_amount =floor_price(tot_ret_double(TOT_LCREDIT_VAT_AMNT)) +
                  floor_price(tot_ret_double(TOT_LCREDIT_AMNT));

    /* TOT_CHANGE is only valid in case the invoice is reprinted. In this    */
    /* case, the amount change is added to the amount cash, so it will       */
    /* have the correct value as it was on the original invoice.             */
    /* It's even possible there was not paid with cash at all.               */

    if ((short)tot_ret_double(TOT_CHANGE_CD) != 0) {
      tot_add_double((short)(TOT_PAYM_0+(short)tot_ret_double(TOT_CHANGE_CD)),
                     (tot_ret_double(TOT_CHANGE) * -1));
    }
	
	used_paym_cd = 1;

	//fix_donation = c_shft.donation; //V3.4.7 acm -

    if (get_num_vale_pavo()==0) /* We don't have turkey coupon */// v3.5.1 acm -
    {
		if (!IS_ANTICIPO())
		{
			/* This code is executed the most of the times */
			used_paym_cd = 1;
			while (used_paym_cd <= MAX_PAYM_WAYS) 
			{
				if (tot_ret_double((short)((used_paym_cd%MAX_PAYM_WAYS)+TOT_PAYM_0)) != 0.0) {
					paym.paym_cd=used_paym_cd%MAX_PAYM_WAYS;
					get_paym(&paym);
					tot_paym = floor_price((tot_ret_double((short)(TOT_PAYM_0 + paym.paym_cd))
											*paym.curr_standard)/paym.curr_rate);

					if (used_paym_cd == extra_paym_cd && 0.0 != extra_amount) {
					  tot_paym -= extra_amount;
					}
						//V3.4.7 acm -  { restar la donacion del medio de pago soles
						/*
						if (used_paym_cd==PAYM_WAY_1){
						 tot_paym-=fix_donation; 

						 fix_donation=0;
						}
						*/
						//V3.4.7 acm -  }  

					ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
					buffer[SMALL_INV_SIZE]=_T('\0');
					ftoa_price(tot_paym, TOTAL_BUF_SIZE, dummy);
					format_string(&string_price13, dummy);
					//3.4.7 acm -+++
					fmt_left_justify_string(buffer, 0, 39, paym.paym_descr);
					/*Aqui imprime los tipos de pago*/
					fmt_right_justify_string(buffer, 26, SMALL_INV_SIZE, string_price13.result);
					buffer[SMALL_INV_SIZE]=_T('\0');
					print_ln(printer, buffer);
				}
				++used_paym_cd;
			}
		} 
		else { /* ES ANTICIPO*/
			//buscar el vale pavo e imprimir

			used_paym_cd = PAYM_WAY_8;
			has_anticipo=0;

			if (tot_ret_double((short)((used_paym_cd%MAX_PAYM_WAYS)+TOT_PAYM_0)) != 0.0) 
			{

				paym.paym_cd=used_paym_cd%MAX_PAYM_WAYS;
				get_paym(&paym);
				tot_paym = floor_price((tot_ret_double((short)(TOT_PAYM_0 + paym.paym_cd))
										*paym.curr_standard)/paym.curr_rate);

				if (tot_paym>0)
				{
					ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
					buffer[SMALL_INV_SIZE]=_T('\0');
					ftoa_price(-tot_paym, TOTAL_BUF_SIZE, dummy);
					format_string(&string_price13_star, dummy);
					//3.4.7 acm -+++
					//sprintf(sbuff_anticipo, "ANT.VALE PAVO(No %d)",cur_valepavo.vale_no);
					sprintf(sbuff_anticipo, "ANTICIPO(No %s)",cur_anticipo.fac_no);
					fmt_left_justify_string(buffer, 1, 39, sbuff_anticipo/*paym.paym_descr*/);
					/*Aqui imprime los tipos de pago*/
					fmt_right_justify_string(buffer, 26, 39, string_price13_star.result);
					print_ln(printer, buffer);

					has_anticipo=1;


					ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
					buffer[SMALL_INV_SIZE]=_T('\0');
					ftoa_price(tot_incl - tot_paym, TOTAL_BUF_SIZE, dummy);
					format_string(&string_price13_star, dummy);
					//3.4.7 acm -+++
					fmt_left_justify_string(buffer, 1, 39, "NETO A PAGAR"/*paym.paym_descr*/);
					/*Aqui imprime los tipos de pago*/
					fmt_right_justify_string(buffer, 26, 39, string_price13_star.result);
					print_ln(printer, buffer);
				}
			}

			used_paym_cd = 1;
			while (used_paym_cd <= MAX_PAYM_WAYS) 
			{
			  if (tot_ret_double((short)((used_paym_cd%MAX_PAYM_WAYS)+TOT_PAYM_0)) != 0.0) 
			  {
				paym.paym_cd=used_paym_cd%MAX_PAYM_WAYS;
				get_paym(&paym);
				tot_paym = floor_price((tot_ret_double((short)(TOT_PAYM_0 + paym.paym_cd))
										*paym.curr_standard)/paym.curr_rate);

				if (used_paym_cd == extra_paym_cd && 0.0 != extra_amount) {
				  tot_paym -= extra_amount;
				}

				//V3.4.7 acm -  { restar la donacion del medio de pago soles
				/*
				if (used_paym_cd==PAYM_WAY_1){
				 tot_paym-=fix_donation; 

				 fix_donation=0;
				}
				*/
				//V3.4.7 acm -  }  

				if ((has_anticipo)&&(used_paym_cd == PAYM_WAY_8))
				{
					//  no hacer nada / el proceso ya imprimio
				}
				else
				{
					ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
					buffer[SMALL_INV_SIZE]=_T('\0');
					ftoa_price(tot_paym, TOTAL_BUF_SIZE, dummy);
					format_string(&string_price13, dummy);
					//3.4.7 acm -+++
					fmt_left_justify_string(buffer, 0, 39, paym.paym_descr);
					/*Aqui imprime los tipos de pago*/
					fmt_right_justify_string(buffer, 26, SMALL_INV_SIZE, string_price13.result);
					buffer[SMALL_INV_SIZE]=_T('\0');
					print_ln(printer, buffer);
				}
			  }
			  ++used_paym_cd;
			}
		}
	}
	else { /* We have a turkey coupon */
		used_paym_cd = PAYM_WAY_3;
		has_valepavo=0;
		if (tot_ret_double((short)((used_paym_cd%MAX_PAYM_WAYS)+TOT_PAYM_0)) != 0.0) 
		{
			paym.paym_cd=used_paym_cd%MAX_PAYM_WAYS;
			get_paym(&paym);
			tot_paym = floor_price((tot_ret_double((short)(TOT_PAYM_0 + paym.paym_cd)) * paym.curr_standard)/paym.curr_rate);

			if (tot_paym > 0)
			{
				ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
				buffer[SMALL_INV_SIZE]=_T('\0');
				ftoa_price(-tot_paym, TOTAL_BUF_SIZE, dummy);
				format_string(&string_price13, dummy);
				//3.4.7 acm -+++
				sprintf(sbuff_anticipo, "ANT.VALE PAVO(No %d)",cur_valepavo.vale_no);
				fmt_left_justify_string(buffer, 1, 39, sbuff_anticipo/*paym.paym_descr*/);
				/*Aqui imprime los tipos de pago*/
				fmt_right_justify_string(buffer, 26, 39, string_price13.result);
				print_ln(printer, buffer);

				has_valepavo = 1;

				ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
				buffer[SMALL_INV_SIZE]=_T('\0');
				ftoa_price(tot_incl - tot_paym, TOTAL_BUF_SIZE, dummy);
				format_string(&string_price13, dummy);
				//3.4.7 acm -+++
				fmt_left_justify_string(buffer, 1, 39, "NETO A PAGAR"/*paym.paym_descr*/);
				/*Aqui imprime los tipos de pago*/
				fmt_right_justify_string(buffer, 26, 39, string_price13.result);
				print_ln(printer, buffer);
			}
		}

		used_paym_cd = 1;
		while (used_paym_cd <= MAX_PAYM_WAYS) 
		{
			if (tot_ret_double((short)((used_paym_cd%MAX_PAYM_WAYS)+TOT_PAYM_0)) != 0.0) 
			{
				paym.paym_cd=used_paym_cd%MAX_PAYM_WAYS;
				get_paym(&paym);
				tot_paym = floor_price((tot_ret_double((short)(TOT_PAYM_0 + paym.paym_cd))
										*paym.curr_standard)/paym.curr_rate);

				if (used_paym_cd == extra_paym_cd && 0.0 != extra_amount) {
				  tot_paym -= extra_amount;
				}

				//V3.4.7 acm -  { restar la donacion del medio de pago soles
				/*
				if (used_paym_cd==PAYM_WAY_1){
				 tot_paym-=fix_donation; 

				 fix_donation=0;
				}
				*/
				//V3.4.7 acm -  }  


				if ((has_valepavo) && (used_paym_cd == PAYM_WAY_3))
				{
					//  no hacer nada / el proceso ya imprimio
				}
				else
				{
					printf_log("Cuando tiene vale de pavo pasa por aqui para imprimir las formas de pago???");
					ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
					buffer[SMALL_INV_SIZE]=_T('\0');
					ftoa_price(tot_paym, TOTAL_BUF_SIZE, dummy);
					format_string(&string_price13, dummy);
					//3.4.7 acm -+++
					fmt_left_justify_string(buffer, 0, 39, paym.paym_descr);
					/*Aqui imprime los tipos de pago*/
					fmt_right_justify_string(buffer, 26, SMALL_INV_SIZE, string_price13.result);
					buffer[SMALL_INV_SIZE]=_T('\0');
					print_ln(printer, buffer);
				}
			}
			++used_paym_cd;
		}
    }

	/* DONACION */
    //if (pos_invoice.invoice_donation > 0.0) {
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    ftoa_price(get_invoice_donation_currmode()/*acm -*/, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price13, dummy);
    fmt_left_justify_string(buffer, 0, 39, prn_inv_TXT[57]);
    fmt_right_justify_string(buffer, 26, SMALL_INV_SIZE, string_price13.result);
    buffer[SMALL_INV_SIZE]=_T('\0');
    print_ln(printer, buffer);
    //}

	/* VUELTO */
    if (amount_due()!=0.0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      ftoa_price((amount_due()*-1), TOTAL_BUF_SIZE, dummy);
      format_string(&string_price13, dummy);
      fmt_left_justify_string(buffer, 0, 39, prn_inv_TXT[56]);
      fmt_right_justify_string(buffer, 26, SMALL_INV_SIZE, string_price13.result);
      buffer[SMALL_INV_SIZE]=_T('\0');
      print_ln(printer, buffer);
	}
	
	fmt_tot_f(dummy, de_total_d);
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));

	memset(aux, 0, sizeof(aux));
	memset(dummy_d, 0, sizeof(dummy_d));
	NoToLet_de(dummy, aux);

	memcpy(dummy_d, "Son: ", 5);
	memcpy(dummy_d + 5, aux, (int)strlen(aux));

	length = strlen(dummy_d);
	parts = length/size + 1;

	
  if((aux_too_long = calloc(parts, sizeof(char *))) == NULL)
    printf_log("malloc error 1[%d][%s]", __LINE__, __FILE__);

	for(i = 0; i < parts; i++)
	{
		if((aux_too_long[i] = calloc(size + 1, sizeof(char))) == NULL)
			printf_log("malloc error 2[%d][%s]", __LINE__, __FILE__);

		memcpy(aux_too_long[i], dummy_d + i * size, size);
	}	
	
	print_ln(printer, aux_too_long[0]);

	for(i = 1; i < parts; i++){
		print_ln(printer, aux_too_long[i]);
	}

	for(i = 0; i < parts; i++)
	{
		memset(aux_too_long[i], 0, size + 1);
		free(aux_too_long[i]);
	}
	
	memset(aux_too_long, 0, parts);
	free(aux_too_long);
		
    /* Reset the payment total which was increased with the amount change, in */
    /* the beginning of this function, to it's old value.                    */
    if ((short)tot_ret_double(TOT_CHANGE_CD) != 0) {
      tot_add_double((short)(TOT_PAYM_0+(short)tot_ret_double(TOT_CHANGE_CD)),
                     tot_ret_double(TOT_CHANGE));
    }

    print_ln(printer, prn_inv_TXT[99]); /* ----- */
  }

  return (SUCCEED);
} /* fn_i_small_total */


/*----------------------------------------------------------------------------*/
/*                            fn_i_small_tax                                  */
/*----------------------------------------------------------------------------*/
VOUCHER_VALE_DEF   voucher;
VOUCHER_ANTICIPO_DEF   voucher_anticipo;

short fn_i_small_tax(PRN_OBJECT *p_object, short action,
                     short offs_grp, short printer)
{
//  short    used_vat_cd;
  _TCHAR   buffer[SMALL_INV_SIZE+1];
  _TCHAR   dummy[TOTAL_BUF_SIZE+1];
//  _TCHAR   perc[7 + 1];
  _TCHAR   dummy1[64];
  _TCHAR   linvoice_buffer[100];
   long    linvoice_no; 
   int cc;


  _TCHAR **aux_too_long;
  int length = 0;
  int i;
  int parts; 
  int size = 25;

  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {
/*
    print_ln(printer, prn_inv_TXT[58]);
    used_vat_cd=0;

    while (used_vat_cd <= TOT_EXCL_9) {

      if (tot_ret_double(used_vat_cd) != 0.0) 
      {

        ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
        buffer[SMALL_INV_SIZE]=_T('\0');

        _stprintf(dummy, _T("%2d"), used_vat_cd % (short)10);
        fmt_right_justify_string(buffer, 0, 1, dummy);
        fmt_right_justify_string(buffer, 2, 2, "=");

        *perc = _T('\0');
        _stprintf(perc, _T("%02d"), (short)(get_vat_perc((short)(used_vat_cd % 10))+0.5));
        *(perc+2) = _T('\0');
        fmt_right_justify_string(buffer, 3, 4, perc);
        fmt_right_justify_string(buffer, 5, 5, "%");

        ftoa_price(floor_price(tot_ret_double((short)(used_vat_cd + TOT_INCL_0)) +
                   tot_ret_double((short)(MSAM_DISC_TOT_INCL_0+used_vat_cd))),
                   TOTAL_BUF_SIZE, dummy);
        format_string(&string_price11_star, dummy);
        fmt_right_justify_string(buffer, 7, 17, string_price11_star.result);

        ftoa_price(floor_price(tot_ret_double((short)(used_vat_cd + TOT_EXCL_0)) +
                   tot_ret_double((short)(MSAM_DISC_TOT_EXCL_0+used_vat_cd))),
                   TOTAL_BUF_SIZE, dummy);
        format_string(&string_price11_star, dummy);
        fmt_right_justify_string(buffer, 18, 28, string_price11_star.result);
      
        ftoa_price(floor_price(tot_ret_double((short)(used_vat_cd + TOT_VAT_0)) +
                   tot_ret_double((short)(MSAM_DISC_TOT_VAT_0+used_vat_cd))),
                   TOTAL_BUF_SIZE, dummy);
        format_string(&string_price11_star, dummy);
        fmt_right_justify_string(buffer, 29, 39, string_price11_star.result);
        print_ln(printer, buffer);
      }
      used_vat_cd++;
    }
    
        
    if ( IS_ANTICIPO())
    {
        print_ln(printer, " Anticipo");

        //PRINT TARIFA 0% Y 18%
        //////////////////////////////////////////////////////////////////////
        used_vat_cd=0;

        if (cur_anticipo.base_exo> 0.0) 
        {

                ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
                buffer[SMALL_INV_SIZE]=_T('\0');

                _stprintf(dummy, _T("%2d"), used_vat_cd % (short)10);
                fmt_right_justify_string(buffer, 0, 1, dummy);
                fmt_right_justify_string(buffer, 2, 2, "=");

                *perc = _T('\0');
                _stprintf(perc, _T("%02d"), (short)(get_vat_perc((short)(used_vat_cd % 10))+0.5));
                *(perc+2) = _T('\0');
                fmt_right_justify_string(buffer, 3, 4, perc);
                fmt_right_justify_string(buffer, 5, 5, "%");

                ftoa_price(floor_price(cur_anticipo.base_exo*-1),
                           TOTAL_BUF_SIZE, dummy);
                format_string(&string_price11_star, dummy);
                fmt_right_justify_string(buffer, 7, 17, string_price11_star.result);

                ftoa_price(floor_price(cur_anticipo.base_exo*-1),
                           TOTAL_BUF_SIZE, dummy);
                format_string(&string_price11_star, dummy);
                fmt_right_justify_string(buffer, 18, 28, string_price11_star.result);
      
                ftoa_price(floor_price(0),
                           TOTAL_BUF_SIZE, dummy);
                format_string(&string_price11_star, dummy);
                fmt_right_justify_string(buffer, 29, 39, string_price11_star.result);
                print_ln(printer, buffer);
        }

        used_vat_cd=2;

        if (cur_anticipo.base_imp> 0.0)
        {
                ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
                buffer[SMALL_INV_SIZE]=_T('\0');

                _stprintf(dummy, _T("%2d"), used_vat_cd % (short)10);
                fmt_right_justify_string(buffer, 0, 1, dummy);
                fmt_right_justify_string(buffer, 2, 2, "=");

                *perc = _T('\0');
                _stprintf(perc, _T("%02d"), (short)(get_vat_perc((short)(used_vat_cd % 10))+0.5));
                *(perc+2) = _T('\0');
                fmt_right_justify_string(buffer, 3, 4, perc);
                fmt_right_justify_string(buffer, 5, 5, "%");

                ftoa_price(floor_price((cur_anticipo.base_imp + cur_anticipo.igv)*-1),
                           TOTAL_BUF_SIZE, dummy);
                format_string(&string_price11_star, dummy);
                fmt_right_justify_string(buffer, 7, 17, string_price11_star.result);

                ftoa_price(floor_price(cur_anticipo.base_imp*-1),
                           TOTAL_BUF_SIZE, dummy);
                format_string(&string_price11_star, dummy);
                fmt_right_justify_string(buffer, 18, 28, string_price11_star.result);
      
                ftoa_price(floor_price(cur_anticipo.igv*-1),
                           TOTAL_BUF_SIZE, dummy);
                format_string(&string_price11_star, dummy);
                fmt_right_justify_string(buffer, 29, 39, string_price11_star.result);
                print_ln(printer, buffer);

        }
        //////////////////////////////////////////////////////////////////////
        ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
        buffer[SMALL_INV_SIZE]=_T('\0');
        fmt_left_justify_string(buffer,  0, 39, prn_inv_TXT[69]);
        print_ln(printer, buffer);

        ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
        buffer[SMALL_INV_SIZE]=_T('\0');
        fmt_left_justify_string(buffer,  0, 39, prn_inv_TXT[59]);

        
        //Compra
        ftoa_price(floor_price(
                    tot_ret_double(TOT_GEN_INCL) + 
                    floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))
                    -( cur_anticipo.base_imp+ 
                       cur_anticipo.base_exo+ 
                       cur_anticipo.igv)) ,
                   TOTAL_BUF_SIZE, dummy);
        format_string(&string_price11_star, dummy);
        fmt_right_justify_string(buffer, 7, 17, string_price11_star.result);
        //printf_ln(printer, "dummy 1[%s]", dummy);
        //memcpy(de_total, dummy, (int)strlen(dummy));
        //printf_ln(printer, "de_total[%s]", de_total);
        //Base
        ftoa_price(floor_price(
                    tot_ret_double(TOT_GEN_EXCL) + 
                    floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_EXCL))
                    -(cur_anticipo.base_imp+ 
                      cur_anticipo.base_exo)
                    ),
                   TOTAL_BUF_SIZE, dummy);
        format_string(&string_price11_star, dummy);
        fmt_right_justify_string(buffer, 18, 28, string_price11_star.result);
        
        //Igv
        ftoa_price(floor_price(
                    tot_ret_double(TOT_GEN_VAT) + 
                    floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_VAT))
                    -(cur_anticipo.igv)
                    ),
                   TOTAL_BUF_SIZE, dummy);
        format_string(&string_price11_star, dummy);
        fmt_right_justify_string(buffer, 29, 39, string_price11_star.result);
        //print_ln(printer, dummy);
        //memcpy(de_impuesto, dummy, (int)strlen(dummy));
        
        ///PERCEPCION

        if (cur_anticipo.percepcion> 0.0)
        {       

            print_ln(printer, prn_inv_TXT[51]); 

            sprintf(buffer," PERCEPCION TICKET   : %4.2f ",get_invoice_perception_currmode());
            print_ln(printer, buffer);
            
            sprintf(buffer," PERCEPCION ANTICIPO : %4.2f-",cur_anticipo.percepcion);
            print_ln(printer, buffer);
            
            sprintf(buffer,"                     --------");
            print_ln(printer, buffer);
            
            sprintf(buffer," TOTAL PERCEPCION    : %4.2f",
                    get_invoice_perception_currmode()-cur_anticipo.percepcion) ;
            print_ln(printer, buffer);
        }
    }else       
    {
        ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
        buffer[SMALL_INV_SIZE]=_T('\0');
        fmt_left_justify_string(buffer,  0, 39, prn_inv_TXT[69]);
        print_ln(printer, buffer);

        ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
        buffer[SMALL_INV_SIZE]=_T('\0');
        fmt_left_justify_string(buffer,  0, 39, prn_inv_TXT[59]);

        ftoa_price(floor_price(tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))),
                   TOTAL_BUF_SIZE, dummy);
        format_string(&string_price11_star, dummy);
        fmt_right_justify_string(buffer, 7, 17, string_price11_star.result);

        ftoa_price(floor_price(tot_ret_double(TOT_GEN_EXCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_EXCL))),
                   TOTAL_BUF_SIZE, dummy);       
        format_string(&string_price11_star, dummy);
        fmt_right_justify_string(buffer, 18, 28, string_price11_star.result);

        ftoa_price(floor_price(tot_ret_double(TOT_GEN_VAT) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_VAT))),
                   TOTAL_BUF_SIZE, dummy);
        format_string(&string_price11_star, dummy);
        fmt_right_justify_string(buffer, 29, 39, string_price11_star.result);
      
        print_ln(printer, buffer);
    }    
    
*/    
   // print_ln(printer, prn_inv_TXT[51]); /* --------------------------------------- */

	/* PASAPORTE No */
    _stprintf(buffer, prn_inv_TXT[60], cust.store_no, cust.cust_no);
    print_ln(printer, buffer);

	/* CAJA No */
    _stprintf(buffer, prn_inv_TXT[61], pos_invoice.invoice_till_no,
                                       pos_invoice.invoice_cashier_no);
    print_ln(printer, buffer);

	/* TOTAL ARTIC's */
    ftoa(tot_ret_double(TOT_PACKS), TOTAL_BUF_SIZE, dummy);
    //format_string(&string_packs6, dummy);
    _stprintf(buffer, prn_inv_TXT[62], dummy);
    print_ln(printer, buffer);

    if ( is_client_pay_FACTURA() ) //paga con factura
    //if(selected_invoice_printer== 0)
	{
		/*FACTURA*/
		/*_stprintf(buffer, prn_inv_TXT[76], pos_system.store_no, pos_system.small_inv_seq);
		print_ln(printer, buffer);*/
		memset(dummy1, 0, sizeof(dummy));
		_stprintf(dummy1, prn_inv_TXT[124], cust.name);
		memcpy(buffer, dummy1, SMALL_INV_SIZE);
		buffer[SMALL_INV_SIZE] = _T('\0');
    print_ln(printer, buffer);
		
		_stprintf(buffer, prn_inv_TXT[125], cust.fisc_no);
		print_ln(printer, buffer);

	}
    else 
	{
		/*BOLETA 2*/
		/*_stprintf(buffer, prn_inv_TXT[74], pos_system.store_no, pos_system.small_inv_seq);
        print_ln(printer, buffer);*/
    memset(dummy1, 0, sizeof(dummy));
    if(ispassday() && (de_total_d > 700.0)){
      if(IS_GEN_PERCEPTION)
      {
        _stprintf(dummy1, prn_inv_TXT[109], usr_perception_name);
      }
      else
      {
        _stprintf(dummy1, prn_inv_TXT[109], usr_name);
      }
    }
    else{
       if(IS_GEN_PERCEPTION)
      {
        _stprintf(dummy1, prn_inv_TXT[109], usr_perception_name);
      }
      else{
        _stprintf(dummy1, prn_inv_TXT[109], cust.name);
      }
    }
		memcpy(buffer, dummy1, SMALL_INV_SIZE);
		buffer[SMALL_INV_SIZE] = _T('\0');
    print_ln(printer, buffer);
		
    memset(dummy1, 0, sizeof(dummy));
    if(ispassday() && (de_total_d > 700.0)){
      if(IS_GEN_PERCEPTION)
      {
        _stprintf(dummy1, prn_inv_TXT[110], usr_perception_document);
      }
      else
      {
        _stprintf(dummy1, prn_inv_TXT[110], usr_document);
      }
    }
    else{
      if(IS_GEN_PERCEPTION)
      {
        _stprintf(dummy1, prn_inv_TXT[110], usr_perception_document);
      }
      else
      {
        _stprintf(dummy1, prn_inv_TXT[110], cust.fisc_no);      
      }
    }
    memcpy(buffer, dummy1, SMALL_INV_SIZE);
		buffer[SMALL_INV_SIZE] = _T('\0');
    print_ln(printer, buffer);
	}
       /* if (IS_GEN_PERCEPTION)
        {
		    
            _stprintf(buffer, prn_inv_TXT[94], usr_perception_document);
		    print_ln(printer, buffer);

		    _stprintf(buffer, prn_inv_TXT[95], usr_perception_name);
            print_ln(printer, buffer);
        
            //strcpy(usr_perception_document,data);
            //strcpy(usr_perception_name,data);
        }
	}
*/
	//ch_memset(buffer, _T(' '), SMALL_INV_SIZE *sizeof(_TCHAR));	
  if(ispassday())
  {
    _stprintf(buffer, prn_inv_TXT[111], "");
    print_ln(printer, buffer);
  }else{
    length = strlen(cust.address);
    parts = length/size + 1;

    if((aux_too_long = calloc(parts, sizeof(char *))) == NULL)
      printf_log("malloc error [%s][%d]", __FILE__, __LINE__);

    for(i = 0; i < parts; i++)
    {
      if((aux_too_long[i] = calloc(size + 1, sizeof(char))) == NULL)
        printf_log("malloc error [%s][%d]", __FILE__, __LINE__);

      memcpy(aux_too_long[i], cust.address + i * size, size);
    }	
    
    _stprintf(buffer, prn_inv_TXT[111], aux_too_long[0]);
    print_ln(printer, buffer);

    for(i = 1; i < parts; i++){
      _stprintf(buffer, prn_inv_TXT[123], aux_too_long[i]);
      print_ln(printer, buffer);
    }

    for(i = 0; i < parts; i++)
    {
      free(aux_too_long[i]);
    }
    free(aux_too_long);
  }

	printf_ln(printer, prn_inv_TXT[51]);

	ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
	buffer[SMALL_INV_SIZE]=_T('\0');
	if (_tcslen(hash_resp) >= 30) {
		hash_resp[30]=_T('\0');
	}
	fmt_centre_justify_string(buffer,  0, SMALL_INV_SIZE - 1, hash_resp);
	print_ln(printer, buffer);
	
	printf_ln(printer, prn_inv_TXT[51]);	
	
	//printf_ln(printer, prn_inv_TXT[117]);
	if(IS_GEN_PERCEPTION){
		printf_ln(printer, prn_inv_TXT[118]);
		printf_ln(printer, prn_inv_TXT[119], get_invoice_perception_currmode());
		printf_ln(printer, prn_inv_TXT[120], de_total_d + get_invoice_perception_currmode());
	}
	//printf_ln(printer, prn_inv_TXT[121]);
	//printf_ln(printer, prn_inv_TXT[122]);
	printf_ln(printer, prn_inv_TXT[99]);
	
   /* 27-Jan-2012 acm -{*/
   /* 27-Jan-2012 acm -}*/
    //print_ln(printer, buffer);

	

    // v3.5.1 acm -{
 //   if (IS_ANTICIPO())
 //   {
      //  get_invoice_no(linvoice_buffer);
        //linvoice_no=atol(linvoice_buffer);

        /*
       memset(&voucher_anticipo,0,sizeof(VOUCHER_ANTICIPO_DEF)); 
       cc=get_voucher_bo_anticipo(cur_anticipo.nro_seq,//.vale_no, 
                                  0,///cur_anticipo.vale_type, 
                                  READ_USED, 
                                  pos_system.store_no,
                                  cust.cust_no,
                                  pos_invoice.invoice_till_no , //till_no
                                  selected_invoice_printer +1, //invoice_type
                                  linvoice_no , //invoice_no
                                  pos_invoice.invoice_date, //invoice_date
                                  pos_invoice.invoice_sequence_no,
                                  &voucher_anticipo,3);

        if (cc!=SUCCEED)
        {
            //err_invoke(VALEPAVO_CONECTION_ERROR);
            //*data = _T('\0');
            //return(UNKNOWN_KEY);
            cc=cc;
        }
        */

        //if ( is_client_pay_FACTURA() )
       // {
	        //printf_ln(printer,"RazonSocial:%-28.28s" ,cur_anticipo.razon_social);
            //printf_ln(printer,"RUC        :%-20.20s" ,cur_anticipo.fisc_no);
//                               DNI CLIENTE         : %s
              //             ("RAZON SOCIAL:   \n%-30.30s")  /*JCP*/
      //      printf_ln(printer,"Nro FA      :%-20.20s" ,cur_anticipo.fac_no);
     //   }else {
	        //printf_ln(printer,"Nombre     :%-28.28s" ,cur_anticipo.razon_social);
            //printf_ln(printer,"DNI        :%-20.20s" ,cur_anticipo.fisc_no);
     //       printf_ln(printer,"Nro BOL     :%-20.20s" ,cur_anticipo.fac_no);
     //   }
        //print_ln(printer, empty);

        //++1printf_ln(printer,"Tipo Vale  :%s" ,(cur_valepavo.vale_type==1?"SAN FERNANDO":"ARO"));
        //++printf_ln(printer,"Nro Vale   :%d" ,cur_valepavo.vale_no);

//        init_invoice_user(NULL,0);
  //  }
    // v3.5.1 acm -}

    // v3.5.1 acm -{
	/* VALE PAVO */
    if ((get_num_vale_pavo()==1)&&  // se presiono tecla valepavo  y 
        (get_valeturkey_count()>=1 ) // v3.5.2 acm -
        )   // se selecciono un vale pavo   // v3.5.1 acm - 14-dic-2013
    {
       get_invoice_no(linvoice_buffer);
       linvoice_no=atol(linvoice_buffer);

       memset(&voucher,0,sizeof(VOUCHER_VALE_DEF)); 
       cc=get_voucher_bo_turkey(cur_valepavo.vale_no, 
                              cur_valepavo.vale_type, 
                              USED, 
                              pos_system.store_no,
                              cust.cust_no,
                              pos_invoice.invoice_till_no , //till_no
                              selected_invoice_printer +1, //invoice_type
                              linvoice_no , //invoice_no
                              pos_invoice.invoice_date, //invoice_date
                              pos_invoice.invoice_sequence_no,
                              &voucher,3);

        if (cc!=SUCCEED)
        {
            //err_invoke(VALEPAVO_CONECTION_ERROR);
            //*data = _T('\0');
            //return(UNKNOWN_KEY);
            cc=cc;
        }

        print_ln(printer, empty);
        printf_ln(printer,"RazonSocial:%-28.28s" ,cur_valepavo.name_cust);
        printf_ln(printer,"RUC/DNI    :%-20.20s" ,cur_valepavo.fisc_no);
        printf_ln(printer,"Nro Doc    :%-20.20s" ,cur_valepavo.fact_no);
        printf_ln(printer,"Tipo Vale  :%s" ,(cur_valepavo.vale_type==1?"SAN FERNANDO":"ARO"));
        printf_ln(printer,"Nro Vale   :%d" ,cur_valepavo.vale_no);

        //init_invoice_user(NULL,0);
    }
    // v3.5.1 acm -}
//++    init_invoice_user(NULL,0);
  }

  return (SUCCEED);
} /* fn_i_small_tax */



extern void  check_print_promotion(short printer);  // v3.5.0 acm -

/*----------------------------------------------------------------------------*/
/*                            fn_i_small_footer                               */
/*----------------------------------------------------------------------------*/
short fn_i_small_footer(PRN_OBJECT *p_object, short action,
                        short offs_grp, short printer)
{
  _TCHAR   prn_time[6];
  _TCHAR   prn_date[12 +1];
  _TCHAR   invoice_no[6 +1];
  _TCHAR   buffer[SMALL_INV_SIZE+1];

  long num_cupon =0; /* 27-Jan-2012 acm - */
  long num_gift  =0; /* 27-Jan-2012 acm - */
  double tot_incl=0; /* 27-Jan-2012 acm - */

  
  if (action == CALCULATE_IT) {
    p_object->ln = 0;       /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {

    print_ln(printer, empty);

    if(_tcslen(genvar.tax_txt4) > 0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      if (_tcslen(genvar.tax_txt4) >= 40) {
        genvar.tax_txt4[40]=_T('\0');
      }
      fmt_centre_justify_string(buffer,  0, 39, genvar.tax_txt4);
      print_ln(printer, buffer);
    }

    if(_tcslen(genvar.tax_txt5) > 0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      if (_tcslen(genvar.tax_txt5) >= 40) {
        genvar.tax_txt5[40]=_T('\0');
      }
      fmt_centre_justify_string(buffer,  0, 39, genvar.tax_txt5);
      print_ln(printer, buffer);
    }

    if(_tcslen(genvar.tax_txt6) > 0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      if (_tcslen(genvar.tax_txt6) >= 40) {
        genvar.tax_txt6[40]=_T('\0');
      }
      fmt_centre_justify_string(buffer,  0, 39, genvar.tax_txt6);
      print_ln(printer, buffer);
    }

    print_ln(printer, empty);
    
    if(is_detraction == 1){
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      fmt_centre_justify_string(buffer,  0, 39, "La venta contiene detraccion");
      print_ln(printer, buffer);
    }
    
/*mlsd*/
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');

    get_invoice_no(invoice_no);
    get_current_time(prn_time);
    format_string(&string_time5, prn_time);
    prn_fmt_date(pos_system.run_date, genvar.date_format, mon_names, prn_date);

    _stprintf(buffer, prn_inv_TXT[50], prn_date, string_time5.result,
                                       pos_system.store_no, pos_system.till_no,
                                       pos_system.run_date/10000, invoice_no);
    print_ln(printer, buffer);

	print_ln(printer, prn_inv_TXT[99]); /* ----- */

   /* 27-Jan-2012 acm -{*/  /* add promocion */
    if (isvigente_promotion())  /* AC2012-003 acm - */
    {
      /*mlsd aca no entro a chequear la promocion */
      check_print_promotion(printer);

      tot_incl = tot_ret_double(TOT_GEN_INCL) + floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL));

      num_cupon = pos_invoice.invoice_cupon;
      num_gift  = pos_invoice.invoice_gift; //get_num_gift(tot_incl);

      if (!ispassday())
      {
          if (num_cupon == 0) 
          {
            print_ln(printer, empty);
            print_ln_fmt_centre(printer, 40,prn_inv_TXT[79]);// PARTICIPE EN LA PROMOCION ANIVERSARIO
          }
          else
          {
            print_ln(printer, empty);
            if (num_cupon == 1)
            {
              print_ln_fmt_centre(printer, 40,prn_inv_TXT[80]);             /* 0080 */ //,_T("PROMOCION ANIVERSARIO") 
              print_ln_fmt_centre(printer, 40,prn_inv_TXT[82],num_cupon);   /* 0082 */ /*,_T("USTED HA GANADO %d CUPON")    */
            }else{
              print_ln_fmt_centre(printer, 40,prn_inv_TXT[80]);             /* 0080 */ //,_T("PROMOCION ANIVERSARIO") 
              print_ln_fmt_centre(printer, 40,prn_inv_TXT[81],num_cupon);   /* 0081 */ /*,_T("USTED HA GANADO %d CUPONES")  */
            }
          }

          //num_gift =get_num_gift(tot_incl);
          /* -- FIX 3.4.2. 27-JUL-2012 acm - 
          if (num_gift  >= 1){
              print_ln(printer, empty);
              print_ln_fmt_centre(printer, 40,prn_inv_TXT[83]);           
          }
          */
      }
    } else 
    {
      /*mlsd Aqui ingresa para chequear por promociones e imprimirlas */
      check_print_promotion(printer);
    }
   /* 27-Jan-2012 acm -}*/

    //print_ln(printer, empty);

	
	/*mlsd*/
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
	fmt_centre_justify_string(buffer,  0, 39, prn_inv_TXT[112]);
    print_ln(printer, buffer);
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
	fmt_centre_justify_string(buffer,  0, 39, prn_inv_TXT[113]);
    print_ln(printer, buffer);
	
	print_ln(printer, empty);
	
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
	fmt_centre_justify_string(buffer,  0, 39, prn_inv_TXT[114]);
    print_ln(printer, buffer);
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
  if(is_client_pay_FACTURA()){
    fmt_centre_justify_string(buffer,  0, 39, prn_inv_TXT[126]);
  }
  else{
    fmt_centre_justify_string(buffer,  0, 39, prn_inv_TXT[115]);
  }  
    print_ln(printer, buffer);
	
	print_ln(printer, empty);
	
	ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
	fmt_centre_justify_string(buffer,  0, 39, prn_inv_TXT[116]);
    print_ln(printer, buffer);
	
    if(_tcslen(genvar.prom_txt_bot3) > 0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      if (_tcslen(genvar.prom_txt_bot3) >= 40) {
        genvar.prom_txt_bot3[40]=_T('\0');
      }
      fmt_centre_justify_string(buffer,  0, 39, genvar.prom_txt_bot3);
      print_ln(printer, buffer);
    }

    if(_tcslen(genvar.prom_txt_bot4) > 0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      if (_tcslen(genvar.prom_txt_bot4) >= 40) {
        genvar.prom_txt_bot4[40]=_T('\0');
      }
      fmt_centre_justify_string(buffer,  0, 39, genvar.prom_txt_bot4);
      print_ln(printer, buffer);
    }

    if(_tcslen(genvar.prom_txt_bot5) > 0) {
      ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
      buffer[SMALL_INV_SIZE]=_T('\0');
      if (_tcslen(genvar.prom_txt_bot5) >= 40) {
        genvar.prom_txt_bot5[40]=_T('\0');
      }
      fmt_centre_justify_string(buffer,  0, 39, genvar.prom_txt_bot5);
      print_ln(printer, buffer);
    }

    print_cut(printer);
  }

	return (SUCCEED);
} /* fn_i_small_footer */


/*----------------------------------------------------------------------------*/
/*                            fn_i_small_void                                 */
/* NOT USED : Voided invoice will be done on NORMAL printer!!!                */
/*----------------------------------------------------------------------------*/
short fn_i_small_void(PRN_OBJECT *p_object, short action,
                      short offs_grp, short printer)
{
  short   i;

  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {
    print_ln(printer, empty);
    for (i=0; i < 3; i++) {
      print_ln(printer, prn_inv_TXT[63]);
      print_ln(printer, empty);
    }
  }

  return (SUCCEED);
} /* fn_i_small_void */


/*----------------------------------------------------------------------------*/
/*                            fn_i_small_promo                                */
/*----------------------------------------------------------------------------*/
short fn_i_small_promo(PRN_OBJECT *p_object, short action,
                       short offs_grp, short printer)
{
  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {

    if (train_mode == CASH_NORMAL) {
      if (copy_invoice_active == YES || bot_copy_invoice_active == YES) {
        print_ln(printer, empty);
        print_ln(printer, prn_inv_TXT[65]);
        print_ln(printer, prn_inv_TXT[71]);
        print_ln(printer, prn_inv_TXT[65]);
      }
    }
    else {
      print_ln(printer, empty);
      print_ln(printer, prn_inv_TXT[64]);
      print_ln(printer, prn_inv_TXT[70]);
      print_ln(printer, prn_inv_TXT[64]);
  
      if (copy_invoice_active == YES) {
        print_ln(printer, empty);
        print_ln(printer, prn_inv_TXT[65]);
        print_ln(printer, prn_inv_TXT[71]);
        print_ln(printer, prn_inv_TXT[65]);
      }
    }
  }

  return (SUCCEED);
} /* fn_i_small_promo */


/*----------------------------------------------------------------------------*/
/*                            fn_i_small_cust_fee                             */
/*----------------------------------------------------------------------------*/
short fn_i_small_cust_fee(PRN_OBJECT *p_object, short action,
                         short offs_grp, short printer)
{
  _TCHAR   buffer[SMALL_INV_SIZE+1];
  _TCHAR   dummy[SMALL_INV_SIZE+1];
  _TCHAR   perc[7+1];
  long     ldate;
  short    remaining = FALSE;
  double   tot_fee_amount;


  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {

    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    _tcscpy(dummy, genvar.fee_descr);
    if (_tcslen(dummy) >= 33) {
      dummy[33]=_T('\0');
    }
    fmt_left_justify_string(buffer, 7, 39, dummy); 
    print_ln(printer, buffer);

    /*
       Determine now the expiry date for this customer depending on
       the cancelling or paying of the due fee.
    */
    if (pos_invoice.invoice_fee_status == FEE_STAT_CANCEL_YEAR) {
      /* The fee payment is overruled for one year 'exp-date += 1 year' */
      ldate = pos_invoice.invoice_date + 10000;
    }
    else if (pos_invoice.invoice_fee_status == FEE_STAT_CANCEL) {
       /* The fee payment is overruled for this time only */
      ldate = cust.exp_date;
    }
    else {
       /* The fee is paid 'exp-date += 1 year*/
      ldate = pos_invoice.invoice_date + 10000;
    }
    prn_fmt_date(ldate, genvar.date_format, mon_names, dummy);

    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    if (cust.card_type_no != genvar.daypass_card_type_no) {
      fmt_left_justify_string(buffer, 7, 20, dummy); 
    }
    else { /* DAY PASS on invoice i.s.o date + vat perc. */
      fmt_left_justify_string(buffer, 7, 20, menu_TXT[43]); 

      *perc = _T('\0');
      fmt_vat_perc(get_vat_perc(genvar.cfee_vat_no), perc);
      fmt_right_justify_string(buffer, 21, 27, perc);
    }

    if (genvar.price_incl_vat == INCLUSIVE) {
      tot_fee_amount = floor_price(calc_incl_vat(tot_ret_double(TOT_FEE_AMOUNT), genvar.cfee_vat_no));
    }
    else {
      tot_fee_amount = tot_ret_double(TOT_FEE_AMOUNT);
    }

    ftoa_price(tot_fee_amount, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 29, 39, string_price11_2.result);

    print_ln(printer, buffer);
  }

  return (SUCCEED);
} /* fn_i_small_cust_fee */


/*----------------------------------------------------------------------------*/
/*                            fn_i_small_msam_total                           */
/*----------------------------------------------------------------------------*/
short fn_i_small_msam_total(PRN_OBJECT *p_object, short action,
                            short offs_grp, short printer)
{
  _TCHAR  buffer[SMALL_INV_SIZE+1];
  _TCHAR  dummy[TOTAL_BUF_SIZE+1];
  double  tot_disc_amount;

  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');
    
    fmt_left_justify_string(buffer, 0, 39, prn_inv_TXT[67]); 
    	  
    tot_disc_amount = (genvar.price_incl_vat == INCLUSIVE) ? floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_INCL))
                                                           : floor_price(tot_ret_double(MSAM_DISC_TOT_GEN_EXCL));
    ftoa_price(floor_price(tot_disc_amount), TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 29, 39, string_price11_2.result);

    print_ln(printer, buffer);
  }

  return (SUCCEED);
}  /* fn_i_small_msam_total */

/*----------------------------------------------------------------------------*/
/*                            fn_i_small_msam_line                            */
/*----------------------------------------------------------------------------*/
short fn_i_small_msam_line(PRN_OBJECT *p_object, short action,
                           short offs_grp, short printer)
{ 
  _TCHAR   buffer[SMALL_INV_SIZE+1];
  _TCHAR   dummy[INV_SIZE+1];
  short    remaining = FALSE;
  double   disc_amount;
  short    length_left, i;

  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {
    ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
    buffer[SMALL_INV_SIZE]=_T('\0');

    if (prn_msam_action.times) {
      ftoa(prn_msam_action.times, 9, dummy);
      format_string(&string_qty4, dummy);
      fmt_right_justify_string(buffer, 3, 6, string_qty4.result);
    } 

    i=0;
    do {
      length_left = _tcslen(&prn_msam_action.line_text[i*33]);
      _tcsncpy(dummy, &prn_msam_action.line_text[i*33], 33);
      dummy[33]=_T('\0');
      fmt_left_justify_string(buffer, 7, 39, dummy);
      if(_tcslen(dummy) > 33-12) {
        print_ln(printer, buffer);
        ch_memset(buffer, _T(' '), SMALL_INV_SIZE*sizeof(_TCHAR));
        buffer[SMALL_INV_SIZE]=_T('\0');
      }
      i++;
    } while (length_left>33);

    disc_amount = (genvar.price_incl_vat == INCLUSIVE) ? prn_msam_action.disc_incl
                                                       : prn_msam_action.disc_excl;
    disc_amount *= -1.0;
//mlsd    desc_item[desc_items].desc = disc_amount;
//mlsd    memcpy(desc_item[desc_items].descr, prn_msam_action.line_text, (int)strlen(prn_msam_action.line_text));
//mlsd    desc_items++;
    ftoa_price(disc_amount, TOTAL_BUF_SIZE, dummy);
    format_string(&string_price11_2, dummy);
    fmt_right_justify_string(buffer, 28, 28, _T(" "));
    fmt_right_justify_string(buffer, 29, 39, string_price11_2.result);
    print_ln(printer, buffer);
  }  

  return(SUCCEED);
} /* fn_i_small_msam_line */


/*----------------------------------------------------------------------------*/
/*                            fn_i_small_msam_header                          */
/*----------------------------------------------------------------------------*/
short fn_i_small_msam_header(PRN_OBJECT *p_object, short action,
                             short offs_grp, short printer)
{
  if (action == CALCULATE_IT) {
    p_object->ln = 0;                     /* Never calculate for smallprinter */
  }

  if (action == PRINT_IT) {
    print_ln(printer, prn_inv_TXT[99]); /* ----- */
    print_ln(printer, prn_inv_TXT[66]);
  }

  return (SUCCEED);
} /* fn_i_small_msam_header */

/*----------------------------------------------------------------------------*/
/*                              add_left_margin                               */
/*----------------------------------------------------------------------------*/
static void add_left_margin(_TCHAR *buffer)
{
  short  i;
  _TCHAR *ptr;

  if (LEFT_MARGIN > 0) {

    ptr = buffer + _tcslen(buffer) -1;
    if (ptr) {
      while (*ptr==_T(' ')) {
        *ptr=_T('\0');
        ptr--;
      }
    }
   
    _tcsrev(buffer);

    for (i=0;i<LEFT_MARGIN;i++) {
      _tcscat(buffer, _T(" "));
    }
    _tcsrev(buffer);
  }
} /* add_left_margin */



/*********************************************/
/*funcion que convierte de numeros a palabras*/
/*********************************************/
char *NoToLet(const char *A){ 
   const char *numero=A;
   char decimal[3], entero[50];
   char Letras[140]; 
   static char Letras_send[145]; /*12-Ago-2011 acm - fixed bug error*/
   int  p,l=0,des; 

   char N1='0',N2='0',N3='0'; 
   int ancho;
   int ancho2;
   int ancho3=strlen(numero); 

   p=0; 
   
   strncpy(decimal,numero+(ancho3-2),2);
   decimal[2]='\0';

   /*printf("decimal [%s] \n",decimal);*/
   
   strncpy(entero,numero,ancho3-2);
   entero[ancho3-2]='\0';
   /*printf("entero [%s] \n",entero);*/

   ancho = ancho2 = strlen(entero);

   Letras[0]='\0'; 
   while(l<=ancho-1){ 

      if(ancho2-3==0 || ancho2-6==0 || ancho2-9==0){ 
         p=3; 
         des=3; 
      } 
      if(ancho2-3==-1 || ancho2-6==-1 || ancho2-9==-1){ 
         p=2; 
         des=2; 
      } 
      if(ancho2-3==-2 || ancho2-6==-2 || ancho2-9==-2){ 
         p=1; 
         des=1; 
      } 
///////////////////// numeros de Cienes. 
      if(p==3){ 
         N1=numero[l]; 
         N2=numero[l+1]; 
         N3=numero[l+2]; 
         switch(N1){ 
            case '0': 
                  l++; 
                  p--; 
                  break; 
            case '1': 
                  if(N2=='0' && N3=='0'){ 
                     strcat(Letras," Cien "); 
                     l+=2; 
                     p-=2; 
                  }else{ 
                     strcat(Letras," Ciento "); 
                     l++; 
                     p--; 
                  } 
                  break; 
            case '2': 
                  strcat(Letras," Doscientos "); 
                  l++; 
                  p--; 
                  break; 
            case '3': 
                  strcat(Letras," Trescientos "); 
                  l++; 
                  p--; 
                  break; 
            case '4': 
                  strcat(Letras," Cuatroscientos "); 
                  l++; 
                  p--; 
                  break; 
            case '5': 
                  strcat(Letras," Quinientos "); 
                  l++; 
                  p--; 
                  break; 
            case '6': 
                  strcat(Letras," Seiscientos "); 
                  l++; 
                  p--; 
                  break; 
            case '7': 
                  strcat(Letras," Setecientos "); 
                  l++; 
                  p--; 
                  break; 
            case '8': 
                  strcat(Letras," Ochocientos "); 
                  l++; 
                  p--; 
                  break; 
            case '9': 
                  strcat(Letras," Novecientos "); 
                  l++; 
                  p--; 
                  break; 
         } 
      } 
///////////////////// numeros de decenas. 
      if(p==2){ 
         N1=numero[l]; 
         N2=numero[l+1]; 
         switch(N1){ 
            case '0': 
                  l++; 
                  p--; 
                  break; 
            case '1': 
                  if(N2!='0'){ 
                     switch(N2){ 
                        case '1':strcat(Letras," Once "); 
                              break; 
                        case '2':strcat(Letras," Doce "); 
                              break; 
                        case '3':strcat(Letras," Trece "); 
                              break; 
                        case '4':strcat(Letras," Catorce "); 
                              break; 
                        case '5':strcat(Letras," Quince "); 
                              break; 
                        case '6':strcat(Letras," Diesciseis "); 
                              break; 
                        case '7':strcat(Letras," Diescisiete "); 
                              break; 
                        case '8':strcat(Letras," Diesciocho "); 
                              break; 
                        case '9':strcat(Letras," Diescinueve "); 
                              break; 
                     } 
                  }else{ 
                     strcat(Letras," Diez "); 
                  } 
                  l+=2; 
                  p-=2; 
                  break; 
            case '2': 
                  if(N2!='0'){ 
                     strcat(Letras," Veinti"); 
                     l++; 
                     p--; 
                  }else{ 
                     strcat(Letras," Veinte "); 
                     l+=2; 
                     p-=2; 
                  } 
                  break; 
            case '3':if(N2!='0'){ 
                     strcat(Letras," Treinta y "); 
                     l++; 
                     p--; 
                  }else{ 
                     strcat(Letras," Treinta "); 
                     l+=2; 
                     p-=2; 
                  } 
                  break; 
            case '4':if(N2!='0'){ 
                     strcat(Letras," Cuarenta y "); 
                     l++; 
                     p--; 
                  }else{ 
                     strcat(Letras," Cuarenta "); 
                     l+=2; 
                     p-=2; 
                  } 
                  break; 
            case '5':if(N2!='0'){ 
                     strcat(Letras," Cincuenta y "); 
                     l++; 
                     p--; 
                  }else{ 
                     strcat(Letras," Cincuenta "); 
                     l+=2; 
                     p-=2; 
                  } 
                  break; 
            case '6':if(N2!='0'){ 
                     strcat(Letras," Sesenta y "); 
                     l++; 
                     p--; 
                  }else{ 
                     strcat(Letras," Sesenta "); 
                     l+=2; 
                     p-=2; 
                  } 
                  break; 
            case '7':if(N2!='0'){ 
                     strcat(Letras," Setenta y "); 
                     l++; 
                     p--; 
                  }else{ 
                     strcat(Letras," Setenta "); 
                     l+=2; 
                     p-=2; 
                  } 
                  break; 
            case '8':if(N2!='0'){ 
                     strcat(Letras," Ochenta y "); 
                     l++; 
                     p--; 
                  }else{ 
                     strcat(Letras," Ochenta "); 
                     l+=2; 
                     p-=2; 
                  } 
                  break; 
            case '9':if(N2!='0'){ 
                     strcat(Letras," Noventa y "); 
                     l++; 
                     p--; 
                  }else{ 
                     strcat(Letras," Noventa "); 
                     l+=2; 
                     p-=2; 
                  } 
                  break; 
         } 
      } 
///////////////////// numeros ordinarios. 
      if(p==1){ 
         N1=numero[l]; 
         switch(N1){ 
            case '0': 
                  break; 
            case '1': 
                  strcat(Letras,"Uno "); 
                  break; 
            case '2':strcat(Letras,"Dos "); 
                  break; 
            case '3':strcat(Letras,"Tres "); 
                  break; 
            case '4':strcat(Letras,"Cuatro "); 
                  break; 
            case '5':strcat(Letras,"Cinco "); 
                  break; 
            case '6':strcat(Letras,"Seis "); 
                  break; 
            case '7':strcat(Letras,"Siete "); 
                  break; 
            case '8':strcat(Letras,"Ocho "); 
                  break; 
            case '9':strcat(Letras,"Nueve "); 
                  break; 
         } 
         l++; 
      } 
      // miles 
      if(N1!='0'){ 
         if(ancho2==11 || ancho2==12 || ancho2==10) 
            if (ancho2==7 && N1=='1') 
               strcpy(Letras," Un Billon "); 
             else 
               strcat(Letras," Billones "); 


         if(ancho2==8 || ancho2==9 || ancho2==7) 
            if (ancho2==7 && N1=='1') 
               strcpy(Letras," Un Millon "); 
             else 
               strcat(Letras," Millones "); 
         if(ancho2==5 || ancho2==6 || ancho2==4) 
            if (ancho2==4 && N1=='1') 
               strcpy(Letras," Mil "); 
             else 
               strcat(Letras," Mil "); 
      } 
      ancho2-=des; 
   } 
  
   strcpy(Letras_send,"     Son: ");
   strcat(Letras_send,Letras);
   strcat(Letras_send," con ");
   strcat(Letras_send,decimal);
   strcat(Letras_send,"/100");
   strcat(Letras_send,"  Soles");
   return(Letras_send); 
}

