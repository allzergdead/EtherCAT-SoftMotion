#ifndef AXISWID_H
#define AXISWID_H

#include <QWidget>
//#include "../widgets/baseform.h"
#include "../ethercatMaster/etcHeader/etcheader.h"
#include "../ethercatMaster/etcHeader/jmc_axis_cmd.h"


#include <QTcpSocket>

#include <QSharedMemory>
#include <QTreeWidget>
#include <QFileDialog>
#include <QDir>
#include <qtablewidget.h>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>

#include "etcHeader/jmc_axis_cmd.h"
namespace Ui {
class axisWid;
}

class axisWid : public QWidget
{
    Q_OBJECT

public:
    explicit axisWid(QWidget *parent = nullptr);
    ~axisWid();
    int showby(int axisNo);
    QTimer t200msTimer;
    QTimer t10msTimer;
    int axNo = -1;
    QString name;

    JMC_Power power;
    JMC_MoveAbs abs;
    JMC_MoveVel vel;
    JMC_SetPos setpos;
    JMC_WTMC_TC chtc;
    JMC_Stop stop;
private slots:
    void on_pos_clicked();
    void btnPressDouble();
    void btnPressInt();
    void t200msTimerSlot();
    void t10msTimerSlot();
    void on_absmode_clicked();

    void on_velmode_clicked();

    void on_setpos_clicked();

    void on_chmode_clicked();

    void on_stop_clicked();

    void on_power_clicked();

private:
    Ui::axisWid *ui;
};

#endif // AXISWID_H
