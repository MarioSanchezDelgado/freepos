/*
 *     Module Name       : Prn_mgr.cpp
 *
 *     Type              : General print manager
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
 * 13-Dec-1999 Initial Release WinPOS                             P.M./J.D.M.
 * --------------------------------------------------------------------------
 * 08-Oct-2000 Bugfix unicode: ec_prn_lf() and ec_prn_ff()
 *             added _tcslen(buffer) instead of 1                      J.D.M.
 * --------------------------------------------------------------------------
 * 08-Oct-2000 Changed PRINTERS_ATTACHED into PRINTER%d_ATTACHED       J.D.M.
 * --------------------------------------------------------------------------
 * 16-Oct-2000 If the text NOT_USED is used for an attribute then the
 *             attibute will be ignored                                J.D.M.
 * --------------------------------------------------------------------------
 * 22-Oct-2001 Bugfix: CloseHandle() should be called in ec_prn_init()
 *             if the call to ec_prn_raw() fails.                      J.D.M.
 * --------------------------------------------------------------------------
 * 25-Sep-2002 Added function print_data_raw
 *             Function to send special printer codes to printer
 *             like initialization strings.                            JHEE
 * 25-Sep-2002 Changed ec_prn_raw for use with print_data_raw          JHEE
 * --------------------------------------------------------------------------
 * 14-Feb-2003 Check to see if double lines are printed.                 M.W.
 * --------------------------------------------------------------------------
 * 08-Aug-2005 Reduced Invoice Form functionality.                       M.W.
 * --------------------------------------------------------------------------
 * 23-Feb-2006 NOT_USED in the registry in case of attributes will
 *             disable the attribute.                                  J.D.M.
 * --------------------------------------------------------------------------
 * 16-Oct-2009 Added Virtual port printer                               JC-PE
 * --------------------------------------------------------------------------
 * 13-Jul-2011 Added Regedit entry PrintModel for printer BEMANTECH    ACM-PE
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>

#include "prn_mgr.h"
#include "err_mgr.h"
#include "registry.h"
#include "mem_mgr.h"
#include "pos_errs.h"
#include "erlg_tls.h"

/* ENVIRONMENT VARIABLES REGISTRY */
#define LEN_OPTIONS       10

/*
 * DEFINITION OF TEXT ATTRIBUTES (See also prn_mgr.h!)
 */
#define LEN_BOLD        8
#define LEN_UNDERLINE   8
#define LEN_ITALIC      8
#define LEN_NLQ         8

_TCHAR  reg_bold[LEN_BOLD+1];
_TCHAR  reg_underline[LEN_UNDERLINE+1];
_TCHAR  reg_italic[LEN_ITALIC+1];
_TCHAR  reg_nlq[LEN_NLQ+1];

_TCHAR  not_printable_attributes[LAST_ATTR+1];
_TCHAR  not_printable_string[LAST_ATTR+1];

typedef struct {
  short  active;                      /* flag                                 */
  _TCHAR on[LEN_OPTIONS+1];           /* attribute escape sequence ON         */
  _TCHAR off[LEN_OPTIONS+1];          /* attribute escape sequence OFF        */
} ATTRIBUTE_DEF;

#define LEN_PORT_TYPE   3

short print_xz;  /*Verifica quien imprime la x y la z 1/2/3/4 Jonathan Peru*/

typedef struct {
  _TCHAR  port_type[LEN_PORT_TYPE+1]; /* COM/LPT                                  */
  short port_number;                  /* 1/2/3/4 etc.                             */
  long  baud_rate;                    /* 110/300/600/1200/2400/4800/9600/19200 etc*/
  short data_bits;                    /* 7/8                                      */
  short parity;                       /* NONE, ODD, EVEN, MARK, SPACE             */
  short stop_bits;                    /* 1/2                                      */
  short time_out;                     /* Time out in milliseconds                 */
} REG_PORT;

/* 13-Jul-2011 acm -{*/
#define LEN_PRINTMODEL   500		
/* 13-Jul-2011 acm -}*/

typedef struct {
  HANDLE         hPrinter;              /* Handle to the printer       */
  short          prn_init;              /* Is printer initialised      */
  _TCHAR         printer[LEN_PRINTER_TYPE+1];
  ATTRIBUTE_DEF  attribute[LAST_ATTR];
  _TCHAR         end_codes[(LEN_OPTIONS*LAST_ATTR)+1];
  OVERLAPPED     aePrinter;
  short          busy;
  DWORD          rqstlen;
  short          printersize;
  short          attached;
  _TCHAR		 printModel[LEN_PRINTMODEL]; /* 13-Jul-2011 acm - */
} REG_PRINTER;

REG_PRINTER printer_no[NUMBER_OF_PRINTERS];
REG_PORT    port_no[NUMBER_OF_PRINTERS];

PRINTER_INFO pos_printer_info[NUMBER_OF_PRINTERS];

static short print_data(_TCHAR *data, short len, short);
static short print_data_raw(_TUCHAR *data, short len, short);
static short is_attribute(_TCHAR);
static void  add_attributes(_TCHAR *, _TCHAR *, short);
static short wait_for_IO(short);
static short ReadCOMPortSettings(_TCHAR *, short);
static short ReadLPTPortSettings(_TCHAR *, short);

/*---------------------------------------------------------------------------*/
/*                          ec_prn_init                                      */
/*---------------------------------------------------------------------------*/
short ec_prn_init(_TUCHAR *init_codes, short len, short device_printer)
{
  _TCHAR       dummy[100];
  DCB          dcb;
  COMMTIMEOUTS commtimeouts;
  _TCHAR       printer_name[20];

  if( (device_printer >= NUMBER_OF_PRINTERS) || device_printer < 0) {
    _stprintf(dummy, _T("PRN_MGR.CPP: Ec_prn_init, Program Bug.\nNumber of printers not correct!"));
    MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);    
    /* TODO: Pos application should exit! */
    return(EC_PRN_NOT_READY);
  }

  if (printer_no[device_printer].prn_init) {
    return(SUCCEED);
  }

  _stprintf(printer_name, _T("\\\\.\\%s%d"), port_no[device_printer].port_type, port_no[device_printer].port_number);
  printer_no[device_printer].hPrinter = CreateFile(
                           printer_name,
                           GENERIC_READ | GENERIC_WRITE,
                           0,
                           0,
                           OPEN_EXISTING,
                           FILE_FLAG_OVERLAPPED,
                           0);
  if (printer_no[device_printer].hPrinter == INVALID_HANDLE_VALUE) {
    return (EC_PRN_NOT_READY);
  }

  printer_no[device_printer].busy = FALSE;

  if(!_tcscmp(_T("COM"), port_no[device_printer].port_type)) {
    if (GetCommState(printer_no[device_printer].hPrinter, &dcb)) {
      dcb.BaudRate = port_no[device_printer].baud_rate;
      dcb.Parity   = (unsigned char)port_no[device_printer].parity; 
      dcb.ByteSize = (unsigned char)port_no[device_printer].data_bits;
      dcb.StopBits = (unsigned char)port_no[device_printer].stop_bits;
      dcb.fOutxDsrFlow = TRUE ; /* Necessary to detect broken connection */
      dcb.fOutX        = TRUE ; /* Necessary to detect on_line / off_line */
      SetCommState(printer_no[device_printer].hPrinter, &dcb);
    }
    else {
      CloseHandle(printer_no[device_printer].hPrinter);
      return (EC_PRN_NOT_READY);
    }
  }

  if (GetCommTimeouts(printer_no[device_printer].hPrinter, &commtimeouts)) {
    commtimeouts.WriteTotalTimeoutConstant = 0;
    commtimeouts.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(printer_no[device_printer].hPrinter, &commtimeouts);
  }
  else {
    CloseHandle(printer_no[device_printer].hPrinter);
    return (EC_PRN_NOT_READY);
  }

  if(ec_prn_raw(init_codes, len, device_printer) == SUCCEED) {
    printer_no[device_printer].prn_init = TRUE;
  }
  else {
    CloseHandle(printer_no[device_printer].hPrinter);
    return (EC_PRN_NOT_READY);
  }

  return(SUCCEED);
} /* ec_prn_init */

/*---------------------------------------------------------------------------*/
/*                          ec_prn_exit                                      */
/*---------------------------------------------------------------------------*/
void ec_prn_exit(short device_printer)
{
  if (printer_no[device_printer].hPrinter) {
    CloseHandle(printer_no[device_printer].hPrinter);
  }

  printer_no[device_printer].prn_init = FALSE;
} /* ec_prn_exit */

/*---------------------------------------------------------------------------*/
/*                          ec_prn_raw                                       */
/*---------------------------------------------------------------------------*/
short ec_prn_raw(_TUCHAR *buffer, short len, short device_printer)
{
  short status;

  status = IsPrinterError(device_printer);
  if (status != SUCCEED) {
    return(status);
  }

  if (printer_no[device_printer].hPrinter) {
    return(print_data_raw(buffer, len, device_printer));
  }

  return(status);
} /* ec_prn_raw */

/*---------------------------------------------------------------------------*/
/*                          ec_prn_print                                     */
/*---------------------------------------------------------------------------*/
short ec_prn_print(_TCHAR *buffer, short device_printer)
{
  short status;
  static _TCHAR same_line[INV_SIZE+1] = _T("");

  status = IsPrinterError(device_printer);
  if (status != SUCCEED) {
    return(status);
  }

  if (_tcscmp(buffer, same_line)==0 && _tcslen(buffer)) {
    err_invoke(DOUBLE_LINE_ON_INVOICE);
  }
  _tcscpy(same_line, buffer);

  if (printer_no[device_printer].hPrinter) {
    return(print_data(buffer, _tcslen(buffer), device_printer));
  }

  return(status);
} /* ec_prn_print */

/*---------------------------------------------------------------------------*/
/*                          ec_prn_lf                                        */
/*---------------------------------------------------------------------------*/
short ec_prn_lf(short device_printer)
{
  short  status;
  _TCHAR buf[2];

  status = IsPrinterError(device_printer);
  if (status != SUCCEED) {
    return(status);
  }

  buf[0] = LINE_FEED;
  buf[1] = _T('\0');
  
  if (printer_no[device_printer].hPrinter) {
    return(print_data(buf, _tcslen(buf), device_printer));
  }

  return(status);
} /* ec_prn_lf */

/*---------------------------------------------------------------------------*/
/*                          ec_prn_ff                                        */
/*---------------------------------------------------------------------------*/
short ec_prn_ff(short device_printer)
{
  short status;
  _TCHAR buf[2];

  status = IsPrinterError(device_printer);
  if (status != SUCCEED) {
    return(status);
  }

  buf[0] = FORM_FEED;
  buf[1] = _T('\0');

  if (printer_no[device_printer].hPrinter) {
    return(print_data(buf, _tcslen(buf), device_printer));
  }

  return(status);
} /* ec_prn_ff */

/*---------------------------------------------------------------------------*/
/*                          ec_prn_cut                                       */
/*---------------------------------------------------------------------------*/
short ec_prn_cut(short device_printer)
{
  short  status;
  _TCHAR buf[4];

  status = IsPrinterError(device_printer);
  if (status != SUCCEED) {
    return(status);
  }
  /*bemantech*/

  /* 13-Jul-2011 acm -{*/ 
  if ((printer_no[device_printer].printModel[0]) && (strcmp((char *)&printer_no[device_printer].printModel, (char *)&(_T("BEMATECH"))) ==0))
  {
  /* 13-Jul-2011 acm -}*/   
	  buf[0] = _T('\x1D'); /* GS */
	  buf[1] = _T('\x56'); /* V  */
	  buf[2] = _T('\x01'); /* 1  */
	  buf[3] = _T('\0');   /* \0 */

/* 13-Jul-2011 acm -{*/ 
  }else{
	  buf[0] = _T('\x1B'); // ESC 
	  buf[1] = _T('\x64'); // D   
	  buf[2] = _T('\x30'); // N:0 
	  buf[3] = _T('\0');   // \0  
  }
/* 13-Jul-2011 acm -}*/ 


  /*buf[0] = _T('\x1D');  / GS /
  buf[1] = _T('\x56');  / V  /
  buf[2] = _T('\x6D');  / m  /
  buf[3] = _T('\0');    / \0 /*/

  /*Star*/
  
  /*buf[0] = _T('\x1B'); / ESC /
  buf[1] = _T('\x64'); / D   /
  buf[2] = _T('\x30'); / N:0 /
  buf[3] = _T('\0');   / \0  /
  */

  if (printer_no[device_printer].hPrinter) {
    return(print_data(buf, _tcslen(buf), device_printer));
  }

  return(status);
} /* ec_prn_cut */

/*---------------------------------------------------------------------------*/
/*                          ec_prn_color                                     */
/*---------------------------------------------------------------------------*/
short ec_prn_color(short device_printer, short color)
{
  short  status;
  _TCHAR buf[4];

  status = IsPrinterError(device_printer);
  if (status != SUCCEED) {
    return(status);
  }

  buf[0] = _T('\x1B');    /* ESC       */
  buf[1] = _T('\x72');    /* r         */
  switch(color) {
    case PRINT_COLOR_RED:
      buf[2] = _T('\x31');  /* 1 = red   */ 
      break;
    case PRINT_COLOR_BLACK:
    default:
      buf[2] = _T('\x30');  /* 0 = black */
      break;
  }
  buf[3] = _T('\0');      /* \0        */

  if (printer_no[device_printer].hPrinter) {
    return(print_data(buf, _tcslen(buf), device_printer));
  }

  return(status);
} /* ec_prn_color */

/*---------------------------------------------------------------------------*/
/*                          IsPrinterError                                   */
/*---------------------------------------------------------------------------*/
short IsPrinterError(short device_printer)
{
  if (!printer_no[device_printer].busy) {
    return(SUCCEED);
  }
  else {
    return(wait_for_IO(device_printer));
  }
} /* IsPrinterError */

/*---------------------------------------------------------------------------*/
/*                          print_data                                       */
/* - len = number of characters in data_in                                   */
/*---------------------------------------------------------------------------*/
static short print_data(_TCHAR *data_in, short len, short device_printer)
{
  DWORD        nWritten=0;
  long         ErrorNumber;
  _TCHAR       data[5*INV_SIZE+1] = _T(""); /* Contains also escape codes that make the array  */
                                             /* longer than the number of printable characters. */
  _TCHAR      *data_copy;

  if (!printer_no[device_printer].hPrinter) {
    return(EC_PRN_NOT_READY);
  }

  if (!len) {
    return(SUCCEED);
  }

  data_copy = (_TCHAR *) mem_allocate ((unsigned int) (len+1)*sizeof(_TCHAR));
  _tcsncpy(data_copy, data_in, len);
  data_copy[len] = _T('\0'); /* make sure the buffer is terminated */
  add_attributes(data, data_copy, device_printer);
  len = _tcslen(data); /* recalculate length of buffer */
  free(data_copy);

  if (!printer_no[device_printer].busy) {
    printer_no[device_printer].aePrinter.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    printer_no[device_printer].rqstlen = len;
    printer_no[device_printer].busy = TRUE;
    if (!WriteFile(printer_no[device_printer].hPrinter, data,
                   len, &nWritten, &printer_no[device_printer].aePrinter)) {
      ErrorNumber = GetLastError();
      if (ErrorNumber != ERROR_IO_PENDING) {
        err_shw(_T("PRN_MGR: Error writing to printer : %ld"), ErrorNumber);
      }
    }
  }

  return (wait_for_IO(device_printer));
} /* print_data */

/*---------------------------------------------------------------------------*/
/*                          print_data_raw                                   */
/* - len = number of characters in data_in                                   */
/*---------------------------------------------------------------------------*/
static short print_data_raw(_TUCHAR *data_in, short len, short device_printer)
{
  DWORD         nWritten=0;

  if (!printer_no[device_printer].hPrinter) {
    return(EC_PRN_NOT_READY);
  }

  if (!len) {
    return(SUCCEED);
  }

  if (!printer_no[device_printer].busy) {
    printer_no[device_printer].aePrinter.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    printer_no[device_printer].rqstlen = len;
    printer_no[device_printer].busy = TRUE;
    WriteFile(printer_no[device_printer].hPrinter, data_in,
              len, &nWritten, &printer_no[device_printer].aePrinter);
  }

  return (wait_for_IO(device_printer));
} /* print_data_raw */

/*---------------------------------------------------------------------------*/
/*                          wait_for_IO                                      */
/*---------------------------------------------------------------------------*/
static short wait_for_IO(short device_printer)
{
  DWORD dRes, nWritten = 0;
  short status;

  dRes = WaitForSingleObject(printer_no[device_printer].aePrinter.hEvent, port_no[device_printer].time_out);
  switch (dRes) {
    case WAIT_OBJECT_0:
      printer_no[device_printer].busy = FALSE;
      GetOverlappedResult(printer_no[device_printer].hPrinter,
           &printer_no[device_printer].aePrinter,
           &nWritten,
           FALSE);
      if (nWritten != printer_no[device_printer].rqstlen) {
        status = EC_PRN_NOT_READY;
      }
      else {
        status = SUCCEED;
        CloseHandle(printer_no[device_printer].aePrinter.hEvent);
      }
      break;
    case WAIT_TIMEOUT:
      status = EC_PRN_NOT_READY;
      break;
    default:
      status = EC_PRN_NOT_READY;
      printer_no[device_printer].busy = FALSE;
      break;
  }

  return(status);
} /* wait_for_IO */

/*----------------------------------------------------------------------------*/
/*                             is_attribute                                   */
/*----------------------------------------------------------------------------*/
/* Checks if we're dealing with a plain character or an attribute code        */
/*----------------------------------------------------------------------------*/
static short is_attribute(_TCHAR chr)
{
  if(chr == *reg_bold) {
    return(BOLD);
  }
  else if(chr == *reg_italic) {
    return(ITALIC);
  }
  else if(chr == *reg_underline) {
    return(UNDERLINE);
  }
  else if(chr == *reg_nlq) {
    return(NLQ);
  }
  else {
    return(DRAFT);
  }
} /* is_attribute */

/*----------------------------------------------------------------------------*/
/*                            add_attributes                                  */
/*----------------------------------------------------------------------------*/
/* Replaces codes for italic, bold, underline and NLQ with their              */
/* corresponding escape sequences                                             */
/*----------------------------------------------------------------------------*/
static void add_attributes(_TCHAR *out, _TCHAR *in, short device_printer)
{
  short  nof_chars; /* number of printable characters */
  short  atno;      /* attribute type                 */
  _TCHAR  *i;      /* input pointer                    */
  _TCHAR  *o;      /* output pointer                   */

  o = out;
  nof_chars = 0;

  for (atno = 0; atno < LAST_ATTR; atno++) {
    /* turn off attributes for this line */
    printer_no[device_printer].attribute[atno].active = 0;
  }

  i = in;
  while (*i && (nof_chars < INV_SIZE)) { /* Don't print more than 'max_len' characters. */
    atno = is_attribute(*i);
    if (atno == DRAFT) {
      *o++ = *i;
      nof_chars++;
    }
    else {
      if (printer_no[device_printer].attribute[atno].active) {
        _stprintf(o, _T("%s"), printer_no[device_printer].attribute[atno].off);
        o += _tcslen(printer_no[device_printer].attribute[atno].off);
        printer_no[device_printer].attribute[atno].active = 0;
      }
      else {
        _stprintf(o, _T("%s"),printer_no[device_printer].attribute[atno].on);
        o += _tcslen(printer_no[device_printer].attribute[atno].on);
        printer_no[device_printer].attribute[atno].active = 1;
      }
    }
    i++;
  }

  _tcscat(o, printer_no[device_printer].end_codes);
} /* add_attributes */


 /*****************************************************************
  * ESCAPE CODES:
  *
  * OKI3320:
  *
  *  Initialize escape codes for the available text attributes:
  *  <Esc> = \x1B
  *
  *  ATTRIB       ON            OFF
  *  ========================================
  *  BOLD         <Esc>E        <Esc>F
  *  ITALIC       <Esc>4        <Esc>5
  *  UNDERLINE    <Esc>-1       <Esc>-0
  *  NLQ          <Esc>\x78\x01 <Esc>\x28\x30
  *
  *
  * Mannesmann Tally:
  *
  *  ATTRIB       ON            OFF
  *  ========================================
  *  BOLD         <Esc>[=z      <Esc>[>z
  *  ITALIC       <Esc>[3m      <Esc>[23m
  *  UNDERLINE    <Esc>[4m      <Esc>[24m
  *  NLQ          <Esc>[1y      <Esc>[0y
  *****************************************************************/

/*---------------------------------------------------------------------------*/
/*                          ReadPrintEnvironment                             */
/*---------------------------------------------------------------------------*/
short ReadPrintEnvironment(void)
{
  _TCHAR  ActivePrinter[100];
  _TCHAR  ActivePort[100];
  short   i;
  _TCHAR  dummy[100];
#define LEN_PORT_NUMBER       2
#define LEN_PRINTERS_ATTACHED 3
#define LEN_PRINTERSIZE       6
  _TCHAR  reg_port_number[LEN_PORT_NUMBER+1];
  _TCHAR  reg_prn_att[LEN_PRINTERS_ATTACHED+1];
  _TCHAR  reg_prn_size[LEN_PRINTERSIZE+1];
  int     temp_value;
  _TCHAR *parm;

  /* Read all attributes */
  ReadEnvironmentValue(TREE_ATTRIB_SETTINGS, _T("Bold"),        reg_bold,        LEN_BOLD);
  ReadEnvironmentValue(TREE_ATTRIB_SETTINGS, _T("Italic"),      reg_italic,      LEN_ITALIC);
  ReadEnvironmentValue(TREE_ATTRIB_SETTINGS, _T("Underline"),   reg_underline,   LEN_UNDERLINE);
  ReadEnvironmentValue(TREE_ATTRIB_SETTINGS, _T("Nlq"),         reg_nlq,         LEN_NLQ);

  if(!*reg_bold || !*reg_underline || !*reg_italic || !*reg_nlq) {
    _stprintf(dummy, _T("Not all text attributes are specified in registry!\nProgram will be terminated."));
    MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
    return(FAIL);
  }

  if (!_tcsicmp(reg_bold, _T("NOT_USED"))) {
    *reg_bold = _T('\0');
  }
  if (!_tcsicmp(reg_italic, _T("NOT_USED"))) {
    *reg_italic = _T('\0');
  }
  if (!_tcsicmp(reg_underline, _T("NOT_USED"))) {
    *reg_underline = _T('\0');
  }
  if (!_tcsicmp(reg_nlq, _T("NOT_USED"))) {
    *reg_nlq = _T('\0');
  }

  not_printable_attributes[BOLD]        =*reg_bold;
  not_printable_attributes[UNDERLINE]   =*reg_underline;
  not_printable_attributes[ITALIC]      =*reg_italic;
  not_printable_attributes[NLQ]         =*reg_nlq;
  not_printable_attributes[LAST_ATTR]   =0;

  /* Reading the type of printers attached */

  memset(printer_no, 0, sizeof(REG_PRINTER)*NUMBER_OF_PRINTERS);
  memset(port_no, 0, sizeof(REG_PORT)*NUMBER_OF_PRINTERS);
  memset(pos_printer_info, 0, sizeof(PRINTER_INFO)*NUMBER_OF_PRINTERS);
  for(i=DEV_PRINTER1; i < NUMBER_OF_PRINTERS; i++) {
    _tcscpy(ActivePrinter, TREE_PRINTER_SETTINGS _T("\\Printer"));
    _stprintf(dummy, _T("%d"), i+1);
    _tcscat(ActivePrinter, dummy);
    ReadEnvironmentValue(ActivePrinter, _T("PrinterType"), printer_no[i].printer, LEN_PRINTER_TYPE);

    /* Check if the printer is attached */
    _stprintf(dummy, _T("PRINTER%d_ATTACHED"), i+1);
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, dummy, reg_prn_att, LEN_PRINTERS_ATTACHED);
    parm = _tcsupr(reg_prn_att);
    printer_no[i].attached = YES;
    if (*parm) {
      if (!_tcscmp(parm, _T("NO"))) {
         printer_no[i].attached = NO;
      }
    }

    /* Reading Port settings */
    ReadEnvironmentValue(ActivePrinter, _T("PortType"), port_no[i].port_type, LEN_PORT_TYPE);
    if(printer_no[i].attached == YES) {

      ReadEnvironmentValue(ActivePrinter, _T("PortNumber"), reg_port_number, LEN_PORT_NUMBER);

      temp_value = _ttoi(reg_port_number);
      if(!temp_value) {
        _stprintf(dummy, _T("Illegal or no port number specified for printer%d!\nProgram will be terminated."), i+1);
        MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
        return(FAIL);
      }
      port_no[i].port_number = temp_value;
      _tcscpy(ActivePort, TREE_PORT_SETTINGS_PRINTER _T("\\"));
      if(!_tcscmp(_T("COM"), port_no[i].port_type)) {
        /* It's a COM-port */
        _tcscat(ActivePort, port_no[i].port_type);
        _stprintf(dummy, _T("%d"), temp_value);
        _tcscat(ActivePort, dummy);
        if(ReadCOMPortSettings(ActivePort, i) == FAIL) {
          return(FAIL);
        }
      }
      else if(!_tcscmp(_T("LPT"), port_no[i].port_type)) {
        /* It's an LPT-port */
        _tcscat(ActivePort, port_no[i].port_type);
        _stprintf(dummy, _T("%d"), temp_value);
        _tcscat(ActivePort, dummy);
        if(ReadLPTPortSettings(ActivePort, i) == FAIL) {
          return(FAIL);
        }
      }
      else if(!_tcscmp(_T("VIR"), port_no[i].port_type)) {
        /* It's an Virtual-port
        _tcscat(ActivePort, port_no[i].port_type);
        _stprintf(dummy, _T("%d"), temp_value);
        _tcscat(ActivePort, dummy);
        if(ReadLPTPortSettings(ActivePort, i) == FAIL) {
          return(FAIL); 
        }*/
      }
      else {
        /* Unknown port type */
        _stprintf(dummy, _T("Unknown port type specified for printer%d!\nProgram will be terminated."), i+1);
        MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
        return(FAIL);
      }
    }
  }

  /* Setting escape codes for the printers */
  for(i=DEV_PRINTER1; i < NUMBER_OF_PRINTERS; i++) {
    _tcscpy(ActivePrinter, TREE_PRINTERS _T("\\"));
    _tcscat(ActivePrinter, printer_no[i].printer);
    ReadEnvironmentValue(ActivePrinter, _T("BoldOn"),       printer_no[i].attribute[BOLD].on,       LEN_OPTIONS);
    if (!_tcsicmp(printer_no[i].attribute[BOLD].on, _T("NOT_USED"))) {
      *printer_no[i].attribute[BOLD].on = _T('\0');
    }
    ReadEnvironmentValue(ActivePrinter, _T("BoldOff"),      printer_no[i].attribute[BOLD].off,      LEN_OPTIONS);
    if (!_tcsicmp(printer_no[i].attribute[BOLD].off, _T("NOT_USED"))) {
      *printer_no[i].attribute[BOLD].off = _T('\0');
    }
    ReadEnvironmentValue(ActivePrinter, _T("ItalicOn"),     printer_no[i].attribute[ITALIC].on,     LEN_OPTIONS);
    if (!_tcsicmp(printer_no[i].attribute[ITALIC].on, _T("NOT_USED"))) {
      *printer_no[i].attribute[ITALIC].on = _T('\0');
    }
    ReadEnvironmentValue(ActivePrinter, _T("ItalicOff"),    printer_no[i].attribute[ITALIC].off,    LEN_OPTIONS);
    if (!_tcsicmp(printer_no[i].attribute[ITALIC].off, _T("NOT_USED"))) {
      *printer_no[i].attribute[ITALIC].off = _T('\0');
    }
    ReadEnvironmentValue(ActivePrinter, _T("NlqOn"),        printer_no[i].attribute[NLQ].on,        LEN_OPTIONS);
    if (!_tcsicmp(printer_no[i].attribute[NLQ].on, _T("NOT_USED"))) {
      *printer_no[i].attribute[NLQ].on = _T('\0');
    }
    ReadEnvironmentValue(ActivePrinter, _T("NlqOff"),       printer_no[i].attribute[NLQ].off,       LEN_OPTIONS);
    if (!_tcsicmp(printer_no[i].attribute[NLQ].off, _T("NOT_USED"))) {
      *printer_no[i].attribute[NLQ].off = _T('\0');
    }
    ReadEnvironmentValue(ActivePrinter, _T("UnderlineOn"),  printer_no[i].attribute[UNDERLINE].on,  LEN_OPTIONS);
    if (!_tcsicmp(printer_no[i].attribute[UNDERLINE].on, _T("NOT_USED"))) {
      *printer_no[i].attribute[UNDERLINE].on = _T('\0');
    }
    ReadEnvironmentValue(ActivePrinter, _T("UnderlineOff"), printer_no[i].attribute[UNDERLINE].off, LEN_OPTIONS);
    if (!_tcsicmp(printer_no[i].attribute[UNDERLINE].off, _T("NOT_USED"))) {
      *printer_no[i].attribute[UNDERLINE].off = _T('\0');
    }
    ReadEnvironmentValue(ActivePrinter, _T("PrinterSize"),  reg_prn_size,                           LEN_PRINTERSIZE);
    parm = _tcsupr(reg_prn_size);
    if(!_tcscmp(_T("NORMAL"), parm)) {
      printer_no[i].printersize = PRINTER_SIZE_NORMAL; /*FredM*/
		
    }
    else if(!_tcscmp(_T("SMALL"), parm)) {
      printer_no[i].printersize = PRINTER_SIZE_SMALL;
    }
    else if(!_tcscmp(_T("SMALLI"), parm)) {
      printer_no[i].printersize = PRINTER_SIZE_SMALL_INV;
/*	  _stprintf(dummy, _T("Small Invoice Printer detected."), i+1);
	  MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);*/
    }
    else {
      _stprintf(dummy, _T("Unknown printer size for printer %s!\nPrinter size is set to 'NORMAL'."),
        ActivePrinter);
      MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
      printer_no[i].printersize = PRINTER_SIZE_NORMAL;
    }
    _stprintf(printer_no[i].end_codes, _T("%s%s%s%s"),
      printer_no[i].attribute[BOLD].off,
      printer_no[i].attribute[ITALIC].off,
      printer_no[i].attribute[NLQ].off,
      printer_no[i].attribute[UNDERLINE].off);

    /* Copy data to make it possible to display it on Select printer screen */
    _tcscpy(pos_printer_info[i].printer, printer_no[i].printer);
    pos_printer_info[i].attached    = printer_no[i].attached;
    pos_printer_info[i].printersize = printer_no[i].printersize;

	/* 13-Jul-2011 acm - {*/
	SetRegistryEcho(FALSE);
	printer_no[i].printModel[0]=_T('\0');
    ReadEnvironmentValue(ActivePrinter, _T("printModel"),  printer_no[i].printModel, LEN_PRINTMODEL); 
	SetRegistryEcho(TRUE);
	_TCHAR * p=printer_no[i].printModel;
    while (*p++) toupper( *p );


	pos_printer_info[i].printModel[0]=_T('\0');
	if (printer_no[i].printModel)
	_tcscpy(pos_printer_info[i].printModel   , printer_no[i].printModel);

	/* 13-Jul-2011 acm - }*/

  }

  return(SUCCEED);
} /* ReadPrintEnvironment */

/*---------------------------------------------------------------------------*/
/*                          GetPrintingAttribute                             */
/*---------------------------------------------------------------------------*/
_TCHAR GetPrintingAttribute(short AttributeType) {
  if(AttributeType<0||AttributeType>=LAST_ATTR) {
    return 0;
  }

  return(not_printable_attributes[AttributeType]);
} /* GetPrintingAttribute */

/*---------------------------------------------------------------------------*/
/*                          ReadCOMPortSettings                              */
/*---------------------------------------------------------------------------*/
short ReadCOMPortSettings(_TCHAR *ActivePort, short ArrayNumber)
{
#define LEN_BAUDRATE    6
#define LEN_DATABITS    1
#define LEN_PARITY      5
#define LEN_STOPBITS    1
#define LEN_TIMEOUT     4
  _TCHAR reg_baudrate[LEN_BAUDRATE+1];
  _TCHAR reg_databits[LEN_DATABITS+1];
  _TCHAR reg_parity[LEN_PARITY+1];
  _TCHAR reg_stopbits[LEN_STOPBITS+1];
  _TCHAR reg_timeout[LEN_TIMEOUT+1];
  _TCHAR *parm;
  long   temp_value;
  _TCHAR dummy[150];

  /* BAUDRATE */
  ReadEnvironmentValue(ActivePort, _T("BaudRate"), reg_baudrate, LEN_BAUDRATE);
  temp_value = _ttol(reg_baudrate);
  switch(temp_value) {
    case CBR_110:
    case CBR_300:
    case CBR_600:
    case CBR_1200:
    case CBR_2400:
    case CBR_4800:
    case CBR_9600:
    case CBR_14400:
    case CBR_19200:
    case CBR_38400:
    case CBR_56000:
    case CBR_57600:
    case CBR_115200:
    case CBR_128000:
    case CBR_256000:
      break;
    default:
      temp_value = CBR_9600;
      _stprintf(dummy, _T("Unknown baudrate for printer port %s%d!\nBaudrate is set to %ld."),
            port_no[ArrayNumber].port_type,
            port_no[ArrayNumber].port_number,
            temp_value);
      MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
      break;
  }
  port_no[ArrayNumber].baud_rate = temp_value;

  /* DATABITS */
  ReadEnvironmentValue(ActivePort, _T("DataBits"), reg_databits, LEN_DATABITS);
  temp_value = _ttol(reg_databits);
  switch(temp_value) {
    case 7:
    case 8:
      break;
    default:
      temp_value = 8;
      _stprintf(dummy, _T("Unknown databits for printer port %s%d!\nDatabits is set to %ld."),
            port_no[ArrayNumber].port_type,
            port_no[ArrayNumber].port_number,
            temp_value);
      MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
      break;
  }
  port_no[ArrayNumber].data_bits = (short)temp_value;

  /* PARITY */
  ReadEnvironmentValue(ActivePort, _T("Parity"), reg_parity, LEN_PARITY);
  parm = _tcsupr(reg_parity);
  if(*parm) {
    if(!_tcscmp(_T("NONE"), parm)) {
      temp_value = NOPARITY;
    }
    else if (!_tcscmp(_T("ODD"), parm)) {
      temp_value = ODDPARITY;
    }
    else if (!_tcscmp(_T("EVEN"), parm)) {
      temp_value = EVENPARITY;
    }
    else if (!_tcscmp(_T("MARK"), parm)) {
      temp_value = MARKPARITY;
    }
    else if (!_tcscmp(_T("SPACE"), parm)) {
      temp_value = SPACEPARITY;
    }
    else {
      temp_value = NOPARITY;
      _stprintf(dummy, _T("Unknown parity for printer port %s%d!\nParity is set to 'NONE'."),
            port_no[ArrayNumber].port_type,
            port_no[ArrayNumber].port_number);
      MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
    }
  }
  else {
      temp_value = NOPARITY;
      _stprintf(dummy, _T("Unknown parity for printer port %s%d!\nParity is set to 'NONE'."),
            port_no[ArrayNumber].port_type,
            port_no[ArrayNumber].port_number);
      MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
  }
  port_no[ArrayNumber].parity = (short)temp_value;

  /* STOPBITS */
  ReadEnvironmentValue(ActivePort, _T("StopBits"), reg_stopbits, LEN_STOPBITS);
  temp_value = _ttol(reg_stopbits);
  switch(temp_value) {
    case 1:
      temp_value = ONESTOPBIT;
      break;
    case 2:
      temp_value = TWOSTOPBITS;
      break;
    default:
      temp_value = ONESTOPBIT;
      _stprintf(dummy, _T("Unknown stopbits for printer port %s%d!\nStopbits is set to 1."),
            port_no[ArrayNumber].port_type,
            port_no[ArrayNumber].port_number,
            temp_value);
      MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
      break;
  }
  port_no[ArrayNumber].stop_bits = (short)temp_value;

  /* TIMEOUT */
  ReadEnvironmentValue(TREE_PORT_SETTINGS_PRINTER, _T("TimeOut"), reg_timeout, LEN_TIMEOUT);
  if(!*reg_timeout) {
    _stprintf(dummy, _T("Timeout in %s not specified for printer%!\nProgram will be terminated."),
        TREE_PORT_SETTINGS_PRINTER, ArrayNumber+1);
    MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
    return(FAIL);
  }
  temp_value = _ttol(reg_timeout);
  if(temp_value < 0) {
    temp_value = 0;
  }
  if(temp_value > 9999) {
    temp_value = 9999;
  }
  port_no[ArrayNumber].time_out = (short)temp_value;
  
  return(SUCCEED);
} /* ReadCOMPortSettings */

/*---------------------------------------------------------------------------*/
/*                          ReadLPTPortSettings                              */
/*---------------------------------------------------------------------------*/
short ReadLPTPortSettings(_TCHAR *ActivePort, short ArrayNumber)
{
  _TCHAR reg_timeout[LEN_TIMEOUT+1];
  long   temp_value;
  _TCHAR dummy[150];

  /* NO NEED OF THESE PARAMETERS */
  port_no[ArrayNumber].baud_rate = 0;
  port_no[ArrayNumber].data_bits = 0;
  port_no[ArrayNumber].parity    = 0;
  port_no[ArrayNumber].stop_bits = 0;

  /* TIMEOUT */
  ReadEnvironmentValue(TREE_PORT_SETTINGS_PRINTER, _T("TimeOut"), reg_timeout, LEN_TIMEOUT);
  if(!*reg_timeout) {
    _stprintf(dummy, _T("Timeout in %s not specified for printer%!\nProgram will be terminated."),
        TREE_PORT_SETTINGS_PRINTER, ArrayNumber+1);
    MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
    return(FAIL);
  }
  temp_value = _ttol(reg_timeout);
  if(temp_value < 0) {
    temp_value = 0;
  }
  if(temp_value > 9999) {
    temp_value = 9999;
  }
  port_no[ArrayNumber].time_out = (short)temp_value;

  return(SUCCEED);
} /* ReadLPTPortSettings */

/*-----------------------------------.----------------------------------------*/
/*                             GetPrinterSize                                */
/*---------------------------------------------------------------------------*/
short GetPrinterSize(short device_printer)
{
  if((device_printer >= NUMBER_OF_PRINTERS) || device_printer < 0) {
    return(PRINTER_SIZE_UNKNOWN);
  }

  return(printer_no[device_printer].printersize);
} /* GetPrinterSize */
