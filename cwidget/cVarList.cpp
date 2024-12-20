#include "cVarList.h"
#include "include/carguments/cBaseVar.h"
cVarList::cVarList(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::cVarListClass())
{
	ui->setupUi(this);
	connect(ui->sourch, &QPushButton::clicked, this, &cVarList::updateList, Qt::QueuedConnection);

	connect(ui->pageUp, &QPushButton::clicked, this, &cVarList::pageUp, Qt::QueuedConnection);
	connect(ui->pageDown, &QPushButton::clicked, this, &cVarList::pageDown, Qt::QueuedConnection);

	

	newCVar(&varPerPage, u8"static.cVarList.varPerPage", C_INPUT, 0, u8"单页显示数量");
	ui->varPerPage->setVar(u8"static.cVarList.varPerPage");

	QStringList header;
	header << u8"变 量" << u8"类 型" << u8" 值 " << u8"备注";
	ui->varTable->setColumnCount(4);
	ui->varTable->setHorizontalHeaderLabels(header);
	ui->varTable->horizontalHeader()->setVisible(true);
	ui->varTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

cVarList::~cVarList()
{
	delete ui;
}
void cVarList::tableClear()
{
	for (int i = 0; i < ui->varTable->rowCount(); i++) {
		cSuperBtn* btn = (cSuperBtn*)ui->varTable->cellWidget(i, 2);
		btn->deleteLater();
	}
	ui->varTable->setRowCount(0);

}
void cVarList::tableUpdate(int namei,int size)
{
	ui->varTable->setRowCount(size);
	ui->varTable->setColumnCount(4);
	ui->varTable->setSortingEnabled(false);

	for (int i = namei; i < namei + size; i++) {
		if (i > names.size()) {
			break;
		}
		cBaseVar* var = cBaseVar::findNode(names[i]);
		if (var == nullptr) {
			continue;
		}
		cSuperBtn* btn = new cSuperBtn();
		
		btn->setVar(var->fullName);
		QTableWidgetItem* fullName = new QTableWidgetItem();
		QTableWidgetItem* typeName = new QTableWidgetItem();
		QTableWidgetItem* varItem = new QTableWidgetItem();
		QTableWidgetItem* info = new QTableWidgetItem();
		ui->varTable->setItem(i - namei, 0, fullName);
		ui->varTable->setItem(i - namei, 1, typeName);
		ui->varTable->setItem(i - namei, 2, varItem);
		ui->varTable->setItem(i - namei, 3, info);

		ui->varTable->item(i - namei, 0)->setText(var->fullName);
		ui->varTable->item(i - namei, 1)->setText(var->typeName);
		//ui->varTable->item(i - namei, 2)->setText(var->fullName);
		ui->varTable->setCellWidget(i - namei, 2, btn);
		ui->varTable->item(i - namei, 3)->setText(var->info);

	}
	ui->varTable->setSortingEnabled(true);
	QString pageTips;
	pageTips = QString::number(curPage) + "/" + QString::number(maxPage);
	ui->pageInfo->setText(pageTips);
}
void cVarList::treeClear()
{
}
void cVarList::pageUp()
{
	maxPage = names.size() / varPerPage + 1;
	if (curPage - 1 < 1) {
		return;
	}
	curPage = curPage - 1;

	int rows = 0;
	if (curPage < maxPage) {
		rows = varPerPage;
	}
	else {
		rows = names.size() % varPerPage;
	}
	tableUpdate((curPage - 1) * varPerPage, rows);

}
void cVarList::pageDown()
{
	maxPage = names.size() / varPerPage + 1;
	if (curPage + 1 > maxPage) {
		return;
	}
	curPage = curPage + 1;


	int rows = 0;
	if (curPage < maxPage) {
		rows = varPerPage;
	}
	else {
		rows = names.size() % varPerPage;
	}
	tableUpdate((curPage - 1) * varPerPage, rows);
}
void cVarList::updateListBy(QString str)
{
	vecMtx.lock();
	names.clear();
	names = cBaseVar::collectNames(str);
	vecMtx.unlock();
	tableClear();
	curPage = 1;
	maxPage = names.size() / varPerPage + 1;

	int rows = 0;
	if (curPage < maxPage) {
		rows = varPerPage;
	}else {
		rows = names.size() % varPerPage;
	}
	tableUpdate((curPage - 1) * varPerPage, rows);



}
void cVarList::updateList()
{
	updateListBy(ui->sourchEdit->text());
}

QString cVarList::getCurName()
{
	int curRow = ui->varTable->currentRow();
	if (curRow < 0) {
		return "";
	}

	QString curNameStr = ui->varTable->item(curRow, 0)->text();
	return curNameStr;

}














