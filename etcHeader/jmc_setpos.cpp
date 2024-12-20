#include "jmc_axis_cmd.h"

//class JMC_SetPos : public JMC_Axis_Cmd{
//    void call(QString axis_name,bool xExecute,double pos);
//    void call(axis_ref *axis,bool xExecute,double pos);
//private:
//    void setParam(axis_ref *axis,double pos);
//};


int JMC_SetPos::setParam(axis_ref *axis,double pos)
{
    axis->cmd.setpos.pos = pos;
    if (axis->state != standstill){
        return -1;
    }
    if (!ec::axis_can_setpos(axis)){
        return 10;
    }
    return 0;
}
void JMC_SetPos::call(QString axis_name)
{
    call( axis_name, Execute, fPos);
}
void JMC_SetPos::call(axis_ref *axis)
{
    call( axis, Execute, fPos);
}
void JMC_SetPos::copyParam(bool xExecute, double pos)
{
    Execute = xExecute;
    fPos = pos;
}
void JMC_SetPos::call(QString axis_name,bool xExecute,double pos)
{
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xExecute,pos);
}
void JMC_SetPos::call(axis_ref *axis,bool xExecute,double pos)
{
    copyParam( xExecute,  pos);
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
            int ret = setParam(axis,pos);
            if (ret != 0){
                state = 25;
                break;
            }
            ret = ec::get_new_act(axis,cmd_type::setpos,&actNo);
            if (ret != 0){
                state = 26;
                break;
            }
            axis->cmd.setpos.error = false;
            axis->cmd.setpos.done = false;

            state = 10;
        }
        break;
        case 10:
            if (ec::axis_in_act(axis,actNo)){
                if (axis->cmd.setpos.done){
                    state = 15;
                }
                if (axis->cmd.setpos.error){
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
