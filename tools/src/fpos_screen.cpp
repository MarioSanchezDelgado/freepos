#include <QApplication>
#include <QtWidgets>
#include <QtGui>

#include <stdio.h>

#include "fpos_screen.h"
#include "fpos_screen_qt.h"
#include "fpos_state.h"

#include "fpos_states.h"

static FposScreen *ptrFposScreen;

int LaunchAppFreepos(int argc, char **argv) 
{
  
  fprintf(stdout, ">Inicio launxh\n");
  QApplication app(argc, argv);
  
  FposScreen fposScreen;
  ptrFposScreen = &fposScreen;

  state_set(&PosStandBy_ST);            /* Initialize State Engine         */
  fprintf(stdout, "seteo posstandby st\n");
//  state_engine();                       /* Start Application               */

  fprintf(stdout, "Por ejecutar\n");
  return app.exec();
}



void standby_window() 
{
  ptrFposScreen->goStandBy();
}
