/*
 *     Module Name       : STRI_TLS.H
 *
 *     Type              : Include file several string processing functions
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
 * 10-Nov-2000 Added new prototypes.                                   R.N.B.
 *             Changed UnicodeToAnsi and AnsiToUnicode makros
 * --------------------------------------------------------------------------
 * 09-Oct-2002 Added lstrip().                                         J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef _stri_tls_h_
#define _stri_tls_h_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tstring {
  short (*fn)(struct tstring *, _TCHAR *);
  _TCHAR *format;
  _TCHAR *cover;
  _TCHAR *replace;
  _TCHAR *result;
} TEMPLATE_STRING;


extern void   format_string(TEMPLATE_STRING *, _TCHAR *);
extern short  r2l_create_string(TEMPLATE_STRING *, _TCHAR *);
extern short  l2r_create_string(TEMPLATE_STRING *, _TCHAR *);

extern short  line_len(_TCHAR *, _TCHAR *);
extern short  word_len (_TCHAR *, _TCHAR *);
extern _TCHAR *word_cat (_TCHAR *, _TCHAR *, _TCHAR *);
extern _TCHAR *word_skip(_TCHAR *, _TCHAR *);

extern double reverse_sign(_TCHAR *);
extern void   toggle_sign (_TCHAR *);

extern _TCHAR *no_leading_zeros(_TCHAR *);
extern void   strcpy_no_null  (_TCHAR *, _TCHAR *);
extern _TCHAR *search_printable(_TCHAR *, _TCHAR *, _TCHAR *);
extern void   fmt_vat_perc(double, _TCHAR *);
extern void   strcat_maxlen(_TCHAR *, _TCHAR *, short);

extern char    *ConvUnicodeToAnsi(wchar_t *);
extern wchar_t *ConvAnsiToUnicode(char *);

extern _TCHAR  *lstrip(_TCHAR*);

                     /* The defines below can be used to make a */
                     /* conversion context dependent            */
#ifdef _UNICODE
#define       UnicodeToAnsi(u)  ConvUnicodeToAnsi(u) /* convert */
#define       AnsiToUnicode(a)  ConvAnsiToUnicode(a) /* convert */
#else 
#define       UnicodeToAnsi(u)  (u)               /* do nothing */
#define       AnsiToUnicode(a)  (a)               /* do nothing */
#endif /* _UNICODE */

#ifdef __cplusplus
}
#endif

#endif
