/*
 *     Module Name       : OPrn_mgr.cpp
 *
 *     Type              : Print manager for OPOS printers
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
 * 29-Nov-2000 Initial Release WinPOS                                  R.N.B.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <stdio.h>
#include "OPrn_mgr.h"
#include "err_mgr.h"
#include "registry.h"
#include "OPOSPrinter.h"


/**************************************************************************
 Declarations and definitions
 **************************************************************************/

 /* OPrinter[] is used to register the initialised printers and stations */
struct PHYS_PRINTER {
  CPosPrinter  *lpPrinter;      /* pointer to the OPOS printer object    */
  short         init_stations;  /* bit mask to store intialised stations */
  _TCHAR       *reg_value;      /* device name in registry for OPOS      */
};

static struct PHYS_PRINTER OPrinter[NR_OPRINTERS] =
{ 
   NULL, 0, REG_PRINTER1
 , NULL, 0, REG_PRINTER2
};

                       /* The lp argument must have type LOGICAL_PRINTER */
#define STATION(lp)    ((short)lp & 0x00FF)    /* yields printer station */
#define DEVICE(lp)     ((short)lp & 0xFF00)    /* yields physical device */
#define OPTR(lp)        OPrinter[DEVICE(lp)]        /* makes life easy...*/


/**************************************************************************
 Error handling utiltities
 **************************************************************************/

#define IS_VALID(lp, s) ((OPTR(lp).init_stations & STATION(lp)) && (STATION(lp) & s))
            /* IS_VALID checks if the printer station is initialised and */
            /* if the function is can be used for this station.          */

    /* RETURN executes fn and returns the extended error if there is one */
#define RETURN(lp,fn)  do {\
                         long status;\
                         status = OPTR(lp).lpPrinter->fn;\
                         if (status == OPOS_E_EXTENDED) {\
                           status = OPTR(lp).lpPrinter->GetResultCodeExtended();\
                         }\
                         return status;\
                       } while (0)



/**************************************************************************
 Manager Functions 
 **************************************************************************/

/*-------------------------------------------------------------------------*/
/*                           oprn_init                                     */
/*-------------------------------------------------------------------------*/
short oprn_init(LOGICAL_PRINTER p)
{
  short     station = STATION(p);
  LPOLESTR  pProp;
  _TCHAR    Val[100];

  assert(DEVICE(p) < NR_OPRINTERS);

  if (!OPTR(p).lpPrinter) {

    ReadEnvironmentValue(TREE_OPOS_DEV_SETTINGS, OPTR(p).reg_value, Val,sizeof(Val)/sizeof(_TCHAR)-1);
    OPTR(p).lpPrinter = NewPrinter(Val);
    if (!OPTR(p).lpPrinter) {
      MessageBox(NULL, _T("Creation of new CPosPrinter element Failed!!!"),
                                                             NULL, MB_OK|MB_SETFOREGROUND);
      return FAIL;
    }
  }

  OPTR(p).init_stations |= station;      /* Set init flag for this station */

                            /* Check if the rqeuested station is available */
  if (station == PTR_S_JOURNAL) {
    pProp = OLESTR("CapJrnPresent");
  }
  if (station == PTR_S_RECEIPT) {
    pProp = OLESTR("CapRecPresent");
  }
  if (station == PTR_S_SLIP) {
    pProp = OLESTR("CapSlpPresent");
  }
//  if (oprn_get_bool_property(p, pProp) == FALSE) {
//    oprn_deinit(p);
//    return FAIL;
//  }
  
  return (SUCCEED);
} /* oprn_init */

/*-------------------------------------------------------------------------*/
/*                         oprn_deinit                                     */
/*-------------------------------------------------------------------------*/
void oprn_deinit(LOGICAL_PRINTER lp)
{
                                       /* Reset init flag for this station */
  if (OPTR(lp).init_stations & STATION(lp)) {
    OPTR(lp).init_stations ^= STATION(lp);
  }
               /* Delete printer object when no more stations are using it */
  if (OPTR(lp).init_stations == 0 && OPTR(lp).lpPrinter) {
    DelPrinter(OPTR(lp).lpPrinter);
    OPTR(lp).lpPrinter = NULL;
  }

  return;
} /* oprn_deinit */

/*-------------------------------------------------------------------------*/
/*                      oprn_is_initialised                                */
/*-------------------------------------------------------------------------*/
short oprn_is_initialised(LOGICAL_PRINTER lp)
{
  return ( OPTR(lp).init_stations & STATION(lp) ? TRUE : FALSE );
} /* oprn_is_initialised */



/**************************************************************************
 OPOS function mappings for C++ -> C
 **************************************************************************/

/*-------------------------------------------------------------------------*/
/*                     oprn_print_immediate                                */
/* NOTE: the OPOS printer only starts printing after it received a complete*/
/* line (ended by '\n'), before that they are buffered.                    */
/*-------------------------------------------------------------------------*/
long oprn_print_immediate(LOGICAL_PRINTER lp, _TCHAR *data)
{
  if (! IS_VALID(lp, PTR_S_SLIP|PTR_S_RECEIPT|PTR_S_JOURNAL)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN(lp, PrintImmediate(STATION(lp), data) );
} /* oprn_print_immediate */

/*-------------------------------------------------------------------------*/
/*                     oprn_print_normal                                   */
/* NOTE: the OPOS printer only starts printing after it received a complete*/
/* line (ended by '\n'), before that they are buffered.                    */
/*-------------------------------------------------------------------------*/
long oprn_print_normal(LOGICAL_PRINTER lp, _TCHAR *data)
{
  if (! IS_VALID(lp, PTR_S_SLIP|PTR_S_RECEIPT|PTR_S_JOURNAL)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN( lp, PrintNormal(STATION(lp), data) );
} /* oprn_print_normal */

/*-------------------------------------------------------------------------*/
/*                     oprn_wait_for_slip                                  */
/*-------------------------------------------------------------------------*/
long oprn_wait_for_slip(LOGICAL_PRINTER lp, long timeout)
{
  if (! IS_VALID(lp, PTR_S_SLIP)) {
    return OPRN_INVALID_PRINTER;
  }
  RETURN( lp, WaitForSlip(timeout) );
} /* oprn_wait_for_slip */

/*-------------------------------------------------------------------------*/
/*                     oprn_close_slip_jaws                                */
/*-------------------------------------------------------------------------*/
long oprn_close_slip_jaws(LOGICAL_PRINTER lp)
{
  if (! IS_VALID(lp, PTR_S_SLIP)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN( lp, CloseSlipJaws() );
} /* oprn_close_slip_jaws */

/*-------------------------------------------------------------------------*/
/*                     oprn_wait_for_slip_removal                          */
/*-------------------------------------------------------------------------*/
long oprn_wait_for_slip_removal(LOGICAL_PRINTER lp, long timeout)
{
  if (! IS_VALID(lp, PTR_S_SLIP)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN( lp, WaitForSlipRemoval(timeout) );
} /* oprn_wait_for_slip_removal */

/*-------------------------------------------------------------------------*/
/*                       oprn_end_removal                                  */
/*-------------------------------------------------------------------------*/
long oprn_end_removal(LOGICAL_PRINTER lp)
{
  if (! IS_VALID(lp, PTR_S_SLIP)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN( lp, EndRemoval() );
} /* oprn_end_removal */

/*-------------------------------------------------------------------------*/
/*                       oprn_rotate_print                                 */
/* NOTE: if rotation is PTR_RP_RIGHT90 or PTR_RP_LEFT90, the print lines   */
/* are buffered until oprn_rotate_print is called with PTR_RP_NORMAL.      */
/*-------------------------------------------------------------------------*/
long oprn_rotate_print(LOGICAL_PRINTER lp, long rotation)
{
  if (! IS_VALID(lp, PTR_S_SLIP|PTR_S_RECEIPT|PTR_S_JOURNAL)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN( lp, RotatePrint(STATION(lp), rotation) );
} /* oprn_rotate_print */

/*-------------------------------------------------------------------------*/
/*                       oprn_clear_output                                 */
/*-------------------------------------------------------------------------*/
long oprn_clear_output(LOGICAL_PRINTER lp)
{
  if (! IS_VALID(lp, PTR_S_SLIP|PTR_S_RECEIPT|PTR_S_JOURNAL)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN( lp, ClearOutput() );
} /* oprn_clear_output */

/*-------------------------------------------------------------------------*/
/*                      oprn_get_state                                     */
/*-------------------------------------------------------------------------*/
long oprn_get_state(LOGICAL_PRINTER lp)
{
  if (! IS_VALID(lp, PTR_S_SLIP|PTR_S_RECEIPT|PTR_S_JOURNAL)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN( lp, GetState() );
} /* oprn_get_state */

/*-------------------------------------------------------------------------*/
/*                     oprn_get_bool_property                              */
/*-------------------------------------------------------------------------*/
long oprn_get_bool_property(LOGICAL_PRINTER lp, LPOLESTR property) 
{
  if (! IS_VALID(lp, PTR_S_SLIP|PTR_S_RECEIPT|PTR_S_JOURNAL)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN( lp, GetBoolSpecProperty(property) );
} /* oprn_get_bool_property */

/*-------------------------------------------------------------------------*/
/*                       oprn_direct_io                                    */
/*-------------------------------------------------------------------------*/
long oprn_direct_io(LOGICAL_PRINTER lp, BSTR pData)
{
  long NCRDIO_PTR_RAW_OUTPUT = 106;
  long  station = STATION(lp);

  if (! IS_VALID(lp, PTR_S_SLIP|PTR_S_RECEIPT|PTR_S_JOURNAL)) {
    return OPRN_INVALID_PRINTER;
  }

  RETURN( lp, DirectIO(NCRDIO_PTR_RAW_OUTPUT, &station, &pData) );
} /* oprn_direct_io */


