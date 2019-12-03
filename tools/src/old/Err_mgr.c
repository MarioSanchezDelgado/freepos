/*
 *     Module Name       : ERR_MGR.C
 *
 *     Type              : Error manager
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
 * 15-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 * 12-Jul-2002 Prevent recursive call of err_invoke (for instance with
 *             NET_CONFIG_ERROR in combination with wait_on_clear).    J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "ConDll.h"
#include "mem_mgr.h"
#include "err_mgr.h"

/* VARIABLES                                                                  */
ERR_NAME        err_current = ERR_NOT_EXIST;
static short    init_errors = NO;
static ERR_TBL *first_element;

/* FUNCTIONS                                                                  */
static ERR_TBL *find_error( ERR_NAME );

/*******************************************************************************
Function    : err_init
Description : Initialize error list
INPUT       : None
OUTPUT      : - short  SUCCEED;
      : - short  FAIL; if aleady initialized;
*******************************************************************************/

short err_init()
{
  if (init_errors == YES) {                          /* already initialized */
    return(FAIL);
  }
  init_errors   = YES;
  first_element = NULL;
  return(SUCCEED);
} /* err_init */


/*******************************************************************************
Function    : err_set
Description : Add or update error in list of errors
INPUT       : - ERR_NAME name    name of error
    ERR_HANDLER *  handler 
OUTPUT      : - short  -1, No initialization done
           MEM_UNAVAILABLE  no memory available to add error
           SUCCEED otherwise
*******************************************************************************/

short err_set (ERR_NAME name, ERR_HANDLER *handler)
{
  ERR_TBL *curr_err;
  ERR_TBL *prev_err;

  if (init_errors == NO) {                               /* not initialized */
    return(-1);
  }
  prev_err = NULL;
  curr_err = first_element; /* look up specified error and the previous one */
  while (curr_err != NULL && curr_err->name != name) {
    prev_err = curr_err;
    curr_err = curr_err->next;
  }

  if (curr_err == NULL) {                /* error not yet in list so add it */
    curr_err = (ERR_TBL *)mem_allocate( sizeof(ERR_TBL) );     /* new error */
    if (curr_err != NULL) {
      curr_err->name = name;
      curr_err->handler = handler;
      curr_err->next = NULL;
      if (prev_err != NULL) {                            /* not first error */
        prev_err->next = curr_err;
      }
      if (first_element == NULL) {                           /* first error */
        first_element = curr_err;
      }
    }
    else { 
      return(MEM_UNAVAILABLE);              /* no memory could be allocated */
    }
  }
  else {
    curr_err->handler = handler;              /* error in list so update it */
  }
  return(SUCCEED);
} /* err_set */


/*******************************************************************************
Function    : err_invoke
Description : Invoke error identified by name
INPUT       : ERR_NAME  name   identification of error
OUTPUT      : - short  -1, No initialization done
           output returned by error handler function
*******************************************************************************/

short err_invoke( ERR_NAME name )
{
  short    status = -1;
  ERR_TBL *curr_err;
  short    show_cur_sav = GetShowCursor();
  static short recurs_cnt = 0;

  if (init_errors == NO) {                               /* not initialized */
    return(status);
  }

  if (recurs_cnt == 1) {
    return(-1);                                  /* prevent recursive calls */
  }
  recurs_cnt = 1;
                                                              /* Hide caret */
  SetShowCursor(FALSE);

  status      = ERR_NOT_EXIST;
  err_current = ERR_NOT_EXIST;

  curr_err = find_error(name);      /* check list if specified error exists */
  if (curr_err != NULL) {
    if (curr_err->handler != NULL) {                         /* Known error */
      err_current = name;      /* execute function specified for this error */
      status      = curr_err->handler->err_fn(curr_err->handler);
    }
    else {                                                 /* Unknown error */
      curr_err = find_error(ERR_NOT_EXIST);
      if (curr_err != NULL) {  /* execute function specified for this error */
        status = curr_err->handler->err_fn(curr_err->handler);
      }
    }
  }
  else {                          /* Unknown error, give default error msg. */
    err_current = name;
    curr_err    = find_error(ERR_NOT_EXIST);
    if (curr_err != NULL && curr_err->handler != NULL) {
      status = curr_err->handler->err_fn(curr_err->handler);
    }
  }
                                                           /* Restore caret */
  SetShowCursor(show_cur_sav);

  recurs_cnt = 0;

  return(status);
}  /* err_invoke */


/*******************************************************************************
Function    : find_error
Description : Find error identified by name
INPUT       : ERR_NAME  name   identification of error
OUTPUT      : ERR_TBL * pointer to error identified by name
*******************************************************************************/

static ERR_TBL *find_error( ERR_NAME name )
{
  ERR_TBL *curr_err;

  curr_err = first_element; 
    /* Go through list, start at first, to find specified error */
  while(curr_err != NULL && curr_err->name != name) {
    curr_err = curr_err->next;
  }
  return(curr_err);
} /* find_error */

/*******************************************************************************
Function    : err_unset_all
Description : Release allocated memory
INPUT       : 
OUTPUT      : 
*******************************************************************************/

void err_unset_all(void)
{
  ERR_TBL *curr_err = first_element,
          *next_err = NULL;

  while(curr_err != NULL) {
    next_err = curr_err->next;
    mem_free(curr_err);
    curr_err = next_err;
  }
  first_element = NULL;
} /* err_unset_all */
