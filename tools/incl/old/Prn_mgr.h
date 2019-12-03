/*
 *     Module Name       : Prn_mgr.h
 *
 *     Type              : Include file general print manager
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
 * 13-Dec-1999 Initial Release WinPOS                                  E.J.
 * --------------------------------------------------------------------------
 */

#ifndef __PRN_MGR_H__
#define __PRN_MGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define INV_SIZE       128
#define SMALL_INV_SIZE  40

/* All printer defines must be in sequence! */
#define DEV_PRINTER1          0         /* Printer no. 1 Normal printer      */
#define DEV_PRINTER2          1         /* Printer no. 2 Small printer       */
#define DEV_PRINTER3          2         /* RESERVED for future use!          */
#define DEV_PRINTER4          3         /* RESERVED for future use!          */
#define NUMBER_OF_PRINTERS    2

/* LAYOUT OF INVOICE */
enum PRINTER_SIZE {
  PRINTER_SIZE_NOT_USED,
  PRINTER_SIZE_NORMAL,
  PRINTER_SIZE_SMALL,
  PRINTER_SIZE_SMALL_INV,
  PRINTER_SIZE_UNKNOWN  /* allways last! */
};

/* PRINT COLORS */
enum PRINT_COLOR {
  PRINT_COLOR_BLACK,
  PRINT_COLOR_RED,
  PRINT_COLOR_UNKNOWN  /* allways last! */
};


/* ERRORS                                                                     */
#define EC_PRN_NOT_READY   -1299
#define EC_PRN_NO_PAPER    -1298

/* ERRORS from prn7155.h */
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

/* MACROS                                                                     */
#define OUT_OFF_PAPER                     _T('\x20')
#define IO_ERROR                          _T('\x08')
#define LINE_FEED                         _T('\x0A')
#define FORM_FEED                         _T('\x0C')
#define CARRIAGE_RETURN                   _T('\x0D')
#define ESCAPE                            _T('\x1B')
#define INIT_PRINTER                      _T('\x40')
#define SELECT_PRINTER                    _T('\x11')
#define CANCEL_COMPR                      _T('\x12')
#define SELECT_ELITE                      _T('\x4D')
#define SELECT_PICA                       _T('\x50')
#define SELECT_15CPI                      _T('\x67')
#define COMPR_PRINT                       _T('\x0F')
#define SELECT_6LPI                       _T('\x32')
#define SELECT_8LPI                       _T('\x30')

/* FUNCTIONS                                                                  */
extern short ec_prn_init(_TUCHAR *, short, short);
extern void  ec_prn_exit(short);
extern short ec_prn_print(char *, short);
extern short ec_prn_raw(_TUCHAR *, short, short);
extern short ec_prn_lf(short);
extern short ec_prn_ff(short);
extern short ec_prn_cut(short);
extern short ec_prn_color(short, short);
extern short IsPrinterError(short);

extern short ReadPrintEnvironment(void);
extern short GetPrinterSize(short);
extern char GetPrintingAttribute(short);

/*
 * DEFINITION OF TEXT ATTRIBUTES
 */
enum TEXT_ATTRIBUTES {
  BOLD,
  UNDERLINE,
  ITALIC,
  NLQ,
  LAST_ATTR /* Always last! */
};
#define DRAFT     LAST_ATTR /* Don't change! */

extern char reg_bold[];
extern char reg_underline[];
extern char reg_italic[];
extern char reg_nlq[];

extern char not_printable_attributes[];

#define LEN_PRINTER_TYPE  40

/* 13-Jul-2011 acm - { */
#define LEN_PRINTMODEL    500
/* 13-Jul-2011 acm - } */

typedef struct {
  char         printer[LEN_PRINTER_TYPE+1];
  short          printersize;
  short          attached;
  char		 printModel[LEN_PRINTMODEL+1] ; // 13-Jul-2011 acm - 
} PRINTER_INFO;

extern short          printers_attached;  /* Number of printers attached to the POS */
extern PRINTER_INFO   pos_printer_info[]; /* Printer info to select printer         */

#ifdef __cplusplus
}
#endif

#endif
