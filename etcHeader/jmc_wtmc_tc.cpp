#include "jmc_axis_cmd.h"

//class JMC_SetPos : public JMC_Axis_Cmd{
//    void call(QString axis_name,bool xExecute,double pos);
//    void call(axis_ref *axis,bool xExecute,double pos);
//private:
//    void setParam(axis_ref *axis,double pos);
//};

void JMC_WTMC_TC::call(QString axis_name)
{
    call( axis_name, Execute, cmode);
}
void JMC_WTMC_TC::call(axis_ref *axis)
{
    call( axis, Execute, cmode);
}
void JMC_WTMC_TC::call(QString axis_name,bool xExecute,uint mode)
{
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xExecute,mode);
}
void JMC_WTMC_TC::call(axis_ref *axis,bool xExecute,uint mode)
{
    Execute = xExecute;
    cmode = mode;
    if(xExecute && not oldExecute){
        if(ec::axis_can_setpos(axis)){
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

            int ret = ec::get_new_act(axis,cmd_type::setwtmct,&actNo);

            if(mode != 0x08 && mode != 0xAA && mode != 0x0A && mode != 0x06){
                mode = 0x08;
            }

            axis->target_mode = mode;
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
