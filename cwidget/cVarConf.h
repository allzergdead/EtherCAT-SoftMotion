#pragma once

#include <QDialog>
#include "ui_cVarConf.h"

QT_BEGIN_NAMESPACE
namespace Ui { class cVarConfClass; };
QT_END_NAMESPACE

class cVarConf : public QDialog
{
	Q_OBJECT

public:
	cVarConf(QWidget *parent = nullptr);
	~cVarConf();
	void show0(QString iname);

	QString name = "";
	QString typeName = "";
	QString info = "";
	int level = 1000;
	QString bindName = "";
	bool bind = 0;

	int accr = 3;
	int base = 10;
public slots:
	void updateVar();
	void setConnectSlot();
	void disConnectSlot();
private:
	Ui::cVarConfClass *ui;
};
