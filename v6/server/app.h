#ifndef APP_H
#define APP_H

#include "configmanager.h"
#include "camera.h"
#include "app_data.h"
#include "server.h"
#include "tool.h"
class DestClient{
public:

    DestClient(string i):ip(i)
    {

    }

    string get_ip()
    {
        return ip;
    }

private:
    string ip;
};
class App:public VdData<AppInputData>
{
public:
    App();
    App(ConfigManager *p);
    ~App();
    void start()
    {
    }
private:
    bool process_event(RequestPkt e,ReplyPkt &r);
    void process_client_cmd(Session *clt,char *data,int len);
    void client_data_request(Session *clt, char *data, int len);
    void process_camera_data(Camera *clt,CameraOutputData);
    Camera* new_cam(CameraInputData data)
    {
        return new Camera(data,bind(&App::process_camera_data,
                                    this,placeholders::_1,
                                    placeholders::_2));
    }
    void save_data()
    {
        DeviceConfigData data(private_data);
        p_cm->set_config(data.data().str());//get
    }
    JsonPacket load_data()
    {
        DeviceConfigData data( p_cm->get_config());
        private_data=data.data();
    }


    void restart_all()
    {
        stop_cams();
        start_cams();
    }
    void start_cams()
    {
        for(CameraInputData p:private_data.CameraData){
            cms.push_back(new_cam( p));
        }
    }
    void stop_cams()
    {
        for(Camera *c:cms){
            delete c;
        }
        cms.clear();
    }
    bool add_camera(int index,CameraInputData data)//after who ?  0~size
    {
        bool ret=false;
        if(0<=index&&index<=cms.size()){
            Camera *c=new_cam( (data));
            vector<Camera*>::iterator it=cms.begin();
            cms.insert(it+index,c);
            private_data.insert_camera(data,index);
            ret=true;
        }
        return ret;
    }
    bool del_camera(int index)//delete who ? 1~size
    {
        if(1<=index&&index<=cms.size()){
#if 0
            delete cms[index-1];
#else
            thread([this,index](){ delete cms[index-1];}).detach();
#endif
            vector<Camera*>::iterator it=cms.begin();
            cms.erase(it+index-1);
            private_data.delete_camera(index);
            return true;
        }
        return false;
    }
    bool mod_camera(int index,RequestPkt pkt)//delete who ? 1~size
    {
        bool ret=false;
        if(1<=index&&index<=cms.size()){
            vector<Camera*>::iterator it=cms.begin();
            Camera *c=cms[index-1];
            if(c->modify(pkt)){
                private_data.modify_camera(c->data(),index);
                ret=true;
            }
        }
        return ret;
    }
    void check_point()
    {

//        while(1){
//            this_thread::sleep_for(chrono::seconds(1));
//        }
        int count=dir_count("/ftphome/pic");
        prt(info,"check files %d---->",count);
        int left=1000;
        if(count>left){
            delete_dir_files("/ftphome/pic",count,left);
        }
    }
private:
    vector <Session*> *stream_cmd;//clients who connected to our server
    string str_stream;//
    vector <Camera*> cms;
    LocationService lservice;
    ConfigManager *p_cm;
    vector <DestClient > dest_clients;
    string client_tmp_ip;
    int udp_fd;
    Timer1 watch_dog;
};


#endif // APP_H
