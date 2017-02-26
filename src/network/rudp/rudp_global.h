/*
 * rudp_global.h
 *
 *  Created on: 2011-6-27
 *      Author: HeHui
 */

#ifndef RUDP_GLOBAL_H_
#define RUDP_GLOBAL_H_


//#include "../networklib.h"


#ifndef RUDP_VERSION
    #define RUDP_VERSION 1
#endif

#ifndef RUDP_PACKET_RETRANSMISSION_TIMES
    #define RUDP_PACKET_RETRANSMISSION_TIMES -1
#endif

#ifndef RUDP_ETHERNET_MSS
    #define RUDP_ETHERNET_MSS 1469 //Bytes 1472-1(Type)-2(SN)
#endif

#ifndef RUDP_INTERNET_MSS
    #define RUDP_INTERNET_MSS 545 //Bytes 548-1(Type)-2(SN)
#endif

#ifndef RUDP_KEEPALIVE_TIMER_INTERVAL
    #define RUDP_KEEPALIVE_TIMER_INTERVAL 60000 //1 minute
#endif

#ifndef RUDP_CONNECTION_TIMEOUT
    #define RUDP_CONNECTION_TIMEOUT 10000 //10 seconds
#endif
//#ifndef RUDP_CONNECTION_MAX_TIMEOUT
//#define RUDP_CONNECTION_MAX_TIMEOUT 30000 //30 seconds
//#endif

#ifndef RUDP_MAX_CHECK_ALIVE_TIMES
    #define RUDP_MAX_CHECK_ALIVE_TIMES 2
#endif

#ifndef RUDP_MAX_PACKET_SN
    #define RUDP_MAX_PACKET_SN 0XFFFF //65535
#endif

#ifndef RUDP_SLOWSTART_THRESHOLD
    #define RUDP_SLOWSTART_THRESHOLD 512
#endif

#ifndef RUDP_MIN_SEND_WINDOW_SIZE
    #define RUDP_MIN_SEND_WINDOW_SIZE 16
#endif
#ifndef RUDP_MAX_SEND_WINDOW_SIZE
    #define RUDP_MAX_SEND_WINDOW_SIZE 5000
#endif

#ifndef RUDP_DEFAULT_CHECK_TRANSMISSION_TIMER_INTERVAL
    #define RUDP_DEFAULT_CHECK_TRANSMISSION_TIMER_INTERVAL 300
#endif
#ifndef RUDP_MAX_CHECK_TRANSMISSION_TIMER_INTERVAL
    #define RUDP_MAX_CHECK_TRANSMISSION_TIMER_INTERVAL 1200000
#endif

#ifndef RUDP_MIN_SEND_ACK_TIMER_INTERVAL
    #define RUDP_MIN_SEND_ACK_TIMER_INTERVAL 10
#endif
#ifndef RUDP_MAX_SEND_ACK_TIMER_INTERVAL
    #define RUDP_MAX_SEND_ACK_TIMER_INTERVAL 10000
#endif

#ifndef RUDP_MIN_RETRANSMISSION_TIMER_INTERVAL
    #define RUDP_MIN_RETRANSMISSION_TIMER_INTERVAL 300
#endif

#ifndef MIN_THREAD_COUNT
    #define MIN_THREAD_COUNT 10
#endif

namespace HEHUI
{
namespace RUDP
{

enum RUDPPacketType {UnKnownPacket = 0, Handshake, Reset, Goodbye, ACK, ACK2, NACK, PacketDropped, KeepAlive, BeginOrEndDataTransmission, CompleteDataPacket, FragmentDataPacket, DataStreamPacket, UnreliableDataPacket, UserDefinedPacket};










} //namespace RUDP
} //namespace HEHUI

#endif /* RUDP_GLOBAL_H_ */
