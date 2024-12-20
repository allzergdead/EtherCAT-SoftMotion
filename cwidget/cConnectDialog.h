#pragma once

#include <QDialog>
#include "ui_cConnectDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class cConnectDialogClass; };
QT_END_NAMESPACE

class cConnectDialog : public QDialog
{
	Q_OBJECT

public:
	cConnectDialog(QWidget *parent = nullptr);
	~cConnectDialog();
	QString getNameStr();

	QString nameStr = "";
public slots:
	void acceptSlot();
	void rejectSlot();
private:
	Ui::cConnectDialogClass *ui;
};
