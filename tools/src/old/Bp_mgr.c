/*
 *     Module Name       : BP_MGR.C
 *
 *     Type              : Buffered printing module
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

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "mem_mgr.h"                      
#include "bp_mgr.h"

/* VARIABLES                                                                  */
static BP_PRINT *head = (BP_PRINT *)NULL;

/*******************************************************************************
Function    : bp_init             
Description : Initialize buffered printing list 
INPUT       : - short bp_id   : identification of list
              - short printer : identification  of printertype
OUTPUT      : - short MEM_UNAVAILABLE, could not be enough memory allocated
                SUCCEED, otherwise
*******************************************************************************/
short bp_init(short bp_id, short printer)
{

  BP_PRINT *bp_print, *prv, *p;                       
 
  bp_print = (BP_PRINT *)mem_allocate( sizeof(BP_PRINT) ); 
                                            
  if ( bp_print == (void *)NULL ) {
    return ( MEM_UNAVAILABLE );       /* not enough memory could be allocated */
  } /* if */
 
  bp_print->bp_id = bp_id;
  bp_print->station = printer;
  bp_print->next = (void *)NULL;

  if ( head == NULL ) {
    head = bp_print;
  }
  else {
    p = head;
    while ( p ) {                                         /* find end of list */
      prv = p;
      p = p->next;
    } /* while */
    prv->next = bp_print;                            /* append to end of list */
  } /* else */
  return ( SUCCEED );                             

} /* bp_init */

/*******************************************************************************
Function    : bp_deinit             
Description : De-initialize buffered printing list 
INPUT       : 
OUTPUT      : 
*******************************************************************************/
void bp_deinit(void)
{
  BP_PRINT *curr_bp = head,
           *next_bp = NULL;

  while (curr_bp != NULL) {
    next_bp = curr_bp->next;
    mem_free(curr_bp);
    curr_bp = next_bp;
  }
  head = NULL;

  return;
} /* bp_deinit */

/*******************************************************************************
Function    : bp_now
Description : Execute print function
INPUT       : - short bp_id       : identification of list
              - short function_nr : function array index
              - short elemid      : identification of element in list
OUTPUT      : - short   error indication if function index not ok
              SUCCEED, otherwise
*******************************************************************************/
extern short bp_now(short bp_id,short function_nr,short elemid)
{
  BP_PRINT *bp_print;                       
                               /* bp_number is a constant defined in bp_mgr.h */
  if ( function_nr > bp_number ) {
    return ( -1 );
  }
 
  bp_print = head;
  while ( bp_print && (bp_print->bp_id != bp_id) ) { 
    bp_print = bp_print->next;  
  } /* while */
 
  if ( bp_print != NULL ) {                         /* execute print function */
    bp_alt_fn[function_nr]( elemid, bp_print->station );
  } /* if */
  return ( SUCCEED );

}  /* bp_now */

/*******************************************************************************
Function    : bp_get_printer_nr
*******************************************************************************/
extern short bp_get_printer_nr(short bp_id) {
  BP_PRINT *bp_print;                       

  bp_print = head;
  while ( bp_print && (bp_print->bp_id != bp_id) ) { 
    bp_print = bp_print->next;
  } /* while */
  if(bp_print && bp_print->bp_id == bp_id) {
    return ( bp_print->station );
  }
  return ( -1 );
} /* bp_get_printer_nr */
