#ifndef SERVER_H
#define SERVER_H
#include "tool.h"
#ifdef IS_UNIX
#include "socket.h"
#endif
#define BUF_SIZE 10000
//#define BUF_SIZE 10//if size if 10,
//recived byte sequence maybe interupt by other connection,and become disorder
#define IP_STR_LEN 20
class Session{
public:
    Session(int fd,string ip,int port):quit(false),client_ip(ip),
        client_port(port)
    {
        skt=fd;
        int ul = 0;
        ioctl(fd, FIONBIO, &ul);
        prt(info,"handle new connection: %s ",ip.data());
        auto func_recv=bind(&Session::recv,this);

        trd=new thread([func_recv](){func_recv();});
    }
    ~Session()
    {
        prt(info,"quit connection: %s ",ip().data());
        quit=true;
        trd->join();
        delete trd;

    }

    int send(const char *buf,int len)
    {
        int ret= Socket::SendDataByTcp(skt,buf,len);

        if(ret){
            prt(info,"send start(%d bytes)<========",ret);
            printf("%s",buf);
            prt(info,"send end(%d bytes)<========",ret);
        }else{
            prt(info,"send fail",ret);
        }
        return ret;
    }
    int recv()
    {
        while(!quit){
            int ret=Socket::RecvDataByTcp1(skt,buf,BUF_SIZE);
            if(ret){
                prt(info,"read %d bytes",ret);
                process_data(this,buf,ret);
            }else{
                prt(info,"read socket error");
                break;
                cout<<"socket maybe closed,retry read after 1sec"  <<endl;
                this_thread::sleep_for(chrono::seconds(1));
            }
        }
        end_this(this);
        return 0;
    }
    string ip()
    {
        return client_ip;
    }
    int port()
    {
        return client_port;
    }
private:
    int skt;
    bool quit;
    char buf[BUF_SIZE];
    std::thread *trd;
    string client_ip;
    int client_port;
public:
    function <void(Session *)> end_this=[](Session *){ cout<<"we  can quit now"<<endl;};
    function <void(Session *,char *data,int len)> process_data=[](Session *,char *data,int len){ cout<<"process data"<<endl;};

};


class Tcpserver{
    function <void(Session *,char *,int)>fct;
public:
    Tcpserver(int p):port(p),quit(false)
    {
        fd= Socket::StartTcpServerSock(port,1000,1000);
        if(fd>0){
            auto func=bind(&Tcpserver::listen,this);
            trd=new thread([func](){func();});
        }else{
            prt(info,"bind port fail");
        }
    }
    Tcpserver(int p,function <void(Session *,char *,int)>fc):port(p),quit(false),fct(fc)
    {
        fd= Socket::StartTcpServerSock(port,1000,1000);
        if(fd>0){
            auto func=bind(&Tcpserver::listen,this);
            trd=new thread([func](){func();});
        }else{
            prt(info,"bind port fail");
        }
    }
    Tcpserver(vector<Session*> *&clts,int p,function <void(Session *,char *,int)>fc):port(p),quit(false),fct(fc)
    {
        clts=&clients;
        fd= Socket::StartTcpServerSock(port,1000,1000);
        if(fd>0){
            auto func=bind(&Tcpserver::listen,this);
            trd=new thread([func](){func();});
        }else{
            prt(info,"bind port fail");
        }
    }
    ~Tcpserver()
    {
        quit=true;
        if(trd){
            trd->join();
            int sz=clients.size();
            for(int i=sz-1;i>=0;i--)
            {
                delete clients[i];
                clients.pop_back();
            }
            delete trd;
        }
    }

    void listen()
    {
        while(!quit){

            memset(peer_ip,0,IP_STR_LEN);
            int cl_fd=Socket::WaitTcpConnect(fd,1,peer_ip,&peer_port);
            if(cl_fd>0){
                prt(info,"get connect from %s:%d",peer_ip,peer_port);
                Session *s=new Session(cl_fd,peer_ip,peer_port);
                s->end_this=bind(&Tcpserver::quit_session,this,placeholders::_1);
                s->process_data=fct;
                clients.push_back(s);
            }
        }
    }
    void send_test()
    {
        if(clients.size())
        {
            clients[0]->send("123456",6);
        }
    }
    void size()
    {
        cout<<clients.size();
    }
private:
    void quit_session(Session *s)
    {
        vector <Session *>::iterator e=find(clients.begin(),clients.end(),s);
        clients.erase(e);
        thread([s](){
            delete s;
        }).detach();
        s=NULL;
    }

    bool quit;
    int port;
    std::thread *trd;
    unsigned short peer_port;
    int fd;
    char peer_ip[IP_STR_LEN];
public:
    vector <Session *> clients;

};
// check port , once "pedestrian" recived , send IP to peer
class LocationService{
    int fd;
public:
    LocationService()
    {
        fd=Socket::CreatBroadcast(12348);
        prt(info,"start location");
        thread([this](){
            while(1){
                this_thread::sleep_for(chrono::microseconds(100));
                string peer_ip;
                int peer_port;
                if(check_port(peer_ip,peer_port)){
                    string ip=get_ip();
                    udp_send(peer_ip,peer_port,(char *)ip.data(),ip.length());
                }
            }

        }).detach();
    }
private:
    // client will send  pedestrian to  12348
    bool check_port(string &peer_ip,int &peer_port)
    {
        //   int fd=Socket::CreatBroadcast(12348);
        unsigned char buf[100];
        memset(buf,0,100);
        int rev_len=Socket::RecvFromBroadcast(buf,strlen("pedestrian"),fd,peer_ip,peer_port);
        // close(fd);
        string str((const char*)buf);
        if(rev_len>0&&"pedestrian"==str)
            return true;
        else
            return false;
    }

    string get_ip()
    {
        char ip[20];
        memset(ip,0,20);
        char mac[20];
        memset(mac,0,20);
        char mask[20];
        memset(mask,0,20);
        Socket::get_ipaddr(ip,mac,mask);
        return string(ip);
    }
    string get_gateway()
    {
        // Socket::get_gateway()
    }
    void udp_send(string ip,int port,char *data,int len )
    {
        // Socket::SendDataByClient((char *)data,len,ip.data(),port);
        int fd=Socket::UdpCreateSocket(0);
        Socket::UdpSendData(fd,ip.data(),port,data,len);
        close(fd);
    }
};
class TcpClient{
    function <void(char *,int)>recv_fun=[](char *data,int len){ prt(info,"get %s",data);};
    bool connected;
public:
#if 0
    TcpClient(int p):port(p),quit(false)
    {
        connected=false;
        skt=Socket::CreateTcpClientSock(0,1000);
        if(skt<=0){
            prt(info,"skt err");
        }

        thread([this](){
            while(1){
                lock.lock();
                if(!connected){
                    skt=Socket::CreateTcpClientSock(0,1000);

                    int tmp= Socket::ConnectTcpClient(skt,"192.168.1.219",6789);
                    if(tmp>0){
                        connected=true;
                    }else{
                    }
                }
                lock.unlock();
                this_thread::sleep_for(chrono::microseconds(1000000));
            }

        }).detach();


        auto func_recv=bind(&TcpClient::recv,this);

        trd=new thread([func_recv](){func_recv();});

    }
#endif
    TcpClient(string ip,int p,function <void(char *,int)>fc):ipaddr(ip),port(p),quit(false),recv_fun(fc)
    {
        connected=false;
        skt=Socket::CreateTcpClientSock(0,100000);
        if(skt<=0){
            prt(info,"skt err");
        }

        thread([this](){
            while(1){
                lock.lock();
                if(!connected){
                    close(skt);
                    skt=Socket::CreateTcpClientSock(0,1000);

                    int tmp= Socket::ConnectTcpClient(skt,ipaddr.data(),port);
                    if(tmp>0){
                        connected=true;
                        prt(info,"connect ok(%d)",skt);
                    }else{
                        prt(info,"connect err(%d)",skt);
                        // close(skt);
                    }
                }
                lock.unlock();
                this_thread::sleep_for(chrono::microseconds(1000000));
            }

        }).detach();


        auto func_recv=bind(&TcpClient::recv,this);

        trd=new thread([func_recv](){func_recv();});
    }
    ~TcpClient()
    {
        //        quit=true;
        //        if(trd){
        //            trd->join();
        //            int sz=clients.size();
        //            for(int i=sz-1;i>=0;i--)
        //            {
        //                delete clients[i];
        //                clients.pop_back();
        //            }
        //            delete trd;
        //        }
    }

    int send(const char *buf,int len)
    {
        int ret=0;
        //lock.lock();
        if(connected){
        ret= Socket::SendDataByTcp(skt,buf,len);

        if(ret){
            //prt(info,"send start(%d bytes)<========",ret);
            printf("%s",buf);
            //prt(info,"send end(%d bytes)<========",ret);
        }else{
            prt(info,"send fail",ret);
        }
        }else{
             prt(info,"send : you cant send without connect");
        }
       // lock.unlock();
        return ret;
    }
    int recv()
    {
        while(!quit){
            if(skt>0){
                memset(buf,0,10000);
                lock.lock();
               //   printf("\nflush  355\n");
                int ret=Socket::RecvDataByTcp1(skt,buf,BUF_SIZE);

                 // printf("\nflush 358 \n");
               // prt(info,"read done ");
                if(ret){
                    connected=true;
                     fflush(NULL);
                  //  prt(info,"read %d bytes ok",ret);
                    recv_fun(buf,ret);
                     fflush(NULL);
                    //process_data(this,buf,ret);
                }else{
                    prt(info,"read  error ");

                    connected=false;
                    //close(skt);

                    //this_thread::sleep_for(chrono::seconds(1));
                }
                lock.unlock();
                if(ret<=0)
                    this_thread::sleep_for(chrono::seconds(1));

            }

        }
        //    end_this(this);
        return 0;
    }

private:
    bool quit;
    string ipaddr;
    int port;
    std::thread *trd;
    unsigned short peer_port;
    //  int fd;
    char peer_ip[IP_STR_LEN];
    int skt;
    char buf[10000];
    mutex lock;
public:
    //   vector <Session *> clients;

};

#endif // SERVER_H
