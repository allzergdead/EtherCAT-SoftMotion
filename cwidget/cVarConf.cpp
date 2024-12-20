#include "cVarConf.h"

cVarConf::cVarConf(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::cVarConfClass())
{
	ui->setupUi(this);
	//QString name = "";
	//QString typeName = "";
	//QString info = "";
	//int level = 1000;
	//QString connectName = "";
	//bool connectMark = 0;

	//int accr = 3;
	//int base = 10;


	connect(ui->setConnect, &QPushButton::clicked, this, &cVarConf::setConnectSlot, Qt::QueuedConnection);
	connect(ui->disConnect, &QPushButton::clicked, this, &cVarConf::disConnectSlot, Qt::QueuedConnection);

	newCVar(&name, u8"varConf.name", C_OUTPUT, 0, u8"修改变量名");
	newCVar(&typeName, u8"varConf.typeName", C_OUTPUT, 0, u8"类型名");
	newCVar(&info, u8"varConf.info", C_INPUT, 0, u8"变量说明");
	newCVar(&level, u8"varConf.level", C_INPUT, 0, u8"变量保护等级");
	newCVar(&bindName, u8"varConf.bindName", C_OUTPUT, 0, u8"变量映射名");
	newCVar(&bind, u8"varConf.bind", C_OUTPUT, 0, u8"变量映射标记");
	newCVar(&accr, u8"varConf.accr", C_INPUT, 0, u8"小数点精度");
	newCVar(&base, u8"varConf.base", C_INPUT, 0, u8"显示进制");

	ui->varName->setVar(u8"varConf.name");
	ui->typeName->setVar(u8"varConf.typeName");
	ui->info->setVar(u8"varConf.info");
	ui->level->setVar(u8"varConf.level");
	ui->bindName->setVar(u8"varConf.bindName");
	ui->bind->setVar(u8"varConf.bind");
	ui->accr->setVar(u8"varConf.accr");
	ui->base->setVar(u8"varConf.base");


	connect(ui->update, &QPushButton::clicked, this, &cVarConf::updateVar, Qt::QueuedConnection);
}
void cVarConf::show0(QString iname)
{
	cBaseVar *var = cBaseVar::findNode(iname);
	if (var == nullptr) {
		return;
	}
	name = iname;
	typeName = var->typeName;
	info = var->info;
	level = var->level;
	bindName = var->bindName;
	bind = var->bind;
	accr = var->accr;
	base = var->base;

	ui->varShow->setVar(name);
	hide();
	show();
}
void cVarConf::updateVar()
{
	cBaseVar* var = cBaseVar::findNode(name);
	if (var == nullptr) {
		return;
	}
	var->info = info;
	var->level = level;
	bindName = var->bindName;
	bind = var->bind;
	var->accr = accr;
	var->base = base;

	var->varToDB();
}
cVarConf::~cVarConf()
{
	delete ui;
}
void cVarConf::setConnectSlot()
{
	QString nameStr = cSuperBtn::getName();
	if (nameStr == "") {
		return;
	}

	cBaseVar* var0 = cBaseVar::findNode(name);
	if (var0 == nullptr) {
		return;
	}
	
	cBaseVar* var1 = cBaseVar::findNode(nameStr);
	if (var1 == nullptr) {
		return;
	}

	if (var0->dir == C_INPUT) {
		var0->connect0(var1, var0);
	}

	var0->varToDB();
	updateVar();
}

void cVarConf::disConnectSlot()
{

	cBaseVar* var0 = cBaseVar::findNode(name);
	if (var0 == nullptr) {
		return;
	}
	if (var0->bind == false) {
		return;
	}
	cBaseVar* var1 = cBaseVar::findNode(var0->fullName);

	if (var0->dir == C_INPUT) {
		var0->disconnect0(var1, var0);
	}

	var0->varToDB();
	updateVar();
}







