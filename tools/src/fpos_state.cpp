/*
 *     Module Name       : FPOS_State.cpp
 *
 *     Type              : State engine
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

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <stdarg.h> 
#include <time.h> 
#include <vector>
using namespace std;

//#include "Template.h" /* Every source should use this one! */
#include "fpos_defs.h"
#include "fpos_state.h"
//#include "ConDLL.h"

/* FORWARD DEFINITIONS OF FUNCTIONS                                           */
static void*      find_key(short, ELEMENT*);
static void*      find_appr_key(short, short*, APPR_ELEMENT*);
static short      exist_key(short, ELEMENT*);
static void       ClearStateStack();
static STATE_OBJ* GetPreviousStateFromStack();

/* VARIABLES STATIC TO MODULE                                                 */
static STATE_OBJ* state_current = 0;
static short      previous_state_number = 0;
static STATE_OBJ* previous_state_address = 0;
static vector<STATE_OBJ*> StateStack;
static short      StateAbortRequested = FALSE;
static STATE_OBJ* StateAfterAbort = 0;

typedef short f_verify (char*, short);
typedef short f_process(char*, short);
typedef short f_approve(short  , short);


APPR_ELEMENT extra_approval[] = {
  UNKNOWN_KEY,       ST_NULL,                  0,       (void*)0
};

/* Special states (DON'T CHANGE) */
STATE_OBJ PREVIOUS_State =
{                         
  ST_PREVIOUS
 ,0 
 ,0 
 ,0 
 ,0 
 ,0 
 ,0 
 ,0 /* State engine forces return to the previous state before the previous state */
};

STATE_OBJ NOT_IMPLEMENTED_State =
{                         
  ST_NOT_IMPLEMENTED
 ,0
 ,0
 ,0
 ,0
 ,0
 ,0
 ,0 /* State engine forces return to the previous state */
};
void PassFunctionForNotImplementedState(void(*CallBackFunction)()) {NOT_IMPLEMENTED_State.view=CallBackFunction;}
/* Special states (DON'T CHANGE) */


/*******************************************************************************
Function    : state_engine              
Description : This function allows the states in an application to be executed
              in the proper manner.
INPUT       : None 
OUTPUT      : None
*******************************************************************************/
void state_engine(void)
{
  f_verify*  verify_key = 0;
  f_process* process_key = 0;
  f_approve* approve_key = 0;
  short      key = UNKNOWN_KEY;
  short      xarg = 0;
  char     buffer[BUFFER_LENGTH];
  STATE_OBJ* state_temp;

  ClearStateStack();
  while (state_current) {         /* Start loop that executes states */
    *buffer = 0;
    StateAbortRequested=FALSE;

    /* Because we are allowed to shortcut the state engine we always */
    /* have to check everywhere if state_current still exists.       */
    if(state_current && state_current->state_number == ST_NOT_IMPLEMENTED) {
      state_current = &NOT_IMPLEMENTED_State;
    }

    if(StateAbortRequested != TRUE
      && state_current && state_current->view) {
      state_current->view();
    }

    do { /* Loop for successful input */
      if(StateAbortRequested != TRUE
         && state_current && state_current->dflt && state_current->input) {
        state_current->dflt(state_current->input->display, buffer);
      }

      if(StateAbortRequested != TRUE
         && state_current && state_current->input) {
        key = inp_get_data(state_current->input, buffer);
      }

      if(StateAbortRequested != TRUE
         && state_current && exist_key(key, (ELEMENT*)state_current->verify_table) == SUCCEED
         && (approve_key = (f_approve*)find_appr_key(key, &xarg, (APPR_ELEMENT*)extra_approval))) {
        key = approve_key( key,xarg );
      }

      if(StateAbortRequested != TRUE
         && state_current) {
        verify_key = (f_verify*)find_key( key, (ELEMENT*)state_current->verify_table );
      }

      sleep(1); /* Don't take 100% processor time! */
    } while(StateAbortRequested != TRUE
            && state_current
            && ( (!key) || (verify_key && !(key = verify_key(buffer, key))) ) );

    if(state_current && state_current->unview) { /* Always unview, also when aborting state */
      state_current->unview();
    }

    if(StateAbortRequested != TRUE
       && state_current) {
      process_key = (f_process*)find_key(key, (ELEMENT*)state_current->process_table);
      if(process_key) {
          key = process_key( buffer,key );
      }
    }

    if(StateAbortRequested==TRUE) {
      /* In this case the state was set by the function state_abort(). This means that */
      /* the previous_state_number and previous_state_address are already correct.     */
      state_current = StateAfterAbort == &PREVIOUS_State ? GetPreviousStateFromStack() : StateAfterAbort;
    }
    else {
      if(state_current) {
        if(state_current->state_number == ST_NOT_IMPLEMENTED) {
          state_temp = previous_state_address;
          previous_state_number = state_current->state_number;
          previous_state_address = state_current;
          state_current = state_temp;
        }
        else if(state_current->state_number == ST_PREVIOUS) {
          /* In this case the state was set by the function state_set(). This means that */
          /* the previous_state_number and previous_state_address are already correct.   */
          state_current = GetPreviousStateFromStack();
        }
        else {
          state_temp = (struct state_struct*)find_key(key, (ELEMENT*)state_current->control_table);
          previous_state_number = state_current->state_number;
          previous_state_address = state_current;
          state_current = state_temp == &PREVIOUS_State ? GetPreviousStateFromStack() : state_temp;
        }
      }
    }
  }
  ClearStateStack();

  return;           
} /* state_engine */

/*******************************************************************************
Function    : state_number              
Description : This function returns the current_state's state number.
INPUT       : None.
OUTPUT      : short, the state_number for the current_state.
*******************************************************************************/
short state_number(void)
{
  return state_current ? state_current->state_number : 0;
} /* state_number */

/*******************************************************************************
Function    : state_set
Description : Set the current_state to start at in the state engine.
INPUT       : STATE_OBJ *x, a ptr to a state structure.
OUTPUT      : None.
*******************************************************************************/
void state_set(STATE_OBJ* x)
{
  if(x==&NOT_IMPLEMENTED_State) {
    /* You are not allowed to switch to this state */
    return;
  }
  if(state_current) {
    previous_state_number = state_current->state_number;
    previous_state_address = state_current;
  }
  else {
    previous_state_number = 0;
    previous_state_address = 0;
  }
  state_current = x;
} /* state_set */

/*******************************************************************************
Function    : state_abort
Description : Leave the current state immediatly with unview function intact
INPUT       : STATE_OBJ *x, a ptr to a state structure.
OUTPUT      : None.
*******************************************************************************/
void state_abort(STATE_OBJ* x)
{
  if(x==&NOT_IMPLEMENTED_State) {
    /* You are not allowed to switch to this state */
    return;
  }
  if(x==&PREVIOUS_State) {
    /* The previous state and address are not allowed to refer to     */
    /* &PREVIOUS_State. They should refer to the current state, which */
    /* is at the point of being left now.                             */
    if(state_current) {
      previous_state_number = state_current->state_number;
      previous_state_address = state_current;
    }
    else {
      previous_state_number = 0;
      previous_state_address = 0;
    }
  }
  StateAbortRequested=TRUE;
  StateAfterAbort = x;
} /* state_abort */

/*******************************************************************************
Function    : state_get              
Description : Get the current_state
*******************************************************************************/
STATE_OBJ* state_get(void)
{
  return state_current;
} /* state_get */

/*******************************************************************************
Function    : state_previous_number
Description : Return previous state number
INPUT       : None
OUTPUT      : short  previous state number
*******************************************************************************/
short state_previous_number(void)
{
  return previous_state_number;
} /* state_previous_number */

/*******************************************************************************
Function    : state_previous_address
Description : Return previous state address
INPUT       : None
OUTPUT      : short  previous state address
*******************************************************************************/
STATE_OBJ* state_previous_address(void)
{
  return previous_state_address;
} /* state_previous_number */

/*******************************************************************************
Function    : state_current_address
Description : Return current state address
INPUT       : None
OUTPUT      : short current state address
*******************************************************************************/
STATE_OBJ* state_current_address(void)
{
  return state_current;
} /* state_current_address */

/*******************************************************************************
Function    : find_key              
Description : Look up function identified by key.
INPUT       : short, a key value
              element, a ptr to a ELEMENT
OUTPUT      : void*, 0 - if the element is 0.
                     element->pointer - if there is ptr.
*******************************************************************************/
static void* find_key(short key, ELEMENT* element)
{
  if(element==0) {
    return 0;
  }
                /* While there are more keys in the table and match not found */
  while(element->keyvalue && element->keyvalue != key) {
    element++;
  }
  
  return (void*)element->pointer;
} /* find_key */

/*******************************************************************************
Function    : find_appr_key
Description :
INPUT       : short, a key value
              element, a ptr to a ELEMENT
OUTPUT      : void*, 0 - if the element is 0.
                     element->pointer - if there is ptr.
                     short , extra argument, message index
*******************************************************************************/
static void* find_appr_key(short key, short* xarg, APPR_ELEMENT* element)
{
  if(element==0) {
    return 0;
  }

  while(element->pointer) {
    if( element->keyvalue == key &&
       (element->state == state_current->state_number ||
        element->state == ST_NULL) ) {
      *xarg = element->msg_index;
      return (void*)element->pointer;
    }
    else {
      element++;
    }
  }
  
  return (void*)element->pointer;
} /* find_appr_key */

/*******************************************************************************
Function    : exist_key
Description : Does specified key exist ?
INPUT       : short, a key value
              element, a ptr to a ELEMENT
OUTPUT      : SUCCEED if found in table of ELEMENT's
              FAIL otherwise
*******************************************************************************/
static short exist_key(short key, ELEMENT* element)
{
  if(element == 0) {
    return SUCCEED;
  }

  while(element->keyvalue && element->keyvalue != key) {
    element++;
  }

  if(element->keyvalue != key) {
    return FAIL;
  }
  else {
    return SUCCEED;
  }
} /* exist_key */

/*---------------------------------------------------------------------------*/
/* GetPreviousStateFromStack                                                 */
/*---------------------------------------------------------------------------*/
static STATE_OBJ* GetPreviousStateFromStack() {
  STATE_OBJ* PoppedState;

  if(!StateStack.empty()) {
    StateStack.pop_back();
    if(!StateStack.empty()) {
      PoppedState=StateStack.back();
      return PoppedState;
    }
  }

  return 0;
} /* GetPreviousStateFromStack */

/*---------------------------------------------------------------------------*/
/* PopStateFromStack                                                         */
/*---------------------------------------------------------------------------*/
extern STATE_OBJ* PopStateFromStack() {
  STATE_OBJ* PoppedState;

  if(!StateStack.empty()) {
    PoppedState=StateStack.back();
    StateStack.pop_back();
    return PoppedState;
  }

  return 0;
} /* PopStateFromStack */

/*---------------------------------------------------------------------------*/
/* PushStateToStack                                                          */
/*---------------------------------------------------------------------------*/
void PushStateToStack(STATE_OBJ* StateToPush) {
  vector<STATE_OBJ*>::iterator j;
 
  for(j=StateStack.begin(); j!=StateStack.end(); j++) {
    if(*j==state_current) {
      StateStack.erase(j, StateStack.end());
      break;
    }
  }

  StateStack.push_back(StateToPush);
} /* PushStateToStack */

/*---------------------------------------------------------------------------*/
/* ClearStateStack                                                           */
/*---------------------------------------------------------------------------*/
static void ClearStateStack() {
  StateStack.clear();
} /* ClearStateStack */

/*---------------------------------------------------------------------------*/
/* StateAbortIsRequested                                                     */
/*---------------------------------------------------------------------------*/
short StateAbortIsRequested()
{
  return StateAbortRequested;
} /* StateAbortIsRequested */
