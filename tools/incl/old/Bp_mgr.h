/*
 *     Module Name       : BP_MGR.H
 *
 *     Type              : Include file buffered printing module
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
 * 21-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef _bp_mgr_h_
#define _bp_mgr_h_

#ifdef __cplusplus
extern "C" {
#endif

/* USER DEFINED ARRAY                                                        */
extern short (*bp_alt_fn[])(short, short);

/* CONSTANT DEFINITION                                                       */
extern const short bp_number;

/* TYPE DEFINITIONS                                                          */
typedef struct bufprint
{
  short bp_id;                  /* element identification                    */
  short station;                /* print station to use                      */
  struct bufprint *next;        /* pointer to next element in list           */
} BP_PRINT; 
             
/* FUNCTIONS                                                                 */
extern short bp_init(short, short);
extern void  bp_deinit(void);
extern short bp_now(short, short, short);
extern short bp_get_printer_nr(short);

#ifdef __cplusplus
}
#endif

#endif
