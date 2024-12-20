#ifndef SLAVE_PDO_WID_H
#define SLAVE_PDO_WID_H

#include <QWidget>
#include <QInputDialog>
//#include "../../widgets/widStu.h"
#include "../../../ethercatMaster/etcHeader/etcheader.h"
#include "qt59to514.h"

#include "qt59to514.h"
namespace Ui {
class slave_pdo_wid;
}

class slave_pdo_wid : public QWidget
{
    Q_OBJECT

public:
    explicit slave_pdo_wid(QWidget *parent = 0);
    ~slave_pdo_wid();
    void from(slave_pdo *pdo);

private slots:
    void index_edit();
    void subindex_edit();
    void size_edit();

    void on_info_clicked();

private:
    Ui::slave_pdo_wid *ui;
    slave_pdo *curpdo;
};

#endif // SLAVE_PDO_WID_H


















