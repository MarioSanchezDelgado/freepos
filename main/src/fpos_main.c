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
 *               Perú
 *
 * --------------------------------------------------------------------------
 *                            CHANGELOG
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 29-Oct-2019 Initial Release FreePOS                                 MLSD 
 * -------------------------------------------------------------------------- 
 */

#include <stdio.h>

#include "fpos_state.h"
#include "fpos_states.h"
#include "fpos_screen.h"

short fpos_main();

int main(int argc, char **argv) 
{
  short  result;

  fprintf(stderr, "ini initial_window\n");
  LaunchAppFreepos(argc, argv);
  fprintf(stderr, "initial_window\n");
  //result = fpos_main();

  //de_init(NULL);

  return(result);
} 

/*---------------------------------------------------------------------------*/
/*                          fpos_main                                        */
/*---------------------------------------------------------------------------*/
short fpos_main()
{
//  state_set(&PosStandBy_ST);            /* Initialize State Engine         */
  fprintf(stdout, "seteo posstandby st\n");
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
