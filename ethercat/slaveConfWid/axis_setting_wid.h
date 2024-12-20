#ifndef AXIS_SETTING_WID_H
#define AXIS_SETTING_WID_H

#include <QWidget>
#include <QInputDialog>
//#include "../../widgets/widStu.h"
#include "../../../ethercatMaster/etcHeader/etcheader.h"
#include "qt59to514.h"
#include <QMessageBox>
namespace Ui {
class axis_setting_wid;
}

class axis_setting_wid : public QWidget
{
    Q_OBJECT

public:
    explicit axis_setting_wid(QWidget *parent = 0);
    ~axis_setting_wid();
    axis_config *curaxis = nullptr;
    int from_axis(axis_config *axis);
    int to_axis(axis_config *axis);
    bool checkAxis();
private slots:
    void edit_name();
    void edit_enc_ratio();
    void edit_pdoNo();
    void edit_circle_diff();
    void edit_press_enable();
    void click_vaxis();
private:
    Ui::axis_setting_wid *ui;
};

#endif // AXIS_SETTING_WID_H
