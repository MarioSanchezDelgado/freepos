/*
 *     Module Name       : POS_SCRL.H
 *
 *     Type              : Include file scroll windows functions
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
 * 05-Dec-2001 Changes for Article Finder.                               M.W.
 * --------------------------------------------------------------------------
 */

#ifndef __POS_SCRL_H__
#define __POS_SCRL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/*    These macro's are used in the application like:                        */
/*          scrl_add_line(window, SCRL_INV_ART_HEADER, tdm_group_id)         */
/*                                                                           */
/*    The macro is the offset of a build function in *scrl_functions[]       */
/*****************************************************************************/

#define SCRL_INV_ART_HEADER     0
#define SCRL_INV_ART_LINE       1
#define SCRL_INV_DISCNT_LINE    2
#define SCRL_INV_DEPOSIT_LINE   3
#define SCRL_ART_FIND_LINE      4

extern short init_scrl(void);

extern TM_INDX selected_item;

#ifdef __cplusplus
}
#endif

#endif
