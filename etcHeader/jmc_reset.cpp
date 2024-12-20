#include "jmc_axis_cmd.h"






//class JMC_Reset : public JMC_Axis_Cmd{
//    void call(QString axis_name,bool xExecute);
//    void call(axis_ref *axis,bool xExecute);
//private:
//};

void JMC_Reset::call(QString axis_name)
{
    call( axis_name, Execute);
}
void JMC_Reset::call(axis_ref *axis)
{
    call( axis, Execute);
}
void JMC_Reset::copyParam(bool xExecute)
{
    Execute = xExecute;
}
void JMC_Reset::call(QString axis_name,bool xExecute)
{
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xExecute);
}
void JMC_Reset::call(axis_ref *axis,bool xExecute)
{
    copyParam( xExecute);
    if(xExecute && not oldExecute){
        state = 4;
    }
    switch (state) {
        case 0:
            out_clear();
        break;
        case 4:
            if (ec::axis_can_reset(axis)){
                state = 5;
            }else{
                state = 15;
            }
        break;
        case 5:{
            int ret = ec::get_new_act(axis,cmd_type::resetaxis,&actNo);
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
