/*
 *     Module Name       : Inp_mgr.cpp
 *
 *     Type              : Input manager
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
 * 17-Nov-2000 Added test if KeyLockApprovalFunc is not NULL.          R.N.B.
 *             Bug solve in rs_open_cash_drawer return value.
 * --------------------------------------------------------------------------
 * 11-Dec-2000 Added sleep time to give_time_to_OS                     R.N.B.
 * --------------------------------------------------------------------------
 * 03-May-2001 Added registry key to make it possible to use an OPOS driven 
 *             cashdrawer and the not OPOS driven cashdrawer.            J.H. 
 * --------------------------------------------------------------------------
 * 23-Oct-2001 Bugfix inp_init(): The devices should be checked if they
 *             are attached. If the devices are not attached and they
 *             are used access violations will occurr and the POS will
 *             crash. Also pointers are set to NULL in inp_exit().     J.D.M.
 * --------------------------------------------------------------------------
 * 25-Jan-2002 Added init_keyb() and deinit_keyb() for use in
 *             stnetp85.cpp                                            J.D.M.
 * --------------------------------------------------------------------------
 * 14-Feb-2002 Changed inp_init() and inp_exit().                      J.D.M.
 * --------------------------------------------------------------------------
 * 12-Jul-2002 Added Colombia serial cashdrawer.                       J.D.M.
 * --------------------------------------------------------------------------
 * 14-Oct-2002 Solved bug assigning pointer out of range in function
 *             remove_last_key_from_buffer().                          J.D.M.
 * --------------------------------------------------------------------------
 * 06-Feb-2003 Added handling of state abort request.                  J.D.M.
 * --------------------------------------------------------------------------
 * 13-Feb-2003 Possible hangup in inp_read_ocia() prevented.
 *             Added err_handle in this source so it is seen by
 *             cpp objects as well as c objects.                       J.D.M.
 * --------------------------------------------------------------------------
 * 28-Apr-2003 Scanner/keyboard conflict solved. In case there is data
 *             coming from keyboard and from scanner it could not be
 *             determined, which one was there first. This could result
 *             in a situation in which the pos application does not know
 *             whether the quantity key was pressed first or a scan was
 *             done first. Thus the qty-action could be executed on the
 *             wrong barcode. After the code changes if a conflict
 *             exists an error is invoked, all data is deleted and
 *             input is reset.                                         J.D.M.
 * --------------------------------------------------------------------------
 * 11-Jun-2003 Solved lagging time problem in OPOS and Windows I/O.    J.D.M.
 * --------------------------------------------------------------------------
 * 11-07-2011  Add Cash DrawerType=4								   A.C.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>
#include <time.h>

#include "inp_mgr.h"                   /* Information of this manager      */

#include "err_mgr.h"
#include "scrn_mgr.h"
#include "mem_mgr.h"
#include "Mapkeys.h"
#include "registry.h"
#include "pos_errs.h"
#include "pos_recs.h"
#include "pos_func.h"
#include "State.h"

#include "ConDLL.h"
#include "OPOSScanner.h"
#include "OPOSKeyboard.h"
#include "OPOSKeylock.h"
#include "OPOSCashDrawer.h"
#include "OPOSLineDisplay.h"

static CPosScanner      *lpScanner1     = NULL;
static CPosScanner      *lpScanner2     = NULL;
static CPosKeyboard     *lpKeyboard     = NULL;
static CPosKeylock      *lpKeylock      = NULL;
static CPosLineDisplay	*lpLinedisplay  = NULL;
static CPosCashDrawer   *lpCashdrawer   = NULL;
static HANDLE            hComCashDrawer = INVALID_HANDLE_VALUE;

short keylock_attached = NO;

/* Serial Cash Drawer */
static short  opos_cashdrawer_attached=NO;
static DWORD  last_drawer_open;
static long   msecs_between_open;
static short  DrawerType; /* 1=INDONESIA, 2=GETRONICS, 3=COLOMBIA */
static short  MilliSecsUntilOpenDetection;

static short inp_read_keyb( CPosKeyboard *, short );
static short pickup_keyboard( _TCHAR *, INPUT_CONTROLLER * );
static short inp_read_ocia(CPosScanner *lpScanner, _TCHAR *, int);
static short pickup_ocia1( _TCHAR *, INPUT_CONTROLLER * );
static short pickup_ocia2( _TCHAR *, INPUT_CONTROLLER * );
static short filter_key( VERIFY_KEY *, _TCHAR *, short );
static void  display_data( INPUT_DISPLAY *, _TCHAR * );
static short add_key_to_buffer( _TCHAR *, INPUT_CONTROLLER *, short );
static short remove_last_key_from_buffer(_TCHAR *, INPUT_CONTROLLER *, short);
static short check_keylock( _TCHAR *, INPUT_CONTROLLER * );
static short check_for_state_abort(_TCHAR *, INPUT_CONTROLLER *);
static short return_prev_keycode( void );
static short init_com_cashdrawer(void);
static void  deinit_com_cashdrawer(void);
static short IsScannerData(short);
static short IsKeyboardData(short);

/* 11-07-2011 acm -{*/
/*#define  __DEBUG 1  */
/* 11-07-2011 acm -}*/

/* External hookup function pointer for input manager                     */
void (*fn_inp_idle)(void) = (void (__cdecl*)(void))NULL;

/* For determining type of barcode */
static int preamble;
short determine_preamble(_TCHAR *, long);

     /* Variable that is used when the keylock is emulated by the keyboard */
static short KeyLockPosition = KEYLOCK_LOCK;
short (*KeyLockApprovalFunc)(short,short) = NULL;

extern "C" {
  short err_handle = 0;
}


/**************************************************************************
 print_xz
 Jonathan Peru
 **************************************************************************/

int print_xz()
{
	_TCHAR  prt_xz[4];
	_TCHAR  dummy[100];
	int temp_value;

	ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("XZ_PRINTER"),prt_xz,4);


      temp_value = _ttoi(prt_xz);
      if(!temp_value) {
        _stprintf(dummy, _T("Illegal printer to X and Z!\nProgram will be terminated."));
        MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_SETFOREGROUND);
        return(0);
      }
	return temp_value;
}


/**************************************************************************
 inp_init
 **************************************************************************/

short inp_init(short mask, void (*err_func)(void))
{
  _TCHAR  Val[100];
  short   flush = 1;

  inp_exit(mask); /* Make sure every object is detached to prevent resource leaks */

  /* KEYBOARD */
  if (mask & KEYBOARD_MASK) {
    if(InitKeyMapTable() == FAIL) {
      return (FAIL);
	}
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("USE_OPOS_KEYBOARD"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
    if (_tcsicmp(Val, _T("YES"))==0) {
      ReadEnvironmentValue(TREE_OPOS_DEV_SETTINGS, REG_KEYBOARD,Val,sizeof(Val)/sizeof(_TCHAR)-1);
      lpKeyboard = NewKeyboard(Val);
      if (!lpKeyboard ) {
        if (err_func) {
          err_func();
        }
        MessageBox(NULL, _T("Creation of new CPosKeyboard element Failed!!!"),
                                                             NULL, MB_OK|MB_SETFOREGROUND);
      }
      else if(lpKeyboard->IsDeviceAttached()!=TRUE) {
        DeleteKeyboard(lpKeyboard);
        lpKeyboard = NULL;
        if (err_func) {
          err_func();
        }
        MessageBox(NULL, _T("CPosKeyboard device attach failed!!!"),
                                                             NULL, MB_OK|MB_SETFOREGROUND);
      }
      else {
        lpKeyboard->SetDeviceEnabled(TRUE);
        lpKeyboard->ClearInput();
      }
    }
  }

  /* SCANNER1 */
  if (mask & OCIA1_MASK) {
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("SCANNER1_ATTACHED"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
    if (_tcsicmp(Val, _T("YES"))==0) {
      WriteEnvironmentValue(TREE_OPOS_OLE_SETTINGS, REG_INIT_OLE_SCANNER, _T("1"), flush);
      ReadEnvironmentValue(TREE_OPOS_DEV_SETTINGS, REG_SCANNER1,Val,sizeof(Val)/sizeof(_TCHAR)-1);
      lpScanner1 = NewScanner(Val);
      if (!lpScanner1) {
        if (err_func) {
          err_func();
        }
        MessageBox(NULL, _T("Creation of new CPosScanner1 element Failed!!!"),
                                                             NULL, MB_OK|MB_SETFOREGROUND);
      }
      else if(lpScanner1->IsDeviceAttached()!=TRUE) {
        DeleteScanner(lpScanner1);
        lpScanner1 = NULL;
        if (err_func) {
          err_func();
        }
        MessageBox(NULL, _T("CPosScanner1 device attach failed!!!"),
                                                             NULL, MB_OK|MB_SETFOREGROUND);
      }
      else {
        lpScanner1->SetAutoDisable(TRUE);
        lpScanner1->SetDecodeData(TRUE);
      }
    }
  }

  /* SCANNER2 */
  if (mask & OCIA2_MASK) {
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("SCANNER2_ATTACHED"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
    if (_tcsicmp(Val, _T("YES"))==0) {
      WriteEnvironmentValue(TREE_OPOS_OLE_SETTINGS, REG_INIT_OLE_SCANNER, _T("2"), flush);
      ReadEnvironmentValue(TREE_OPOS_DEV_SETTINGS, REG_SCANNER2,Val,sizeof(Val)/sizeof(_TCHAR)-1);
      lpScanner2 = NewScanner(Val);
      if (!lpScanner2) {
        if (err_func) {
          err_func();
        }
        MessageBox(NULL, _T("Creation of new CPosScanner2 element Failed!!!"),
                                                            NULL, MB_OK|MB_SETFOREGROUND);
      }
      else if(lpScanner2->IsDeviceAttached()!=TRUE) {
        DeleteScanner(lpScanner2);
        lpScanner2 = NULL;
        if (err_func) {
          err_func();
        }
        MessageBox(NULL, _T("CPosScanner2 device attach failed!!!"),
                                                             NULL, MB_OK|MB_SETFOREGROUND);
      }
      else {
       lpScanner2->SetAutoDisable(TRUE);
       lpScanner2->SetDecodeData(TRUE);
      }
    }
  }

  /* KEYLOCK */
  keylock_attached = NO;
  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("KEYLOCK_ATTACHED"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
  if (_tcsicmp(Val, _T("YES"))==0) {
    ReadEnvironmentValue(TREE_OPOS_DEV_SETTINGS, REG_KEYLOCK,Val,sizeof(Val)/sizeof(_TCHAR)-1);
    lpKeylock = NewKeylock(Val);
    if (!lpKeylock) {
      if (err_func) {
        err_func();
      }
      MessageBox(NULL, _T("Creation of new CPosKeyLock element Failed!!!"),
                                                             NULL, MB_OK|MB_SETFOREGROUND);
    }
    else if(lpKeylock->IsDeviceAttached()!=TRUE) {
      DeleteKeylock(lpKeylock);
      lpKeylock = NULL;
      if (err_func) {
        err_func();
      }
      MessageBox(NULL, _T("CPosKeyLock device attach failed!!!"),
                                                           NULL, MB_OK|MB_SETFOREGROUND);
    }
    else {
      keylock_attached = YES;
    }
  }

  /* CASHDRAWER */
  opos_cashdrawer_attached = NO;
  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("CASHDRAWER_ATTACHED"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
  if (_tcsicmp(Val, _T("YES")) == 0) {
    /* Which cash drawer is attached? "To OPOS or not to OPOS?" */
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("USE_OPOS_CASHDRAWER"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
    if (_tcsicmp(Val, _T("YES")) == 0) {
      /* OPOS CASHDRAWER */
      opos_cashdrawer_attached = YES;

      ReadEnvironmentValue(TREE_OPOS_DEV_SETTINGS, REG_CASHDRAWER,Val,sizeof(Val)/sizeof(_TCHAR)-1);
      lpCashdrawer = NewCashDrawer(Val);
      if (!lpCashdrawer) {
        if (err_func) {
          err_func();
		}
        MessageBox(NULL, _T("Creation of new CPosCashdrawer element Failed!!!"),
                                                            NULL, MB_OK|MB_SETFOREGROUND);
	  }
      else if(lpCashdrawer->IsDeviceAttached()!=TRUE) {
        DeleteCashDrawer(lpCashdrawer);
        lpCashdrawer = NULL;
        if (err_func) {
          err_func();
		}
        MessageBox(NULL, _T("CPosCashdrawer device attach failed!!!"),
                                                           NULL, MB_OK|MB_SETFOREGROUND);
	  }
	}
    else {
      /* NOT OPOS CASHDRAWER */
      if(init_com_cashdrawer()==FAIL) {
        if(err_func) {
          err_func();
        }
        MessageBox(NULL, _T("Initialising Serial Cash Drawer Failed!!!"),
                                                              NULL, MB_OK|MB_SETFOREGROUND);
      }
    }
  }

  /* LINEDISPLAY */
  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("LINEDISPLAY_ATTACHED"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
  if (_tcsicmp(Val, _T("YES")) == 0) {
    ReadEnvironmentValue(TREE_OPOS_DEV_SETTINGS, REG_LINEDISPLAY,Val,sizeof(Val)/sizeof(_TCHAR)-1);
    lpLinedisplay = NewLineDisplay(Val);
    if (!lpLinedisplay) {
      if (err_func) {
        err_func();
      }
      MessageBox(NULL, _T("Creation of new CPosLineDisplay element Failed!!!"),
                                                            NULL, MB_OK|MB_SETFOREGROUND);
    }
    else if(lpLinedisplay->IsDeviceAttached()!=TRUE) {
      DeleteLineDisplay(lpLinedisplay);
      lpLinedisplay = NULL;
      if (err_func) {
        err_func();
      }
      MessageBox(NULL, _T("CPosLineDisplay device attach failed!!!"),
                                                           NULL, MB_OK|MB_SETFOREGROUND);
    }
    else {
      cdsp_clear();
    }
  }

  if (lpKeyboard) {
    lpKeyboard->SetDataEventEnabled(TRUE); /* start reading now */
  }

  return (SUCCEED);
} /* inp_init */

/**************************************************************************
 init_keyb
 **************************************************************************/

CPosKeyboard* init_keyb(void (*err_func)(void))
{
  _TCHAR  Val[100];

  deinit_keyb();  /* Make sure the keyboard object is detached to prevent resource leaks */
  if(InitKeyMapTable() == FAIL) {
    return lpKeyboard;
  }
  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("USE_OPOS_KEYBOARD"),Val,sizeof(Val)/sizeof(_TCHAR)-1);
  if (_tcsicmp(Val, _T("YES"))==0) {
    ReadEnvironmentValue(TREE_OPOS_DEV_SETTINGS, REG_KEYBOARD,Val,sizeof(Val)/sizeof(_TCHAR)-1);
    lpKeyboard = NewKeyboard(Val);
    if (!lpKeyboard ) {
      if (err_func) {
        err_func();
      }
      MessageBox(NULL, _T("Creation of new CPosKeyboard element Failed!!!"),
                                                           NULL, MB_OK|MB_SETFOREGROUND);
    }
    else if(lpKeyboard->IsDeviceAttached()!=TRUE) {
      DeleteKeyboard(lpKeyboard);
      lpKeyboard = NULL;
      if (err_func) {
        err_func();
      }
      MessageBox(NULL, _T("CPosKeyboard device attach failed!!!"),
                                                           NULL, MB_OK|MB_SETFOREGROUND);
    }
    else {
      lpKeyboard->SetDeviceEnabled(TRUE);
      lpKeyboard->ClearInput();
    }
  }

  if (lpKeyboard) {
    lpKeyboard->SetDataEventEnabled(TRUE); /* start reading now */
  }

  return lpKeyboard;
} /* init_keyb */

/**************************************************************************
 INP_EXIT()
 **************************************************************************/

void inp_exit(short mask)
{
  if (lpKeyboard && (mask & KEYBOARD_MASK)) {
    DeleteKeyboard(lpKeyboard);
    lpKeyboard = NULL;
  }

  if (lpScanner1) {
    DeleteScanner(lpScanner1);
    lpScanner1 = NULL;
  }

  if (lpScanner2) {
    DeleteScanner(lpScanner2);
    lpScanner2 = NULL;
  }

  if (lpCashdrawer) {
    DeleteCashDrawer(lpCashdrawer);
    lpCashdrawer = NULL;
  }

  deinit_com_cashdrawer();

  if (lpKeylock) {
    DeleteKeylock(lpKeylock);
    lpKeylock = NULL;
  }

  if (lpLinedisplay) {
    DeleteLineDisplay(lpLinedisplay);
    lpLinedisplay = NULL;
  }

  return;
} /* inp_exit */

/**************************************************************************
 deinit_keyb
 **************************************************************************/

void deinit_keyb(void)
{
  if (lpKeyboard) {
    DeleteKeyboard(lpKeyboard);
    lpKeyboard = NULL;
  }
} /* deinit_keyb */

/**************************************************************************
 ARRAY_OF_FUNC
   array of functions to the interface functions, NULL determines
   and of the array.
 **************************************************************************/

static short (*array_of_func[])(_TCHAR *, INPUT_CONTROLLER *) =
{
  pickup_ocia1,
  pickup_ocia2,
  pickup_keyboard,
  check_keylock,
  check_for_state_abort,
  NULL
};  /* array_of_func */

/**************************************************************************
 IsScannerData
 **************************************************************************/

static short IsScannerData(short DataValue)
{
  if(DataValue >= OCIA1_DATA && DataValue <= OCIA2_DATA) {
    return TRUE;
  }

  return FALSE;
} /* IsScannerData */

/**************************************************************************
 IsKeyboardData
 **************************************************************************/

static short IsKeyboardData(short DataValue)
{
  if(DataValue>INP_NO_DATA && DataValue<OCIA1_DATA) {
    return TRUE;
  }

  return FALSE;
} /* IsKeyboardData */


/**************************************************************************
 INP_GET_DATA
 **************************************************************************/

short inp_get_data(INPUT_CONTROLLER *control, _TCHAR *data)
{
  static unsigned short index;        /* Process the array of functions   */
  short  status;
  short  ConflictDetected=FALSE;
  static _TCHAR temp_data[BUFFER_LENGTH]={_T('\0')};

  index = 0;
  status = INP_NO_DATA;

  if (lpScanner1 && (control->device_mask & OCIA1_MASK)) {
    lpScanner1->SetDeviceEnabled(TRUE);
  }
  if (lpScanner2 && (control->device_mask & OCIA2_MASK)) {
    lpScanner2->SetDeviceEnabled(TRUE);
  }

  if (*data) {
    display_data(control->display,data);/* show format string and data   */
  }

  do {
    /* Give OS some time... */
    give_time_to_OS(PM_REMOVE, 0);

    status = array_of_func[index](data,control);/* Call the inp function */

    if(IsScannerData(status)) {
      Sleep(10);
      /* In case there is a lagging time when ALL I/O is coming from OPOS      */
      /* (See comments below!!!).                                              */
      /* Larger sleep times will cause more I/O to be rejected, but reduces or */
      /* eliminates the risk that I/O is interpreted in the wrong order.       */

      /* Check if there is also keyboard data (we can use a destructive read). */
      *temp_data=_T('\0');
      ConflictDetected=IsKeyboardData(pickup_keyboard(temp_data,control));
      /* Also if temp_data is filled we have a conflict */
      if(*temp_data) {
        ConflictDetected=TRUE;
      }
    }
    else if(IsKeyboardData(status)) {

      Sleep(40);
      /* It appears that on a Celeron processor 400 MHz the OPOS I/O always       */
      /* arrives around 13 milliseconds later than direct Windows I/O. This must  */
      /* be compensated to determine if there is data from both I/O channels at   */
      /* the same time. So we need a little sleep here. To be on the safe side we */
      /* will sleep a little bit longer than 13 milliseconds. On faster           */
      /* processors this lagging effect will only become less. If also the        */
      /* keyboard data is retrieved through OPOS and not directly from Windows    */
      /* there should be no lagging effect and the sleep should not be necessary. */
      /* Larger sleep times will cause more I/O to be rejected, but reduces or    */
      /* eliminates the risk that I/O is interpreted in the wrong order.          */
      /* Elimination of this problem can only be done completely if the           */
      /* application is going to be rewritten to an event driven application.     */

      /* Check if there is also scanner1 data (we can use a destructive read). */
      *temp_data=_T('\0');
      ConflictDetected=IsScannerData(pickup_ocia1(temp_data,control));
      /* Also if temp_data is filled we have a conflict */
      if(*temp_data) {
        ConflictDetected=TRUE;
      }

      /* Check if there is also scanner2 data (we can use a destructive read). */
      if(ConflictDetected!=TRUE) {
        *temp_data=_T('\0');
        ConflictDetected=IsScannerData(pickup_ocia2(temp_data,control));
        /* Also if temp_data is filled we have a conflict */
        if(*temp_data) {
          ConflictDetected=TRUE;
        }
      }
    }

    if(ConflictDetected) {
      ConflictDetected = FALSE;
      if(check_for_state_abort(data,control)==INP_STATE_ABORT_REQUEST) {
        /* Leave immediatly! */
        status = INP_STATE_ABORT_REQUEST;
      }
      else {
        /* Invoke an error and remove all data and input. */
        display_data(control->display, "");/* clear the data display line */
        err_invoke(SCAN_KEYB_CONFLICT);
        inp_abort_data();
        *data=0;
        if (lpScanner1 && (control->device_mask & OCIA1_MASK)) {
          lpScanner1->SetDeviceEnabled(TRUE);
        }
        if (lpScanner2 && (control->device_mask & OCIA2_MASK)) {
          lpScanner2->SetDeviceEnabled(TRUE);
        }
        status = INP_NO_DATA;
      }
    }

    index ++;                        /* Prepare for next function        */
    if (!array_of_func[index]) {
      index = 0;
    }
  } while (status == INP_NO_DATA);

  return status;                     /* Return function key pressed      */
} /* inp_get_data */


/**************************************************************************
 PICKUP_KEYBOARD
 **************************************************************************/

static short pickup_keyboard(_TCHAR *data,INPUT_CONTROLLER *control)
{
  short status;
  short mask = control->device_mask;

  if (lpKeylock) {                                 /* keylock is attached */
    if (!(mask & KEYBOARD_MASK)) {        /* No input allowed on keyboard */
      return(INP_NO_DATA);
    }
  }
  else {                                           /* no keylock attached */
    if (!(mask & (KEYLOCK_N_MASK | KEYLOCK_X_MASK | KEYLOCK_S_MASK |
                  KEYLOCK_L_MASK | KEYBOARD_MASK))) {
      return(INP_NO_DATA);
    }
  }

  status = inp_read_keyb(lpKeyboard,mask); /* Get a key if available      */
  if (status!=INP_NO_DATA) {               /* A key has been pressed...   */

    status = filter_key(control->filter,data,status);  /* so filter it!   */

    switch(status) {
      case 0:
        break;
      case DOUBLE_NULL_KEY:
        status = add_key_to_buffer(data,control,_T('0'));
        if (status == INP_NO_DATA) {
          status = add_key_to_buffer(data,control,_T('0'));
        }
        break;
      case BACKSPACE_KEY:
        status = remove_last_key_from_buffer(data,control,status);
        break;
      default:
        status = add_key_to_buffer(data,control,status);
        break;
    }
  }

  return(status);                    /* Returns zero on non-function keys */
} /* pickup keyboard */


/* Prev_keycode contains the key value of the key read by inp_peek_key()  */
/* but must be remembered by the toolset                                  */

static int   prev_keycode = 0;
static long  kbmask;
static short read_kb_set = NO;

/**************************************************************************
 INP_READ_KEYB
 **************************************************************************/

static short inp_read_keyb(CPosKeyboard *kbhandle, short mask)
{
  short VirtKey, OutputKey;
  long  result, result_ext, err_locus;
  _TCHAR err_buf[40];

  if (prev_keycode != 0) {
    return(return_prev_keycode());
  }

  /* Tekens lezen. Als er geen handle is, dan maar ConDLL gebruiken */
  if (!kbhandle) {
    if (IsKeyAvailable()) {
      VirtKey = (short) GetVirtKey();
      OutputKey = MapKeyToFunction(VirtKey);

      if (!lpKeylock) {                             /* keylock emulator */
        if (OutputKey/KEYLOCK_LOCK) {       /* is a keylock-key pressed? */
          if (KeyLockPosition != OutputKey) {
            if(!KeyLockApprovalFunc ||              /* approve keylock-key */
               KeyLockApprovalFunc(OutputKey,mask)) { 
              KeyLockPosition = OutputKey; /* set current keylock position */
            }
          }
          OutputKey = INP_NO_DATA;         /* never return key_lock keys */
        }
      }
      return OutputKey;
    }
  }
  else {          /* use OPOS */
    if (kbhandle->HasError()) {
      kbhandle->GetLastError(&result, &result_ext, &err_locus);
      _stprintf(err_buf, _T("%ld,%ld,%ld"), result, result_ext, err_locus);
      error_extra_msg = err_buf;
      err_invoke(OPOS_KEYB_ERROR);
      kbhandle->ResetError();
    }
    else if (kbhandle->SKeyAvailable()) {
      VirtKey = kbhandle->GetVirtKey16(NULL);
      if (VirtKey) {  /* '0x0' keys are disabled keys */
        OutputKey = MapKeyToFunction(VirtKey);
        if (!lpKeylock  &&                           /* keylock emulator */
            OutputKey/KEYLOCK_LOCK) {       /* is a keylock-key pressed? */
          if (KeyLockPosition != OutputKey) {
            if(!KeyLockApprovalFunc ||              /* approve keylock-key */
               KeyLockApprovalFunc(OutputKey,mask)) { 
              KeyLockPosition = OutputKey; /* set current keylock position */
            }
          }
          OutputKey = INP_NO_DATA;         /* never return key_lock keys */
        }
        return OutputKey;
      }
    }
  }

  if (fn_inp_idle != (void *)NULL) {           /* control to application */
    fn_inp_idle();
  }

  return INP_NO_DATA;
} /* inp_read_keyb() */


/**************************************************************************
 RETURN_PREV_KEYCODE
 **************************************************************************/

static short return_prev_keycode(void)
{
  short keycode;

  if (fn_inp_idle != (void *)NULL) {           /* control to application */
    fn_inp_idle();
  }
  keycode = prev_keycode;                              /* remember value */
  prev_keycode = 0;                                    /* erase contents */

  return( keycode );
} /* return_prev_keycode */


/**************************************************************************
 INP_PICK_UP_KEY
 **************************************************************************/

short inp_pick_up_key(short emul_keyl_mask)  /* application compatibility */
{
  if (prev_keycode != 0) {
    return( return_prev_keycode() );
  }

  return( inp_read_keyb( lpKeyboard, emul_keyl_mask ) );
} /* inp_pick_up_key() */


/**************************************************************************
 INP_PEEK_KEY
 **************************************************************************/

short inp_peek_key(short keyl_emul_mask)
{
  if (prev_keycode == 0) {                            /* assign new value */
    prev_keycode = inp_read_keyb( lpKeyboard, keyl_emul_mask);
  }
  else {
    if (fn_inp_idle != (void *)NULL) {
      fn_inp_idle();
    }
  }

  return prev_keycode;
} /* inp_peek_key() */


/**************************************************************************
 FILTER_KEY
 **************************************************************************/

static short filter_key(VERIFY_KEY *filter,_TCHAR *data,short key)
{
  if (filter) {
     return filter->fn(filter,data,key);
  }

  return key;
} /* filter_key */


/**************************************************************************
 DISPLAY_DATA
 **************************************************************************/

static void display_data(INPUT_DISPLAY *display,_TCHAR *data)
{
  if (display) {                      /* show format string and data      */
    display->fn(display,data);
  }

  return;                             /* Otherwise just pass it through...*/
} /* display_data */


/**************************************************************************
 IS_FUNCTION_KEY
 **************************************************************************/

short is_function_key(short key)
{
  /* All function keys must be mapped to a code larger than NO_MAPPED_KEY.*/
  return (key<NO_MAPPED_KEY ? NO : YES);
} /* is_function_key() */


/**************************************************************************
 ADD_KEY_TO_BUFFER
 **************************************************************************/

static short add_key_to_buffer(_TCHAR *data,
                               INPUT_CONTROLLER *control, short key)
{
  int len = _tcslen(data);             /* length of the string thus far    */
  _TCHAR *add_at = data + len;          /* Where we will add this character */

  if (YES == is_function_key(key)) {
    return key;
  }

  if (len < control->buffer_length -1) {/* If it will fit in the buffer...*/
     *add_at++ = (_TCHAR)key;           /* add data string and... NULL okay */
     *add_at = _T('\0');                /* NULL terminate the string        */
  }

  if (len < control->key_length) {    /* If length is less than key length*/
     display_data(control->display,data);/* show format string and data   */
     return INP_NO_DATA;              /* ... then we are not done         */
  }                                   /* else invoke TOO_MANY_KEYS error  */

  if (err_invoke(INP_TOO_MANY_KEYS) != ERR_NOT_EXIST) {
     *data = _T('\0');                   /* always wipe out data, return NULL*/
     display_data(control->display,data);/* show format string and data   */
     return INP_NO_DATA;
  }

  return INP_TOO_MANY_KEYS;           /* ... or return the error key      */
} /* add_key_to_buffer() */

/**************************************************************************
 REMOVE_LAST_KEY_FROM_BUFFER
 **************************************************************************/
static short remove_last_key_from_buffer(_TCHAR *data,
                               INPUT_CONTROLLER *control, short key)
{
  int len = _tcslen(data);             /* length of the string thus far    */
  _TCHAR *rem_at;

  if (len > 0) {
    rem_at = data + len - 1; /* this character will be removed */
    *rem_at = _T('\0');
    display_data(control->display,data);/* show format string and data     */
  }

  return INP_NO_DATA;
} /* remove_last_key_from_buffer() */

/**************************************************************************
 CHECK_KEYLOCK
 **************************************************************************/

static short check_keylock(_TCHAR *data, INPUT_CONTROLLER *control)
{
  short status;
  short mask = control->device_mask;


  status = rs_keylock_position();

  switch (status) {
    case KEYLOCK_NORMAL:
      if (KEYLOCK_N_MASK & mask) {
        return (KEYLOCK_NORMAL);
      }
      break;
    case KEYLOCK_SUPERVISOR:
      if (KEYLOCK_S_MASK & mask) {
        return (KEYLOCK_SUPERVISOR);
      }
      break;
    case KEYLOCK_EXCEPTION:
      if (KEYLOCK_X_MASK & mask) {
        return (KEYLOCK_EXCEPTION);
      }
      break;
    case KEYLOCK_LOCK:
      if (KEYLOCK_L_MASK & mask) {
        return (KEYLOCK_LOCK);
      }
      break;
    default:
      break;
  }

  if (fn_inp_idle != (void*)NULL) {
    fn_inp_idle();
  }

  return (INP_NO_DATA);
} /* check_keylock() */

/**************************************************************************
 CHECK_FOR_STATE_ABORT
 **************************************************************************/

static short check_for_state_abort(_TCHAR *data, INPUT_CONTROLLER *control)
{
  if(StateAbortIsRequested()==TRUE) {
    inp_abort_data();
    return INP_STATE_ABORT_REQUEST;
  }

  return INP_NO_DATA;
} /* check_for_state_abort */

/**************************************************************************
 RS_KEYLOCK_POSITION
 **************************************************************************/

short rs_keylock_position(void)
{
  if (fn_inp_idle != (void *)NULL) {
    fn_inp_idle();
  }

  if (lpKeylock) {
    long ReturnCode;
  
    ReturnCode = lpKeylock->GetKeyPosition();
    switch (ReturnCode) {
      case LOCK_KP_LOCK:
        return(KEYLOCK_LOCK);
        break;
      case LOCK_KP_NORM:
        return(KEYLOCK_NORMAL);
        break;
      case LOCK_KP_SUPR:
        return(KEYLOCK_SUPERVISOR);
        break;
      case LOCK_KP_EXCE:
        return(KEYLOCK_EXCEPTION);
        break;
      case LOCK_KP_ANY:
      default:
        return(KEYLOCK_NONE);
        break;
    }
  }
  else {                                  /* No OPOS keylock is attached */
    return (KeyLockPosition);
  }
} /* rs_keylock_position */


/**************************************************************************
 RS_WAIT_KEYLOCK_POS
 Use this function to wait for a specific keylock position. When no keylock
 is used, inp_pick_up_key is used to emulate the keylock by the keyboard.
 **************************************************************************/

void rs_wait_keylock_pos(short Pos)
{ 
  short mask = 0;

  if (!lpKeylock) {              /* keylock emulator */
    switch (Pos) {
      case KEYLOCK_NORMAL:
        mask = KEYLOCK_N_MASK;
        break;
      case KEYLOCK_SUPERVISOR:
        mask = KEYLOCK_S_MASK;
        break;
      case KEYLOCK_LOCK:
        mask = KEYLOCK_L_MASK;
        break;
      case KEYLOCK_EXCEPTION:
        mask = KEYLOCK_X_MASK;
        break;
      default:
        break;
    }
  }

  while (rs_keylock_position() != Pos) {
    if (check_for_state_abort(NULL, NULL)==INP_STATE_ABORT_REQUEST) {
      return;
    }
    if (!lpKeylock) {
      inp_pick_up_key(mask);
    }

    if (fn_inp_idle != (void *)NULL) {
      fn_inp_idle();
    }
  }
} /* rs_wait_keylock_pos */


/**************************************************************************
 RS_ERROR_TONE
 **************************************************************************/

void rs_error_tone(void)
{
#define TONE_LENGTH 100

  Beep( 600, TONE_LENGTH);
  Beep( 950, TONE_LENGTH);
  Beep( 600, TONE_LENGTH);

  return;
} /* rs_error_tone() */

/*--------------------------------------------------------------------*/
/*                  RS_POWER_FAILED                                   */
/*--------------------------------------------------------------------*/
short rs_power_failed(void)
{
                /* This feature is currently not available for WinPOS */
  return POWER_OK;

} /* rs_power_failed */

/*--------------------------------------------------------------------*/
/*                  RS_POWER_DOWN                                     */
/*--------------------------------------------------------------------*/
short rs_power_down(void)
{
                /* This feature is currently not available for WinPOS */
  return POWER_OK;
} /* rs_power_down */


/*--------------------------------------------------------------------*/
/*                  RS_WAIT_FOR_ANY_INPUT                             */
/*--------------------------------------------------------------------*/
extern void rs_wait_for_any_input(void) /* non destructive            */
{
  inp_abort_data();
  while (inp_data_avail(0)==NO) { /* endless loop until something happens */
    if (check_for_state_abort(NULL, NULL)==INP_STATE_ABORT_REQUEST) {
      return;
    }
  }
  return;
} /* rs_wait_for_any_input */


/*--------------------------------------------------------------------*/
/*                  RS_WAIT_FOR_KEY_IN_SET                            */
/*--------------------------------------------------------------------*/
extern short rs_wait_for_key_in_set(short *set_pointer)
{
  short *index_pointer;
  short  keycode;

  inp_abort_data();                     /* abort all existing data    */
  while (TRUE) {
    if (check_for_state_abort(NULL, NULL)==INP_STATE_ABORT_REQUEST) {
      return 0;
    }

    if (fn_inp_idle != (void *)NULL) {
      fn_inp_idle();
    }

    keycode = inp_pick_up_key(0);            /* read any key from keyboard */
    if (keycode != 0) {                      /* something read?            */
      index_pointer = set_pointer;
      while (*index_pointer != 0) {          /* Walk through set           */
        if (keycode == *index_pointer) {     /* check key against set      */
          return keycode;
        }
        *index_pointer++;
      }
      rs_error_tone();
    }
  } /* endless loop */

  return 0;
} /* rs_wait_for_key_in_set */


/*--------------------------------------------------------------------*/
/*                      VFY_PREAMBLE                                  */
/*--------------------------------------------------------------------*/
int vfy_preamble(_TCHAR bcd_id)
{
  return ((bcd_id == preamble) ? 1 : 0);
} /* vfy_preamble */


/*--------------------------------------------------------------------*/
/*                      DETERMINE_PREAMBLE                            */
/* For determining the preamble with the LS4004 Symbol Scanner.       */
/* The return value is the starting position of the real barcode.     */
/*--------------------------------------------------------------------*/
short determine_preamble(_TCHAR *barc, long length)
{
  short n;

  for (n=0; *(barc+n) != _T(']') && n < length; n++) {
    ;
  }
  if(n < length) {                                 /* A ']' was found */
    switch (*(barc+n+1)) {
      case _T('A'):
        preamble = _T('B'); /* code 39  */
        break;
      case _T('C'):
        preamble = _T('D'); /* code 128 */
        break;
      case _T('I'):
        preamble = _T('F'); /* ITF      */
        break;
      case _T('E'):
        preamble = _T('A'); /* EAN, UPC */
        break;
      default:
        preamble = _T('X');
        break;
    }
    return(n+3);
  }
  else {
    return(2);
  }
} /* determine_preamble */


/*--------------------------------------------------------------------*/
/*                         OCIA_CONVERT                               */
/*--------------------------------------------------------------------*/
static short ocia_convert(_TCHAR *data)
{
  /* Not necessary anymore at the moment */
  //int l = _tcslen(data);

  //data[l-1] = _T('\0');
  return 1;
} /* ocia_convert */

/*--------------------------------------------------------------------*/
/*                         PICKUP_OCIA1                               */
/*--------------------------------------------------------------------*/
static short pickup_ocia1(_TCHAR *data,INPUT_CONTROLLER *control)
{
  if (!lpScanner1 || !(control->device_mask & OCIA1_MASK)) {
    return (INP_NO_DATA);
  }

  if (SUCCEED != inp_read_ocia(lpScanner1, data,control->buffer_length-1)) {
    return (INP_NO_DATA);
  }

  ocia_convert(data);
  display_data(control->display,data);

  return (OCIA1_DATA);
} /* pickup ocia1 */


/*--------------------------------------------------------------------*/
/*                         PICKUP_OCIA2                               */
/*--------------------------------------------------------------------*/
static short pickup_ocia2(_TCHAR *data,INPUT_CONTROLLER *control)
{
  if (!lpScanner2 || !(control->device_mask & OCIA2_MASK)) {
    return (INP_NO_DATA);
  }

  if (SUCCEED != inp_read_ocia(lpScanner2, data,control->buffer_length-1)) {
    return (INP_NO_DATA);
  }

  ocia_convert(data);
  display_data(control->display,data);

  return (OCIA2_DATA);
} /* pickup ocia2 */


/* Prev_barcode contains the barcode which is read by inp_data_avail()    */
/* You can't push back an already read barcode.                           */

static _TCHAR  prev_barcode[MAX_OCIA_DATA_LENGTH] = _T("\0");
static short read_ocia_set = NO;     /* an read is made to the ocia port */
static long  ocmask;                 /* the ocia mask                    */


/**************************************************************************
 INP_READ_OCIA
 **************************************************************************/

static short inp_read_ocia(CPosScanner *lpScanner, _TCHAR *data, int max_len)
{
  static _TCHAR  ocbuffer[MAX_OCIA_DATA_LENGTH];
//  short start_barc;
  time_t t0, t1;
  static short recurs_cnt = 0;

  if (recurs_cnt == 1) {
    goto fail_inp_read_ocia; /* prevent recursive calls by for instance err_invoke() */
  }
  recurs_cnt = 1;

  memset(ocbuffer,0,max_len);

  if (!lpScanner) {                             /* Not connected or driver */
    goto fail_inp_read_ocia;                    /* not installed.          */
  }

  if (prev_barcode[0] != _T('\0')) {
    if (_tcslen(prev_barcode) <= (size_t)max_len) {
      _tcscpy(data,prev_barcode);
      memset(prev_barcode, 0, MAX_OCIA_DATA_LENGTH);
      goto succeed_inp_read_ocia;
    }
  }

  if (lpScanner->GetDataCount() > 0) {
    lpScanner->SetDataEventEnabled(TRUE);

    /* Wait until Control Object has put Data into Properties */

    time(&t0);
    do {
      give_time_to_OS(PM_REMOVE,0);
      time(&t1);
      if(difftime(t1, t0) > 5.0) { /* Prevent a hangup if there is no data */
        err_invoke(OPOS_SCANNER_READ_TIME_OUT);
        lpScanner->ClearInput();
        inp_init(OCIA1_MASK|OCIA2_MASK, NULL); /* Also calls an inp_exit first! */
        goto fail_inp_read_ocia;
      }
    } while (lpScanner->GetDataEventEnabled()==-1);  /* -1 = TRUE in OPOS terms */

    lpScanner->GetScanDataLabel(ocbuffer);

/*    start_barc = determine_preamble(ocbuffer, max_len+2);                 */
/*    _tcscpy(data,ocbuffer+start_barc);           /* eliminate header bytes */
    _tcscpy(data, ocbuffer);

    goto succeed_inp_read_ocia;
  }

  if (fn_inp_idle != (void *)NULL) {
    fn_inp_idle();
  }

  goto fail_inp_read_ocia;

fail_inp_read_ocia:
  recurs_cnt = 0;
  return FAIL;

succeed_inp_read_ocia:
  recurs_cnt = 0;
  return SUCCEED;
} /* inp_read_ocia() */


/**************************************************************************
 INP_ABORT_DATA
 **************************************************************************/

void inp_abort_data(void)
{
  /* OCIA */
  memset(prev_barcode, 0, MAX_OCIA_DATA_LENGTH);    /* destroy last barcode */

  if (lpScanner1) {
    lpScanner1->ClearInput();
  }
  if (lpScanner2) {
    lpScanner2->ClearInput();
  }

  /* KEYB */
  prev_keycode = 0;                               /* destroy last keycode */
  EmptyKeyBuffer();
  if (lpKeyboard) {
    lpKeyboard->ClearInput();
  }

  return;
} /* inp_abort_data() */


/**************************************************************************
 INP_DATA_AVAIL
 **************************************************************************/

short inp_data_avail(short keyl_emul_mask)
{
  _TCHAR  buffer[MAX_OCIA_DATA_LENGTH];

  if (check_for_state_abort(NULL, NULL)==INP_STATE_ABORT_REQUEST) {
    return YES;
  }

  if (prev_keycode!=0) {              /* something to remember            */
     return YES;
  }
                                      /* Something in the keyboard buffer */
               /* loop to filter keylock-keys if keylock emulator is used */
  while (( lpKeyboard && lpKeyboard->SKeyAvailable()) ||        /* OPOS   */
         (!lpKeyboard && IsKeyAvailable()) ) {                  /* ConDLL */
                         /* A key is pressed. If the keylock emulator is  */
                         /* used, check if it was a keylock-key.          */
    if (!lpKeylock) { 
      if (inp_peek_key(keyl_emul_mask) != INP_NO_DATA) {
        return YES;
      }
    }
    else {
      return YES;
    }
  }

  if (prev_barcode[0]!=_T('\0')) {    /* something to remember            */
    return YES;
  }
  if (inp_read_ocia(lpScanner1, buffer, MAX_OCIA_DATA_LENGTH-1) == SUCCEED) {
    _tcscpy(prev_barcode,buffer);      /* remember scanned value           */
    return YES;
  }
  if (inp_read_ocia(lpScanner2, buffer, MAX_OCIA_DATA_LENGTH-1) == SUCCEED) {
    _tcscpy(prev_barcode,buffer);      /* remember scanned value           */
    return YES;
  }

  return NO;
} /* inp_data_avail() */


/**************************************************************************
 R2L_DISPLAY
 **************************************************************************/

short r2l_display(TEMPLATE_DISPLAY1 *disp, _TCHAR *data)
{
  _TCHAR buffer[MAX_FORMAT_LENGTH+1];
  _TCHAR *dpointer, *fpointer, *index;

  _tcsncpy(buffer,disp->format,MAX_FORMAT_LENGTH);
  buffer[MAX_FORMAT_LENGTH] = _T('\0');

  if(_tcslen(buffer)) {   /* to prevent reading and writing beyond boundaries */
    fpointer = buffer + _tcslen(buffer);
    index = fpointer - 1;
    if(*index!=_T('-')) {
      index=(_TCHAR *)NULL;                  /* No minus in format-string     */
    }
    else {
      *index=_T('~');                         /* Clear minus, set it if in data */
    }

    dpointer = data + _tcslen(data);

    while (fpointer > buffer) { /* work right to left until the format is exhausted */
      fpointer--;
      if (dpointer > data) {
        dpointer--;
        if ((*dpointer==_T('-')) && (index!=(_TCHAR *)NULL)) {
            *index=_T('-');                                        /* minus found */
            if (index != fpointer) {
              fpointer++;
            }
        }
        else {
          if (_tcschr(disp->cover,*fpointer)) {
            *fpointer = *(dpointer);
          }
          else {
            dpointer++;
          }

        }
      }
    }
    if(index != (_TCHAR *)NULL) {
      if(*index==_T('~')) {
        *index=_T('+');
      }
    }
    scrn_select_window(disp->window);
    scrn_string_out(buffer,disp->row,disp->col);
  }
  else { /* just display the format string as is */
    scrn_select_window(disp->window);
    scrn_string_out(buffer,disp->row,disp->col);
  }

  return SUCCEED;
}  /* r2l_display */


/**************************************************************************
 L2R_DISPLAY
 **************************************************************************/

short l2r_display(TEMPLATE_DISPLAY1 *disp, _TCHAR *data)
{
  _TCHAR buffer[MAX_FORMAT_LENGTH+1];
  _TCHAR *fpointer;
  _TCHAR *index;
  short len_buf,
        len_data = _tcslen(data);


  _tcsncpy(buffer,disp->format,MAX_FORMAT_LENGTH);
  buffer[MAX_FORMAT_LENGTH] = _T('\0');
  len_buf = _tcslen(buffer);

  scrn_select_window(disp->window);

  if(len_buf && len_data) { /* to prevent reading and writing beyond boundaries */
    fpointer = buffer;
    index = buffer + len_buf - 1;            /* Determine minus position      */
    if(*index!=_T('-')) {
      index=(_TCHAR *)NULL;                  /* No minus in format-string     */
    }
    else {
      *index=_T('~');                         /* Clear minus, set it if in data */
    }

    if (len_data > len_buf) {
      if (*data==_T('-') && index!=(_TCHAR *)NULL) {
        *index=_T('-');
      }
      data += len_data - len_buf;
    }

    while (*fpointer) {
      if (*data==_T('-') && index!=(_TCHAR *)NULL) {
        *index=_T('-');
        if (index == fpointer) {
          fpointer++;
        }
        data++;
      }
      else {
        if (*data) {
          if (_tcschr(disp->cover,*fpointer)) {
            *fpointer = *data++;
          }
        }
        else {
          *fpointer = _T('\0');
          break;
        }
        fpointer++;
      }
    }
    if(index != (_TCHAR *)NULL) {
      if(*index==_T('~')) {
        *index=_T('+');
      }
    }

    scrn_string_out(disp->format,disp->row,disp->col); /* this is needed to get the */
    scrn_string_out(buffer,disp->row,disp->col);       /* caret on the correct pos  */
  }
  else { /* just display the format string as is */
    scrn_string_out(buffer,disp->row,disp->col);
    scrn_string_out(_T(""),disp->row,disp->col); /* this is needed to get the */
                                                 /* caret on the correct pos  */
  }

  return SUCCEED;
} /* l2r_display() */


/**************************************************************************
 R2L_PASSWORD
 **************************************************************************/

short r2l_password(TEMPLATE_PASSWORD *disp, _TCHAR *data)
{
  TEMPLATE_DISPLAY1 tmp_display;
  _TCHAR buffer[MAX_FORMAT_LENGTH+1];
  short length;

  length = _tcslen(data);
  ch_memset(buffer, disp->password_char, MAX_FORMAT_LENGTH*sizeof(_TCHAR));
  buffer[MAX_FORMAT_LENGTH] = _T('\0');                 /* Make password string    */
  if (length < MAX_FORMAT_LENGTH) {
    buffer[length] = _T('\0');
  }
  memcpy(&tmp_display, disp, sizeof(TEMPLATE_DISPLAY1));
  r2l_display(&tmp_display, buffer);

  return(SUCCEED);
} /* r2l_password */


/**************************************************************************
 format_display
 **************************************************************************/

void format_display(TEMPLATE_DISPLAY1 *dspl, _TCHAR *data)
{
  if (dspl) {
    dspl->fn(dspl,data);
  }
} /* format_display */


/**************************************************************************
 format_display_password
 **************************************************************************/

void format_display_passwrd(TEMPLATE_PASSWORD *dspl, _TCHAR *data)
{
  if (dspl) {
    dspl->fn(dspl,data);
  }
} /* format_display_passwrd */

/**************************************************************************
 init_com_cashdrawer
 **************************************************************************/

short init_com_cashdrawer(void)
{
  DCB    dcb;
  BOOL   fSuccess;
  _TCHAR pcCommPort[20] = _T("\\\\.\\"),
         reg_buf[10],
        *parm = NULL;

  /* Read CASHDRAWER COM port from registry */
  ReadEnvironmentValue(TREE_DRAWER_SETTINGS, _T("Port"), reg_buf, 9);
  parm = _tcsupr(reg_buf);
  if(!*parm) {
    return(FAIL);
  }
  _tcscat(pcCommPort, parm);
  
  msecs_between_open = 5000;      /* default */
  ReadEnvironmentValue(TREE_DRAWER_SETTINGS, _T("SecsBetweenOpen"), reg_buf, 9);
  if(*reg_buf != '\0') {
    msecs_between_open = _tcstol(reg_buf,NULL,10) * 1000;  /* from secs to milli secs */
  }

  DrawerType = 1; /* Default */
  ReadEnvironmentValue(TREE_DRAWER_SETTINGS, _T("DrawerType"), reg_buf, 9);
  if(*reg_buf != '\0') {
    DrawerType = (short)_tcstol(reg_buf,NULL,10);
  }

  MilliSecsUntilOpenDetection = 100; /* Default */
  ReadEnvironmentValue(TREE_DRAWER_SETTINGS, _T("MilliSecsUntilOpenDetection"), reg_buf, 9);
  if(*reg_buf != '\0') {
    MilliSecsUntilOpenDetection = (short)_tcstol(reg_buf,NULL,10);
  }

  /* Initialise CASHDRAWER COM PORT */
  hComCashDrawer = CreateFile(
                     pcCommPort,
                     GENERIC_READ | GENERIC_WRITE,
                     0,    // comm devices must be opened w/exclusive-access
                     NULL, // no security attributes
                     OPEN_EXISTING, // comm devices must use OPEN_EXISTING
                     0,    // not overlapped I/O
                     NULL  // hTemplate must be NULL for comm devices
                    );

  if(hComCashDrawer == INVALID_HANDLE_VALUE) {
    return (FAIL);
  }

  /* We will build on the current configuration and skip setting the size */
  /* of the input and output buffers with SetupComm.                      */
  
  fSuccess = GetCommState(hComCashDrawer, &dcb);
  if(!fSuccess) {
    deinit_com_cashdrawer();
    return (FAIL);
  }

  /* Fill in the DCB: baud=1200 bps, 8 data bits, no parity, and 1 stop bit. */
  dcb.BaudRate    = CBR_1200;           // baud rate
  dcb.ByteSize    = 8;                  // data size, xmit, and rcv
  dcb.Parity      = NOPARITY;           // no parity bit
  dcb.StopBits    = ONESTOPBIT;         // one stop bit
  dcb.fRtsControl = RTS_CONTROL_ENABLE; // RTS Should always be up,
                                        // very important for Cash Drawer!
  fSuccess = SetCommState(hComCashDrawer, &dcb);
  if(!fSuccess) {
    deinit_com_cashdrawer();
    return (FAIL);
  }

  last_drawer_open = GetTickCount();

  return (SUCCEED);
} /* init_com_cashdrawer */


/**************************************************************************
 deinit_com_cashdrawer
 **************************************************************************/

void deinit_com_cashdrawer(void) {
  if(hComCashDrawer!=INVALID_HANDLE_VALUE) {
    CloseHandle(hComCashDrawer);
    hComCashDrawer=INVALID_HANDLE_VALUE;
  }
} /* deinit_com_cashdrawer */

/***************************************************************************/
/*                       RS_CASH_DRAWER_STATUS                             */
/***************************************************************************/

/* 11-07-2011 acm -{*/
 
char CASH_DRAWER_STATE_OPENED_BEMATECH =(char)1  ;
char CASH_DRAWER_STATE_CLOSED_BEMATECH =(char)128;

char cash_drawer_cmd_open []={7,0,0}; // command open of cash drawer
char cash_drawer_cmd_state[]={5,0,0}; // command get state of cash drawer
char cash_drawer_state_curr [100];    // buffer for state of cash drawer

short rs_cash_drawer_status_bematech(short drawer)
{

  char comm[100];
  int  state_curr     = RS_DRAWER_FAILURE;
  unsigned long status= 0;
  BOOL  fSuccess;

  #if __DEBUG
    FILE *f;
    f=fopen("c:\\winpos.log","a");
  #endif


  strcpy(comm,cash_drawer_cmd_state);

  #if __DEBUG
	fprintf(f,"State WriteFile before:status/value:%d/%d\n",status,comm[0]);
  #endif

  fSuccess = WriteFile(hComCashDrawer,comm,  1, &status, NULL);

  #if __DEBUG
	fprintf(f,"State  WriteFile after:status/value:%d/%d\n",status,comm[0]);
  #endif
  if (!fSuccess ) goto __Exit;
  //++Sleep(MilliSecsUntilOpenDetection);

  memset(cash_drawer_state_curr,0, sizeof(cash_drawer_state_curr)-1);
  #if __DEBUG
	fprintf(f,"State ReadFile before: status/value %d/%d\n",status, cash_drawer_state_curr[0]);
  #endif


  fSuccess = ReadFile(hComCashDrawer,  cash_drawer_state_curr, 1, &status, NULL);
  #if __DEBUG
	fprintf(f,"State ReadFile after: status/value %d/%d\n",status, cash_drawer_state_curr[0]);
  #endif
  if (!fSuccess ) goto __Exit;


  if (CASH_DRAWER_STATE_OPENED_BEMATECH==cash_drawer_state_curr[0]) 
      state_curr=DRAWER_OPEN;

  else if (CASH_DRAWER_STATE_CLOSED_BEMATECH==cash_drawer_state_curr[0]) 
      state_curr=DRAWER_CLOSED;
  else 
      state_curr=RS_DRAWER_FAILURE;


__Exit:
  #if __DEBUG
  fprintf(f,"\n");
	fclose(f);
  #endif
  return state_curr;
}
/* 11-07-2011 acm -}*/

short rs_cash_drawer_status(short drawer)
{
  unsigned long status;
  BOOL  fSuccess;

  if (!lpCashdrawer && hComCashDrawer == INVALID_HANDLE_VALUE) {
    return(DRAWER_CLOSED);
  }

  if (lpCashdrawer) {
    if (lpCashdrawer->GetCapStatus()) {
      if (lpCashdrawer->GetDrawerOpened()) {
        return(DRAWER_OPEN);
      }
      else {
        return(DRAWER_CLOSED);
      }
    }
  }

  if (hComCashDrawer != INVALID_HANDLE_VALUE) {
	/* 11-07-2011 acm -{*/
    if (DrawerType == 4){
      return rs_cash_drawer_status_bematech(drawer);
    }
    /* 11-07-2011 acm -}*/

    fSuccess = GetCommModemStatus(hComCashDrawer, &status);
    if (fSuccess) {
      if (  ( DrawerType == 1 /* INDONESIA */ && (status & MS_RING_ON) )
          ||( DrawerType == 2 /* GETRONICS */ && (status == ((DWORD)0x00B0)) )
          ||( DrawerType == 3 /* COLOMBIA  */ && (status == ((DWORD)0x0070)) ) 
		  ) {
        return(DRAWER_OPEN);
      }
      return(DRAWER_CLOSED);
    }
  }

  return(RS_DRAWER_FAILURE);
} /* rs_cash_drawer_status */

/***************************************************************************/
/*                       RS_OPEN_CASH_DRAWER                               */
/***************************************************************************/

short rs_open_cash_drawer(short drawer)
{
  unsigned long status;
  BOOL          fSuccess;
  DWORD         new_tick_count = GetTickCount();
  //  11-07-2011 acm -{
  char comm[100];
  #if __DEBUG
    FILE *f;
  #endif
  //  11-07-2011 acm -}

  if (!lpCashdrawer && hComCashDrawer == INVALID_HANDLE_VALUE) {
    return SUCCEED;
  }

  if (lpCashdrawer) {
    if (lpCashdrawer->GetCapStatus()) {
      if (lpCashdrawer->GetDrawerOpened()) {
        return(SUCCEED);
      }
      else if (lpCashdrawer->OpenDrawer()==0) {  /* 0=OPOS_SUCCESS */
        return(SUCCEED);
      }
    }
  }

  if (hComCashDrawer != INVALID_HANDLE_VALUE) {
    while (new_tick_count < last_drawer_open + msecs_between_open) {
      display_working(YES);
      Sleep(500);
      new_tick_count = GetTickCount();
      if (new_tick_count < last_drawer_open) { /* tick counter resets itself so now and then */
        last_drawer_open = new_tick_count;
      }
    }
    last_drawer_open = new_tick_count;
    display_working(NO);

	  /*  11-07-2011 acm -{  */
	  if (DrawerType == 4){
        #if __DEBUG
            f=fopen("c:\\winpos.log","a");
        #endif

	      fSuccess = GetCommModemStatus(hComCashDrawer, &status);
		    #if __DEBUG
	        fprintf(f,"Open - ModemStatus:%d\n",status);
        #endif


		    strcpy(comm,cash_drawer_cmd_open);
		    #if __DEBUG
	        fprintf(f,"Open - WriteFile Before Status/value:%d/%d \n",status,comm[0]);
		    #endif

		    fSuccess = WriteFile(hComCashDrawer, comm, 1, &status, NULL); 
		    #if __DEBUG
	        fprintf(f,"Open - WriteFile After Status/value:%d/%d \n",status,comm[0]);
		    #endif

		    #if __DEBUG
		      fprintf(f,"\n");
	        fclose(f);
        #endif
	  }else{
	  /*  11-07-2011 acm -}  */
		  fSuccess = WriteFile(hComCashDrawer, "Sesam Open", 11, &status, NULL);
	  } /*  11-07-2011 acm - */
	

    /* Give the drawer some time to open in case it is a little bit slow. */
    Sleep(MilliSecsUntilOpenDetection);
    if (fSuccess) {
      fSuccess = GetCommModemStatus(hComCashDrawer, &status);
    
	    /* 11-07-2011 acm -{*/
      if (DrawerType == 4){
        status=rs_cash_drawer_status_bematech(drawer);
      }
      /* 11-07-2011 acm -}*/
      if (fSuccess &&
          ( ( DrawerType == 1 /* INDONESIA */ && (status & MS_RING_ON) )
          ||( DrawerType == 2 /* GETRONICS */ && (status == ((DWORD)0x00B0/*176*/)) )
          ||( DrawerType == 3 /* COLOMBIA  */ && (status == ((DWORD)0x0070)) ) ) 
		  ||( DrawerType == 4 /* BEMANTECH */ && (status == ((DWORD)DRAWER_OPEN)) )  // 11-07-2011 acm -
		  ) {
        return(SUCCEED);
      }
    }
  }
  return(RS_DRAWER_FAILURE);
} /* rs_open_cash_drawer() */

/**************************************************************************/
/*                     CDSP_SET_CSR                                       */
/**************************************************************************/

short cdsp_set_csr( short row, short column)
{
  if (lpLinedisplay) {
    lpLinedisplay->SetCursorRow(row);
    give_time_to_OS(PM_REMOVE, 0);

    lpLinedisplay->SetCursorColumn(column);
    give_time_to_OS(PM_REMOVE, 0);
  }

  return(SUCCEED);
} /* cdsp_set_csr */


/**************************************************************************/
/*                     CDSP_CLEAR                                         */
/**************************************************************************/

short cdsp_clear(void)
{
  if (lpLinedisplay) {
    lpLinedisplay->ClearText();
    give_time_to_OS(PM_REMOVE, 0);
  }

  return (SUCCEED);
} /* cdsp_clear() */


/**************************************************************************/
/*                     CDSP_CLEAR_LINE                                    */
/**************************************************************************/
extern short cdsp_clear_line( short row )
{
  _TCHAR output[21] = _T("                    ");

  if (lpLinedisplay) {
    cdsp_set_csr(row,0);

    lpLinedisplay->DisplayText(output, DISP_DT_NORMAL);
    give_time_to_OS(PM_REMOVE, 0);
  }

  return(SUCCEED);
}  /* cdsp_clear_line() */


/**************************************************************************/
/*                     CDSP_WRITE_ABS_STRING                              */
/**************************************************************************/

short cdsp_write_abs_string( _TCHAR *buffer )
{
  if (lpLinedisplay) {
    lpLinedisplay->DisplayText(buffer, DISP_DT_NORMAL);
    give_time_to_OS(PM_REMOVE, 0);
  }

  return(SUCCEED);
} /* cdsp_write_abs_string */


/**************************************************************************/
/*                     CDSP_WRITE_STRING                                  */
/**************************************************************************/
short cdsp_write_string( _TCHAR *buffer, short row, short column)
{
  if (lpLinedisplay) {
    if (buffer) {
      if (*buffer != _T('\x1B')) {
        if( (column == CDSP_RIGHT_JUSTIFY) || (column > 20) ) {
          column = 20 - _tcslen(buffer);
        }
        cdsp_set_csr(row,column);
      }
      return(cdsp_write_abs_string(buffer));
    }
    else {
      return(cdsp_write_abs_string(_T("")));
    }
  }

  return(SUCCEED);
} /* cdsp_write_string() */
