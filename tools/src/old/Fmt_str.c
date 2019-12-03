/*
 *     Module Name       : FMT_STR.C
 *
 *     Type              : Formatting string left or right justified
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
 * 16-Feb-2006 Added function fmt_centre_justify_string()              J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "fmt_tool.h"

/******************************************************************************
Function    : fmt_right_justify_string                                
Description : right justify given string in specified buffer
INPUT       : - _TCHAR *data  : buffer in which org is placed to position right
              - short left  : left position 
              - short right : right position 
              - _TCHAR *org   : string to be right justified
OUTPUT      : - _TCHAR * NULL, org does not fit between left and right
                        data, with org in it otherwise
*******************************************************************************/

_TCHAR *fmt_right_justify_string(_TCHAR *data, short left, short right, _TCHAR *org)
{
  short len;

  len = (short)_tcslen(org);
  if (len <= (right - left + 1)) { /*check if org fits within given positions*/
    _tcsncpy(&data[right - len + 1], org,len);
    return(data);
  } 
  else {
    return(NULL);
  }
}

/******************************************************************************
Function    : fmt_left_justify_string                                
Description : left justify given string in specified buffer
INPUT       : - _TCHAR * data  : buffer in which org is placed from position left
              - short left  : left position 
              - short right : right position 
              - _TCHAR * org   : string to be left justified
OUTPUT      : - _TCHAR *   NULL, org does not fit between left and right
                        data, with org in it otherwise
*******************************************************************************/

_TCHAR *fmt_left_justify_string(_TCHAR *data, short left, short right, _TCHAR *org)
{
  short len; 

  len = (short)_tcslen(org);
  if (len <= (right - left + 1)) { /*check if org fits within given positions*/
    _tcsncpy(&data[left], org, len);
    return(data);
  }
  else {
    return(NULL);
  }
}

/*----------------------------------------------------------------------------*/
/* fmt_centre_justify_string                                                  */
/*----------------------------------------------------------------------------*/
_TCHAR* fmt_centre_justify_string(_TCHAR* data, short left, short right, _TCHAR* org)
{
  short  len;
  short  length_to_fill;
  short  i,j;

  data+=left;
  len = (short)_tcslen(org);
  if (len <= (right - left + 1)) { /*check if org fits within given positions*/
    length_to_fill = (right - left + 1) - len;
    for(i=0; i<(length_to_fill)/2; i++) {
//      *(data+i) = _T(' ');
    }
    _tcsncpy(data+i, org, len);
    j=i+len;
    if(length_to_fill%2 == 0) {
      for(i=0; i<(length_to_fill)/2; i++) {
//        *(data+j) = _T(' ');
        j++;
      }
    }
    else {
      for(i=0; i<(length_to_fill+0.5)/2; i++) {
//        *(data+j) = _T(' ');
        j++;
      }
    }
    return(data);
  }
  else {
    return(NULL);
  }
} /* fmt_centre_justify_string */
