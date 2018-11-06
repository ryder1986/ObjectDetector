#ifndef MISC_H
#define MISC_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include <QTcpSocket>
#include <QDataStream>
#include "json/json.h"
#include"tool.h"
#include "jsonpacket.h"
#include "app_data.h"
class ServerReplyCheckRouting : public QObject
{
    Q_OBJECT

public slots:
    void check_reply(    QUdpSocket *udp_skt_find_server) {
        QString str;
        str.clear();
        int try_times=1000;
        while(try_times--){
            if(udp_skt_find_server->hasPendingDatagrams()){
                datagram.resize((udp_skt_find_server->pendingDatagramSize()));
                udp_skt_find_server->readDatagram(datagram.data(),datagram.size());
                prt(info,"get server info : %s",datagram.data());
                server_ip.clear();
                server_ip.append(datagram.split(',')[0]);
                //prt(info,"ip : %s",server_ip.toStdString().data());
                ip_list.append(server_ip);
                emit resultReady(server_ip);
            }else{
                //      prt(info,"searching");
            }
            QThread::msleep(10);
        }
    }

signals:
    void resultReady(  QString result);
private:
    QByteArray datagram;
    QString server_ip;
    QStringList ip_list;
};

class ServerInfoSearcher : public QObject{
    Q_OBJECT
    QThread check_thread;
    ServerReplyCheckRouting *p_checker;

public:
    ServerInfoSearcher()
    {

        udp_skt_find_server=new QUdpSocket(this);
        udp_skt_find_server->bind(12347,QUdpSocket::ShareAddress);
        p_checker=new ServerReplyCheckRouting;
        p_checker->moveToThread(&check_thread);
        connect(&check_thread,&QThread::finished,p_checker,&QObject::deleteLater);
        connect(this,SIGNAL(begin_search(QUdpSocket*)),p_checker,SLOT(check_reply(QUdpSocket*)),Qt::QueuedConnection);
        connect(p_checker,SIGNAL(resultReady(QString)),this,SLOT(ip_found(QString)),Qt::QueuedConnection);
    }
    ~ServerInfoSearcher()
    {
        check_thread.quit();
        check_thread.wait();
    }
    void broadcast_info()
    {
        QByteArray b;
        b.append("pedestrian");
        udp_skt_find_server->writeDatagram(b.data(), b.size(),
                                           QHostAddress::Broadcast, 12348);
        prt(info,"finding server ...");

    }
    void search()
    {
        check_thread.start();
        emit begin_search(udp_skt_find_server);
    }

    void search_device()
    {
        ip_list.clear();
        broadcast_info();
        search();
    }
    QStringList search_rst()
    {
        return ip_list;
    }
    static void find_server()
    {
        //prt(info," find server thread");
        int times=10;
        while(times--){

        }
    }
signals:
    void begin_search( QUdpSocket *udp_skt_find_server);
    void find_ip(QString ip);

public slots:
    void ip_found(QString ip)
    {
        qDebug()<<ip;
        // ip_list.append(ip);
        emit find_ip(ip);
    }
private :
    QUdpSocket *udp_skt_find_server;

    QByteArray datagram;
    QString server_ip;
    QStringList ip_list;
};
class Client : public QObject
{
    QByteArray tmp_msg;
    string tmp_msg1;
    Q_OBJECT
public:
    char buf[2000];
    Client()
    {
        tmp_msg.clear();
        server_ip.clear();
        tcp_socket=new QTcpSocket();
        in.setDevice(tcp_socket);
        in.setVersion(QDataStream::Qt_1_0);
        connect(tcp_socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
        connect(tcp_socket,SIGNAL(readyRead()),this,SLOT(handle_server_msg1()),Qt::DirectConnection);
        connect(tcp_socket,SIGNAL(connected()),this,SLOT(handle_connected()),Qt::DirectConnection);
    }
    ~Client()
    {
        delete tcp_socket;
    }
    void get_config()
    {
        RequestPkt e(AppInputData::Operation::GET_CONFIG,0,JsonPacket());
        bool ret= send(e.data().str());//talk to server
        if(!ret){
            prt(info,"fail send");
        }
    }
    void set_config(QString data)
    {
        string str=data.toStdString().data();
        JsonPacket pkt_tmp(str);
        RequestPkt e(AppInputData::Operation::SET_CONFIG,0,pkt_tmp);
        bool ret= send(e.data().str());//talk to server
        if(!ret){
            prt(info,"fail send");
        }
    }
    void add_camera(string url,int index)
    {
//        RequestPkt pkt(AppInputData::Operation::INSERT_CAMERA,index,PlayerWidget::make_test_camera_data(url).data());
//        bool ret= send(pkt.data().str());//talk to server
//        if(!ret){
//            prt(info,"fail send");
//        }
    }
    void del_camera(int index)
    {
        RequestPkt pkt(AppInputData::Operation::DELETE_CAMERA,index,JsonPacket());
        bool ret= send(pkt.data().str());//talk to server
        if(!ret){
            prt(info,"fail send");
        }

    }
    void send_cmd(JsonPacket pkt)
    {
        //        RequestPkt pkt(App::Operation::MODIFY_CAMERA,index,JsonPacket());
        bool ret= send(pkt.str());//talk to server
        if(!ret){
            prt(info,"fail send");
        }
    }

    int count_begin_symbol(QByteArray ba)
    {
        char *tmp=ba.data();
        int sz=ba.size();
        int ret=0;
        int i;
        for( i=0;i<sz;i++){
            if(tmp[i]=='{'){
                ret++;
            }
        }
        return ret;
    }
    int count_end_symbol(QByteArray ba)
    {
        char *tmp=ba.data();
        int sz=ba.size();
        int ret=0;
        int i;
        for( i=0;i<sz;i++){
            if(tmp[i]=='}'){
                ret++;
            }
        }
    }
    void find_pos(QByteArray ba,int &endpos,int &left)
    {

    }
    bool try_get_obj_buf(QByteArray src,QByteArray &dst)
    {
        int ret=false;
        int stack=0;
        char *p_tmp=src.data();
        bool flg=false;
        //    bool flg_real_end=false;
        //char *p_start=src.data();
        dst.clear();
        dst.append(src);
        int i;
        if(count_begin_symbol(src)>0){
            for(i=0;i<src.size();i++){
                if(p_tmp[i]=='{')
                {
                    stack++;
                    flg=true;
                }
                if(p_tmp[i]=='}')
                    stack--;


                if(stack==0&&flg)
                {

                    break;
                }

            }
            if(i<src.size()){
                ret=true;
                if(src[i+1]=='\n')
                    dst.truncate(i+2);
                else
                    dst.truncate(i+1);
            }
        }
        return ret;
    }

    //dst:a sting which contain a compelete json object
    //src:a slice ofstream buffer
    //tmp_msg:last slice of buffer(maybe begining of json string)
    bool get_valid_buf(QByteArray &src,QByteArray &dst)
    {
        if(try_get_obj_buf(src,dst)){
            src.remove(0,dst.size());
            return true;
        }
        return false;

    }

    bool get_valid_buf1(QByteArray &src,QByteArray &dst)
    {
        string src1(src.data());
        string dst1(dst.data());
        bool ret=false;

        if(JsonStr::get_valid_buf(src1,dst1)){
            dst.setRawData(dst1.data(),dst1.size());
            src.setRawData(src1.data(),src1.size());
            //  src=QByteArray(src1.data(),src1.size());
            ret= true;
        }
        ret= false;
        return ret;
    }
public slots:
    void handle_connected()
    {
        prt(info,"connected");
        emit connect_done();
    }

    void handle_server_msg1()
    {

        //tmp_msg1.clear();
        string  ret_str=tcp_socket->readAll().data();
        string valid_buf;
        valid_buf.clear();
        tmp_msg1.append(ret_str);
        prt(info,"get server msg(%d bytes): %s",ret_str.size(),ret_str.data());
        //   while(get_valid_buf1(tmp_msg,valid_buf)) {
        while(JsonStr::get_valid_buf(tmp_msg1,valid_buf)) {
            JsonPacket pkt(valid_buf);
            prt(info,"get object(%d bytes)---->:%s(%d bytes left) ",valid_buf.size(),valid_buf.data(),tmp_msg.size());
            if(valid_buf.size()>0)
                need_read=true;
            emit server_msg(pkt.str().data());
        }

    }
    void handle_server_msg()
    {

#if 0
        ret_ba=tcp_socket->readAll();
        QByteArray valid_buf;
        valid_buf.clear();
        tmp_msg.append(ret_ba);
        while(get_valid_buf(tmp_msg,valid_buf)) {
            DataPacket pkt(string(valid_buf.data()));
            prt(info,"get %d bytes ",valid_buf.size());
            if(valid_buf.size()>0)
                need_read=true;
            int op=pkt.get_int("type");
            emit get_ret(pkt.data().data());
            switch(op)
            {
            case Pvd::GET_CONFIG:
            {
                emit signal_get_config_done(true,DataPacket(pkt.get_value("config")).data());
            }
                break;
            case Pvd::NEED_UPDATE:
                need_update_config();
                break;
            default:break;
            }
        }
#else
        tmp_msg.clear();
        ret_ba=tcp_socket->readAll();
        QByteArray valid_buf;
        valid_buf.clear();
        tmp_msg.append(ret_ba);
        prt(info,"get server msg %s",ret_ba.data());
        //   while(get_valid_buf1(tmp_msg,valid_buf)) {
        while(get_valid_buf(tmp_msg,valid_buf)) {
            JsonPacket pkt(string(valid_buf.data()));
            prt(info,"get object(%d bytes)---->:%s(%d bytes left) ",valid_buf.size(),valid_buf.data(),tmp_msg.size());
            if(valid_buf.size()>0)
                need_read=true;
            emit server_msg(pkt.str().data());
        }

#endif
    }

    void connect_to_server(QString ip)
    {
        if(ip.size()){
            if(tcp_socket->state()==QAbstractSocket::ConnectedState)
                tcp_socket->disconnectFromHost();
            if(ip==server_ip){
                prt(info,"reconnecting %s",server_ip.toStdString().data());
                tcp_socket->connectToHost(server_ip,12345);
            }else{
                prt(info,"connecting %s",ip.toStdString().data());
                server_ip=ip;
                tcp_socket->connectToHost(server_ip,12345);
            }

        }else{
        }
    }
    void  displayError(QAbstractSocket::SocketError socketError)
    {
        prt(info,"err when connecting to server");
        switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        {
            prt(info,"err when connecting to server");
            break;
        }
        case QAbstractSocket::HostNotFoundError:
        {
            prt(info,"err when connecting to server");
            break;
        }
        case QAbstractSocket::ConnectionRefusedError:
        {
            prt(info,"err when connecting to server");
            break;
        }
        default:
            break;
        }
    }


signals:
    void signal_get_config_done(bool,string);
    void need_update_config();
    void connect_done();
    void get_ret(QByteArray);
    void send_done(QByteArray);
    void server_msg(QString);
public:
    bool send(const QByteArray ba)
    {
        prt(info,"sending---------------> %s",ba.data());
        emit send_done(ba);
        bool ret=false;
        int write_bytes=0;
        int len=ba.length();
        write_bytes=tcp_socket->write(ba.data(),ba.length());
        bool flush_ret=tcp_socket->flush();//TODO,not work for flush
        if(flush_ret){
            prt(info,"flush ok");
        }else{
            prt(info,"flush err");
        }

        if(write_bytes!=len){
            prt(info,"send %d bytes in state %d , %d bytes left",write_bytes,tcp_socket->state(),len-write_bytes);
        }else{
            ret=true;
        }
        return ret;
    }
    bool send(const string ba)
    {
        emit send_done(QByteArray(ba.data()));
        bool ret=false;
        int write_bytes=0;
        int len=ba.length();
        write_bytes=tcp_socket->write(ba.data(),ba.length());
        bool flush_ret=tcp_socket->flush();//TODO,not work for flush
        if(flush_ret){
            prt(info,"send %d bytes----> %s",write_bytes,ba.data());
        }else{
            prt(info,"send err");
        }

        if(write_bytes!=len){
            prt(info,"send %d bytes in state %d , %d bytes left",write_bytes,tcp_socket->state(),len-write_bytes);
        }else{
            ret=true;
        }
        return ret;
    }
private:
    QString server_ip;
    QTcpSocket *tcp_socket;
    QDataStream in;
    QByteArray ret_ba;
    bool need_read;
};


#endif // MISC_H
