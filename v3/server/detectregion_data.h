#ifndef DETECTREGION_DATA_H
#define DETECTREGION_DATA_H
#include "tool.h"
#include "jsonpacket.h"
#include "videoprocessor.h"
class AreaVersJsonDataRequest:public JsonData{
public:
    vector <VdPoint>ExpectedAreaVers;
    AreaVersJsonDataRequest(JsonPacket pkt):JsonData(pkt)
    {
        decode();
    }
    AreaVersJsonDataRequest(vector <VdPoint> vs)
    {
        ExpectedAreaVers.assign(vs.begin(),vs.end());
        encode();
    }
    void decode()
    {
        try{
            DECODE_JSONDATA_ARRAY_MEM(ExpectedAreaVers);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }
    }
    void encode()
    {
        try{
            ENCODE_JSONDATA_ARRAY_MEM(ExpectedAreaVers);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }

    }
};

class DetectRegionInputData:public JsonData,public PaintableData
{

public:
    enum OP{
        CHANGE_RECT=1,
        CHANGE_PROCESSOR,
        MODIFY_PROCESSOR
    };
    string SelectedProcessor;
    vector <VdPoint>ExpectedAreaVers;
    JsonPacket ProcessorData;//TODO: dynamic binding
    void *p_processor;
    DetectRegionInputData(JsonPacket pkt):JsonData(pkt),p_processor(NULL)
    {
        decode();
        alloc_processor();
    }
    DetectRegionInputData(JsonPacket pkt,PaintableData pd):JsonData(pkt),PaintableData(pd),p_processor(NULL)
    {
        decode();
        alloc_processor();
    }
    DetectRegionInputData(JsonPacket pdata,string type,vector<VdPoint> vers)
        :ProcessorData(pdata),SelectedProcessor(type),ExpectedAreaVers(vers),p_processor(NULL)
    {
        encode();
        alloc_processor();
    }
    void alloc_processor()
    {
        delete_processor();
        if(SelectedProcessor==LABEL_PROCESSOR_DUMMY){
            p_processor=new DummyProcessorInputData(ProcessorData);
        }
        if(SelectedProcessor==LABEL_PROCESSOR_MVD){
            p_processor=new MvdProcessorInputData(ProcessorData);
        }
    }
    void delete_processor()
    {
        if(p_processor)
            delete p_processor;
    }

    inline VdRect reshape_2_rect(vector <VdPoint> area)
    {
        int x_min=10000;
        int y_min=10000;
        int x_max=0;
        int y_max=0;
        for(VdPoint pkt: area) {
            int x=pkt.x;
            int y=pkt.y;
            if(x<x_min)
                x_min=x;
            if(x>x_max)
                x_max=x;
            if(y<y_min)
                y_min=y;
            if(y>y_max)
                y_max=y;
        }
        x_min=x_min>0?x_min:0;
        y_min=y_min>0?y_min:0;
        x_max=x_max>0?x_max:0;
        y_max=y_max>0?y_max:0;
        return VdRect(x_min,y_min,x_max-x_min,y_max-y_min);
    }
    void set_point(VdPoint p,int index)
    {
        ExpectedAreaVers[index-1]=p;
        encode();
    }

    void set_points(vector <VdPoint> p)
    {
        ExpectedAreaVers=p;
        encode();
    }

    void set_processor_data(JsonPacket p)
    {
        ProcessorData=p;
        encode();
    }

    void set_processor(string p,JsonPacket pd)
    {
        SelectedProcessor=p;
        ProcessorData=pd;
        encode();
    }
    void operator =(DetectRegionInputData dt)
    {
        config=dt.config;
        decode();
    }
    void decode()
    {
        try{
            DECODE_JSONDATA_ARRAY_MEM(ExpectedAreaVers);
            DECODE_PKT_MEM(ProcessorData);
            DECODE_STRING_MEM(SelectedProcessor);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }
    }
    void encode()
    {
        try{
            ENCODE_JSONDATA_ARRAY_MEM(ExpectedAreaVers);
            ENCODE_STRING_MEM(SelectedProcessor);
            ENCODE_PKT_MEM(ProcessorData);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }
    }
    static DetectRegionInputData get_region_test_data(JsonPacket pkt,string SelectedProcessor)
    {

        vector <VdPoint>ExpectedAreaVers;
        ExpectedAreaVers.push_back(VdPoint(0,0));
        ExpectedAreaVers.push_back(VdPoint(640,0));
        ExpectedAreaVers.push_back(VdPoint(640,480));
        ExpectedAreaVers.push_back(VdPoint(0,480));

        DetectRegionInputData rd(pkt,SelectedProcessor,ExpectedAreaVers);
        return rd;
    }

    static DetectRegionInputData get_region_test_data()
    {

        DummyProcessorInputData di=DummyProcessorInputData::get_dummy_test_data();
        //   DetectRegionInputData dri=DetectRegionInputData::get_region_test_data(di.data(),LABEL_PROCESSOR_DUMMY);

        vector <VdPoint>ExpectedAreaVers;
        ExpectedAreaVers.push_back(VdPoint(0,0));
        ExpectedAreaVers.push_back(VdPoint(640,0));
        ExpectedAreaVers.push_back(VdPoint(640,480));
        ExpectedAreaVers.push_back(VdPoint(0,480));

        DetectRegionInputData rd(di.data(),LABEL_PROCESSOR_DUMMY,ExpectedAreaVers);
        return rd;
    }


    static RequestPkt get_request(int op,int index ,JsonPacket arg)
    {
        switch (op) {
        case CHANGE_RECT:

            break;
        default:
            break;
        }
        return RequestPkt(op, index , arg);
    }

    virtual bool press(VdPoint pnt_ori)
    {

        if((point_index=p_on_v(ExpectedAreaVers,pnt_ori))){
            seizing=true;
            event_type=PaintableData::Event::MoveVer;
            return true;
        }
        if(p_on_vl(ExpectedAreaVers,pnt_ori)){
            seizing=true;
            ori_pnt=pnt_ori;
            event_type=PaintableData::Event::MoveAll;
            return true;
        }

        VdRect r=reshape_2_rect(ExpectedAreaVers);
        //   prt(info,"offset %d %d",r.x,r.y);
        VdPoint pnt(pnt_ori.x-r.x,pnt_ori.y-r.y);
        //  VdPoint pnt(pnt_ori.x,pnt_ori.y);
        if(SelectedProcessor== LABEL_PROCESSOR_DUMMY)
        {
            //            draw_text(LABEL_PROCESSOR_DUMMY,VdPoint(100,200),100,PaintableData::Blue,30);
            //            VdRect r= reshape_2_rect(ExpectedAreaVers);
            // DummyProcessorInputData data=Result;
            //            data.draw(DetectionRect.x,DetectionRect.y, draw_line,
            //                       draw_circle, draw_text);
        }
        if(SelectedProcessor== LABEL_PROCESSOR_MVD)
        {
            //MvdProcessorInputData mid(ProcessorData,paintable_data);

            //mid.press(pnt);
            MvdProcessorInputData *mi=(MvdProcessorInputData *)p_processor;
            bool ret=mi->press(pnt);
            if(ret)
                return ret;
        }
        return false;
    }
    enum CMD{
        SET_MVD=0,
        SET_DUMMY,
        CMD_COUNT
    };

    virtual bool right_press(VdPoint pnt,vector<right_press_menu_item>&items_ret)
    {

        RequestPkt p;
        if((point_index=p_on_v(ExpectedAreaVers,pnt))){
            right_press_menu_item itmmvd;
            itmmvd.text="change to mvd";
            itmmvd.pkt=get_request(DetectRegionInputData::CHANGE_PROCESSOR,0,
                                   get_region_test_data(MvdProcessorInputData::get_mvd_test_data().data(),LABEL_PROCESSOR_MVD).data());
            items_ret.push_back(itmmvd);

            right_press_menu_item itmdummy;
            itmdummy.text="change to dummy";
            itmdummy.pkt=get_request(DetectRegionInputData::CHANGE_PROCESSOR,0,
                                     get_region_test_data(DummyProcessorInputData::get_dummy_test_data().data(),LABEL_PROCESSOR_DUMMY).data());
            items_ret.push_back(itmdummy);
            return true;
        }
        if(p_on_vl(ExpectedAreaVers,pnt)){

            return true;
        }

        if(SelectedProcessor== LABEL_PROCESSOR_MVD)
        {

            VdRect r=reshape_2_rect(ExpectedAreaVers);
            // prt(info,"offset %d %d",r.x,r.y);
            VdPoint pnt_new(pnt.x-r.x,pnt.y-r.y);


            MvdProcessorInputData *mi=(MvdProcessorInputData *)p_processor;
            //  bool ret=mi->right_press(pnt_new,pkts,text);
            bool ret=mi->right_press(pnt_new,items_ret);
            if(ret){
                right_press_menu_item itm4;
                if(mi->point_index>(mi->LaneData.size()*12)){
                    MvdProcessorInputData d4(mi->data());
                    int idx=(mi->point_index-12*mi->LaneData.size()-1)/4+1;

                    d4.del_event_rect(idx);
                    itm4.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,d4.data());
                    itm4.text="del event";
                    items_ret.push_back(itm4);

                    right_press_menu_item itm;

                    vector <bool > types=mi->get_events_attr(idx);


                    {
                        itm.text="OVER_SPEED";
                        MvdProcessorInputData tmp_os(mi->data());tmp_os.reverse_event(idx,EventRegion::OVER_SPEED);
                        itm.checkable=true;
                        itm.checked=types[EventRegion::OVER_SPEED-1];
                        itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,tmp_os.data());
                        items_ret.push_back(itm);
                    }

                    {
                        itm.text="REVERSE_DRIVE";
                        MvdProcessorInputData tmp_os(mi->data());tmp_os.reverse_event(idx,EventRegion::REVERSE_DRIVE);
                        itm.checkable=true;
                        itm.checked=types[EventRegion::REVERSE_DRIVE-1];
                        itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,tmp_os.data());
                        items_ret.push_back(itm);
                    }

                    {
                        itm.text="STOP_INVALID";
                        MvdProcessorInputData tmp_os(mi->data());tmp_os.reverse_event(idx,EventRegion::STOP_INVALID);
                        itm.checkable=true;
                        itm.checked=types[EventRegion::STOP_INVALID-1];
                        itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,tmp_os.data());
                        items_ret.push_back(itm);
                    }

                    {
                        itm.text="NO_PEDESTRIANTION";
                        MvdProcessorInputData tmp_os(mi->data());tmp_os.reverse_event(idx,EventRegion::NO_PEDESTRIANTION);
                        itm.checkable=true;
                        itm.checked=types[EventRegion::NO_PEDESTRIANTION-1];
                        itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,tmp_os.data());
                        items_ret.push_back(itm);
                    }

                    {
                        itm.text="DRIVE_AWAY";
                        MvdProcessorInputData tmp_os(mi->data());tmp_os.reverse_event(idx,EventRegion::DRIVE_AWAY);
                        itm.checkable=true;
                        itm.checked=types[EventRegion::DRIVE_AWAY-1];
                        itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,tmp_os.data());
                        items_ret.push_back(itm);
                    }

                    {
                        itm.text="CONGESTION";
                        MvdProcessorInputData tmp_os(mi->data());tmp_os.reverse_event(idx,EventRegion::CONGESTION);
                        itm.checkable=true;
                        itm.checked=types[EventRegion::CONGESTION-1];
                        itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,tmp_os.data());
                        items_ret.push_back(itm);
                    }

                    {
                        itm.text="AbANDON_OBJECT";
                        MvdProcessorInputData tmp_os(mi->data());tmp_os.reverse_event(idx,EventRegion::AbANDON_OBJECT);
                        itm.checkable=true;
                        itm.checked=types[EventRegion::AbANDON_OBJECT-1];
                        itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,tmp_os.data());
                        items_ret.push_back(itm);
                    }
                    {
                        itm.text="NON_MOTOR";
                        MvdProcessorInputData tmp_os(mi->data());tmp_os.reverse_event(idx,EventRegion::NON_MOTOR);
                        itm.checkable=true;
                        itm.checked=types[EventRegion::NON_MOTOR-1];
                       // itm.checked=types[EventRegion::event_type_size-1];
                        itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,tmp_os.data());
                        items_ret.push_back(itm);
                    }

                    {
                        itm.text="revert direction";
                        int dir=mi->Events[idx-1].Direction;
                        MvdProcessorInputData tmp_os(mi->data());tmp_os.set_event_direction(idx,!dir);
                        itm.checkable=false;
                        itm.checked=false;
                        itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,tmp_os.data());
                        items_ret.push_back(itm);
                    }
#if 0
                    itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,mi->data());
                    itm.text="REVERSE_DRIVE";
                    itm.checkable=true;
                    itm.checked=types[1];
                    items_ret.push_back(itm);

                    itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,mi->data());
                    itm.text="STOP_INVALID";
                    itm.checkable=true;
                    itm.checked=types[2];
                    items_ret.push_back(itm);

                    itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,mi->data());
                    itm.text="NO_PEDESTRIANTION";
                    itm.checkable=true;
                    itm.checked=types[3];
                    items_ret.push_back(itm);

                    itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,mi->data());
                    itm.text="DRIVE_AWAY";
                    itm.checkable=true;
                    itm.checked=types[4];
                    items_ret.push_back(itm);

                    itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,mi->data());
                    itm.text="CONGESTION";
                    itm.checkable=true;
                    itm.checked=types[5];
                    items_ret.push_back(itm);

                    itm.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,mi->data());
                    itm.text="AbANDON_OBJECT";
                    itm.checkable=true;
                    itm.checked=types[6];
                    items_ret.push_back(itm);
#endif

                }else{
                    MvdProcessorInputData d1(mi->data());
                    d1.add_lane();
                    right_press_menu_item itm1;
                    itm1.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,d1.data());
                    itm1.text="add lane";
                    items_ret.push_back(itm1);

                    MvdProcessorInputData d2(mi->data());
                    d2.del_lane((mi->point_index-1)/12+1);
                    prt(info,"deling lane %d",(mi->point_index-1)/12+1);
                    \
                    right_press_menu_item itm2;
                    itm2.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,d2.data());
                    itm2.text="del lane";
                    items_ret.push_back(itm2);


                    MvdProcessorInputData d3(mi->data());
                    d3.add_event_rect();
                    right_press_menu_item itm3;
                    itm3.pkt=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,d3.data());
                    itm3.text="add event";
                    items_ret.push_back(itm3);
                }
                return ret;
            }
        }
        return false;
    }
#if 0
    bool right_press(VdPoint pnt,vector<RequestPkt> &pkts,vector<string> &text)
    {
        RequestPkt p;
        if((point_index=p_on_v(ExpectedAreaVers,pnt))){
            text.push_back("change to mvd");
            text.push_back("change to dummy");
            p=get_request(DetectRegionInputData::CHANGE_PROCESSOR,0,
                          get_region_test_data(MvdProcessorInputData::get_mvd_test_data().data(),LABEL_PROCESSOR_MVD).data());
            pkts.push_back(p);
            p=get_request(DetectRegionInputData::CHANGE_PROCESSOR,0,
                          get_region_test_data(DummyProcessorInputData::get_dummy_test_data().data(),LABEL_PROCESSOR_DUMMY).data());
            pkts.push_back(p);
            return true;
        }
        if(p_on_vl(ExpectedAreaVers,pnt)){

            return true;
        }

        if(SelectedProcessor== LABEL_PROCESSOR_MVD)
        {
            VdRect r=reshape_2_rect(ExpectedAreaVers);
            // prt(info,"offset %d %d",r.x,r.y);
            VdPoint pnt_new(pnt.x-r.x,pnt.y-r.y);


            MvdProcessorInputData *mi=(MvdProcessorInputData *)p_processor;
            bool ret=mi->right_press(pnt_new,pkts,text);
            if(ret){

                if(mi->point_index>(mi->LaneData.size()*12)){
                    MvdProcessorInputData d4(mi->data());
                    int idx=(mi->point_index-12*mi->LaneData.size()-1)/4+1;
                    d4.del_event_rect(idx);
                    prt(info,"deling event ");
                    p=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,d4.data());
                    pkts.push_back(p);
                    text.push_back("del event");
                }else{
                    MvdProcessorInputData d1(mi->data());
                    d1.add_lane();
                    p=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,d1.data());
                    pkts.push_back(p);
                    text.push_back("add lane");

                    MvdProcessorInputData d2(mi->data());
                    d2.del_lane((mi->point_index-1)/12+1);
                    prt(info,"deling lane %d",(mi->point_index-1)/12+1);
                    p=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,d2.data());
                    pkts.push_back(p);
                    text.push_back("del lane");


                    MvdProcessorInputData d3(mi->data());
                    d3.add_event_rect();
                    p=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,d3.data());
                    pkts.push_back(p);
                    text.push_back("add event");
                }
                return ret;
            }
        }
        return false;
    }
#endif
    void change_processor_2_dummy()
    {
        prt(info,"change processor to dummy");
    }
    void change_processor_2_mvd()
    {

    }

    template <typename A>
    bool right_press(VdPoint pnt,A exec_menu)
    {
        if((point_index=p_on_v(ExpectedAreaVers,pnt))){
            return true;
        }
        if(p_on_vl(ExpectedAreaVers,pnt)){

            return true;
        }
        return true;
    }
    virtual bool move(VdPoint pnt_ori)
    {
        if(seizing){
            switch (event_type) {
            case Event::MoveVer:
                ExpectedAreaVers[point_index-1]=pnt_ori;
                break;
            case Event::MoveAll:
            {
                int offx=pnt_ori.x-ori_pnt.x;
                int offy=pnt_ori.y-ori_pnt.y;
                ori_pnt=pnt_ori;
                int i=0;
                int sz=ExpectedAreaVers.size();
                for(i=0;i<sz;i++){
                    ExpectedAreaVers[i]=VdPoint(ExpectedAreaVers[i].x+offx,ExpectedAreaVers[i].y+offy);
                }
                break;
            }
            default:
                break;
            }
            encode();
            return true;

        }

        if(SelectedProcessor== LABEL_PROCESSOR_MVD)
        {
            VdRect r=reshape_2_rect(ExpectedAreaVers);
            //prt(info,"offset %d %d",r.x,r.y);
            VdPoint pnt(pnt_ori.x-r.x,pnt_ori.y-r.y);
            MvdProcessorInputData *mi=(MvdProcessorInputData *)p_processor;
            bool ret=mi->move(pnt);
            ProcessorData=mi->data();
            encode();
            if(ret)
                return ret;
        }
        return true;
    }
    virtual bool double_click(VdPoint pnt)
    {
        return true;
    }
    virtual void release()
    {
        if(seizing){
            AreaVersJsonDataRequest req(ExpectedAreaVers);
            DetectRegionInputData::get_request_pkt(DetectRegionInputData::CHANGE_RECT,0,req.data());
            release_event();
        }
    }
    bool release(RequestPkt &req)
    {
        if(seizing){
            AreaVersJsonDataRequest req1(ExpectedAreaVers);
            req=DetectRegionInputData::get_request_pkt(DetectRegionInputData::CHANGE_RECT,0,req1.data());
            release_event();
            return true;
        }

        if(SelectedProcessor== LABEL_PROCESSOR_MVD)
        {
            MvdProcessorInputData *mi=(MvdProcessorInputData *)p_processor;
            bool ret=mi->release(req);
            if(ret){
                req=get_request(DetectRegionInputData::MODIFY_PROCESSOR,0,mi->data());
                return true;
            }

            //      MvdProcessorInputData mid(ProcessorData,paintable_data);

            //    mid.release();
            //            draw_text(LABEL_PROCESSOR_MVD,VdPoint(100,200),100,PaintableData::Blue,30);
            //            VdRect r= reshape_2_rect(ExpectedAreaVers);
            //            MvdProcessorInputData data(ProcessorData);
            //            data.draw(r.x,r.y,draw_line, draw_circle,draw_text);
            //       MvdProcessorInputData data=Result;
            //     dat
            //            data.draw(DetectionRect.x,DetectionRect.y, draw_line,
            //                       draw_circle, draw_text);
        }
        return false;
    }

    template <typename A,typename B,typename C>
    void draw(
            A draw_line,
            B draw_circle,C draw_text)
    {
        for(int i=1;i<ExpectedAreaVers.size();i++){
            if(!seizing)
                draw_line(ExpectedAreaVers[i-1],ExpectedAreaVers[i],PaintableData::Colour::Blue,2);
            else
                draw_line(ExpectedAreaVers[i-1],ExpectedAreaVers[i],PaintableData::Colour::Red,2);

            draw_circle(ExpectedAreaVers[i-1],3,PaintableData::Colour::Red,2);

        }
        if(seizing){
            draw_text("detect region",ExpectedAreaVers[0],100,PaintableData::Blue,30);
        }
        draw_circle(ExpectedAreaVers.front(),3,PaintableData::Colour::Red,2);
        draw_circle(ExpectedAreaVers.back(),3,PaintableData::Colour::Red,2);
        if(!seizing)
            draw_line(ExpectedAreaVers.front(),ExpectedAreaVers.back(),PaintableData::Colour::Blue,2);
        else
            draw_line(ExpectedAreaVers.front(),ExpectedAreaVers.back(),PaintableData::Colour::Red,2);

        if(ClientConfig::show_camera_state){
            if(SelectedProcessor== LABEL_PROCESSOR_DUMMY)
            {
                draw_text(LABEL_PROCESSOR_DUMMY,VdPoint(100,200),100,PaintableData::Blue,30);
            }
            if(SelectedProcessor== LABEL_PROCESSOR_MVD)
            {
                draw_text(LABEL_PROCESSOR_MVD,VdPoint(100,200),100,PaintableData::Blue,30);
            }
        }
        if(SelectedProcessor== LABEL_PROCESSOR_DUMMY)
        {

        }
        if(SelectedProcessor== LABEL_PROCESSOR_MVD)
        {
            VdRect r= reshape_2_rect(ExpectedAreaVers);
//            MvdProcessorInputData data(ProcessorData);
//            data.draw(r.x,r.y,draw_line, draw_circle,draw_text);


             MvdProcessorInputData *mi=(MvdProcessorInputData *)p_processor;
             mi->draw(r.x,r.y,draw_line, draw_circle,draw_text);

        }


    }

};
class ProcessorDataJsonDataRequest:public JsonData{
public:
    string SelectedProcessor;
    JsonPacket ProcessorData;
    ProcessorDataJsonDataRequest(JsonPacket pkt):JsonData(pkt)
    {
        decode();
    }
    ProcessorDataJsonDataRequest(string pro,JsonPacket d)
    {
        ProcessorData=d;
        SelectedProcessor=pro;
        encode();
    }
    void decode()
    {
        try{
            DECODE_STRING_MEM(SelectedProcessor);
            DECODE_PKT_MEM(ProcessorData);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }
    }
    void encode()
    {
        try{
            ENCODE_STRING_MEM(SelectedProcessor);
            ENCODE_PKT_MEM(ProcessorData);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }
    }
};
class DetectRegionOutputData:public JsonData
{
public:
    VdRect DetectionRect;
    JsonPacket Result;
    DetectRegionOutputData(JsonPacket pkt):JsonData(pkt)
    {
        decode();
    }

    DetectRegionOutputData(JsonPacket rst ,VdRect rct)
    {
        DetectionRect=rct;
        Result=rst;
        encode();
    }

    void decode()
    {
        try{
            DECODE_PKT_MEM(DetectionRect);
            DECODE_PKT_MEM(Result);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }
    }

    void encode()
    {
        try{
            ENCODE_JSONDATA_MEM(DetectionRect);
            ENCODE_PKT_MEM(Result);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }
    }
    template <typename A,typename B,typename C>
    void draw(
            A draw_line,
            B draw_circle,C draw_text,DetectRegionInputData input_data)
    {
        if(input_data.SelectedProcessor== LABEL_PROCESSOR_DUMMY)
        {
            DummyProcessorOutputData data=Result;
            data.draw(DetectionRect.x,DetectionRect.y, draw_line,
                      draw_circle, draw_text);
        }
        if(input_data.SelectedProcessor== LABEL_PROCESSOR_MVD)
        {
            MvdProcessorOutputData data=Result;
            MvdProcessorInputData i_data(input_data.ProcessorData);
            data.draw(i_data,DetectionRect.x,DetectionRect.y, draw_line,
                      draw_circle, draw_text);
        }

    }

};


#endif // DETECTREGION_DATA_H
