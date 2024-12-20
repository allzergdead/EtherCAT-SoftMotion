/**********************************************************************************************
# last Change :
# $vision : 0.1$
# $Author : Tom Chang $
# $PRG_LICENSE : Proprietary
# $JRS ETHCAT User space code , using IGH 1.5 and Claser device v0.1
# $
# ********************************************************************************************/
#ifndef ETCHEADER_H
#define ETCHEADER_H

#include "ecrt.h"
#include "mutex"
#include <QHash>
#include <QVector>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>                   //mmap head file
#include <unistd.h>
#include <cstdio>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <math.h>
#include <mutex>
#include <sstream>

// QT JSON
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QVariant>
// MAP
#include <QHash>
#include <QVector>
//#include <QMutex>

#include <QDebug>

#define ETC_SHM_SIZE 1024 * 1024 * 3
#define ETC_FILE "etcshm"
#define PT_SHM "PT_SHM"

#define MAX_AX_PT 40000

#define ETC_CONF_PORT 8890
#define MAX_PDO_SIZE 40
#define MAX_SLAVE 256
#define MAX_AXIS 64

#define PDO_INPUT 0
#define PDO_OUTPUT 1

#define SETPACK __attribute__((aligned(8)))

#define getQStrHex(x) QString::number(x,16)
#define toJosnVal(x) QJsonValue::fromVariant(QVariant::fromValue(x))
struct axis_Cmd;
struct axis_ref;
struct etc_shm;
struct axis_offset;
struct pod_config;
struct slave_pdo_conf_info;
struct cec_shm_pdo;
extern QHash<QString ,axis_ref*> axis_hash;
struct slave_pdo;
struct slave_index;
struct slave_assign;
struct slave_config;
class CPid;
//extern inline int appendAxisHash(QString name,axis_ref* axis);
//extern inline bool axis_can_move(axis_ref *axis);
//extern inline bool axis_in_act(axis_ref *axis,uint cmd);
//extern inline uint get_new_act(axis_ref *axis);
//ref = ecrt_master_sdo_download(master,axis->pdoNo,0x2003,0xB,(uint8_t *)&DI_FUN,2,&abort_code);


static int insertLog(int type,QString name,QString info)
{
    qDebug() << type << name << info;
    return 0;
}
class SETPACK CPid{
public :
    double setValue = 0;
    double actValue = 0;
    double Kp = 10;
    double Ki = 0.1;
    double Kd = 0;

    double Yp = 0;
    double Yi = 0;
    double Yd = 0;
    double Y = 0;

    double delta = 0;
    double isum = 0;

    double Yp_max = 30;
    double Yi_max = 30;
    double Yd_max = 30;
    double Y_max = 30;

    double Yp_min = -30;
    double Yi_min = -30;
    double Yd_min = -30;
    double Y_min = -30;

    bool xReset = false;
    bool xEnable = false;
    bool xError = false;

    bool xOldEnable = false;
    //bool xOldReset = false;

    int state = 0;
    int exec(double set ,double act);
    bool xRuning = false;
};


struct SETPACK axis_config
{
    QString name;
    uint enc_ratio = 1048576;           //
    double circle_diff = 1;
    int pdoNo = -1;
    bool press_enable = false;
};
struct SETPACK slave_startup
{
    uint index;
    uint subindex;
    uint size;
    long value;
    uint type;
    QString info;
};
struct SETPACK slave_pdo{
    uint index;
    uint sub_index;
    uint size;
    QString info;

};
struct SETPACK slave_index{
    QVector<slave_pdo> pdo;
    uint index;
    QString info;
};
struct SETPACK slave_assign{
    QVector<slave_index> index;
    QString info;
    uint WD = 0;
};
struct SETPACK slave_config{
    slave_assign MbxOut;
    slave_assign MbxIn;
    slave_assign output;
    slave_assign input;

    QVector<slave_startup> startup;
    QString name;

    uint16_t alias;/**< Slave alias address. */
    uint16_t position; /**< Slave position. */
    uint32_t vid; /**< Slave vid. */
    uint32_t pid; /**< Slave pid. */

};
enum SETPACK axis_state{
    power_off,// PLCOPEN call Disabled
    errorstop,
    stopping,
    standstill,
    discrete_motion,
    continuous_motion,
    synchronized_motion,
    homing
};
enum SETPACK cmd_type{
    null,
    poweron,
    absmove,
    realmove,
    vel,
    jog,
    stop,
    home,
    setpos,
    resetaxis,
    setwtmct,
    bu5_tc
};
struct SETPACK axis_status
{
    bool ready;
    bool switch_on;
    bool operation_enabled;
    bool fault;
    bool voltage_enabled;
    bool quick_stop;
    bool switch_on_disabled;
    bool warming;
    bool manufacturer_spec0;
    bool remote;
    bool target_reach;
    bool internal_limit_active;
    bool op_mode_spec0;
    bool op_mode_spec1;
    bool manufacturer_spec1;
    bool home_find;
};
struct SETPACK kenrel_TaskInfo_etc{
    int writing;
    char name[256];
    unsigned long task_countor;
    //unsigned long task_cost_target;
    unsigned long task_cost;
    unsigned long task_cost_avg;
    unsigned long task_cost_max;
    unsigned long task_cost_min;
    unsigned long task_cost_sum;

    unsigned long task_period;
    unsigned long task_period_avg;
    unsigned long task_period_max;
    unsigned long task_period_min;
    unsigned long task_period_sum;

    unsigned long task_outof_sync;

    unsigned long task_interrupt_delay;

    unsigned int cpu;
    unsigned int clear;
};
struct SETPACK axis_data{
    long num;
    double time[MAX_AX_PT];
    double pos[MAX_AX_PT];
    double vel[MAX_AX_PT];
    double acc[MAX_AX_PT];
    double torque[MAX_AX_PT];
};
struct SETPACK axis_pid{
    double kp;
    double ki;
    double kd;

    double delta;
    double delta_old;
    double yp;
    double yi;
    double yd;

    double pool_i;

    double set;
    double act;

    double y;

    bool reset;
    double p_dead_area;

    double act_old;
    double act_a;
};
struct SETPACK axis_cmd_vel{
    double vel;
    double acc;
    double dec;
    double jerk;
    bool invel;
    bool error;

    double setVel;
    double time;
    uint errorid;
};
struct SETPACK axis_cmd_stop{
    double dec;
    double jerk;
    bool done;
    bool error;

    double setVel;


    uint errorid;
};
struct SETPACK axis_cmd_abs{
    double pos;
    double vel;
    double acc;
    double dec;
    double jerk;
    bool done;
    bool error;
    double setVel;

    double time;


    uint errorid;
};
struct SETPACK axis_cmd_setpos{
    double pos;

    bool done;
    bool error;

    uint errorid;
};
struct SETPACK axis_cmd_drv_home{
    double timeOut = 60;

    double timeCount = 60;
    bool done;
    bool error;

    uint home_count;
    uint old_type;
    uint errorid;
};
struct SETPACK axis_cmd_bu5_tc{
    double zero_pos;
    double spr2_pos;

    double change_pos;
    double chip_deepst_pos;
    double chip_target_pos;



    double chip_protect_limit;
    double chip_protect_torque;
    int chip_protect_count;
    int chip_protect_act_count;

    double chip_pressure_pos_limit;
    int chip_pressure_pos_count;
    int chip_pressure_pos_act_count;


    double keep_torque;
    double keep_k;
    bool keep_enbale;

    int conf;

    int chip_protect_neg_count;
    int chip_protect_neg_act_count;
    int chip_protect_neg_torque;

    double vel;
    double end_vel;
    double acc;
    double dec;
    double jerk;
    double pressure;

    double torque_a;
    double torque_filter;
    double time;
    int state;
    bool done;
    bool error;

    int torque_start;
    int torque_count;
    int torque_delay;

    bool torque_change_enable;
    int torque_change_count;
    int torque_change_delay;
    int torque_change_add;
    int torque_change_time;
    double torque_change_pos;
    CPid torque_change_pid;

    double torque_act_pos;

    int timecount;

    int act_1s_count;

    double t_a;

    int axis_type;

    double step_1_time;
    double step_1_pos;
    double step_1_vel;
    double step_1_torque;
    double step_1_avgtorque;
    uint errorid;

    bool auto_change_mode;
    double auto_change_delay;
    double auto_change_count;

    double study_torque;
    double study_pos;
    int study_type;

    int study_delay;
    int study_count;

    double study_set_torque;
    double study_set_target_pos;
    double study_set_change_pos;

    int study_set_protect_count;
    double study_set_protect_torque;

    int study_set_t_protect_count;
    double study_set_t_protect_pos;
    //double study_set_;
};
struct SETPACK axis_Cmd{
    uint act_Now = 0;
    uint act_Runing = 0;
    uint act_Old = 0;





    double freal_pos;
    double freal_vel;
    double freal_acc;
    double freal_dec;
    double freal_jerk;
    bool xreal_done;
    bool xreal_error;


    double fjog_vel;
    double fjog_acc;
    double fjog_dec;
    double fjog_jerk;
    int fmanual_dir;
    bool xjog_error;



    double fsetpos_pos;
    bool xset_done;
    bool xset_error;

    bool xpower_drvstart = false;
    bool xpower_on = false;

    bool xpower_inpower;


    bool xReset;
    bool xreset_error;



    bool xDone;
    bool xError;
    uint errorID;


    short t_fsetacc;
    short t_v_pid;
    short t_p_pid;
    short t_miniT;
    //double pos_delta_sum;
    int pidCount;
    char cmdRet[256];
    int cmdFlush;



    axis_cmd_vel vel;

    axis_cmd_stop stop;

    axis_cmd_abs abs;

    axis_cmd_bu5_tc bu5_tc;

    axis_cmd_setpos setpos;

    axis_cmd_drv_home dev_home;

    cmd_type type_Runing = cmd_type::null;

    axis_pid pos_pid;
    axis_pid vel_pid;
    axis_pid pv_pid;

    char space0[200];
    std::mutex mtx;
    char feed0[80];

};
struct SETPACK tc_ref{
    uint tc_state;
    char tc_mcst;

    double down_vel = 95;
    double down_acc = 200000;
    double down_dec = 10000;
    double down_jerk = 20000000;
    double down_endVel = 0.4;
    double down_pressure = 50;

    double up_vel = 95;
    double up_acc = 50000;
    double up_dec = 50000;
    double up_jerk = 20000000;



    bool io_power;
    bool io_down;
    bool io_up;
    bool io_reset;

    bool ui_power;
    bool ui_down;
    bool ui_up;
    bool ui_reset;
    bool ui_study;

    bool io_power_old;
    bool io_down_old;
    bool io_up_old;
    bool io_reset_old;

    bool ui_power_old;
    bool ui_down_old;
    bool ui_up_old;
    bool ui_reset_old;
    bool ui_study_old;

    long up_time;
    long down_time;
    long change_time;
    long wait_time;

    timespec start_tim;
    timespec end_tim;

};
struct SETPACK axis_ref{
    uint tpID = 0xae;                   // etc struct typeid
    char name[64];
    axis_state state;                   // axis_ref plcopen state
    char feed0[4];
    ushort status_word;                   // axis_ref stateword

    int bindEtc = -1;                   // bind etc axis
    int axNo = 0;
    int syncState;
    //bool xDriveStart = false;           //
    //bool xPowerOn = false;
    bool xCommunication;                // ETC Communication on
    uint wCommunicationState;           // ETC CommunicationState

    double fETC_CycleTime;              // axis ETC cycletime
    bool xError;                        // axis error
    uint uiErrorID;                     // axis errorid

    uint errorAry[10];                  // buffer axis errors

    uint enc_ratio = 1048576;           //
    uint ratio_deno = 1048576;          //
    uint ratio_num = 1;                 //
    double circle_diff = 1;

    uint offset_ipos = 0;
    uint offset_itrun = 0;

    uint target_mode = 0x08;
    double offset_pos = 0;
    double offset_turn_unit;
    double fSetPos;                     // CSP set pos
    double fActPos;                     // CSP act pos

    double fSetVel;                     // CSP set vel
    double fActVel;                     // CSP act vel
    double fMaxVel;                     // if fSetVel > fMaxVel or fActVel > fMaxVel axis_ref will stop and run to errorstop

    double fSetAcc;                     // CSP set acc
    double fActAcc;                     // CSP act acc
    double fMaxAcc;                     // if fSetAcc > fMaxAcc or fActAcc > fMaxAcc axis_ref will stop and run to errorstop

    double fSetTorque;                  // CSP set torque
    double fActTorque;                  // CSP act torque
    double fMaxTorque;                  // if fSetTorque > fMaxTorque or fActTorque > fMaxTorque axis_ref will stop and run to errorstop
    short sSetTorque;

    int following_error_actual_value;
    double fRatedTorque = 0.32;

    int iSetPos;                        // CSP set pos
    int iActPos;                        // CSP act pos

    //int iSetTorque;                        // CSP set pos
    short sActTorque;

    int iOldSetPos;                     // CSP set pos
    int iOldActPos;                     // CSP act pos



    int iTurn = 0;


    long lActPos = 0;
    long lSetPos = 0;
    long lOffsetPos = 0;

    long lOldSetPos = 0;
    long lOldActPos = 0;

    double fOldActPos;
    double fOldActVel;

    double fOldSetPos;
    double fOldSetVel;


    int pdoNo = -1;
    uint modes_of_operation = 0x08;
    uint errorcode;
    uint dinput;
    char modes_of_operation_display;

    ushort ctrword;

    void *plan;
    int axis_wtm_type = 0;
    double drv_j = 0.0000041;
    int doneCount;
    int posequaCount;
    int posequaCount_p;
    int iActPosJY;

    int check_times;
    double check_area;


    uint digital_inputs;
    uint digital_outputs;

    bool press_enable = false;


    bool tc_i_power = false;
    bool tc_i_up = false;
    bool tc_i_down = false;
    bool tc_i_reset = false;

    bool tc_o_0 = false;
    bool tc_o_1 = false;
    bool tc_o_2 = false;


    bool xOldCommunication = false;
    bool xFristTime = false;


    uint tc_input_uint;
    uint tc_output_uint;
    axis_status status_bit;

    axis_Cmd cmd;
    tc_ref stu_tc_ref;
};
struct etc_shm{

};
struct SETPACK axis_offset{
    unsigned int ctrl_word;
    unsigned int operation_mode;
    unsigned int target_position;
    unsigned int target_torque;
    unsigned int digital_outputs;
    unsigned int error_code;
    unsigned int status_word;
    unsigned int modes_of_operation_display;
    unsigned int position_actual_value;
    unsigned int torque_actual_velue;
    unsigned int following_error_actual_value;
    unsigned int digital_inputs;
};

struct SETPACK pod_config{
    ec_pdo_entry_reg_t *domain1_regs;
    ec_pdo_entry_info_t *device_pdo_entries;
    ec_pdo_info_t *device_pdos;
    ec_sync_info_t *device_syncs;
    unsigned int *offset;
    uint inNum = 0;
    uint outNum = 0;

    uint16_t alias;/**< Slave alias address. */
    uint16_t position; /**< Slave position. */
    uint32_t vid; /**< Slave vid. */
    uint32_t pid; /**< Slave pid. */

    ec_slave_config_t *sc = NULL;

    char pdo_data[400];
    ec_pdo_entry_info_t info[60];
    int pdo_dir[60];
    char pdoname[4000];
    char slaveName[64];

    uint assign = 0x300;
    ec_slave_config_state_t slave_state;
    ec_master_t *master;

    uint pdoSize;
    uint indexSize;

    bool syncWrite;
};

//#define EC_PDO_INFO  ec_pdo_entry_info_t
struct SETPACK slave_pdo_conf_info{
    uint inNum = 0;
    ec_watchdog_mode_t inWD;
    uint outNum = 0;
    ec_watchdog_mode_t outWD;
    ec_pdo_entry_info_t *info;
    int dir[100];
    char *pdoName;
};
struct SETPACK mem_info
{
    int type;
    QString name;
    QString info;
};
struct SETPACK cec_shm_pdo{
    int pdoSize = 0;

    int pdo_Num = 0;
    int axis_Num = 0;

    unsigned int etc_state = 0;

    unsigned long etc_hreat_counter = 3343;
    unsigned long etc_task_counter = 243;

    unsigned long CRTtools_counter = 5654;
    unsigned long CRTtools_counter_old = 3434;
    axis_ref axis[MAX_AXIS];
    kenrel_TaskInfo_etc task_info;
    uint ecNo = 0;

    char taskinfo[5000];
    pod_config pdoconf[MAX_SLAVE];
};
struct SETPACK cec_shm_write{

    int beUse1 = 0;
    int beUse2 = 0;
    axis_Cmd cmd1[MAX_AXIS];
    axis_Cmd cmd2[MAX_AXIS];
    uint target_mode1[MAX_AXIS] = {0x08};
    uint target_mode2[MAX_AXIS] = {0x08};
    uint ecNo = 0;
    char taskinfo[5000];

    //char _2M[2048 * 1024];
};
class SETPACK ec{
public :

    static bool axis_in_act(axis_ref *axis,uint cmd);
    static bool axis_can_home(axis_ref *axis);
    static bool axis_can_change_move(axis_ref *axis);
    static bool axis_can_setpos(axis_ref *axis);
    static bool axis_can_reset(axis_ref *axis);
    static bool axis_can_stop(axis_ref *axis);
    static bool axis_can_bu5_tc(axis_ref *axis);



    static int get_new_act(axis_ref *axis,cmd_type type,uint *act);
    static void set_axis_cmdtype(axis_ref *axis,cmd_type type);
    static axis_ref *get_axis(QString name);
    static int appendAxisHash(QString name,axis_ref* axis);

};
extern QString last_json_error;
int etc_vector2json(QString path , QVector<slave_config> *vec, QJsonObject *obj);
int etc_json2vector(QString path , QJsonObject *obj, QVector<slave_config> *vec);

int axis_vector2json(QString path , QVector<axis_config> *axes, QJsonObject *obj);
int axis_json2vector(QString path , QJsonObject *obj, QVector<axis_config> *axes);

QJsonObject etc_pdo2json(slave_pdo *pdo ,int *_ret);
QJsonObject etc_index2json(slave_index *index,int *_ret);
QJsonObject etc_assign2json(slave_assign *assign,int *_ret);
QJsonObject etc_startup2json(slave_startup *startup,int *_ret);
QJsonObject etc_slave2json(slave_config *slave,int *_ret);

slave_config etc_json2jslave(QJsonObject obj,int *_ret);
slave_assign etc_json2assign(QJsonObject obj,int *_ret);
slave_index etc_json2index(QJsonObject obj,int *_ret);
slave_pdo etc_json2pdo(QJsonObject obj,int *_ret);
slave_startup etc_json2startup(QJsonObject obj,int *_ret);




//bool status_ready(uint status);
//bool status_switch_on(uint status);
//bool status_enabled(uint status);
//bool status_fault(uint status);
//bool status_voltage_enabled(uint status);
//bool status_quick_stop(uint status);
//bool status_disable(uint status);
//bool status_warning(uint status);
//bool status_facture_spec(uint status);
//bool status_remote(uint status);
//bool status_target_reach(uint status);
//bool status_internal_limit(uint status);

#endif // ETCHEADER_H
