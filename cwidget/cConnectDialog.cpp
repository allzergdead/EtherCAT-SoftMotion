#include "cConnectDialog.h"

cConnectDialog::cConnectDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::cConnectDialogClass())
{
	ui->setupUi(this);
	connect(ui->accept, &QPushButton::clicked, this, &cConnectDialog::acceptSlot);
	connect(ui->reject, &QPushButton::clicked, this, &cConnectDialog::acceptSlot);
}

cConnectDialog::~cConnectDialog()
{
	delete ui;
}

QString cConnectDialog::getNameStr()
{
	exec();
	return nameStr;
}

void cConnectDialog::acceptSlot()
{
	nameStr = ui->varList->getCurName();
	accept();
}
void cConnectDialog::rejectSlot()
{
	nameStr = "";
	reject();
}
