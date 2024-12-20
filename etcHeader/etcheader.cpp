#include "etcheader.h"
//#include <QtMath>
QHash<QString ,axis_ref*> axis_hash;





    bool ec::axis_in_act(axis_ref *axis,uint cmd)
    {
        if (axis->cmd.act_Now == cmd){
            return true;
        }else{
            return false;
        }
    }
    bool ec::axis_can_home(axis_ref *axis)
    {
        if(axis->state == standstill){
            return true;
        }
        return false;
    }
    bool ec::axis_can_change_move(axis_ref *axis)
    {
        if (axis->state == continuous_motion
        || axis->state == discrete_motion
        || axis->state == synchronized_motion
        || axis->state == standstill){
            return true;
        }else{
            return false;
        }
    }
    bool ec::axis_can_setpos(axis_ref *axis)
    {
        if (axis->state == power_off
        || axis->state == standstill){
            return true;
        }else{
            return false;
        }
    }
    bool ec::axis_can_reset(axis_ref *axis)
    {
        if (axis->state == errorstop || axis->state == power_off){
            return true;
        }else{
            return false;
        }
    }
    bool ec::axis_can_stop(axis_ref *axis)
    {
        if (axis->state == continuous_motion
        || axis->state == discrete_motion
        || axis->state == synchronized_motion
        || axis->state == standstill
        || axis->state == homing){
            return true;
        }else{
            return false;
        }
    }
    bool ec::axis_can_bu5_tc(axis_ref *axis)
    {
        if (axis->state == standstill && abs(axis->fSetPos - axis->cmd.bu5_tc.zero_pos) < axis->check_area){
            return true;
        }else{
            return false;
        }
    }
    int ec::get_new_act(axis_ref *axis,cmd_type type,uint *act)
    {
        axis->cmd.mtx.lock();
        int ret = -1;
        if(type == cmd_type::poweron){
            ret = 1;
        }else if(type == cmd_type::resetaxis){
            if (axis_can_reset(axis)){
                ret = 0;
            }
        }else if(type == cmd_type::setpos){
            if (axis_can_setpos(axis)){
                ret = 0;
            }
        }else if(type == cmd_type::vel){
            if (axis_can_change_move(axis)){
                ret = 0;
            }
        }else if(type == cmd_type::absmove){
            if (axis_can_change_move(axis)){
                ret = 0;
            }
        }else if(type == cmd_type::stop){
            if (axis_can_stop(axis)){
                ret = 0;
            }
        }else if(type == cmd_type::setwtmct){
            if (axis_can_setpos(axis)){
                ret = 0;
            }
        }else if(type == cmd_type::bu5_tc){
            if (axis_can_bu5_tc(axis)){
                ret = 0;
            }
        }else if(type == cmd_type::home){
            if (axis_can_home(axis)){
                ret = 0;
            }
        }
        if (ret == 0){
            axis->cmd.act_Old = axis->cmd.act_Now;
            axis->cmd.act_Now++;
            *act = axis->cmd.act_Now;
            axis->cmd.type_Runing = type;
        }
        axis->cmd.mtx.unlock();
        return ret;
    }
    void ec::set_axis_cmdtype(axis_ref *axis,cmd_type type)
    {
        axis->cmd.mtx.lock();
        axis->cmd.type_Runing = type;
        axis->cmd.mtx.unlock();
    }
    axis_ref *ec::get_axis(QString name)
    {

        if(axis_hash.find(name) == axis_hash.end()){
            return nullptr;
        }
        return axis_hash[name];
    }
    int ec::appendAxisHash(QString name,axis_ref* axis)
    {
        if(axis_hash.find(name) != axis_hash.end()){
            return -1;
        }
        axis_hash[name] = axis;
        memcpy(axis->name,name.toStdString().c_str(),name.size());
        //axis->name = name;
        return 0;
    }


QString last_json_error;
int etc_vector2json(QString path,QVector<slave_config> *vec , QJsonObject *obj)
{
    QJsonArray slave_ary;
    int ret;
    for(int i = 0;i < vec->size();i++){
        QJsonObject slave = etc_slave2json(&(*vec)[i],&ret);
        if(ret != 0){
            last_json_error = (*vec)[i].name + " slave - 转换json失败";
            return -1;
        }
        slave_ary.append(slave);
    }

    *obj->insert("slaveAry",slave_ary);
    return 0;
}

QJsonObject etc_pdo2json(slave_pdo *pdo ,int *_ret)
{
    QJsonObject pdo_obj;
    pdo_obj.insert("index",toJosnVal(pdo->index));
    pdo_obj.insert("info",pdo->info);
    pdo_obj.insert("size",toJosnVal(pdo->size));
    pdo_obj.insert("sub_index",toJosnVal(pdo->sub_index));
    *_ret = 0;
    return pdo_obj;
}
QJsonObject etc_index2json(slave_index *index,int *_ret)
{
    QJsonObject index_obj;
    QJsonArray pdo_ary;
    for(int i = 0;i < index->pdo.size();i++){
        int ret;
        QJsonObject pdo_obj = etc_pdo2json(&index->pdo[i],&ret);
        if(ret != 0){
            last_json_error = index->pdo[i].info + " pdo - 转换json失败";
            *_ret = -1;
            return index_obj;
        }
        pdo_ary.append(pdo_obj);
    }
    index_obj.insert("pdo",pdo_ary);
    index_obj.insert("index",toJosnVal(index->index));
    index_obj.insert("info",index->info);
    *_ret = 0;
    return index_obj;
}
QJsonObject etc_assign2json(slave_assign *assign,int *_ret)
{
    QJsonObject assign_obj;
    QJsonArray index_ary;
    for(int i = 0;i < assign->index.size();i++){
        int ret;
        QJsonObject index_obj = etc_index2json(&assign->index[i],&ret);
        if(ret != 0){
            last_json_error = assign->index[i].info + " index - 转换json失败";
            *_ret = -1;
            return assign_obj;
        }
        index_ary.append(index_obj);
    }
    assign_obj.insert("index",index_ary);
    assign_obj.insert("info",assign->info);
    assign_obj.insert("WD",toJosnVal(assign->WD));
    *_ret = 0;
    return assign_obj;
}

QJsonObject etc_startup2json(slave_startup *startup,int *_ret)
{
    QJsonObject startup_obj;
    startup_obj.insert("index",toJosnVal(startup->index));
    startup_obj.insert("info",startup->info);
    startup_obj.insert("size",toJosnVal(startup->size));
    startup_obj.insert("subindex",toJosnVal(startup->subindex));
    startup_obj.insert("type",toJosnVal(startup->type));
    startup_obj.insert("value",QString::number(startup->value,16));
    *_ret = 0;
    return startup_obj;
}
QJsonObject etc_slave2json(slave_config *slave,int *_ret)
{
    QJsonObject slave_obj;
    QJsonObject Mxin_obj , Mxout_boj , input_obj , output_obj;
    QJsonArray startup_ary;
    int ret;
    Mxin_obj = etc_assign2json(&slave->MbxIn,&ret);
    if(ret != 0){
        last_json_error = slave->name + " " + slave->MbxIn.info + " assign - 转换json失败";
        *_ret = -1;
        return slave_obj;
    }
    Mxout_boj = etc_assign2json(&slave->MbxOut,&ret);
    if(ret != 0){
        last_json_error = slave->name + " " + slave->MbxOut.info + " assign - 转换json失败";
        *_ret = -1;
        return slave_obj;
    }
    input_obj = etc_assign2json(&slave->input,&ret);
    if(ret != 0){
        last_json_error = slave->name + " " + slave->input.info + " assign - 转换json失败";
        *_ret = -1;
        return slave_obj;
    }
    output_obj = etc_assign2json(&slave->output,&ret);
    if(ret != 0){
        last_json_error = slave->name + " " + slave->output.info + " assign - 转换json失败";
        *_ret = -1;
        return slave_obj;
    }

    for(int i = 0;i < slave->startup.size();i++){
        QJsonObject startup_obj;
        startup_obj = etc_startup2json(&slave->startup[i],&ret);
        if(ret != 0){
            last_json_error = slave->name + " " + slave->startup[i].info + " satrtup - 转换json失败";
            *_ret = -1;
            return slave_obj;
        }
        startup_ary.append(startup_obj);
    }


    slave_obj.insert("MbxOut",Mxout_boj);
    slave_obj.insert("MbxIn",Mxin_obj);
    slave_obj.insert("output",output_obj);
    slave_obj.insert("input",input_obj);
    slave_obj.insert("startup",startup_ary);
    slave_obj.insert("name",slave->name);
    slave_obj.insert("alias",toJosnVal(slave->alias));
    slave_obj.insert("position",toJosnVal(slave->position));
    slave_obj.insert("pid",toJosnVal(slave->pid));
    slave_obj.insert("vid",toJosnVal(slave->vid));

    *_ret = 0;
    return slave_obj;
}


int etc_json2vector(QString path , QJsonObject *obj, QVector<slave_config> *vec)
{
    bool bret = obj->contains("slaveAry");

    if(!bret){
        last_json_error = "slaveAry 未找到";
        return -1;
    }
    //QJsonObject = obj["salveAry"];
    QJsonArray slave_ary =  obj->value("slaveAry").toArray();
    for(int i = 0;i < slave_ary.size();i ++){
        int ret;
        slave_config slave = etc_json2jslave(slave_ary[i].toObject(),&ret);
        if(ret != 0){
            return -1;
        }
        vec->append(slave);
    }
    return 0;
}





slave_config etc_json2jslave(QJsonObject obj,int *_ret)
{
    slave_config slave;
    bool bret = true;
    bret = bret && obj.contains("MbxOut");
    bret = bret && obj.contains("MbxIn");
    bret = bret && obj.contains("output");
    bret = bret && obj.contains("input");
    bret = bret && obj.contains("startup");
    bret = bret && obj.contains("alias");
    bret = bret && obj.contains("position");
    bret = bret && obj.contains("pid");
    bret = bret && obj.contains("vid");


    if(bret == false){
        last_json_error = "slave 转换失败";
        *_ret = -1;
        return slave;
    }


    slave.name = obj.value("name").toString();
    QJsonObject mbxOut_obj = obj.value("MbxOut").toObject();
    QJsonObject mbxIn_obj = obj.value("MbxIn").toObject();
    QJsonObject output_obj = obj.value("output").toObject();
    QJsonObject input_obj = obj.value("input").toObject();

    slave.alias = obj.value("alias").toVariant().toUInt();
    slave.position = obj.value("position").toVariant().toUInt();
    slave.pid = obj.value("pid").toVariant().toUInt();
    slave.vid = obj.value("vid").toVariant().toUInt();
    int ret;
    slave.MbxOut = etc_json2assign(mbxOut_obj,&ret);
    if (ret != 0){
        *_ret = -1;
        return slave;
    }

    slave.MbxIn = etc_json2assign(mbxIn_obj,&ret);
    if (ret != 0){
        *_ret = -1;
        return slave;
    }
    slave.output = etc_json2assign(output_obj,&ret);
    if (ret != 0){
        *_ret = -1;
        return slave;
    }
    slave.input = etc_json2assign(input_obj,&ret);
    if (ret != 0){
        *_ret = -1;
        return slave;
    }

    QJsonArray startup_ary = obj.value("startup").toArray();
    for(int i = 0;i < startup_ary.size();i++){
        slave_startup startup = etc_json2startup(startup_ary[i].toObject(),&ret);
        if (ret != 0){
            *_ret = -1;
            return slave;
        }
        slave.startup.append(startup);
    }
    *_ret = 0;
    return slave;
}
slave_assign etc_json2assign(QJsonObject obj,int *_ret)
{
    slave_assign assign;
    bool bret = true;
    bret = bret && obj.contains("index");
    bret = bret && obj.contains("info");
    bret = bret && obj.contains("WD");

    if(bret == false){
        last_json_error = "assign 转换失败";
        *_ret = -1;
        return assign;
    }
    assign.info = obj.value("info").toString();
    assign.WD = obj.value("WD").toVariant().toUInt();

    QJsonArray index_ary = obj.value("index").toArray();
    int ret;
    for(int i = 0; i < index_ary.size();i++){
        slave_index index = etc_json2index(index_ary.at(i).toObject(),&ret);
        if (ret != 0){
            *_ret = -1;
            return assign;
        }
        assign.index.append(index);
    }
    *_ret = 0;
    return assign;
}
slave_index etc_json2index(QJsonObject obj,int *_ret)
{
    slave_index index;
    bool bret = true;
    bret = bret && obj.contains("pdo");
    bret = bret && obj.contains("index");
    bret = bret && obj.contains("info");

    if(bret == false){
        last_json_error = "index 转换失败";
        *_ret = -1;
        return index;
    }
    index.info = obj.value("info").toString();
    index.index = obj.value("index").toVariant().toUInt();

    QJsonArray pdo_ary = obj.value("pdo").toArray();
    int ret;
    for(int i = 0; i < pdo_ary.size();i++){
        slave_pdo pdo = etc_json2pdo(pdo_ary.at(i).toObject(),&ret);
        if (ret != 0){
            *_ret = -1;
            return index;
        }
        index.pdo.append(pdo);
    }
    *_ret = 0;
    return index;
}

slave_pdo etc_json2pdo(QJsonObject obj,int *_ret)
{
    slave_pdo pdo;
    bool bret = true;
    bret = bret && obj.contains("index");
    bret = bret && obj.contains("info");
    bret = bret && obj.contains("size");
    bret = bret && obj.contains("sub_index");


    if(bret == false){
        last_json_error = "pdo 转换失败";
        *_ret = -1;
        return pdo;
    }

    pdo.index = obj.value("index").toVariant().toUInt();
    pdo.size = obj.value("size").toVariant().toUInt();
    pdo.info = obj.value("info").toString();
    pdo.sub_index = obj.value("sub_index").toVariant().toUInt();
    *_ret = 0;
    return pdo;
}
slave_startup etc_json2startup(QJsonObject obj,int *_ret)
{
    slave_startup startup;
    bool bret = true;
    bret = bret && obj.contains("index");
    bret = bret && obj.contains("info");
    bret = bret && obj.contains("size");
    bret = bret && obj.contains("subindex");
    bret = bret && obj.contains("type");
    bret = bret && obj.contains("value");



    if(bret == false){
        last_json_error = "startup 转换失败";
        *_ret = -1;
        return startup;
    }
    startup.index = obj.value("index").toVariant().toUInt();
    startup.info = obj.value("info").toString();
    startup.size = obj.value("size").toVariant().toUInt();
    startup.subindex = obj.value("subindex").toVariant().toUInt();
    startup.type = obj.value("type").toVariant().toUInt();
    startup.value = obj.value("value").toString().toLong(nullptr,16);

    *_ret = 0;
    return startup;

}

int axis_vector2json(QString path , QVector<axis_config> *axes, QJsonObject *obj)
{
    QJsonArray axis_ary;
    int ret;
    for(int i = 0;i < axes->size();i++){
        QJsonObject axis_obj;
        axis_config *axis = &(*axes)[i];
        //(*axes)[i].name
        axis_obj.insert("name",toJosnVal(axis->name));
        axis_obj.insert("enc_ratio",toJosnVal(axis->enc_ratio));
        axis_obj.insert("pdoNo",toJosnVal(axis->pdoNo));
        axis_obj.insert("circle_diff",toJosnVal(axis->circle_diff));
        axis_obj.insert("press_enable",toJosnVal(axis->press_enable));
        axis_ary.append(axis_obj);
    }
    *obj->insert("axisAry",axis_ary);
    return 0;
}
int axis_json2vector(QString path , QJsonObject *obj, QVector<axis_config> *axes)
{
    bool bret = obj->contains("axisAry");

    if(!bret){
        last_json_error = "slaveAry 未找到";
        return 0;
    }
    //QJsonObject = obj["salveAry"];
    QJsonArray axes_ary =  obj->value("axisAry").toArray();
    for(int i = 0;i < axes_ary.size();i ++){
        QJsonObject obj = axes_ary[i].toObject();
        bret = true;
        bret = bret && obj.contains("name");
        bret = bret && obj.contains("enc_ratio");
        bret = bret && obj.contains("pdoNo");
        bret = bret && obj.contains("circle_diff");
        bret = bret && obj.contains("press_enable");
        if(!bret){
            last_json_error = "slaveAry 数据结构错误";
            return -1;
        }
        axis_config axis;

        axis.name = obj.value("name").toString();
        axis.enc_ratio = obj.value("enc_ratio").toVariant().toInt();
        axis.pdoNo = obj.value("pdoNo").toVariant().toInt();
        axis.circle_diff = obj.value("circle_diff").toVariant().toDouble();
        axis.press_enable = obj.value("press_enable").toVariant().toBool();
        axes->append(axis);
    }
    return 0;
}




int CPid::exec(double set ,double act)
{
    switch (state) {
    case 0:
        Yp = 0;
        Yi = 0;
        Yd = 0;
        Y = Yp + Yi + Yd;

        delta = 0;
        isum = 0;
        xRuning = false;
        if(xEnable && !xOldEnable){
            state = 5;
        }
        break;
    case 5:
        xRuning = true;
        setValue = set;
        actValue = act;

        delta = set - act;
        isum = isum + delta;
        // PID三参数计算，优化版，都采用K值
        Yp = (setValue - actValue) * Kp;
        Yi = isum * Ki;
        Yd = delta * Kd;
        // P输出限制
        Yp = std::max(Yp , Yp_min);
        Yp = std::min(Yp , Yp_max);
        // I输出限制
        Yi = std::max(Yi , Yi_min);
        Yi = std::min(Yi , Yi_max);
        // 简易积分饱和
        if(Ki > 0){
            isum = std::max(isum , Yi_min / Ki);
            isum = std::min(isum , Yi_max / Ki);
        }

        // D输出限制
        Yd = std::max(Yd , Yd_min);
        Yd = std::min(Yd , Yd_max);        // Y输出限制
        Y = Yp + Yi + Yd;
        Y = std::max(Y , Y_min);
        Y = std::min(Y , Y_max);
        if (xReset){
            Yp = 0;
            Yi = 0;
            Yd = 0;
            Y = Yp + Yi + Yd;

            isum = 0;
        }
        if(!xEnable){
            state = 0;
        }
        break;
    default:
        break;
    }
    xOldEnable = xEnable;
    return 0;
}










