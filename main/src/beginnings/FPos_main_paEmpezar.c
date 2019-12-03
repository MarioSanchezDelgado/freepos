/*
 *     Module Name       : FPOS_MAIN.C
 *
 *     Type              : Main entry of the FreePOS Application
 *                         
 *
 *     Author/Location   : Mario Sanchez Delgado, Lima-Per�
 *
 *     Copyright Daichin International SA
 *               Lima
 *               Per�
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 29-Oct-2019 Initial Release FreePOS                                 MLSD 
 * -------------------------------------------------------------------------- 
 */

#include <gtk/gtk.h>

short fpos_main(void);

void ini_windows(void);


int main(int argc, char **argv) 
{
  short  result;

  //---------- A implementar -------------
  ini_windows();
  result = fpos_main();

  //de_init(NULL);

  return(result);
} 

/*---------------------------------------------------------------------------*/
/*                          fpos_main                                        */
/*---------------------------------------------------------------------------*/
short fpos_main(void)
{
//  char  Val[15];
//  char  dummy[20];
//  short   i, count;
//  ASYNC_CALLBACK ac;
//  short   SaveRegistryEcho;
//  short   NoPrinterNagging;

  /*                                                                         */
  /* Main Entry Point of the POS Application:                                */
  /*                                                                         */
  /*   Initialize Devices, Data Managers and start State Engine.             */
  /*   If the Engine Stops, perform graceful exit.                           */
  /*                                                                         */

//  MyThreadHandle = CreateThread(
//        NULL,                       /* pointer to thread security attributes */
//        0,                          /* initial thread stack size, in bytes   */
//        PrintDots,                  /* pointer to thread function            */
//        NULL,                       /* argument for new thread               */
//        CREATE_SUSPENDED,           /* creation flags                        */
//        &MyThreadId                 /* pointer to returned thread identifier */
//      );

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
//  for(i=DEV_PRINTER1; i < NUMBER_OF_PRINTERS; i++) {
//    _stprintf(dummy, _T("PRINTER%d_ATTACHED"), i+1);
//    ReadEnvironmentValue(TREE_TILL_DEPENDENT, dummy, Val, sizeof(Val)/sizeof(char)-1);
//    prn_on[i] = YES;
//    if (!_tcsicmp(Val, _T("NO"))) {
//      prn_on[i] = NO;
//    }
//  }
//
//  /*  print_to_file is used to reroute the printer output to a file.        */
//  print_to_file = NO;
//  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("PRINT_TO_FILE"), Val, sizeof(Val)/sizeof(char)-1);
//  if (!_tcsicmp(Val, _T("YES"))) {
//    clear_all_used_print_files();
//    print_to_file = YES;
//  }
//       /* register entity types that are handled asynchronously by the application */
//                                                                  /* Crazy article */
//  ac.type = CART_TYPE;
//  ac.callback_fn = &get_cart_callback; 
//  register_async_callback(&ac);
//                                                                      /* Powerdown */
//  ac.type = PWDN_TYPE;
//  ac.callback_fn = &activate_pwdn_callback; 
//  register_async_callback(&ac);
//
//  ac.type = STAT_TYPE;
//  ac.callback_fn = &return_pos_status; 
//  register_async_callback(&ac);
//
//  ac.type = MMML_TYPE;
//  ac.callback_fn = &update_multisam_definitions_callback; 
//  register_async_callback(&ac);
//
//  ac.type = TILL_TYPE;
//  ac.callback_fn = &update_till_callback; 
//  register_async_callback(&ac);
//
//  WriteStr( _T("- Network..") );
//  StartPrintDots();
//  chk_status(_T("StartNetwork()"), (short)StartNetwork(PausePrintDots), TRUE, DONE, FALSE);
//
//  WriteStr( _T("- Environment..") );
//  StartPrintDots();
//  chk_status(_T("init_environment_records()"),
//             init_environment_records(NO_WINDOW,PausePrintDots), SUCCEED, DONE, TRUE);
//
//  WriteStr( _T("- Transaction manager..") );
//  StartPrintDots();
//
//  chk_status(_T("tm_define_struct()"), tm_define_struct(TM_ITEM_NAME, TM_ITEM_MAXI, sizeof(TM_ITEM_GROUP))
//             , SUCCEED, NOT_DONE, FALSE);
//  chk_status(_T("tm_define_struct()"), tm_define_struct(TM_SHFT_NAME, TM_SHFT_MAXI, sizeof(TM_SHFT_GROUP))
//             , SUCCEED, NOT_DONE, FALSE);
//  chk_status(_T("tm_define_struct()"), tm_define_struct(TM_ARTF_NAME, TM_ARTF_MAXI, sizeof(TM_ARTF_GROUP))
//             , SUCCEED, NOT_DONE, FALSE);
//  chk_status(_T("tm_init()"), tm_init(), SUCCEED, DONE, FALSE);
//
//  memset(&c_item, 0, sizeof(TM_ITEM_GROUP));
//  memset(&c_shft, 0, sizeof(TM_SHFT_GROUP));
//  memset(&c_day,  0, sizeof(SHIFT_TDM_DEF));
//
//  init_cust_rec(&cust);                   /* Initialise customer record.   */
//  comm_tls_init(display_working);
//
//  /* At startup, the current-mode is set to the default mode.              */
//  pos_system.current_mode=pos_system.default_mode;
//  pos_upda_system(POS_SYST_SIZE, SYSTEM_FNO, &pos_system);
//  invoice_mode     = pos_system.current_mode;  /* Initialise invoice and    */
//  invoice_line_mode=invoice_mode;             /*   invoice-line mode.      */
//  next_invoice_mode=-1;
//  if(pos_system.current_mode==RETURN){
//    reverse_invoice_active=YES;
//  }
//
//  /*                                                                       */
//  /* Initialise some global variables.                                     */
//  /*                                                                       */
//  subt_display = NO;                  /* Default, display the article.     */
//  cash_pincd_mistakes=0;              /* No mistakes till now.             */
//
//                                      /* Set up scroll functions           */
//  WriteStr(_T("- Scroll manager.."));
//  StartPrintDots();
//  chk_status(_T("init_scrl()"), init_scrl(), SUCCEED, DONE, FALSE);
//  //init_scrn();                        /* Set up screen windows             */
//
//  pos_init_errors();                  /* Set up Error Manager and errors   */
//
//  /*                                                                       */
//  /* The unblank-fix in tools\scrn_ uses an entry in the 7052 retail ROM.  */
//  /* To avoid a crash on a non-retail system (illegal entry) the function  */
//  /* rs_keyboard_type() is used to determine the system-type.              */
//  /* This type is used in scrn_unblank() to decide if the unblank-fix      */
//  /* is called or not.                                                     */
//  /*                                                                       */
//
//  setup_inp_mgr_price_fmt(genvar.ind_price_dec);   /* Setup price-formats. */
//
//  /* Initialising input manager, customer display and cashdrawer           */
//                                                    /* Setup keyboard mgr. */
//  WriteStr( _T("- Input manager..") );
//  StartPrintDots();
//  (void *)fn_inp_idle  = (void *)application_inp_idle; /* External hookup. */
//  KeyLockApprovalFunc  = KeylockEmulApprove;
//  chk_status(_T("inp_init()"), inp_init(KEYBOARD_MASK|OCIA1_MASK|OCIA2_MASK,PausePrintDots),
//                                                                        SUCCEED, DONE, FALSE);
//
//  cdsp_closed_till(0, FALSE);           /* Display time and promo-txt.     */
//
//  printers_attached = 0;
//  count = 0;
//
//  SaveRegistryEcho=GetRegistryEcho();
//  SetRegistryEcho(FALSE);
//  memset(Val, 0, sizeof(Val));
//  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("NO_PRINTER_NAGGING"), Val, sizeof(Val)/sizeof(char)-1);
//  NoPrinterNagging=FALSE;
//  if (!_tcsicmp(Val, _T("YES"))) {
//    NoPrinterNagging=TRUE;
//  }
//  NoPrinterNagging=FALSE;
//
//  SetRegistryEcho(SaveRegistryEcho);
//  for(i=DEV_PRINTER1; i < NUMBER_OF_PRINTERS; i++) {
//	  /*comento esta parte para poder iniciar sin la impresora grande*/
//    if (pos_printer_info[i].printersize == PRINTER_SIZE_NORMAL || pos_printer_info[i].printersize == PRINTER_SIZE_SMALL_INV) {
//      bp_init(BP_XREAD, i);             /* Init X-read printer.            */
//      bp_init(BP_ZREAD, i);             /* Init Z-read printer.            */
//      if (pos_printer_info[i].printersize == PRINTER_SIZE_NORMAL) {
//        bp_init(BP_INVOICE, i);           /* Init invoice printer.           */
//		WriteStr(_T("NOW_NORMAL"));
//        bp_now(BP_INVOICE, BP_INV_INIT, 0);   /* Initialise invoice printer. */
//      }
//	  else {
//        bp_init(BP_SMALL_INVOICE, i);     /* Init invoice printer small.     */
//		WriteStr(_T("NOW_SMALL_INV"));
//        bp_now(BP_SMALL_INVOICE, BP_INV_INIT, 0);   /* Initialise invoice printer. */
//      }
//	  if (pos_printer_info[i].attached == YES) {
//		printers_attached |= pos_printer_info[i].printersize;
//        /*printers_attached |= PRINTER_SIZE_NORMAL*/;    
//        selected_printer = i;                        
//        count++;                                   
//        if (count > 1 && NoPrinterNagging!=TRUE) {   
//          MessageBox(NULL,(LPTSTR) _T("Only 1 NORMAL/SMALL_INV printer allowed. Check registry settings for printers !!!"), 
//                                       (LPTSTR) _T("Error"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);                 
//          de_init(_T("")); 
//          exit(1);         
//        }                  
//      }                   
//    }                      
//
//    if (pos_printer_info[i].printersize == PRINTER_SIZE_SMALL) {
//	   WriteStr(_T("- INIT.. "));
//      bp_init(BP_SMALL_INVOICE, i);     /* Init invoice printer small.     */
//	   WriteStr(_T("DONE "));
//	   WriteStr(_T("- NOW.. "));
//      bp_now(BP_SMALL_INVOICE, BP_INV_INIT, 0);
//	   WriteStr(_T("DONE "));
//      if (pos_printer_info[i].attached == YES) {
//        printers_attached |= PRINTER_SIZE_SMALL;
//      }
//    }
//  }
//
//  /* Para anular el uso de la primera impresora */
//  if (!((printers_attached & PRINTER_SIZE_NORMAL) || (printers_attached & PRINTER_SIZE_SMALL_INV)) && NoPrinterNagging!=TRUE) {
//    MessageBox(NULL,(LPTSTR) _T("A NORMAL/SMALL_INV printer is mandatory. Check registry settings for printers !!!"),
//                                 (LPTSTR) _T("Error"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
//    de_init(_T(""));
//    exit(1);
//  }
//
//  init_cheque_printers();
//
//  WriteStr(_T("- Totals manager.."));
//  StartPrintDots();
//  chk_status(_T("tot_init()"), tot_init(MAX_TOTALS), SUCCEED, DONE, FALSE);
//
//  /* In case a power-down forced an application re-start, the shifts in tdm*/
//  /* must be re-initialised.                                               */
//  status_of_pos = START_OF_DAY;
//  recover_shift_2tm();
//
//  if (c_shft.shift_no!=0) {
//    /* One or more shifts have been recovered.                             */
//    /* - Startup with 'New Shift'                                          */
//    /* - if last shift not closed:                                         */
//    /*   . perform an end of shift                                         */
//    /*   . send a logoff                                                   */
//    /*                                                                     */
//    status_of_pos = START_OF_SHIFT;
//    if (c_shft.time_off == 9999) {
//      WriteStr(_T("*** ;)")); /*mlsd*/
//      err_invoke(SHIFT_RECOVERED_NOT_CLOSED);
//      send_logoff_cashier=YES;
//      EndOfShift(_T(""),0);
//    }
//  }
//
//  StopDots = TRUE;
//  CloseHandle(MyThreadHandle);
//
//  state_set(&PosStandBy_ST);            /* Initialize State Engine         */
//  state_engine();                       /* Start Application               */
//
  return(0);
} /* pos_main */


/*---------------------------------------------------------------------------*/
/*                           PrintMsg                                        */
/*---------------------------------------------------------------------------*/
//void PrintMsg(char *msg )
//{
//  char buf[81];
//
//  _stprintf(buf, _T("%-80s"), msg );
//  DisplayString( buf, 0, 24, TEXT_REVERSE );
//
//  GetVirtKey();
//} /* PrintMsg */
//
///*---------------------------------------------------------------------------*/
///*                             de_init                                       */
///* - main deinit function of FreePOS. All fatal life functions will be        */
///*   disabled.                                                               */
///*---------------------------------------------------------------------------*/
//void de_init(char *msg)
//{
//  short i;
//
//  //SetShowCursor(FALSE);
//  ClearScreen();
//
//  if (msg && *msg) {
//    WriteLn(msg);
//    WriteLn("");
//  }
//  WriteLn("Please wait while de-initialising...");
//
//  cdsp_clear();
//
//  StopNetwork();
//  tm_exit();                          /* Free the transaction manager      */
//  tot_deinit();                       /* Free the totals manager           */
//  inp_exit(KEYBOARD_MASK|OCIA1_MASK|OCIA2_MASK); /* Free the input manager */
//  //OLE_DeInit();
//
//  for(i=DEV_PRINTER1; i<NUMBER_OF_PRINTERS; i++) {
//    ec_prn_exit(i);                   /* Free all printers                 */
//  }
//  oprn_deinit(SLIP_PRINTER1);         /* free the cheque printer           */
//  bp_deinit();                        /* Free bp manager                   */
//  scrn_close_windows();               /* Close Screen Manager              */
//  err_unset_all();                    /* release allocated memory          */
//
//  StopDisplay();
//
//  Sleep(500);      /* Give ConDll some time to remove itself from my memory */
//
//  return;
//} /* de_init */
//
///*---------------------------------------------------------------------------*/
///*                          chk_status                                       */
///*---------------------------------------------------------------------------*/
//void chk_status(char *name, short status, short status_success,
//                short succ_action, short continue_on_error)
//{
//  char buf[100];
//
//  if (status != status_success) {
//    StopDots = TRUE;
//    CloseHandle(MyThreadHandle);
//    WriteStr( _T("failed") );
//    _stprintf(buf, _T("Error %d on %s\n"), status, name);
//    if(continue_on_error!=TRUE) {
//      de_init(buf);
//      exit(1);
//    }
//    WriteLn( _T("") );
//  }
//  else if (succ_action==DONE) {
//    PausePrintDots();
//    WriteStr( _T("done") );
//    WriteLn( _T("") );
//  }
//} /* chk_status */

/*-------------------- ini_windows ------------ */
void ini_windows(void)
{
//  GtkBuilder *builder;
//  GObject *window;
  GError *error = NULL;
//  GdkColor color;
//
//  color.red =0x0000;
//  color.green = 0x0000;
//  color.blue = 0x0000;
//
//  gtk_init(NULL, NULL);
//  builder = gtk_builder_new ();
//  if (gtk_builder_add_from_file (builder, "builder.ui", &error) == 0)
//  {
//    g_printerr ("Error loading file: %s\n", error->message);
//    g_clear_error (&error);
//    return;
//  }
//
//  /* Connect signal handlers to the constructed widgets. */
//  window = gtk_builder_get_object (builder, "window");
//  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
//  gtk_window_fullscreen((GtkWindow *) window);
//  gtk_widget_modify_bg((GtkWidget *) window, GTK_STATE_NORMAL, &color);
  GtkWidget *main_window;
  GtkCssProvider *provider;
  GtkStyleContext *context;
  GdkDisplay *display;
  GdkScreen *screen;

  GtkWidget *frame;


  gtk_init(NULL, NULL);
  main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect (main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  

  /* CSS */
  provider = gtk_css_provider_new();
  display = gdk_display_get_default();
  screen = gdk_display_get_default_screen(display);
  gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  gtk_css_provider_load_from_file(provider, g_file_new_for_path("styles.css"), &error);
   if (error != NULL){
    fprintf (stderr, "Unable to read file: %s\n", error->message);
  }
  g_object_unref(provider);

  context = gtk_widget_get_style_context(main_window);
  gtk_style_context_add_class(context, "main_window");
  gtk_window_fullscreen((GtkWindow *) main_window);
  
  frame = gtk_frame_new(NULL);
  gtk_container_add(GTK_CONTAINER (main_window), frame);
  //gtk_frame_set_label(GTK_FRAME (frame), "akfdlakfdalf");
  //gtk_frame_set_label_align(GTK_FRAME (frame), 0.0, 0.0);
  gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_NONE);

  gtk_widget_show(frame);
  gtk_widget_show_all(main_window); 
  gtk_main ();

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
  return;
} /* ini_windows */

/*---------------------------------------------------------------------------*/
/*                     PausePrintDots                                        */
/*---------------------------------------------------------------------------*/
//void PausePrintDots(void) 
//{
//  if (MyThreadHandle) {
//    paused = TRUE;
//    SuspendThread(MyThreadHandle);
//  }
//  return;
//} /* PausePrintDots */
//
///*---------------------------------------------------------------------------*/
///*                     StartPrintDots                                        */
///*---------------------------------------------------------------------------*/
//void StartPrintDots(void) 
//{
//  if (MyThreadHandle) {
//    paused = FALSE;
//    ResumeThread(MyThreadHandle);
//  }
//  return;
//} /* StartPrintDots */
//
///*---------------------------------------------------------------------------*/
///*                          PrintDots                                        */
///*---------------------------------------------------------------------------*/
//unsigned int __stdcall PrintDots(void *lp)
//{
//  short row, col;
//
//  while (StopDots != TRUE) {
//    if (paused == FALSE) {
//      scrn_get_csr(&row, &col);
//      if (col > 75) {
//        WriteLn(_T(""));
//        WriteStr(_T("  "));
//      }
//      WriteCh('.');
//    }
//    Sleep(200);
//  }
//  return (0);
//} /* PrintDots */
