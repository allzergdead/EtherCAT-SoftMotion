#pragma once

#include <QWidget>
#include <QPushButton>
#include <QMutex>
#include "ui_cSuperBtn.h"
#include "include/carguments/cBaseVar.h"
#include "include/taskmanager/taskmanger.h"

//#include "cKeyBoard.h"
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class cSuperBtnClass; };
QT_END_NAMESPACE

class cSuperBtn : public QPushButton
{
	Q_OBJECT

public:
	cSuperBtn(QWidget *parent = nullptr);
	~cSuperBtn();

	QString name;
	int setVar(QString varname);
	//static int checkShow();
	static QMutex btnMtx;
	static QHash<cSuperBtn*, cSuperBtn*> cBtnHash;
	static TaskManger taskPool;
	static bool inited;
	static QVector<cSuperBtn*> cBtnVec;

	static void* whoShow(void* arg);

	static QDialog *keyBoard;
	static QDialog *varConf;
	static QWidget *varList;

	QString oldStylesheet = "";

	static QDialog* connectDialog;
	static QString getName();

signals:
	void updateSign();
public slots:
	void updateSlot();
	void inoputSlot();
	void CreatMenu(const QPoint&);
	void varConfShow();
	void showVarList();
	
private:
	Ui::cSuperBtnClass *ui;
};
