/*
 *     Module Name       : Pos_chq.C
 *
 *     Type              : Print functions for cheque printers
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
 * 29-Nov-2000 Initial Release WinPOS                                  R.N.B.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <stdio.h>                          /* System include files.         */
#include <math.h>
                                            /* POS (library) include files.  */
#include "registry.h"
#include "appbase.h"
#include "comm_tls.h"
#include "stri_tls.h"
#include "tot_mgr.h"
#include "err_mgr.h"
#include "tm_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "OPrn_mgr.h"

#include "pos_inp.h"                        /* Application include files.    */
#include "pos_txt.h"
#include "pos_tm.h"
#include "pos_tot.h"
#include "wpos_mn.h"
#include "pos_func.h"
#include "st_main.h"
#include "pos_tran.h"
#include "pos_errs.h"
#include "Pos_chq.h"

#define NEW_LINE  _T("\n")

struct ERR_TABLE {
  short opos_error;                      /* Map OPOS errors to ...          */
  short prn_error;                       /* ...printer manager error        */
} err_table[] = {
   -1,                  PRN_UNKNOWN_ERR   /* always first!                  */
 , OPOS_SUCCESS,        -1                /* ignore                         */
 , SUCCEED,             -1                /* ignore                         */
 , FAIL,                -1                /* ignore                         */
 , OPOS_E_CLOSED,       PRN_INIT_ERROR    /* deviced is not properly inited */
 , OPOS_E_NOSERVICE,    PRN_INIT_ERROR    /* deviced is not properly inited */
 , OPOS_E_NOHARDWARE,   PRN_INIT_ERROR    /* deviced is not properly inited */
 , OPOS_E_CLAIMED,      PRN_INIT_ERROR    /* deviced is not properly inited */
 , OPOS_E_NOTCLAIMED,   PRN_INIT_ERROR    /* deviced is not properly inited */
 , OPOS_E_DISABLED,     PRN_INIT_ERROR    /* deviced is not properly inited */
 , OPOS_E_ILLEGAL,      PRN_METHOD_ERR
 , OPOS_E_FAILURE,      PRN_METHOD_ERR
 , OPRN_INVALID_PRINTER,PRN_METHOD_ERR
 , OPOS_E_OFFLINE,      PRN_OFFLINE       /* device is offline              */
 , OPOS_E_TIMEOUT,      PRN_TIMEOUT
 , OPOS_E_BUSY,         PRN_BUSY       
 , OPOS_EPTR_COVER_OPEN,PRN_COVER_OPEN 
 , OPOS_EPTR_JRN_EMPTY, PRN_NO_PAPER   
 , OPOS_EPTR_REC_EMPTY, PRN_NO_PAPER   
 , OPOS_EPTR_SLP_EMPTY, PRN_NO_PAPER   
 , OPOS_EPTR_SLP_FORM,  PRN_NOT_EMPTY  
};

static void  process_cheque_anum_line(_TCHAR *, short, short, short, LOGICAL_PRINTER);
static short process_cheque_anum(_TCHAR *, _TCHAR, LOGICAL_PRINTER);
static void  chq_err_handler(long);
static long  insert_slip(LOGICAL_PRINTER);
static long  remove_slip(LOGICAL_PRINTER);
static long  set_sideways_mode(LOGICAL_PRINTER, long);


/*----------------------------------------------------------------------------*/
/*                         init_cheque_printers                               */
/*----------------------------------------------------------------------------*/
short init_cheque_printers(void)
{
  _TCHAR  Val[100];

  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("CHQ_PRINTER1_ATTACHED"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
  if (_tcsicmp(Val, _T("YES"))==0) {
    return oprn_init(SLIP_PRINTER1);
  }


  return SUCCEED;
} /* Init_cheque_printers */


/*----------------------------------------------------------------------------*/
/*                         print_ch_side_1                                    */
/*----------------------------------------------------------------------------*/
/* This function prints several lines on side one of a cheque.                */
/*----------------------------------------------------------------------------*/
short print_ch_side_1(LOGICAL_PRINTER printer)
{
  short   no_lines;
  _TCHAR  buffer[256];
  _TCHAR  buffer1[64];
  _TCHAR  buffer2[64];
  _TCHAR  invoice_no[6 + 1];
  _TCHAR *result;
  _TCHAR *p;
  long    status;

  no_lines=0;

  
  if ((status = insert_slip(printer)) != OPOS_SUCCESS) {
    goto error;
  }

  if ((status=set_sideways_mode(printer, PTR_RP_RIGHT90)) != OPOS_SUCCESS) {
    goto error;
  }
                                                       /* start buffered printing */
  /* Add or remove NEW_LINE to adjust the height from the printing of the cheque    */
//  oprn_print_normal(printer, NEW_LINE NEW_LINE);
//                             NEW_LINE NEW_LINE NEW_LINE 
//                             NEW_LINE NEW_LINE NEW_LINE);


             /* Get amount and remove leading and trailing spaces.                */
  ftoa_price(cheque_amount, 18, buffer2);
  format_string(&string_chequ11, buffer2);

  p = _tcstok(string_chequ11.result, _T(" "));
  if (_tcslen(prn_ch_TXT[5]) <= 10) {
    _tcscpy(buffer1, prn_ch_TXT[5]);  /* "****" */
    _tcscat(buffer1, p ? p : _T(""));
  } 
  else {
    _tcscpy(buffer1, p ? p : _T(""));
  }
  _stprintf(buffer, _T("%*.*s"), CH_WIDTH-2, CH_WIDTH-2, buffer1);
//  oprn_print_normal(printer, NEW_LINE);  /* line 1 */
  oprn_print_normal(printer, buffer);    
  oprn_print_normal(printer, NEW_LINE);

  /* Si monto mayor 999999 se queda 999999 LA */
  if (cust.appr_cheque_amount > 999999.99) {
    cust.appr_cheque_amount = 999999.99;
  }

  _stprintf(buffer, prn_ch_TXT[6]); /* NO ENDOSABLE") */
  oprn_print_normal(printer, buffer);
  oprn_print_normal(printer, NEW_LINE NEW_LINE);

  /* Print autorized cheque amount RvdB. */
  ftoa_price(floor(cust.appr_cheque_amount / 1000.0), 18, buffer1);
  format_string(&string_chequ11, buffer1);
  p = _tcstok(string_chequ11.result, _T(" "));

  /*PARA IMPRIMIR CLIENTE,APROBACION,No.FACTURA BAJO NO ENDOSABLE 31/05/95*/
  get_invoice_no(invoice_no);
  _stprintf(buffer, prn_ch_TXT[7], pos_invoice.invoice_cashier_no, 
	                                 pos_invoice.invoice_till_no,
                								   cust.store_no, cust.cust_no, p,
                                   pos_system.store_no,invoice_no);
  oprn_print_normal(printer, buffer);
  oprn_print_normal(printer, NEW_LINE NEW_LINE);

  oprn_print_normal(printer, prn_ch_TXT[1]);
  oprn_print_normal(printer, NEW_LINE NEW_LINE);

  ftoa_price(cheque_amount,18,buffer1);
  result = num_to_alpha(buffer1, _T('@'));
  if (*result) {
    no_lines = process_cheque_anum(result, _T('@'), printer);
  }
  if (no_lines == 0) {    /* No conversion performed, print numerical. */
    _stprintf(buffer, prn_ch_TXT[3], buffer2);
    oprn_print_normal(printer, buffer);
    oprn_print_normal(printer, NEW_LINE);
  }
  else if (no_lines == 1) {
    process_cheque_anum_line(empty, CH_ANUM_OFFS_2, CH_ANUM_SIZE_2, CH_ANUM_OVFLW_2, printer);
  }

  oprn_print_normal(printer, NEW_LINE);
  ftoa(pos_system.run_date, 10, buffer1);
  _tcsncpy(buffer2 ,buffer1+6,2);
  buffer2[2] = _T('/');
  _tcsncpy(buffer2+3 ,buffer1+4,2);
  buffer2[5] = _T('\0');
  buffer1[4] = _T('\0');
  _stprintf(buffer, prn_ch_TXT[2], genvar.town, buffer2, buffer1);
  oprn_print_normal(printer, buffer);

  /* Add or remove NEW_LINE to adjust the height from the printing of the cheque    */
  oprn_print_normal(printer, NEW_LINE NEW_LINE NEW_LINE);
  oprn_print_normal(printer, NEW_LINE NEW_LINE NEW_LINE);
  oprn_print_normal(printer, NEW_LINE NEW_LINE NEW_LINE);

                                           /* print the buffered lines */
  if ((status=set_sideways_mode(printer, PTR_RP_NORMAL)) != OPOS_SUCCESS) {
    goto error;
  }

  if ((status=remove_slip(printer)) != OPOS_SUCCESS) {
    goto error;
  }

  return(SUCCEED);

error:
  chq_err_handler(status);
  oprn_clear_output(printer);
  cheque_amount = 0.0;       /* I reset to 0 so that I can see in */
                             /* vfy_total that the printing did   */
                             /* succeed cannot pass status from   */
                             /* module to higher modules. So use  */
                             /* this sick trick!       RvdB       */
  return (FAIL);
} /* print_ch_side_1() */


/*---------------------------------------------------------------------------*/
/*                             process_cheque_anum_line                      */
/*---------------------------------------------------------------------------*/
/* Prints an alphanumeric line recieved from proc_cheque_anum().             */
/*---------------------------------------------------------------------------*/
  static void
process_cheque_anum_line(_TCHAR *line, short offset, short maxlen, short ovflw, LOGICAL_PRINTER printer)
{
  _TCHAR  buffer[256];

  assert(_tcslen(line)+offset <= 256);
  assert(maxlen < 256-2);

  ch_memset(buffer, _T(' '), offset*sizeof(_TCHAR));
  ch_memset(buffer+offset, _T('-'), (maxlen-ovflw)*sizeof(_TCHAR));
  ch_memset(buffer+offset+maxlen-ovflw, _T(' '), ovflw*sizeof(_TCHAR));
  memcpy(buffer+offset, line, _tcslen(line)*sizeof(_TCHAR));
  buffer[maxlen+offset] = _T('\n');
  buffer[maxlen+offset+1] = _T('\0');

  oprn_print_normal(printer, buffer);

  return;
} /* process_cheque_anum_line */

/*---------------------------------------------------------------------------*/
/*                             process_cheque_anum                           */
/*---------------------------------------------------------------------------*/
/* Prints the alphanumeric result from the function num_to_alpha().          */
/* It ensures printing of non-broken words.                                  */
/*---------------------------------------------------------------------------*/
  static short
process_cheque_anum(_TCHAR *result, _TCHAR break_char, LOGICAL_PRINTER printer)
{
  short   cur_ln;
  _TCHAR *rp;
  _TCHAR  break_str[2];
  _TCHAR  line[256];

  /* Print alphanumeric lines.                                             */

  *line = _T('\0');
  break_str[0] = break_char;
  break_str[1] = _T('\0');
  cur_ln = 0;
  if (*result) {
    /* First line.                                                         */
    for (rp=result; _tcschr(break_str, *rp) != NULL; rp++);
    while (*rp != _T('\0') &&
           ((signed)_tcslen(line))+word_len(rp, break_str) <= CH_ANUM_SIZE_1) {
      word_cat(line, rp, break_str);
      rp = word_skip(rp, break_str);
    }
    /* If there is something to print and the rest fits on the seccond     */
    /* line, print it and process seccond line.                            */
    if (*line != _T('\0') &&
        line_len(rp, break_str) < CH_ANUM_SIZE_2) {

      process_cheque_anum_line(line, CH_ANUM_OFFS_1, CH_ANUM_SIZE_1, CH_ANUM_OVFLW_1, printer);
      oprn_print_normal(printer, NEW_LINE);
      cur_ln = 1;
      *line = _T('\0');

      /* Seccond and last line.                                            */
      while (*rp != _T('\0') &&
             ((signed)_tcslen(line))+word_len(rp, break_str) <= CH_ANUM_SIZE_2) {
        word_cat(line, rp, break_str);
        rp = word_skip(rp, break_str);
      }
      if (*line != _T('\0')) {
        process_cheque_anum_line(line, CH_ANUM_OFFS_2, CH_ANUM_SIZE_2, CH_ANUM_OVFLW_2, printer);
        cur_ln = 2;
      }
    }
  }

  return(cur_ln);
} /* process_cheque_anum */


/*----------------------------------------------------------------------------*/
/*                         insert_slip                                        */
/*----------------------------------------------------------------------------*/
long insert_slip(LOGICAL_PRINTER printer)
{
  long status;

  status = oprn_wait_for_slip(printer, 0);                      /* open jaws */
  if (status==OPOS_SUCCESS || status==OPOS_E_TIMEOUT) {
    do {
      if (err_invoke(PRN_INSERT_SLIP) != SUCCEED) {
        return FAIL;
      }
      status = oprn_wait_for_slip(printer, 0);
    } while (status == OPOS_E_TIMEOUT);
  }

  if (status == OPOS_SUCCESS) {
    do {
      status = oprn_close_slip_jaws(printer);                     /* close jaws */
      if (status == OPOS_EPTR_COVER_OPEN) {
        chq_err_handler(OPOS_EPTR_COVER_OPEN);
        oprn_wait_for_slip(printer, 0);                 /* must be called again */
      }
    } while (status == OPOS_EPTR_COVER_OPEN);
  }

  return status;
} /* insert_slip */

/*----------------------------------------------------------------------------*/
/*                       remove_slip                                          */
/*----------------------------------------------------------------------------*/
long remove_slip(LOGICAL_PRINTER printer)
{
  long status;

  while ( (status=oprn_wait_for_slip_removal(printer, 0)) == OPOS_E_TIMEOUT) {
    err_invoke(PRN_REMOVE_SLIP);
  }
   
  if (status == OPOS_SUCCESS) {
    status = oprn_end_removal(printer);                        /* close jaws */
  }

  return status;
} /* remove_slip */

/*----------------------------------------------------------------------------*/
/*                       remove_slip                                          */
/*----------------------------------------------------------------------------*/
long set_sideways_mode(LOGICAL_PRINTER printer, long rotation)
{
  long status;

  do {
    status = oprn_rotate_print(printer, rotation);
    if (status == OPOS_EPTR_COVER_OPEN) {
      chq_err_handler(OPOS_EPTR_COVER_OPEN);
    }
  } while (status == OPOS_EPTR_COVER_OPEN);

  return status;
} /* remove_slip */


/*----------------------------------------------------------------------------*/
/*                         chq_err_handler                                    */
/*----------------------------------------------------------------------------*/
void chq_err_handler(long opos_err)
{
  short idx = sizeof(err_table)/sizeof(struct ERR_TABLE); /* number of elements in table */                                    

  while (idx && err_table[--idx].opos_error != opos_err) ;      /* find error */

  if (err_table[idx].prn_error != -1) {
    err_invoke(err_table[idx].prn_error);              /* invoke error */
  }

  return;
} /* chq_err_handler */



