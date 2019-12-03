/*
 *     Module Name       : POS_TOT.H
 *
 *     Type              : Application Data Names
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
 * 10-Jan-1999 Added Multisam discount totals                            P.M.
 * --------------------------------------------------------------------------
 * 22-Jan-2002 define of MAX_AMOUNT                                      M.W.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 22-Jul-2002 Multisam Phase 3.                                         P.M.
 * --------------------------------------------------------------------------
 * 27-Aug-2003 Added extra total carried forward totals to be able to
 *             calculate the total carried forward inclusive correctly.
 *                                                                     J.D.M.
 * --------------------------------------------------------------------------
 * 17-Feb-2004 A Separate 'carried forward' total is used to store           
 *             MultiSam discounts in order to avoid rounding diffs.      P.M.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_TOT_H__
#define __POS_TOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TOTAL_BUF_SIZE     18 /* Length of a total                           */

#define TOTAL               1 /* used in prnt_packs_part                     */
#define SUBTOTAL            2 /* used in prnt_packs_part                     */

#define PAYM_WAY_0          0 /* payment way 0                               */
#define PAYM_WAY_1          1 /* payment way 1                               */
#define PAYM_WAY_2          2 /* payment way 2                               */
#define PAYM_WAY_3          3 /* payment way 3                               */
/*      ..........          .  */
#define PAYM_WAY_8          8 /* payment way 8                               */
#define PAYM_WAY_9          9 /* payment way 9                               */

#define MAX_PAYM_WAYS      10 /* total number of payment ways                */

#define TOT_EXCL_0          0 /* total amnt excl.vat of art with vat cd 0    */
#define TOT_EXCL_1          1 /* total amnt excl.vat of art with vat cd 1    */
/*      ..........         ..  */
#define TOT_EXCL_8          8 /* total amnt excl.vat of art with vat cd 9    */
#define TOT_EXCL_9          9 /* total amnt excl.vat of art with vat cd 9    */

#define TOT_VAT_0          10 /* total vat amnt of vat cd 0                  */
#define TOT_VAT_1          11 /* total vat amnt of vat cd 1                  */
/*      .........          ..  */
#define TOT_VAT_9          19 /* total vat amnt of vat cd 9                  */


#define TOT_INCL_0         20 /* total amnt incl.vat of art with vat cd 0    */
#define TOT_INCL_1         21 /* total amnt incl.vat of art with vat cd 1    */
/*      ..........         ..  */
#define TOT_INCL_9         29 /* total amnt incl.vat of art with vat cd 9    */

#define TOT_GEN_EXCL       30 /* total amnt excl.vat of all articles         */
#define TOT_GEN_VAT        31 /* total vat amnt of all articles              */
#define TOT_GEN_INCL       32 /* total amnt incl.vat of all articles         */
#define TOT_PAID           33 /* total amnt paid by the customer             */
#define SUB_TOT_GEN_INCL   34 /* subtotal amnt incl.vat of all articles      */
#define TOT_CARR_FORWD_INCL 35/* total carried forward to next page incl. vat*/

#define TOT_CREDIT_AMNT    36 /* total credit-extra amount exclusive vat     */
#define TOT_CREDIT_VAT_AMNT 37 /* total credit-extra vat amount              */
#define TOT_CREDIT_VAT_NO  38  /* credit-extra vat code                      */
#define TOT_CREDIT_PAYM_CD 39  /* credit-extra payment code                  */

#define TOT_PACKS          40 /* total qty of packs                          */
#define TOT_SUB_PACKS      41 /* total qty of packs per subtotal             */

#define TOT_CHANGE         42 /* amount change                               */
#define TOT_CHANGE_CD      43 /* amount change booked on TOT_PAYM_0 + .._CD  */

#define TOT_PAYM_0         50 /* total amnt paid with payment way 0          */
#define TOT_PAYM_1         51 /* total amnt paid with payment way 1          */
/*      ..........         ..  */
#define TOT_PAYM_9         59 /* total amnt paid with payment way 9          */

#define SUB_TOT_EXCL_0     60 /* subtotal amnt excl.vat of art with vat cd 0 */
#define SUB_TOT_EXCL_1     61 /* subtotal amnt excl.vat of art with vat cd 1 */
/*      ..............     ..  */
#define SUB_TOT_EXCL_9     69 /* subtotal amnt excl.vat of art with vat cd 9 */

#define SUB_TOT_VAT_0      70 /* subtotal vat amnt of vat cd 0               */
#define SUB_TOT_VAT_1      71 /* subtotal vat amnt of vat cd 1               */
/*      .............      ..  */
#define SUB_TOT_VAT_9      79 /* subtotal vat amnt of vat cd 9               */

#define SUB_TOT_INCL_0     80 /* subtotal amnt incl.vat of art with vat cd 0 */
#define SUB_TOT_INCL_1     81 /* subtotal amnt incl.vat of art with vat cd 1 */
/*      ..............     ..  */
#define SUB_TOT_INCL_9     89 /* subtotal amnt incl.vat of art with vat cd 9 */

#define TOT_LCREDIT_AMNT     90 /* total credit-extra amount exclusive vat   */
#define TOT_LCREDIT_VAT_AMNT 91 /* total credit-extra vat amount             */
#define TOT_FEE_AMOUNT       92 /* total of membership fee amount            */

#define MSAM_DISC_TOT_EXCL_0     93 /* total msam disc amnt excl.vat of art with vat cd 0 */
/*      ....................    ...  */
#define MSAM_DISC_TOT_EXCL_9    102 /* total msam disc amnt excl.vat of art with vat cd 9 */

#define MSAM_DISC_TOT_VAT_0     103 /* total msam disc vat amnt of vat cd 0  */
/*      ...................     ...  */
#define MSAM_DISC_TOT_VAT_9     112 /* total msam disc vat amnt of vat cd 9  */

#define MSAM_DISC_TOT_INCL_0    113 /* total msam disc amnt incl.vat of art with vat cd 0 */
/*      ....................    ...  */
#define MSAM_DISC_TOT_INCL_9    122 /* total msam disc amnt incl.vat of art with vat cd 0 */

#define MSAM_DISC_TOT_GEN_EXCL  123 /* total amnt excl. vat of msam discount */
#define MSAM_DISC_TOT_GEN_VAT   124 /* total vat amnt of msam discount       */
#define MSAM_DISC_TOT_GEN_INCL  125 /* total amnt incl. vat of msam discount */

#define MSAM_TOT_DISC1          126 /* total amnt excl. vat of discounts type 1 */
#define MSAM_TOT_DISC2          127 /* total amnt excl. vat of discounts type 2 */

#define TOT_CARR_FORWD_EXCL_0   128 /* total carried forward to next page excl. vat of art with vat cd 0 */
/*      ....................    ...  */
#define TOT_CARR_FORWD_EXCL_9   137 /* total carried forward to next page excl. vat of art with vat cd 9 */

#define TOT_CARR_FORWD_VAT_0    138 /* total carried forward to next page vat amnt of vat cd 0           */
/*      ....................    ...  */
#define TOT_CARR_FORWD_VAT_9    147 /* total carried forward to next page vat amnt of vat cd 9           */

#define TOT_CARR_FORWD_INCL_0   148 /* total carried forward to next page incl. vat of art with vat cd 0 */
/*      ....................    ...  */
#define TOT_CARR_FORWD_INCL_9   157 /* total carried forward to next page incl. vat of art with vat cd 9 */

#define TOT_CARR_FORWD_VAT      158 /* total carried forward to next page vat amount  */
#define TOT_CARR_FORWD_EXCL     159 /* total carried forward to next page excl. vat   */

#define TOT_CARR_FMSAM_EXCL_0   160 /* total carried forward to next page excl. vat of art with vat cd 0 - MSAM Discounts */
/*      ....................    ...  */
#define TOT_CARR_FMSAM_EXCL_9   169 /* total carried forward to next page excl. vat of art with vat cd 9 - MSAM Discounts  */

#define TOT_CARR_FMSAM_VAT_0    170 /* total carried forward to next page vat amnt of vat cd 0 - MSAM Discounts */
/*      ....................    ...  */
#define TOT_CARR_FMSAM_VAT_9    179 /* total carried forward to next page vat amnt of vat cd 9 - MSAM Discounts */

#define TOT_CARR_FMSAM_INCL_0   180 /* total carried forward to next page incl. vat of art with vat cd 0 - MSAM Discounts */
/*      ....................    ...  */
#define TOT_CARR_FMSAM_INCL_9   189 /* total carried forward to next page incl. vat of art with vat cd 9 - MSAM Discounts */

#define TOT_CARR_FMSAM_VAT      190 /* total carried forward to next page vat amount - MSAM Discounts */
#define TOT_CARR_FMSAM_EXCL     191 /* total carried forward to next page excl. vat  - MSAM Discounts */
#define TOT_CARR_FMSAM_INCL     192 /* total carried forward to next page incl. vat  - MSAM Discounts */

#define CALC_INP                193 /* input value for the calculator              */
#define CALC_TOT                194 /* calculated value returned by the calculator */

#define AMOUNT_IN_DRAWER        195 /* Cash amount in drawer                       */

#define MAX_TOTALS     (AMOUNT_IN_DRAWER+1)                  /* for initialisation */

#define MAX_AMOUNT_ON_POS  99999999999.0 /* Highest amount on Pos                  */

#ifdef __cplusplus
}
#endif

#endif
