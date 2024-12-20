#include "ethercatmaster.h"
#include "ui_ethercatmaster.h"

#define ETCJSON_PATH "/home/etcConfig/config.json"
EtherCatMaster::EtherCatMaster(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EtherCatMaster)
{
    ui->setupUi(this);
    xCloseSig = false;


    connect(this,&EtherCatMaster::message,this,&EtherCatMaster::message_show,Qt::QueuedConnection);
    connect(&_10ms_timer,SIGNAL(timeout()),this,SLOT(_10ms_fn()));
    _10ms_timer.setSingleShot(false);
    _10ms_timer.setInterval(1);
    //_10ms_timer.start();

    connect(&_100ms_timer,SIGNAL(timeout()),this,SLOT(_100ms_fn()));
    _100ms_timer.setSingleShot(false);
    _100ms_timer.setInterval(100);
    _100ms_timer.start();

    //sock.bind(9900,QUdpSocket::ShareAddress);
    //connect(&sock,&QUdpSocket::readyRead,this,&EtherCatMaster::ec_udp);
    bool listen = server.listen(QHostAddress::Any,9900);
    if (!listen){
        emit message(this,"error","listen error");
    }
    connect(&server,&QTcpServer::newConnection,this,&EtherCatMaster::tcp_new);

    bool ret = false;

    ecm = (cec_shm_pdo *)etcinit();
    if (ecm != nullptr){
        ret = true;
        xShmRet = ret;
    }else{
        xShmRet = false;
        emit message(this,"error","get shm error");
    }
    ecm->pdoSize = sizeof(cec_shm_pdo);
    for(int i=0;i < MAX_AXIS;i++){
        ecm->axis[i].axNo = i;
    }
    //ecm->task_info.name = "Ethercat_Task";
    sprintf(ecm->task_info.name,"Ethercat_Task");
    bool udp =  udpSock.bind(9901);
    if (!udp){
        emit message(this,"error","udp error");
    }
    qDebug() <<"ecm size" << sizeof(cec_shm_pdo) << " mem percent "<< ((double)sizeof(cec_shm_pdo)) / ((double)ETC_SHM_SIZE) * 100 << " ecm ptr" << ecm;
//home/icon


    QMenu *iconMenup = new QMenu();

    QAction *close = new QAction(" 关闭程序 ");
    QAction *widshow = new QAction(" 显示主站界面 ");
    QAction *startetc = new QAction(" 启动主站 ");
    QAction *stioetc = new QAction(" 停止主站 ");
    iconMenup->addAction(close);
    iconMenup->addAction(widshow);
    iconMenup->addAction(startetc);
    iconMenup->addAction(stioetc);


    connect(close,&QAction::triggered,this,[=](){
        this->close();
    });
    connect(widshow,&QAction::triggered,this,[=](){
        show();
    });
    connect(startetc,&QAction::triggered,this,[=](){
        if(start){
            return;
        }
        //ethercatRun();
        sleep(1);
        start = false;
        sleep(2);
        ethercatRun();
        mem_insertLog(100,"EtherCat","rereadDB");
    });
    connect(stioetc,&QAction::triggered,this,[=](){
        //mem_insertLog(0,"EtherCat","click stop");
        qDebug() << "click stop";
        if (start == false){
            return;
        }
        show();
        QMessageBox::StandardButtons ret = QMessageBox::information(this,"info","close master , please click yes to close",QMessageBox::Yes,QMessageBox::No);
        hide();
        if(ret == QMessageBox::No){
            return;
        }
        start = false;
        mem_insertLog(-1,"EtherCat","主站已停止");
    });


    QIcon icon0("/home/icon/config.png");
    ticon.setIcon(icon0);
    ticon.setToolTip("CRTools EtherCat Master");
    ticon.show();
    ticon.setContextMenu(iconMenup);
    //resize(10,10);
//    hide();
//    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
//    this->setVisible(false);
    setWindowFlags(Qt::Tool);

    show();
//    close();
//    hide();
//    ethercatRun();
//    sleep(1);
//    start = false;
//    sleep(2);
//    ethercatRun();
    //axisWid *axisPage = new axisWid();
    //axisPage->show();
}
void *EtherCatMaster::etcinit(void)
{

    void *ptr = new cec_shm_pdo();
    return ptr;

}
#define NOT_RUN "background: rgb(200,200,200);\n  border: 1px solid #333333;\n    padding: 1px;\n"
#define RUNING "background: rgb(32,233,23);\n  border: 1px solid #333333;\n    padding: 1px;\n"
void EtherCatMaster::_100ms_fn()
{

    ui->period->setText(QString::number(period_ns));
    if(start){
        ui->runing->setStyleSheet(RUNING);
        ec_master_state_t state_temp;
        ecrt_master_state(master,(ec_master_state_t*)&state_temp);
        ecm->etc_state = state_temp.al_states;
        QIcon icon0("/home/icon/configDone.png");
        ticon.setIcon(icon0);
    }else{
        ui->runing->setStyleSheet(NOT_RUN);
        ecm->etc_state = 0;
        QIcon icon0("/home/icon/notConfig.png");
        ticon.setIcon(icon0);
    }

    if(master != nullptr){
        ec_master_state_t state_temp;
        ecrt_master_state(master,(ec_master_state_t*)&state_temp);
        ecm->etc_state = state_temp.al_states;
    }else{
        ecm->etc_state = 0;
    }
    ecm->etc_hreat_counter++;


    if((ecm->CRTtools_counter_old == ecm->CRTtools_counter) && start){
        CRTtools_timeOut++;
    }else{
        CRTtools_timeOut = 0;
    }
    if(CRTtools_timeOut >= 5){
        //start = false;
        //CRTtools_timeOut = 0;
        //ticon.showMessage("EtherCatMaster","CRTtools 后台被关闭，主站自动停止");
    }
    ecm->CRTtools_counter_old = ecm->CRTtools_counter;
}
void EtherCatMaster::_10ms_fn()
{



}
void EtherCatMaster::getdouble(double *dest,double min,double max)
{
    QInputDialog input;
    *dest = input.getDouble(this,"input","value",*dest,min,max,6);
}
void EtherCatMaster::message_show(QObject *father,QString title,QString info)
{
    QMessageBox::information(this,title,info);
}

void _ino_gl10_etc_conf(pod_config * conf)
{


    int ref;
    uint32_t abort_code;
    uint8_t data8 = 0;
    uint16_t data16 = 0;
    uint32_t data32 = 0;

    //0xFB00:00 -> 1
    data8 = 1;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0xFB00,0,(uint8_t *)&data8,1,&abort_code);
    if (ref == 0){
        qDebug() << "0xFB00:00 -> 0 dong";
    }else{
        qDebug() << "0xFB00:00 -> 0 error";
        //return;
    }

    //0x1c12:00 -> 0
    data8 = 0;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c12,0,(uint8_t *)&data8,1,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c12:00 -> 0 dong";
    }else{
        qDebug() << "0x1c12:00 -> 0 error";
        //return;
    }

    //0x1c13:00 -> 0
    data8 = 0;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c13,0,(uint8_t *)&data8,1,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c13:00 -> 0 dong";
    }else{
        qDebug() << "0x1c13:00 -> 0 error";
        //return;
    }

    //0x1c12:01 -> 0x1604
    data16 = 0x1600;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c12,1,(uint8_t *)&data16,2,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c12:01 -> 0x1604 dong";
    }else{
        qDebug() << "0x1c12:01 -> 0x1604 error";
        //return;
    }

    //0x1c12:00 -> 1
    data8 = 1;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c12,0,(uint8_t *)&data8,1,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c12:00 -> 1 dong";
    }else{
        qDebug() << "0x1c12:00 -> 1 error";
        //return;
    }

    //0x1c13:01 -> 0x1a14
    data16 = 0x1a10;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c13,1,(uint8_t *)&data16,2,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c13:01 -> 0x1a14 dong";
    }else{
        qDebug() << "0x1c13:01 -> 0x1a14 error";
        //return;
    }

    //0x1c13:00 -> 1
    data8 = 1;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c13,0,(uint8_t *)&data8,1,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c13:00 -> 1 dong";
    }else{
        qDebug() << "0x1c13:00 -> 1 error";
        //return;
    }

    //0xf030:00 -> 0
    data8 = 0;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0xf030,0,(uint8_t *)&data8,1,&abort_code);
    if (ref == 0){
        qDebug() << "0xf030:00 -> 1 dong";
    }else{
        qDebug() << "0xf030:00 -> 1 error";
        //return;
    }

    //0xf030:01 -> 0x10f40220
    data32 = 0x10f40220;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0xf030,1,(uint8_t *)&data32,4,&abort_code);
    if (ref == 0){
        qDebug() << "0xf030:01 -> 0x10f40220 dong";
    }else{
        qDebug() << "0xf030:01 -> 0x10f40220 error";
        //return;
    }

    //0xf030:02 -> 0x10f40210
    data32 = 0x10f40210;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0xf030,2,(uint8_t *)&data32,4,&abort_code);
    if (ref == 0){
        qDebug() << "0xf030:02 -> 0x10f40210 dong";
    }else{
        qDebug() << "0xf030:02 -> 0x10f40210 error";
        //return;
    }

    //0x1c32:01 -> 0
    data16 = 0;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c32,1,(uint8_t *)&data16,2,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c32:01 -> 0 dong";
    }else{
        qDebug() << "0x1c32:01 -> 0 error";
        //return;
    }


    //0x1c32:02 -> 1000000
    data32 = 500000;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c32,2,(uint8_t *)&data32,4,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c32:02 -> 1000000 done";
    }else{
        qDebug() << "0x1c32:02 -> 1000000 error";
        //return;
    }

    //0x1c33:01 -> 0
    data16 = 0;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c33,1,(uint8_t *)&data16,2,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c33:01 -> 0 dong";
    }else{
        qDebug() << "0x1c33:01 -> 0 error";
        //return;
    }


    //0x1c33:02 -> 1000000
    data32 = 500000;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0x1c33,2,(uint8_t *)&data32,4,&abort_code);
    if (ref == 0){
        qDebug() << "0x1c33:02 -> 1000000 done";
    }else{
        qDebug() << "0x1c33:02 -> 1000000 error";
        //return;
    }

    //0xFB00:00 -> 0
    data8 = 0;
    ref = ecrt_master_sdo_download(conf->master,conf->position,0xFB00,0,(uint8_t *)&data8,1,&abort_code);
    if (ref == 0){
        qDebug() << "0xFB00:00 -> 0 dong";
    }else{
        qDebug() << "0xFB00:00 -> 0 error";
        //return;
    }

    int size = 4;
    conf->indexSize = 2;
    conf->pdoSize = 4;
    conf->offset = (uint *)malloc(size *sizeof(uint));
    conf->domain1_regs = (ec_pdo_entry_reg_t *)malloc(size * sizeof(ec_pdo_entry_reg_t));
    conf->device_pdo_entries = (ec_pdo_entry_info_t *)malloc(size * sizeof(ec_pdo_entry_info_t));
    conf->device_syncs = (ec_sync_info_t *)malloc(5 * sizeof(ec_sync_info_t));
    conf->device_pdos = (ec_pdo_info_t *)malloc(2 * sizeof(ec_pdo_info_t));
    //ec_pdo_entry_info_t *info = conf_info->info;
    //conf->pdoname = (char *)malloc(5 * 64);
    memset(conf->pdoname,0,size * 64);
    struct name64{
      char cstr[64];
    };
    name64 *name = (name64 *)conf->pdoname;

    sprintf(name[0].cstr,"DO-CH1");
    sprintf(name[1].cstr,"DO-CH2");
    sprintf(name[2].cstr,"DI-CH1");
    sprintf(name[3].cstr,"DI-CH2");



    conf->domain1_regs[0] = {conf->alias,conf->position, conf->vid,conf->pid, 0X7002, 0x01, &conf->offset[0]};
    conf->device_pdo_entries[0] = {0X7002, 0x01, 8};
    conf->info[0] = {0X7002, 0x01, 8};
    conf->pdo_dir[0] = PDO_OUTPUT;

    conf->domain1_regs[1] = {conf->alias,conf->position, conf->vid,conf->pid, 0X7002, 0x02, &conf->offset[1]};
    conf->device_pdo_entries[1] = {0X7002, 0x02, 8};
    conf->info[1] = {0X7002, 0x02, 8};
    conf->pdo_dir[1] = PDO_OUTPUT;

    conf->domain1_regs[2] = {conf->alias,conf->position, conf->vid,conf->pid, 0x6101, 0x01, &conf->offset[2]};
    conf->device_pdo_entries[2] = {0x6101, 0x01, 8};
    conf->info[2] = {0x6101, 0x01, 8};
    conf->pdo_dir[2] = PDO_INPUT;

    conf->domain1_regs[3] = {conf->alias,conf->position, conf->vid,conf->pid, 0x6101, 0x02, &conf->offset[3]};
    conf->device_pdo_entries[3] = {0x6101, 0x02, 8};
    conf->info[3] = {0x6101, 0x02, 8};
    conf->pdo_dir[3] = PDO_INPUT;




    conf->device_pdos[0] = {0x1600, 2, conf->device_pdo_entries + 0 };
    conf->device_pdos[1] = {0x1A10, 2, conf->device_pdo_entries + 2};

    conf->device_syncs[0] = { 0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE };
    conf->device_syncs[1] = { 1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE };
    conf->device_syncs[2] = { 2, EC_DIR_OUTPUT, 1, conf->device_pdos + 0, EC_WD_ENABLE};
    conf->device_syncs[3] = { 3, EC_DIR_INPUT, 1, conf->device_pdos + 1, EC_WD_ENABLE};
    conf->device_syncs[4] = { 0xFF};



    conf->inNum = 2;
    conf->outNum = 2;


    //conf->
    //conf->alias = alias;
    //conf->position = position;
    //conf->vid = vid;
    //conf->pid = pid;
}
void _ino_gl10_etc_conf_TEST(pod_config * conf)
{


    int ref;
    uint32_t abort_code;
    uint8_t data8 = 0;
    uint16_t data16 = 0;
    uint32_t data32 = 0;



    int size = 6;
    conf->indexSize = 3;
    conf->pdoSize = 6;
    conf->offset = (uint *)malloc(size *sizeof(uint));
    conf->domain1_regs = (ec_pdo_entry_reg_t *)malloc(size * sizeof(ec_pdo_entry_reg_t));
    conf->device_pdo_entries = (ec_pdo_entry_info_t *)malloc(size * sizeof(ec_pdo_entry_info_t));
    conf->device_syncs = (ec_sync_info_t *)malloc(6 * sizeof(ec_sync_info_t));
    conf->device_pdos = (ec_pdo_info_t *)malloc(3 * sizeof(ec_pdo_info_t));
    //ec_pdo_entry_info_t *info = conf_info->info;
    //conf->pdoname = (char *)malloc(5 * 64);
    memset(conf->pdoname,0,size * 64);
    struct name64{
      char cstr[64];
    };
    name64 *name = (name64 *)conf->pdoname;

    sprintf(name[0].cstr,"DO-CH1");
    sprintf(name[1].cstr,"DO-CH2");
    sprintf(name[2].cstr,"DI-CH1");
    sprintf(name[3].cstr,"DI-CH2");
    sprintf(name[4].cstr,"DI-CH3");
    sprintf(name[5].cstr,"DI-CH4");


    conf->domain1_regs[0] = {conf->alias,conf->position, conf->vid,conf->pid, 0X7002, 0x01, &conf->offset[0]};
    conf->device_pdo_entries[0] = {0X7002, 0x01, 8};
    conf->info[0] = {0X7002, 0x01, 8};
    conf->pdo_dir[0] = PDO_OUTPUT;

    conf->domain1_regs[1] = {conf->alias,conf->position, conf->vid,conf->pid, 0X7002, 0x02, &conf->offset[1]};
    conf->device_pdo_entries[1] = {0X7002, 0x02, 8};
    conf->info[1] = {0X7002, 0x02, 8};
    conf->pdo_dir[1] = PDO_OUTPUT;

    conf->domain1_regs[2] = {conf->alias,conf->position, conf->vid,conf->pid, 0x6101, 0x01, &conf->offset[2]};
    conf->device_pdo_entries[2] = {0x6101, 0x01, 8};
    conf->info[2] = {0x6101, 0x01, 8};
    conf->pdo_dir[2] = PDO_INPUT;

    conf->domain1_regs[3] = {conf->alias,conf->position, conf->vid,conf->pid, 0x6101, 0x02, &conf->offset[3]};
    conf->device_pdo_entries[3] = {0x6101, 0x02, 8};
    conf->info[3] = {0x6101, 0x02, 8};
    conf->pdo_dir[3] = PDO_INPUT;

    conf->domain1_regs[4] = {conf->alias,conf->position, conf->vid,conf->pid, 0x6111, 0x01, &conf->offset[5]};
    conf->device_pdo_entries[4] = {0x6111, 0x01, 8};
    conf->info[4] = {0x6111, 0x01, 8};
    conf->pdo_dir[4] = PDO_INPUT;

    conf->domain1_regs[5] = {conf->alias,conf->position, conf->vid,conf->pid, 0x6121, 0x02, &conf->offset[6]};
    conf->device_pdo_entries[5] = {0x6121, 0x02, 8};
    conf->info[5] = {0x6121, 0x02, 8};
    conf->pdo_dir[5] = PDO_INPUT;


    conf->device_pdos[0] = {0x1600, 2, conf->device_pdo_entries + 0 };
    conf->device_pdos[1] = {0x1A10, 2, conf->device_pdo_entries + 2};
    conf->device_pdos[2] = {0x1A11, 2, conf->device_pdo_entries + 4};

    conf->device_syncs[0] = { 0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE };
    conf->device_syncs[1] = { 1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE };
    conf->device_syncs[2] = { 2, EC_DIR_OUTPUT, 1, conf->device_pdos + 0, EC_WD_ENABLE};
    conf->device_syncs[3] = { 3, EC_DIR_INPUT, 2, conf->device_pdos + 1, EC_WD_ENABLE};
    conf->device_syncs[4] = { 0xFF};



    conf->inNum = 2;
    conf->outNum = 4;


    //conf->
    //conf->alias = alias;
    //conf->position = position;
    //conf->vid = vid;
    //conf->pid = pid;
}
void EtherCatMaster::oneinotest()
{
//    ec_master_t *master = ecrt_request_master(0);
//    ecm->pdo_Num = 1;
//    ecm->axis_Num = 0;
//    pod_config *conf = &ecm->pdoconf[0];
//    conf->pid = 0x10f40910;
//    conf->vid = 0x00100000;
//    conf->position = 0;
//    conf->alias = 0;
//    conf->master = master;
//    _ino_gl10_etc_conf(conf);
//    ecrt_release_master(master);



    ecm->pdo_Num = 1;
    ecm->axis_Num = 1;
    int axisNo = 1;
    QString nameAry[20];
    nameAry[0] = 	"上料上下压"		;
    nameAry[1] = 	"编带上下压"		;
    nameAry[2] = 	"3D光检上下压"		;
    nameAry[3] = 	"B盘上下压"		;
    nameAry[4] = 	"极性上下压"		;
    nameAry[5] = 	"定位1上下压"		;
    nameAry[6] = 	"定位2上下压"		;
    nameAry[7] = 	"定位3上下压"		;
    nameAry[8] = 	"测试1上下压"		;
    nameAry[9] = 	"660N上下压"		;
    nameAry[10] = 	"680N上下压"		;
    nameAry[11] = 	"sigma7上下压"		;


    for (int i = 0;i <= 10 ; i++){
        axis_struct_init(&ecm->axis[i]);
        if ( i == 10){// 680
            getDrvPodConf(0,i ,0x00100000,0x000c0116,&ecm->pdoconf[i]);
            ecm->axis[i].pdoNo = i;
            ecm->axis[i].ratio_deno = 67108864;
            ecm->axis[i].ratio_num = 1;
            ecm->axis[i].enc_ratio = 67108864;
        }else if(i == 11){// sigma7
            getDrvPodConf(0,i ,0x00000539,0x02200301,&ecm->pdoconf[i]);
            ecm->axis[i].pdoNo = i;
            ecm->axis[i].ratio_deno = 8388608;
            ecm->axis[i].ratio_num = 1;
            ecm->axis[i].enc_ratio = 8388608;
        }else{
            getDrvPodConf(0,i ,0x00100000,0x000c010d,&ecm->pdoconf[i]);
            ecm->axis[i].pdoNo = i;
            ecm->axis[i].ratio_deno = 1048576;
            ecm->axis[i].ratio_num = 1;
            ecm->axis[i].enc_ratio = 1048576;
        }
        memcpy(ecm->axis[i].name,nameAry[i].toStdString().c_str(),nameAry[i].toStdString().length());
    }

    return;


}


struct timespec timespec_add(struct timespec time1, struct timespec time2)
{
    struct timespec result;

    if ((time1.tv_nsec + time2.tv_nsec) >= NSEC_PER_SEC) {
        result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec - NSEC_PER_SEC;
    } else {
        result.tv_sec = time1.tv_sec + time2.tv_sec;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
    }

    return result;
}

void EtherCatMaster::tcp_cmd_thread(EtherCatMaster *self)
{

}
void EtherCatMaster::initShmData(void *data)
{
    cec_shm_pdo *etc_data = (cec_shm_pdo *)data;
}
int EtherCatMaster::getNormalConf(   uint16_t alias, /**< Slave alias address. */
                                     uint16_t position, /**< Slave position. */
                                     uint32_t vid, /**< Slave vid. */
                                     uint32_t pid, /**< Slave pid. */
                                     void *arg,
                                     pod_config *conf)
{
    //pod_config conf;
    #define SLAVE_POS   alias,position
    #define VID_PID     vid,pid
    slave_pdo_conf_info *conf_info = (slave_pdo_conf_info *)arg;
    ec_pdo_entry_info_t *info = conf_info->info;
    int sumPdo = conf_info->inNum + conf_info->outNum;

    if(conf_info->inNum > 10){
        return -1;
    }
    if(conf_info->outNum > 10){
        return -2;
    }
    int inSize = 0;
    for(int i = 0;i<conf_info->inNum;i++){
        inSize = inSize + info[i].bit_length;
    }
    if(inSize > MAX_PDO_SIZE * 8){
        return -3;
    }
    int outSize = 0;
    for(int i = conf_info->inNum;i<conf_info->inNum + conf_info->outNum;i++){
        outSize = outSize + info[i].bit_length;
    }
    if(outSize > MAX_PDO_SIZE * 8){
        return -4;
    }

    conf->offset = (uint *)malloc(sumPdo *sizeof(uint));
    conf->domain1_regs = (ec_pdo_entry_reg_t *)malloc(sumPdo * sizeof(ec_pdo_entry_reg_t));
    conf->device_pdo_entries = (ec_pdo_entry_info_t *)malloc(sumPdo * sizeof(ec_pdo_entry_info_t));
    conf->device_syncs = (ec_sync_info_t *)malloc(5 * sizeof(ec_sync_info_t));
    conf->device_pdos = (ec_pdo_info_t *)malloc(2 * sizeof(ec_pdo_info_t));

    for(int i = 0 ; i < sumPdo ; i++){
        conf->domain1_regs[i] = {SLAVE_POS, VID_PID, info[i].index, info[i].subindex, &conf->offset[i]};
        conf->device_pdo_entries[i] = {info[i].index, info[i].subindex, info[i].bit_length};
        conf->pdo_dir[i] = conf_info->dir[i];
    }

    conf->device_pdos[0] = {0x1600, conf_info->inNum, conf->device_pdo_entries + 0 };
    conf->device_pdos[1] = {0x1A00, conf_info->outNum, conf->device_pdo_entries + conf_info->inNum };

    conf->device_syncs[0] = { 0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE };
    conf->device_syncs[1] = { 1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE };
    conf->device_syncs[2] = { 2, EC_DIR_OUTPUT, 1, conf->device_pdos + 0, conf_info->inWD};
    conf->device_syncs[3] = { 3, EC_DIR_INPUT, 1, conf->device_pdos + 1, conf_info->outWD};
    conf->device_syncs[4] = { 0xFF};


    conf->inNum = conf_info->inNum;
    conf->outNum = conf_info->outNum;
    conf->alias = alias;
    conf->position = position;
    conf->vid = vid;
    conf->pid = pid;
    memcpy(conf->info,info,sumPdo * sizeof(ec_pdo_entry_info_t));
    return 0;
}
int EtherCatMaster::getDrvPodConf(    uint16_t alias, /**< Slave alias address. */
                                             uint16_t position, /**< Slave position. */
                                             uint32_t vid, /**< Slave vid. */
                                             uint32_t pid, /**< Slave pid. */
                                             pod_config *conf)
{
    //pod_config conf;

    slave_pdo_conf_info slave_conf;
    slave_conf.inWD = EC_WD_DEFAULT;
    slave_conf.outNum = EC_WD_DEFAULT;
    slave_conf.inNum = 5;
    slave_conf.outNum = 7;
    int sumNum = 12;
    /*TxPdo 0x1600*/
    slave_conf.info = (ec_pdo_entry_info_t *)malloc(sumNum * sizeof(ec_pdo_entry_info_t));
    slave_conf.info[0] =  {0x6040, 0x00, 16};slave_conf.dir[0] = 1;      //    unsigned int ctrl_word;
    slave_conf.info[1] =  {0x6060, 0x00, 8 };slave_conf.dir[1] = 1;       //    unsigned int operation_mode;
    slave_conf.info[2] =  {0x607A, 0x00, 32};slave_conf.dir[2] = 1;       //    unsigned int target_position;
    slave_conf.info[3] =  {0x6071, 0x00, 16};slave_conf.dir[3] = 1;       //    unsigned int target_torque;
    slave_conf.info[4] =  {0x60FE, 0x01, 32};slave_conf.dir[4] = 1;       //    unsigned int digital_outputs;
    /*RxPdo 0x1A00*/
    slave_conf.info[5] =  {0x603F, 0x00, 16};slave_conf.dir[5] = 0;        //    unsigned int error_code;
    slave_conf.info[6] =  {0x6041, 0x00, 16};slave_conf.dir[6] = 0;       //    unsigned int status_word;
    slave_conf.info[7] =  {0x6061, 0x00, 8 };slave_conf.dir[7] = 0;       //    unsigned int modes_of_operation_display;
    slave_conf.info[8] =  {0x6064, 0x00, 32};slave_conf.dir[8] = 0;       //    unsigned int position_actual_value;
    slave_conf.info[9] =  {0x6077, 0x00, 16};slave_conf.dir[9] = 0;       //    unsigned int torque_actual_velue;
    slave_conf.info[10] = {0x60F4, 0x00, 32};slave_conf.dir[10] = 0;       //    unsigned int following_error_actual_value;
    slave_conf.info[11] = {0x60FD, 0x00, 32};slave_conf.dir[11] = 0;      //    unsigned int digital_inputs;

    slave_conf.pdoName = (char *)malloc(sumNum * 64);
    memset(slave_conf.pdoName,0,sumNum * 64);
    struct name64{
      char cstr[64];
    };
    name64 *name = (name64 *)slave_conf.pdoName;
    sprintf(name[0].cstr,"ctrl_word");
    sprintf(name[1].cstr,"operation_mode");
    sprintf(name[2].cstr,"target_position");
    sprintf(name[3].cstr,"target_torque");
    sprintf(name[4].cstr,"digital_outputs");
    sprintf(name[5].cstr,"error_code");
    sprintf(name[6].cstr,"status_word");
    sprintf(name[7].cstr,"modes_of_operation_display");
    sprintf(name[8].cstr,"position_actual_value");
    sprintf(name[9].cstr,"torque_actual_velue");
    sprintf(name[10].cstr,"following_error_actual_value");
    sprintf(name[11].cstr,"digital_inputs");

    memcpy(conf->pdoname,slave_conf.pdoName,sumNum * 64);

//    unsigned int ctrl_word;
//    unsigned int operation_mode;
//    unsigned int target_position;
//    unsigned int target_torque;
//    unsigned int error_code;
//    unsigned int status_word;
//    unsigned int modes_of_operation_display;
//    unsigned int position_actual_value;
//    unsigned int torque_actual_velue;
//    unsigned int following_error_actual_value;
//    unsigned int digital_inputs;


    int ret;
    ret = getNormalConf(alias,position,vid,pid,&slave_conf,conf);

    free(slave_conf.info);

    return ret;


}
void EtherCatMaster::syncPod_Read(pod_config *pdo)
{
    int pos = 0;
    ecrt_slave_config_state(pdo->sc,&pdo->slave_state);
    pdo->syncWrite = false;
    for(int i = 0;i < pdo->inNum + pdo->outNum;i++){
        int size = pdo->info[i].bit_length / 8;
        if (pdo->pdo_dir[i] != 0){

        }else{

            //memcpy(pdo->pdo_data + pos,domain1_pd + pdo->offset[i],size);
            switch(size){
                case 1:
                    *(char *)&pdo->pdo_data[pos] = EC_READ_U8(domain1_pd + pdo->offset[i]);
                break;
                case 2:
                    *(ushort *)&pdo->pdo_data[pos] = EC_READ_U16(domain1_pd + pdo->offset[i]);
                break;
                case 4:
                    *(uint *)&pdo->pdo_data[pos] = EC_READ_U32(domain1_pd + pdo->offset[i]);
                break;
                case 8:
                    *(ulong *)&pdo->pdo_data[pos] = EC_READ_U64(domain1_pd + pdo->offset[i]);
                break;
                default:
                break;
            }
        }

        pos = pos + size;
    }
}
void EtherCatMaster::syncPod_Write(pod_config *pdo)
{
    int pos = 0;
    if(pdo->syncWrite){
        return;
    }
    pdo->syncWrite = true;
    for(int i = 0;i < pdo->inNum + pdo->outNum;i++){
        if (pdo->pdo_dir[i] != PDO_OUTPUT){
            continue;
        }
        int size = pdo->info[i].bit_length / 8;
        //memcpy(pdo->pdo_data + pos,domain1_pd + pdo->offset[i],size);
        switch(size){
        case 1:
            //*(char *)&pdo->pdo_data[pos] = EC_WRITE_U8(domain1_pd + pdo->offset[i]);
            EC_WRITE_U8(domain1_pd + pdo->offset[i],*((char *)&pdo->pdo_data[pos]));
        break;
        case 2:
            //*(ushort *)&pdo->pdo_data[pos] = EC_READ_U16(domain1_pd + pdo->offset[i]);
            EC_WRITE_U16(domain1_pd + pdo->offset[i],*((ushort *)&pdo->pdo_data[pos]));
        break;
        case 4:
            //*(uint *)&pdo->pdo_data[pos] = EC_READ_U32(domain1_pd + pdo->offset[i]);
            EC_WRITE_U32(domain1_pd + pdo->offset[i],*((uint *)&pdo->pdo_data[pos]));
        break;
        case 8:
            //*(ulong *)&pdo->pdo_data[pos] = EC_READ_U64(domain1_pd + pdo->offset[i]);
            EC_WRITE_U64(domain1_pd + pdo->offset[i],*((ulong *)&pdo->pdo_data[pos]));
        break;
        default:
        break;
        }
        pos = pos + size;
    }
}
int EtherCatMaster::freeConf(pod_config *conf)
{
    free(conf->device_pdos);
    free(conf->device_syncs);
    free(conf->domain1_regs);
    free(conf->device_pdo_entries);
    free(conf->offset);
    free(conf->pdoname);

    return 0;
}
#define MEASURE_TIMING

void EtherCatMaster::closeEvent(QCloseEvent *e)
{
    show();
    QMessageBox::StandardButtons ret = QMessageBox::information(this,"alarm","close etc if click Yes , click No to minisize",QMessageBox::Yes,QMessageBox::No);
    hide();
    if(ret == QMessageBox::No){
        e->ignore();
        return;
    }else{

    }
    udpSock.close();
    xCloseSig = true;
    sleep(2);
    ticon.deleteLater();
    //shm.detach();
    server.close();
    close();

}


EtherCatMaster::~EtherCatMaster()
{

    delete ui;
}
void EtherCatMaster::ethercatRun()
{
    if (start){
        return;
    }

    bool ret = false;
    if (ecm != nullptr){
        ret = true;
    }else{
        ecm = (cec_shm_pdo *)etcinit();
        if (ecm != nullptr){
            ret = true;
        }
    }
    if (sizeof(cec_shm_pdo) > ETC_SHM_SIZE){
        MESSAGE_INFO("error","kenrel mem < ecm size!");
        return;
    }

    if(!ret){
        MESSAGE_INFO("error","get shm error!");
    }else{
        //MESSAGE_INFO("error","get shm done!");
        start = true;
        memset(ecm,0,sizeof(cec_shm_pdo));
        int ref = pthread_create(&ec_sync,NULL,call_sync,(void *)this);
        if(ref != 0){
            ref = ref;
        }

    }
//ecrt_slave_config_reg_pdo_entry
    qDebug() <<"ecm size" << sizeof(cec_shm_pdo)<<"ecm ptr" << ecm;
    for(int i = 0;i < MAX_SLAVE;i++){
        ecm->axis[i].plan = &vel_plan[i];
    }
    //MESSAGE_INFO("error","get shm done!");
    sleep(2);

    //_10ms_timer.start();

    c_state = 0;
    ticon.showMessage("EtherCatMaster","CRTtools ETC主站已启动");
}


ec_watchdog_mode_t getWD(int wd)
{
    switch (wd) {
    case 0:
        return EC_WD_DEFAULT;
    case 1:
        return EC_WD_ENABLE;
    case 2:
        return EC_WD_DISABLE;
    default:
        return EC_WD_DEFAULT;
    }
}
#define HEX_OUT(x) QString::number(x,16)
void EtherCatMaster::read_config_by_json()
{
    mem_insertLog(0,"EtherCatMaster","开始读取配置");
    QString path = ETCJSON_PATH;
    QFile file(path);
    bool bret = file.open(QIODevice::ReadOnly);
    if (!bret){
        mem_insertLog(-1,"EtherCatMaster","打开配置文件失败");
        return ;
    }

    file.seek(0);
    QJsonDocument jsonDoc;
    QJsonParseError error;
    QByteArray fileMem = file.readAll();
    QString fileStr = QString::fromStdString(fileMem.toStdString());


    //insertLog(0,"JSON","文件大小 " + QString::number(fileMem.size()));
    jsonDoc = QJsonDocument::fromJson(fileMem,&error);

    QJsonObject vec_obj = jsonDoc.object();
    QVector<slave_config> slave_vec;
    int ret = etc_json2vector("",&vec_obj,&slave_vec);
    if(ret != 0){
        //insertLog(-1,"警告","解析JSON文件失败 " + last_json_error);
        mem_insertLog(-1,"EtherCatMaster","解析JSON文件失败");
        return;
    }
    QVector<axis_config> axis_vec;
    ret = axis_json2vector("",&vec_obj,&axis_vec);
    if(ret != 0){
        //insertLog(-1,"警告","解析JSON文件失败 " + last_json_error);
        mem_insertLog(-1,"EtherCatMaster","解析JSON文件失败");
        return;
    }
    file.close();
    ecm->pdo_Num = slave_vec.size();
    ecm->axis_Num = axis_vec.size();
    ec_master_t *etcMaster = ecrt_request_master(0);

    if(etcMaster == 0){
        mem_insertLog(-1,"EtherCatMaster","主站驱动未打开");
        return;
    }
    for(int i = 0; i < slave_vec.size(); i++){
        pod_config *conf = &ecm->pdoconf[i];
        slave_config *slave = &slave_vec[i];
        conf->alias = slave->alias;
        conf->position = i;
        conf->pid = slave->pid;
        conf->vid = slave->vid;
        char chr[64];
        memcpy(conf->slaveName,slave->name.toUtf8().data(),slave->name.toUtf8().size());
        memcpy(chr,slave->name.toUtf8().data(),slave->name.toUtf8().size());
        for(int startup_i = 0 ;  startup_i < slave_vec[i].startup.size(); startup_i ++){
            uint32_t abort_code;
            uint8_t udata8 = 0;
            uint16_t udata16 = 0;
            uint32_t udata32 = 0;
            uint64_t udata64 = 0;

            int8_t data8 = 0;
            int16_t data16 = 0;
            int32_t data32 = 0;
            int64_t data64 = 0;

            slave_startup *startup = &slave_vec[i].startup[startup_i];
            int ref;
            switch (startup->type) {
            case 0:
                udata8 = startup->value;
                ref = ecrt_master_sdo_download(etcMaster,slave->position,startup->index,startup->subindex,(uint8_t *)&udata8,1,&abort_code);
                break;
            case 1:
                data8 = startup->value;
                ref = ecrt_master_sdo_download(etcMaster,slave->position,startup->index,startup->subindex,(uint8_t *)&data8,1,&abort_code);
                break;
            case 2:
                udata16 = startup->value;
                ref = ecrt_master_sdo_download(etcMaster,slave->position,startup->index,startup->subindex,(uint8_t *)&udata16,2,&abort_code);
                break;
            case 3:
                data16 = startup->value;
                ref = ecrt_master_sdo_download(etcMaster,slave->position,startup->index,startup->subindex,(uint8_t *)&data16,2,&abort_code);
                break;
            case 4:
                udata32 = startup->value;
                ref = ecrt_master_sdo_download(etcMaster,slave->position,startup->index,startup->subindex,(uint8_t *)&udata32,4,&abort_code);
                break;
            case 5:
                data32 = startup->value;
                ref = ecrt_master_sdo_download(etcMaster,slave->position,startup->index,startup->subindex,(uint8_t *)&data32,4,&abort_code);
                break;
            case 6:
                udata64 = startup->value;
                ref = ecrt_master_sdo_download(etcMaster,slave->position,startup->index,startup->subindex,(uint8_t *)&udata64,8,&abort_code);
                break;
            case 7:
                data64 = startup->value;
                ref = ecrt_master_sdo_download(etcMaster,slave->position,startup->index,startup->subindex,(uint8_t *)&data64,8,&abort_code);
                break;
            default:
                break;
            }


            if (ref == 0){
                qDebug() << slave->name << startup->info << "done";
            }else{
                qDebug() << slave->name << startup->info << "error";
            }
        }
        int size = 0;
        for(int i = 0;i < slave->output.index.size();i++){
            size = size + slave->output.index[i].pdo.size();
        }
        conf->outNum = size;
        for(int i = 0;i < slave->input.index.size();i++){
            size = size + slave->input.index[i].pdo.size();
        }
        conf->inNum = size - conf->outNum;
        qDebug() << slave->name << "pdo size " << size ;
        int index_Size = slave->output.index.size() + slave->input.index.size();
        conf->offset = (uint *)malloc(size *sizeof(uint));
        conf->domain1_regs = (ec_pdo_entry_reg_t *)malloc(size * sizeof(ec_pdo_entry_reg_t));
        conf->device_pdo_entries = (ec_pdo_entry_info_t *)malloc(size * sizeof(ec_pdo_entry_info_t));
        conf->device_syncs = (ec_sync_info_t *)malloc(5 * sizeof(ec_sync_info_t));

        conf->device_pdos = (ec_pdo_info_t *)malloc((index_Size + 1) * sizeof(ec_pdo_info_t));
//        //ec_pdo_entry_info_t *info = conf_info->info;
//        //conf->pdoname = (char *)malloc(5 * 64);
        memset(conf->pdoname,0,size * 64);
        struct name64{
          char cstr[64];
        };
        name64 *name = (name64 *)conf->pdoname;

        //sprintf(name[0].cstr,"DO-CH1");
        //sprintf(name[1].cstr,"DO-CH2");
        //sprintf(name[2].cstr,"DI-CH1");
        //sprintf(name[3].cstr,"DI-CH2");
        int pdoNo = 0;
        int indexNo = 0;
        for(int i = 0;i < slave->output.index.size();i++){
            slave_index *index = &(slave->output.index[i]);
            conf->device_pdos[indexNo] = {index->index, index->pdo.size(), conf->device_pdo_entries + pdoNo };
            for(int k = 0; k < index->pdo.size();k++){
                slave_pdo *pdo = &(index->pdo[k]);
                //sprintf(name[pdoNo].cstr,pdo->info.data());
                //memcpy(name[pdoNo].cstr,pdo->info.data(),pdo->info.size());
                memcpy(name[pdoNo].cstr,pdo->info.toUtf8().data(),pdo->info.toUtf8().size());
                conf->domain1_regs[pdoNo] = {conf->alias,conf->position, conf->vid,conf->pid, pdo->index, pdo->sub_index, &conf->offset[pdoNo]};
                conf->device_pdo_entries[pdoNo] = {pdo->index, pdo->sub_index, pdo->size * 8};
                conf->info[pdoNo] = {pdo->index, pdo->sub_index, pdo->size * 8};
                conf->pdo_dir[pdoNo] = PDO_OUTPUT;
                qDebug() << "pdo : " << HEX_OUT(pdo->index) << pdo->sub_index << pdo->info;
                pdoNo++;
            }
            qDebug() << "index : " << HEX_OUT(index->index)  << index->info;
            indexNo++;
        }

        for(int i = 0;i < slave->input.index.size();i++){
            slave_index *index = &(slave->input.index[i]);
            conf->device_pdos[indexNo] = {index->index, index->pdo.size(), conf->device_pdo_entries + pdoNo };
            for(int k = 0; k < index->pdo.size();k++){
                slave_pdo *pdo = &(index->pdo[k]);
                //sprintf(name[pdoNo].cstr,pdo->info.data());
                //memcpy(name[pdoNo].cstr,pdo->info.data(),pdo->info.size());
                memcpy(name[pdoNo].cstr,pdo->info.toUtf8().data(),pdo->info.toUtf8().size());
                conf->domain1_regs[pdoNo] = {conf->alias,conf->position, conf->vid,conf->pid, pdo->index, pdo->sub_index, &conf->offset[pdoNo]};
                conf->device_pdo_entries[pdoNo] = {pdo->index, pdo->sub_index, pdo->size * 8};
                conf->info[pdoNo] = {pdo->index, pdo->sub_index, pdo->size * 8};
                conf->pdo_dir[pdoNo] = PDO_INPUT;
                qDebug() << "pdo : " << HEX_OUT(pdo->index) << pdo->sub_index << pdo->info;
                pdoNo++;
            }
            qDebug() << "index : " << HEX_OUT(index->index)  << index->info;
            indexNo++;
        }


        conf->device_syncs[0] = { 0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE };
        conf->device_syncs[1] = { 1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE };
        conf->device_syncs[2] = { 2, EC_DIR_OUTPUT, slave->output.index.size(), conf->device_pdos + 0, getWD(slave->output.WD)};
        conf->device_syncs[3] = { 3, EC_DIR_INPUT, slave->input.index.size(), conf->device_pdos + slave->output.index.size(), getWD(slave->input.WD)};



        conf->device_syncs[4] = { 0xFF};

        conf->pdoSize = size;
        conf->indexSize = index_Size;
    }
    for(int i = 0; i < axis_vec.size();i++){
        //ecm->axis[i].pdoNo = axis_vec[i].pdoNo;
        axis_struct_init(&ecm->axis[i]);
        if(ecm->axis[i].pdoNo < 0){
            ecm->axis[i].pdoNo = -1;
        }else{
            ecm->axis[i].pdoNo = axis_vec[i].pdoNo - 1;
        }
        ecm->axis[i].ratio_deno = axis_vec[i].enc_ratio;
        ecm->axis[i].ratio_num = 1;
        ecm->axis[i].enc_ratio = axis_vec[i].enc_ratio;
        ecm->axis[i].circle_diff = axis_vec[i].circle_diff;
        ecm->axis[i].press_enable = axis_vec[i].press_enable;
        memcpy(ecm->axis[i].name,axis_vec[i].name.toStdString().c_str(),axis_vec[i].name.toStdString().length());
    }

    ecrt_release_master(etcMaster);
    mem_insertLog(1,"EtherCatMaster","配置读取成功");
    return;
}






















void EtherCatMaster::on_start_clicked()
{
    ethercatRun();
    //MESSAGE_INFO("error","get shm done!");
}

void EtherCatMaster::on_stop_clicked()
{
    //ecrt_release_master(master);
    if (start == false){
        return;
    }
    QMessageBox::StandardButtons ret = QMessageBox::information(this,"info","close master , please click yes to close",QMessageBox::Yes,QMessageBox::No);
    if(ret == QMessageBox::No){
        return;
    }
    start = false;
}

void EtherCatMaster::on_t250_clicked()
{
    if (!start){
        period_ns = 250 * 1000;
        cycletime = {0, period_ns};
    }
}

void EtherCatMaster::on_t500_clicked()
{
    if (!start){
        period_ns = 500 * 1000;
        cycletime = {0, period_ns};
    }
}

void EtherCatMaster::on_t1000_clicked()
{
    if (!start){
        period_ns = 1000 * 1000;
        cycletime = {0, period_ns};
    }
}

void EtherCatMaster::on_movevel_pressed()
{
    movevel = true;
}

void EtherCatMaster::on_movevel_released()
{
    movevel = false;
}

void EtherCatMaster::on_movestop_pressed()
{
    movestop = true;
}

void EtherCatMaster::on_movestop_released()
{
    movestop = false;
}

void EtherCatMaster::on_vel_vel_clicked()
{
    getdouble(&vel_vel,-50,1000000);
}

void EtherCatMaster::on_vel_acc_clicked()
{
    getdouble(&vel_acc,0.1,1000000);
}

void EtherCatMaster::on_vel_dec_clicked()
{
    getdouble(&vel_dec,0.1,1000000);
}

void EtherCatMaster::on_vel_jerk_clicked()
{
    getdouble(&vel_jerk,0.1,10000000);
}

void EtherCatMaster::on_stop_dec_clicked()
{
     getdouble(&stop_dec,0.1,10000);
}

void EtherCatMaster::on_stop_jerk_clicked()
{
    getdouble(&stop_jerk,0.1,10000000);
}

void EtherCatMaster::on_abs_pos_clicked()
{
    getdouble(&abs_pos,-10000000000,10000000000);
}



void EtherCatMaster::on_moveabs_pressed()
{
    moveabs = true;
}

void EtherCatMaster::on_moveabs_released()
{
    moveabs = false;
}

void EtherCatMaster::on_wtmc_pressed()
{
    wtmc = true;
}

void EtherCatMaster::on_wtmc_released()
{
    wtmc = false;
}

void EtherCatMaster::on_stop_dec_2_clicked()
{
    getdouble(&pkp,0,100000);

}

void EtherCatMaster::on_stop_dec_3_clicked()
{
    getdouble(&vkp,0,100000);
}

void EtherCatMaster::on_stop_dec_4_clicked()
{
    getdouble(&vki,0,100000);
}

void EtherCatMaster::on_resret_pressed()
{
    reset = true;
}

void EtherCatMaster::on_resret_released()
{
    reset = false;
}

void EtherCatMaster::on_wtcsp_pressed()
{
    wtcsp = true;
}

void EtherCatMaster::on_wtcsp_released()
{
    wtcsp = false;
}

void EtherCatMaster::on_pos_kd_clicked()
{
    getdouble(&pkd,0,100000);
}









int EtherCatMaster::getCoreConf(kv_pair *arry)
{
    for(int i = 0;i < MAX_KV_PAIRS;i++){
        arry[i].key = "";
        arry[i].value =  -1;
    }
    QFile confFile(CONFIG_FILE);
    if(!confFile.open(QIODevice::ReadOnly)){
        return -1;
    }
    int arryIndex = 0;
    while(true){
        if(confFile.atEnd()){
            break;
        }
        QString line = confFile.readLine(256);
        auto lineList = line.split("=");
        if(lineList.size() != 2){
            continue;
        }
        arry[arryIndex].key = lineList[0];
        arry[arryIndex].value = lineList[1].toUInt();
        arryIndex++;
        if(arryIndex >= MAX_KV_PAIRS){
            break;
        }
    }
    return 0;
}


















