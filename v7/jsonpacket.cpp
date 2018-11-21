#include "jsonpacket.h"
JsonPacket JsonObject::get_request_pkt(int op, int index, JsonPacket data)
{
    return RequestPkt(op,index,data).data();
}
