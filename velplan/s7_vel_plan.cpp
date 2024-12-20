#include "s7_vel_plan.h"

double s1_get(double t,double vs,double ve,double j,double a);
double v1_get(double t,double vs,double ve,double j,double a);
double s2_get(double t,double vs,double ve,double j,double a);
double v2_get(double t,double vs,double ve,double j,double a);
double s3_get(double t,double vs,double ve,double j,double a);
double v3_get(double t,double vs,double ve,double j,double a);
double s4_get(double t,double vs,double ve,double j,double a);
double v4_get(double t,double vs,double ve,double j,double a);
double s5_get(double t,double vs,double ve,double j,double a);
double v5_get(double t,double vs,double ve,double j,double a);
double s6_get(double t,double vs,double ve,double j,double a);
double v6_get(double t,double vs,double ve,double j,double a);
double s7_get(double t,double vs,double ve,double j,double a);
double v7_get(double t,double vs,double ve,double j,double a);
s7_vel_plan::s7_vel_plan()
{

}

void s7_vel_plan::delplan()
{
    planDone = false;
}
double s7_vel_plan::max_vel()
{
    double max = v_max;
    if (v_start > max){
        max = v_start;
    }
    if (v_end > max){
        max = v_end;
    }
    return max;
}
double s7_vel_plan::min_vel()
{
    double min = v_max;
    if (v_start < min){
        min = v_start;
    }
    if (v_end < min){
        min = v_end;
    }
    return min;
}
int s7_vel_plan::cplan(FP64 _v_start,FP64 _v_end,FP64 _s_target,FP64 _v_target,FP64 _a_target,
        FP64 _d_target,FP64 _j_max)
{
    delplan();
    if(abs(_s_target) > 0.001 && abs(_s_target) < 100000000){
        F_10 = 1;
    }else if(abs(_s_target) <= 0.001){
        F_10 = 10;
    }else if(abs(_s_target) >= 100000000){
        F_10 = 1;
    }
//    if(abs(_s_target) >= 1){

//    }else{
//        F_10 = 1000.0;
//    }
//    F_10 = 1000.0 / (abs(_s_target) + 1.0);

    _v_start = _v_start * F_10;
    _v_end = _v_end * F_10;
    _s_target = _s_target * F_10;
    _v_target = _v_target * F_10;
    _a_target = _a_target * F_10;
    _d_target = _d_target * F_10;
    _j_max = _j_max * F_10;

    P_min = 0.00001 * F_10;
//    if(P_min < 0.000001){
//        P_min = 0.000001;
//    }
//    if(P_min > 0.01){
//        P_min = 0.01;
//    }
    if (abs(_s_target) < P_min){
        _s_target = 0;
    }
    // 迭代初始值赋值

    v_max = _v_target;
    v_target = v_max;
    j_max = _j_max;
    a_max = _a_target;
    d_max = _d_target;

    // jiang
    a_target = _a_target;   //目标加速度
    d_target = _d_target;   //目标减速度
    flag = 0;
    flag1 = 0;

    if (_s_target  >= 0){
        s_target = _s_target;
        v_start = _v_start;   //初速度
        v_end = _v_end;     //末速度
        p_dir = 1;
    }else{
        s_target = -_s_target;
        v_start = -_v_start;   //初速度
        v_end = -_v_end;     //末速度
        p_dir = -1;
    }
    double sv_max = max_vel();
    double sv_min = -max_vel();
    // 通过迭代计算合理的v_max
    for(int i = 0;i < 100;i++){
        a_max = _a_target;
        d_max = _d_target;
        // 分析加速段位移与速度,并计算出来结果
        double va_delta = abs(v_max - v_start);
        if (va_delta >= a_target * a_target / j_max){
            // 存在匀加速区
            // 加加速区域参数运算
            t1 = a_max / j_max;
            //DBG << "t1" << t1;
            double v1_delta = 0.5 * j_max * t1 * t1;
            //double s1_delta = (j_max * t1 * t1 * t1) / 6.0;
            v1 = v1_get(t1,v_start,v_max,j_max,a_max);
            s1 = s1_get(t1,v_start,v_max,j_max,a_max);
            // 匀加速区域参数运算
            double v2_delta = va_delta - 2.0 * v1_delta;
            t2 = v2_delta / a_max;
            v2 = v2_get(t2,v1,v_max,j_max,a_max);
            s2 = s2_get(t2,v1,v_max,j_max,a_max);
            // 减加速区域参数运算
            t3 = t1;
            v3 = v3_get(t3,v2,v_max,j_max,a_max);
            s3 = s3_get(t3,v2,v_max,j_max,a_max);
        }else{
            // 不存在匀加速区
            // 加加速区域参数运算
            t1 = sqrt(va_delta / j_max);
            //DBG << "j_max" << j_max;
            a_max = j_max * t1;
            double v1_delta = 0.5 * j_max * t1 * t1;
            v1 = v1_get(t1,v_start,v_max,j_max,a_max);
            s1 = s1_get(t1,v_start,v_max,j_max,a_max);
            // 匀加速区域参数运算
            t2 = 0;
            v2 = v1;
            s2 = 0;
            // 减加速区域参数运算
            t3 = t1;
            v3 = v3_get(t3,v2,v_max,j_max,a_max);
            s3 = s3_get(t3,v2,v_max,j_max,a_max);
        }
        // 分析减速段位移与速度,并计算出来结果
        double vd_delta = abs(v_max - v_end);
        //DBG << "vd_delta" << vd_delta << "va_delta" << va_delta;
        if (vd_delta >= d_target * d_target / j_max){
            // 存在匀减速区
            // 加减速速区域参数运算
            t5 = d_max / j_max;
            double v5_delta = 0.5 * j_max * t5 * t5;
            v5 = v5_get(t5,v_max,v_end,j_max,d_max);
            s5 = s5_get(t5,v_max,v_end,j_max,d_max);
            // 匀减速区域参数运算
            double v6_delta = vd_delta - 2.0 * v5_delta;
            t6 = v6_delta / d_max;
            v6 = v6_get(t6,v5,v_end,j_max,d_max);
            s6 = s6_get(t6,v5,v_end,j_max,d_max);
            // 减减速区域参数运算
            t7 = t5;
            v7 = v7_get(t7,v6,v_end,j_max,d_max);
            s7 = s7_get(t7,v6,v_end,j_max,d_max);
        }else{
            // 不存在匀减速区
            // 加减速速区域参数运算
            t5 = sqrt(vd_delta / j_max);
            d_max = j_max * t5;
            t5 = d_max / j_max;
            double v5_delta = 0.5 * j_max * t5 * t5;
            v5 = v5_get(t5,v_max,v_end,j_max,d_max);
            s5 = s5_get(t5,v_max,v_end,j_max,d_max);
            // 匀减速区域参数运算
            t6 = 0;
            v6 = v5;
            s6 = 0;
            // 减减速区域参数运算
            t7 = t5;
            v7 = v7_get(t7,v6,v_end,j_max,d_max);
            s7 = s7_get(t7,v6,v_end,j_max,d_max);

        }
        // 加减速总长对比，并输出结果，或者迭代
        L = s1 + s2 + s3 + s5 + s6 + s7;
        if (s_target - L <= -0.00001){
            // 加减速总长大于设定总长，需要迭代
            double max = max_vel();
            double min = min_vel();
            //二分法迭代
            sv_max = v_max;
            if (sv_max - sv_min < 0.01){
                P_min = P_min * 10.0;
                sv_max = _v_target;
                sv_min = -_v_target;
            }
            v_max = (sv_min + sv_max) * 0.5;

            //DBG <<s_target - L  << "v_max" <<sv_max <<"sv_min" <<sv_min;
            //DBG << " L > s_target" << v_max <<"L - s_target" << L - s_target;
        }else{
            if (i == 0){
                flag = 1;
            }
            if (s_target - L < P_min &&  s_target - L > -0.00001){
                flag1 = 1;
            }
            if(s_target == L){
                flag1 = 1;
            }
            //DBG << s_target - L  << "v_max" <<sv_max <<"sv_min" <<sv_min;
            if(flag == 1 || flag1 == 1){
                v4 = v_max;
                s4 = s_target - L;
                if (v4 == 0){
                    t4 = 0.001;
                    v4 =  s4 / t4;
                }else{
                    if(s4 > 0){
                        t4 = s4 / v4;
                    }else{
                        s4 = 0;
                        t4 = s4 / v4;
                    }

                }
//                DBG << "s_target - L" << s_target - L;
//                DBG <<" v : " << v1 << v2 << v3 << v4 << v5 << v6 << v7;
//                DBG <<" s : " <<  s1 << s2 << s3 << s4 << s5 << s6 << s7;
//                DBG <<" t : " <<  t1 << t2 << t3 << t4 << t5 << t6 << t7;
                planDone = true;
                return i;
            }

            if (s_target - L > P_min){
                sv_min = v_max;
                v_max = (sv_min + sv_max) * 0.5;
            }

            //DBG << v_max;
        }

    }
    return -2;
}

int s7_vel_plan::cgetAct(double t,double *pos,double *vel,double *acc)
{
    double jerk;
    return cgetAct( t, pos, vel, acc, &jerk);
}
int s7_vel_plan::cgetAct(double t,double *pos,double *vel,double *acc,double *jerk)
{
    invel = false;
    if (!planDone){
        return -11;
    }
    double curpos,curvel,curacc,curjerk;
    int ret = 0;
    if (flag == 1 || flag1 == 1){
        if (t >= 0 && t < t1){
            double curt = t;
            double sum_pos = 0;
            curjerk = j_max;
            curacc = j_max * curt;
            curvel = v1_get(curt,v_start,v_max,j_max,a_max);
            curpos = s1_get(curt,v_start,v_max,j_max,a_max) + sum_pos;
        }else if(t >= t1 && t < t1 + t2){
            double curt = t - t1;
            double sum_pos = s1;
            curjerk = 0;
            curacc = a_max;
            curvel = v2_get(curt,v1,v_max,j_max,a_max);
            curpos = s2_get(curt,v1,v_max,j_max,a_max) + sum_pos;
        }else if(t >= t1 + t2 && t < t1 + t2 + t3){
            double curt = t - t1 - t2;
            double sum_pos = s1 + s2;
            curjerk = j_max;
            curacc = a_max - j_max * curt;
            curvel = v3_get(curt,v2,v_max,j_max,a_max);
            curpos = s3_get(curt,v2,v_max,j_max,a_max) + sum_pos;
        }else if((t >= t1 + t2 + t3 && t < t1 + t2 + t3 + t4)){
            invel = true;
            double curt = t - t1 - t2 - t3;
            double sum_pos = s1 + s2 + s3;
            curjerk = 0;
            curacc = 0;
            curvel = v4_get(curt,v3,v_max,j_max,a_max);
            curpos = s4_get(curt,v3,v_max,j_max,a_max) + sum_pos;
        }else if((t >= t1 + t2 + t3 + t4 && t < t1 + t2 + t3 + t4 + t5)){
            double curt = t - t1 - t2 - t3 - t4;
            double sum_pos = s1 + s2 + s3 + s4;
            curjerk = j_max;
            curacc = j_max * curt;
            curvel = v5_get(curt,v3,v_end,j_max,d_max);
            curpos = s5_get(curt,v3,v_end,j_max,d_max) + sum_pos;
        }else if((t >= t1 + t2 + t3 + t4 + t5 && t < t1 + t2 + t3 + t4 + t5 + t6)){
            double curt = t - t1 - t2 - t3 - t4 - t5;
            double sum_pos = s1 + s2 + s3 + s4 + s5;
            curjerk = 0;
            curacc = d_max;
            curvel = v6_get(curt,v5,v_end,j_max,d_max);
            curpos = s6_get(curt,v5,v_end,j_max,d_max) + sum_pos;
        }else if((t >= t1 + t2 + t3 + t4 + t5 + t6 && t <= t1 + t2 + t3 + t4 + t5 + t6 + t7)){
            double curt = t - t1 - t2 - t3 - t4 - t5 - t6;
            double sum_pos = s1 + s2 + s3 + s4 + s5 + s6;
            curjerk = j_max;
            curacc = d_max - j_max * curt;
            curvel = v7_get(curt,v6,v_end,j_max,d_max);
            curpos = s7_get(curt,v6,v_end,j_max,d_max) + sum_pos;
        }else if(t > t1 + t2 + t3 + t4 + t6 + t7){
            double curt = t - t1 - t2 - t3 - t4 - t5 - t6 - t7;
            double sum_pos = s1 + s2 + s3 + s4 + s5 + s6 + s7;
            curjerk = 0;
            curacc = 0;
            curvel = v_end;
            curpos = sum_pos + v_end * curt;
            ret = 1;
        }else{
            //qDebug() << t;
            ret = -12;
        }
    }else{
        return -13;
    }
    if (ret < 0){
        return ret;
    }else{
        *pos = curpos / F_10;
        *vel = curvel / F_10;
        *acc = curacc / F_10;
        *jerk = curjerk / F_10;
//        *pos = curpos ;
//        *vel = curvel ;
//        *acc = curacc ;
//        *jerk = curjerk ;
        return ret;
    }
}






double s1_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double s = vs * t + dir * j * t * t * t / 6.0;
    return s;
}
double v1_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double v = vs + dir * j * t * t * 0.5;
    return v;
}
double s2_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double s = vs * t + dir * a * t * t * 0.5;
    return s;
}
double v2_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double v = vs + dir * a * t;
    return v;
}
double s3_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double s = vs * t + dir * (a * 0.5 * t * t - j * t * t * t /6.0);
    return s;
}
double v3_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double v = vs + dir * (a * t - j * t * t * 0.5);
    return v;
}
double s4_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double s = vs * t;
    return s;
}
double v4_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double s = vs;
    return s;
}
double s5_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double s = vs * t + dir * j * t * t * t / 6.0;
    return s;
}
double v5_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double v = vs + dir * j * t * t * 0.5;
    return v;
}
double s6_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double s = vs * t + dir * a * t * t * 0.5;
    return s;
}
double v6_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double v = vs + dir * a * t;
    return v;
}
double s7_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double s = vs * t + dir * (a * t * t * 0.5 - j * t * t * t / 6.0);
    return s;
}
double v7_get(double t,double vs,double ve,double j,double a)
{
    double dir = 1;
    if (vs < ve){
        dir = 1;
    }else{
        dir = -1;
    }
    double v = vs + dir * (a * t - j * t * t * 0.5);
    return v;
}
