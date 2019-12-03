/*
 *     Module Name       : POS_CHQ.H
 *
 *     Type              : Include file application print functions
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
 * --------------------------------------------------------------------------
 */

#ifndef __POS_CHQ_H__
#define __POS_CHQ_H__

#ifdef __cplusplus
extern "C" {
#endif


#define REV_NBR_LINES         3         /* number of reverse feed lines      */
#define CH_WIDTH             52 // 65         /* cheque width (number of chars)    */  
#define CH_LINES             14         /* cheque number of lines            */
#define CH_START              4         /* cheque offset in lines            */

#define CH_ANUM_OFFS_1        6         /* start of alphanum amount line 1   */
#define CH_ANUM_OFFS_2        1         /* start of alphanum amount line 2   */
#define CH_ANUM_SIZE_1       (CH_WIDTH -  3 - CH_ANUM_OFFS_1)
#define CH_ANUM_SIZE_2       (CH_WIDTH - 19 - CH_ANUM_OFFS_2)
#define CH_ANUM_OVFLW_1       0         /* part of alphanum amount line reserved for overflow */
#define CH_ANUM_OVFLW_2       13         /* part of alphanum amount line reserved for overflow */


extern short print_ch_side_1(LOGICAL_PRINTER);
extern short init_cheque_printers(void);


#ifdef __cplusplus
}
#endif

#endif
