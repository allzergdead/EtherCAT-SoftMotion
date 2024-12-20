#include "jmc_axis_cmd.h"


//class JMC_Home_Drv : public JMC_Axis_Cmd{
//    void call(QString axis_name,bool xExecute);
//    void call(axis_ref *axis,bool xExecute);
//    bool invel = false;
//private:
//};
void JMC_Home_Drv::call(QString axis_name)
{
    call( axis_name, Execute, fTimeOut);
}
void JMC_Home_Drv::call(axis_ref *axis)
{
    call( axis, Execute, fTimeOut);
}

void JMC_Home_Drv::call(QString axis_name,bool xExecute,double timeOut)
{
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xExecute,timeOut);
}

void JMC_Home_Drv::call(axis_ref *axis,bool xExecute,double timeOut)
{
    if(xExecute && not oldExecute){
        if(ec::axis_can_home(axis)){
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
            if(timeOut <= 0){
                state = 26;
                break;
            }
            int ret = ec::get_new_act(axis,cmd_type::home,&actNo);
            if (ret == 0){
                state = 10;
            }else{
                state = 26;
            }

            axis->cmd.dev_home.error = false;
            axis->cmd.dev_home.done = false;
            axis->cmd.dev_home.timeOut = timeOut;
            //axis->cmd.dev_home.errorid = 0;
        }
       break;
        case 10:
            if (ec::axis_in_act(axis,actNo)){
                if (axis->cmd.dev_home.done){
                    state = 15;
                }
                if (axis->cmd.dev_home.error){
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
            out_error(axis->cmd.dev_home.errorid);
            if(!xExecute){
                state = 0;
            }
        default:

        break;
    }



    oldExecute = xExecute;
}
