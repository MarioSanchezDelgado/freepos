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
 */

#ifndef _fpos_string_tools_h_
#define _fpos_string_tools_h_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct tstring {
  short (*fn)(struct tstring *, char *);
  char *format;
  char *cover;
  char *replace;
  char *result;
} TEMPLATE_STRING;


extern void   format_string(TEMPLATE_STRING *, char *);
extern short  r2l_create_string(TEMPLATE_STRING *, char *);
extern short  l2r_create_string(TEMPLATE_STRING *, char *);

extern short  line_len(char *, char *);
extern short  word_len (char *, char *);
extern char *word_cat (char *, char *, char *);
extern char *word_skip(char *, char *);

extern double reverse_sign(char *);
extern void   toggle_sign (char *);

extern char *no_leading_zeros(char *);
extern void   strcpy_no_null  (char *, char *);
extern char *search_printable(char *, char *, char *);
extern void   fmt_vat_perc(double, char *);
extern void   strcat_maxlen(char *, char *, short);

/*extern char    *ConvUnicodeToAnsi(wchar_t *);
extern wchar_t *ConvAnsiToUnicode(char *);
*/
extern char  *lstrip(char*);

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
