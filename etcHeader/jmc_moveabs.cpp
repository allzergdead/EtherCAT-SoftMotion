#include "jmc_axis_cmd.h"

int JMC_MoveAbs::setParam(axis_ref *axis,double pos,double vel,double acc,double dec,double jerk)
{

    if (acc <= 0){
        return 1;
    }
    if (dec <= 0){
        return 2;
    }
    if (vel <= 0){
        return 3;
    }
    if(jerk <= 0){
        return 4;
    }
    if (!ec::axis_can_change_move(axis)){
        return 10;
    }
    axis->cmd.abs.pos = pos;
    axis->cmd.abs.vel = vel;
    axis->cmd.abs.acc = acc;
    axis->cmd.abs.dec = dec;
    axis->cmd.abs.jerk = jerk;
    return 0;
}
void JMC_MoveAbs::call(QString axis_name)
{
    call(axis_name,Execute, fPos, fVel, fAcc, fDec, fJerk);
}
void JMC_MoveAbs::call(axis_ref *axis)
{
    call(axis,Execute, fPos, fVel, fAcc, fDec, fJerk);
}
void JMC_MoveAbs::copyParam(bool xExecute,double pos, double vel, double acc, double dec, double jerk)
{
    Execute = xExecute;
    fVel = vel;
    fAcc = acc;
    fDec = dec;
    fJerk = jerk;
}
void JMC_MoveAbs::call(QString axis_name,bool xExecute,double pos,double vel,double acc,double dec,double jerk)
{
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xExecute,pos,vel,acc,dec,jerk);
}
void JMC_MoveAbs::call(axis_ref *axis,bool xExecute,double pos,double vel,double acc,double dec,double jerk)
{
    copyParam( xExecute, pos , vel,  acc,  dec,  jerk);
    if(xExecute && not oldExecute){
        if(ec::axis_can_change_move(axis)){
            out_busy();
            state = 5;
        }else{
            out_error(1);
            state = 0;
        }
    }
    switch (state) {
        case 0:
            out_clear();
        break;
        case 5:{
            int ret = setParam(axis,pos,vel,acc,dec,jerk);
            if (ret != 0){
                state = 26;
                break;
            }
            ret = ec::get_new_act(axis,cmd_type::absmove,&actNo);
            if (ret != 0){
                state = 26;
                break;
            }
            axis->cmd.abs.done = false;
            axis->cmd.abs.error = false;
            axis->cmd.abs.errorid = 0;
            state = 10;
        }
        break;
        case 10:
            if (ec::axis_in_act(axis,actNo)){
                if (axis->cmd.abs.done){
                    state = 15;
                }
                if (axis->cmd.abs.error){
                    state = 25;
                }
            }else{
                state = 20;
            }
        break;
        case 15:
            out_done();
            if(!xExecute){
                state = 0;
            }
        break;
        case 20:
            out_abort();
            if(!xExecute){
                state = 0;
            }
        break;
        case 25:
            out_error(axis->cmd.errorID);
            if(!xExecute){
                state = 0;
            }
        break;
        case 26:
            out_error(1);
            if(!xExecute){
                state = 0;
            }
        default:
        break;
    }



    oldExecute = xExecute;
}
