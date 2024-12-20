#include "cSuperBtn.h"
#include "cKeyBoard.h"
#include "cVarList.h"

#include <QMenu>
#include <QInputDialog>
#include <QFileDialog>

#include "cVarConf.h"
#include "cConnectDialog.h"
QMutex cSuperBtn::btnMtx;
QHash<cSuperBtn*, cSuperBtn*> cSuperBtn::cBtnHash;
TaskManger cSuperBtn::taskPool;
bool cSuperBtn::inited = false;;
QVector<cSuperBtn*> cSuperBtn::cBtnVec;
QDialog *cSuperBtn::keyBoard = nullptr;
QDialog* cSuperBtn::varConf = nullptr;
QWidget* cSuperBtn::varList = nullptr;
QDialog* cSuperBtn::connectDialog = nullptr;

QString normalStyle = R"(
QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
    stop:0 #4d4d4d, stop:1 #5e5e5e); /* 更明显的灰色渐变背景 */
    border: 2px solid #666666; /* 更深的灰色边框 */
    border-radius: 10px; /* 圆角 */
    padding: 2px 2px; /* 内边距 */
    color: #ffffff; /* 白色文字，增加对比 */
    font-size: 16px; /* 字体大小 */
    text-align: center; /* 文本居中 */
    box-shadow: 0 4px 8px rgba(0,0,0,0.25); /* 增强的外部阴影，更强立体感 */
}
)";
QString pressedStyle = R"(
QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
    stop:0 #297229, stop:1 #3cb043); /* 绿色渐变背景 */
    border: 2px solid #256f25; /* 更深绿色边框 */
    color: #ffffff; /* 白色文字 */
    box-shadow: 0 2px 5px rgba(0,0,0,0.3) inset; /* 内凹阴影，强调按下的立体效果 */
    border-radius: 10px; /* 圆角 */
	padding: 2px 2px; /* 适度内边距 */
	text-align: center; /* 文本居中 */
    font-size: 16px; /* 字体大小 */
}
)";
QString disabledStyle = R"(
QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
    stop:0 #636363, stop:1 #747474); /* 更浅的灰色渐变背景 */
    border: 2px solid #7d7d7d; /* 浅灰色边框 */
    color: #aaaaaa; /* 更浅的灰色文本，增强对比度 */
    box-shadow: none; /* 去除阴影，表示不可操作 */
    border-radius: 10px; /* 圆角 */
	padding: 2px 2px; /* 适度内边距 */
	text-align: center; /* 文本居中 */
    font-size: 16px; /* 字体大小 */
}
)";
QString pressedDisabledStyle = R"(
QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
    stop:0 #297229, stop:1 #3cb043); /* 绿色背景的渐变与按下时一致 */
    border: 2px solid #555555; /* 较暗的灰色边框，表示不可操作 */
    color: #ffffff; /* 白色文本 */
    box-shadow: none; /* 去除阴影，强调不可操作状态 */
    border-radius: 10px; /* 圆角 */
	padding: 2px 2px; /* 适度内边距 */
	text-align: center; /* 文本居中 */
    font-size: 16px; /* 字体大小 */
}
)";
QString normalInputStyle = R"(
QPushButton {
    background-color: #333333; /* 暗灰色背景 */
    border: 1px solid #555555; /* 较暗的灰色边框 */
    border-radius: 6px; /* 柔和的圆角，类似输入框 */
    padding: 2px 2px; /* 适度内边距 */
    color: #cccccc; /* 浅灰色文本，增加可读性 */
    font-size: 16px; /* 字体大小适中 */
    text-align: left; /* 文本左对齐 */
    box-shadow: 0 1px 3px rgba(0,0,0,0.1); /* 轻微外部阴影，增加层次感 */
}
)";

QString disabledInputStyle = R"(
QPushButton {
    background-color: #2e2e2e; /* 更暗的灰色背景，模拟不可输入的输入框 */
    border: 1px solid #4d4d4d; /* 与背景协调的边框颜色 */
	padding: 2px 2px; /* 适度内边距 */
    color: #999999; /* 更淡的灰色文本，降低视觉焦点 */
    border-radius: 6px; /* 保持圆角一致 */
    text-align: left; /* 文本左对齐 */
    font-size: 16px; /* 维持一致的字体大小 */
    box-shadow: none; /* 去除阴影，强调不可用状态的平面感 */
}
)";

QString menuStyle = R"(
QMenu {
    background-color: #3a3a3a; /* 暗色背景 */
    color: #eeeeee; /* 浅灰色文字，提高可读性 */
    border: 1px solid #505050; /* 边框颜色与背景协调 */
    font-size: 14px; /* 适中的字体大小，保持风格一致性 */

}

QMenu::item {
    background-color: transparent; /* 默认背景透明 */
    padding: 5px 20px; /* 菜单项内边距，足够空间确保可读性 */
}

QMenu::item:selected {
    background-color: #4b4b4b; /* 选中项背景颜色稍亮，提高区分度 */
    color: #ffffff; /* 选中项的文字为白色，增加对比 */
}

QMenu::item:disabled {
    color: #666666; /* 不可用项的文字颜色更暗，减少视觉焦点 */
}

QMenu::separator {
    height: 2px; /* 分隔线高度 */
    background: #505050; /* 分隔线颜色与边框相同 */
    margin-left: 10px; /* 分隔线左侧空间 */
    margin-right: 10px; /* 分隔线右侧空间 */
}
)";
cSuperBtn::cSuperBtn(QWidget *parent)
	: QPushButton(parent)
	, ui(new Ui::cSuperBtnClass())
{
	ui->setupUi(this);
	if (!inited) {
		taskPool.init(5);
		keyBoard = new cKeyBoard(nullptr);
		
		taskPool.addTask(whoShow, nullptr);
		inited = true;
		varConf = new cVarConf(nullptr);
		varList = new cVarList(nullptr);
	}
	btnMtx.lock();
	cBtnHash.insert(this, this);
	btnMtx.unlock();

	connect(this, &cSuperBtn::updateSign, this, &cSuperBtn::updateSlot,Qt::QueuedConnection);
	connect(this, &cSuperBtn::clicked, this, &cSuperBtn::inoputSlot, Qt::QueuedConnection);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(CreatMenu(const QPoint&)));
}
int cSuperBtn::setVar(QString varname)
{
	cBaseVar* var = cBaseVar::findNode(varname);
	if (var == nullptr) {
		return - 1;
	}
	name = varname;
	setToolTip(var->info);
	return 0;
}
cSuperBtn::~cSuperBtn()
{
	btnMtx.lock();
	if (cBtnHash.find(this) != cBtnHash.end()) {
		cBtnHash.erase(cBtnHash.find(this));
	}
	btnMtx.unlock();
	delete ui;
}
void cSuperBtn::CreatMenu(const QPoint& pt) {
	QMenu menu(this);
	menu.setStyleSheet(menuStyle);
	menu.addAction(u8"变量配置", this, SLOT(varConfShow()));
	menu.addAction(u8"显示变量列表", this, SLOT(showVarList()));

	QPoint pt0(pt.x(), pt.y());
	menu.exec(QCursor::pos());
}
void cSuperBtn::varConfShow()
{
	cBaseVar* var = cBaseVar::findNode(name);
	if (var == nullptr) {
		return;
	}
	if (!var->checkLevel()) {
		return;
	}
	((cVarConf *)varConf)->show0(name);
}
void cSuperBtn::updateSlot()
{
	if (visibleRegion().isNull()) { return; }
	if (visibleRegion().isEmpty()) { return; }
	cBaseVar* var = cBaseVar::findNode(name);
	if (var == nullptr) {
		return;
	}
	bool check = var->checkLevel() && (var->dir == C_INPUT) && (!var->bind);
	bool ok = false;
	if (var->is<bool>()) {
		bool value = var->to<bool>(&ok);
		QString newStyle;
		if (check && !value) {
			newStyle = normalStyle;
		}
		if (check && value) {
			newStyle = pressedStyle;
		}
		if (!check && !value) {
			newStyle = disabledStyle;
		}
		if (!check && value) {
			newStyle = pressedDisabledStyle;
		}

		if (newStyle != oldStylesheet) {
			setStyleSheet(newStyle);
			oldStylesheet = newStyle;
		}
	}else {
		QString value = var->toString();
		QString newStyle;
		if (check) { 
			newStyle = normalInputStyle;
		}else {
			newStyle = disabledInputStyle;
		}
		if (newStyle != oldStylesheet) {
			setStyleSheet(newStyle);
			oldStylesheet = newStyle;
		}
		setText(value);
	}
}
void cSuperBtn::inoputSlot()
{
	cBaseVar* var = cBaseVar::findNode(name);
	if (var == nullptr) {
		return;
	}
	bool check = var->checkLevel() && (var->dir == C_INPUT) && (!var->bind);
	if (!check) {
		return;
	}
	bool ok = false;
	inputType type0 = var->getInputType();
	if (type0 == inputType::C_BTN) {
		if (var->is<bool>()) {
			bool value = var->to<bool>(&ok);
			if (value) {
				var->byString(u8"0");
				var->varToDB();
			}else {
				var->byString(u8"1");
				var->varToDB();
			}
		}else {
			bool ok = false;
			QString value = var->toString();
			double valued = value.toDouble(&ok);
			if (ok) {
				if (valued == 0) {
					var->byString(u8"1");
					var->varToDB();
				}
				else {
					var->byString(u8"0");
					var->varToDB();
				}
			}
		}
	}else if (type0 == inputType::C_KEYBOARD_DOT || type0 == inputType::C_KEYBOARD_NO_DOT_SIGNED || type0 == inputType::C_KEYBOARD_NO_DOT_UNSIGNED) {
		//keyBoard->show0(var);
		cKeyBoard* keyboard0 = (cKeyBoard*)keyBoard;
		keyboard0->show0(name);
	}else if (type0 == inputType::C_USER_SET){
		if (var->func->userSetInput != nullptr) {
			var->func->userSetInput(var);
		}
	}else if (type0 == inputType::C_OPEN_FILE_DIR) {
		QString filePath = QFileDialog::getOpenFileName();
		if (filePath != "") {
			var->byString(filePath);
			var->varToDB();
		}
	}else if (type0 == inputType::C_SAVE_FILE_DIR) {
		QString filePath = QFileDialog::getSaveFileName();
		if (filePath != "") {
			var->byString(filePath);
			var->varToDB();
		}
	}else if (type0 == inputType::C_STRING_INPUT) {
		bool ok = false;;
		QString str = QInputDialog::getText(this, var->fullName, "", QLineEdit::Normal, var->toString(), &ok);
		if (ok) {
			var->byString(str);
			var->varToDB();
		}
	}
	else {
	}
}

void* cSuperBtn::whoShow(void* arg)
{
	cSuperBtn* self = (cSuperBtn*)arg;
	while (true) {

		btnMtx.lock();
		cBtnVec.clear();
		auto it = cBtnHash.begin();
		for (it = cBtnHash.begin(); it != cBtnHash.end(); it++) {
			//if (self->visibleRegion().isNull()) { continue; }
			//if (self->visibleRegion().isEmpty()) { continue; }
			if (it.value()->visibleRegion().isNull()) { continue; }
			if (it.value()->visibleRegion().isEmpty()) { continue; }
			cBtnVec.append(it.value());
		}
		btnMtx.unlock();
		for (int i = 0; i < cBtnVec.size(); i++) {
			emit cBtnVec[i]->updateSign();
		}

		Sleep(300);
	}
}


void cSuperBtn::showVarList()
{
	if (varList == nullptr) {
		return;
	}
	cVarList* list = (cVarList*)varList;
	list->hide();
	list->show();
}


QString cSuperBtn::getName()
{
	if (connectDialog == nullptr) {
		connectDialog = new cConnectDialog();
	}
	cConnectDialog* tempDialog = (cConnectDialog*)connectDialog;
	return tempDialog->getNameStr();
	//connectDialog->exec();
	
}

