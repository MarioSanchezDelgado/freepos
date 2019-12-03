#include "fpos_screen_qt.h"

#include <QApplication>
#include <QtGui>

FposScreen::FposScreen(QWidget *parent):
  QWidget(parent)
{
  showFullScreen();

  connect(this, SIGNAL(goStandBy()), QApplication::instance(), SLOT(StandByState()));

}

FposScreen::~FposScreen()
{

}

void FposScreen::StandByState()
{
  QFile File("/home/mlsd/programming/devpos/freepos/files/freepos_standby.qss");
  if(!File.open(QFile::ReadOnly)){
    fprintf(stdout, "NO abrio archivo\n");
  }
  QString fposStyleSheet = QLatin1String(File.readAll());

  setObjectName("StandByWindow");
  setStyleSheet(fposStyleSheet);
}

