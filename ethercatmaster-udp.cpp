#include "ethercatmaster.h"
#include "ui_ethercatmaster.h"

void EtherCatMaster::ec_udp()
{

}
void EtherCatMaster::tcp_new()
{
    while(server.hasPendingConnections()){
        QTcpSocket *client = server.nextPendingConnection();
        clients.append(client);
        connect(client,&QTcpSocket::readyRead,this,&EtherCatMaster::tcp_recv);
        connect(client,&QTcpSocket::disconnected,this,&EtherCatMaster::tcp_disc);
        //qDebug() << "connect!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    }
}
void EtherCatMaster::tcp_recv()
{
    QTcpSocket *obj = (QTcpSocket *)sender();
    QByteArray buf = obj->readAll();
    if(buf.isEmpty()){return;}

    char *data = buf.data();
    unsigned int *cover = (unsigned int *)data;
    if (*cover != 0x6a6a6a6a){
        return;
    }
    unsigned int *cmd = (unsigned int *)&data[4];
    //emit message(this,"tcp recv",QString::number(*cmd));
    logMtx.lock();
    tcp_cmd(*cmd,obj);
    logMtx.unlock();
}
void EtherCatMaster::tcp_disc()
{
    if (!clients.isEmpty()){
        //qDebug() << "disconnect!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        QTcpSocket *obj = (QTcpSocket *)sender();
        obj->close();
        clients.removeOne(obj);
    }
}
void setCHeader(char *mem,int lenth,int ret)
{
    unsigned int *head = (unsigned int *)mem;
    *head = 0x6a6a6a6a;
    int *len = (int *)&mem[4];
    *len = lenth;
    int *re = (int *)&mem[8];
    *re = ret;
}
void setCData(char *mem,void *data,int size)
{
    memcpy(&mem[12],data,size);
}
int EtherCatMaster::tcp_cmd(int cmd , QTcpSocket *obj)
{
    char name[256];
    char data[1000];
    //logMtx.lock();
    switch (cmd) {
    case 0:{
        shm_axdata.setKey(shm_name);
        if (shm_axdata.isAttached()){

        }else{
            int ret = shm_axdata.attach();
            if (ret == false){
                MESSAGE_INFO("error","cannot get shm");
            }
        }
        int lenth = 256;
        if (ecm != nullptr){
            setCHeader(data,lenth,-1);
            memset(name,0,sizeof(name));
            setCData(data,name,sizeof(name));
            obj->write(data,lenth + 12);
            obj->flush();
            obj->waitForBytesWritten(3000);


            if(start){
                return 0;
            }
            //mem_insertLog(1,"EtherCat","主站准备启动");
            test = true;
            //ethercatRun();
            //sleep(1);
            //start = false;
            //sleep(2);
            test = false;
            ethercatRun();
            //mem_insertLog(100,"EtherCat","rereadDB");
        }else{
            setCHeader(data,lenth,0);
            memset(name,0,sizeof(name));
            //memcpy(name,shm.key().toLocal8Bit().data(),shm.key().length());
            QString ETC_name = ETC_FILE;
            QByteArray ary = ETC_name.toLocal8Bit();
            memcpy(name,ary.data(),ary.size());
            setCData(data,name,sizeof(name));
            obj->write(data,lenth + 12);
            obj->flush();
            obj->waitForBytesWritten(3000);

        }
    }

    break;
    }
    //logMtx.unlock();
    return 0;
}
void EtherCatMaster::send_log()
{

}
void EtherCatMaster::mem_insertLog(int type,QString name,QString info)
{
    QString logStr;
    logStr = logStr + QString::number(type) + "&|";
    logStr = logStr + name + "&|";
    logStr = logStr + info;
    QByteArray ary = logStr.toLocal8Bit();
    uint size = ary.size();
    char data[2000];
    memset(data,0,2000);
    memcpy(data,&size,4);
    memcpy(&data[4],ary.data(),ary.size());
    udpSock.writeDatagram(data,ary.size() + 4,QHostAddress("127.0.0.1"),10900);
    udpSock.flush();
}



