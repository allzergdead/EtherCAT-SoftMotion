#ifndef SLAVE_CONF_DG_H
#define SLAVE_CONF_DG_H

#include <QDialog>

#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Ui {
class slave_conf_dg;
}

class slave_conf_dg : public QDialog
{
    Q_OBJECT

public:
    explicit slave_conf_dg(QWidget *parent = nullptr);
    ~slave_conf_dg();

private:
    Ui::slave_conf_dg *ui;
};

#endif // SLAVE_CONF_DG_H
