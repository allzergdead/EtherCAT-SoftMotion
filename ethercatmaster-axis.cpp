#include "ethercatmaster.h"
#include "ui_ethercatmaster.h"
int pid_exec(double set,double act,axis_pid *pid,axis_ref *axis);
int pid_exec_p(double set,double act,axis_pid *pid,axis_ref *axis);
int pid_exec_v(double set,double act,axis_pid *pid,axis_ref *axis);
uint cmd_vel_param_check(axis_ref *axis)
{
    if(axis->cmd.vel.acc <= 0){
        return 1;
    }
    if(axis->cmd.vel.dec <= 0){
        return 2;
    }
    return 0;
}
uint cmd_stop_param_check(axis_ref *axis)
{
    if (axis->cmd.stop.dec <= 0){
        return 2;
    }
    return 0;
}
uint cmd_abs_param_check(axis_ref *axis)
{
    if(axis->cmd.abs.acc <= 0){
        return 1;
    }
    if (axis->cmd.abs.dec <= 0){
        return 2;
    }
    if (axis->cmd.abs.vel == 0){
        return 3;
    }
    return 0;
}
void EtherCatMaster::axis_i_f_pos_sync(axis_ref *axis,double pos)
{

}

void EtherCatMaster::axis_update_factpos(axis_ref *axis)
{


    //long diff = axis->iActPos - axis->iOldActPos;
    //uint uiPos = axis->iActPos + 0xEFFFFFFF;


    if ((!axis->xCommunication) && axis->pdoNo >= 0){
        axis->iOldActPos = 0;
        axis->xOldCommunication = axis->xCommunication;
        return;
    }
    long lactpos0,lactpos1;

    //axis->iActPos = axis->iActPos + actposdiff;
    lactpos0 = axis->iOldActPos + 0xEFFFFFFF;
    lactpos1 = axis->iActPos + 0xEFFFFFFF ;
    long diff = axis->iActPos - axis->iOldActPos;

    axis->iOldActPos = axis->iActPos;

    if (abs(diff) >= 0xAFFFFFFF){
        if (diff > 0){
            //axis->lActPos = axis->lActPos - 0x100000000;
            //axis->lActPos = (axis->lActPos & 0xFFFFFFFF00000000) + uiPos;
            diff = -((0xEFFFFFFF - lactpos1) + (0xEFFFFFFF + lactpos0));
            //qDebug() << "----------------------------------------------------------";
        }else{
            //axis->lActPos = axis->lActPos + 0x100000000;
            //axis->lActPos = (axis->lActPos & 0xFFFFFFFF00000000) + uiPos;
            diff = (0xEFFFFFFF - lactpos0) + (0xEFFFFFFF + lactpos1);
            //qDebug() << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
        }
    }else{
        //axis->lActPos = (axis->lActPos & 0xFFFFFFFF00000000) + uiPos;
        //axis->lActPos = axis->lActPos + diff;
    }
    //axis->lActPos = axis->iActPos;

    axis->lActPos = axis->lActPos + diff;
    //if diff
    int actposdiff = 0;
    if (axis->lActPos == axis->lOldActPos && axis->syncState > 100){
        if(axis->fSetVel > 0.1){
            actposdiff = (axis->lSetPos - axis->lOldSetPos) * 0.5 ;
        }
    }

    if(axis->xCommunication && !axis->xOldCommunication && axis->pdoNo >= 0){
        axis->lOffsetPos = 0;
        axis->iOldActPos = axis->iActPos;
        axis->lActPos = axis->iActPos ;
        axis->lOldActPos = axis->lActPos;
        axis->fSetPos = 0;
        axis->fOldSetPos = 0;
        axis->lSetPos = 0;
        axis->lOldSetPos = 0;
        axis->xFristTime = true;
    }
    long lpos = axis->lActPos - axis->lOffsetPos;                                                          
    //axis->fActPos = ((double)lpos) / ((double)axis->ratio_deno) * ((double)axis->ratio_num);


    axis->fActPos = (double(lpos)) / double(axis->ratio_deno) * double(axis->ratio_num) * axis->circle_diff;
    axis->fOldActVel = axis->fActVel;
    axis->fActVel = (axis->fActPos - axis->fOldActPos) / (((double)period_ns) / ((double)NSEC_PER_SEC));

    axis->fActAcc = (axis->fActVel - axis->fOldActVel) / (((double)period_ns) / ((double)NSEC_PER_SEC));


    if(axis->xCommunication && !axis->xOldCommunication && axis->pdoNo >= 0){

        axis_set_fpos(axis,axis->fActPos);
        qDebug() << "axis->fActPos" << axis->fActPos;
    }
    axis->xOldCommunication = axis->xCommunication;
    if (axis->iOldActPos == axis->iActPos && axis->syncState > 100){
        //axis->posequaCount++;
        if(axis->fSetVel > 0.1){
            axis->posequaCount++;
        }

        //actposdiff = (axis->fSetVel + axis->fOldActVel) * 0.5 * (((double)period_ns) / ((double)NSEC_PER_SEC)) *  double(axis->ratio_deno) / double(axis->ratio_num);
        //qDebug() << "actposdiff ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<actposdiff;

    }
}
void EtherCatMaster::axis_update_isetPos(axis_ref *axis)
{
    //pod_config *pdo = &ecm->pdoconf[axis->pdoNo];
    //axis_offset *offset = (axis_offset *)pdo->offset;

    axis->lOldSetPos = axis->lSetPos;
    long lSetPos = axis->fSetPos / axis->circle_diff * (double)axis->ratio_deno / (double)axis->ratio_num;
    axis->lSetPos =  lSetPos + axis->lOffsetPos;
    int iChange = axis->lSetPos - axis->lOldSetPos;
    axis->iOldSetPos = axis->iSetPos;
    axis->iSetPos = axis->iSetPos + iChange;

    axis->fSetVel = (axis->fSetPos - axis->fOldSetPos)  / (((double)period_ns) / ((double)NSEC_PER_SEC));
    axis->fSetAcc = (axis->fSetVel - axis->fOldSetVel)  / (((double)period_ns) / ((double)NSEC_PER_SEC));

    //EC_WRITE_S32(domain1_pd + offset->target_position,axis->iSetPos);
}
void EtherCatMaster::axis_set_fpos(axis_ref *axis,double pos)
{
    //qDebug() <<"axis->fActPos" << axis->fActPos;
    axis->fSetPos = pos;
    axis->fActPos = pos;
    //qDebug() << "pos"<<pos;

    //qDebug() << "axis->fSetPos"<<axis->fSetPos;
    long lPos = pos / axis->circle_diff * (double)axis->ratio_deno / (double)axis->ratio_num;
    long lActPos = axis->iActPos + 0xEFFFFFFF;
    //long lActPos = axis->iActPos;
    axis->lOffsetPos = lActPos - lPos;
    axis->iSetPos = axis->iActPos;
    axis->lActPos = lActPos;
    axis->lSetPos = axis->lActPos;
    axis->lOldSetPos = axis->lSetPos;
}

void EtherCatMaster::axis_pdo_read(axis_ref *axis)
{
    pod_config *pdo;
    axis_offset *offset;
    if(axis->pdoNo >= 0){
        pdo = &ecm->pdoconf[axis->pdoNo];
        offset = (axis_offset *)pdo->offset;
        pdo->syncWrite = false;
    }
    if ((!axis->xCommunication) && axis->pdoNo >= 0){


        //axis->lOffsetPos = 0;
        return;
    }

    //axis->iOldActPos = axis->iActPos;
    axis->fOldActPos = axis->fActPos;
    axis->fOldSetPos = axis->fSetPos;
    axis->fOldSetVel = axis->fSetVel;

    //axis->lOldActPos = axis->lActPos;

    if (axis->pdoNo < 0){
        axis->iActPos = axis->iSetPos;
        axis->lOldActPos = axis->lActPos;
    
    
        //axis->errorcode = EC_READ_U16(domain1_pd + offset->error_code);
        axis->errorcode = 0;
        //axis->dinput = EC_READ_U16(domain1_pd + offset->digital_inputs);
        axis->dinput = 0;
        //axis->modes_of_operation_display = EC_READ_U8(domain1_pd + offset->modes_of_operation_display);
        axis->modes_of_operation_display = 8;
        //axis->status_word = EC_READ_U16(domain1_pd + offset->status_word);
        axis->status_word = 0;
        //axis->sActTorque = EC_READ_S16(domain1_pd + offset->torque_actual_velue);
        axis->sActTorque = 0; 
        //axis->digital_inputs = EC_READ_U32(domain1_pd + offset->digital_inputs);  
        axis->digital_inputs = 0;
    }else{

        int ipos = 0;

        for(int i = 0;i < pdo->inNum + pdo->outNum;i++){
                if (pdo->pdo_dir[i] != PDO_INPUT){
                    continue;
                }
                switch(pdo->info[i].index){
                case 0x603F:
                    axis->errorcode = EC_READ_U16(domain1_pd + pdo->offset[i]);
                    break;
                case 0x6041:
                    axis->status_word = EC_READ_U16(domain1_pd + pdo->offset[i]);
                    break;
                case 0x6061:
                    axis->modes_of_operation_display = EC_READ_U8(domain1_pd + pdo->offset[i]);
                    break;
                case 0x6064:
                    ipos = EC_READ_S32(domain1_pd + pdo->offset[i]);
                    break;
                case 0x6077:
                    axis->sActTorque = EC_READ_U16(domain1_pd + pdo->offset[i]);
                    break;
                case 0x60F4:
                    axis->following_error_actual_value = EC_READ_S32(domain1_pd + pdo->offset[i]);
                    break;
                case 0x60FD:
                    axis->digital_inputs = EC_READ_U32(domain1_pd + pdo->offset[i]);
                    axis->dinput = axis->digital_inputs;
                    break;
                default:
                    break;
                }
        }
        int actposdiff = 0;
        if (ipos == axis->iActPos && axis->syncState > 100){
    //        //axis->posequaCount++;
            if(axis->fSetVel > 0.1){
    //            axis->iActPosJY = axis->iActPos;
                axis->posequaCount_p++;
                //actposdiff = (axis->fSetVel + axis->fActVel) * (((double)period_ns) / ((double)NSEC_PER_SEC)) *  double(axis->ratio_deno) / double(axis->ratio_num);
                actposdiff = (axis->lSetPos - axis->lOldSetPos) ;
    //            //actposdiff = (axis->lSetPos - axis->lOldSetPos) * 0.5 ;
    //            //axis->iActPos = ipos + actposdiff;
    //            //qDebug() << "actposdiff ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<
    //            //            (double)actposdiff / (double)axis->ratio_deno * (double)axis->ratio_num * 4000.0;
            }
       }else{
    
       }
        axis->iActPos = ipos + actposdiff;
        axis->lOldActPos = axis->lActPos;



        axis->status_bit.ready =((axis->status_word & (1 << 0)) == (1 << 0));
        axis->status_bit.switch_on = ((axis->status_word & (1 << 1)) == (1 << 1));
        axis->status_bit.operation_enabled = ((axis->status_word & (1 << 2)) == (1 << 2));
        axis->status_bit.fault = ((axis->status_word & (1 << 3)) == (1 << 3));
        axis->status_bit.voltage_enabled = ((axis->status_word & (1 << 4)) == (1 << 4));
        axis->status_bit.quick_stop = ((axis->status_word & (1 << 5)) == (1 << 5));
        axis->status_bit.switch_on_disabled = ((axis->status_word & (1 << 6)) == (1 << 6));
        axis->status_bit.warming = ((axis->status_word & (1 << 7)) == (1 << 7));
        axis->status_bit.manufacturer_spec0 = ((axis->status_word & (1 << 8)) == (1 << 8));
        axis->status_bit.remote = ((axis->status_word & (1 << 9)) == (1 << 9));
        axis->status_bit.target_reach = ((axis->status_word & (1 << 10)) == (1 << 10));
        axis->status_bit.internal_limit_active = ((axis->status_word & (1 << 11)) == (1 << 11));
        axis->status_bit.op_mode_spec0 = ((axis->status_word & (1 << 12)) == (1 << 12));
        axis->status_bit.op_mode_spec1 = ((axis->status_word & (1 << 13)) == (1 << 13));
        axis->status_bit.manufacturer_spec1 = ((axis->status_word & (1 << 14)) == (1 << 14));
        axis->status_bit.home_find = ((axis->status_word & (1 << 15)) == (1 << 15));
    }


}
void EtherCatMaster::axis_pdo_write(axis_ref *axis)
{
    pod_config *pdo;
    axis_offset *offset;
    if(axis->pdoNo >= 0){
        pdo = &ecm->pdoconf[axis->pdoNo];
        offset = (axis_offset *)pdo->offset;
        pdo->syncWrite = true;
    }else{
        return;
    }
    if ((!axis->xCommunication) && axis->pdoNo >= 0){
        return;
    }


    //EC_WRITE_S32(domain1_pd + offset->target_position,axis->iSetPos);
    //EC_WRITE_U16(domain1_pd + offset->ctrl_word,axis->ctrword);
    //EC_WRITE_S16(domain1_pd + offset->target_torque,axis->sSetTorque);
    //EC_WRITE_U8(domain1_pd + offset->operation_mode,axis->modes_of_operation);
    //EC_WRITE_U32(domain1_pd + offset->digital_outputs,axis->digital_outputs);
    /*TxPdo 0x1600*/
//    slave_conf.info = (ec_pdo_entry_info_t *)malloc(sumNum * sizeof(ec_pdo_entry_info_t));
//    slave_conf.info[0] =  {0x6040, 0x00, 16};slave_conf.dir[0] = 1;      //    unsigned int ctrl_word;
//    slave_conf.info[1] =  {0x6060, 0x00, 8 };slave_conf.dir[1] = 1;       //    unsigned int operation_mode;
//    slave_conf.info[2] =  {0x607A, 0x00, 32};slave_conf.dir[2] = 1;       //    unsigned int target_position;
//    slave_conf.info[3] =  {0x6071, 0x00, 16};slave_conf.dir[3] = 1;       //    unsigned int target_torque;
//    slave_conf.info[4] =  {0x60FE, 0x01, 32};slave_conf.dir[4] = 1;       //    unsigned int digital_outputs;
    //int getpos;
    for(int i = 0;i < pdo->inNum + pdo->outNum;i++){
            if (pdo->pdo_dir[i] != PDO_OUTPUT){
                continue;
            }
            switch(pdo->info[i].index){
            case 0x6040:
                EC_WRITE_U16(domain1_pd + pdo->offset[i],axis->ctrword);
                break;
            case 0x6060:
                EC_WRITE_U8(domain1_pd + pdo->offset[i],axis->modes_of_operation);
                break;
            case 0x607A:
                EC_WRITE_S32(domain1_pd + pdo->offset[i],axis->iSetPos);
                //getpos  = EC_READ_S32(domain1_pd + pdo->offset[i]);
                //qDebug() << " getpos :"  <<getpos;
                break;
            case 0x6071:
                EC_WRITE_S16(domain1_pd + pdo->offset[i],axis->sSetTorque);
                break;
            case 0x60FE:
                EC_WRITE_U32(domain1_pd + pdo->offset[i],axis->digital_outputs);
                break;
            default:
                break;
            }
    }

}
void EtherCatMaster::axis_struct_init(axis_ref *axis)
{
    //memset((void *)axis,0,sizeof(axis_ref));
    //axis->lOffsetPos = lActPos - lPos;
    //axis->iSetPos = axis->iActPos;
    //axis->lActPos = lActPos;
    //axis->lSetPos = axis->lActPos;
    //axis->lOldSetPos = axis->lSetPos;
    axis->xCommunication =false;
    axis->xOldCommunication =false;
    axis->xError = false;
    axis->lOffsetPos = 0;
    axis->lActPos = 0;
    axis->iOldActPos = 0;
    axis->modes_of_operation = 0x08;
    axis->syncState = 0;
    axis->uiErrorID = 0;
    axis->ratio_deno = 8388608;
    axis->ratio_num = 1;
    axis->enc_ratio = 8388608;//8388608;
    axis->axis_wtm_type = 0;

    axis->check_area = 0.001;
    axis->check_times = 1;


    //axis->cmd.cmdRet = "";

    sprintf(axis->cmd.cmdRet," ");
    axis->cmd.cmdFlush = 0;
    axis->cmd.act_Now = 0;
    axis->cmd.act_Old = 0;
    axis->cmd.act_Runing = cmd_type::null;
    axis->drv_j = 0.0000041;
    axis->fRatedTorque = 0.32;

    axis->cmd.pidCount = 0;
    axis->cmd.pos_pid.kp = 0;
    axis->cmd.pos_pid.ki = 0;
    axis->cmd.pos_pid.kd = 0;
    axis->cmd.pos_pid.p_dead_area = 0;
    axis->cmd.pos_pid.act_a = 1.0;

    axis->cmd.vel_pid.kp = 0.001;
    axis->cmd.vel_pid.ki = 0;
    axis->cmd.vel_pid.kd = 0;
    axis->cmd.vel_pid.p_dead_area = 0.1;
    axis->cmd.vel_pid.act_a = 0.1;

    axis->cmd.xpower_on = false;
    axis->cmd.xpower_drvstart = false;

    axis->cmd.bu5_tc.keep_torque = 30;
    axis->cmd.bu5_tc.keep_k = 100;
    axis->cmd.bu5_tc.torque_delay = 1;
    axis->cmd.bu5_tc.zero_pos = 0.040;
    axis->cmd.bu5_tc.change_pos = 0.259;
    axis->cmd.bu5_tc.chip_target_pos = 0.260;
    axis->cmd.bu5_tc.chip_deepst_pos = 0.320;
//    axis->cmd.bu5_tc.chip_protect_limit = 0.04;
//    axis->cmd.bu5_tc.chip_pressure_pos_limit = 0.004;
    axis->cmd.bu5_tc.chip_protect_limit = 0.06;
    axis->cmd.bu5_tc.chip_protect_torque = 200;
    axis->cmd.bu5_tc.chip_protect_count = 2;

    axis->cmd.bu5_tc.chip_pressure_pos_limit = 0.004;
    axis->cmd.bu5_tc.chip_pressure_pos_count = 10;

    axis->cmd.bu5_tc.chip_protect_neg_count = 4;
    axis->cmd.bu5_tc.chip_protect_neg_act_count = 0;
    axis->cmd.bu5_tc.chip_protect_neg_torque = -300;


    axis->cmd.bu5_tc.torque_change_enable = false;
    axis->cmd.bu5_tc.torque_change_delay = 1;
    axis->cmd.bu5_tc.torque_change_add = 2;
    axis->cmd.bu5_tc.torque_change_pos = 0.260;
    axis->cmd.bu5_tc.torque_change_time = 20;

    axis->cmd.bu5_tc.auto_change_mode = false;
    axis->cmd.bu5_tc.auto_change_delay = 10;


    axis->cmd.bu5_tc.torque_change_pid.setValue = 0;
    axis->cmd.bu5_tc.torque_change_pid.actValue = 0;
    axis->cmd.bu5_tc.torque_change_pid.Kp = 10;
    axis->cmd.bu5_tc.torque_change_pid.Ki = 0.1;
    axis->cmd.bu5_tc.torque_change_pid.Kd = 0;

    axis->cmd.bu5_tc.torque_change_pid.Yp = 0;
    axis->cmd.bu5_tc.torque_change_pid.Yi = 0;
    axis->cmd.bu5_tc.torque_change_pid.Yd = 0;
    axis->cmd.bu5_tc.torque_change_pid.Y = 0;

    axis->cmd.bu5_tc.torque_change_pid.delta = 0;
    axis->cmd.bu5_tc.torque_change_pid.isum = 0;

    axis->cmd.bu5_tc.torque_change_pid.Yp_max = 30;
    axis->cmd.bu5_tc.torque_change_pid.Yi_max = 30;
    axis->cmd.bu5_tc.torque_change_pid.Yd_max = 30;
    axis->cmd.bu5_tc.torque_change_pid.Y_max = 30;

    axis->cmd.bu5_tc.torque_change_pid.Yp_min = -30;
    axis->cmd.bu5_tc.torque_change_pid.Yi_min = -30;
    axis->cmd.bu5_tc.torque_change_pid.Yd_min = -30;
    axis->cmd.bu5_tc.torque_change_pid.Y_min = -30;

    axis->cmd.bu5_tc.torque_change_pid.xReset = false;
    axis->cmd.bu5_tc.torque_change_pid.xEnable = false;
    axis->cmd.bu5_tc.torque_change_pid.xError = false;

    axis->cmd.bu5_tc.torque_change_pid.xOldEnable = false;

    //double study_torque;
    //double study_pos;
    //int study_type;

    axis->cmd.bu5_tc.study_torque = 80;
    axis->cmd.bu5_tc.study_pos = 0.260;
    axis->cmd.bu5_tc.study_type = 0;
    axis->cmd.bu5_tc.study_delay = 4;
    axis->cmd.bu5_tc.study_count = 0;

    axis->cmd.bu5_tc.study_set_torque = 80;
    axis->cmd.bu5_tc.study_set_target_pos = 0.260;
    axis->cmd.bu5_tc.study_set_change_pos = 0.259;

    axis->cmd.bu5_tc.study_set_protect_count = 4;
    axis->cmd.bu5_tc.study_set_protect_torque = 400;

    axis->cmd.bu5_tc.study_set_t_protect_count = 10;
    axis->cmd.bu5_tc.study_set_t_protect_pos = 0.005;
    //axis->cmd.bu5_tc.torque_change_pid.Kp = 0;
    //axis->cmd.bu5_tc.torque_change_pid.Ki = 0;
    //axis->cmd.bu5_tc.torque_change_pid.Kd = 0;



    axis->cmd.bu5_tc.axis_type = 0;


    axis->digital_inputs = 0;
    axis->digital_outputs = 0;

    axis->stu_tc_ref.down_vel = 95;
    axis->stu_tc_ref.down_acc = 200000;
    axis->stu_tc_ref.down_dec = 10000;
    axis->stu_tc_ref.down_jerk = 20000000;
    axis->stu_tc_ref.down_endVel = 0.4;
    axis->stu_tc_ref.down_pressure = 50;

    axis->stu_tc_ref.up_vel = 95;
    axis->stu_tc_ref.up_acc = 50000;
    axis->stu_tc_ref.up_dec = 50000;
    axis->stu_tc_ref.up_jerk = 20000000;
}
void EtherCatMaster::syncAxis_StateChange(axis_ref *axis)
{
    //axis->cmd.mtx.lock();
    if ((axis->status_word  & 0x08) == 0x08 || (axis->xCommunication == false && axis->pdoNo >= 0) ){
        axis->syncState = 10000;  // axisErrorState
        return;
    }
    if ((axis->status_word & 0x4f) == 0x40 && (axis->xCommunication == true || axis->pdoNo < 0)){
        if (axis->cmd.xpower_drvstart){
            axis->syncState = 5;
        }else{
            axis->syncState = 0;
        }
    }
    if (axis->pdoNo < 0){
        if (axis->cmd.xpower_drvstart && axis->syncState == 0){
            axis->syncState = 5;
        }axis->xFristTime = true;
        if (!axis->cmd.xpower_drvstart){
            axis->syncState = 0;
        }
    }
    if(axis->cmd.act_Old != axis->cmd.act_Now){
        if(axis->cmd.type_Runing == cmd_type::poweron){
            //
        }else if(axis->cmd.type_Runing == cmd_type::resetaxis){
            //
            if (ec::axis_can_reset(axis) ){
                axis->syncState = 5000;
            }
        }else if(axis->cmd.type_Runing == cmd_type::setpos){
            if (ec::axis_can_setpos(axis)){
                axis->syncState = 6000;
            }
        }else if(axis->cmd.type_Runing == cmd_type::vel){
            if (ec::axis_can_change_move(axis)){
                velo_Planing *plan = (velo_Planing *)axis->plan;
                int ret = plan->vplan(axis);
                if (ret == 0){
                    //axis->cmd.abs.time = 0;
                    axis->cmd.vel.time = 0 + (double(period_ns) / double(NSEC_PER_SEC));
                    axis->syncState = 11005;
                    if (shm_axdata.isAttached()){
                        axis_data *data = (axis_data *)shm_axdata.data();
                        data[axis->axNo].num = 0;
                    }
                }else{
                    axis->cmd.vel.error = true;
                    axis->cmd.stop.setVel = axis->fSetVel;
                    axis->cmd.stop.dec = axis->cmd.vel.dec;
                    axis->cmd.stop.jerk = axis->cmd.vel.jerk;
                    axis->syncState = 15005;
                }

                //axis->cmd.vel.setVel = axis->fSetVel;
                //axis->syncState = 11005;
            }
        }else if(axis->cmd.type_Runing == cmd_type::absmove){
            if (ec::axis_can_change_move(axis)){
                velo_Planing *plan = (velo_Planing *)axis->plan;
                int ret = plan->plan(axis);
                if (ret == 0){
                    //axis->cmd.abs.time = 0;
                    axis->cmd.abs.time = 0 + (double(period_ns) / double(NSEC_PER_SEC));
                    axis->syncState = 12005;
                    axis->cmd.bu5_tc.torque_filter = axis->sActTorque;
//                    if (shm_axdata.isAttached()){
//                        axis_data *data = (axis_data *)shm_axdata.data();
//                        data[axis->axNo].num = 0;
//                    }
                }else{
                    axis->cmd.abs.error = true;
                    axis->cmd.stop.setVel = axis->fSetVel;
                    axis->cmd.stop.dec = axis->cmd.abs.dec;
                    axis->cmd.stop.jerk = axis->cmd.abs.jerk;
                    axis->syncState = 15005;
                }
            }
        }else if(axis->cmd.type_Runing == cmd_type::stop){
            if (ec::axis_can_stop(axis)){
                axis->cmd.stop.setVel = axis->fSetVel;
                axis->syncState = 15005;
            }

        }else if(axis->cmd.type_Runing == cmd_type::setwtmct){
            if (ec::axis_can_setpos(axis)){
                axis->syncState = 16005;
            }
        }else if(axis->cmd.type_Runing == cmd_type::bu5_tc){
            if(ec::axis_can_bu5_tc(axis)){
                velo_Planing *plan = (velo_Planing *)axis->plan;
                int ret = plan->bu5_tcplan(axis);
                if (ret == 0){
                    //axis->cmd.abs.time = 0;
                    axis->cmd.bu5_tc.time = 0 + (double(period_ns) / double(NSEC_PER_SEC));
                    axis->cmd.bu5_tc.state = 0;
                    axis->cmd.bu5_tc.timecount = 0;
                    axis->cmd.bu5_tc.done = false;
                    axis->cmd.bu5_tc.error = false;
                    axis->cmd.bu5_tc.chip_protect_act_count = 0;
                    axis->cmd.bu5_tc.chip_pressure_pos_act_count = 0;
                    if (shm_axdata.isAttached()){
                        axis_data *data = (axis_data *)shm_axdata.data();
                        data[axis->axNo].num = 0;
                    }
                    axis->syncState = 17005;
                }else{
                    axis->cmd.vel.error = true;
                    axis->cmd.stop.setVel = axis->fSetVel;
                    axis->cmd.stop.dec = axis->cmd.bu5_tc.dec;
                    axis->cmd.stop.jerk = axis->cmd.bu5_tc.jerk;
                    axis->syncState = 15005;
                }
            }
        }else if(axis->cmd.type_Runing == cmd_type::home){
            if (ec::axis_can_home(axis)){
                axis->syncState = 18005;
            }
        }
        //axis->syncState = 11000;
        axis->cmd.act_Old = axis->cmd.act_Now;
        //qDebug() << "cmd" << axis->cmd.act_Now << "type" << axis->cmd.act_Runing;
    }
    //axis->cmd.mtx.unlock();
    if (!axis->cmd.xpower_on || !axis->cmd.xpower_drvstart){
        if (axis->syncState >= 100){
            axis->syncState = 95;
        }
    }
}



void EtherCatMaster::syncAxis_Ref(axis_ref *axis)
{
    if(axis->pdoNo >= 0 && axis->pdoNo < MAX_SLAVE){
        pod_config *pdo = &ecm->pdoconf[axis->pdoNo];
        axis_offset *offset = (axis_offset *)pdo->offset;
        axis->wCommunicationState = pdo->slave_state.al_state;
        axis->xCommunication = pdo->slave_state.al_state == 8;
    }


    //axis->cmd.type_Runing = cmd_type::vel;
    axis_pdo_read(axis);
    axis_update_factpos(axis);

    syncAxis_StateChange(axis);


    switch (axis->syncState) {
        case 0:
            axis->state = power_off;
            //axis->ctrword = 0x00;
        break;
        case 5:
            axis->state = power_off;
            if (axis->cmd.xpower_drvstart){
                axis->syncState = 10;
                axis->ctrword = 0x06;
                //axis->fActPos = 0;
                //qDebug() << "step 5 actpos" << axis->fActPos;
                axis_set_fpos(axis,axis->fActPos);
                //axis->fSetPos = axis->fActPos;
                //EC_WRITE_S32(domain1_pd + offset->target_position,axis->iActPos);
                //qDebug() << axis->pdoNo  << axis->cmd.xpower_on;;
            }
        break;
        case 10:
            axis->state = power_off;
            if ((axis->status_word & 0x6f) == 0x0021 || axis->pdoNo < 0){
                if(axis->cmd.xpower_on){
                    axis->ctrword = 0x07;
                    axis->syncState = 15;
                }

            }
            qDebug() << axis->pdoNo << axis->cmd.xpower_on;
        break;
        case 15:
            axis->state = power_off;
            if((axis->status_word & 0x6f) == 0x0023 || axis->pdoNo < 0){
                axis->ctrword = 0x0f;
                axis->syncState = 20;
            }
        break;
        case 20:
            axis->state = power_off;
            if((axis->status_word & 0x6f) == 0x0027 || axis->pdoNo < 0){
                axis->ctrword = 0x1f;
                axis->syncState = 100;
            }
        break;
        case 95:
            axis->state = standstill;
            axis->ctrword = 0x00;
            axis->syncState = 0;
        case 100:
            axis->state = standstill;
        break;

        case 5000:
            axis->state = errorstop;
            axis->ctrword = 0x80;
            if (axis->xFristTime){
                axis->syncState = 5005;
                axis->xFristTime = false;
            }
            //axis->syncState = 5005;
        break;
        case 5005:
            axis->state = errorstop;
            axis->syncState = 0;
        break;
        case 6000:
            axis->state = stopping;
            axis_set_fpos(axis,axis->cmd.setpos.pos);
            axis->syncState = 6005;

        break;
        case 6005:
            axis->state = stopping;
            axis->cmd.setpos.done = true;
            axis->syncState = 100;

        break;
        case 10000:
            axis->state = errorstop;
            //axis->ctrword = 0x80;
        break;
        case 11005:{
            axis->state = continuous_motion;
            int ret = axis_cmd_vel_cyc(axis);
            if (ret < 0){
                axis->cmd.vel.error = true;
                axis->cmd.stop.setVel = axis->fSetVel;
                axis->cmd.stop.dec = axis->cmd.vel.dec;
                axis->cmd.stop.jerk = axis->cmd.vel.jerk;
                axis->syncState = 15005;
                qDebug() << "11005 -> 15005" << ret;
            }else if(ret == 1){
                axis->syncState = 11010;
            }else{

            }
        }
        break;
        case 11010:
            axis->state = continuous_motion;
            axis->fSetPos = axis->fSetPos + axis->cmd.vel.setVel * (double(period_ns) / double(NSEC_PER_SEC));
        break;
        case 12005:{
            actCount++;
            axis->state = discrete_motion;
            int ret =axis_cmd_abs_cyc(axis);
            if (ret < 0){
                axis->cmd.abs.error = true;
                axis->cmd.stop.setVel = axis->fSetVel;
                axis->cmd.stop.dec = axis->cmd.abs.dec;
                axis->cmd.stop.jerk = axis->cmd.abs.jerk;
                axis->syncState = 15005;
                qDebug() << "12005 -> 15005" << ret;
            }else if(ret == 1){
                axis->syncState = 12010;
                axis->doneCount = 0;
            }else{
                //
            }
        }
        break;
        case 12010:{
            axis->state = discrete_motion;
            //actCount++;
            if (axis->check_times < 1){
                axis->check_times = 1;
            }
            if (abs(axis->fActPos - axis->fSetPos) < abs(axis->check_area)){
                axis->doneCount++;
                if (axis->doneCount >= axis->check_times){
                    movetime = (double)actCount * (double(period_ns) / double(NSEC_PER_SEC));
                    actCount = 0;
                    axis->cmd.abs.done = true;
                    axis->syncState = 100;
                    actCount++;
                }
            }else{
                axis->doneCount = 0;
            }

        }
        break;
        case 12015:{
            axis->state = discrete_motion;
            //actCount++;
            axis->cmd.abs.pos = -axis->cmd.abs.pos;
            velo_Planing *plan = (velo_Planing *)axis->plan;
            int ret = plan->plan(axis);
            if (ret == 0){
                //axis->cmd.abs.time = 0;
                axis->cmd.abs.time = 0 + (double(period_ns) / double(NSEC_PER_SEC));
                axis->syncState = 12005;
            }else{
                axis->cmd.abs.error = true;
                axis->cmd.stop.setVel = axis->fSetVel;
                axis->cmd.stop.dec = axis->cmd.abs.dec;
                axis->cmd.stop.jerk = axis->cmd.abs.jerk;
                axis->syncState = 15005;
            }
        }
        break;
        case 15005:{
            axis->state = stopping;
            int ret = axis_cmd_stop_cyc(axis);
            //time++;
            if (ret == 1){
                axis->syncState = 15010;
            }
        }
        break;
        case 15010:
            axis->state = stopping;
            axis->cmd.stop.done = true;
            //qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << time;
            //time = 0;
            axis->syncState = 100;
        break;
        case 16005:{
            axis->state = stopping;
            //axis->modes_of_operation = 0x0A;
            //axis->syncState = 100;
            if (axis->target_mode == 0xaa){
                axis->cmd.pos_pid.reset = true;
                axis->cmd.vel_pid.reset = true;
                axis_wtmct_sync(axis);
                axis->cmd.pos_pid.reset = false;
                axis->cmd.vel_pid.reset = false;
            }
            //axis_set_fpos(axis,axis->fActPos);
            axis->syncState = 16010;
        }
        break;
        case 16010:{
            axis->state = stopping;
            if (axis->target_mode == 0xaa){
                axis->modes_of_operation = 0x0A;
                axis->axis_wtm_type = 11;
            }else{
                axis->modes_of_operation = axis->target_mode;
                axis->axis_wtm_type = 0;
            }
            axis_set_fpos(axis,axis->fActPos);
            axis->syncState = 100;

        }
        break;
        case 17005:{
            axis->state = continuous_motion;
            axis->cmd.bu5_tc.timecount++;
            int ret = axis_bu5_tc_sync(axis);

            if (ret == 1){
                axis->syncState = 100;
                axis->cmd.bu5_tc.state = 11;
                axis->cmd.bu5_tc.done = true;
            }
            if (ret == 2){
                axis->cmd.bu5_tc.done = true;
                axis->modes_of_operation = 0x08;
                axis_set_fpos(axis,axis->fActPos);
                axis->cmd.bu5_tc.state = 11;
                axis->syncState = 100;
            }
            if(ret < 0){
                axis->cmd.bu5_tc.error = true;
                axis->modes_of_operation = 0x0A;
                axis->cmd.bu5_tc.state = 11;
                axis->sSetTorque = 0;
                axis->syncState = 100;
            }
            if (abs(axis->fActPos) >= axis->cmd.bu5_tc.chip_deepst_pos){
                axis->cmd.bu5_tc.error = true;
                axis->modes_of_operation = 0x0A;
                axis->cmd.bu5_tc.state = 11;
                axis->sSetTorque = 0;
                axis->syncState = 100;
            }
        }

        break;
        case 18005:
            axis->state = homing;
            axis->cmd.dev_home.old_type = axis->modes_of_operation_display;
            axis->modes_of_operation = 0x06;
            axis->syncState = 18010;
            axis->cmd.dev_home.timeCount = 0;
            axis->cmd.dev_home.home_count = 0;
        break;
        case 18010:
            axis->state = homing;


            if(axis->modes_of_operation_display == 0x06){
                axis->syncState = 18015;
            }
            axis->cmd.dev_home.timeCount = axis->cmd.dev_home.timeCount + period_ns / 1000000000;
            if(axis->cmd.dev_home.timeCount > axis->cmd.dev_home.timeOut){
                axis->syncState = 18100;
            }

        break;
        case 18015:
            axis->state = homing;
            //axis->ctrword = 0x1f;
            if(axis->status_bit.ready){
                axis->syncState = 18020;
            }

//            if(!axis->status_bit.home_find){
//                axis->syncState = 18020;
//            }
            axis->cmd.dev_home.timeCount = axis->cmd.dev_home.timeCount + period_ns / 1000000000;
            if(axis->cmd.dev_home.timeCount > axis->cmd.dev_home.timeOut){
                axis->syncState = 18100;
            }
        break;
        case 18020:
            axis->state = homing;
            if (axis->cmd.xpower_drvstart){
                axis->syncState = 18021;
                axis->ctrword = 0x06;
                //axis->fActPos = 0;
                //qDebug() << "step 5 actpos" << axis->fActPos;
                //axis_set_fpos(axis,axis->fActPos);
                //axis->fSetPos = axis->fActPos;
                //EC_WRITE_S32(domain1_pd + offset->target_position,axis->iActPos);
                //qDebug() << axis->pdoNo  << axis->cmd.xpower_on;;
            }
            axis->cmd.dev_home.timeCount = axis->cmd.dev_home.timeCount + period_ns / 1000000000;
            if(axis->cmd.dev_home.timeCount > axis->cmd.dev_home.timeOut){
                axis->syncState = 18100;
            }
        break;
        case 18021:
            axis->state = homing;
            if ((axis->status_word & 0x6f) == 0x0021 || axis->pdoNo < 0){
                if(axis->cmd.xpower_on){
                    axis->ctrword = 0x07;
                    axis->syncState = 18022;
                }

            }
            axis->cmd.dev_home.timeCount = axis->cmd.dev_home.timeCount + period_ns / 1000000000;
            if(axis->cmd.dev_home.timeCount > axis->cmd.dev_home.timeOut){
                axis->syncState = 18100;
            }
        break;
        case 18022:
            axis->state = power_off;
            if((axis->status_word & 0x6f) == 0x0023 || axis->pdoNo < 0){
                axis->ctrword = 0x0f;
                axis->syncState = 18023;
            }
            axis->cmd.dev_home.timeCount = axis->cmd.dev_home.timeCount + period_ns / 1000000000;
            if(axis->cmd.dev_home.timeCount > axis->cmd.dev_home.timeOut){
                axis->syncState = 18100;
            }
        break;
        case 18023:
            axis->state = power_off;
            if((axis->status_word & 0x6f) == 0x0027 || axis->pdoNo < 0){
                axis->ctrword = 0x1f;
                axis->syncState = 18025;
            }
            axis->cmd.dev_home.timeCount = axis->cmd.dev_home.timeCount + period_ns / 1000000000;
            if(axis->cmd.dev_home.timeCount > axis->cmd.dev_home.timeOut){
                axis->syncState = 18100;
            }
        break;
        case 18025:
            axis->state = homing;
            axis->cmd.dev_home.home_count++;
            if(axis->cmd.dev_home.home_count > 5 && axis->status_bit.home_find){
                axis->syncState = 18030;
            }
            axis->cmd.dev_home.timeCount = axis->cmd.dev_home.timeCount + period_ns / 1000000000;
            if(axis->cmd.dev_home.timeCount > axis->cmd.dev_home.timeOut){
                axis->syncState = 18100;
            }
        break;
        case 18030:
            axis->state = homing;
            axis_set_fpos(axis,axis->fActPos);
            axis->modes_of_operation = axis->cmd.dev_home.old_type;
            if(axis->modes_of_operation_display == axis->cmd.dev_home.old_type){
                axis->syncState = 18035;
            }
            axis->cmd.dev_home.timeCount = axis->cmd.dev_home.timeCount + period_ns / 1000000000;
            if(axis->cmd.dev_home.timeCount > axis->cmd.dev_home.timeOut){
                axis->syncState = 18100;
            }
        break;
        case 18035:
            axis->state = homing;

            axis->syncState = 100;
            axis->cmd.dev_home.done = true;
            break;
        case 18100:
            axis->state = homing;
            //axis->cmd.dev_home.error = true;
            axis_set_fpos(axis,axis->fActPos);
            axis->modes_of_operation = axis->cmd.dev_home.old_type;
            if(axis->modes_of_operation_display == axis->cmd.dev_home.old_type){
                axis->syncState = 18105;
            }

        break;
        case 18105:
            axis->state = homing;
            axis->syncState = 100;
            axis->cmd.dev_home.error = true;
        break;
    }
    if (shm_axdata.isAttached()){
        axis_data *data = (axis_data *)shm_axdata.data();
        if (axis->state == continuous_motion){

            //data->num = 0;
            int setnum = data[axis->axNo].num % MAX_AX_PT;
            data[axis->axNo].time[setnum] = (double)setnum * (double(period_ns) / double(NSEC_PER_SEC));
            data[axis->axNo].pos[setnum] = axis->fActPos;
            data[axis->axNo].vel[setnum] = axis->fSetVel;
            data[axis->axNo].acc[setnum] = axis->iSetPos;
            data[axis->axNo].torque[setnum] = axis->sActTorque;
            data[axis->axNo].num++;
        }else{
            //data[axis->axNo].num = 0;
        }


    }
    axis_update_isetPos(axis);
    if (axis->axis_wtm_type == 11){
        axis_wtmct_sync(axis);
    }
    axis_pdo_write(axis);

}
int EtherCatMaster::axis_cmd_vel_cyc(axis_ref *axis)
{
    velo_Planing *plan = (velo_Planing *)axis->plan;
    double pos,vel,acc;
    int ret = plan->getAct(axis,axis->cmd.vel.time,&pos,&vel,&acc);
    axis->cmd.vel.time = axis->cmd.vel.time + (double(period_ns) / double(NSEC_PER_SEC));
    if (ret < 0){
        return ret;
    }
    double testVel = (pos - axis->fSetPos) / (((double)period_ns) / ((double)NSEC_PER_SEC));
    if(abs(testVel) > 99.9 * axis->circle_diff){
        qDebug() << "vel too large" << axis->fSetPos << pos << axis->cmd.vel.time;
        return -100;
    }
    axis->cmd.vel.setVel = vel;
    if(plan->invel){
        axis->fSetPos = pos;
        return 1;
    }else{
        axis->fSetPos = pos;
        return 0;
    }
    return 0;
}
int EtherCatMaster::axis_cmd_stop_cyc(axis_ref *axis)
{
    if (axis->cmd.stop.setVel >= 0){
        axis->cmd.stop.setVel = axis->cmd.stop.setVel - axis->cmd.stop.dec * (double(period_ns) / double(NSEC_PER_SEC));
        if (axis->cmd.stop.setVel <= 0){
            axis->cmd.stop.setVel = 0;
        }
    }else{
        axis->cmd.stop.setVel = axis->cmd.stop.setVel + axis->cmd.stop.dec * (double(period_ns) / double(NSEC_PER_SEC));
        if (axis->cmd.stop.setVel >= 0){
            axis->cmd.stop.setVel = 0;
        }
    }
     axis->fSetPos = axis->fSetPos + axis->cmd.stop.setVel * (double(period_ns) / double(NSEC_PER_SEC));
     if (axis->cmd.stop.setVel == 0){
         return 1;
     }
     return 0;
}
int EtherCatMaster::axis_cmd_abs_cyc(axis_ref *axis)
{
    velo_Planing *plan = (velo_Planing *)axis->plan;
    double pos,vel,acc;
    int ret = plan->getAct(axis,axis->cmd.abs.time,&pos,&vel,&acc);
    axis->cmd.abs.time = axis->cmd.abs.time + (double(period_ns) / double(NSEC_PER_SEC));
    if (ret < 0){
        return ret;
    }
    double testVel = (pos - axis->fSetPos) / (((double)period_ns) / ((double)NSEC_PER_SEC));
    if(abs(testVel) > 99.9  * axis->circle_diff){
        qDebug() << "vel too large" << axis->fSetPos << pos << axis->cmd.abs.time;
        return -100;
    }
    if(ret == 1){
        axis->fSetPos = pos;
        return 1;
    }else{
        axis->fSetPos = pos;
        return 0;
    }
}
int EtherCatMaster::axis_bu5_tc_sync(axis_ref *axis)
{
    axis->cmd.bu5_tc.torque_change_pid.exec(axis->cmd.bu5_tc.torque_act_pos,axis->fActPos);

    switch(axis->cmd.bu5_tc.state){
        case 0:{
            velo_Planing *plan = (velo_Planing *)axis->plan;
            axis->cmd.bu5_tc.torque_act_pos = axis->fActPos;
            axis->cmd.bu5_tc.torque_change_pid.xEnable = false;
            double pos,vel,acc;
            if( axis->cmd.bu5_tc.t_a > 1){
                axis->cmd.bu5_tc.t_a = 1;
            }
            if( axis->cmd.bu5_tc.t_a < 0.01){
                axis->cmd.bu5_tc.t_a = 0.01;
            }
            axis->cmd.bu5_tc.torque_filter = (double)axis->sActTorque * axis->cmd.bu5_tc.t_a + (1.0 - axis->cmd.bu5_tc.t_a) * axis->cmd.bu5_tc.torque_filter;
            int ret = plan->getAct(axis,axis->cmd.bu5_tc.time,&pos,&vel,&acc);
            axis->cmd.bu5_tc.time = axis->cmd.bu5_tc.time + (double(period_ns) / double(NSEC_PER_SEC));
            if (ret < 0){
                axis->cmd.bu5_tc.state = 1000;
                return ret;
            }

            double testVel = (pos - axis->fSetPos) / (((double)period_ns) / ((double)NSEC_PER_SEC));
            if(abs(testVel) > 99.9 * axis->circle_diff){
                //qDebug() << "vel too large" << axis->fSetPos << pos << axis->cmd.bu5_tc.time;
                return -100;
            }
            bool step1_done = false;
            if(ret == 1){
                axis->fSetPos = pos;
                if (axis->fActVel >= 0){
                    if (axis->fActPos >= axis->cmd.bu5_tc.change_pos){
                        step1_done = true;
                    }
                }else{
                    if (axis->fActPos <= axis->cmd.bu5_tc.change_pos){
                        step1_done = true;
                    }
                }
                if (step1_done){
                    axis->cmd.bu5_tc.state = 1;
                    axis->cmd.bu5_tc.torque_count = 0;
                    axis->cmd.bu5_tc.step_1_time = (double)axis->cmd.bu5_tc.timecount * (double(period_ns) / double(NSEC_PER_SEC));
                    axis->cmd.bu5_tc.step_1_pos = axis->fActPos;
                    axis->cmd.bu5_tc.step_1_vel = axis->fActVel;
                    axis->cmd.bu5_tc.step_1_torque = axis->sActTorque;
                    axis->cmd.bu5_tc.step_1_avgtorque = axis->cmd.bu5_tc.torque_filter;

                    if (axis->cmd.bu5_tc.conf == 1){
                        return 2;
                        //axis->modes_of_operation = 0x0A;
                        //axis->sSetTorque = -abs(axis->cmd.bu5_tc.chip_protect_neg_torque);
                        //axis->cmd.bu5_tc.state = 5;
                    }
                }
            }else{
                axis->fSetPos = pos;
            }
            // 位置模式下检测是否叠料
            if (axis->cmd.bu5_tc.chip_target_pos - axis->fActPos < axis->cmd.bu5_tc.chip_protect_limit){
                if(axis->sActTorque > axis->cmd.bu5_tc.chip_protect_torque){
                    axis->cmd.bu5_tc.chip_protect_act_count++;
                }else{
                    axis->cmd.bu5_tc.chip_protect_act_count = 0;
                }
                if(axis->cmd.bu5_tc.chip_protect_count < 1){
                    axis->cmd.bu5_tc.chip_protect_count = 1;
                }
                if (axis->cmd.bu5_tc.chip_protect_act_count >= axis->cmd.bu5_tc.chip_protect_count){
                    //return -1;
                    axis->cmd.bu5_tc.chip_protect_neg_act_count = 0;
                    //axis->modes_of_operation = 0x0A;
                    //axis->sSetTorque = -abs(axis->cmd.bu5_tc.chip_protect_neg_torque);
                    //axis->fSetPos = axis->fActPos;
                    axis->fSetPos = axis->fActPos - 0.01;
                    axis->cmd.bu5_tc.state = 5;


                }
            }
            return 0;
        }
        break;
        case 1:{
            axis->modes_of_operation = 0x0A;
            short sSetTorque;
            double PFeed;

            //PFeed = axis->cmd.bu5_tc.keep_torque * (axis->cmd.bu5_tc.chip_target_pos - axis->fActPos) * axis->cmd.bu5_tc.keep_k;
            PFeed = axis->cmd.bu5_tc.torque_change_pid.Y;

            if (PFeed > axis->cmd.bu5_tc.keep_torque){
                PFeed = axis->cmd.bu5_tc.keep_torque;
            }else if(PFeed < -axis->cmd.bu5_tc.keep_torque){
                PFeed = -axis->cmd.bu5_tc.keep_torque;
            }
            sSetTorque = (short)(axis->cmd.bu5_tc.pressure);

            if (axis->cmd.bu5_tc.torque_delay < 1){
                axis->cmd.bu5_tc.torque_delay = 1;
            }
            axis->cmd.bu5_tc.torque_count++;
            if (axis->cmd.bu5_tc.torque_count > axis->cmd.bu5_tc.torque_delay){
                axis->cmd.bu5_tc.torque_count = axis->cmd.bu5_tc.torque_delay;
            }
            if (axis->cmd.bu5_tc.torque_start > axis->cmd.bu5_tc.pressure){
                axis->cmd.bu5_tc.torque_start = axis->cmd.bu5_tc.pressure;
            }
            axis->sSetTorque = axis->cmd.bu5_tc.torque_start + (sSetTorque - axis->cmd.bu5_tc.torque_start) * axis->cmd.bu5_tc.torque_count / axis->cmd.bu5_tc.torque_delay;
            if (axis->cmd.bu5_tc.torque_count >= axis->cmd.bu5_tc.torque_delay){
                axis->cmd.bu5_tc.torque_change_count = 0;
                axis->cmd.bu5_tc.torque_act_pos = axis->fActPos;
                if (axis->cmd.bu5_tc.conf == 1){
                    axis->cmd.bu5_tc.state = 2;
                }else{
                    axis->cmd.bu5_tc.state = 2;
                }               
            }else{
                return 0;
            }
        }
        break;
        case 2:{
            short sSetTorque;
            double PFeed;

            //PFeed = axis->cmd.bu5_tc.keep_torque * (axis->cmd.bu5_tc.torque_act_pos - axis->fActPos) * axis->cmd.bu5_tc.keep_k;
            PFeed = axis->cmd.bu5_tc.torque_change_pid.Y;
            if (PFeed > axis->cmd.bu5_tc.keep_torque){
                PFeed = axis->cmd.bu5_tc.keep_torque;
            }else if(PFeed < -axis->cmd.bu5_tc.keep_torque){
                PFeed = -axis->cmd.bu5_tc.keep_torque;
            }
            sSetTorque = (short)(axis->cmd.bu5_tc.pressure);

            //axis->sSetTorque = sSetTorque;
            // 扭矩偏差补正
            if(axis->cmd.bu5_tc.torque_change_enable){
                axis->cmd.bu5_tc.torque_change_count++;
            }else{
                axis->cmd.bu5_tc.torque_change_count = 0;
            }
            if(axis->cmd.bu5_tc.torque_change_delay >= 500 || axis->cmd.bu5_tc.torque_change_delay < 1){
                //axis->cmd.bu5_tc.torque_change_delay = 0;
                sprintf(axis->cmd.cmdRet,"扭矩偏差补正延时设置错误");
                axis->cmd.cmdFlush = 1;
                return -2;
            }
            if(abs(axis->cmd.bu5_tc.torque_change_add) >= 500 || axis->cmd.bu5_tc.torque_change_add < 0){
                sprintf(axis->cmd.cmdRet,"扭矩偏差补正累计值设置错误");
                axis->cmd.cmdFlush = 1;
                return -2;
            }
            if(abs(axis->cmd.bu5_tc.torque_change_time) >= 500 || axis->cmd.bu5_tc.torque_change_time < 1){
                sprintf(axis->cmd.cmdRet,"扭矩偏差补正时间设置错误");
                axis->cmd.cmdFlush = 1;
                return -2;
            }
            if(axis->cmd.bu5_tc.torque_change_count >= axis->cmd.bu5_tc.torque_change_delay){
                //axis->cmd.bu5_tc.torque_change_count = axis->cmd.bu5_tc.torque_change_delay;
                if(axis->cmd.bu5_tc.torque_change_count >= axis->cmd.bu5_tc.torque_change_delay + axis->cmd.bu5_tc.torque_change_time){
                    axis->cmd.bu5_tc.torque_change_count = axis->cmd.bu5_tc.torque_change_delay + axis->cmd.bu5_tc.torque_change_time;
                }
                axis->cmd.bu5_tc.torque_change_pid.xEnable = true;
                axis->cmd.bu5_tc.torque_change_pos = axis->cmd.bu5_tc.chip_target_pos;
                axis->cmd.bu5_tc.torque_act_pos = axis->cmd.bu5_tc.torque_change_pos;
                axis->sSetTorque = sSetTorque + PFeed
                                    + axis->cmd.bu5_tc.torque_change_add
                                    * (axis->cmd.bu5_tc.torque_change_count - axis->cmd.bu5_tc.torque_change_delay + 1)
                                    / (axis->cmd.bu5_tc.torque_change_time);
            }else{
                axis->cmd.bu5_tc.torque_change_pid.xEnable = false;
                axis->cmd.bu5_tc.torque_act_pos = axis->fActPos;
                axis->sSetTorque = sSetTorque;
            }
            // 扭矩模式下检测是否到位
            if (axis->cmd.bu5_tc.chip_pressure_pos_count < 4){
                axis->cmd.bu5_tc.chip_pressure_pos_count = 4;
            }
            if ((axis->cmd.bu5_tc.chip_target_pos- axis->fActPos) < axis->cmd.bu5_tc.chip_pressure_pos_limit){
                axis->cmd.bu5_tc.chip_pressure_pos_act_count = 0;
            }else{
                axis->cmd.bu5_tc.chip_pressure_pos_act_count++;
            }
            if(axis->cmd.bu5_tc.chip_pressure_pos_act_count > axis->cmd.bu5_tc.chip_pressure_pos_count){
                sprintf(axis->cmd.cmdRet,"扭矩模式下检测到叠料，请检查输出波形");
                axis->cmd.cmdFlush = 1;
                return -2;
            }


            if (axis->cmd.bu5_tc.keep_enbale){
                axis->cmd.bu5_tc.act_1s_count++;
                return 0;
            }else{

                return 1;
            }
        }
        break;
        case 5:{
            //axis->modes_of_operation = 0x0A;
            //axis->sSetTorque = -abs(axis->cmd.bu5_tc.chip_protect_neg_torque);
            axis->cmd.bu5_tc.chip_protect_neg_act_count++;
            if(axis->cmd.bu5_tc.chip_protect_neg_count > 20){
                axis->cmd.bu5_tc.chip_protect_neg_count = 20;
            }
            if (axis->cmd.bu5_tc.chip_protect_neg_act_count >= axis->cmd.bu5_tc.chip_protect_neg_count){
                axis->sSetTorque = 0;
                axis->cmd.bu5_tc.state = 11;
                sprintf(axis->cmd.cmdRet,"位置模式叠料错误，请检查输出波形");
                axis->cmd.cmdFlush = 1;
                return -1;
            }
        }
        break;
        case 11:{
        }
        break;
    }
    return 0;
}
int EtherCatMaster::axis_wtmct_sync(axis_ref *axis)
{
    // pos pid
    axis->cmd.pidCount++;
    //if (axis->cmd.pidCount > 1 || axis->cmd.pidCount < 0){
    pid_exec_p(axis->fOldSetPos,axis->fActPos,&axis->cmd.pos_pid,axis);
        //axis->cmd.pidCount = 0;
    //}


    axis->cmd.pv_pid.kp = axis->cmd.vel_pid.kp;
    axis->cmd.pv_pid.ki = axis->cmd.vel_pid.ki;
    axis->cmd.pv_pid.kd = axis->cmd.vel_pid.kd;
    axis->cmd.pv_pid.p_dead_area = axis->cmd.vel_pid.p_dead_area;
    axis->cmd.pv_pid.act_a = axis->cmd.vel_pid.act_a;
    axis->cmd.pv_pid.reset = axis->cmd.vel_pid.reset;

    pid_exec_v(axis->fSetVel,axis->fActVel,&axis->cmd.vel_pid,axis);
    pid_exec_v(axis->cmd.pos_pid.y,0,&axis->cmd.pv_pid,axis);

    short miniT = 12;

    double facc_set = axis->fSetAcc   * 1.0 * (double)axis->ratio_deno / (double)axis->ratio_num / (double)axis->enc_ratio;

    short t_fsetacc = (short)(facc_set * axis->drv_j * 3.1415926 * 2 / axis->fRatedTorque * 1000.0);
    short t_v_pid = (short)(axis->cmd.vel_pid.y / axis->fRatedTorque * 1000.0);
    short t_p_pid = (short)(axis->cmd.pv_pid.y / axis->fRatedTorque * 1000.0);


    double ft_set = facc_set  * axis->drv_j * 3.1415926 * 2 + axis->cmd.vel_pid.y + axis->cmd.pv_pid.y;

    axis->fSetTorque = ft_set;

    //axis->sSetTorque = (short)(axis->fSetTorque / axis->fRatedTorque * 1000.0);


    if (axis->fSetAcc != 0){
        // axis->sSetTorque = (short)axis->cmd.pos_pid.kp;
    }

    short protectT = 500;
    if (axis->sSetTorque > protectT){
        axis->sSetTorque = protectT;
    }else if(axis->sSetTorque < -protectT){
        axis->sSetTorque = -protectT;
    }
    double kmini = 0;
    if (abs(axis->fSetVel)  > 0.1){
        kmini = -axis->fSetVel / 0.1;
        if (kmini > 1){
            kmini = 1;
        }else if(kmini < -1){
            kmini = -1;
        }
    }
   // miniT = miniT + axis->fActVel / 3 * 1;

    miniT = - kmini * miniT + axis->fActVel / 3 * 1;

    short t_miniT = miniT;
    axis->cmd.t_fsetacc = t_fsetacc;
    axis->cmd.t_v_pid = t_v_pid;
    axis->cmd.t_p_pid = t_p_pid;
    axis->cmd.t_miniT = t_miniT;

    axis->sSetTorque = axis->cmd.t_fsetacc  + axis->cmd.t_v_pid  + axis->cmd.t_p_pid  + axis->cmd.t_miniT ;
    axis->sSetTorque = axis->sSetTorque + miniT;


    return 0;
}
int pid_exec(double set,double act,axis_pid *pid,axis_ref *axis)
{
    pid->delta_old = pid->delta;
    pid->delta = set - act;


    double kip = 1;
    if (abs(pid->delta) > abs(pid->p_dead_area)){
        kip = 0;
    }
    pid->pool_i = pid->pool_i + pid->delta * kip;
    pid->yi = pid->ki * kip *  pid->pool_i ;


    pid->yd = pid->kd * (pid->delta - pid->delta_old);
    double kpp;
    kpp = (abs(pid->delta) - abs(pid->p_dead_area)) / abs(pid->p_dead_area + 0.00001);
    if (kpp < 0){
        kpp = 0;
    }
    if (kpp >= 1){
        kpp = 1;
    }
    if (axis->fSetAcc == 0){
        //kpp = 0;
    }
    pid->yp = pid->kp * kpp * pid->delta;



    pid->y = pid->yp + pid->kp * (pid->yi + pid->yd);
    if (pid->reset){
        pid->delta_old = 0;
        pid->delta = 0;
        pid->pool_i = 0;

        pid->yp = 0;
        pid->yi = 0;
        pid->yd = 0;

        pid->y = 0;
    }


    return 0;
}
int pid_exec_p(double set,double act,axis_pid *pid,axis_ref *axis)
{
    pid->delta_old = pid->delta;
    pid->delta = set - act;
    pid->pool_i = pid->pool_i + pid->delta;

    double kpp = 1;
    if(abs(pid->delta) < 0.0001){
        kpp = 0;
    }
    //pid->yp = pid->kp  * pid->delta;
    pid->yp = pid->kp * kpp * pid->delta;

    pid->yi = 0;

    pid->yd = pid->kd * (pid->delta -pid->delta_old);
    pid->y = pid->yp + pid->yi + pid->yd;
    if (pid->reset){
        pid->delta_old = 0;
        pid->delta = 0;
        pid->pool_i = 0;

        pid->yp = 0;
        pid->yi = 0;
        pid->yd = 0;

        pid->y = 0;
    }
    return 0;
}
int pid_exec_v(double set,double act,axis_pid *pid,axis_ref *axis)
{
    pid->delta_old = pid->delta;
    pid->act_old = pid->act;
    //pid->delta = (set - act)* pid->act_a + pid->delta_old * (1.0 - pid->act_a);
    double math_act = act * pid->act_a + pid->act_old * (1.0 - pid->act_a);
    pid->delta = set - act;
    double i_dead_area;
    double p_dead_area;
    double kip = 1;
    double kpp;

    p_dead_area = abs(axis->fActVel) * 0.03;
    if (p_dead_area > 0.3){
        p_dead_area = 0.3;
    }if (p_dead_area < 0.05){
        p_dead_area = 0.05;
    }
    kpp = (abs(pid->delta) - abs(p_dead_area)) / (abs(p_dead_area) + 0.00001);
    if (kpp < 0){
        kpp = 0;
    }
    if (kpp >= 1){
        kpp = 0.1;
    }
    //kpp = 1;
    if (abs(axis->fSetVel - axis->fActVel) < 0.01){
        //kpp = 0;
    }
    pid->yp = pid->kp * kpp * pid->delta;


    i_dead_area = abs(axis->fActVel) * 0.03;
    if (i_dead_area < 1){
        i_dead_area = 1;
    }
    //i_dead_area = 10;
    if (abs(pid->delta) > i_dead_area){
        kip = 0.1;
        //kip = i_dead_area / (abs(pid->delta) + 0.000001);
    }else{
        kip =1.0 - abs(pid->delta) / i_dead_area + 0.1;
    }
    //kip = 1;
    pid->pool_i = pid->pool_i + pid->delta * kip;
    pid->yi = pid->kp * pid->ki *  pid->pool_i ;
    if (abs(pid->yi) > abs(axis->fRatedTorque) && pid->ki != 0){
        if (pid->yi > 0){
            pid->yi = axis->fRatedTorque;
            pid->pool_i = pid->yi / (pid->ki);
        }else{
            pid->yi = -axis->fRatedTorque;
            pid->pool_i = pid->yi / (pid->ki);
        }
    }

    pid->yd = pid->kd * (pid->delta - pid->delta_old);


    pid->y = pid->yp + pid->yi + pid->yd;
    if (pid->reset){
        pid->delta_old = 0;
        pid->delta = 0;
        pid->pool_i = 0;

        pid->yp = 0;
        pid->yi = 0;
        pid->yd = 0;

        pid->y = 0;
        pid->act_old = 0;
    }
    if(abs(pid->y) > abs(axis->fRatedTorque)){
        if (pid->y > 0){
            pid->y = axis->fRatedTorque;
        }else{
            pid->y = -axis->fRatedTorque;
        }
    }
    return 0;
}
int tc_stete_check(axis_ref *axis)
{
    if (axis->modes_of_operation_display == 0x08){
        double diff = axis->fActPos - axis->cmd.bu5_tc.zero_pos;
        if (abs(diff) < axis->check_area){
            return 2;
        }else{
            return 1;
        }
    }else if (axis->modes_of_operation_display == 0x0A){
        double diff = axis->fActPos - axis->cmd.bu5_tc.chip_target_pos;
        if (abs(diff) < axis->cmd.bu5_tc.chip_pressure_pos_limit){
            return 5;
        }else{
            return 4;
        }
    }
    return 7;
}










