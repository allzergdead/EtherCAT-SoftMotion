#include "jmc_axis_cmd.h"



//class JMC_Power : public JMC_Axis_Cmd{
//    void call(QString axis_name,bool xDrvStart,bool xPowerOn);
//    void call(axis_ref *axis,bool xDrvStart,bool xPowerOn);
//private:
//};

void JMC_Power::call(QString axis_name)
{
    call( axis_name, xDStart, xPOn);
}
void JMC_Power::call(axis_ref *axis)
{
    call( axis, xDStart, xPOn);
}

void JMC_Power::call(QString axis_name,bool xDrvStart,bool xPowerOn)
{
    xDStart = xDrvStart;
    xPOn = xPowerOn;
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xDrvStart,xPowerOn);
}
void JMC_Power::call(axis_ref *axis,bool xDrvStart,bool xPowerOn)
{
    axis->cmd.xpower_drvstart = xDrvStart;
    axis->cmd.xpower_on = xPowerOn;
    xInPower = axis->cmd.xpower_inpower;
}
