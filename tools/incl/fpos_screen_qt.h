#ifndef FPOS_SCREEN_QT_H
#define FPOS_SCREEN_QT_H

#include <QWidget>

class FposScreen : public QWidget
{
  Q_OBJECT
  public:
    explicit  FposScreen(QWidget *parent = 0);
    ~FposScreen();
  protected:   
    void paintEvent(QPaintEvent *);
  signals:
    void goStandBy();
  private slots:
    void StandByState();

};


#endif
