 #ifndef ETHERCATMASTER_H
#define ETHERCATMASTER_H

#include <QMainWindow>


#include <QDebug>
#include <QSharedMemory>
#include <QMessageBox>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sched.h> /* sched_setscheduler() */
#include <pthread.h>
#include <sys/socket.h>

#include <QDialog>
#include <QInputDialog>
#include <QCloseEvent>
#include <QTimer>
#include <QUdpSocket>
#include "ecrt.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QVector>
#include <QFile>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMutex>
#include <QUdpSocket>
//#include <QDir>
#include "ecrt.h"
#include "etcHeader/etcheader.h"
#include "etcHeader/jmc_axis_cmd.h"

#include "axiswid.h"
#define MAX_KEY_LEN 64
#define MAX_KV_PAIRS 256
#define CONFIG_FILE "/etc/core_config.conf"

struct kv_pair {
    QString key;
    int value;
};

struct axis_pdo{
    int offset;
    void *value;
    int size;
    QString pdo_name;
};
struct axis_pdo_vector{
    QVector<axis_pdo> to_slave;
    QVector<axis_pdo> from_slave;
    bool xVirtual = true;
};

//#define MESSAGE_INFO(title,info) emit message(this,title,info)
#define MESSAGE_INFO(title,info) mem_insertLog(-1,title,info)
//mem_insertLog
#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC + \
        (B).tv_nsec - (A).tv_nsec)

#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
#define NSEC_PER_SEC (1000000000L)




namespace Ui {
class EtherCatMaster;
}
struct tc_jmc{
    JMC_Power power;
    JMC_MoveAbs moveabs;
    JMC_Reset reset;
    //JMC_BU5_TC bu5tc;
    JMC_WTMC_TC wmtc;
    JMC_MoveAbs moveabs_study;
};
class EtherCatMaster : public QMainWindow
{
    Q_OBJECT

public:
    explicit EtherCatMaster(QWidget *parent = nullptr);
    ~EtherCatMaster();
    //QSharedMemory shm;
    int getDrvPodConf(    uint16_t alias, /**< Slave alias address. */
                          uint16_t position, /**< Slave position. */
                          uint32_t vid, /**< Slave vid. */
                          uint32_t pid, /**< Slave pid. */
                          pod_config *conf);
    int getNormalConf(   uint16_t alias, /**< Slave alias address. */
                         uint16_t position, /**< Slave position. */
                         uint32_t vid, /**< Slave vid. */
                         uint32_t pid, /**< Slave pid. */
                         void *arg,
                         pod_config *conf);
    int freeConf(pod_config *conf);
    int etc_run_cycle();
    static void tcp_cmd_thread(EtherCatMaster *arg);


    int tcp_cmd(int cmd , QTcpSocket *obj);
    //pod_config slave_pdo_conf[MAX_SLAVE];
    cec_shm_pdo *ecm;
    void closeEvent(QCloseEvent *e);
    QSystemTrayIcon ticon;
    QMutex logMtx;
    QVector<mem_info> logVec;
    QUdpSocket udpSock;

    int CRTtools_timeOut = 0;
    void send_log();
    //static pthread_t pth;
public slots:
    void message_show(QObject *father,QString title,QString info);
    void mem_insertLog(int type,QString name,QString info);


signals:
    void message(QObject *father,QString title,QString info);
private:
    Ui::EtherCatMaster *ui;
    void *etcinit(void);
    int ETCfd = 0;
    void initShmData(void *data);
    unsigned int period_ns = 250000;
    uint8_t *domain1_pd;

    QSharedMemory shm_axdata;
    QString shm_name = "ETCAX";
    // EtherCAT
    ec_master_t *master = NULL;
    ec_master_state_t master_state = {};

    ec_domain_t *domain1 = NULL;
    ec_domain_state_t domain1_state = {};

    ec_slave_config_t *sc = NULL;
    ec_slave_config_state_t sc_state = {};

    unsigned int counter = 0;
    unsigned int blink = 0;
    unsigned int sync_ref_counter = 0;
    struct timespec cycletime = {0, period_ns};

    pthread_t ec_sync;
    pthread_t tcp_pth;

    //QUdpSocket sock;
    QTcpServer server;
    QVector<QTcpSocket *> clients;
    static void *call_sync(void *arg);
    void cyclic_task();
    void oneinotest();

    void read_config_by_json();


    void syncPod_Read(pod_config *pdo);
    void syncPod_Write(pod_config *pdo);
    //void axis_bind_pdo(axis_ref *axis,)

    void check_domain1_state();
    void check_master_state(void);
    void check_slave_config_states(void);
    // axis_fun
    void syncAxis_Ref(axis_ref *axis);
    void syncAxis_StateChange(axis_ref *axis);
    void axis_i_f_pos_sync(axis_ref *axis,double pos);
    void axis_update_factpos(axis_ref *axis);
    void axis_update_isetPos(axis_ref *axis);
    void axis_set_fpos(axis_ref *axis,double pos);
    void axis_pdo_read(axis_ref *axis);
    void axis_pdo_write(axis_ref *axis);
    void axis_struct_init(axis_ref *axis);


    int axis_cmd_vel_cyc(axis_ref *axis);
    int axis_cmd_stop_cyc(axis_ref *axis);
    int axis_cmd_abs_cyc(axis_ref *axis);
    int axis_wtmct_sync(axis_ref *axis);
    int axis_bu5_tc_sync(axis_ref *axis);

    velo_Planing vel_plan[MAX_AXIS];
    tc_jmc tc_jmc_ary[MAX_AXIS];

    axis_pdo_vector pdos_vector[MAX_SLAVE];
    int tc_sync_call_680N(axis_ref *axis,tc_jmc *tc_jmc_p);
    bool xCloseSig = false;
    bool xShmRet = false;
// test Prg
private:
    JMC_Power c_power;
    JMC_MoveVel c_movevel;
    JMC_Stop c_stop;
    JMC_MoveAbs c_abs;
    JMC_WTMC_TC c_wtmc;

    JMC_WTMC_TC c_wtcsp;

    JMC_Reset c_reset;

    QTimer _10ms_timer;
    QTimer _100ms_timer;

    int actCount = 0;
    double movetime = 0;
    int c_state = 0;
    bool start = false;
    bool movevel = false;
    bool movestop = false;
    bool moveabs = false;
    bool wtmc = false;
    bool wtcsp = false;

    bool reset = false;

    double vel_vel = 49;
    double vel_acc = 3000;
    double vel_dec = 3000;
    double vel_jerk = 10000;

    double stop_dec = 3000;
    double stop_jerk =1000;

    double abs_pos = 10;
    double abs_vel = 10;
    double abs_acc = 10;
    double abs_dec = 10;
    double abs_jerk = 1000;

    uint time = 0;

    double pkp = 0;
    double pkd = 0;
    double vkp = 0.001;
    double vki = 0;
    uchar testCount = 0;
    int IO_counter = 0;
    void getdouble(double *dest,double min,double max);
    void ethercatRun();
    bool test = false;
    QMenu iconMenu;

    static int getCoreConf(kv_pair *arry);
private slots:
    void _10ms_fn();
    void _100ms_fn();
    void ec_udp();
    void tcp_new();
    void tcp_recv();
    void tcp_disc();
    void on_start_clicked();
    void on_stop_clicked();
    void on_t250_clicked();
    void on_t500_clicked();
    void on_t1000_clicked();
    void on_movevel_pressed();
    void on_movevel_released();
    void on_movestop_pressed();
    void on_movestop_released();
    void on_vel_vel_clicked();
    void on_vel_acc_clicked();
    void on_vel_dec_clicked();
    void on_vel_jerk_clicked();
    void on_stop_dec_clicked();
    void on_stop_jerk_clicked();
    void on_abs_pos_clicked();

    void on_moveabs_pressed();
    void on_moveabs_released();
    void on_wtmc_pressed();
    void on_wtmc_released();
    void on_stop_dec_2_clicked();
    void on_stop_dec_3_clicked();
    void on_stop_dec_4_clicked();
    void on_resret_pressed();
    void on_resret_released();
    void on_wtcsp_pressed();
    void on_wtcsp_released();
    void on_pos_kd_clicked();
};
struct timespec timespec_add(struct timespec time1, struct timespec time2);
struct chrtimer_req {
    clockid_t clock_id;  // 例如 CLOCK_MONOTONIC
    int flags;           // 如 TIMER_ABSTIME 标志
    struct timespec time; // 定时时间 (绝对或相对)
};

#endif // ETHERCATMASTER_H
