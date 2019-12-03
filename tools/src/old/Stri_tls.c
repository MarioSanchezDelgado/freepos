/*
 *     Module Name       : STRI_TLS.C
 *
 *     Type              : Several string processing functions
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
 * 10-Nov-2000 Improved Ansi to Unicode and vice versa functions       R.N.B.
 * --------------------------------------------------------------------------
 * 09-Oct-2002 Added lstrip().                                         J.D.M.
 * --------------------------------------------------------------------------
 */
#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>
 
#include <math.h>

#include "stri_tls.h"
#include "storerec.h"


/*---------------------------------------------------------------------------*/
/*                             format_string                                 */
/*---------------------------------------------------------------------------*/
void format_string(TEMPLATE_STRING *create, _TCHAR *data)
{
  if (create) {
    create->fn(create,data);
  }
} /* format_string */


/*---------------------------------------------------------------------------*/
/*                             r2l_create_string                             */
/*---------------------------------------------------------------------------*/
short r2l_create_string(TEMPLATE_STRING *create, _TCHAR *data)
{
  /*                                                                       */
  /* This function has the same functionality as r2l_display() exept for   */
  /* the following two things:                                             */
  /*                                                                       */
  /* 1. After the format-string is copied in the workspace-string and after*/
  /*    the workspace-string is replaced by all characters in data,        */
  /*    the remaining characters in the workspace are replaced by the      */
  /*    replacement-characters (only if they exist in the replacement-     */
  /*    string). With this feature it is possible to clear comma's like    */
  /*    this:  "  , 12.00"  ->  "    12.00"                                */
  /*                                                                       */
  /* 2. Another feature is (in case of numbers) to place the minus-sign    */
  /*    right of the number, or not at all. Like this:                     */
  /*       "-123" -> r2l_create_string() -> "1.23-"   Right of the number  */
  /*       "Ab-c" -> r2l_create_string() -> "Ab-c"    Let it be            */
  /*                                                                       */

  static _TCHAR workspace[100];
  _TCHAR *format_pointer, *data_pointer, *replace_pos, *minus;

  _tcscpy(workspace, create->format);         /* workspace is resulting string */

  if(_tcslen(workspace)) {  /* to prevent reading and writing beyond boundaries */
    format_pointer = workspace + _tcslen(workspace);
    minus = format_pointer-1;                 /* Determine minus position      */
    if (*minus != _T('-')) {
      minus=(_TCHAR *)NULL;                   /* No minus in format-string     */
    }
    else {
      *minus=_T('~');                         /* Clear minus, set it if in data*/
    }

    data_pointer = data + _tcslen(data);      /* Start with last char. entered */

    while (format_pointer > workspace) {      /* work right to left            */
      format_pointer--;
      if (data_pointer > data) {              /* handle the data               */
        data_pointer--;
        if (*data_pointer == _T('-') && minus != (_TCHAR *)NULL) {
          *minus=_T('-');                       /* minus found and in format str */
          if (minus != format_pointer) {
            format_pointer++;
          }
        }
        else {
          /* If it's ok according to the cover-string, to put the char on    */
          /* this position of the workspace, do it                           */
          if (_tcschr(create->cover,*format_pointer)) {
            *format_pointer = *(data_pointer);
          }
          else {
            data_pointer++; 
          }
        }
      }
      else {            /* data_pointer<=data, so handle rest of workspace   */
        if ( (_tcslen(create->replace) > 1) &&
            !(_tcslen(create->replace)%2) ) {    /* must be 2, 4, 6 etc       */
          /* End of data, process replacement characters                     */

          /* If the current char exists in the replacement-string,           */
          /* find out if it is a character to replace (uneven position       */
          /* in replacement-string) or a character to replace with.          */

          replace_pos=_tcschr(create->replace,*format_pointer);
          if (replace_pos && !((replace_pos-create->replace)%2)) {
            *format_pointer = *(replace_pos+1);
          }
        }
      }
    }

    /* If data is larger than work_space (data is truncated), look for a     */
    /* possible minus-sign (only if minus-sign not yet found and in form-str */
    while( (minus != (_TCHAR *)NULL) &&
           (*minus != _T('-')) && (data_pointer > data) ) {
      data_pointer--;
      if (*data_pointer == _T('-')) {
        *minus=_T('-');
      }
    } /* THIS WHILE LOOP IS NOT TESTED YET IN THE NEW SITUATION : MWN */

    /* Finaly, if no minus sign is found, replace the dummy '~' character    */
    /* with a space.                                                         */
    if (minus != (_TCHAR *)NULL) {
      if (*minus==_T('~')) {
        *minus=_T(' ');
      }
    }

    create->result=workspace;
  }
  else { /* just the workspace as is */
    create->result=workspace;
  }

  return(SUCCEED);
} /* r2l_create_string */


/*---------------------------------------------------------------------------*/
/*                             l2r_create_string                             */
/*---------------------------------------------------------------------------*/
short l2r_create_string(struct tstring *create, _TCHAR *data)
{
  /* This function has the same functionality as l2r_display() exept for   */
  /* the following two things:                                             */
  /* 1. After the format-string is copied in the workspace-string and after*/
  /*    the workspace-string is replaced by all characters in data,        */
  /*    the remaining characters in the workspace are replaced by the      */
  /*    replacement-characters (only if they exist in the replacement-     */
  /*    string). With this feature it is possible to clear comma's like    */
  /*    this:  "  , 12.00"  ->  "    12.00"                                */
  /*                                                                       */
  /* 2. Another feature is (in case of numbers) to place the minus-sign    */
  /*    right of the number, or not at all. Like this:                     */
  /*       "-123" -> r2l_create_string() -> "1.23-"   Right of the number  */
  /*       "Ab-c" -> r2l_create_string() -> "Ab-c"    Let it be            */
  /*                                                                       */
  static _TCHAR workspace[100];
  _TCHAR *format_pointer, *data_pointer, *replace_pos, *minus;

  _tcscpy(workspace, create->format);

  if(_tcslen(workspace)) { /* to prevent reading and writing beyond boundaries */

    format_pointer = workspace;
    minus = workspace + _tcslen(workspace)-1;   /* Determine minus position    */
    if (*minus!=_T('-')) { 
      minus=(_TCHAR *)NULL;                   /* No minus in format-string     */
    }
    else {
      *minus=_T('~');                           /* Clear minus, set it if in data*/
    }

    data_pointer = data;                    /* Start with first char entered */

    while (*format_pointer != 0) {          /* work left to right            */
      if (*data_pointer != 0) {             /* handle the data               */
        if (*data_pointer == _T('-') && minus != (_TCHAR *)NULL) {
          *minus=_T('-');                       /* minus found and in format str */
          if (minus == format_pointer) {
            ++format_pointer;
          }
          ++data_pointer;
        }
        else {
          /* put the char on this position of the workspace, if it's legal   */
          if (_tcschr(create->cover,*format_pointer)) {
            *format_pointer = (*data_pointer)++;
          }
          ++format_pointer;
        }
      }
      else {              /*   *data_pointer==0, so handle rest of workspace */
        if ( (_tcslen(create->replace) > 1) &&
            !(_tcslen(create->replace)%2) ) {        /* must be 2, 4, 6 etc   */
          /* End of data, process replacement characters                     */

          /* If the current char exists in the replacement-string,           */




          /* find out if it is a character to replace (uneven position       */
          /* in replacement-string) or a character to replace with.          */

          replace_pos=_tcschr(create->replace,*format_pointer);
          if (replace_pos && !((replace_pos-create->replace)%2)) {
            *format_pointer = *(replace_pos+1);
          }
        }
        ++format_pointer;
      }
    }

    /* If data is larger than work_space (data is truncated) look for a      */
    /* possible minus-sign (only if minus-sign not yet found and in form-str */

    while( (minus != (_TCHAR *)NULL) &&
           (*minus != _T('-')) &&
           (*data_pointer != 0) ) {
      if (*data_pointer == _T('-')) {
        *minus=_T('-');
      }
      else {
        ++data_pointer;
      }
    }

    /* Finaly, if no minus sign is found, replace the dummy '~' character    */
    /* with a space.                                                         */

    if (minus != (_TCHAR *)NULL) {
      if (*minus==_T('~')) {
        *minus=_T(' ');
      }
    }
 
    create->result=workspace;
  }
  else { /* just the workspace as is */
    create->result=workspace;
  }
  return(SUCCEED);
} /* l2r_create_string */


/*---------------------------------------------------------------------------*/
/*                             no_leading_zeros                              */
/*---------------------------------------------------------------------------*/
/* Remove leading zero's from 'src' leaving signs '+' and '-' intact.        */
/*---------------------------------------------------------------------------*/
_TCHAR *no_leading_zeros(_TCHAR *src)
{
  _TCHAR *p, *q;

  if (*src == _T('-') || *src == _T('+')) {
    p=q=src+1;
  }
  else {
    p=q=src;
  }

  while (*p == _T('0')) {
    p++;
  }

  if (q != p) {
    while (*p) {
      *q++=*p++;
    }
    *q=*p;
  }

  return(src);
} /* no_leading_zeros */


/*---------------------------------------------------------------------------*/
/*                             strcpy_no_null                                */
/*---------------------------------------------------------------------------*/
void strcpy_no_null(_TCHAR *dest, _TCHAR *src)
{
  while (*src) {
    *dest++ = *src++;
  }
} /* strcpy_no_null */

/*---------------------------------------------------------------------------*/
/*                             line_len                                      */
/*---------------------------------------------------------------------------*/
/* Returns length of line excluding break-characters.                        */
/*---------------------------------------------------------------------------*/
short line_len(_TCHAR *line, _TCHAR *break_str)
{
  short i;

  for (i=0; *line; line++) {
    if (_tcschr(break_str, *line) == NULL) {
      i++;
    }
  }

  return(i);
} /* line_len */

/*---------------------------------------------------------------------------*/
/*                             word_len                                      */
/*---------------------------------------------------------------------------*/
/* Returns length of the word pointed to by 'str' till first occurence of    */
/* a break_str member.                                                       */
/*---------------------------------------------------------------------------*/
short word_len(_TCHAR *str, _TCHAR *break_str)
{
  _TCHAR *p;

  for (p=str; *p && _tcschr(break_str, *p) == NULL; p++) {
    ;
  }

  return((short)(p-str));
} /* word_len */


/*---------------------------------------------------------------------------*/
/*                             word_cat                                      */
/*---------------------------------------------------------------------------*/
/* Appends the first word from str2 delimited by a member of break_str too   */
/* str1.                                                                     */
/*---------------------------------------------------------------------------*/
_TCHAR *word_cat(_TCHAR *str1, _TCHAR *str2, _TCHAR *break_str)
{
  _TCHAR *p;

  for (p=str1; *p; p++) {
    ;
  }
  while (*str2 && _tcschr(break_str, *str2) == NULL) {
    *p++=*str2++;
  }
  *p=_T('\0');

  return(str1);
} /* word_cat */


/*---------------------------------------------------------------------------*/
/*                             word_skip                                     */
/*---------------------------------------------------------------------------*/
/* Finds first occurence in str of a break_str member, then returns a        */
/* p[ointer which points to the first non-member of break_str character.     */
/*---------------------------------------------------------------------------*/
_TCHAR *word_skip(_TCHAR *str, _TCHAR *break_str)
{
  while (*str && _tcschr(break_str, *str) == NULL) {
    str++;
  }
  while (*str && _tcschr(break_str, *str) != NULL) {
    str++;
  }
  
  return(str);
} /* word_skip */

 
/*---------------------------------------------------------------------------*/
/*                             reverse_sign                                  */
/*---------------------------------------------------------------------------*/
double reverse_sign(_TCHAR *data)
{
  double value;

  value = -1 * _tcstod(data, NULL);
  _stprintf(data, _T("%-0.0f"), value);

  return(value);
} /* reverse_sign */


/*---------------------------------------------------------------------------*/
/*                             toggle_sign                                   */
/*---------------------------------------------------------------------------*/
/* Like reverse_sign, but handles empty 'data' also.  "" -> "-"              */
/*---------------------------------------------------------------------------*/
void toggle_sign(_TCHAR *data)
{
  _TCHAR *p;

  p=data;
  if (*data==_T('-')) {                                               /* - / + */
    for(;*p;p++) {
      *p=*(p+1);
    }
  }
  else {                                                          /* + / - */
    while(*p) {
      p++;
    }
    *(p+1) = _T('\0');
    for(;p > data;p--) {
      *p = *(p-1);
    }
    *data=_T('-');
  }
} /* toggle_sign */


/*---------------------------------------------------------------------------*/
/*                             search_printable                              */
/*---------------------------------------------------------------------------*/
_TCHAR *search_printable(_TCHAR *p, _TCHAR *circular_src, _TCHAR *not_printable)
{
  for (;*p && NULL != _tcschr(not_printable,*p); p++) {
    ;
  }
  if (*p == _T('\0')) {
    for (p=circular_src; *p && NULL != _tcschr(not_printable, *p); p++) {
      ;
    }
  }
  
  return(p);
} /* search_printable */


/*----------------------------------------------------------------------------*/
/*                             fmt_vat_perc                                   */
/*----------------------------------------------------------------------------*/
/* The vat percentage is expressed with 3 decimals. However if the third      */
/* decimal is equal to 0, it is replaced by a space.                          */
/* Example: input % =  18.000  output =  "18.00 "                             */
/*                      5.125             "5.125"                             */
/*----------------------------------------------------------------------------*/
void fmt_vat_perc(double perc, _TCHAR *result)
{
  unsigned int len;

  _stprintf(result, _T("%3.3f"), perc);
  len = _tcslen(result);
  if ((result[len - 1] == _T('0')) &&
      (result[len - 4] == _T('.'))) {
    result[len - 1] = _T(' ');
    result[len]     = _T('\0');
  }
} /* fmt_vat_perc */


/*---------------------------------------------------------------------------*/
/*                             strcat_maxlen                                 */
/*---------------------------------------------------------------------------*/
/* Appends str2 to str1. str1 will be truncated before appending till str1   */
/* and str2 will fit in maxlen bytes.                                        */
/* Example 1: input  = "Deposit" and " x  1"                                 */
/*            output = "Deposit x  1 "                                       */
/* Example 2: input  = "Deposit of the best Taiwan beer " and " x 1"         */
/*            output = "Deposit of the best Taiwa x  1"                      */
/*---------------------------------------------------------------------------*/
void strcat_maxlen(_TCHAR *str1, _TCHAR *str2, short maxlen)
{
  int len1, len2;

  len1=_tcslen(str1);
  len2=_tcslen(str2);
  if (maxlen < len2) {
    _tcsncpy(str1, str2, maxlen);
    *(str1 + maxlen) = _T('\0');
  }
  else if (maxlen - len1 >= len2) {
    _tcscat(str1, str2);
  }
  else {
    _tcscpy(str1 + maxlen - len2, str2);
  }
} /* strcat_maxlen */

#define NUM_BUF        3
static  char    GlobAnsiBuffer[NUM_BUF][MAX_BO_ENT_SIZE];
static  wchar_t GlobUniBuffer[NUM_BUF][MAX_BO_ENT_SIZE];

/*--------------------------------------------------------------------*/
/*                ConvUnicodeToAnsi()                                 */
/*--------------------------------------------------------------------*/
/* Converts a Unicode string to an ANSI string.                       */
/* It uses a cyclic Ansicode buffer to store and return the result.   */
/* Take care when multiple threads use this function!                 */
/*--------------------------------------------------------------------*/
char *ConvUnicodeToAnsi(wchar_t *uni_str)
{
  static short idx = 0;

  assert( WideCharToMultiByte(CP_ACP, 0, uni_str, -1, NULL,
                   0, NULL, NULL) < MAX_BO_ENT_SIZE-1 );

  if (++idx >= NUM_BUF) {
    idx = 0;
  }
  if (WideCharToMultiByte(CP_ACP, 0, uni_str, -1, GlobAnsiBuffer[idx], 
                                  MAX_BO_ENT_SIZE-1, NULL, NULL)==0) {
    *GlobAnsiBuffer[idx] = '\0';
  }
  return (GlobAnsiBuffer[idx]);
} /* ConvUnicodeToAnsi */

/*--------------------------------------------------------------------*/
/*                ConvAnsiToUnicode()                                 */
/*--------------------------------------------------------------------*/
/* Converts an ANSI string to an Unicode string.                      */
/* It uses a cyclic Unicode buffer to store and return the result.    */
/* Take care when multiple threads use this function!                 */
/*--------------------------------------------------------------------*/
wchar_t *ConvAnsiToUnicode(char *ansi_str)
{
  static short idx = 0;

  assert( MultiByteToWideChar(CP_ACP, 0, ansi_str, -1,
                        NULL, 0) < MAX_BO_ENT_SIZE-1 );

  if (++idx >= NUM_BUF) {
    idx = 0;
  }
  if (MultiByteToWideChar(CP_ACP, 0, ansi_str, -1,
                        GlobUniBuffer[idx], MAX_BO_ENT_SIZE-1)==0) {
    *GlobUniBuffer[idx] = L'\0';
  }
  return (GlobUniBuffer[idx]);
} /* ConvAnsiToUnicode */

/*-------------------------------------------------------------------------*/
/*                            lstrip                                       */
/* Stripping left spaces.                                                  */
/* The input string is not modified, the return value is the address of the*/
/* first non-space character.                                              */
/*-------------------------------------------------------------------------*/
_TCHAR *lstrip(_TCHAR *str)
{
  _TCHAR *lstr = str;

  if (lstr) {
    for (; *lstr==_T(' '); lstr++) ;
  }

  return lstr;
} /* lstrip */
