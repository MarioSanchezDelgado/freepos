/*
 *     Module Name       : POS_TXT.H
 *
 *     Type              : Include file Application TEXT Structures
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
 */

#ifndef __POS_TXT_H__
#define __POS_TXT_H__

#ifdef __cplusplus
extern "C" {
#endif

/********/
 
extern _TCHAR *prn_ch_TXT[];
 
extern _TCHAR *one_till_nine[];
extern _TCHAR *eleven_till_nineteen[];
extern _TCHAR *ten_till_ninety[];
extern _TCHAR *exception[];

#if LANGUAGE==GREEK || LANGUAGE==ESPANOL
extern _TCHAR *hundred_till_ninehundred[];
extern _TCHAR *inter_text1[];
#endif

extern _TCHAR *inter_text[];
 
/*******/

extern _TCHAR *err_msg_TXT[];
extern _TCHAR *scrn_inv_TXT[];
extern _TCHAR *prompt_TXT[];
extern _TCHAR *menu_TXT[];
extern _TCHAR *input_TXT[];
extern _TCHAR *appr_msg_TXT[];
extern _TCHAR *cdsp_TXT[];
extern _TCHAR *prn_xr_TXT[];
extern _TCHAR *prn_inv_TXT[];
extern _TCHAR *mon_names[];


/***Jonathan Peru***/
/*Soporte de X y Z en tirilla*/
extern _TCHAR *prn_xr_TXT_tirilla[];


#ifdef __cplusplus
}
#endif

#endif