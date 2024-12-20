#include "axis_setting_wid.h"
#include "ui_axis_setting_wid.h"

axis_setting_wid::axis_setting_wid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::axis_setting_wid)
{
    ui->setupUi(this);
    connect(ui->name,&QPushButton::clicked,this,&axis_setting_wid::edit_name);
    connect(ui->enc_ratio,&QPushButton::clicked,this,&axis_setting_wid::edit_enc_ratio);
    connect(ui->pdoNo,&QPushButton::clicked,this,&axis_setting_wid::edit_pdoNo);
    connect(ui->circle_diff,&QPushButton::clicked,this,&axis_setting_wid::edit_circle_diff);


    connect(ui->set_vaxis,&QPushButton::clicked,this,&axis_setting_wid::click_vaxis);
}

axis_setting_wid::~axis_setting_wid()
{
    delete ui;
}
bool axis_setting_wid::checkAxis()
{
    if(curaxis == nullptr){
        insertLog(-1,"轴编辑器","绑定错误！程序有误");
        return false;
    }
    return true;
}
void axis_setting_wid::edit_name()
{
    if(!checkAxis()){
        insertLog(-1,"轴编辑器","绑定错误！程序有误 1");
        return;
    }
    QInputDialog dialog;
    bool ok;
    QString name = dialog.getText(this,"axis name","请输入",QLineEdit::Normal,curaxis->name,&ok);
    if(!ok){
        return;
    }
    if(name == ""){
        return;
    }
    curaxis->name = name;
    ui->name->setText(name);
}
void axis_setting_wid::edit_enc_ratio()
{
    if(!checkAxis()){
        insertLog(-1,"轴编辑器","绑定错误！程序有误 2");
        return;
    }
    QInputDialog dialog;
    bool ok;
    int enc_ratio = dialog.getInt(this,"enc_ratio","请输入",curaxis->enc_ratio,0,2137483647,1,&ok);
    if(!ok){
        return;
    }
    curaxis->enc_ratio = enc_ratio;
    ui->enc_ratio->setText(QString::number(enc_ratio));
}
void axis_setting_wid::edit_pdoNo()
{
    if(!checkAxis()){
        insertLog(-1,"轴编辑器","绑定错误！程序有误 3");
        return;
    }
    QInputDialog dialog;
    bool ok;
    int pdoNo = dialog.getInt(this,"pdoNo","请输入",curaxis->enc_ratio,1,64,1,&ok);
    if(!ok){
        return;
    }
    curaxis->pdoNo = pdoNo;
    ui->pdoNo->setText(QString::number(pdoNo));
}
void axis_setting_wid::edit_circle_diff()
{
    if(!checkAxis()){
        insertLog(-1,"轴编辑器","绑定错误！程序有误 4");
        return;
    }
    QInputDialog dialog;
    bool ok;
    double circle_diff = dialog.getDouble(this,"circle_diff","请输入",curaxis->circle_diff,0.0000001,10000000,8,&ok);
    if(!ok){
        return;
    }
    curaxis->circle_diff = circle_diff;
    ui->circle_diff->setText(QString::number(circle_diff));
}
void axis_setting_wid::edit_press_enable()
{
    if(!checkAxis()){
        insertLog(-1,"轴编辑器","绑定错误！程序有误 5");
        return;
    }

}
void axis_setting_wid::click_vaxis()
{
    if(!checkAxis()){
        insertLog(-1,"轴编辑器","绑定错误！程序有误 6");
        return;
    }
    if(QMessageBox::Yes == QMessageBox::information(this,"警告","即将把轴 "+curaxis->name + "设置为虚轴",QMessageBox::Yes |QMessageBox::No)){
        curaxis->pdoNo = -1;
        ui->pdoNo->setText(QString::number(-1));
    }
}
int axis_setting_wid::from_axis(axis_config *axis)
{
    curaxis = axis;
    ui->name->setText(axis->name);
    ui->enc_ratio->setText(QString::number(axis->enc_ratio));
    ui->circle_diff->setText(QString::number(axis->circle_diff));
    ui->pdoNo->setText(QString::number(axis->pdoNo));


    return 0;
}
int axis_setting_wid::to_axis(axis_config *axis)
{
    axis->name = ui->name->text();
    axis->enc_ratio = ui->enc_ratio->text().toUInt();
    axis->circle_diff = ui->circle_diff->text().toDouble();
    axis->pdoNo = ui->pdoNo->text().toInt();

    return 0;
}




