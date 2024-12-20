#include "slave_startup_wid.h"
#include "ui_slave_startup_wid.h"

slave_startup_wid::slave_startup_wid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::slave_startup_wid)
{
    ui->setupUi(this);
    curstartup = nullptr;

    QRegExp regx("[A-Fa-f0-9]{1,4}");
    QValidator *valid = new QRegExpValidator(regx,ui->index);
    ui->index->setValidator(valid);


    QRegExp regx_subindex("[A-Fa-f0-9]{1,2}");
    QValidator *valid_subindex = new QRegExpValidator(regx_subindex,ui->index);
    ui->subindex->setValidator(valid_subindex);

    connect(ui->type,&QComboBox::currentTextChanged,this,&slave_startup_wid::typechanged);
    settype(0);

    connect(ui->index,&QLineEdit::editingFinished,this,&slave_startup_wid::setIndex);
    connect(ui->subindex,&QLineEdit::editingFinished,this,&slave_startup_wid::setSubIndex);
    connect(ui->value,&QLineEdit::editingFinished,this,&slave_startup_wid::setValue);
    foreach (QComboBox *cb , this->findChildren<QComboBox *>()){
        cb->installEventFilter(this);
    }
}

slave_startup_wid::~slave_startup_wid(){
    delete ui;
}
void slave_startup_wid::typechanged()
{
    settype(ui->type->currentIndex());
}
void slave_startup_wid::settype(int type)
{
    ui->type->setCurrentIndex(type);

    switch (type) {
    case 0:{//UCHAR
        QRegExp regx("[A-Fa-f0-9]{1,2}");
        QValidator *valid = new QRegExpValidator(regx,ui->index);
        ui->value->setValidator(valid);
    }
        break;
    case 1:{//CHAR
        QRegExp regx("^-?[A-Ea-e0-9][A-Fa-f0-9]");
        QValidator *valid = new QRegExpValidator(regx,ui->index);
        ui->value->setValidator(valid);
    }
        break;
    case 2:{//USHORT
        QRegExp regx("[A-Fa-f0-9]{1,4}");
        QValidator *valid = new QRegExpValidator(regx,ui->index);
        ui->value->setValidator(valid);
    }
        break;
    case 3:{//SHORT
        QRegExp regx("^-?[A-Ea-e0-9][A-Fa-f0-9]{1,3}");
        QValidator *valid = new QRegExpValidator(regx,ui->index);
        ui->value->setValidator(valid);
    }
        break;
    case 4:{//UINT
        QRegExp regx("[A-Fa-f0-9]{1,8}");
        QValidator *valid = new QRegExpValidator(regx,ui->index);
        ui->value->setValidator(valid);
    }
        break;
    case 5:{//INT
        QRegExp regx("^-?[A-Ea-e0-9][A-Fa-f0-9]{1,7}");
        QValidator *valid = new QRegExpValidator(regx,ui->index);
        ui->value->setValidator(valid);
    }
        break;
    case 6:{//ULONG
        QRegExp regx("[A-Fa-f0-9]{1,16}");
        QValidator *valid = new QRegExpValidator(regx,ui->index);
        ui->value->setValidator(valid);
    }
        break;
    case 7:{//LONG
        QRegExp regx("^-?[A-Ea-e0-9][A-Fa-f0-9]{1,15}");
        QValidator *valid = new QRegExpValidator(regx,ui->index);
        ui->value->setValidator(valid);
    }
        break;
    default:
        return;
        break;
    }
    if(curstartup == nullptr){
        return;
    }
    curstartup->type = type;

}
void slave_startup_wid::from(slave_startup *startup)
{
    curstartup = startup;
    ui->info->setText(curstartup->info);
    ui->index->setText(QString::number(curstartup->index,16));
    ui->subindex->setText(QString::number(curstartup->subindex,16));
    ui->value->setText(QString::number(curstartup->value,16));


    settype(curstartup->type);
    //ui->type->setCurrentIndex(curstartup->type);
}
void slave_startup_wid::setIndex()
{
    if(curstartup == nullptr){
        return;
    }
    curstartup->index = ui->index->text().toUInt(nullptr,16);
}
void slave_startup_wid::setSubIndex()
{
    if(curstartup == nullptr){
        return;
    }
    curstartup->subindex = ui->subindex->text().toUInt(nullptr,16);
}
void slave_startup_wid::setValue()
{
    if(curstartup == nullptr){
        return;
    }
    curstartup->value = ui->value->text().toLong(nullptr,16);
}

void slave_startup_wid::on_info_clicked()
{
    if(curstartup == nullptr){
        return;
    }
    QInputDialog infoGet;
    QString info = infoGet.getText(this,"输入注释","",QLineEdit::Normal,ui->info->text());
    if (info == ""){
        return;
    }
    ui->info->setText(info);
    curstartup->info = info;
}

void slave_startup_wid::on_write_clicked()
{

}
bool slave_startup_wid::eventFilter(QObject *obj, QEvent *event)
{
    if(obj->inherits("QComboBox")){
        if(event->type() == QEvent::Wheel) return true;
    }
    //return bCut_Line_wid::eventFilter(obj,event);
    return false;
}
