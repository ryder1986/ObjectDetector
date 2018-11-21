#ifndef TEST_H
#define TEST_H
#include "jsonpacket.h"
#include "tool.h"
#include "opencv2/core.hpp"
#include "videosource.h"
class DummyProcessorObjectOutJsonData:public JsonObject{

public:
    vector<VdPoint> Points;
    int Radii;
    DummyProcessorObjectOutJsonData()
    {
    }
    DummyProcessorObjectOutJsonData(JsonPacket str):JsonObject(str)
    {
        decode();
    }
    DummyProcessorObjectOutJsonData(vector<VdPoint> o,int r):Radii(r)
    {
        Points.assign(o.begin(),o.end());
        encode();
    }
    void decode()
    {
        DECODE_JSONDATA_ARRAY_MEM(Points);
        DECODE_INT_MEM(Radii);
    }
    void encode()
    {
        ENCODE_JSONDATA_ARRAY_MEM(Points);
        ENCODE_INT_MEM(Radii);
    }
    template <typename A,typename B,typename C>
    void draw(int offx,int offy,
              A draw_line,
              B draw_circle,C draw_text)
    {
        for(VdPoint p:Points){
            draw_circle(VdPoint(p.x+offx,p.y+offy),Radii,PaintableData::Colour::Red,2);
        }
    }

};
class DummyProcessorObject:public JsonObject{
public:
    bool Horizon;
    bool Vertical;
    int Radii;
    DummyProcessorObject(JsonPacket pkt):JsonObject(pkt)
    {
        decode();
    }
    DummyProcessorObject()
    {

    }
    void decode()
    {
        DECODE_BOOL_MEM(Horizon);
        DECODE_BOOL_MEM(Vertical);
        DECODE_INT_MEM(Radii);

    }
    void encode()
    {
        ENCODE_BOOL_MEM(Horizon);
        ENCODE_BOOL_MEM(Vertical);
        ENCODE_INT_MEM(Radii);
    }


    int loopx;
    int loopy;
    virtual bool process(cv::Mat img_src,JsonPacket &output_pkt)
    {
        this_thread::sleep_for(chrono::milliseconds(1000));
        bool ret=false;
        loopx+=30;
        if(loopx>=img_src.cols)
            loopx=0;

        loopy+=30;
        if(loopy>=img_src.rows)
            loopy=0;
        vector<VdPoint> ps;
        if(Horizon){
            VdPoint p(loopx,10);
            ps.push_back(p);
        }
        if(Vertical){
            VdPoint p(10,loopy);
            ps.push_back(p);
        }
        DummyProcessorObjectOutJsonData d(ps,Radii);
        output_pkt=d.data();
        ret=true;
        return  ret;
    }

    void start()
    {

    }
};
class RegionObject:public JsonObject{
public:

    vector <VdPoint>ExpectedAreaVers;
    JsonPacket ProcessorData;
    string SelectedProcessor;
    RegionObject(JsonPacket pkt):JsonObject(pkt)
    {
        decode();
    }
    RegionObject()
    {

    }
    void decode()
    {
        DECODE_PKT_MEM(ProcessorData);
        DECODE_STRING_MEM(SelectedProcessor);
        DECODE_JSONDATA_ARRAY_MEM(ExpectedAreaVers);
    }
    void encode()
    {
        ENCODE_PKT_MEM(ProcessorData);
        ENCODE_STRING_MEM(SelectedProcessor);
        ENCODE_JSONDATA_ARRAY_MEM(ExpectedAreaVers);
    }

    void start()
    {

    }
};
class CameraInputData:public JsonObject{
public:
    vector <RegionObject>DetectRegion;
    string Url;

    CameraInputData(JsonPacket pkt):JsonObject(pkt)
    {
        decode();
    }

    CameraInputData()
    {

    }

    void decode()
    {
        DECODE_JSONDATA_ARRAY_MEM(DetectRegion);
        DECODE_STRING_MEM(Url);
    }

    void encode()
    {
        ENCODE_JSONDATA_ARRAY_MEM(DetectRegion);
        ENCODE_STRING_MEM(Url);
    }

};
class CameraManagerInputData:public JsonObject{
public:
    vector <CameraObject> CameraData;
    CameraManagerInputData(JsonPacket pkt):JsonObject(pkt)
    {
        decode();
    }
    CameraManagerInputData()
    {
    }
    void decode()
    {
        DECODE_JSONDATA_ARRAY_MEM(CameraData);
    }
    void encode()
    {
        ENCODE_JSONDATA_ARRAY_MEM(CameraData);
    }

};

class DeviceInputData:public JsonObject{
public:
    CameraManagerInputData DeviceConfig;
    DeviceInputData(JsonPacket pkt):JsonObject(pkt)
    {
        decode();
    }
    DeviceInputData()
    {
    }
    void decode()
    {
        DECODE_JSONDATA_MEM(DeviceConfig);
    }
    void encode()
    {
        ENCODE_JSONDATA_MEM(DeviceConfig);
    }

};

#endif // TEST_H

