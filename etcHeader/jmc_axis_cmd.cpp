#include "jmc_axis_cmd.h"
void JMC_Axis_Cmd::out_clear()
{
    error = false;
    errorCode = 0;// cant move
    aborted = false;
    busy = false;
    done = false;
}
void JMC_Axis_Cmd::out_error(uint errorid)
{
    error = true;
    errorCode = errorid;// cant move
    aborted = false;
    busy = false;
    done = false;
}
void JMC_Axis_Cmd::out_done()
{
    error = false;
    errorCode = 0;// cant move
    aborted = false;
    busy = false;
    done = true;
}
void JMC_Axis_Cmd::out_abort()
{
    error = false;
    errorCode = 0;// cant move
    aborted = true;
    busy = false;
    done = false;
}
void JMC_Axis_Cmd::out_busy()
{
    error = false;
    errorCode = 0;// cant move
    aborted = false;
    busy = true;
    done = false;
}
