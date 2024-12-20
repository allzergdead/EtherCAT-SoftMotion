#include "jmc_axis_cmd.h"

//class JMC_MoveVel : public JMC_Axis_Cmd{
//    void call(QString axis_name,bool xExecute,double vel,double acc,double dec,double jerk);
//    void call(axis_ref *axis,bool xExecute,double vel,double acc,double dec,double jerk);
//private:
//    void setParam(axis_ref *axis,double vel,double acc,double dec,double jerk);
//};


int JMC_MoveVel::setParam(axis_ref *axis,double vel,double acc,double dec,double jerk)
{
    axis->cmd.vel.vel = vel;
    axis->cmd.vel.acc = acc;
    axis->cmd.vel.dec = dec;
    axis->cmd.vel.jerk = jerk;

    if (acc <= 0){
        return 1;
    }
    if (dec <= 0){
        return 2;
    }
    if (vel == 0){
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
void JMC_MoveVel::call(QString axis_name)
{
    call( axis_name, Execute, fVel, fAcc, fDec, fJerk);
}
void JMC_MoveVel::call(axis_ref *axis)
{
    call( axis, Execute, fVel, fAcc, fDec, fJerk);
}
void JMC_MoveVel::copyParam(bool xExecute, double vel, double acc, double dec, double jerk)
{
    Execute = xExecute;
    fVel = vel;
    fAcc = acc;
    fDec = dec;
    fJerk = jerk;
}
void JMC_MoveVel::call(QString axis_name,bool xExecute,double vel,double acc,double dec,double jerk)
{
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xExecute,vel,acc,dec,jerk);
}
void JMC_MoveVel::call(axis_ref *axis,bool xExecute,double vel,double acc,double dec,double jerk)
{
    copyParam( xExecute,  vel,  acc,  dec,  jerk);
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
            invel = false;
        break;
        case 5:{
            int ret = setParam(axis,vel,acc,dec,jerk);
            if (ret != 0){
                state = 26;
                break;
            }
            ret = ec::get_new_act(axis,cmd_type::vel,&actNo);
            if (ret != 0){
                qDebug("*************************************************************************** axis_can_change_move 2");
                qDebug() << ret;
                state = 26;
                break;
            }
            invel = false;
            state = 10;
        }
        break;
        case 10:
            if (ec::axis_in_act(axis,actNo)){
                if (axis->cmd.vel.invel){
                    invel = true;
                }
                if (axis->cmd.vel.error){
                    state = 25;
                }
            }else{
                state = 20;
            }
        break;
        case 20:
            out_abort();
            invel = false;
            if(!xExecute){
                state = 0;
            }
        break;
        case 25:
            out_error(axis->cmd.vel.errorid);
            invel = false;
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
