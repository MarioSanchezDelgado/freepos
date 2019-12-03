#include "fpos_screen_qt.h"

#include <QApplication>
#include <QtWidgets>
#include <QtGui>

FposScreen::FposScreen(QWidget *parent):
  QWidget(parent)
{
  QFile File("/home/mlsd/programming/devpos/freepos/files/freepos_initial.qss");
  if(!File.open(QFile::ReadOnly)){
    fprintf(stdout, "NO abrio archivo\n");
  }
  QString fposStyleSheet = QLatin1String(File.readAll());
  fprintf(stdout, "del archivo %s", fposStyleSheet.toLocal8Bit().data());

  setObjectName("MainWindow");
  setStyleSheet(fposStyleSheet);
//  setStyleSheet("background-color: green;");
//  setStyleSheet("background-image: url(/home/mlsd/programming/devpos/freepos/files/test2.jpg);");

//:wq!  fprintf(stdout, "full screen\n");
  showFullScreen();
//  show();
  connect(this, SIGNAL(goStandBy()), this, SLOT(StandByState()));

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

void FposScreen::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
