#include "slave_assign_wid.h"
#include "ui_slave_assign_wid.h"

slave_assign_wid::slave_assign_wid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::slave_assign_wid)
{
    ui->setupUi(this);
    foreach (QComboBox *cb , this->findChildren<QComboBox *>()){
        cb->installEventFilter(this);
    }
}
int slave_assign_wid::setSize(int size)
{
    ui->size->setText(QString::number(size));
    return 0;
}
int slave_assign_wid::setType(QString type)
{
    ui->type->setText(type);
    return 0;
}
int slave_assign_wid::setWD(int WD)
{
    ui->watchDog->setCurrentIndex(WD);
    return 0;
}
int slave_assign_wid::from(slave_assign *assign)
{
    curassign = assign;
    setType(assign->info);
    setSize(assign->index.size());
    setWD(assign->WD);
    return 0;
}
slave_assign_wid::~slave_assign_wid()
{
    delete ui;
}

void slave_assign_wid::on_watchDog_currentIndexChanged(int index)
{
    if (curassign == nullptr){
        return;
    }
    curassign->WD = index;
}
bool slave_assign_wid::eventFilter(QObject *obj, QEvent *event)
{
    if(obj->inherits("QComboBox")){
        if(event->type() == QEvent::Wheel) return true;
    }
    //return bCut_Line_wid::eventFilter(obj,event);
    return false;
}
