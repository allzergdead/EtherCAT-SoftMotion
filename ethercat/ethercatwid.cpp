#include "ethercatwid.h"
#include "ui_ethercatwid.h"
#include <qtablewidget.h>

#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC + \
        (B).tv_nsec - (A).tv_nsec)

#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
#define NSEC_PER_SEC (1000000000L)
QVector<ethercatwid *> ethercatwid::members;
ethercatwid::ethercatwid(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ethercatwid)
{
    ui->setupUi(this);

    eport = 9900;
    econnect = 0;

    QTimer *myTimer = new QTimer(this);
    myTimer->start(1000);
    connect(myTimer, &QTimer::timeout, [=](){
        if (CopyAera != nullptr){
            qDebug() << CopyAera->name;
        }
        if (myTimer->isActive() == false){
            myTimer->start(1000);
        }
//        cBaseVar::errorList.xEtherCATError = (ecm->etc_state != 8);
//        if(cBaseVar::errorList.xEtherCATError){
//             showBtn.setStyleSheet("background: rgb(250,50,50);\n  border: 0px solid #333333;\n   ");
//        }else{
//            showBtn.setStyleSheet("background: rgb(50,250,50);\n  border: 0px solid #333333;\n   ");
//        }
//        case errorstop:
//            showBtn.setStyleSheet("background: rgb(250,50,50);\n  border: 0px solid #333333;\n   ");
//        break;
//        case standstill:
//            showBtn.setStyleSheet("background: rgb(50,250,50);\n  border: 0px solid #333333;\n   ");
//        break;
    });

    QTimer *etcLogTimer = new QTimer(this);
    etcLogTimer->setSingleShot(false);
    etcLogTimer->start(100);
    connect(etcLogTimer, &QTimer::timeout, [=](){
//        if(ecm != nullptr){
//            ecm->CRTtools_counter++;
//        }

    });




    connect(ui->slaveTable,&QTableWidget::cellClicked,this,[=](){
        slaveTable_curchange();
        //pdos_update();
    });

    connect(ui->assignTable,&QTableWidget::cellClicked,this,[=](){
        assignTable_curchange();
        //pdos_update();
    });

    connect(ui->indexTable,&QTableWidget::cellClicked,this,[=](){
        indexTable_curchange();
        //pdos_update();
    });

    connect(ui->PDOTable,&QTableWidget::cellClicked,this,[=](){
        pdoTable_curchange();
        //pdos_update();
    });
    connect(ui->slaveTable,&QTableWidget::customContextMenuRequested,this,&ethercatwid::slaveTable_menu);

    connect(ui->indexTable,&QTableWidget::customContextMenuRequested,this,&ethercatwid::assign_menu);

    connect(ui->PDOTable,&QTableWidget::customContextMenuRequested,this,&ethercatwid::pdo_menu);

    connect(ui->startupTable,&QTableWidget::customContextMenuRequested,this,&ethercatwid::start_menu);

    connect(ui->axisTable,&QTableWidget::customContextMenuRequested,this,&ethercatwid::axis_menu);

    //connect(ui->syncTable,&QTableWidget::currentItemChanged,this,&ethercatwid::pdos_update);

    //connect(ui->syncTable,&QTableWidget::cellClicked,this,&ethercatwid::pdos_update);

    //ui->syncTable->clicked();

    members.append(this);

    QStringList argList;
    argList.append("主站编号(int)");



    QStringList slave_table_header;
    slave_table_header << "从站名";
    //ui->logList->setColumnCount(4);
    //ui->logList->setHorizontalHeaderLabels(header);
    ui->slaveTable->setColumnCount(1);
    ui->slaveTable->setHorizontalHeaderLabels(slave_table_header);
    ui->slaveTable->setColumnWidth(0,300);
    ui->slaveTable->horizontalHeader()->setVisible(true);
    ui->slaveTable->setEditTriggers(QAbstractItemView::NoEditTriggers);


    QRegExp regx("[A-Fa-f0-9]{1,8}");
    QValidator *valid = new QRegExpValidator(regx,this);
    ui->alias->setValidator(valid);
    ui->position->setValidator(valid);
    ui->pid->setValidator(valid);
    ui->vid->setValidator(valid);

    connect(ui->alias,&QLineEdit::editingFinished,this,&ethercatwid::alias_edit);
    //connect(ui->position,&QLineEdit::editingFinished,this,&ethercatwid::pos_edit);
    connect(ui->pid,&QLineEdit::editingFinished,this,&ethercatwid::pid_edit);
    connect(ui->vid,&QLineEdit::editingFinished,this,&ethercatwid::vid_edit);
}

ethercatwid::~ethercatwid()
{
    //shm.detach();
    sock.close();
    delete ui;
}
void ethercatwid::slaveTable_curchange()
{

    ui->assignTable->clear();

    QTableWidgetItem *item0 = new QTableWidgetItem();
    QTableWidgetItem *item1 = new QTableWidgetItem();
    QTableWidgetItem *item2 = new QTableWidgetItem();
    QTableWidgetItem *item3 = new QTableWidgetItem();

    ui->assignTable->setRowCount(4);
    ui->assignTable->setColumnCount(1);
    if (ui->slaveTable->currentRow() < 0 || ui->slaveTable->currentRow() > ui->slaveTable->rowCount()){
        return;
    }
    QStringList header;
    slave_config *config = get_Slave();
    if (config == nullptr){
        return;
    }

    header <<    config->name + " SMBox Info";
    ui->assignTable->setHorizontalHeaderLabels(header);
    ui->assignTable->horizontalHeader()->show();
    ui->assignTable->setColumnWidth(0,300);

    ui->assignTable->setItem(0,0,item0);
    ui->assignTable->setItem(1,0,item1);
    ui->assignTable->setItem(2,0,item2);
    ui->assignTable->setItem(3,0,item3);

    slave_assign_wid *sm0 = new slave_assign_wid();
    slave_assign_wid *sm1 = new slave_assign_wid();
    slave_assign_wid *sm2 = new slave_assign_wid();
    slave_assign_wid *sm3 = new slave_assign_wid();


    sm0->from(&config->MbxOut);
    sm1->from(&config->MbxIn);
    sm2->from(&config->output);
    sm3->from(&config->input);

    ui->assignTable->setCellWidget(0,0,sm0);
    ui->assignTable->setCellWidget(1,0,sm1);
    ui->assignTable->setCellWidget(2,0,sm2);
    ui->assignTable->setCellWidget(3,0,sm3);


    ui->indexTable->clear();
    ui->indexTable->setRowCount(0);
    ui->indexTable->horizontalHeader()->hide();

    ui->PDOTable->clear();
    ui->PDOTable->setRowCount(0);
    ui->PDOTable->horizontalHeader()->hide();

    config->position = ui->slaveTable->currentRow();
    ui->alias->setText(QString::number(config->alias,16));
    ui->position->setText(QString::number(config->position,16));
    ui->pid->setText(QString::number(config->pid,16));
    ui->vid->setText(QString::number(config->vid,16));


    startup_fresh();
}
void ethercatwid::startup_fresh()
{


    slave_config *slave = get_Slave();
    if(slave == nullptr){
        ui->startupTable->clear();
        ui->startupTable->setRowCount(0);
        return;
    }
    ui->startupTable->clear();
    QStringList header;
    header << slave->name + ", startup";
    ui->startupTable->setColumnCount(1);
    ui->startupTable->setHorizontalHeaderLabels(header);
    ui->startupTable->horizontalHeader()->show();
    ui->startupTable->setColumnWidth(0,650);
    ui->startupTable->horizontalHeader()->setVisible(true);

    ui->startupTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->startupTable->setRowCount(slave->startup.size());
    for(int i = 0;i < slave->startup.size();i++){
        slave_startup_wid *wid = new slave_startup_wid();
        QTableWidgetItem *item = new QTableWidgetItem();

        ui->startupTable->setItem(i,0,item);
        ui->startupTable->setCellWidget(i,0,wid);

        wid->from(&slave->startup[i]);
    }
}


void ethercatwid::axis_fresh()
{
    ui->axisTable->clear();
    QStringList header;
    header <<    " Axis Config table";
    ui->axisTable->setColumnCount(1);
    ui->axisTable->setHorizontalHeaderLabels(header);
    ui->axisTable->horizontalHeader()->show();
    ui->axisTable->setColumnWidth(0,365);
    ui->axisTable->horizontalHeader()->setVisible(true);

    ui->axisTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->axisTable->setRowCount(axes_vec.size());
    for(int i = 0;i < axes_vec.size();i++){
        QTableWidgetItem *item = new QTableWidgetItem(axes_vec[i].name);
        axis_setting_wid *wid = new axis_setting_wid();

        ui->axisTable->setItem(i,0,item);
        wid->from_axis(&axes_vec[i]);
        ui->axisTable->setCellWidget(i,0,wid);
        ui->axisTable->setRowHeight(i,wid->height());
        wid->show();
    }
}
void ethercatwid::assignTable_curchange()
{
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        //insertLog(-1,"ETC","error  - 1");
        return;
    }
    slave_assign *assign = get_assign();
    if(assign == nullptr){
        //insertLog(-1,"ETC","error  - 2");
        return;
    }
    //insertLog(0,"ETC","show" + slave->name + " " + assign->info);
    ui->indexTable->setColumnCount(1);
    //ui->slaveTable->setHorizontalHeaderLabels(slave_table_header);
    ui->indexTable->setColumnWidth(0,300);
    ui->indexTable->horizontalHeader()->setVisible(true);
    ui->indexTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //ui->indexTable->clear();

    QStringList header;
    header << slave->name + "," + assign->info;
    ui->indexTable->setHorizontalHeaderLabels(header);
    ui->indexTable->horizontalHeader()->show();
    ui->indexTable->setRowCount(assign->index.size());
    for(int i = 0; i < assign->index.size();i++){
        slave_index_wid *index_wid = new slave_index_wid();
        QTableWidgetItem *item = new QTableWidgetItem();

        ui->indexTable->setItem(i,0,item);
        ui->indexTable->setCellWidget(i,0,index_wid);
        index_wid->setIndex(assign->index[i].index);
        index_wid->setInfo(assign->index[i].info);
        index_wid->setIndexAdr(&assign->index[i]);

    }
    ui->PDOTable->clear();
    ui->PDOTable->setRowCount(0);
    ui->PDOTable->horizontalHeader()->hide();
    //insertLog(0,"ETC",header[0]);
}
void ethercatwid::indexTable_curchange()
{
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        return;
    }
    slave_assign *assign = get_assign();
    if(assign == nullptr){
        return;
    }
    slave_index *index = get_index();
    if(index == nullptr){
        return;
    }
    pdo_show();
}
void ethercatwid::pdoTable_curchange()
{

}
//void slave_insert_head();
//void slave_push();
//void slave_addPre();
//void slave_addNext();
//void slave_delete();
void ethercatwid::slaveTable_menu()
{
    QMenu menu;
    menu.addAction("在头部添加从站",this,SLOT(slave_insert_head()));
    menu.addAction("在尾部添加从站",this,SLOT(slave_push()));
    menu.addAction("在选中项前添加从站",this,SLOT(slave_addPre()));
    menu.addAction("在选中项后添加从站",this,SLOT(slave_addNext()));
    menu.addAction("选中项配置为DS402伺服",this,SLOT(slave_change_to_axisslave()));
    menu.addAction("删除从站",this,SLOT(slave_delete()));
    menu.addAction("更改从站名",this,SLOT(slave_change_name()));
    menu.addAction("Copy", this, SLOT(Copy()));
    menu.addAction("Paste", this, SLOT(Paste()));
    menu.exec(QCursor::pos());
}
void ethercatwid::assign_menu()
{


    if (get_assign() == nullptr){
        return;
    }


    QMenu menu;
    menu.addAction("在头部添加PDO_index",this,SLOT(new_pdoassign_head()));
    menu.addAction("在尾部添加PDO_index",this,SLOT(new_pdoassign_tail()));
    menu.addAction("在选中项前添加PDO_index",this,SLOT(new_pdoassign_addPre()));
    menu.addAction("在选中项后添加PDO_index",this,SLOT(new_pdoassign_addNext()));
    menu.addAction("删除PDO_index",this,SLOT(del_pdoassign_cur()));
    //menu.addAction("更改从站名",this,SLOT(slave_change_name()));
    menu.exec(QCursor::pos());

}
void ethercatwid::pdo_menu()
{
    if (get_index() == nullptr){
        return;
    }

    QMenu menu;
    menu.addAction("在头部添加PDO参数",this,SLOT(new_pdo_head()));
    menu.addAction("在尾部添加PDO参数",this,SLOT(new_pdo_tail()));
    menu.addAction("在选中项前添加PDO参数",this,SLOT(new_pdo_addPre()));
    menu.addAction("在选中项后添加PDO参数",this,SLOT(new_pdo_addNext()));
    menu.addAction("删除PDOIndex",this,SLOT(del_pdo_cur()));
    //menu.addAction("更改从站名",this,SLOT(slave_change_name()));
    menu.exec(QCursor::pos());
}
void ethercatwid::start_menu()
{
    if (get_Slave() == nullptr){
        return;
    }

    QMenu menu;
    menu.addAction("在头部添加启动参数",this,SLOT(new_startup_head()));
    menu.addAction("在尾部添加启动参数",this,SLOT(new_startup_tail()));
    menu.addAction("在选中项前添加启动参数",this,SLOT(new_startup_addPre()));
    menu.addAction("在选中项后添加启动参数",this,SLOT(new_startup_addNext()));
    menu.addAction("删除启动参数",this,SLOT(del_startup_cur()));
    //menu.addAction("更改从站名",this,SLOT(slave_change_name()));
    menu.exec(QCursor::pos());
}
void ethercatwid::axis_menu()
{
    QMenu menu;
    menu.addAction("在头部添加Axis",this,SLOT(new_axis_head()));
    menu.addAction("在尾部添加Axis",this,SLOT(new_axis_tail()));
    menu.addAction("在选中项前添加Axis",this,SLOT(new_axis_addPre()));
    menu.addAction("在选中项后添加Axis",this,SLOT(new_axis_addNext()));
    menu.addAction("删除Axis",this,SLOT(del_axis_cur()));
    //menu.addAction("更改从站名",this,SLOT(slave_change_name()));
    menu.exec(QCursor::pos());
}
void ethercatwid::_1ms_slot()
{
    return;
}
int ethercatwid::init(QString sname)
{

    return 0;
}

int ethercatwid::reconfdata(void *data)
{

    return 0;
}
void setCMD(char *mem,uint cmd)
{
    memset(mem,0,64);
    uint *ehead = (uint *)mem;
    *ehead = 0x6a6a6a6a;
    uint *ecmd = (uint *)&mem[4];
    *ecmd = cmd;
}
struct retinfo{
    int lenth;
    int ret;
    char *data;
};
int getCLenth(QTcpSocket *obj,retinfo *info)
{

    char head[12];
    int reflen = obj->read(head,12);
    if (reflen != 12){
        insertLog(-1,"警告","数据头长度不为12");
        return -1;
    }
    uint *ehead = (uint *)head;
    if (*ehead != 0x6a6a6a6a){
        insertLog(-1,"警告","数据头校验失败");
        return -2;
    }
    //int *len = (int *)&head[8];
    info->lenth = *(int *)&head[4];
    info->ret = *(int *)&head[8];
    return 0;
}
int getCRef(QTcpSocket *obj,retinfo *info)
{
    //char head[12];

    info->data = (char *)malloc(info->lenth);
    int reflen = obj->read(info->data,info->lenth);
    if (reflen != info->lenth){
        insertLog(-1,"警告","数据头长度不符合数据头信息");
        return -1;
    }
    return 0;
}
int ethercatwid::tcp_cmd(int cmd)
{
    switch (cmd) {
    case 0:{// 链接
        shm.setKey(shm_name);
        if(shm.isAttached()){
            shm.attach();
        }else{
            bool ret = shm.attach();
            if (!ret){
                ret = shm.create(sizeof(axis_data) * MAX_AXIS);
            }
            if (!ret){
                insertLog(-1,"通知","共享内存创建失败，无法使用精确采样");
            }
        }
        if (sock.state() == QAbstractSocket::ConnectedState){
            insertLog(-1,"通知","已经连接");
            return 0;
        }

        sock.connectToHost("127.0.0.1",eport);
        bool ret = sock.waitForConnected(3000);

        if (ret){
            insertLog(0,"通知","连接成功");
        }else{
            insertLog(-1,"警告","连接失败");
        }

        return 0;
    }
    break;
    case 1:{// 启动
        if (sock.state() != QAbstractSocket::ConnectedState){
            insertLog(-1,"警告","主战控制接口未链接");
            return 0;
        }
        char cmdbyte[64];
        setCMD(cmdbyte,0);
        sock.write(cmdbyte,64);
        sock.flush();

        bool ret = sock.waitForReadyRead(3000);
        if (ret){

        }else{
            insertLog(-1,"警告","主站未回应");
            return 0;
        }
        retinfo info;
        ret = getCLenth(&sock,&info);
        if (ret != 0){
            return 0;
        }
        ret = getCRef(&sock,&info);
        if (ret != 0){
            return 0;
        }
        qDebug() << info.lenth << info.ret;
        std::string str = (char *)info.data;
        shmName = QString::fromStdString(str);
        free(info.data);

        sleep(10);
        //ecm = (cec_shm_pdo *)laserDev.etcinit();
        if (ecm == nullptr){
            insertLog(-1,"警告","主战控制接-MEMERROR");
            return 0;
        }
        sleep(2);
        qDebug() << " axis num : " << ecm->axis_Num;
        axis_hash.clear();
        for(int i = 0;i < ecm->axis_Num;i++){
            std::string str(ecm->axis[i].name);
            QString axisname = QString::fromStdString(str);
            axis_hash[axisname] = &ecm->axis[i];
        }
        qDebug() << " axis conf num : " << axis_hash.size();
        reconfdata(ecm);
        emit updateETC(ecm);
        //sleep(10);
        return 0;
    }
    break;
    }
    return 0;
}



void ethercatwid::on_pushButton_clicked()
{
    return;
    ec_master *master = ecrt_open_master(0);
    if(master == nullptr){
        insertLog(0,"ETC_INFO","request master error");
    }
    //uint32_t output_enable = 0xffffffff;
    uint32_t abort_code;
    size_t size = 4;
    int ref = ecrt_master_sdo_download(master,0,0x60fe,2,(uint8_t *)&output_enable,4,&abort_code);
    qDebug() << "ecrt_master_sdo_download" << ref << output_enable;
    ref = ecrt_master_sdo_upload(master,0,0x60fe,2,(uint8_t *)&output_enable,4,&size,&abort_code);
    qDebug() << "ecrt_master_sdo_upload" << ref << output_enable;
    output_enable--;
}

int ethercatwid::fun_connectETCMaster(QStringList arg)
{
    if(arg.size() != 1){
        insertLog(-1,"fun_connectETCMaster","参数个数不匹配");
        return -1;
    }
    int num = 0;
    int ref = members[num]->tcp_cmd(0);
    ref = members[num]->tcp_cmd(1);
    sleep(10);
    return ref;
}











void ethercatwid::slave_insert_head()
{
    new_slave_item(0);
}
void ethercatwid::slave_push()
{

    new_slave_item(ui->slaveTable->rowCount());
}
void ethercatwid::slave_addPre()
{
    new_slave_item(ui->slaveTable->currentRow());
}
void ethercatwid::slave_addNext()
{
    new_slave_item(ui->slaveTable->currentRow() + 1);
}
void ethercatwid::Copy(){
    if (ui->slaveTable->rowCount() < 0 || ui->slaveTable->currentRow() < 0){
        return;
    }

    CopyAera = get_Slave();

}
void ethercatwid::Paste(){

    if (CopyAera == nullptr){
        return;
    }
//    int count = 0; // the num of same SlaveConfig's name
//    for (int i = 0; i < ui->slaveTable->rowCount(); i++){
//        if (ui->slaveTable->selectedItems()[i]->text().endsWith(")")){
//            count++;
//        }
//    }
    QString name = getSlaveName();//slave->name +"(" + QString::number(count) + ")";
    CopyAera->name = name;
    if(name == "#)"){
        return;
    }
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setText(name);

    ui->slaveTable->insertRow(ui->slaveTable->currentRow() + 1);
    ui->slaveTable->setItem(ui->slaveTable->currentRow() + 1,0,item);


    slaves_hash.insert(name,*CopyAera);
}
void ethercatwid::slave_delete()
{
    del_slave_item(ui->slaveTable->currentRow());
    ui->assignTable->clear();
    ui->assignTable->setRowCount(0);
    ui->assignTable->horizontalHeader()->hide();

    ui->indexTable->clear();
    ui->indexTable->setRowCount(0);
    ui->indexTable->horizontalHeader()->hide();

}
void ethercatwid::slave_change_name()
{
    change_name_slave_item(ui->slaveTable->currentRow());
}

void ethercatwid::new_pdoassign_head()
{
    new_pdoassign(0);
}
void ethercatwid::new_pdoassign_tail()
{
    new_pdoassign(ui->indexTable->rowCount());
}
void ethercatwid::new_pdoassign_addPre()
{
    new_pdoassign(ui->indexTable->currentRow());
}
void ethercatwid::new_pdoassign_addNext()
{
    new_pdoassign(ui->indexTable->currentRow() + 1);
}
void ethercatwid::del_pdoassign_cur()
{
    del_pdoassign(ui->indexTable->currentRow());
}
int ethercatwid::updateAssign(slave_assign *assign)
{

}
bool ethercatwid::hasSlave(QString slaveName)
{
    if (slaves_hash.find(slaveName) == slaves_hash.end()){
        insertLog(-1,"ETC配置","该从站已被删除或者已经更名");
        return false;
    }
    return true;
}
void ethercatwid::new_pdoassign(int index)
{

    slave_assign *assign;
    assign = get_assign();
    if(assign == nullptr){
        return;
    }

    //ui->indexTable->setCellWidget(index,0,wid);

    slave_index pdoIndex;
    pdoIndex.info = "info";
    pdoIndex.index = 0;
    assign->index.insert(index,pdoIndex);
    assignTable_curchange();
    sleep(0.5);// 等待append真实完成，不然输入参数会崩溃
    return;
}
void ethercatwid::del_pdoassign(int index)
{
    slave_assign *assign;
    assign = get_assign();
    if(assign == nullptr){
        return;
    }
    assign->index.erase(assign->index.begin() + index);
    ui->indexTable->removeRow(index);
    return;
}
void ethercatwid::pdos_update()
{
    //qDebug() << "update";
    int slaveNo = ui->slaveTable->currentRow();
    if(slaveNo < 0 || slaveNo >= MAX_SLAVE){
        return;
    }
    QString name = ui->slaveTable->item(slaveNo,0)->text();
    int smNo = ui->assignTable->currentRow();
    if(smNo < 0 || smNo >= 4){
        return;
    }

    if (!hasSlave(name)){
        return;
    }
    switch (smNo) {
    case 0:
        assign_show(&slaves_hash[name],smNo);
        break;
    case 1:
        assign_show(&slaves_hash[name],smNo);
        break;
    case 2:
        assign_show(&slaves_hash[name],smNo);
        break;
    case 3:
        assign_show(&slaves_hash[name],smNo);
        break;
    default:
        return;
        ui->indexTable->clear();
        break;
    }


}
void ethercatwid::assign_show(slave_assign *assign)
{
    ui->indexTable->clear();
    ui->indexTable->setColumnWidth(0,300);
    ui->indexTable->horizontalHeader()->setVisible(true);
    ui->indexTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //ui->assignTable->setHorizontalHeaderLabels(PDOAssign_header);
    ui->indexTable->horizontalHeader()->show();

    ui->indexTable->setRowCount(assign->index.size());

    for(int i = 0; i < assign->index.size();i++){
        slave_index_wid *index_wid = new slave_index_wid();
        QTableWidgetItem *item = new QTableWidgetItem();

        ui->indexTable->setItem(i,0,item);
        ui->indexTable->setCellWidget(i,0,index_wid);
        index_wid->setIndex(assign->index[i].index);
        index_wid->setInfo(assign->index[i].info);
        index_wid->setIndexAdr(&assign->index[i]);

    }
}
void ethercatwid::assign_show(slave_config *conf,int smNo)
{
    ui->indexTable->clear();
    slave_assign *assign;
    QStringList PDOAssign_header;
    //slave_table_header << "从站名";
    //ui->logList->setColumnCount(4);
    //ui->logList->setHorizontalHeaderLabels(header);
    ui->indexTable->setColumnCount(1);
    //ui->slaveTable->setHorizontalHeaderLabels(slave_table_header);
    ui->indexTable->setColumnWidth(0,300);
    ui->indexTable->horizontalHeader()->setVisible(true);
    ui->indexTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    switch (smNo) {
    case 0:
        PDOAssign_header << conf->name + ",MbxOut";
        assign = &conf->MbxOut;
        break;
    case 1:
        PDOAssign_header << conf->name + ",MbxIn";
        assign = &conf->MbxIn;
        break;
    case 2:
        PDOAssign_header << conf->name + ",output";
        assign = &conf->output;
        break;
    case 3:
        PDOAssign_header << conf->name + ",input";
        assign = &conf->input;
        break;
    default:
        return;
        ui->indexTable->clear();
        break;
    }

    ui->indexTable->setHorizontalHeaderLabels(PDOAssign_header);
    ui->indexTable->horizontalHeader()->show();

    ui->indexTable->setRowCount(assign->index.size());

    for(int i = 0; i < assign->index.size();i++){
        slave_index_wid *index_wid = new slave_index_wid();
        QTableWidgetItem *item = new QTableWidgetItem();

        ui->indexTable->setItem(i,0,item);
        ui->indexTable->setCellWidget(i,0,index_wid);
        index_wid->setIndex(assign->index[i].index);
        index_wid->setInfo(assign->index[i].info);
        index_wid->setIndexAdr(&assign->index[i]);

    }

}

void ethercatwid::new_slave_item(int index)
{
    QString name = getSlaveName();
    if(name == "#"){
        return ;
    }
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setText(name);

    ui->slaveTable->insertRow(index);
    ui->slaveTable->setItem(index,0,item);
    slave_config slave;

    slave.name = name;
    slave.input.index.clear();
    slave.input.info = "input";
    slave.output.index.clear();
    slave.output.info = "output";
    slave.MbxIn.index.clear();
    slave.MbxIn.info = "MbxIn";
    slave.MbxOut.index.clear();
    slave.MbxOut.info = "MbxOut";

    //slaves_hash[name] = slave;
    slaves_hash.insert(name,slave);
}
void ethercatwid::del_slave_item(int index)
{
    QString oldName = ui->slaveTable->item(index,0)->text();
    slaves_hash.erase(slaves_hash.find(oldName));
    ui->slaveTable->removeRow(index);
    slaveTable_curchange();
}
void ethercatwid::change_name_slave_item(int index)
{
    QString name = getSlaveName();
    if(name == "#" || name == ""){
        return ;
    }
    QString oldName = ui->slaveTable->item(index,0)->text();
    //slaves_hash[name] = slaves_hash[oldName];
    slaves_hash.insert(name,slaves_hash[oldName]);
    slaves_hash[name].name = name;
    slaves_hash.erase(slaves_hash.find(oldName));
    QTableWidgetItem *item = ui->slaveTable->item(index,0);
    item->setText(name);
    slaveTable_curchange();

}
/*void ethercatwid::new_slave(int index)
{

    QJsonValueRef jsSlaves_ref = jsonObj["jsSlaves"];
    if (jsSlaves_ref.isNull() || !jsSlaves_ref.isArray()){
        insertLog(-1,"ETC配置","JSON结构未建立");
        return;
    }
    QJsonArray jsSlaves = jsSlaves_ref.toArray();
    QJsonObject slave;
    QString name = getSlaveName();
    if(name == "#"){
        return ;
    }
    slave.insert("name",name);
    //slave.insert("no",jsSlaves.size());
    jsSlaves.insert(index,slave);
    jsonObj["jsSlaves"] = jsSlaves;
    update_slave_table();
}
void ethercatwid::del_slave(int index)
{
    QJsonValueRef jsSlaves_ref = jsonObj["jsSlaves"];
    if (jsSlaves_ref.isNull() || !jsSlaves_ref.isArray()){
        insertLog(-1,"ETC配置","JSON结构未建立");
        return;
    }
    QJsonArray jsSlaves = jsSlaves_ref.toArray();
    jsSlaves.erase(jsSlaves.begin() + index);
    jsonObj["jsSlaves"] = jsSlaves;
    update_slave_table();


}
void ethercatwid::change_name_slave(int index)
{
    QJsonValueRef jsSlaves_ref = jsonObj["jsSlaves"];
    if (jsSlaves_ref.isNull() || !jsSlaves_ref.isArray()){
        return;
    }
    QJsonArray jsSlaves = jsSlaves_ref.toArray();

    if (jsSlaves[index].isNull() || !jsSlaves[index].isObject()){
        insertLog(-1,"ETC配置","不存在该从站");
        return;
    }

    //QJsonObject slave = slaveRef.toObject();
    QString name = getSlaveName();
    if(name == "#"){
        return ;
    }
    //QJsonValueRef = jsSlaves[index];
    //update_slave_table();

}
void ethercatwid::update_slave_table()
{
    QJsonValueRef jsSlaves_ref = jsonObj["jsSlaves"];
    if (jsSlaves_ref.isNull() || !jsSlaves_ref.isArray()){
        return;
    }


    ui->slaveTable->clear();
    QStringList slave_table_header;
    slave_table_header << "从站名";
    //ui->logList->setColumnCount(4);
    //ui->logList->setHorizontalHeaderLabels(header);
    ui->slaveTable->setColumnCount(1);
    ui->slaveTable->setHorizontalHeaderLabels(slave_table_header);
    ui->slaveTable->setColumnWidth(0,300);
    ui->slaveTable->horizontalHeader()->setVisible(true);
    ui->slaveTable->horizontalHeader()->show();
    ui->slaveTable->setEditTriggers(QAbstractItemView::NoEditTriggers);


    QJsonArray jsSlaves = jsSlaves_ref.toArray();
    ui->slaveTable->setRowCount(jsSlaves.size());
    for (int i = 0; i < jsSlaves.size();i++){
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(jsSlaves[i].toObject()["name"].toString());
        ui->slaveTable->setItem(i,0,item);
    }
}*/
void ethercatwid::new_pdo_head()
{
    new_pdo(0);
}
void ethercatwid::new_pdo_tail()
{
    new_pdo(ui->PDOTable->rowCount());
}
void ethercatwid::new_pdo_addPre()
{
    new_pdo(ui->PDOTable->currentRow());
}
void ethercatwid::new_pdo_addNext()
{
    new_pdo(ui->PDOTable->currentRow() + 1);
}
void ethercatwid::del_pdo_cur()
{
    del_pdo(ui->PDOTable->currentRow());
}
void ethercatwid::new_pdo(int index)
{
    slave_index *slaveIndex = get_index();
    if(slaveIndex == nullptr){
        return;
    }
    slave_pdo pdo;
    pdo.index = 0;
    pdo.sub_index = 0;
    pdo.info = "info";
    pdo.size = 1;
    slaveIndex->pdo.insert(index,pdo);
    pdo_show();
}
void ethercatwid::del_pdo(int index)
{
    slave_index *slaveIndex = get_index();
    if(slaveIndex == nullptr){
        return;
    }
    slaveIndex->pdo.erase(slaveIndex->pdo.begin() +  index);
    pdo_show();
}
void ethercatwid::pdo_show()
{
    slave_config *config = get_Slave();
    if(config == nullptr){
        //insertLog(-1,"ETC","error  - 1");
        return;
    }
    slave_assign *assign = get_assign();
    if(assign == nullptr){
        //insertLog(-1,"ETC","error  - 2");
        return;
    }
    slave_index *slaveIndex = get_index();
    if(slaveIndex == nullptr){
        //insertLog(-1,"ETC","error  - 3");
        return;
    }
    ui->PDOTable->clear();
    //insertLog(0,"ETC","show pdo");
    ui->PDOTable->setColumnCount(1);
    //ui->slaveTable->setHorizontalHeaderLabels(slave_table_header);
    ui->PDOTable->horizontalHeader()->setVisible(true);
    ui->PDOTable->setEditTriggers(QAbstractItemView::NoEditTriggers);


    QStringList header;
    header << config->name + "," + assign->info + "," + slaveIndex->info + " -  0x" + QString::number(slaveIndex->index,16);
    ui->PDOTable->setHorizontalHeaderLabels(header);
    ui->PDOTable->horizontalHeader()->show();
    ui->PDOTable->setColumnWidth(0,650);
    ui->PDOTable->setRowCount(slaveIndex->pdo.size());
    for(int i = 0;i < slaveIndex->pdo.size();i++){
        slave_pdo_wid *wid = new slave_pdo_wid();
        QTableWidgetItem *item = new QTableWidgetItem();
        wid->from(&slaveIndex->pdo[i]);
        ui->PDOTable->setItem(i,0,item);
        ui->PDOTable->setCellWidget(i,0,wid);
    }

}

void ethercatwid::new_startup_head()
{
    new_startup(0);
}
void ethercatwid::new_startup_tail()
{
    new_startup(ui->startupTable->rowCount());
}
void ethercatwid::new_startup_addPre()
{
    new_startup(ui->startupTable->currentRow());
}
void ethercatwid::new_startup_addNext()
{
    new_startup(ui->startupTable->currentRow() + 1);
}
void ethercatwid::del_startup_cur()
{
    del_startup(ui->startupTable->currentRow());
}

void ethercatwid::new_startup(int index)
{
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        return;
    }
    slave_startup startup;
    startup.index = 0;
    startup.size = 0;
    startup.type = 0;
    startup.subindex = 0;
    startup.value = 0;

    slave->startup.insert(index,startup);

    startup_fresh();
}
void ethercatwid::del_startup(int index)
{
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        return;
    }
    slave->startup.erase(slave->startup.begin() + index);
    startup_fresh();
}
slave_pdo conf_pdo(uint index,uint sub_index,uint size,QString info)
{
    slave_pdo tempPdo;
    tempPdo.index = index;
    tempPdo.sub_index = sub_index;
    tempPdo.size = size;
    tempPdo.info = info;
    return tempPdo;
}
void ethercatwid::slave_change_to_axisslave()
{
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        return;
    }
    slave->MbxOut.index.clear();
    slave->MbxIn.index.clear();
    slave->output.index.clear();
    slave->input.index.clear();
//    /*TxPdo 0x1600*/
//    slave_conf.info = (ec_pdo_entry_info_t *)malloc(sumNum * sizeof(ec_pdo_entry_info_t));
//    slave_conf.info[0] =  {0x6040, 0x00, 16};slave_conf.dir[0] = 1;      //    unsigned int ctrl_word;
//    slave_conf.info[1] =  {0x6060, 0x00, 8 };slave_conf.dir[1] = 1;       //    unsigned int operation_mode;
//    slave_conf.info[2] =  {0x607A, 0x00, 32};slave_conf.dir[2] = 1;       //    unsigned int target_position;
//    slave_conf.info[3] =  {0x6071, 0x00, 16};slave_conf.dir[3] = 1;       //    unsigned int target_torque;
//    slave_conf.info[4] =  {0x60FE, 0x01, 32};slave_conf.dir[4] = 1;       //    unsigned int digital_outputs;
//    /*RxPdo 0x1A00*/
//    slave_conf.info[5] =  {0x603F, 0x00, 16};slave_conf.dir[5] = 0;        //    unsigned int error_code;
//    slave_conf.info[6] =  {0x6041, 0x00, 16};slave_conf.dir[6] = 0;       //    unsigned int status_word;
//    slave_conf.info[7] =  {0x6061, 0x00, 8 };slave_conf.dir[7] = 0;       //    unsigned int modes_of_operation_display;
//    slave_conf.info[8] =  {0x6064, 0x00, 32};slave_conf.dir[8] = 0;       //    unsigned int position_actual_value;
//    slave_conf.info[9] =  {0x6077, 0x00, 16};slave_conf.dir[9] = 0;       //    unsigned int torque_actual_velue;
//    slave_conf.info[10] = {0x60F4, 0x00, 32};slave_conf.dir[10] = 0;       //    unsigned int following_error_actual_value;
//    slave_conf.info[11] = {0x60FD, 0x00, 32};slave_conf.dir[11] = 0;      //    unsigned int digital_inputs;
    slave_index out_1600;
    out_1600.index = 0x1600;
    out_1600.info = "servo_1600";
    out_1600.pdo.append(conf_pdo(0x6040,0x00,2,"ctrl_word"));
    out_1600.pdo.append(conf_pdo(0x6060,0x00,1,"operation_mode"));
    out_1600.pdo.append(conf_pdo(0x607A,0x00,4,"target_position"));
    out_1600.pdo.append(conf_pdo(0x6071,0x00,2,"target_torque"));
    out_1600.pdo.append(conf_pdo(0x60FE,0x01,4,"digital_outputs"));
    slave->output.index.append(out_1600);

    slave_index in_1A00;
    in_1A00.index = 0x1A00;
    in_1A00.info = "servo_1A00";
    in_1A00.pdo.append(conf_pdo(0x603F,0x00,2,"error_code"));
    in_1A00.pdo.append(conf_pdo(0x6041,0x00,2,"status_word"));
    in_1A00.pdo.append(conf_pdo(0x6061,0x00,1,"modes_of_operation_display"));
    in_1A00.pdo.append(conf_pdo(0x6064,0x00,4,"position_actual_value"));
    in_1A00.pdo.append(conf_pdo(0x6077,0x00,2,"torque_actual_velue"));
    in_1A00.pdo.append(conf_pdo(0x60F4,0x00,4,"following_error_actual_value"));
    in_1A00.pdo.append(conf_pdo(0x60FD,0x00,4,"digital_inputs"));
    slave->input.index.append(in_1A00);

    insertLog(1,slave->name,"切换为伺服配置成功");
}





void ethercatwid::alias_edit()
{
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        return;
    }
    slave->alias = ui->alias->text().toUInt(nullptr,16);
}
void ethercatwid::pos_edit()
{
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        return;
    }
    slave->position = ui->position->text().toUInt(nullptr,16);
}
void ethercatwid::pid_edit()
{
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        return;
    }
    slave->pid = ui->pid->text().toUInt(nullptr,16);
}
void ethercatwid::vid_edit()
{
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        return;
    }
    slave->vid = ui->vid->text().toUInt(nullptr,16);
}
void ethercatwid::new_axis_head()
{
    new_axis_item(0);
}
void ethercatwid::new_axis_tail()
{
    new_axis_item(ui->axisTable->rowCount());
}
void ethercatwid::new_axis_addPre()
{
     new_axis_item(ui->axisTable->currentRow());
}
void ethercatwid::new_axis_addNext()
{
    new_axis_item(ui->axisTable->currentRow() + 1);
}
void ethercatwid::del_axis_cur()
{

}
void ethercatwid::new_axis_item(int index)
{
    axis_config axis;
    QInputDialog dialog;
    bool ok;
    axis.name = dialog.getText(this,"axis name","请输入",QLineEdit::Normal,"",&ok);
    if(!ok){
        return;
    }
    axis.enc_ratio = 1048576;
    axis.pdoNo = -1;
    axis.circle_diff = 1.0;
    axes_vec.insert(index,axis);
    insertLog(1,"轴编辑器",axis.name + " 添加成功");
    axis_fresh();
}
void ethercatwid::del_axis_item(int index)
{

}



QString ethercatwid::getSlaveName()
{
    bool ok;
    QInputDialog dilog;
    QString name = dilog.getText(this,"请输入被从站名","从站名",QLineEdit::Normal,"",&ok);
    if(!ok){
        return "#";
    }

    if (name == ""){
        return "#";
    }
    if (hasSlaveName(name)){
        insertLog(-1,"ETC配置","从站名已被占用，请更换");
        return "#";
    }
    return name;
}


bool ethercatwid::hasSlaveName(QString name)
{

    return false;
}


slave_config *ethercatwid::get_Slave()
{
    if (ui->slaveTable->currentRow() < 0 || ui->slaveTable->currentRow() > ui->slaveTable->rowCount()){
        return nullptr;
    }
    QString name = ui->slaveTable->item(ui->slaveTable->currentRow(),0)->text();
    if (!hasSlave(name)){
        return nullptr;
    }
    return &slaves_hash[name];
}
slave_assign *ethercatwid::get_assign()
{
/*    slave_config *slave = get_Slave();
    if (slave == nullptr){
        return nullptr;
    }
    if (ui->syncTable->currentRow() < 0 || ui->syncTable->currentRow() >= ui->syncTable->rowCount()){
        return nullptr;
    }
    switch (ui->syncTable->currentRow()) {
    case 0:
        return &slave->MbxOut;
        break;
    case 1:
        return &slave->MbxIn;
        break;
    case 2:
        return &slave->output;
        break;
    case 3:
        return &slave->input;
        break;
    default:
        return nullptr;
        break;
    }
*/
    slave_config *slave = get_Slave();
    if(slave == nullptr){
        return nullptr;
    }
    slave_assign *assign;
    if(ui->assignTable->currentRow() < 0 || ui->assignTable->currentRow() >= ui->assignTable->rowCount()){
        return nullptr;
    }
    //slave_assign_wid *wid = (slave_assign_wid *)ui->assignTable->cellWidget(ui->assignTable->currentRow(),0);

    //Qstring smName = ui->assignTable->cellWidget(ui->assignTable->currentRow(),0);
    if (ui->assignTable->currentRow() == 0){
        assign = &slave->MbxOut;
    }else if(ui->assignTable->currentRow() == 1){
        assign = &slave->MbxIn;
    }else if(ui->assignTable->currentRow() == 2){
        assign = &slave->output;
    }else if(ui->assignTable->currentRow() == 3){
        assign = &slave->input;
    }else{
        return nullptr;
    }
    return assign;
}
slave_index *ethercatwid::get_index()
{
    slave_assign *assign = get_assign();
    if (assign == nullptr){
        return nullptr;
    }
    if(ui->indexTable->currentRow() < 0 || ui->indexTable->currentRow() >= ui->indexTable->rowCount()){
        return nullptr;
    }
    return &assign->index[ui->indexTable->currentRow()];
}
slave_pdo *ethercatwid::get_pdo()
{
    return nullptr;
}




void ethercatwid::on_saveAs_JSON_clicked()
{
    QFileDialog fileDialog;
    QString path = fileDialog.getSaveFileName(this);
    //QMessageBox::information(this,"警告",path);
 
    JSON_Save(path);
}

void ethercatwid::on_open_JSON_clicked()
{
    QFileDialog fileDialog;
    QString path = fileDialog.getOpenFileName(this);
    //QMessageBox::information(this,"警告",path);
    JSON_Read(path);

}
int check_axis_name(QVector<axis_config> *vec,QString name)
{
    int count = 0;
    for(int i = 0;i < vec->size();i++){
        if (name == (*vec)[i].name){
            count++;
        }
    }
    return count;
}
int ethercatwid::JSON_Save(QString path)
{
    QFile file(path);
    bool ret = file.open(QIODevice::WriteOnly);
    if (!ret){
        insertLog(-1,"警告","创建文件失败");
        return -1;
    }
    QVector<slave_config> slave_vec;
    for(int i  = 0;i < ui->slaveTable->rowCount();i++){
        QString name = ui->slaveTable->item(i,0)->text();
        if(!hasSlave(name)){
            return -1;
        }
        slaves_hash[name].position = i;
        slave_vec.append(slaves_hash[name]);
    }


    QJsonObject vec_obj;
    ret = etc_vector2json("",&slave_vec,&vec_obj);
    if (ret != 0){
        insertLog(-1,"警告","创建JSON文件失败 - ETCPDO" + last_json_error);
        return -1;
    }
//    for(int i = 0;i < axes_vec->size();i++){
//        int count = check_axis_name(&axes_vec,axes_vec[i].name);
//        if(count != 1){
//            insertLog(-1,axes_vec[i].name,"伺服驱动重名，请检查");
//            return -1;
//        }
//    }
    ret = axis_vector2json("",&axes_vec,&vec_obj);
    if (ret != 0){
        insertLog(-1,"警告","创建JSON文件失败 - AXIS" + last_json_error);
        return -1;
    }
    QJsonDocument jsonDoc;
    jsonDoc.setObject(vec_obj);
    //jsonDoc.
    file.write(jsonDoc.toJson(QJsonDocument::Compact));
    file.flush();
    file.close();
    insertLog(1,"ETC配置器","保存ETC从站配置完成");
    return 0;
}
int ethercatwid::JSON_Read(QString path)
{
    QFile file(path);
    bool bret = file.open(QIODevice::ReadOnly);
    if (!bret){
        insertLog(-1,"警告","打开文件失败");
        return -1;
    }
    QMessageBox::StandardButton result = QMessageBox::warning(this,"警告","请保存已修改内容，重新读取将会覆盖，是否读取？"
                                                              ,QMessageBox::Yes | QMessageBox::No);
    if(result == QMessageBox::No){
        return -1;
    }
    file.seek(0);
    QJsonDocument jsonDoc;
    QJsonParseError error;
    QByteArray fileMem = file.readAll();
    QString fileStr = QString::fromStdString(fileMem.toStdString());


    insertLog(0,"JSON","文件大小 " + QString::number(fileMem.size()));
    jsonDoc = QJsonDocument::fromJson(fileMem,&error);

    QJsonObject vec_obj = jsonDoc.object();
    QVector<slave_config> slave_vec;
    int ret = etc_json2vector("",&vec_obj,&slave_vec);
    if(ret != 0){
        insertLog(-1,"警告","解析JSON文件失败 - ETCPDO" + last_json_error);
        return -1;
    }
    axes_vec.clear();
    axis_fresh();
    ret = axis_json2vector("",&vec_obj,&axes_vec);
    if(ret != 0){
        insertLog(-1,"警告","解析JSON文件失败 - AXIS" + last_json_error);
        return -1;
    }

    file.close();
    ui->slaveTable->clear();
    ui->assignTable->clear();
    ui->indexTable->clear();
    ui->PDOTable->clear();
    ui->startupTable->clear();

    ui->slaveTable->setRowCount(0);
    ui->assignTable->setRowCount(0);
    ui->indexTable->setRowCount(0);
    ui->PDOTable->setRowCount(0);
    ui->startupTable->setRowCount(0);


    slaves_hash.clear();
    QStringList slave_table_header;
    slave_table_header << "从站名";
    //ui->logList->setColumnCount(4);
    //ui->logList->setHorizontalHeaderLabels(header);
    ui->slaveTable->setColumnCount(1);
    ui->slaveTable->setHorizontalHeaderLabels(slave_table_header);
    ui->slaveTable->setColumnWidth(0,300);
    ui->slaveTable->horizontalHeader()->setVisible(true);
    ui->slaveTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->slaveTable->setRowCount(slave_vec.size());
    for(int i = 0;i< slave_vec.size();i++){
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(slave_vec[i].name);
        ui->slaveTable->setItem(i,0,item);
        slave_vec[i].position = i;
        slaves_hash.insert(slave_vec[i].name,slave_vec[i]);
        insertLog(1,"JSON","生成 " + slave_vec[i].name + " 成功");
    }
    axis_fresh();
    insertLog(1,"ETC配置器","加载ETC从站配置完成");
    return 0;
}
void ethercatwid::on_creat_JSON_clicked()
{
    QMessageBox::StandardButton result = QMessageBox::warning(this,"警告","将会覆盖默认ETC配置",QMessageBox::Yes | QMessageBox::No);
    if(result == QMessageBox::No){
        return;
    }

    QString dirPath = "/home/etcConfig";
    QDir dir(dirPath);
    if (!dir.isReadable()){
        bool bret = dir.mkdir(dirPath);
        if(bret){
            insertLog(1,"ETC配置","创建配置目录成功");
        }else{
            insertLog(-1,"ETC配置","创建配置目录失败");
            return;
        }
    }
    QString path = "/home/etcConfig/config.json";
    JSON_Save(path);
}
void ethercatwid::on_getconf_JSON_clicked()
{
    QString path = "/home/etcConfig/config.json";
    JSON_Read(path);
}

void ethercatwid::on_auto_config_clicked()
{
    QMessageBox::StandardButton result = QMessageBox::warning(this,"警告","将会覆盖所有PID,VID,ALIAS参数",QMessageBox::Yes | QMessageBox::No);
    if(result == QMessageBox::No){
        return;
    }
    ec_master *master = ecrt_open_master(0);
    if(master == nullptr){
        insertLog(-1,"","打开主站失败");
        return;
    }

    for(int i = 0; i < ui->slaveTable->rowCount();i++){
        ec_slave_info_t info;
        int ret = ecrt_master_get_slave(master,i,&info);
        if(ret != 0){
            insertLog(-1,"","PDO配置于实际链接个数不符");
            return;
        }
        ui->slaveTable->setCurrentItem(ui->slaveTable->item(i,0));
        slave_config *config = get_Slave();
        if(config == nullptr){
            insertLog(-1,"","找不到从站 - " + ui->slaveTable->item(i,0)->text());
            return;
        }
        config->alias = info.alias;
        config->pid = info.product_code;
        config->vid = info.vendor_id;
        insertLog(1,ui->slaveTable->item(i,0)->text(),"配置完成 ");


    }
}



















void ethercatwid::on_updatAxis_clicked()
{
    ui->axisTab->clear();
    auto it = axis_hash.begin();
    for(it  = axis_hash.begin() ; it != axis_hash.end() ; it++){
        axisWid *page = new axisWid();
        page->axNo = it.value()->axNo;
        page->name = it.key();
        page->showby(it.value()->axNo);
        ui->axisTab->addTab((QWidget *)page ,it.key());
    }
}
