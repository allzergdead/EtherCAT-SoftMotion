#include "slave_pdo_wid.h"
#include "ui_slave_pdo_wid.h"

slave_pdo_wid::slave_pdo_wid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::slave_pdo_wid)
{
    ui->setupUi(this);

    curpdo = nullptr;

    QRegExp regx("[A-Fa-f0-9]{1,4}");
    QValidator *valid = new QRegExpValidator(regx,ui->index);
    ui->index->setValidator(valid);


    QRegExp regx_subindex("[A-Fa-f0-9]{1,2}");
    QValidator *valid_subindex = new QRegExpValidator(regx_subindex,ui->index);
    ui->subindex->setValidator(valid_subindex);

    QRegExp regx_size("[0-9]{1,4}");
    QValidator *valid_size = new QRegExpValidator(regx_size,ui->index);
    ui->size->setValidator(valid_size);


    connect(ui->index,&QLineEdit::editingFinished,this,&slave_pdo_wid::index_edit);
    connect(ui->subindex,&QLineEdit::editingFinished,this,&slave_pdo_wid::subindex_edit);
    connect(ui->size,&QLineEdit::editingFinished,this,&slave_pdo_wid::size_edit);


}

slave_pdo_wid::~slave_pdo_wid()
{
    delete ui;
}
void slave_pdo_wid::index_edit()
{
    if(curpdo == nullptr){
        return;
    }
    curpdo->index = ui->index->text().toUInt(nullptr,16);
}
void slave_pdo_wid::subindex_edit()
{
    if(curpdo == nullptr){
        return;
    }
    curpdo->sub_index = ui->subindex->text().toUInt(nullptr,16);
}
void slave_pdo_wid::size_edit()
{
    if(curpdo == nullptr){
        return;
    }
    curpdo->size = ui->size->text().toUInt();
}
void slave_pdo_wid::from(slave_pdo *pdo)
{
    if(pdo == nullptr){
        return;
    }
    curpdo = pdo;
    ui->index->setText(QString::number(curpdo->index,16));
    ui->subindex->setText(QString::number(curpdo->sub_index,16));
    ui->size->setText(QString::number(curpdo->size));
    ui->info->setText(curpdo->info);
}

void slave_pdo_wid::on_info_clicked()
{
    if(curpdo == nullptr){
        return;
    }
    QInputDialog infoGet;
    QString info = infoGet.getText(this,"输入注释","",QLineEdit::Normal,ui->info->text());
    if (info == ""){
        return;
    }
    ui->info->setText(info);
    curpdo->info = info;
}
