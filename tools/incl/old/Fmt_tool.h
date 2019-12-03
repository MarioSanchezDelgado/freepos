/*
 *     Module Name       : FMT_TOOL.H
 *
 *     Type              : Include file formatting string left or right
 *                         justified
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
 * 16-Feb-2006 Added function fmt_centre_justify_string()              J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef _fmt_tool_h_
#define _fmt_tool_h_

#ifdef __cplusplus
extern "C" {
#endif

/* FUNCTIONS                                                                 */
extern _TCHAR* fmt_right_justify_string(_TCHAR*, short, short, _TCHAR*);
extern _TCHAR* fmt_left_justify_string (_TCHAR*, short, short, _TCHAR*);
extern _TCHAR* fmt_centre_justify_string(_TCHAR*, short, short, _TCHAR*);

#ifdef __cplusplus
}
#endif

#endif
