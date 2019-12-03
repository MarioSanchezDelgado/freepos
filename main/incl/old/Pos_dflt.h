/*
 *     Module Name       : POS_DFLT.H
 *
 *     Type              : Include file application Default Functions
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

#ifndef __POS_DFLT_H__
#define __POS_DFLT_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void no_DFLT(INPUT_DISPLAY *, _TCHAR *);
extern void no_DFLT_2(INPUT_DISPLAY *, _TCHAR *);
extern void price_DFLT(INPUT_DISPLAY *, _TCHAR *);
extern void minus_DFLT(INPUT_DISPLAY *, _TCHAR *);
extern void DFLT_last_condition(INPUT_DISPLAY *, _TCHAR *);
extern void art_no_DFLT(INPUT_DISPLAY *, _TCHAR *);

extern void value_DFLT(INPUT_DISPLAY *x, _TCHAR *data);

extern void perception_name_DFLT(INPUT_DISPLAY *x, _TCHAR *data);
extern void perception_doc_DFLT(INPUT_DISPLAY *x, _TCHAR *data);

extern void name_DFLT(INPUT_DISPLAY *x, _TCHAR *data);
extern void doc_DFLT(INPUT_DISPLAY *x, _TCHAR *data);

#ifdef __cplusplus
}
#endif

#endif
