#ifndef APP_H
#define APP_H

#include "configmanager.h"
#include "camera.h"
#include "app_data.h"
#include "server.h"
#include "tool.h"
#include <mysql/mysql.h>
#include <list>

#define MAX_CAM_NUM 1000
#define COUNT_SECONDS 300
class DatabaseInstance
{
public:
    static DatabaseInstance &get_instance()
    {
        static DatabaseInstance ins;
        return ins;
    }
    bool connect()
    {
        if(!mysql_real_connect(sql,host.data(),user.data(),passwd.data(),NULL,3306,0,0)){
            sql=NULL;
            const char *err= mysql_error(sql);
            prt(info,"connect database error: %s" ,err);
            mysql_close(sql);
            return false;
        }else{
            prt(info,"connect database %s ok",db.data());
            int ret=  mysql_select_db(sql,db.data());
            prt(info,"ok use %d",ret);
            return true;
        }
    }

    bool connect(string host,string user,string passwd,string db)
    {
        if(!mysql_real_connect(sql,host.data(),user.data(),passwd.data(),NULL,3306,0,0)){
            sql=NULL;
            const char *err= mysql_error(sql);
            prt(info,"connect database error: %s" ,err);
            mysql_close(sql);
            return false;
        }else{
            prt(info,"connect database %s ok",db.data());
            int ret=  mysql_select_db(sql,db.data());
            prt(info,"ok use %d",ret);
            return true;
        }
    }
    void print_result()
    {
    }
    void print_table()
    {
        if(!query_flag){
            prt(info,"eror:can not print table  because query failed");
            return ;
        }
#if 1
        //   MYSQL_FIELD *field;
        if(!resid)
            return;
        while((field = mysql_fetch_field(resid)))
        {
            printf(" %s ", field->name);
        }
#endif

        cout<<endl;
        // query("select * from  table1");
        //  resid = mysql_store_result(sql);
        MYSQL_ROW row;
        unsigned int num_fields;
        unsigned int i;

        num_fields = mysql_num_fields(resid);
        // return ;
        while ((row = mysql_fetch_row(resid)))
        {
            unsigned long *lengths;
            lengths = mysql_fetch_lengths(resid);
            for(i = 0; i < num_fields; i++)
            {
                //  field = mysql_fetch_field(resid);
                //  printf("field   %p\n", field);
                // printf("field name %s\n", field->name);
                printf("[%.*s] ", (int) lengths[i],
                       row[i] ? row[i] : "NULL");
            }
            printf("\n");
        }
    }
    void query(char *statement)
    {

        //mysql_free_result(resid );
        int mysqlret = mysql_real_query(sql,statement,(unsigned int)strlen(statement));
        if(mysqlret!=0)
        {
            query_flag=false;
            //       abort();
            //  assert(false);
            const char *err_str=  mysql_error(sql);
            prt(info,"error: %s" , err_str);
        }else{
            query_flag=true;
            resid = mysql_store_result(sql);
        }
    }
    ~DatabaseInstance()
    {
        mysql_close(sql);
    }
private:
    DatabaseInstance(char *host,char *user,char*passwd,char *db):
        host(host),user(user),passwd(passwd),db(db),sql(NULL),query_flag(false),need_connect(true)
    {
        sql=mysql_init(NULL);
    }

    DatabaseInstance()
    {
        sql=mysql_init(NULL);
    }
    string host;
    string user;
    string passwd;
    string db;
    MYSQL *sql;
    MYSQL_RES *resid;
    MYSQL_FIELD *field;
    bool query_flag;
    bool need_connect;
    // MYSQL_ROW row_str_list;
};

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
            Camera *cm=cms[index-1];

            vector<Camera*>::iterator it=cms.begin();
            cms.erase(it+index-1);
            private_data.delete_camera(index);

#if 0
            delete cms[index-1];
#else
            thread([this,index,cm](){ delete cm;}).detach();
#endif
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
                private_data.modify_camera(c->get_data(),index);
                ret=true;
            }
        }
        return ret;
    }
    void check_point()
    {
        return ;

//        while(1){
//            this_thread::sleep_for(chrono::seconds(1));
//        }
        int count=dir_count("/ftphome/pic");
      //  prt(info,"/ftphome/pic pics count %d---->",count);
       // prt(info,"server camera count %d---->",cms.size());
        int left=30;
        if(count>left){
            delete_dir_files("/ftphome/pic",count,left);
        }
    }


    //////////db/////////


    DatabaseInstance  &  get_database()
    {
        DatabaseInstance &ins=DatabaseInstance::get_instance();
        string user="root";
        string passwd="root";
        string db="AIPD";
        string host="localhost";
        string fn;
        fn.append("/ftphome/pic/");
        if(sql_need_connect){
            ins.connect(host,user,passwd,db);
            sql_need_connect=false;
        }
        return ins;
    }
    void get_tis_str(char *buf,char *time, int type)
    {

      //  "INSERT INTO TIS ( `RecordID`, `SST`, `SP`, `AnalyceID`, `SAvenue`, `CameraID`, `RegionID`, `TEType`, `TEPAddr`, `TEVAddr`)\
      //   VALUES  ( '3', '2018-03-07 17:46:24', '60', '32', '大学城', '12', '12', '21', 'pic_path', 'video_path');"

        sprintf(buf," INSERT INTO TIS ( `RecordID`, `SST`, `SP`, `AnalyceID`, `SAvenue`, `CameraID`, `RegionID`, `TEType`, `TEPAddr`, `TEVAddr`)\
         VALUES  ( '3', '%s', '60', '32', '大学城', '12', '12', '%d', 'pic_path', 'video_path')",time,type);
    }
    void single_insert(EventRegionObjectOutput data,Mat frame)
    {
        //if(mvddata.DriveAwayData.size()>0){
            if(1){
                  prt(info,"get incident");
            DatabaseInstance &ins=DatabaseInstance::get_instance();
            string user="root";
            string passwd="root";
            string db="AIPD";
            string host="localhost";
            string fn;
            fn.append("/ftphome/pic/");

            stringstream  stream;
            stream<<get_time_point_ms();
            fn.append(stream.str());
            fn.append(".png");
            imwrite(fn,frame);
            if(sql_need_connect){
                ins.connect(host,user,passwd,db);
                sql_need_connect=false;
            }
            static char bf[1000];
            memset(bf,0,1000);
            get_tis_str(bf,get_sql_time(),1);
            prt(info,"start qurey");
            ins.query(bf);
            prt(info,"stop qurey");
        }
    }
    string insert_pic(Mat frame)
    {
            if(1){
                  prt(info,"get incident");

            string user="root";
            string passwd="root";
            string db="AIPD";
            string host="localhost";
            string fn;
            fn.append("/ftphome/pic/");

            stringstream  stream;
            stream<<get_time_point_ms();
            fn.append(stream.str());
            fn.append(".png");
            imwrite(fn,frame);
            prt(info,"get pic");
            stream<<".png";
            return stream.str();


        }
    }

    string insert_pic_ex(Mat frame,vector <VdPoint> outline,int type,vector <VdPoint> region)
    {
           string str;

           switch(type){
            case EventRegion::OVER_SPEED:
               str.append("OVER_SPEED");
               break;
           case EventRegion::REVERSE_DRIVE:
                 str.append("REVERSE_DRIVE");
              break;
           case EventRegion::STOP_INVALID:
                 str.append("STOP_INVALID");
              break;
           case EventRegion::NO_PEDESTRIANTION:
                 str.append("NO_PEDESTRIANTION");
              break;
           case EventRegion::DRIVE_AWAY:
                 str.append("DRIVE_AWAY");
              break;
           case EventRegion::CONGESTION:
                 str.append("CONGESTION");
              break;
           case EventRegion::AbANDON_OBJECT:
                 str.append("AbANDON_OBJECT");
              break;
           default:break;

           }

           putText(frame, str, Point(100,130),CV_FONT_HERSHEY_SIMPLEX,1,Scalar(0,255,255),3,8);
           VdRect rct= vers_2_rect(region);
           VdPoint offset(rct.x,rct.y);
            prt(info," start record rect incident");
           for(VdPoint p:outline){
            prt(info," () %d %d)",p.x,p.y);
           }
            prt(info," end record rect incident ");
           prt(info,"offset %d %d",rct.x,rct.y);
           if(outline.size()==4){
               for(int i=0;i<outline.size()-1;i++){
                   VdPoint start=add_point_offset(outline[i],offset);
                   VdPoint end=add_point_offset(outline[i+1],offset);
                          line(frame, Point(start.x,start.y),Point( end.x,end.y), Scalar(255, 255 ,0), 3, 8, 0 );
               }
               VdPoint start=add_point_offset(outline.front(),offset);
               VdPoint end=add_point_offset(outline.back(),offset);
                line(frame, Point(start.x,start.y),Point( end.x,end.y), Scalar(255, 255 ,0), 3, 8, 0 );

           }
            string user="root";
            string passwd="root";
            string db="AIPD";
            string host="localhost";
            string fn;
            fn.append("/ftphome/pic/");

            stringstream  stream;
            stream<<get_time_point_ms();
            fn.append(stream.str());
            fn.append(".png");
            imwrite(fn,frame);
            prt(info,"get pic");
            stream<<".png";
            return stream.str();



    }
    void insert_database(CameraOutputData data,int index,Mat frame)
    {
#if 1


        if(index<1)
        {
            prt(info,"err in inserting database");
            //           / return -1;
        }
        CameraInputData input=cms[index-1]->get_data();
        #if 0
        for(int i=0;i<data.DetectionResult.size();i++){
            DetectRegionOutputData d=data.DetectionResult[i];
            DetectRegionInputData  id= input.DetectRegion[i];
            JsonPacket rst=d.Result;
            //   prt(info," %s ",id.SelectedProcessor.data());
            if(id.SelectedProcessor==LABLE_PROCESSOR_MVD){
                MvdProcessorOutputData mvddata(rst);
                outputs[index-1].push_back(mvddata);
                vector<MvdProcessorOutputData> &ops=outputs[index-1];
                ops.push_back(mvddata);
                // prt(info," %s ",mvddata.data().str().data());


                int new_event_flag[7];
                memset(new_event_flag,0,7*sizeof(int));
                for( EventRegionObjectOutput  eo:  mvddata.EventObjects){
                   new_event_flag[eo.Type-1]++;
                }
                for(int k=0;k<7;k++){
                    if(new_event_flag[k]>old_event_flag[k]){
                        for(int t=0;t<mvddata.EventObjects.size();t++){
                            if(mvddata.EventObjects[t].Type==k){
                                single_insert(mvddata.EventObjects[t],frame);
                                break;
                            }
                        }

                    }
                       old_event_flag[k]=new_event_flag[k];
                }

//                if(mvddata.DriveAwayData.size()>0){
//                    prt(info,"get incident");
//                    DatabaseInstance &ins=DatabaseInstance::get_instance();
//                    string user="root";
//                    string passwd="root";
//                    string db="AIPD";
//                    string host="localhost";
//                    string fn;
//                    fn.append("/ftphome/pic/");

//                    stringstream  stream;
//                    stream<<get_time_point_ms();
//                    fn.append(stream.str());
//                    fn.append(".png");
//                    imwrite(fn,frame);
//                    if(sql_need_connect){
//                        ins.connect(host,user,passwd,db);
//                        sql_need_connect=false;
//                    }
//                    static char bf[1000];
//                    memset(bf,0,1000);
//                    get_tis_str(bf,get_sql_time(),1);
//                    prt(info,"start qurey");
//                    ins.query(bf);
//                      prt(info,"stop qurey");
//                    ins.query("INSERT INTO TIS \
//                              ( `RecordID`, `SST`, `SP`, `AnalyceID`, `SAvenue`, `CameraID`, `RegionID`, `TEType`, `TEPAddr`, `TEVAddr`)\
//                              VALUES\
//                              ( '3', '2018-03-07 17:46:24', '60', '32', '大学城', '12', '12', '21', 'pic_path', 'video_path');");

//                            ins.query(" INSERT INTO PS ( `RecordID`, `SST`, `SP`, `AnalyceID`, `SAvenue`, `CameraID`, `RegionID`, `PSum`, `PPSum`, `PNSum`, `PDensity`) \
//                                      VALUES \
//                                      ('2', '2018-10-17 16:34:07', '90', '1', '沙河西路', '2', '2', '15', '10', '5', '40');");


             //   }
             }




            //  .PersonFlow2;
            //  mvddata.

        }
        #else
        for(int i=0;i<data.DetectionResult.size();i++){
            DetectRegionOutputData d=data.DetectionResult[i];
            DetectRegionInputData  id= input.DetectRegion[i];
            JsonPacket rst=d.Result;
            //   prt(info," %s ",id.SelectedProcessor.data());
            if(id.SelectedProcessor==LABEL_PROCESSOR_MVD){
                MvdProcessorOutputData mvddata(rst);
                for(EventRegionObjectOutput eo:mvddata.EventObjects){
                    if(!exist_in_last(eo)){
                        prt(info," inserting  event %d ",eo.Type);
                      //  string name= insert_pic(frame);
                        string name= insert_pic_ex(frame,eo.Vers,eo.Type,id.ExpectedAreaVers);
                        string path("/ftphome/pic/");
                        path.append(name);

                        database_insert_tis(get_sql_time(),eo.Type,path.data());

                    }
                }
                last_events.clear();
                last_events=mvddata.EventObjects;
                //outputs[index-1].push_back(mvddata);
                //vector<MvdProcessorOutputData> &ops=outputs[index-1];
                //ops.push_back(mvddata);
            }
        }
        #endif
#endif

    }
    inline bool exist_in_last(EventRegionObjectOutput event)
    {
        for(EventRegionObjectOutput e:last_events){
            if(e.EventID==event.EventID&&e.Type==event.Type){
                return true;
            }
        }
        return false;
    }
    void database_insert_tis(const char *sql_time,int type,const char * pic_path)
    {
        prt(info,"event %d",type);
        DatabaseInstance &ins=DatabaseInstance::get_instance();

        string user="root";
        string passwd="root";
        string db="AIPD";
        string host="localhost";
        if(sql_need_connect){
            ins.connect(host,user,passwd,db);
            sql_need_connect=false;
        }
//        ins.query("INSERT INTO TIS \
//        ( `RecordID`, `SST`, `SP`, `AnalyceID`, `SAvenue`, `CameraID`, `RegionID`, `TEType`, `TEPAddr`, `TEVAddr`)\
//           VALUES\
//        ( '3', '2018-03-07 17:46:24', '60', '32', 'abc', '12', '12', '21', 'pic_path', 'video_path');");

        char buf[1000];memset(buf,0,1000);
#if 1
        sprintf(buf,"INSERT INTO TIS \
                ( `RecordID`, `SST`, `SP`, `AnalyceID`, `SAvenue`, `CameraID`, `RegionID`, `TEType`, `TEPAddr`, `TEVAddr`)\
                   VALUES\
                ( '3', '%s', '60', '32', 'abcde', '12', '12', '%d', '%s', 'video_path');",sql_time,type,pic_path);
        #else

                sprintf(buf,"INSERT INTO TIS \
                        ( `RecordID`, `SST`, `SP`, `AnalyceID`, `SAvenue`, `CameraID`, `RegionID`, `TEType`, `TEPAddr`, `TEVAddr`)\
                           VALUES\
                        ( '3', '%s', '60', '32', 'abcde', '12', '12', '%d', '%s', 'video_path');",get_sql_time(),1,"5678");
        #endif
        ins.query(buf);
    }

    void count_fun1()
    {

        int count_time=COUNT_SECONDS;//10s
        int person_count[100];
        int car_count[100];
        int speed_count[100];


        int old_exist[100];//max lane num
        memset(old_exist,0,100);
        while((get_time_second()%6)){
            this_thread::sleep_for(chrono::microseconds(1000000));
        }
        while(!quit_count){
            if(count_time--==1){
                prt(info,"time up , start cal ---------------------------->");
               // lock.lock();
                int camera_size=private_data.CameraData.size();
                for(int loop_cams=0;loop_cams<camera_size;loop_cams++){
                    prt(info,"cam %d",loop_cams);
                    CameraInputData cd= private_data.CameraData[loop_cams];
                    int region_size=cd.DetectRegion.size();
                    vector< MvdProcessorOutputData>  &cam_out=outputs[loop_cams];
                    for(int loop_regions=0;loop_regions<region_size;loop_regions++){
                        prt(info,"region %d",loop_regions);
                        DetectRegionInputData dr=  cd.DetectRegion[loop_regions];
                        MvdProcessorInputData mvddata(  dr.ProcessorData);
                        int lane_size=mvddata.LaneData.size();
                        if(lane_size>=100){
                            prt(info,"max lane  100 ,err");
                            continue;
                        }

                        DatabaseInstance &ins=get_database();

                        for(int i=0;i<cam_out.size();i++){
                                  prt(info,"frame %d",loop_regions);
                            MvdProcessorOutputData tmp=cam_out[i];
                            // tmp.CongestionData
                            for(int j=0;j<tmp.LaneOutputData.size();j++){
                                if(old_exist[j]==0&&tmp.LaneOutputData[j].NearCarExist)
                                {
                                    car_count[j]++;
                                    speed_count[j]+=tmp.LaneOutputData[j].VehicleSpeed;

                                }
                                old_exist[j]=tmp.LaneOutputData[j].NearCarExist;
                            }
                        }
                        for(int j=0;j<mvddata.LaneData.size();j++){
                            database_insert_vs(j,speed_count,car_count);
                        }


                     }//loop regions
                }//loop cams
                for(int i=0;i<100;i++)
                    outputs[i].clear();

                  count_time=COUNT_SECONDS;
                    prt(info," stop cal ---------------------------->");
            }

            this_thread::sleep_for(chrono::microseconds(1000000));
        }
    }

#define INSERT_DB_MEM(xxx)       \
    stream<< "'";\
    stream<<xxx;\
    stream<< "'";
    void database_insert_vs(int lane_index,int speed_count[],int car_count[])
    {
        int vs_record_id=3;

        //strstream tmp1;tmp1<<COUNT_SECONDS;
        char buf_tmp[100];memset(buf_tmp,0,100);sprintf(buf_tmp,"%d",COUNT_SECONDS);
        string vs_sp(buf_tmp);
        string vs_sst=get_sql_time();
        string vs_analyce_id="2";
        string vs_savenue="12345";
        string vs_CameraID="1";
        string vs_LaneID="12";
        string vs_VSum="";//car_count
        string vs_VPSum="0";
        string vs_VNSum="0";
        string vs_VDSum="0";
        string vs_VBSum="0";
        string vs_VCSum="0";//car_count
        string vs_VMSum="0";
        string vs_VKSum="0";
        if(car_count[lane_index]){
            prt(info,"car num %d, speed total %d",car_count[lane_index],speed_count[lane_index]);
            int average_speed=speed_count[lane_index]/car_count[lane_index];
            memset(buf_tmp,0,100);
            sprintf(buf_tmp,"%d",average_speed);
            //  tmp2<<average_speed;
        }else{
              memset(buf_tmp,0,100);
                sprintf(buf_tmp,"%d",0);
        }
        string vs_ASpeed(buf_tmp);
        string vs_ATime="1";
        string vs_ASpace="1";
        string vs_AVM="1";
        string vs_AOccupy="1";
        string vs_RState="1";
        DatabaseInstance &ins=DatabaseInstance::get_instance();
        stringstream stream;
        stream<< "INSERT INTO VS \
            ( `RecordID`, `SST`, `SP`, `AnalyceID`, `SAvenue`, `CameraID`, `LaneID`,\
              `VSum`, `VPSum`, `VNSum`, `VTSum`, `VBSum`, `VCSum`, `VMSum`,\
              `VKSum`, `ASpeed`, `ATime`, `ASpace`, `AVM`, `AOccupy`,`RState`) VALUES";
        stream<< "(";

        INSERT_DB_MEM(vs_record_id) stream<< ",";
        INSERT_DB_MEM(vs_sst) stream<< ",";
        INSERT_DB_MEM(vs_sp) stream<< ",";
        INSERT_DB_MEM(vs_analyce_id) stream<< ",";
        INSERT_DB_MEM(vs_savenue) stream<< ",";
        INSERT_DB_MEM(vs_CameraID) stream<< ",";
        INSERT_DB_MEM(vs_LaneID) stream<< ",";


        INSERT_DB_MEM(car_count[lane_index]) stream<< ",";
        INSERT_DB_MEM(vs_VPSum) stream<< ",";
        INSERT_DB_MEM(vs_VNSum) stream<< ",";
        INSERT_DB_MEM(vs_VDSum) stream<< ",";
        INSERT_DB_MEM(vs_VBSum) stream<< ",";
        INSERT_DB_MEM(car_count[lane_index];) stream<< ",";
        INSERT_DB_MEM(vs_VMSum) stream<< ",";

        INSERT_DB_MEM(vs_VKSum) stream<< ",";
        INSERT_DB_MEM(vs_ASpeed) stream<< ",";
        INSERT_DB_MEM(vs_ATime) stream<< ",";
        INSERT_DB_MEM(vs_ASpace) stream<< ",";
        INSERT_DB_MEM(vs_AVM) stream<< ",";
        INSERT_DB_MEM(vs_AOccupy) stream<< ",";
        INSERT_DB_MEM(vs_RState)
        stream<< ");";

        prt(info,"%s",stream.str().data());
        if(stream.str().size()>1000){
            prt(info,"sql too long");
            return;
        }
        char buf1[1000];
        memset(buf1,0,1000);
        sprintf(buf1,"%s",stream.str().data());
        ins.query(buf1);
    }

    //////db end///////
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

    bool sql_need_connect;
    thread *p_count_thread;
    vector <Mat> frames;
    bool quit_count;
    vector <EventRegionObjectOutput> last_events;
    vector <MvdProcessorOutputData> outputs[MAX_CAM_NUM];//support 100 cameras
    mutex lock;
};


#endif // APP_H
