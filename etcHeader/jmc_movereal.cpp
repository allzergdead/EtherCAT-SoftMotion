#include "jmc_axis_cmd.h"

int JMC_MoveReal::setParam(axis_ref *axis,double pos,double vel,double acc,double dec,double jerk)
{
    axis->cmd.freal_pos = pos;
    axis->cmd.freal_vel = vel;
    axis->cmd.freal_acc = acc;
    axis->cmd.freal_dec = dec;
    axis->cmd.freal_jerk = jerk;

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
    return 0;
}
void JMC_MoveReal::call(QString axis_name,bool xExecute,double pos,double vel,double acc,double dec,double jerk)
{
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xExecute,pos,vel,acc,dec,jerk);
}
void JMC_MoveReal::call(axis_ref *axis,bool xExecute,double pos,double vel,double acc,double dec,double jerk)
{
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
            ret = ec::get_new_act(axis,cmd_type::realmove,&actNo);
            if (ret != 0){
                state = 26;
                break;
            }
            state = 10;
        }
        break;
        case 10:
            if (ec::axis_in_act(axis,actNo)){
                if (axis->cmd.xDone){
                    state = 15;
                }
                if (axis->cmd.xError){
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
        break;
        default:

        break;
    }



    oldExecute = xExecute;
}
