/*
 *     Module Name       : POS_DFLT.C
 *     Type              : Application Default Functions
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

#include <stdio.h>
#include "fpos_input_mgr.h"
#include "fpos_default.h"
#include "fpos_string_tools.h"
#include "fpos_input.h"

//mname
//void name_DFLT(INPUT_DISPLAY *x, char *data)
//{
//  
//  if (x) {
//      if(*last_name) {
//          _tcscpy(data, last_name);
//          x->fn(x, data);
//       }
//       else {
//          x->fn(x, empty);                        /* Call display function NO data */
//        }
//
//  }
//  SetShowCursor(TRUE);
//  return;
//} /* minus_DFLT */
//
//
//void perception_name_DFLT(INPUT_DISPLAY *x, char *data)
//{
//  
//  if (x) {
//      if(*last_perception_name) {
//          _tcscpy(data, last_perception_name);
//          x->fn(x, data);
//       }
//       else {
//          x->fn(x, empty);                        /* Call display function NO data */
//        }
//
//  }
//  SetShowCursor(TRUE);
//  return;
//} /* minus_DFLT */
//
//void passday700_document_DFLT(INPUT_DISPLAY *x, char *data)
//{
//  
//  if (x) {
//      if(*last_document) {
//          _tcscpy(data, last_document);
//          x->fn(x, data);
//       }
//       else {
//          x->fn(x, empty);                        /* Call display function NO data */
//       }
//  }
//  SetShowCursor(TRUE);
//  return;
//} /* minus_DFLT */
//
//void perception_document_DFLT(INPUT_DISPLAY *x, char *data)
//{
//  
//  if (x) {
//      if(*last_perception_document) {
//          _tcscpy(data, last_perception_document);
//          x->fn(x, data);
//       }
//       else {
//          x->fn(x, empty);                        /* Call display function NO data */
//        }
//
//  }
//  SetShowCursor(TRUE);
//  return;
//} /* minus_DFLT */
//
//
//
///*---------------------------------------------------------------------------*/
///*                            minus_DFLT()                                   */
///*---------------------------------------------------------------------------*/
//void minus_DFLT(INPUT_DISPLAY *x, char *data)
//{
//  if (x) {
//    if (invoice_line_mode == SALES) {
//      _tcscpy(data,_T("-"));
//    }
//    x->fn(x,data);
//  }
//  SetShowCursor(TRUE);
//  return;
//} /* minus_DFLT */
//
///*---------------------------------------------------------------------------*/
///*                              no_DFLT                                      */
///*---------------------------------------------------------------------------*/
//void no_DFLT(INPUT_DISPLAY *x, char *data)
//{
//  /*                                                                         */
//  /* Display input-field with no data                                        */
//  /*                                                                         */
//  if (x) {                                  /* If there is a display         */
//    x->fn(x,empty);                         /* Call display function NO data */
//  }
//  SetShowCursor(TRUE);
//  return;
//} /* no_DFLT */

/*---------------------------------------------------------------------------*/
/*                              no_DFLT_2                                    */
/*---------------------------------------------------------------------------*/
void no_DFLT_2(INPUT_DISPLAY *x, char *data)
{
  /*                                                                         */
  /* Display input-field with no data                                        */
  /*                                                                         */
  if (x) {                                  /* If there is a display         */
    x->fn(x,empty);                         /* Call display function NO data */
  }

  /* do not change cursor mode */
  return;
} /* no_DFLT_2 */

/*---------------------------------------------------------------------------*/
/*                              price_DFLT                                   */
/*---------------------------------------------------------------------------*/
//void price_DFLT(INPUT_DISPLAY *x, char *data)
//{
//  /* Only used in StartFloat_ST.                                             */
//  if (x) {                          /* If the display structure is not NULL  */
//    if (assign_price++ == 0) {
//      /* With help of this global counter it is possible to do the           */
//      /* assingment only once. The variable is set to 0 in the function      */
//      /* StartFloat_VW                                                       */
//      ftoa_price(((invoice_mode == SALES)? genvar.def_fl_nm: genvar.def_fl_rt),
//                 TOTAL_BUF_SIZE, data);
//    }
//    x->fn(x, data);
//  }
//  SetShowCursor(TRUE);
//  return;
//} /* price_DFLT */
//
///*---------------------------------------------------------------------------*/
///*                  DFLT_last_condition                                      */
///*---------------------------------------------------------------------------*/
//void DFLT_last_condition(INPUT_DISPLAY *x, char *data)
//{
//  if (x) {                          /* If the display structure is not NULL  */
//    if(*last_condition) {
//      _tcscpy(data, last_condition);
//      x->fn(x, data);
//    }
//    else {
//      x->fn(x, empty);                        /* Call display function NO data */
//    }
//  }
//
//  SetShowCursor(TRUE);
//    
//  return;
//} /* DFLT_last_condition */
//
///*---------------------------------------------------------------------------*/
///*                          art_no_DFLT                                      */
///*---------------------------------------------------------------------------*/
//void art_no_DFLT(INPUT_DISPLAY *x, char *data)
//{
//  /*                                                                         */
//  /* Display input-field with no data                                        */
//  /*                                                                         */
//  if (x) {                                  /* If there is a display         */
//    if (art_no_from_finder > 0) {
//      _stprintf(data, _T("%ld"), art_no_from_finder);
//      x->fn(x,data); 
//      art_no_from_finder = 0;
//    }
//    else {
//      x->fn(x,empty);                       /* Call display function NO data */
//    }
//  }
//
//  SetShowCursor(TRUE);
//  return;
//} /* art_no_DFLT */
