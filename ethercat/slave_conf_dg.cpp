#include "slave_conf_dg.h"
#include "ui_slave_conf_dg.h"

slave_conf_dg::slave_conf_dg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::slave_conf_dg)
{
    ui->setupUi(this);
}

slave_conf_dg::~slave_conf_dg()
{
    delete ui;
}
