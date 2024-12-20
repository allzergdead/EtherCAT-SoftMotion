#ifndef SLAVE_INDEX_WID_H
#define SLAVE_INDEX_WID_H

#include <QWidget>
#include <QInputDialog>
//#include "../../widgets/widStu.h"
#include "../../../ethercatMaster/etcHeader/etcheader.h"
#include "qt59to514.h"
namespace Ui {
class slave_index_wid;
}

class slave_index_wid : public QWidget
{
    Q_OBJECT

public:
    explicit slave_index_wid(QWidget *parent = 0);
    ~slave_index_wid();
    unsigned int getIndex();
    QString getInfo();
    void setIndex(unsigned int size);
    void setInfo(QString info);
    slave_index *index_adr = nullptr;
    void setIndexAdr(slave_index *adr);
private slots:
    void editIndex();
    void on_info_clicked();

private:
    Ui::slave_index_wid *ui;
};

#endif // SLAVE_INDEX_WID_H
