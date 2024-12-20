#include "jmc_axis_cmd.h"

//class velo_Planing
//{
//public:
//    int plan(axis_ref *axis, double _pos,double _vel,double _acc,double _dec,double _jerk,velo_type _type);
//    int getpos(double *pos,double t);
//    int getvel(double *pos,double t);
//    int getacc(double *pos,double t);
//    int deplan();
//private:
//    double pos;
//    double vel;
//    double acc;
//    double dec;
//    double jerk;

//    double vs;
//    double ve;

//    bool canget;
//    velo_type type;
//};
int velo_Planing::plan(axis_ref *axis)
{
    return plan(axis, axis->cmd.abs.pos, axis->cmd.abs.vel, axis->cmd.abs.acc, axis->cmd.abs.dec, axis->cmd.abs.jerk,0, velo_type::s7);
}
int velo_Planing::vplan(axis_ref *axis)
{
    double virPos;
    double virvel;
    if (axis->cmd.vel.vel >= 0){
        virPos = 1000000000;
        virvel = axis->cmd.vel.vel;
    }else{
        virPos = -1000000000;
        virvel = -axis->cmd.vel.vel;
    }
    return plan(axis, virPos, virvel, axis->cmd.vel.acc, axis->cmd.vel.dec, axis->cmd.vel.jerk,0, velo_type::s7);
}
int velo_Planing::plan(axis_ref *axis, double _pos, double _vel, double _acc, double _dec, double _jerk,double _vend, velo_type _type)
{
    // mush check parameter
    vs = axis->fSetVel;
    ps = axis->fSetPos;
    ve = _vend;
    pos = _pos - axis->fSetPos;
    vel = _vel;
    acc = _acc;
    dec = _dec;
    jerk = _jerk;
    type = _type;

    startp = axis->fSetPos;
    canget = false;
    int ret;
    switch(type){
        case velo_type::s7 :{
            ret = s7_plan.cplan(vs,0,pos,vel,acc,dec,jerk);
            //qDebug() << "s7_plan" << ret;
            if (ret < 0){
                return ret;
            }
            canget = true;
            return 0;
        }
        break;
        default:
            ret = -1;
        break;
    }


    return -1;
}
int velo_Planing::bu5_tcplan(axis_ref *axis)
{
    // mush check parameter
    vs = axis->fSetVel;
    ps = axis->fSetPos;
    ve = axis->cmd.bu5_tc.end_vel;
    pos = axis->cmd.bu5_tc.chip_target_pos - axis->fSetPos;
    vel = axis->cmd.bu5_tc.vel;
    acc = axis->cmd.bu5_tc.acc;
    dec = axis->cmd.bu5_tc.dec;
    jerk = axis->cmd.bu5_tc.jerk;
    type = velo_type::s7;

    startp = axis->fSetPos;
    canget = false;
    int ret;
    switch(type){
        case velo_type::s7 :{
            ret = s7_plan.cplan(vs,ve,pos,vel,acc,dec,jerk);
            //qDebug() << "s7_plan" << ret;
            if (ret < 0){
                return ret;
            }
            canget = true;
            return 0;
        }
        break;
        default:
            ret = -1;
        break;
    }


    return -1;
}
int velo_Planing::getAct(axis_ref *axis,double time,double *pos,double *vel,double *acc)
{
    if (!canget){return -99;}
    double apos,avel,aacc;
    int ret;
    switch(type){
        case velo_type::s7 :{
            ret = s7_plan.cgetAct(time,&apos,&avel,&aacc);
            invel = s7_plan.invel;
            if (ret < 0){
                return ret;
            }
        }
        break;
        default:
            ret = -2;
        break;
    }
    *pos = startp + s7_plan.p_dir * apos;
    *vel = s7_plan.p_dir * avel;
    *acc = s7_plan.p_dir * aacc;
    return ret;
}



















