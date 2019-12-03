/*
 *     Module Name       : TOT_MGR.C
 *
 *     Type              : Totals manager
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
 * 14-May-2002 Added tot_set_double.                                   R.N.B.
 * --------------------------------------------------------------------------
 */
 
#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>

#include "mem_mgr.h"
#include "tot_mgr.h"

/* VARIABLES                                                                  */
static short   max_total = -1;
static double *total;


/*******************************************************************************
Function    : tot_init
Description : Initialize total 
INPUT       : short nr_totals   maximum number of totals
OUTPUT      : - short   MEM_UNAVAILABLE  no memory could be allocated
      SUCCEED  otherwise
*******************************************************************************/

short tot_init(short nr_totals)
{
  short status, i;

  total = (double *) mem_allocate((unsigned short)(nr_totals*sizeof(double)));
  if (total == NULL) {
    status = MEM_UNAVAILABLE;
  }
  else {
    max_total = nr_totals;
    for (i = 0 ; i < nr_totals ; i++) {
      *(total + i) = 0.0;
    }
    status = SUCCEED;
  }

  return(status);
} /* tot_init */

/*******************************************************************************
Function    : tot_deinit
Description : De-initialize total 
INPUT       : 
OUTPUT      : 
*******************************************************************************/
void tot_deinit(void)
{ 
  mem_free(total);
  max_total = -1;
  return;
} /* tot_deinit */


/*******************************************************************************
Function    : tot_add_double
Description : add double to total
INPUT       : - short offset   : offset into total
              - double amount : total-vaue to be added
OUTPUT      : - short -1 offset exceeds maximum specified
      SUCCEED otherwise
*******************************************************************************/

short tot_add_double(short offset, double amount)
{
  short status;

  status = -1;   
  if (offset < max_total) {
    *(total + offset) += amount; 
    status = SUCCEED;
  }

  return(status);
} /* tot_add_double() */

/*******************************************************************************
Function    : tot_set_double
Description : set double in total
INPUT       : - short offset   : offset into total
              - double amount : total-value to be set
OUTPUT      : - short -1 offset exceeds maximum specified
      SUCCEED otherwise
*******************************************************************************/

short tot_set_double(short offset, double amount)
{
  short status = -1;

  if (offset < max_total) {
    *(total + offset) = amount; 
    status = SUCCEED;
  }

  return(status);
} /* tot_set_double() */


/*******************************************************************************
Function    : tot_ret_string
Description : return string data from specified offset in total 
INPUT       : - short offset  : offset of total 
              - _TCHAR * str_double :  strinbg with double value
OUTPUT      : - _TCHAR *   data string 
*******************************************************************************/

_TCHAR *tot_ret_string(short offset,_TCHAR *str_double)
{
  _stprintf(str_double, _T("%-0.0f"), tot_ret_double(offset));

  return(str_double);
} /* tot_ret_string */


/*******************************************************************************
Function    : tot_ret_double
Description : return double data from total placed at offset
INPUT       : - short offset   : offset in total 
OUTPUT      : - double   double at offset in total 
*******************************************************************************/

double tot_ret_double(short offset)
{
  double tot;

  tot = 0.0;
  if (offset < max_total) {
    tot = *(total + offset);
  }

  return(tot);
} /* tot_ret_double() */


/*******************************************************************************
Function    : tot_reset_double
Description : reset double data to 0 from specified element 
INPUT       : - short offset   : offset of total 
OUTPUT      : - short -1, offset exceeds maximum specified   
      SUCCEED, otherwise
*******************************************************************************/

short tot_reset_double(short offset)
{
  return( tot_set_double(offset, 0.0) );
} /* tot_reset_double() */


/*******************************************************************************
Function    : tot_add_string
Description : Add a string to the total 
INPUT       : - short offset     : offset of total 
    _TCHAR * str_double : string to add to total  (as double)
OUTPUT      : - short -1, offset exceeds maximum specified   
      SUCCEED, otherwise
*******************************************************************************/

short tot_add_string(short offset, _TCHAR *str_double)
{
  return(tot_add_double(offset, _tcstod(str_double, NULL)));
}

