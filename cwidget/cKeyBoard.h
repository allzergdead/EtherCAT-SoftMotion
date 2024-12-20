#pragma once

#include <QDialog>
#include "ui_cKeyBoard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class cKeyBoardClass; };
QT_END_NAMESPACE

class cKeyBoard : public QDialog
{
	Q_OBJECT

public:
	cKeyBoard(QWidget *parent = nullptr);
	~cKeyBoard();
	QString varName = "";
public slots:
	void readSlot();
	void delSlot();
	void num0Slot();
	void num1Slot();
	void num2Slot();
	void num3Slot();
	void num4Slot();
	void num5Slot();
	void num6Slot();
	void num7Slot();
	void num8Slot();
	void num9Slot();
	void dotSlot();
	void addSlot();
	void decSlot();
	void enterSlot();
	void delcSlot();
	void exitSlot();
	void exInputSlot();

	void show0(QString name);
private:
	Ui::cKeyBoardClass *ui;
};
