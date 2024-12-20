#ifndef JMC_AXIS_CMD_H
#define JMC_AXIS_CMD_H
#include "etcheader.h"
#include "../velplan/s7_vel_plan.h"

#include <QDebug>
class JMC_Axis_Cmd{
public :
    int state = 0;
    bool done = false;
    bool error = false;
    bool aborted = false;
    bool busy = false;
    bool oldExecute = false;
    uint actNo = 0;
    uint errorCode = 0;
    //virtual void call(QString axis_name,bool xExecute,double pos,double vel,double acc,double dec,double jerk);
    //virtual void call(axis_ref *axis,bool xExecute,double pos,double vel,double acc,double dec,double jerk);
    //virtual void setParam(axis_ref *axis,double pos,double vel,double acc,double dec,double jerk);
    void out_clear();
    void out_error(uint errorid);
    void out_done();
    void out_abort();
    void out_busy();

};
//static bool axis_can_change_move(axis_ref *axis);
//static bool axis_can_setpos(axis_ref *axis);
//static bool axis_can_reset(axis_ref *axis);
//static bool axis_can_stop(axis_ref *axis);
struct abs_input{
    bool xExecute;
    double pos;
    double vel;
    double acc;
    double dec;
    double jerk;
};
struct stop_input{
    bool xExecute;
    double dec;
    double jerk;
};
struct bu5_tc_input{
    bool xExecute;double vel;double acc;double dec;double jerk;double end_vel;double pressure;int conf;
};
//JMC_BU5_TC
class JMC_MoveAbs : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xExecute,double pos,double vel,double acc,double dec,double jerk);
    void call(axis_ref *axis,bool xExecute,double pos,double vel,double acc,double dec,double jerk);
    void copyParam(bool xExecute,double pos,double vel,double acc,double dec,double jerk);
    bool Execute = false;
    double fPos = 0;
    double fVel = 0;
    double fAcc = 0;
    double fDec = 0;
    double fJerk = 0;
private:
    int setParam(axis_ref *axis,double pos,double vel,double acc,double dec,double jerk);
};
class JMC_MoveReal : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xExecute,double pos,double vel,double acc,double dec,double jerk);
    void call(axis_ref *axis,bool xExecute,double pos,double vel,double acc,double dec,double jerk);
    bool Execute = false;
    double fPos = 0;
    double fVel = 0;
    double fAcc = 0;
    double fDec = 0;
    double fJerk = 0;
private:
    int setParam(axis_ref *axis,double pos,double vel,double acc,double dec,double jerk);
};
class JMC_MoveVel : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xExecute,double vel,double acc,double dec,double jerk);
    void call(axis_ref *axis,bool xExecute,double vel,double acc,double dec,double jerk);
    void copyParam(bool xExecute,double vel,double acc,double dec,double jerk);
    bool Execute = false;
    double fVel = 0;
    double fAcc = 0;
    double fDec = 0;
    double fJerk = 0;
    bool invel = false;
private:
    int setParam(axis_ref *axis,double vel,double acc,double dec,double jerk);

};

class JMC_Home_Drv : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xExecute,double timeOut);
    void call(axis_ref *axis,bool xExecute,double timeOut);
    bool Execute = false;
    double fTimeOut = 60;
private:
};
class JMC_Stop : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xExecute,double dec,double jerk);
    void call(axis_ref *axis,bool xExecute,double dec,double jerk);
    void copyParam(bool xExecute,double dec,double jerk);
    bool Execute = false;
    double fDec = 0;
    double fJerk = 0;
private:
    int setParam(axis_ref *axis,double dec,double jerk);
};
class JMC_SetPos : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xExecute,double pos);
    void call(axis_ref *axis,bool xExecute,double pos);
    void copyParam(bool xExecute,double pos);
    bool Execute = false;
    double fPos = 0;
private:
    int setParam(axis_ref *axis,double pos);
};
class JMC_Reset : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xExecute);
    void call(axis_ref *axis,bool xExecute);
    void copyParam(bool xExecute);
    bool Execute = false;
private:
};
class JMC_WTMC_TC : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xExecute,uint mode);
    void call(axis_ref *axis,bool xExecute,uint mode);
    void copyParam(bool xExecute,uint mode);
    bool Execute = false;
    uint cmode = 0x08;
private:
};
class JMC_Power : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xDrvStart,bool xPowerOn);
    void call(axis_ref *axis,bool xDrvStart,bool xPowerOn);
    bool xDStart = false;
    bool xPOn = false;
    bool xInPower = false;
private:
};

class JMC_JOG : public JMC_Axis_Cmd{
public:
    void call(QString axis_name);
    void call(axis_ref *axis);
    void call(QString axis_name,bool xPositive,bool xNegative,double vel,double acc,double dec,double jerk);
    void call(axis_ref *axis,bool xPositive,bool xNegative,double vel,double acc,double dec,double jerk);
    void copyParam(bool xPositive,bool xNegative,double vel,double acc,double dec,double jerk);
    bool positive = false;
    bool negative = false;
    double fVel = 0;
    double fAcc = 0;
    double fDec = 0;
    double fJerk = 0;
    bool xOldPostive;
    bool xOldNegative;


private:
    int setParam(axis_ref *axis,bool xPositive,bool xNegative,double vel,double acc,double dec,double jerk);
    JMC_MoveVel movevel;
    JMC_Stop stop;
};
enum velo_type{
    s7
};
class velo_Planing
{
public:
    int plan(axis_ref *axis, double _pos,double _vel,double _acc,double _dec,double _jerk,double _vend,velo_type _type);
    int plan(axis_ref *axis);
    int vplan(axis_ref *axis);
    int bu5_tcplan(axis_ref *axis);
    int getAct(axis_ref *axis,double time,double *pos,double *vel,double *acc);
    s7_vel_plan s7_plan;
    int deplan();
    bool invel;
    double pos = 0;
    double vel = 0;
    double acc = 0;
    double dec = 0;
    double jerk = 0;
    double startp = 0;
    double vs = 0;
    double ve = 0;
    double ps = 0;
private:


    //double time = 0;
    bool canget = false;
    velo_type type;
    double t[100];
    double l[100];
    double v[100];

};
#endif // JMC_AXIS_CMD_H
