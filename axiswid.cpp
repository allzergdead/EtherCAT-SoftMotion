#include "axiswid.h"
#include "ui_axiswid.h"

#include <QPushButton>
#include <QInputDialog>
//#include "ethercatmaster.h"
axisWid::axisWid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::axisWid)
{
    ui->setupUi(this);

    connect(ui->pos,&QPushButton::clicked,this,&axisWid::btnPressDouble);
    connect(ui->vel,&QPushButton::clicked,this,&axisWid::btnPressDouble);
    connect(ui->acc,&QPushButton::clicked,this,&axisWid::btnPressDouble);
    connect(ui->dec,&QPushButton::clicked,this,&axisWid::btnPressDouble);
    connect(ui->jerk,&QPushButton::clicked,this,&axisWid::btnPressDouble);


    connect(ui->torque,&QPushButton::clicked,this,&axisWid::btnPressInt);

    connect(ui->axmode,&QPushButton::clicked,this,&axisWid::btnPressInt);


}

axisWid::~axisWid()
{
    delete ui;
}

int axisWid::showby(int axisNo)
{
    if(axisNo >= MAX_AXIS || axisNo < 0){
        return 0;
    }
    //hide();
    axNo = axisNo;

    connect(&t200msTimer,&QTimer::timeout,this,&axisWid::t200msTimerSlot);
    t200msTimer.setInterval(200);
    t200msTimer.start();

    connect(&t10msTimer,&QTimer::timeout,this,&axisWid::t10msTimerSlot);
    t10msTimer.setInterval(10);
    t10msTimer.start();

    //show();
    return 0;
}
//power_off,// PLCOPEN call Disabled
//errorstop,
//stopping,
//standstill,
//discrete_motion,
//continuous_motion,
//synchronized_motion,
//homing
QString formIntAxisState(int state)
{
    switch (state) {
        case 0:
            return "power_off";
        case 1:
            return "errorstop";
        case 2:
            return "stopping";
        case 3:
            return "standstill";
        case 4:
            return "discrete_motion";
        case 5:
            return "continuous_motion";
        case 6:
            return "synchronized_motion";
        case 7:
            return "homing";
        default:
            return "error_state";

    }
}
void  axisWid::t10msTimerSlot()
{
    //return;
    if(axNo >= MAX_AXIS || axNo < 0){
        return;
    }
    if(axis_hash.find(name) == axis_hash.end()){
        return;
    }
    power.call(name);
    abs.call(name);
    vel.call(name);
    setpos.call(name);
    chtc.call(name);
    stop.call(name);


    if(abs.Execute == true ){
        abs.Execute = false;
    }
    if(vel.Execute == true){
        vel.Execute = false;
    }
    if(setpos.Execute == true){
        setpos.Execute = false;
    }
    if(chtc.Execute == true){
        chtc.Execute = false;
    }
    if(stop.Execute == true){
        stop.Execute = false;
    }

    if(axis_hash[name]->modes_of_operation_display == 10){
        axis_hash[name]->sSetTorque = ui->torque->text().toInt();
    }
}
void axisWid::t200msTimerSlot()
{
    //return;
    if(axNo >= MAX_AXIS || axNo < 0){
        return;
    }
    if(axis_hash.find(name) == axis_hash.end()){
        return;
    }
    auto axisp = axis_hash[name];
    QString info;
    info = info + "位置 ： " + QString::number(axisp->fActPos) + "\n";
    info = info + "速度 ： " + QString::number(axisp->fActVel) + "\n";
    info = info + "加速度 ： " + QString::number(axisp->fActAcc) + "\n";
    info = info + "扭矩 ： " + QString::number(axisp->sActTorque) + "\n";
    info = info + "状态机 ： " + formIntAxisState(axisp->state) + "\n";
    info = info + "控制模式 ： " + QString::number(axisp->modes_of_operation_display) + "\n";


    ui->axisState->setText(info);

}
void axisWid::btnPressDouble()
{

   QPushButton *self = (QPushButton *)sender();
   double input = QInputDialog::getDouble(this,"请输入","",0);
   self->setText(QString::number(input));
}
void axisWid::btnPressInt()
{
    QPushButton *self = (QPushButton *)sender();
    double input = QInputDialog::getInt(this,"请输入","",0);
    self->setText(QString::number(input));
}
//JMC_Power power;
//JMC_MoveAbs abs;
//JMC_MoveVel vel;
//JMC_SetPos setpos;
//JMC_WTMC_TC chtc;
void axisWid::on_pos_clicked()
{

}
void axisWid::on_absmode_clicked()
{
    abs.fPos = ui->pos->text().toDouble();
    abs.fVel = ui->vel->text().toDouble();
    abs.fAcc = ui->acc->text().toDouble();
    abs.fDec = ui->dec->text().toDouble();
    abs.fJerk = ui->jerk->text().toDouble();
    abs.Execute = true;
}

void axisWid::on_velmode_clicked()
{
    vel.fVel = ui->vel->text().toDouble();
    vel.fAcc = ui->acc->text().toDouble();
    vel.fDec = ui->dec->text().toDouble();
    vel.fJerk = ui->jerk->text().toDouble();
    vel.Execute = true;
}

void axisWid::on_setpos_clicked()
{
    setpos.fPos = ui->pos->text().toDouble();
    setpos.Execute = true;
}

void axisWid::on_chmode_clicked()
{
    chtc.cmode = ui->axmode->text().toInt();
    chtc.Execute = true;
}

void axisWid::on_stop_clicked()
{

    stop.fDec = ui->dec->text().toDouble();
    stop.fJerk = ui->jerk->text().toDouble();
    stop.Execute = true;
}

void axisWid::on_power_clicked()
{
    power.xPOn = !power.xPOn;
    power.xDStart = !power.xDStart;

}
