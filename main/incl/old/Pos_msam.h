/*
 *     Module Name       : POS_MSAM.H
 *
 *     Type              : Pos Multisam discount processing
 *
 *
 *     Author/Location   : Getronics, Distribution & Retail, Nieuwegein
 *
 *     Copyright Getronics N.V.
 *               Bakenmonde 1
 *               3434KK NIEUWEGEIN
 *               The Netherlands
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 06-Jan-2000 Initial Release WinPOS                                P.M/R.L.
 * 17-Jan-2000 Unicode                                               M.W.
 * --------------------------------------------------------------------------
 * 22-Jul-2002 Multisam Phase 3.                                         P.M.
 * --------------------------------------------------------------------------
 * 27-Aug-2003 Added disc_incl/excl_amnt_per_vat_cd in ACTION_RESULT
 *             to be able to calculate total carried forward totals
 *             correctly.                                              J.D.M.
 * --------------------------------------------------------------------------
 * 22-Mar-2004 Added vat totals to the action results.                 J.D.M.
 * --------------------------------------------------------------------------
 * 12-Oct-2004 Bugfix: added #ifdef __cplusplus at the end, else
 *             problems with including in cpp files.                   J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_MSAM_H__
#define __POS_MSAM_H__

#include "Sll_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define T_DISC_ART     1
#define T_DISC_GIFT    2
#define T_DISC_AMNT    3
#define T_DISC_PERC    4
                    /* 5 */
#define T_DISC_PREMIUM 6

typedef struct {
  long    mmail_no;
  short   maction_no;
  long    version_no;
  short   times;
  short   action_type;
  short   result_type;
  long    threshold_qty;
  long    discount_qty;
  _TCHAR  line_text[81];
  double  disc_incl;
  double  disc_excl;
  double  disc_vat;
  double  disc_incl_amnt_per_vat_cd[10];
  double  disc_excl_amnt_per_vat_cd[10];
  double  disc_vat_amnt_per_vat_cd[10];
  double  threshold_amount;
  double  discount_amount;
  double  excluded_amount;
} ACTION_RESULT;


extern SLL action_results;
extern SLL crazy_art_hist;
extern ACTION_RESULT prn_msam_action;


extern int  apply_discount(short, long, short, short, double, LLIST*);
extern void multisam_discounts(short);
extern int  reset_multisam_discounts(void);
extern void init_invoice_msam_disc_totals(void);

#ifdef __cplusplus
}
#endif

#endif
