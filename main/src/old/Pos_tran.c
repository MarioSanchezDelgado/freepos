/*
 *     Module Name       : POS_TRAN.C
 *
 *     Type              : Convert numeric into alpha-numeric
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
 * 16-Nov-2000 Initial Release WinPOS                                    J.H.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
                                            /* POS (library) include files.  */


#include "err_mgr.h"                        /* Toolsset include files.       */
#include "mem_mgr.h"


#include "pos_txt.h"                        /* Application include files.    */
#include "pos_tran.h"


/*                                                                           */
/* Language depended alphanumeric strings are defined in txt_eng.c           */
/*                                                                           */


/*                                                                           */
/* Translation functions which translate one (or more) digits.               */
/*                                                                           */

#if LANGUAGE == ESPANOL
_TCHAR * dig_1_9(_TCHAR *str, _TCHAR *p)
{
  short position=_tcslen(str)-(p-str);

  /*                                                                         */
  /* Translate one digit '1' till '9' to alphanumeric. Handle exceptions     */
  /* 'UN MIL' instead of 'UNO...' and 'VEINTIUNO' (21) instead of \          */
  /* 'VEINTI UNO'.                                                           */
  /*                                                                         */
  if (*p==_T('0')) {
    return (_TCHAR*)NULL;
  }
  if (*p==_T('1') && (position == 4 || position == 7)) {
    return(exception[0]);
  }
  else if (p>str && *(p-1)==_T('2')) {
    return(one_till_nine[(int)(*p-_T('0'))-1]+1);   /* Skip space.       */
  }
  else {
    return(one_till_nine[(int)(*p-_T('0'))-1]);
  }
}
#else
_TCHAR * dig_1_9(_TCHAR *str, _TCHAR *p)
{
  /*                                                                         */
  /* Translate one digit '0' till '9' to alphanumeric.                       */
  /*                                                                         */
  if (*p==_T('0')) {
    return((_TCHAR*)NULL);
  }
  return(one_till_nine[(int)(*p-_T('0'))-1]);
}
#endif


_TCHAR * dig1_1_9(_TCHAR *str, _TCHAR *p)
{
  /*                                                                         */
  /* Translate one digit '0' till '9' to alphanumeric.                       */
  /* Instead of translating p, translate the digit next to it.               */
  /*                                                                         */
  if (*(p+1)) {
    return(dig_1_9(str, p+1));
  }
  return((_TCHAR*)NULL);
}

#if LANGUAGE == DUTCH
_TCHAR * dig_10_90(_TCHAR *str, _TCHAR *p)
{
  /*                                                                         */
  /* Translate two digits '10', '20' till '90'.                              */
  /*                                                                         */
  if (*p==_T('0')) {
    return((_TCHAR*)NULL);
  }
  return(ten_till_ninety[(int)(*p-_T('0'))-1]);
}
#elif LANGUAGE == ENGLISH || LANGUAGE == GREEK || LANGUAGE == ESPANOL
_TCHAR * dig_1x_9x(_TCHAR *str, _TCHAR *p)
{
  /*                                                                         */
  /* Translate the digits '10' and '2x' till '9x'.                           */
  /*                                                                         */
  if (*p==_T('0')) {
    return((_TCHAR*)NULL);
  }
  return(ten_till_ninety[(int)(*p-_T('0'))-1]);
}
#endif

#if LANGUAGE == ESPANOL
_TCHAR * dig_20(_TCHAR *str, _TCHAR *p)
{
  /*                                                                         */
  /* Translate the digit '20'                                                */
  /*                                                                         */
  if (*p==_T('0')) {
    return((_TCHAR*)NULL);
  }
  return(exception[1]);
}
#endif


_TCHAR * dig1_11_19(_TCHAR *str, _TCHAR *p)
{
  /*                                                                         */
  /* Translate two digits '11' till '19'.                                    */
  /* Instead of using p, use the digit next to it.                           */
  /*                                                                         */
  if (*p==_T('0')) {
    return((_TCHAR *)NULL);
  }
  return(eleven_till_nineteen[(int)(*(p+1)-_T('0'))-1]);
}


#if LANGUAGE == GREEK
_TCHAR * dig_1xx_9xx(_TCHAR *str, _TCHAR *p)
{
  /*                                                                         */
  /* Translate the digits '1xx' till '9xx'.                                  */
  /*                                                                         */
  if (*p==_T('0')) {
    return((_TCHAR*)NULL);
  }
  return(hundred_till_ninehundred[(int)(*p-_T('0'))-1]);
}
#endif

#if LANGUAGE == ESPANOL
_TCHAR * dig_1xx(_TCHAR *str, _TCHAR *p)
{
  /*                                                                         */
  /* Translate '100' and '1xx' where xx > 00                                 */
  /*                                                                         */
  if (*p==_T('0')) {
    return((_TCHAR*)NULL);
  }
  if (*(p+1)==_T('0') && *(p+2)==_T('0')) {
    return(inter_text1[0]);
  }
  else {
    return(inter_text1[1]);
  }
}

_TCHAR * dig_2xx_9xx(_TCHAR *str, _TCHAR *p)
{
  /*                                                                         */
  /* Translate the digits '2xx' till '9xx'.                                  */
  /*                                                                         */
  if (*p==_T('0') || *p==_T('1')) {
    return((_TCHAR*)NULL);
  }
  return(hundred_till_ninehundred[(int)(*p-_T('0'))-1]);
}
#endif


#if LANGUAGE == DUTCH
_TCHAR * post_en(_TCHAR *str, _TCHAR *p)
{
  static _TCHAR t[]=_T("En");
  /*                                                                         */
  /* Combine alphanumeric text within range '21' till '99'.                  */
  /*                                                                         */

  return(t);
}
#endif

#if LANGUAGE == ESPANOL
_TCHAR * post_y(_TCHAR *str, _TCHAR *p)
{
  static _TCHAR t[]=_T(" Y");
  /*                                                                         */
  /* Combine alphanumeric text within range '31' till '99'.                  */
  /*                                                                         */

  return(t);
}
#endif


short inter_value(_TCHAR *str, _TCHAR *p)
{
  short iv=(short)(*p-_T('0'));

  /*                                                                         */
  /* To determine if inter-text must be returned (like Thousand etc)         */
  /* the digits left from the intertext must be > 0.                         */
  /*     xxp 000                                                             */
  /*       ^-------- p+x+x > 0  to return 'thousand'                         */
  /*                                                                         */

  if ((p-1)>=str) {
    iv+=(short)*(p-1)-_T('0');
  }
  if ((p-2)>=str) {
    iv+=(short)*(p-2)-_T('0');
  }
  return(iv);
}


#if LANGUAGE == GREEK
_TCHAR * post_inter1(_TCHAR *str, _TCHAR *p)
{
  short position=_tcslen(str)-(p-str);

  /*                                                                         */
  /* Determine the inter-text like 'hundred', 'thousand' etc.                */
  /*                                                                         */

  if (position%3==0 && *p!=_T('0')) {
    return(inter_text1[0]);
  }
  else if (position==4 && inter_value(str, p) > 0) {
    return(inter_text1[1]);
  }
  else if (position==7 && inter_value(str, p) > 0) {
    return(inter_text1[2]);
  }
  else if (position==10 && inter_value(str, p) > 0) {
    return(inter_text1[3]);
  }

  return((_TCHAR *)NULL);
}
#endif

#if LANGUAGE == ESPANOL
_TCHAR * post_inter(_TCHAR *str, _TCHAR *p)
{
  short position=_tcslen(str)-(p-str);

  /*                                                                         */
  /* Determine the inter-text like 'hundred', 'thousand' etc.                */
  /* Handle exception '5xx'.                                                 */
  /*                                                                         */

  if (position%3==0 && *p==_T('5')) {
    return(inter_text1[2]);
  }
  if (position%3==0 && *p!=_T('0')) {
    return(inter_text[0]);
  }
  else if (position==4 && inter_value(str, p) > 0) {
    return(inter_text[1]);
  }
  else if (position==7 && inter_value(str, p) > 0 && *p==_T('1')) {
    return(inter_text[2]);                          /* Millon        */
  }
  else if (position==7 && inter_value(str, p) > 0 && *p!=_T('1')) {
    return(inter_text[3]);                          /* Millones      */
  }

  return((_TCHAR *)NULL);
}
#else
_TCHAR * post_inter(_TCHAR *str, _TCHAR *p)
{
  short position=_tcslen(str)-(p-str);

  /*                                                                         */
  /* Determine the inter-text like 'hundred', 'thousand' etc.                */
  /*                                                                         */

  if (position%3==0 && *p!=_T('0')) {
    return(inter_text[0]);
  }
  else if (position==4 && inter_value(str, p) > 0) {
    return(inter_text[1]);
  }
  else if (position==7 && inter_value(str, p) > 0) {
    return(inter_text[2]);
  }
  else if (position==10 && inter_value(str, p) > 0) {
    return(inter_text[3]);
  }

  return((_TCHAR *)NULL);
}
#endif


/*                                                                           */
/* Digit condition functions. They give TRUE if other digits (context)       */
/* meet its conditions.                                                      */
/*                                                                           */

#if LANGUAGE == DUTCH
  static short
dno_2_5_8_is0(_TCHAR *str, _TCHAR *p)
{
  return(p==str || (p>str && *(p-1)==_T('0')));
}
#elif LANGUAGE == ENGLISH || LANGUAGE == GREEK || LANGUAGE == ESPANOL
  static short
dno_2_5_8_not1(_TCHAR *str, _TCHAR *p)
{
  return (p==str || (p>str && *(p-1)!=_T('1')));
}
#endif


#if LANGUAGE == DUTCH
  static short
dno_1_4_7_is0(_TCHAR *str, _TCHAR *p)
{
  return (*(p+1)==_T('\0') || *(p+1)==_T('0'));
}
#endif


#if LANGUAGE == GREEK
  static short
dno_1_4_7_is1(_TCHAR *str, _TCHAR *p)
{
  return (*p==_T('1'));
}

  static short
dno_1_4_7_g1_or0(_TCHAR *str, _TCHAR *p)
{
  return(*p!=_T('1'));
}
#endif



  static short
dno_1not0_l20(_TCHAR *str, _TCHAR *p)
{
  if (*p!=_T('1') || *(p+1)==_T('\0') || *(p+1)==_T('0')) {
    return(FALSE);
  }
  return(TRUE);
}

#if LANGUAGE == DUTCH
  static short
dno_1not0_g20(_TCHAR *str, _TCHAR *p)
{
  if (*p==_T('0') || *p==_T('1') || *(p+1)==_T('\0') || *(p+1)==_T('0')) {
    return(FALSE);
  }
  return(TRUE);
}
#endif

#if LANGUAGE == ESPANOL
  static short
dno_1not0_g30(_TCHAR *str, _TCHAR *p)
{
  if (*p==_T('0') || *p==_T('1') || *p==_T('2') || *(p+1)==_T('\0') || *(p+1)==_T('0')) {
    return(FALSE);
  }
  return(TRUE);
}
#endif


#if LANGUAGE == ENGLISH || LANGUAGE == GREEK || LANGUAGE == ESPANOL
  static short
dno_eq10(_TCHAR *str, _TCHAR *p)
{
  if (*p==_T('1') && *(p+1)==_T('0')) {
    return(TRUE);
  }
  return(FALSE);
}
#endif

#if LANGUAGE == ESPANOL
  static short
dno_eq20(_TCHAR *str, _TCHAR *p)
{
  if (*p==_T('2') && *(p+1)==_T('0')) {
    return(TRUE);
  }
  return(FALSE);
}
#endif


#if LANGUAGE == ENGLISH || LANGUAGE == GREEK
  static short
dno_g19(_TCHAR *str, _TCHAR *p)
{
  if (*p==_T('0') || *p==_T('1')) {
    return(FALSE);
  }
  return(TRUE);
}
#endif

#if LANGUAGE == ESPANOL
  static short
dno_g20(_TCHAR *str, _TCHAR *p)
{
  if (*p==_T('0') || *p==_T('1') || (*p==_T('2') && *(p+1)==_T('0'))) {
    return(FALSE);
  }
  return(TRUE);
}

  static short
dno_eq1xx(_TCHAR *str, _TCHAR *p)
{
  if (*p!=_T('1'))
    return(FALSE);
  return(TRUE);
}

  static short
dno_g199(_TCHAR *str, _TCHAR *p)
{
  if (*p==_T('0') || *p==_T('1'))
    return(FALSE);
  return(TRUE);
}
#endif


/*                                                                           */
/* Digit number functions. They give TRUE if the digit to convert is         */
/* within their limits.                                                      */
/*                                                                           */

  static short
dno_1_4_7_etc(_TCHAR *str, _TCHAR *p)
{
  short position=_tcslen(str)-(p-str);
  return((position-1)%3==0);
}

  static short
dno_2_5_8_etc(_TCHAR *str, _TCHAR *p)
{
  short position=_tcslen(str)-(p-str);
  return((position+1)%3==0);
}

  static
short dno_3_6_9_etc(_TCHAR *str, _TCHAR *p)
{
  short position=_tcslen(str)-(p-str);
  return (position%3==0);
}


/*                                                                           */
/* Conversion table.                                                         */
/*                                                                           */
/*  This table is used in the num_to_alpha() function to convert numerics    */
/*  to alpha-numerics. Each element in the table consists of a function      */
/*  which makes sure the correct digit is converted. The second function     */
/*  ensures the correct context before translation is performed. The         */
/*  third and last function performs the actual conversion.                  */
/*                                                                           */

struct parse_element {
    short (*digit_no_fn)(_TCHAR *, _TCHAR *);           /* Compare digit number. */
    short (*digit_condition_fn)(_TCHAR *, _TCHAR *);    /* Digit condition.      */
    _TCHAR *(*trans_fn)(_TCHAR *, _TCHAR *);              /* Translate function.   */
} parse_list[] = {
 /* digit-no       digit-condition   translation function                    */

#if LANGUAGE == DUTCH
    dno_1_4_7_etc, dno_2_5_8_is0,    dig_1_9,       /* '1' .. '9'            */
#elif LANGUAGE == ENGLISH || LANGUAGE == GREEK || LANGUAGE == ESPANOL
    dno_1_4_7_etc, dno_2_5_8_not1,   dig_1_9,       /* '1' .. '9' (handle    */
                                                    /* exception for ESPANOL)*/
#endif

#if LANGUAGE == DUTCH || LANGUAGE == ENGLISH || LANGUAGE == ESPANOL
    dno_1_4_7_etc, (void *)NULL,     post_inter,    /* Thousand, Miljon etc. */
#elif LANGUAGE == GREEK
    dno_1_4_7_etc, dno_1_4_7_is1,    post_inter1,   /* Thousand, Miljon etc. */
    dno_1_4_7_etc, dno_1_4_7_g1_or0, post_inter,    /* Thousands, Miljons    */
#endif

#if LANGUAGE == DUTCH
    dno_2_5_8_etc, dno_1_4_7_is0,    dig_10_90,     /* '10'.. '90'           */
    dno_2_5_8_etc, dno_1not0_l20,    dig1_11_19,    /* '11' .. '19'          */

    dno_2_5_8_etc, dno_1not0_g20,    dig1_1_9,      /* '[2-9]1' .. '[2-9]9'  */
    dno_2_5_8_etc, dno_1not0_g20,    post_en,
    dno_2_5_8_etc, dno_1not0_g20,    dig_10_90,
#elif LANGUAGE == ENGLISH || LANGUAGE == GREEK
    dno_2_5_8_etc, dno_eq10,         dig_1x_9x,     /* '10'                  */
    dno_2_5_8_etc, dno_1not0_l20,    dig1_11_19,    /* '11' .. '19'          */
    dno_2_5_8_etc, dno_g19,          dig_1x_9x,     /* '2x'.. '9x'           */
#elif LANGUAGE == ESPANOL
    dno_2_5_8_etc, dno_eq10,         dig_1x_9x,     /* '10'                  */
    dno_2_5_8_etc, dno_eq20,         dig_20,        /* '20'                  */
    dno_2_5_8_etc, dno_1not0_l20,    dig1_11_19,    /* '11' .. '19'          */
    dno_2_5_8_etc, dno_g20,          dig_1x_9x,     /* '2x'.. '9x'           */
    dno_2_5_8_etc, dno_1not0_g30,    post_y,        /* '3yx'.. '9yx'         */
#endif

#if LANGUAGE == DUTCH || LANGUAGE == ENGLISH
    dno_3_6_9_etc, (void *)NULL,     dig_1_9,       /* '1' .. '9'            */
    dno_3_6_9_etc, (void *)NULL,     post_inter     /* Hundreds              */
#elif LANGUAGE == GREEK
    dno_3_6_9_etc, (void *)NULL,     dig_1xx_9xx,   /* '1xx' .. '9xx'        */
#elif LANGUAGE == ESPANOL
    dno_3_6_9_etc, dno_eq1xx,        dig_1xx,       /* '1xx'                 */
    dno_3_6_9_etc, dno_g199,         dig_2xx_9xx,   /* '2xx' .. '9xx'        */
    dno_3_6_9_etc, dno_g199,         post_inter,    /* Hundreds              */
#endif
};

#define PROC_SIZE sizeof(parse_list)/sizeof(struct parse_element)


static int is_digit_str(_TCHAR *str)
{
  _TCHAR *p;
  for (p=str; isdigit(*p); p++);
  return(!(int)*p);
}


/*                                                                           */
/* num_to_alpha()                                                            */
/*                                                                           */
/*  This conversion function converts numerics to alpha-numerics.            */
/*  It uses the conversion table parse_list.                                 */
/*  The result of the conversion is passed to the calling function. This     */
/*  function must ensure that the result is freed from memory!               */
/*                                                                           */

_TCHAR* num_to_alpha(_TCHAR *str, _TCHAR break_char)
{
  _TCHAR *p, *sub_result;
  static _TCHAR result[255];                       /* do not allocate */
  _TCHAR break_str[2];
  short len, dig_no, dig_cond;
  struct parse_element *parse_elem;

  /*                                                                         */
  /* Numeric to alphanumeric parser driven by the parse_list.                */
  /*                                                                         */
  /*   The digits in the string are numbered right to left while             */
  /*   the parser parses the string from left to right.                      */
  /*                                                                         */
  /*      parsing direction  -------->                                       */
  /*      digit numbering    <--------                                       */
  /*                                                                         */

  break_str[0]=break_char;
  break_str[1]='\0';
  len=_tcslen(str);

  result[0] = _T('\0');                                /* Make it a legal string */
  /* Don't pass Maximum amount and do not parse if not numeric.              */
  if (len <= MAX_AMOUNT_LENGTH &&
      !(len==1 && *str==_T('0')) && is_digit_str(str) ) {

    /* Parse the string from left to right.                                  */
    for (p=str; *p; p++) {
      for (parse_elem=&parse_list[0]; parse_elem-&parse_list[0]<PROC_SIZE; parse_elem++) {
        dig_no=TRUE;
        dig_cond=TRUE;

        /* Find out if this parse-element must be applied to the             */
        /* current digit to translate.                                       */
        if (parse_elem->digit_no_fn != (void *)NULL) {
          dig_no=parse_elem->digit_no_fn(str,p);
        }

        if (dig_no) {       /* Find out if the current digit comply to the   */
                            /* restrictions of this element.                 */
          if (parse_elem->digit_condition_fn != (void *)NULL) {
            dig_cond=parse_elem->digit_condition_fn(str,p);
          }
        }

        if (dig_no && dig_cond) {
          /* Ok to translate, call the translation function.       */
          if (parse_elem->trans_fn != (void *)NULL) {
            sub_result=parse_elem->trans_fn(str,p);
            if (sub_result != (_TCHAR *)NULL) {
              _tcscat(result, break_str);
              _tcscat(result, sub_result);
            }
          }
        }
      }
    }
  }
  return(result);                    /* That's why it is a static char array */
} /* num_to_alpha() */

