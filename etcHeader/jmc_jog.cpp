#include "jmc_axis_cmd.h"


int JMC_JOG::setParam(axis_ref *axis,bool xPositive,bool xNegative,double vel,double acc,double dec,double jerk)
{
    if (acc <= 0){
        return 1;
    }
    if (dec <= 0){
        return 2;
    }

    if(jerk <= 0){
        return 4;
    }
    if (!ec::axis_can_change_move(axis)){
        return 10;
    }
    if (xPositive && xNegative){
        return 11;
    }

    movevel.Execute = false;
    movevel.fVel = vel;
    movevel.fAcc = acc;
    movevel.fDec = dec;
    movevel.fJerk = jerk;

    stop.Execute = false;
    stop.fDec = dec;
    stop.fJerk = jerk;

    return 0;
}
void JMC_JOG::call(QString axis_name)
{
    call(axis_name, positive, negative, fVel, fAcc, fDec, fJerk);
}
void JMC_JOG::call(axis_ref *axis)
{
    call(axis, positive, negative, fVel, fAcc, fDec, fJerk);
}
void JMC_JOG::copyParam(bool xPositive,bool xNegative,double vel,double acc,double dec,double jerk)
{
    positive = xPositive;
    negative = xNegative;
    fVel = vel;
    fAcc = acc;
    fDec = dec;
    fJerk = jerk;
}
void JMC_JOG::call(QString axis_name,bool xPositive,bool xNegative,double vel,double acc,double dec,double jerk){
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis, positive, negative, fVel, fAcc, fDec, fJerk);
}
void JMC_JOG::call(axis_ref *axis,bool xPositive,bool xNegative,double vel,double acc,double dec,double jerk)
{
    copyParam( xPositive, xNegative, vel, acc, dec, jerk);

    movevel.call(axis);
    stop.call(axis);
    if (xPositive && !xOldPostive){
        if(ec::axis_can_change_move(axis)){
            out_busy();
            state = 5;
        }else{
            out_error(1);
            state = 0;
        }
    }
    if (xNegative && !xOldNegative){
        if(ec::axis_can_change_move(axis)){
            out_busy();
            state = 6;
        }else{
            out_error(1);
            state = 0;
        }
    }
    switch (state) {
        case 0:
            out_clear();
            movevel.Execute = false;
            stop.Execute = false;
        break;
        case 5:{
            int ret = setParam(axis, xPositive, xNegative,vel,acc,dec,jerk);
            if (ret != 0){
                state = 26;
                break;
            }
//            ret = ec::get_new_act(axis,cmd_type::absmove,&actNo);
//            if (ret != 0){
//                state = 26;
//                break;
//            }
//            axis->cmd.abs.done = false;
//            axis->cmd.abs.error = false;
//            axis->cmd.abs.errorid = 0;
            state = 10;
        }
        break;
        case 6:{
            int ret = setParam(axis, xPositive, xNegative,-vel,acc,dec,jerk);
            if (ret != 0){
                state = 26;
                break;
            }
//            ret = ec::get_new_act(axis,cmd_type::absmove,&actNo);
//            if (ret != 0){
//                state = 26;
//                break;
//            }
//            axis->cmd.abs.done = false;
//            axis->cmd.abs.error = false;
//            axis->cmd.abs.errorid = 0;
            state = 10;
        }
        break;
        case 10:
            movevel.Execute = true;
            state = 11;
        break;
        case 11:
            if (movevel.error){
                state = 25;
            }
            if (movevel.aborted){
                state = 20;
            }
            if (!positive && !negative){
                state = 12;
            }
        break;
        case 12:
            stop.Execute = true;
            if(stop.done){
                state = 15;
            }
            if(stop.error){
                state = 25;
            }
        break;
        case 15:
            out_done();
            if(!positive && !negative){
                state = 0;
            }
        break;
        case 20:
            out_abort();
            if(!positive && !negative){
                state = 0;
            }
        break;
        case 25:
            out_error(axis->cmd.errorID);
            if(!positive && !negative){
                state = 0;
            }
        break;
        case 26:
            out_error(1);
            if(!positive && !negative){
                state = 0;
            }
        default:
        break;
    }


    xOldPostive = xPositive;
    xOldNegative = xNegative;

}
