/*
 *     Module Name       : WRITE.C
 *
 *     Type              : Standard functions for writing an object and
 *                         printing a line
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
 * 04-Oct-2000 Modified print_ln_to_file in a more generic way.        J.D.M.
 * 04-Oct-2000 Added function clear_all_used_print_files()             J.D.M.
 * 09-Oct-2000 Changed send_formfeed() and send_linefeed()             J.D.M.
 * 09-Oct-2000 Changed prn_on into prn_on[printer]                     J.D.M.
 * --------------------------------------------------------------------------
 * 30-Aug-2001 Added debug information                                 J.D.M.
 * --------------------------------------------------------------------------
 * 16-Oct-2002 Moved printing of init codes to this source.            J.D.M.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>
                                            /* Toolsset include files.       */
#include "err_mgr.h"
#include "prn_mgr.h"
#include "tm_mgr.h"
#include "inp_mgr.h"
#include "intrface.h"
#include "write.h"
                                            /* Application include files.    */
#include "pos_tot.h"
#include "pos_recs.h"
#include "pos_txt.h"
#include "pos_bp1.h"
#include "pos_bp2.h"
#include "pos_tm.h"
#include "WPos_mn.h"

#define PRINT_TO_FILE  1
#define CLEAR_FILE     2

static void  wait_for_prn_succeed(short);
static void  print_ln_to_file(short, _TCHAR *, short);

extern short on_ln_inv;                           /* current line on invoice */

_TUCHAR init_codes[] =                              /* Initialisation codes */
{ ESCAPE
 ,INIT_PRINTER                                      /* init printer         */
 ,SELECT_PRINTER                                    /* select printer       */
/* TODO */
/*   ,ESCAPE, SELECT_12CPI  */                        /* compressed (15 CPI)  */
/*   ,ESCAPE, SELECT_8LPI   */                        /* 8 LPI                */
};

short size_init_codes = sizeof(init_codes)/sizeof(_TUCHAR);

_TUCHAR cpi_lpi_codes[] =                         /* Initialisation codes */
{ ESCAPE, SELECT_ELITE                            /* 12 CPI               */
 ,ESCAPE, SELECT_6LPI                             /* 6 LPI                */
};
short size_cpi_lpi_codes = sizeof(cpi_lpi_codes)/sizeof(_TUCHAR);

/*----------------------------------------------------------------------------*/
/*                           write_ln                                         */
/*----------------------------------------------------------------------------*/
/* This function prints 'objects'. It first calculates the number of lines    */
/* the object requires and verifies if it fits on the current page. If it     */
/* doesn't fit, a page skip will be forced.                                   */
/* Each object corresponds with two arrays of other objects (trail_arr and    */
/* head_arr). Trail_arr contains objects that have to be printed when the     */
/* current object doesn't fit on the current page. After all the objects of   */
/* trail_arr are processed (= printed), the objects of the head_arr will be   */
/* processed. Finally the current object can be printed.                      */
/* Example:                                                                   */
/* - object 'i_article_line' must be printed but doesn't fit on the page,     */
/*  ->  loop in trail_arr: f.e. print 'i_to_transport'                        */
/*                              print 'i_empty_bottom'                        */
/*  ->  loop in  head_arr: f.e. print 'i_promo'                               */
/*                              print 'i_customer'                            */
/*                              print 'i_transported'                         */
/*  ->  finally 'i_article_line' can be printed.                              */
/*----------------------------------------------------------------------------*/

short write_ln(PRN_OBJECT *p_object, TRAIL *trail_arr,
               HEAD *head_arr, short offs_grp, short printer)
{
  short i;

  if (p_object) {
    p_object->fn(p_object, CALCULATE_IT, offs_grp, printer);
    if (p_object->ln + LN_BOT_INV <= ((LN_PG_INV - on_ln_inv)+1)) {
      p_object->fn(p_object, PRINT_IT, offs_grp, printer);
    }
    else {                              /* No fit, process trailer and header.*/
      for (i=0; trail_arr[i].obj; i++) {
        trail_arr[i].obj->fn(p_object, PRINT_IT, offs_grp, printer);
      }
      for (i=0; head_arr[i].obj; i++) {
        head_arr[i].obj->fn(p_object, PRINT_IT, offs_grp, printer);
      }
      p_object->fn(p_object, PRINT_IT, offs_grp, printer);
    }
    return(SUCCEED);
  }
  err_invoke(-31404);

  return(FAIL);
} /* write_ln */

/*----------------------------------------------------------------------------*/
/*                           print_ln                                         */
/*----------------------------------------------------------------------------*/
/* This function prints lines or empty lines (line feeds) on the specified    */
/* printer. A check is made to see if the printer is ready (on-line) and if   */
/* there is enough paper available.                                           */
/* After each printed line the line counter (on_ln_inv) is incremented.       */
/*----------------------------------------------------------------------------*/

short print_ln(short printer, _TCHAR *inbuffer)
{
  short  prn_status;
  _TCHAR buffer[5*INV_SIZE+1];

  _tcscpy(buffer, inbuffer);

#ifdef DEBUG
   if (_tcslen(buffer)>5)
     _stprintf(buffer, _T("[%3d]%s"), on_ln_inv, inbuffer+5);
   else
     _stprintf(buffer, _T("[%3d]%s"), on_ln_inv, inbuffer);
   buffer[INV_SIZE]=_T('\0');
#endif

  if (print_to_file==YES) {
    print_ln_to_file(printer, buffer, PRINT_TO_FILE);
  }
  else {
    if (prn_on[printer]==YES) {
      if (buffer == (_TCHAR)NULL) {
        /* print an empty line */
        if (prn_status=ec_prn_lf(printer) != SUCCEED) {
          wait_for_prn_succeed(printer);
        }
      }
      else {
        if (prn_status=ec_prn_print(buffer, printer) != SUCCEED) {
          wait_for_prn_succeed(printer);
        }
        if (prn_status=ec_prn_lf(printer) != SUCCEED) {
          wait_for_prn_succeed(printer);
        }
      }
    }
  }

  /* Don't need to count lines on small size printer -> no pages */
  if (GetPrinterSize(printer) != PRINTER_SIZE_SMALL) {
    on_ln_inv++;
    if (on_ln_inv > LN_PG_INV) {
      on_ln_inv-=LN_PG_INV;
    }
  }

  return (SUCCEED);
} /* print_ln */

/*----------------------------------------------------------------------------*/
/*                           print_skip_ln                                    */
/*----------------------------------------------------------------------------*/
/* Almost the same as print_ln() except that on_ln_inv is not incremented.    */
/*----------------------------------------------------------------------------*/
short print_skip_ln(short printer, _TCHAR *buffer)
{
  short prn_status;

  if (print_to_file==YES) {
    print_ln_to_file(printer, buffer, PRINT_TO_FILE);
  }
  else {
    if (prn_on[printer]==YES) {
      if (buffer == (_TCHAR) NULL) {                        /* print an empty line */
        if (prn_status=ec_prn_lf(printer) != SUCCEED) {
          wait_for_prn_succeed(printer);
        }
      }
      else {
        if (prn_status=ec_prn_print(buffer, printer) != SUCCEED) {
          wait_for_prn_succeed(printer);
        }
      }
    }
  }

  return (SUCCEED);
} /* print_skip_ln */

/*----------------------------------------------------------------------------*/
/*                           init_printer                                     */
/*----------------------------------------------------------------------------*/
/* Used to send escape sequences to the Epson LQ-300K printer.                */
/*----------------------------------------------------------------------------*/
void init_printer(short printer, _TUCHAR *codes, short init_len)
{
  short prn_status;

  if (print_to_file==YES) {
    /* Do nothing */
  }
  else {
    while ((prn_status = ec_prn_init(codes, (short) init_len, printer)) != SUCCEED) {
      if (prn_status == EC_PRN_NOT_READY) {
        err_invoke(EC_PRN_NOT_READY);
      }
      if (prn_status == EC_PRN_NO_PAPER) {
        err_invoke(EC_PRN_NO_PAPER);
      }
    }
  }

  return;
} /* init_printer */

/*----------------------------------------------------------------------------*/
/*                             prn_print                                      */
/*----------------------------------------------------------------------------*/
/* Generic function to print a single line directly                           */
/*----------------------------------------------------------------------------*/
void prn_print(short printer, _TCHAR *buffer)
{
  print_skip_ln(printer, buffer);

  return;
} /* prn_print */

/*----------------------------------------------------------------------------*/
/*                           send_formfeed                                    */
/*----------------------------------------------------------------------------*/
/* Send a formfeed                                                            */
/*----------------------------------------------------------------------------*/
void send_formfeed(short printer)
{
  short prn_status;

  if (print_to_file==YES) {
    print_ln_to_file(printer, _T("\n\n\n\n\n\n\n\n\n"), PRINT_TO_FILE);
  }
  else {
    if (prn_on[printer]==YES) {
      if (prn_status=ec_prn_ff(printer) != SUCCEED) {
        wait_for_prn_succeed(printer);
      }
    }
  }

  return;
} /* send_formfeed */

/*----------------------------------------------------------------------------*/
/*                           send_linefeed                                    */
/*----------------------------------------------------------------------------*/
/* Send a linefeed                                                            */
/*----------------------------------------------------------------------------*/
void send_linefeed(short printer)
{
  short prn_status;

  if (print_to_file==YES) {
    print_ln_to_file(printer, _T(""), PRINT_TO_FILE);
  }
  else {
    if (prn_on[printer]==YES) {
      if (prn_status=ec_prn_lf(printer) != SUCCEED) {
        wait_for_prn_succeed(printer);
      }
    }
  }

  return;
} /* send_linefeed */

/*----------------------------------------------------------------------------*/
/*                          send_cpi_and_lpi                                  */
/*----------------------------------------------------------------------------*/
/* Printers can forget their codes if you turn them off. This function will   */
/* send the two most important ones to the printer.                           */
/*----------------------------------------------------------------------------*/
void send_cpi_and_lpi(short printer) {
  short prn_status;

  if (print_to_file==YES) {
    /* Do nothing */
  }
  else {
    if (prn_on[printer]==YES) {
      if (prn_status=ec_prn_raw(cpi_lpi_codes, size_cpi_lpi_codes, printer) != SUCCEED) {
        wait_for_prn_succeed(printer);
      }
    }
  }

  return;
} /* send_cpi_and_lpi */

/*----------------------------------------------------------------------------*/
/*                          send_init_codes                                   */
/*----------------------------------------------------------------------------*/
/* Printers can forget their codes if you turn them off. This function will   */
/* send all init codes to the printer.                                        */
/*----------------------------------------------------------------------------*/
void send_init_codes(short printer) {
  short prn_status;

  if (print_to_file==YES) {
    /* Do nothing */
  }
  else {
    if (prn_on[printer]==YES) {
      if (prn_status=ec_prn_raw(init_codes, size_init_codes, printer) != SUCCEED) {
        wait_for_prn_succeed(printer);
      }
    }
  }

  return;
} /* send_init_codes */

/*----------------------------------------------------------------------------*/
/*                           wait_for_prn_succeed                             */
/*----------------------------------------------------------------------------*/
static void wait_for_prn_succeed(short printer)
{
  short prn_status;

  if (print_to_file==YES) {
    /* Do nothing */
  }
  else {
    if (prn_on[printer]==YES) {
      while ((prn_status = IsPrinterError(printer)) != SUCCEED) {
        if (prn_status == EC_PRN_NOT_READY) {
          err_invoke(PRN_FATAL_ERROR);
        }
        if (prn_status == EC_PRN_NO_PAPER) {
          err_invoke(PRN_JOURNAL_PAPER_OUT);
        }
      }
    }
  }

  return;
} /* wait_for_prn_succeed */

/*----------------------------------------------------------------------------*/
/*                           print_cut                                        */
/*----------------------------------------------------------------------------*/
short print_cut(short printer)
{
  short prn_status;
  short i;

  if (print_to_file==YES) {
    for (i=0;i<3;i++) {
      print_ln_to_file(printer, _T(""), PRINT_TO_FILE);
    }
    if (GetPrinterSize(printer) != PRINTER_SIZE_SMALL) {
      print_ln_to_file(printer, prn_inv_TXT[52], PRINT_TO_FILE);
    }
    else {
      print_ln_to_file(printer, prn_inv_TXT[53], PRINT_TO_FILE);
    }
  }
  else {
    if (prn_on[printer]==YES) {
      for (i=0;i<8;i++) {
        if (prn_status=ec_prn_lf(printer) != SUCCEED) {
          wait_for_prn_succeed(printer);
        }
      }
      if (prn_status=ec_prn_cut(printer) != SUCCEED) {
        wait_for_prn_succeed(printer);
      }
    }
  }

  return (SUCCEED);
} /* print_cut */

/*----------------------------------------------------------------------------*/
/*                           print_color                                      */
/*----------------------------------------------------------------------------*/
short print_color(short printer, short color)
{
  short prn_status;

  if (print_to_file==YES) {
    /* Do nothing */
  }
  else {
    if (prn_on[printer]==YES) {
      if (prn_status=ec_prn_color(printer, color) != SUCCEED) {
        wait_for_prn_succeed(printer);
      }
    }
  }

  return (SUCCEED);
} /* print_color */

/*----------------------------------------------------------------------------*/
/*                        clear_all_used_print_files                          */
/*----------------------------------------------------------------------------*/
void clear_all_used_print_files(void)
{
  short i;

  for(i=DEV_PRINTER1; i<NUMBER_OF_PRINTERS; i++) {
    print_ln_to_file(i, _T(""), CLEAR_FILE);
  }

  return;
} /* clear_all_used_print_files */

/*----------------------------------------------------------------------------*/
/*                           print_ln_to_file                                 */
/*----------------------------------------------------------------------------*/
void print_ln_to_file(short printer, _TCHAR *buffer, short action)
{
  FILE   *stream = 0;
  _TCHAR *fName = 0;
  _TCHAR dummy[100];

  if( (printer >= NUMBER_OF_PRINTERS) || printer < 0) {
    _stprintf(dummy, _T("WRITE.C: Print_ln_to_file, Program Bug.\nNumber of printers not correct!"));
    MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);    
    return;
  }

  switch (printer) {
    case DEV_PRINTER1:
      fName = PRINT_FILE1;
      break;
    case DEV_PRINTER2:
      fName = PRINT_FILE2;
      break;
    case DEV_PRINTER3:
      fName = PRINT_FILE3;
      break;
    case DEV_PRINTER4:
      fName = PRINT_FILE4;
      break;
    default:
      /* Undefined printer */
      return;
      break;
  }

  if (fName) {
    switch(action) {
      case CLEAR_FILE:
        stream = _tfopen(fName, _T("w"));
        break;
      case PRINT_TO_FILE:
      default:
        stream = _tfopen(fName, _T("a"));
        break;
    }
  }

  if (stream) {
    switch(action) {
      case CLEAR_FILE:
        break;
      case PRINT_TO_FILE:
      default:
        _ftprintf(stream, _T("%s\n"), (buffer ? buffer : _T("")));
        break;
    }
    fclose(stream);
  }

  return;
} /* print_ln_to_file */
