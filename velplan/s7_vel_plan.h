#ifndef S7_VEL_PLAN_H
#define S7_VEL_PLAN_H
#include <math.h>
#include <stdlib.h>
//#define QDEBUG
#define  FP64 double


#ifdef QDEBUG
#include <QDebug>
#define DBG qDebug()

#endif
#ifndef QDEBUG
#define DBG  //
#endif
class s7_vel_plan
{
public:
    s7_vel_plan();
    int cplan(FP64 _v_start,FP64 _v_end,FP64 _s_target,FP64 _v_target,FP64 _a_target,
            FP64 _d_target,FP64 _j_max);
    double max_vel();
    double min_vel();
    void delplan();
    int cgetAct(double t,double *pos,double *vel,double *acc);
    int cgetAct(double t,double *pos,double *vel,double *acc,double *jerk);

    //double s1_get(double t,double vs,double j);
    bool planDone = false;
    FP64 v_start;   //初速度
    FP64 v_end;     //末速度
    FP64 v_max = 0;     //最大速度(有匀速段时,v_max = v_target)


    FP64 s_target;   //目标位移量
    FP64 v_target;   //目标速度
    FP64 a_target;   //目标加速度
    FP64 d_target;   //目标减速度
    FP64 j_max;      //目标加加速度
    FP64 t_sum;
    FP64 L;          //算出的位移量
    FP64 v_real;     //实际的最大速度


    FP64 s_max;      //L>s_target时候规划的位移


    int  t;         //运行时间
    FP64 t_aver;    //匀速区时间
    FP64 t1;        //加加速时间
    FP64 t2;        //匀加速时间
    FP64 t3;        //减加速时间
    FP64 t4;        //匀速运行时间
    FP64 t5;
    FP64 t6;
    FP64 t7;


    FP64 v1;
    FP64 v2;
    FP64 v3;
    FP64 v4;
    FP64 v5;
    FP64 v6;
    FP64 v7;

    FP64 a_max;     //加速度最大值(拥有匀加速段时 a_max = a_target)
    FP64 d_max;

    FP64 s1;        //加加速位移 s1
    FP64 s2;        //匀加速位移 s2
    FP64 s3;        //减加速位移 s3
    FP64 s4;        //匀速位移   s4
    FP64 s5;        //加减速位移 s5
    FP64 s6;        //匀减速位移 s6
    FP64 s7;        //减减速位移 s7

    FP64 s_acc;     //加速区位移
    FP64 s_dec;     //减速区位移

    FP64 j_cur;     //当前加加速度
    FP64 a_cur;     //当前加速度
    FP64 v_cur;     //当前速度
    FP64 s_cur;     //当前位移量
    FP64 v_down;

    FP64 v_low;
    FP64 v_high;

    FP64 p_dir;
    FP64 P_min = 0.00001;
    FP64 F_10 = 1;

    bool invel = false;
    short int   end_count=0;
    short int   flag = 0;
    short int   flag1 = 0;
    short int   count=0;
};

#endif // S7_VEL_PLAN_H
