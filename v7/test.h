#ifndef TEST_H
#define TEST_H
#include "jsonpacket.h"
#include "tool.h"
class DeviceObject:public JsonObject{
public:
    AppInputData DeviceConfig;
    DeviceObject(JsonPacket pkt):JsonObject(pkt)
    {
        decode();
    }
    DeviceObject(AppInputData appdata):DeviceConfig(appdata)
    {
        encode();
    }
    DeviceObject()
    {
        //decode();
    }
    void decode()
    {
        try{
            DECODE_JSONDATA_MEM(DeviceConfig);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }
    }
    void encode()
    {
        try{
            ENCODE_JSONDATA_MEM(DeviceConfig);
        }catch(exception e){
            PRT_DECODE_EXCEPTION
        }
    }
};

#endif // TEST_H

