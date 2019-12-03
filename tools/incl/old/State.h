/*
 *     Module Name       : STATE.H
 *
 *     Type              : State header file
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
 * 08-Oct-2002 Ported to C++.                                          J.D.M.
 * --------------------------------------------------------------------------
 * 08-Oct-2002 Added functionality for previous state and not
 *             implemented state.                                      J.D.M.
 * --------------------------------------------------------------------------
 * 13-Feb-2003 Added functions state_abort() and
 *             StateAbortIsRequested().                                J.D.M.
 * --------------------------------------------------------------------------
 */


#ifndef __State_h__
#define __State_h__

#include "Inp_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MACROS */
#define UNKNOWN_KEY 0x0000       
#define ST_NULL     0   

extern short previous_state_number;

/* STRUCTURES */
typedef struct                           /* Actual declaration of the type   */
{                                        /* VERIFY_ELEMENT to be a short key */
  short keyvalue;                        /* and a pointer to a function.     */
  short (*verify)(_TCHAR*,short);
} VERIFY_ELEMENT;

typedef struct                           /* Actual declaration of the type   */
{                                        /* PROCESS_ELEMENT to be short key  */
  short keyvalue;                        /* and a pointer to a function.     */
  short (*process)(_TCHAR*,short);
}PROCESS_ELEMENT;

typedef struct                           /* Actual declaration of the type   */
{                                        /* CONTROL_ELEMENT to be short key  */
  short keyvalue;                        /* and a pointer to a STATE_OBJ.    */
  struct state_struct* next;
} CONTROL_ELEMENT;

typedef struct                           /* Actual declaration of the type   */
{                                        /* ELEMENT to be a short key        */
  short keyvalue;                        /* and a pointer to anything. This  */ 
  void* pointer;                         /* is used as generic for the above */
} ELEMENT;                               /* three types of elements.         */

typedef struct state_struct              /* Actual declaration of the type   */
{                                        /* STATE_OBJ to be:                 */
  short state_number;                    /* a (unique) identifying short     */
  void (*view)();                        /* a pointer to a view function     */
  void (*dflt)(INPUT_DISPLAY*, _TCHAR*); /* a pointer to a default function  */
  INPUT_CONTROLLER* input;               /* a pointer to an input structure  */
  VERIFY_ELEMENT* verify_table;          /* a pointer to a verify table      */
  void (*unview)();                      /* a pointer to an unview function  */
  PROCESS_ELEMENT* process_table;        /* a pointer to a process table     */
  CONTROL_ELEMENT* control_table;        /* a pointer to a control table     */
} STATE_OBJ;

typedef struct
{
  short keyvalue;                       /* KEY to approve                    */
  short state;                          /* STATE in which MSG_INDEX is legal */
  short msg_index;                      /* MSG_INDEX to appr_msg_TXT[]       */
  void* pointer;                        /* Approval function                 */
} APPR_ELEMENT;

/* Extra approval functions */
extern APPR_ELEMENT extra_approval[];

/* FUNCTIONS */
extern short state_number(void);           /* Function that returns state number */
                                                             /* of current state */
extern void  state_set(STATE_OBJ*);           /* Sets the current state variable */
extern void  state_abort(STATE_OBJ*);             /* Aborts the state immediatly */
extern STATE_OBJ* state_get(void);                      /* Returns current state */
extern short state_previous_number(void);       /* Returns previous state number */
extern STATE_OBJ* state_previous_address(void);/* Returns previous state address */
extern STATE_OBJ* state_current_address(void);  /* Returns current state address */
extern void  state_engine(void);                /* Function that executes states */
extern void PushStateToStack(STATE_OBJ*);
extern STATE_OBJ* PopStateFromStack();
extern void PassFunctionForNotImplementedState(void(*)());
extern short StateAbortIsRequested();

/* Special states (DON'T CHANGE) */
#define ST_PREVIOUS        19999
#define ST_NOT_IMPLEMENTED 19998
extern  STATE_OBJ PREVIOUS_State;
extern  STATE_OBJ NOT_IMPLEMENTED_State;
/* Special states (DON'T CHANGE) */

#ifdef __cplusplus
}
#endif

#endif 
