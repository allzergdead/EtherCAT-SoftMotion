#include "slave_index_wid.h"
#include "ui_slave_index_wid.h"
//#define QRegularExpressionValidator QRegExpValidator

slave_index_wid::slave_index_wid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::slave_index_wid)
{
    ui->setupUi(this);
    index_adr = nullptr;
    QRegExp regx("[A-Fa-f0-9]{1,4}");
    QValidator *valid = new QRegExpValidator(regx,ui->index);
    ui->index->setValidator(valid);
    connect(ui->index,&QLineEdit::editingFinished,this,&slave_index_wid::editIndex);
}

slave_index_wid::~slave_index_wid()
{
    delete ui;
}
void slave_index_wid::setIndexAdr(slave_index *adr)
{
    index_adr = adr;
    setInfo(index_adr->info);
    setIndex(index_adr->index);
}
void slave_index_wid::editIndex()
{
    if (index_adr == nullptr){
        insertLog(-1,"ETC配置","程序错误，未关联index变量");
        return;
    }
    index_adr->index = ui->index->text().toInt(nullptr,16);
    insertLog(0,"ETC配置",ui->info->text() + " -> " + ui->index->text());
}

unsigned int slave_index_wid::getIndex()
{
    return ui->index->text().toUInt(nullptr,16);
}

void slave_index_wid::setIndex(unsigned int size)
{
    ui->index->setText(QString::number(size,16));
}


QString slave_index_wid::getInfo()
{
    return ui->info->text();
}

void slave_index_wid::setInfo(QString info)
{
    ui->info->setText(info);
}

void slave_index_wid::on_info_clicked()
{
    if (index_adr == nullptr){
        insertLog(-1,"ETC配置","程序错误，未关联index变量");
        return;
    }
    QInputDialog infoGet;
    QString info = infoGet.getText(this,"输入注释","",QLineEdit::Normal,ui->info->text());
    if (info == ""){
        return;
    }
    ui->info->setText(info);
    index_adr->info = info;
}
