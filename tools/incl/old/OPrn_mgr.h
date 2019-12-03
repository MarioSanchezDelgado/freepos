/*
 *     Module Name       : OPrn_mgr.h
 *
 *     Type              : Include file OPOS print manager
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

#ifndef OPRN_MGR_H
#define OPRN_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

                            /* Printer stations (low byte); don't use */
#define PTR_S_JOURNAL        0x01
#define PTR_S_RECEIPT        0x02
#define PTR_S_SLIP           0x04

                    /* physical printer device (high byte); don't use */
#define DEV_OPRINTER1        0x0000
#define DEV_OPRINTER2        0x0100
#define NR_OPRINTERS         2      /* max number of physical devices */

                   /* Logical printer ID's; these should be used.     */
                   /* A logical printer ID contains information about */
                   /* the physical device and the printer station     */
typedef enum LOGICAL_PRINTER_ID {
  JOURNAL_PRINTER1  =  (DEV_OPRINTER1 + PTR_S_JOURNAL),
  RECEIPT_PRINTER1  =  (DEV_OPRINTER1 + PTR_S_RECEIPT),
  SLIP_PRINTER1     =  (DEV_OPRINTER1 + PTR_S_SLIP),
  JOURNAL_PRINTER2  =  (DEV_OPRINTER2 + PTR_S_JOURNAL),
  RECEIPT_PRINTER2  =  (DEV_OPRINTER2 + PTR_S_RECEIPT),
  SLIP_PRINTER2     =  (DEV_OPRINTER2 + PTR_S_SLIP)
} LOGICAL_PRINTER;

                                    /* "Rotation" Parameter Constants */
#define PTR_RP_NORMAL        0x0001
#define PTR_RP_RIGHT90       0x0101
#define PTR_RP_LEFT90        0x0102
#define PTR_RP_ROTATE180     0x0103


extern short oprn_init(LOGICAL_PRINTER);
extern void  oprn_deinit(LOGICAL_PRINTER);
extern long  oprn_print_immediate(LOGICAL_PRINTER, _TCHAR *);     
extern long  oprn_print_normal(LOGICAL_PRINTER, _TCHAR *);     
extern long  oprn_wait_for_slip(LOGICAL_PRINTER printer, long);
extern long  oprn_close_slip_jaws(LOGICAL_PRINTER printer);
extern long  oprn_wait_for_slip_removal(LOGICAL_PRINTER, long);
extern long  oprn_end_removal(LOGICAL_PRINTER);
extern long  oprn_get_state(LOGICAL_PRINTER);
extern short oprn_is_initialised(LOGICAL_PRINTER);
extern long  oprn_clear_output(LOGICAL_PRINTER);
extern long  oprn_rotate_print(LOGICAL_PRINTER, long);
extern long  oprn_get_bool_property(LOGICAL_PRINTER, LPOLESTR);
extern long  oprn_direct_io(LOGICAL_PRINTER, BSTR);

                             /* OPOS errors */
#define OPOS_SUCCESS           0
#define OPOS_E_CLOSED          101
#define OPOS_E_CLAIMED         102
#define OPOS_E_NOTCLAIMED      103
#define OPOS_E_NOSERVICE       104
#define OPOS_E_DISABLED        105
#define OPOS_E_ILLEGAL         106
#define OPOS_E_NOHARDWARE      107
#define OPOS_E_OFFLINE         108
#define OPOS_E_FAILURE         111
#define OPOS_E_TIMEOUT         112
#define OPOS_E_BUSY            113
#define OPOS_E_EXTENDED        114
                              /* OPOS extended errors */
#define OPOS_EPTR_COVER_OPEN   201
#define OPOS_EPTR_JRN_EMPTY    202
#define OPOS_EPTR_REC_EMPTY    203
#define OPOS_EPTR_SLP_EMPTY    204
#define OPOS_EPTR_SLP_FORM     205
                                        /* OPRN manager errors */
#define OPRN_INVALID_PRINTER  -100


#ifdef __cplusplus
}
#endif

#endif
