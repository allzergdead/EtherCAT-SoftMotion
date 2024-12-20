#include "jmc_axis_cmd.h"
//class JMC_Stop : public JMC_Axis_Cmd{
//    void call(QString axis_name,bool xExecute,double dec,double jerk);
//    void call(axis_ref *axis,bool xExecute,double dec,double jerk);
//private:
//    void setParam(axis_ref *axis,double dec,double jerk);
//};


int JMC_Stop::setParam(axis_ref *axis,double dec,double jerk)
{
    axis->cmd.stop.dec = dec;
    axis->cmd.stop.jerk = jerk;

    if (dec <= 0){
        return 2;
    }
    if(jerk <= 0){
        return 4;
    }
    if (!ec::axis_can_stop(axis)){
        return 10;
    }
    return 0;
}
void JMC_Stop::call(QString axis_name)
{
    call( axis_name, Execute, fDec, fJerk);
}
void JMC_Stop::call(axis_ref *axis)
{
    call( axis, Execute, fDec, fJerk);
}
void JMC_Stop::copyParam(bool xExecute, double dec, double jerk)
{
    Execute = xExecute;
    fDec = dec;
    fJerk = jerk;
}
void JMC_Stop::call(QString axis_name,bool xExecute,double dec,double jerk)
{
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xExecute,dec,jerk);
}
void JMC_Stop::call(axis_ref *axis,bool xExecute,double dec,double jerk)
{
    copyParam( xExecute,  dec,  jerk);
    if(xExecute && not oldExecute){
        if(ec::axis_can_stop(axis)){
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
            int ret = setParam(axis,dec,jerk);
            if (ret != 0){
                state = 26;
                break;
            }
            ret = ec::get_new_act(axis,cmd_type::stop,&actNo);
            if (ret != 0){
                state = 26;
                break;
            }
            axis->cmd.stop.done = false;
            axis->cmd.stop.error = false;
            axis->cmd.stop.errorid = 0;
            state = 10;
        }
        break;
        case 10:
            if (ec::axis_in_act(axis,actNo)){
                if (axis->cmd.stop.done){
                    state = 15;
                }
                if (axis->cmd.stop.error){
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
