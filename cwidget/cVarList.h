#pragma once

#include <QWidget>
#include "ui_cVarList.h"

QT_BEGIN_NAMESPACE
namespace Ui { class cVarListClass; };
QT_END_NAMESPACE

class cVarList : public QWidget
{
	Q_OBJECT

public:
	cVarList(QWidget *parent = nullptr);
	~cVarList();
	void updateListBy(QString str);
	QMutex vecMtx;
	QVector<QString> names;
	int varPerPage = 50;
	int curPage = 1;
	int maxPage = 1;

	//QString curName = "";
public slots:
	QString getCurName();
	void updateList();
	void tableClear();
	void tableUpdate(int namei, int size);
	void treeClear();

	void pageUp();
	void pageDown();

	//void setConnectSlot();
private:
	Ui::cVarListClass *ui;
};
