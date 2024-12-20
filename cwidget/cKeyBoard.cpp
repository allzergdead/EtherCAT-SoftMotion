#include "cKeyBoard.h"
#include "include/carguments/cBaseVar.h"

cKeyBoard::cKeyBoard(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::cKeyBoardClass())
{
	ui->setupUi(this);
	connect(ui->read, &QPushButton::clicked, this, &cKeyBoard::readSlot);
	connect(ui->del, &QPushButton::clicked, this, &cKeyBoard::delSlot);
	connect(ui->num0, &QPushButton::clicked, this, &cKeyBoard::num0Slot);
	connect(ui->num1, &QPushButton::clicked, this, &cKeyBoard::num1Slot);
	connect(ui->num2, &QPushButton::clicked, this, &cKeyBoard::num2Slot);
	connect(ui->num3, &QPushButton::clicked, this, &cKeyBoard::num3Slot);
	connect(ui->num4, &QPushButton::clicked, this, &cKeyBoard::num4Slot);
	connect(ui->num5, &QPushButton::clicked, this, &cKeyBoard::num5Slot);
	connect(ui->num6, &QPushButton::clicked, this, &cKeyBoard::num6Slot);
	connect(ui->num7, &QPushButton::clicked, this, &cKeyBoard::num7Slot);
	connect(ui->num8, &QPushButton::clicked, this, &cKeyBoard::num8Slot);
	connect(ui->num9, &QPushButton::clicked, this, &cKeyBoard::num9Slot);
	connect(ui->dot, &QPushButton::clicked, this, &cKeyBoard::dotSlot);
	connect(ui->add, &QPushButton::clicked, this, &cKeyBoard::addSlot);
	connect(ui->dec, &QPushButton::clicked, this, &cKeyBoard::decSlot);
	connect(ui->enter, &QPushButton::clicked, this, &cKeyBoard::enterSlot);
	connect(ui->exit, &QPushButton::clicked, this, &cKeyBoard::exitSlot);
	connect(ui->delc, &QPushButton::clicked, this, &cKeyBoard::delcSlot);
	connect(ui->exInput, &QPushButton::clicked, this, &cKeyBoard::exInputSlot);
}

cKeyBoard::~cKeyBoard()
{
	delete ui;
}
void cKeyBoard::show0(QString name)
{
	cBaseVar *var = cBaseVar::findNode(name);
	if (var == nullptr) {
		return;
	}
	ui->name->setText(var->fullName);
	if (var->getInputType() == inputType::C_KEYBOARD_DOT) {
		ui->dot->setEnabled(true);
		ui->dec->setEnabled(true);
		ui->add->setEnabled(true);
	}else if (var->getInputType() == inputType::C_KEYBOARD_NO_DOT_SIGNED) {
		ui->dot->setEnabled(false);
		ui->dec->setEnabled(true);
		ui->add->setEnabled(true);
	}else if (var->getInputType() == inputType::C_KEYBOARD_NO_DOT_UNSIGNED) {
		ui->dot->setEnabled(false);
		ui->dec->setEnabled(false);
		ui->add->setEnabled(false);
	}else {
		return;
	}
	varName = name;
	ui->name->setText(name);
	//ui->value->setText(var->toString());
	ui->value->setText(0);
	exec();
}
void cKeyBoard::readSlot()
{
	cBaseVar* var = cBaseVar::findNode(varName);
	if (var == nullptr) {
		return;
	}
	ui->value->setText(var->toString());
}
void cKeyBoard::delSlot()
{
	ui->value->setText(u8"0");
}
void cKeyBoard::num0Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	if (dvalue == 0) {
		return;
	}
	QString newStr = ui->value->text();
	newStr = newStr + u8"0";
	ui->value->setText(newStr);
}
void cKeyBoard::num1Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr;
	if (dvalue == 0) {
		newStr = u8"";
	}else {
		newStr = ui->value->text();
	}
	newStr = newStr + u8"1";
	ui->value->setText(newStr);
}
void cKeyBoard::num2Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr;
	if (dvalue == 0) {
		newStr = u8"";
	}
	else {
		newStr = ui->value->text();
	}
	newStr = newStr + u8"2";
	ui->value->setText(newStr);
}
void cKeyBoard::num3Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr;
	if (dvalue == 0) {
		newStr = u8"";
	}
	else {
		newStr = ui->value->text();
	}
	newStr = newStr + u8"3";
	ui->value->setText(newStr);
}
void cKeyBoard::num4Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr;
	if (dvalue == 0) {
		newStr = u8"";
	}
	else {
		newStr = ui->value->text();
	}
	newStr = newStr + u8"4";
	ui->value->setText(newStr);
}
void cKeyBoard::num5Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr;
	if (dvalue == 0) {
		newStr = u8"";
	}
	else {
		newStr = ui->value->text();
	}
	newStr = newStr + u8"5";
	ui->value->setText(newStr);
}
void cKeyBoard::num6Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr;
	if (dvalue == 0) {
		newStr = u8"";
	}
	else {
		newStr = ui->value->text();
	}
	newStr = newStr + u8"6";
	ui->value->setText(newStr);
}
void cKeyBoard::num7Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr;
	if (dvalue == 0) {
		newStr = u8"";
	}
	else {
		newStr = ui->value->text();
	}
	newStr = newStr + u8"7";
	ui->value->setText(newStr);
}
void cKeyBoard::num8Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr;
	if (dvalue == 0) {
		newStr = u8"";
	}
	else {
		newStr = ui->value->text();
	}
	newStr = newStr + u8"8";
	ui->value->setText(newStr);
}
void cKeyBoard::num9Slot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr;
	if (dvalue == 0) {
		newStr = u8"";
	}
	else {
		newStr = ui->value->text();
	}
	newStr = newStr + u8"9";
	ui->value->setText(newStr);
}
void cKeyBoard::dotSlot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr = ui->value->text();
	if (newStr.contains(u8".")) {
		return;
	}
	newStr = newStr + u8".";
	ui->value->setText(newStr);
}
void cKeyBoard::addSlot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr = ui->value->text();
	if (!newStr.contains(u8"-")) {
		return;
	}
	newStr.remove(u8"-");
	ui->value->setText(newStr);
}
void cKeyBoard::decSlot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr = ui->value->text();
	if (newStr.contains(u8"-")) {
		return;
	}
	newStr = u8"-" + newStr;
	ui->value->setText(newStr);
}
void cKeyBoard::enterSlot()
{
	cBaseVar* var = cBaseVar::findNode(varName);
	if (var == nullptr) {
		return;
	}
	QString newStr = ui->value->text();
	var->byString(newStr);
	var->varToDB();
	accept();
}

void cKeyBoard::delcSlot()
{
	double dvalue = 0;
	dvalue = ui->value->text().toDouble();
	QString newStr = ui->value->text();
	if (newStr.size() >= 1) {
		newStr.remove(newStr.size() - 1, 1);
		ui->value->setText(newStr);
	}
	
}
void cKeyBoard::exitSlot()
{
	reject();
}
void cKeyBoard::exInputSlot()
{

}
