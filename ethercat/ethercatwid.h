#ifndef ETHERCATWID_H
#define ETHERCATWID_H

#include <QWidget>
//#include "../widgets/baseform.h"
#include "../../ethercatMaster/etcHeader/etcheader.h"
#include "../../ethercatMaster/etcHeader/jmc_axis_cmd.h"
#include "slaveConfWid/slave_assign_wid.h"
#include "slaveConfWid/slave_index_wid.h"
#include "slaveConfWid/slave_startup_wid.h"
#include "slaveConfWid/axis_setting_wid.h"
//#include "../widgets/cbutton.h"
#include "slave_conf_dg.h"
#include "slaveConfWid/slave_pdo_wid.h"
#include <QTcpSocket>
//#include "../dev/claseruserdev.h"
#include <QSharedMemory>
#include <QTreeWidget>
#include <QFileDialog>
#include <QDir>
#include <qtablewidget.h>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>

#include "../axiswid.h"
extern class claserUserDev laserDev;
extern QHash<QString ,axis_ref*> axis_hash;

extern QString last_json_error;

namespace Ui {
class ethercatwid;
}

class ethercatwid : public QWidget
{
    Q_OBJECT

public:
    explicit ethercatwid(QWidget *parent = nullptr);
    ~ethercatwid();
    virtual int init(QString sname);
    cec_shm_pdo nullshm;
    cec_shm_pdo *ecm;
    int econnect;
    virtual int reconfdata(void *data);
    QTcpSocket sock;
    QString shm_name = "ETCAX";
    QSharedMemory shm;

    QTimer _1ms_timer;
    QTimer update_timer;

    static QVector<ethercatwid *> members;
    static int fun_connectETCMaster(QStringList arg);
    bool hasSlave(QString slaveName);
    unsigned int etc_state = 0;
    unsigned long heart = 0;
    int box_test = 0;
signals:
    void updateETC(cec_shm_pdo *ecm);
private slots:
    void _1ms_slot();
    void slaveTable_menu();
    void assign_menu();
    void pdo_menu();
    void start_menu();
    void axis_menu();

    void pdos_update();

    void on_pushButton_clicked();
    void on_creat_JSON_clicked();
//    void new_slave(int index);
//    void del_slave(int index);
//    void change_name_slave(int index);
    void slaveTable_curchange();
    void assignTable_curchange();
    void indexTable_curchange();
    void pdoTable_curchange();
    void startup_fresh();

    void axis_fresh();

    void new_pdoassign_head();
    void new_pdoassign_tail();
    void new_pdoassign_addPre();
    void new_pdoassign_addNext();
    void del_pdoassign_cur();


    void new_pdo_head();
    void new_pdo_tail();
    void new_pdo_addPre();
    void new_pdo_addNext();
    void del_pdo_cur();

    void new_axis_head();
    void new_axis_tail();
    void new_axis_addPre();
    void new_axis_addNext();
    void del_axis_cur();


    void new_startup_head();
    void new_startup_tail();
    void new_startup_addPre();
    void new_startup_addNext();
    void del_startup_cur();

    void new_pdoassign(int index);
    void del_pdoassign(int index);


    void new_slave_item(int index);
    void del_slave_item(int index);


    void new_axis_item(int index);
    void del_axis_item(int index);

    void new_pdo(int index);
    void del_pdo(int index);

    void new_startup(int index);
    void del_startup(int index);

    void change_name_slave_item(int index);


//    void update_slave_table();

    void slave_insert_head();
    void slave_push();
    void slave_addPre();
    void slave_addNext();
    void slave_delete();
    void slave_change_name();
    void on_saveAs_JSON_clicked();
    
    void on_open_JSON_clicked();

    void alias_edit();
    void pos_edit();
    void pid_edit();
    void vid_edit();
    void on_getconf_JSON_clicked();

    void slave_change_to_axisslave();

    void Copy();
    void Paste();
    void on_auto_config_clicked();

    void on_updatAxis_clicked();

private:

    Ui::ethercatwid *ui;
    int eport;
    int tcp_cmd(int cmd);
    QString shmName;

    //QJsonDocument jsonFile;
    QJsonDocument jsonFile;
    QJsonObject jsonObj;

    timespec t_old;

    uint32_t output_enable = 0xffffffff;

    QString getSlaveName();
    //slave_config slaves_config[MAX_SLAVE];
    QHash<QString,slave_config> slaves_hash;
    QHash<QString,axis_config> axes_hash;
    QVector<axis_config> axes_vec;

    bool hasSlaveName(QString name);

    void assign_show(slave_config *conf,int smNo);
    void assign_show(slave_assign *assign);
    void pdo_show();
    //void pdo_subindex_show();

    int updateAssign(slave_assign *assign);
    int assignsave(slave_assign *assign);

    slave_config *get_Slave();
    slave_assign *get_assign();
    slave_index *get_index();
    slave_pdo *get_pdo();

    int JSON_Save(QString path);
    int JSON_Read(QString path);

    slave_config *CopyAera;
    //slave_startup *get_startup();

};

#endif // ETHERCATWID_H
