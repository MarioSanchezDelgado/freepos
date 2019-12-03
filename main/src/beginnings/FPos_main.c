/*
 *     Module Name       : FPOS_MAIN.C
 *
 *     Type              : Main entry of the FreePOS Application
 *                         
 *
 *     Author/Location   : Mario Sanchez Delgado, Lima-Perú
 *
 *     Copyright Daichin International SA
 *               Lima
 *               Switzerland
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 29-Oct-2019 Initial Release FreePOS                                 MLSD 
 * -------------------------------------------------------------------------- 
 */
/*
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" // Every source should use this one! Pos (library) include files 
#include "intrface.h"
#include "stnetp24.h"
#include "DllMnP24.h"
#include "comm_tls.h"
#include "stri_tls.h"

#include "OLETools.h"
#include "ConDLL.H"

#include "err_mgr.h"
#include "bp_mgr.h"
#include "prn_mgr.h"
#include "inp_mgr.h"
#include "mem_mgr.h"
#include "scrn_mgr.h"
#include "tm_mgr.h"
#include "tot_mgr.h"
#include "scrn_mgr.h"
#include "state.h"
#include "OPrn_mgr.h"
#include "sll_mgr.h"

#include "pos_tm.h"   //  Application include files. 
#include "pos_recs.h"
#include "st_main.h"
#include "pos_tot.h"
#include "pos_st.h"
#include "pos_inp.h"
#include "write.h"
#include "pos_bp1.h"
#include "pos_errs.h"
#include "pos_com.h"
#include "pos_func.h"
#include "pos_txt.h"
#include "WPos_mn.h"
#include "pos_scrl.h"
#include "pos_chq.h"
#include "pos_edoc.h"

#include "registry.h"
#include "Version_mgr.h"
#include <time.h>  // 27-April-2012 acm - fix 
*/

static char ident[] = "@(#) Copyright Makro International AG,\n Aspermonstrasse 24,\n 7006 CHUR,\n Switzerland\n";

#define  DONE       1
#define  NOT_DONE   0

short prn_on[NUMBER_OF_PRINTERS];
short print_to_file;
short selected_printer;
short selected_invoice_printer;

short printers_attached;  // Number of printers attached to the POS          
char prn_fname[80];

short subt_display;                     // Customer display mode.            
short cash_pincd_mistakes;              // Number of times the cashier pincd.
short invoice_mode;                     // Invoice: SALES or RETURN mode.    
short invoice_line_mode;                // Invoice-line: SALES or RETURN.    
short next_invoice_mode;                // Invoice mode next customer.       
short train_mode = CASH_NORMAL;         // Training or normal cashier.       
short closed_till = NO;                 // Till is closed or not.            
short status_of_pos = START_OF_DAY;     // Status of the Pos: START_OF_DAY,  
                                        // START_OF_SHIFT, SHIFT_OPEN or     
                                        // CASHIER_ON_BREAK.                 
short assign_price = 0;                 // Used in st_cash, StartFloat state.
short assign_minus = 0;                 // Used in st_disc, DiscAmount state.
short send_logoff_cashier;              // If YES, send a logoff to BO.      
double cheque_amount;                   // Cheque amount to be printed       
short copy_invoice = NO;                // If YES, print invoice twice.      
short copy_invoice_active = NO;         // If YES, special text is being pr. 
short bot_copy_invoice_active = NO;     // If YES, special text is being pr. 
short voided_invoice = NO;              // If YES, special text is being pr. 
short reverse_invoice_active = NO;      // If YES , si esta en modo devolu   
short err_init_environment = FALSE;     // init_environment err (TRUE/FALSE) 
char  sinal[4] = _T("   ");           // 3 characters calculator           

TM_ITEM_GROUP c_item;                   // Current item                      
TM_SHFT_GROUP c_shft;                   // Current shift                     
SHIFT_TDM_DEF c_day;                    // Current day                       

SYSTEM_DEF       pos_system;            // Active system settings.           
GENVAR_DEF       genvar;                // General var's.                    
POS_CUST_DEF     cust;                  // Customer data.                    
SHIFT_TDM_DEF    tdm_shift;             // Shift-on/off data.                
SHIFT_LIFT_DEF   till_lift;             // Till lift/refill data.            
CASH_DEF         cash;                  // Cashier data.                     
POS_INVOICE_DEF  pos_invoice;           // Current invoice information.      
TILL_DEF         till;

long  nbr_log_lines  = 0;           // Number of lines logged to BO(norm+dep)
long  nbr_inv_lines  = 0;           // Number of items (norm/dep/disc).      
long  nbr_void_lines = 0;           // Number of voided invoice lines.       
short wallet_seq_no  = 0;           // Wallet number sequence number/shift.  

TM_INDX last_item    = TM_ROOT;     // Last entered item (if available).     
TM_INDX corr_item    = TM_ROOT;     // Item to correct (voided.              
TM_INDX display_item = TM_ROOT;     // Item currently displayed.             

HANDLE MyThreadHandle = 0;
static DWORD MyThreadId = 0;
BOOL StopDots = FALSE;
BOOL paused  = TRUE;

WINDOW screen_windows[ NUM_WINDOWS ];   /* used by scrn_mgr.c */

extern short KeylockEmulApprove(short,short); /* see pos_appr.c */

static void chk_status (char *, short, short, short, short);
static void PrintMsg   (char *);
static void init_scrn  (void);
static void PausePrintDots(void);
static void StartPrintDots(void);
unsigned int __stdcall PrintDots(void *);

FILE * pLogFile = NULL;


//12-Jul-2011 acm - {
//==================================================================================================
char *replace_str(_TCHAR *str, _TCHAR *orig, _TCHAR *rep)
{
  static char buffer[4096];
  char  *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

char * window_gettext(_TCHAR * buffer, HWND hwnd)
{
	int length;
	length=GetWindowTextLength(hwnd)+1;
	GetWindowText(hwnd,buffer,length);
  return buffer;   /* 27-April-2012 acm - fix */
}

/*
Al parecer lo retaron para cambiarle el nombre de la ventana
void SetTitleApp(char * app_new_title, char * app_old_title) //"Makro Perú"
{
	HWND hwnd;
	double  t1;
	double t2;

	t1 = time (NULL);
	t2 = time (NULL);
	ClearScreen();

	while(1){
	  if (t2 - t1 > 5) break;// si es mayor a 4 segundos salir del sistema
	  hwnd=FindWindow("WinClass EJ",app_old_title);
	  if (hwnd){
		SetWindowText(hwnd,app_new_title);
		break;
	  }
	  Sleep(50);
    }
}*/
/*---------------------------------------------------------------------------*/
/*                           _tWinMain                                       */
/*---------------------------------------------------------------------------*/
int main(int argc, char **argv) 
{
  //char   Val[15];
  short  result;

//  SetTitleApp("Point Of Sale - Makro Perú"," Point Of Sale - Makro Colombia");// //28-Jun-2011 acm - ad
  
  /* 
    Esta funcion hace uso del version_mgr para obtener número de version
    y fecha de compilación de los ejecutables escribirlas en
    el regedit con el RegWinPos, 
    para supuestamente luego enviarlas al backoffice una vez levantada la
    red. Sería util hacer lo mismo pero de otra forma ya que serviria para
    comprobar si un POS esta bien actualizada o no 
  */
  check_version(NULL);
  
  /*
    Hasta no saber la forma de comunicación con la entidad tributaria
    se comentará esta función
  */
  /* Read Electronic Document configuration data from file */
  //de_read_conf_file();
  

  /*
    Se comenta hasta encontrar utilidad en el nuevo sistema, esta función
    se encuentra en la libreria oposdll
  */
  //SetRegistryEcho(FALSE);
  

  /*
    La función ReadEnvironmentValue (oposdll) 
    ya tiene como argumentos leer desde
    el registro con la rama del programa que ahora no se tendrá.
    Esta función se dejará de lado para leer de la base de datos local.
    Se verá si las funcionalidades son necesarias para la nueva
    inplementación
  */
  
  /* 
     Se retirará lo siguiente hasta ver si resulta necesario 
     mostrar el cursor
  */
/* ReadEnvironmentValue(TREE_SCRN_SETTINGS, _T("ShowCursor"), Val, sizeof(Val)/sizeof(char)-1);
  SetRegistryEcho(TRUE);
  if (_tcsicmp(Val, _T("vertical")) != 0) {
    ShiftPosCaret(-1);  // shift caret -1 position with respect to cursor
  }
*/

  init_invoice_user(0,0);
  //---------- A implementar -------------
  // data_load();

  //reg_turkey_load(); /* 12-Ago-2011 acm - read "article turkey" y "vale virtual turkey" */
  // reg_aditional_regedit_load(); /* 27-Jan-2012 acm -  */

  //---------- A implementar -------------
  //WriteLn( "Initialising:" );

  /* Posiblemente innecesario */
  /* if (FAIL == OLE_Init()) {
    MessageBox(NULL,(LPTSTR) _T("Fail to init OLE"), (LPTSTR) _T("Error"), MB_OK|MB_SETFOREGROUND);
    return(1);
  }
  */

  err_init();

  //---------- A implementar -------------
  //init_scrn();

  //Tal vez no sea necesario colocar un icono
  //SetWindowIcon(GetModuleHandle(NULL), _T("WINPOS"));

  result = pos_main();

  de_init(NULL);

  return(result);
} /* _tWinMain */

/*---------------------------------------------------------------------------*/
/*                           pos_main                                        */
/*---------------------------------------------------------------------------*/
short pos_main(void)
{
  char  Val[15];
  char  dummy[20];
  short   i, count;
  ASYNC_CALLBACK ac;
  short   SaveRegistryEcho;
  short   NoPrinterNagging;

  /*                                                                         */
  /* Main Entry Point of the POS Application:                                */
  /*                                                                         */
  /*   Initialize Devices, Data Managers and start State Engine.             */
  /*   If the Engine Stops, perform graceful exit.                           */
  /*                                                                         */

  MyThreadHandle = CreateThread(
        NULL,                       /* pointer to thread security attributes */
        0,                          /* initial thread stack size, in bytes   */
        PrintDots,                  /* pointer to thread function            */
        NULL,                       /* argument for new thread               */
        CREATE_SUSPENDED,           /* creation flags                        */
        &MyThreadId                 /* pointer to returned thread identifier */
      );

	/* ReadPrintEnvironment() must be done before calling functions of the   */
	/* customer display. ReadPrintEnvironment() also initialises the         */
	/* attributes for BOLD, UNDERLINE, NLQ and ITALIC, which must be filtered*/
	/* from the customer display.                                            */
  /* ------------ A implementar funcionalidad de impresora ----------------
  if (ReadPrintEnvironment() == FAIL) {
    return(FAIL);
  }*/

  /*  prn_on is[printer] used to overrule the error messages when no printer is  */
  /*  connected to the machine. prn_on[printer] can be set to NO (that is do not */
  /*  print) with the registry key PRINTERS_ATTACHED.                            */

  /* Check if the printers are attached */
  for(i=DEV_PRINTER1; i < NUMBER_OF_PRINTERS; i++) {
    _stprintf(dummy, _T("PRINTER%d_ATTACHED"), i+1);
    ReadEnvironmentValue(TREE_TILL_DEPENDENT, dummy, Val, sizeof(Val)/sizeof(char)-1);
    prn_on[i] = YES;
    if (!_tcsicmp(Val, _T("NO"))) {
      prn_on[i] = NO;
    }
  }

  /*  print_to_file is used to reroute the printer output to a file.        */
  print_to_file = NO;
  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("PRINT_TO_FILE"), Val, sizeof(Val)/sizeof(char)-1);
  if (!_tcsicmp(Val, _T("YES"))) {
    clear_all_used_print_files();
    print_to_file = YES;
  }
       /* register entity types that are handled asynchronously by the application */
                                                                  /* Crazy article */
  ac.type = CART_TYPE;
  ac.callback_fn = &get_cart_callback; 
  register_async_callback(&ac);
                                                                      /* Powerdown */
  ac.type = PWDN_TYPE;
  ac.callback_fn = &activate_pwdn_callback; 
  register_async_callback(&ac);

  ac.type = STAT_TYPE;
  ac.callback_fn = &return_pos_status; 
  register_async_callback(&ac);

  ac.type = MMML_TYPE;
  ac.callback_fn = &update_multisam_definitions_callback; 
  register_async_callback(&ac);

  ac.type = TILL_TYPE;
  ac.callback_fn = &update_till_callback; 
  register_async_callback(&ac);

  WriteStr( _T("- Network..") );
  StartPrintDots();
  chk_status(_T("StartNetwork()"), (short)StartNetwork(PausePrintDots), TRUE, DONE, FALSE);

  WriteStr( _T("- Environment..") );
  StartPrintDots();
  chk_status(_T("init_environment_records()"),
             init_environment_records(NO_WINDOW,PausePrintDots), SUCCEED, DONE, TRUE);

  WriteStr( _T("- Transaction manager..") );
  StartPrintDots();

  chk_status(_T("tm_define_struct()"), tm_define_struct(TM_ITEM_NAME, TM_ITEM_MAXI, sizeof(TM_ITEM_GROUP))
             , SUCCEED, NOT_DONE, FALSE);
  chk_status(_T("tm_define_struct()"), tm_define_struct(TM_SHFT_NAME, TM_SHFT_MAXI, sizeof(TM_SHFT_GROUP))
             , SUCCEED, NOT_DONE, FALSE);
  chk_status(_T("tm_define_struct()"), tm_define_struct(TM_ARTF_NAME, TM_ARTF_MAXI, sizeof(TM_ARTF_GROUP))
             , SUCCEED, NOT_DONE, FALSE);
  chk_status(_T("tm_init()"), tm_init(), SUCCEED, DONE, FALSE);

  memset(&c_item, 0, sizeof(TM_ITEM_GROUP));
  memset(&c_shft, 0, sizeof(TM_SHFT_GROUP));
  memset(&c_day,  0, sizeof(SHIFT_TDM_DEF));

  init_cust_rec(&cust);                   /* Initialise customer record.   */
  comm_tls_init(display_working);

  /* At startup, the current-mode is set to the default mode.              */
  pos_system.current_mode=pos_system.default_mode;
  pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
  invoice_mode     = pos_system.current_mode;  /* Initialise invoice and    */
  invoice_line_mode=invoice_mode;             /*   invoice-line mode.      */
  next_invoice_mode=-1;
  if(pos_system.current_mode==RETURN){
    reverse_invoice_active=YES;
  }

  /*                                                                       */
  /* Initialise some global variables.                                     */
  /*                                                                       */
  subt_display = NO;                  /* Default, display the article.     */
  cash_pincd_mistakes=0;              /* No mistakes till now.             */

                                      /* Set up scroll functions           */
  WriteStr(_T("- Scroll manager.."));
  StartPrintDots();
  chk_status(_T("init_scrl()"), init_scrl(), SUCCEED, DONE, FALSE);
  //init_scrn();                        /* Set up screen windows             */

  pos_init_errors();                  /* Set up Error Manager and errors   */

  /*                                                                       */
  /* The unblank-fix in tools\scrn_ uses an entry in the 7052 retail ROM.  */
  /* To avoid a crash on a non-retail system (illegal entry) the function  */
  /* rs_keyboard_type() is used to determine the system-type.              */
  /* This type is used in scrn_unblank() to decide if the unblank-fix      */
  /* is called or not.                                                     */
  /*                                                                       */

  setup_inp_mgr_price_fmt(genvar.ind_price_dec);   /* Setup price-formats. */

  /* Initialising input manager, customer display and cashdrawer           */
                                                    /* Setup keyboard mgr. */
  WriteStr( _T("- Input manager..") );
  StartPrintDots();
  (void *)fn_inp_idle  = (void *)application_inp_idle; /* External hookup. */
  KeyLockApprovalFunc  = KeylockEmulApprove;
  chk_status(_T("inp_init()"), inp_init(KEYBOARD_MASK|OCIA1_MASK|OCIA2_MASK,PausePrintDots),
                                                                        SUCCEED, DONE, FALSE);

  cdsp_closed_till(0, FALSE);           /* Display time and promo-txt.     */

  printers_attached = 0;
  count = 0;

  SaveRegistryEcho=GetRegistryEcho();
  SetRegistryEcho(FALSE);
  memset(Val, 0, sizeof(Val));
  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("NO_PRINTER_NAGGING"), Val, sizeof(Val)/sizeof(char)-1);
  NoPrinterNagging=FALSE;
  if (!_tcsicmp(Val, _T("YES"))) {
    NoPrinterNagging=TRUE;
  }
  NoPrinterNagging=FALSE;

  SetRegistryEcho(SaveRegistryEcho);
  for(i=DEV_PRINTER1; i < NUMBER_OF_PRINTERS; i++) {
	  /*comento esta parte para poder iniciar sin la impresora grande*/
    if (pos_printer_info[i].printersize == PRINTER_SIZE_NORMAL || pos_printer_info[i].printersize == PRINTER_SIZE_SMALL_INV) {
      bp_init(BP_XREAD, i);             /* Init X-read printer.            */
      bp_init(BP_ZREAD, i);             /* Init Z-read printer.            */
      if (pos_printer_info[i].printersize == PRINTER_SIZE_NORMAL) {
        bp_init(BP_INVOICE, i);           /* Init invoice printer.           */
		WriteStr(_T("NOW_NORMAL"));
        bp_now(BP_INVOICE, BP_INV_INIT, 0);   /* Initialise invoice printer. */
      }
	  else {
        bp_init(BP_SMALL_INVOICE, i);     /* Init invoice printer small.     */
		WriteStr(_T("NOW_SMALL_INV"));
        bp_now(BP_SMALL_INVOICE, BP_INV_INIT, 0);   /* Initialise invoice printer. */
      }
	  if (pos_printer_info[i].attached == YES) {
		printers_attached |= pos_printer_info[i].printersize;
        /*printers_attached |= PRINTER_SIZE_NORMAL*/;    
        selected_printer = i;                        
        count++;                                   
        if (count > 1 && NoPrinterNagging!=TRUE) {   
          MessageBox(NULL,(LPTSTR) _T("Only 1 NORMAL/SMALL_INV printer allowed. Check registry settings for printers !!!"), 
                                       (LPTSTR) _T("Error"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);                 
          de_init(_T("")); 
          exit(1);         
        }                  
      }                   
    }                      

    if (pos_printer_info[i].printersize == PRINTER_SIZE_SMALL) {
	   WriteStr(_T("- INIT.. "));
      bp_init(BP_SMALL_INVOICE, i);     /* Init invoice printer small.     */
	   WriteStr(_T("DONE "));
	   WriteStr(_T("- NOW.. "));
      bp_now(BP_SMALL_INVOICE, BP_INV_INIT, 0);
	   WriteStr(_T("DONE "));
      if (pos_printer_info[i].attached == YES) {
        printers_attached |= PRINTER_SIZE_SMALL;
      }
    }
  }

  /* Para anular el uso de la primera impresora */
  if (!((printers_attached & PRINTER_SIZE_NORMAL) || (printers_attached & PRINTER_SIZE_SMALL_INV)) && NoPrinterNagging!=TRUE) {
    MessageBox(NULL,(LPTSTR) _T("A NORMAL/SMALL_INV printer is mandatory. Check registry settings for printers !!!"),
                                 (LPTSTR) _T("Error"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
    de_init(_T(""));
    exit(1);
  }

  init_cheque_printers();

  WriteStr(_T("- Totals manager.."));
  StartPrintDots();
  chk_status(_T("tot_init()"), tot_init(MAX_TOTALS), SUCCEED, DONE, FALSE);

  /* In case a power-down forced an application re-start, the shifts in tdm*/
  /* must be re-initialised.                                               */
  status_of_pos = START_OF_DAY;
  recover_shift_2tm();

  if (c_shft.shift_no!=0) {
    /* One or more shifts have been recovered.                             */
    /* - Startup with 'New Shift'                                          */
    /* - if last shift not closed:                                         */
    /*   . perform an end of shift                                         */
    /*   . send a logoff                                                   */
    /*                                                                     */
    status_of_pos = START_OF_SHIFT;
    if (c_shft.time_off == 9999) {
      WriteStr(_T("*** ;)")); /*mlsd*/
      err_invoke(SHIFT_RECOVERED_NOT_CLOSED);
      send_logoff_cashier=YES;
      EndOfShift(_T(""),0);
    }
  }

  StopDots = TRUE;
  CloseHandle(MyThreadHandle);

  state_set(&PosStandBy_ST);            /* Initialize State Engine         */
  state_engine();                       /* Start Application               */

  return(SUCCEED);
} /* pos_main */


/*---------------------------------------------------------------------------*/
/*                           PrintMsg                                        */
/*---------------------------------------------------------------------------*/
void PrintMsg(char *msg )
{
  char buf[81];

  _stprintf(buf, _T("%-80s"), msg );
  DisplayString( buf, 0, 24, TEXT_REVERSE );

  GetVirtKey();
} /* PrintMsg */

/*---------------------------------------------------------------------------*/
/*                             de_init                                       */
/* - main deinit function of FreePOS. All fatal life functions will be        */
/*   disabled.                                                               */
/*---------------------------------------------------------------------------*/
void de_init(char *msg)
{
  short i;

  //SetShowCursor(FALSE);
  ClearScreen();

  if (msg && *msg) {
    WriteLn(msg);
    WriteLn("");
  }
  WriteLn("Please wait while de-initialising...");

  cdsp_clear();

  StopNetwork();
  tm_exit();                          /* Free the transaction manager      */
  tot_deinit();                       /* Free the totals manager           */
  inp_exit(KEYBOARD_MASK|OCIA1_MASK|OCIA2_MASK); /* Free the input manager */
  //OLE_DeInit();

  for(i=DEV_PRINTER1; i<NUMBER_OF_PRINTERS; i++) {
    ec_prn_exit(i);                   /* Free all printers                 */
  }
  oprn_deinit(SLIP_PRINTER1);         /* free the cheque printer           */
  bp_deinit();                        /* Free bp manager                   */
  scrn_close_windows();               /* Close Screen Manager              */
  err_unset_all();                    /* release allocated memory          */

  StopDisplay();

  Sleep(500);      /* Give ConDll some time to remove itself from my memory */

  return;
} /* de_init */

/*---------------------------------------------------------------------------*/
/*                          chk_status                                       */
/*---------------------------------------------------------------------------*/
void chk_status(char *name, short status, short status_success,
                short succ_action, short continue_on_error)
{
  char buf[100];

  if (status != status_success) {
    StopDots = TRUE;
    CloseHandle(MyThreadHandle);
    WriteStr( _T("failed") );
    _stprintf(buf, _T("Error %d on %s\n"), status, name);
    if(continue_on_error!=TRUE) {
      de_init(buf);
      exit(1);
    }
    WriteLn( _T("") );
  }
  else if (succ_action==DONE) {
    PausePrintDots();
    WriteStr( _T("done") );
    WriteLn( _T("") );
  }
} /* chk_status */

/*---------------------------------------------------------------------------*/
/*                          init_scrn                                        */
/*---------------------------------------------------------------------------*/
//void init_scrn(void)
//{
//  /*                                                                        */
//  /* Initialise the screen manager and define the windows.                  */
//  /*                                                                        */
//  screen_wnd = screen_windows;                           /* external hookup */
//
//  scrn_init_windows(NUM_WINDOWS);
//
//  /*                                                                        */
//  /* cashier inlog screen: CH02                                             */
//  /*                                                                        */
//  scrn_define_window(CASHIER_LOGON_WINDOW,0,0,22,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,CARET);
//
//  /*                                                                        */
//  /* invoice screen: CH01                                                   */
//  /*                                                                        */
//  scrn_define_window(INV_ART_DESCR_WINDOW,0,0,1,40,S40x12,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,NO_CARET);
//  scrn_define_window(INV_HEADER_WINDOW,3,0,1,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,NO_CARET);
//  scrn_define_window(INV_ART_INPUT_WINDOW,19,0,1,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,CARET);
//  scrn_define_window(INV_ART_LIST_WINDOW,5,0,13,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,SCROLL,NO_CARET);
//  scrn_define_window(INV_SUBT_WINDOW,21,0,2,34,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,NO_CARET);
//  scrn_define_window(INV_TOT_WINDOW,21,34,1,23,S40x12,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,NO_CARET);
//#ifdef NO_VIEW_POS_STATE
//  scrn_define_window(ERROR_WINDOW_ROW1,23,0,1,80,S80x25,
//     BLACK_ON_WHITE,BDR_NONE,NO_SCROLL,NO_CARET);
//#else
//  scrn_define_window(ERROR_WINDOW_ROW1,23,0,1,77,S80x25,
//     BLACK_ON_WHITE,BDR_NONE,NO_SCROLL,NO_CARET);
//  scrn_define_window(POS_STATUS_WINDOW,23,77,1,3,S80x25,
//     BLACK_ON_WHITE,BDR_NONE,NO_SCROLL,NO_CARET);
//#endif
//  scrn_define_window(ERROR_WINDOW_ROW2,24,0,1,71,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,NO_CARET);
//  scrn_define_window(LAN_STATUS_WINDOW,24,71,1,9,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,NO_CARET);
//  scrn_define_window(FISC_NO_WINDOW,8,2,5,38,S40x12,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,CARET);
//  scrn_define_window(SELECT_CUST_WINDOW,3,1,16,78,S80x25,
//     BLACK_ON_WHITE,BDR_NONE,NO_SCROLL,CARET);
//
//  /*                                                                        */
//  /* total screen: CH??                                                     */
//  /*                                                                        */
//  scrn_define_window(TOTAL_INPUT_WINDOW,21,0,1,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,CARET);
//  scrn_define_window(SELECT_PRINTER_WINDOW,9,6,7,68,S80x25,
//     BLACK_ON_WHITE,BDR_NONE,NO_SCROLL,CARET);
//
//
//  
//  /*                                                                        */
//  /* total screen: CH??                                                     */
//  /*                                                                        */
//  scrn_define_window(TOTAL_INPUT_WINDOW_PERCEPTION,14,0,2,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,CARET);
//
//  /*                                                                        */
//  /* operator key screen: CH03                                              */
//  /*                                                                        */
//  scrn_define_window(OPERATOR_WINDOW,0,0,23,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,CARET);
//
//  /*                                                                        */
//  /* paymentway screen: CH011                                               */
//  /*                                                                        */
//  scrn_define_window(PAYMENT_1_WINDOW,1,15,1,30,S40x12,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,NO_CARET);
//  scrn_define_window(PAYMENT_2_WINDOW,3,0,18,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,  CARET);
//
//  /*                                                                        */
//  /* article finder screen: CH0??                                           */
//  /*                                                                        */
//  scrn_define_window(ART_FIND_HEADER_WINDOW,2,0,1,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,NO_SCROLL,NO_CARET);
//  scrn_define_window(ART_FIND_LIST_WINDOW,4,0,17,80,S80x25,
//     WHITE_ON_BLACK,BDR_NONE,SCROLL,CARET);
//
//  return;
//} /* init_scrn */

/*---------------------------------------------------------------------------*/
/*                     PausePrintDots                                        */
/*---------------------------------------------------------------------------*/
void PausePrintDots(void) 
{
  if (MyThreadHandle) {
    paused = TRUE;
    SuspendThread(MyThreadHandle);
  }
  return;
} /* PausePrintDots */

/*---------------------------------------------------------------------------*/
/*                     StartPrintDots                                        */
/*---------------------------------------------------------------------------*/
void StartPrintDots(void) 
{
  if (MyThreadHandle) {
    paused = FALSE;
    ResumeThread(MyThreadHandle);
  }
  return;
} /* StartPrintDots */

/*---------------------------------------------------------------------------*/
/*                          PrintDots                                        */
/*---------------------------------------------------------------------------*/
unsigned int __stdcall PrintDots(void *lp)
{
  short row, col;

  while (StopDots != TRUE) {
    if (paused == FALSE) {
      scrn_get_csr(&row, &col);
      if (col > 75) {
        WriteLn(_T(""));
        WriteStr(_T("  "));
      }
      WriteCh('.');
    }
    Sleep(200);
  }
  return (0);
} /* PrintDots */
