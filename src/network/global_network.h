/*
 ****************************************************************************
 * global_network.h
 *
 * Created on: 2009-4-27
 *     Author: 贺辉
 *    License: LGPL
 *    Comment:
 *
 *
 *    =============================  Usage  =============================
 *|
 *|
 *    ===================================================================
 *
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 ****************************************************************************
 */

/*
 ***************************************************************************
 * Last Modified on: 2010-05-15
 * Last Modified by: 贺辉
 ***************************************************************************
 */




#ifndef GLOBAL_NETWORK_H_
#define GLOBAL_NETWORK_H_

//#include "networklib.h"

//#ifdef Q_OS_WIN
//#define OS_IS_WINDOWS 1
//#else
//#define OS_IS_WINDOWS 0
//#endif

#ifndef MAX_ETHERNET_UDP_DATAGRAM_SIZE
#define MAX_ETHERNET_UDP_DATAGRAM_SIZE 1472 //Bytes
#endif

#ifndef MAX_INTERNET_UDP_DATAGRAM_SIZE
#define MAX_INTERNET_UDP_DATAGRAM_SIZE 548 //Bytes
#endif

#ifndef PACKET_TERMINATING_SYMBOL
#define PACKET_TERMINATING_SYMBOL 0xFFFF
#endif

#ifndef GROUP_SEPARTOR
#define GROUP_SEPARTOR 0X1D
#endif

#ifndef PACKET_DATA_SEPARTOR
#define PACKET_DATA_SEPARTOR 0X1E
#endif

#ifndef UNIT_SEPARTOR
#define UNIT_SEPARTOR 0X1F
#endif

//#ifndef MAX_PACKET_SEQUENCE_NUMBER
//#define MAX_PACKET_SEQUENCE_NUMBER 0XFFFF
//#endif

#ifndef PACKET_RETRANSMISSION_TIMES
#define PACKET_RETRANSMISSION_TIMES 5
#endif

//#ifndef ONLINE_STATUS_CHECKING_TIMES
//#define ONLINE_STATUS_CHECKING_TIMES 3
//#endif

#ifndef HEARTBEAT_TIMER_INTERVAL
#define HEARTBEAT_TIMER_INTERVAL 60000 //1 minute
#endif

#ifndef UDP_PACKET_WAITING_FOR_REPLY_TIMEOUT
#define UDP_PACKET_WAITING_FOR_REPLY_TIMEOUT 10000 //10 seconds
#endif


#ifndef TCP_CONNECTION_TIMEOUT
#define TCP_CONNECTION_TIMEOUT 5000
#endif

namespace HEHUI {


enum TransmissionProtocol {TP_UNKNOWN = 0, TP_TCP, TP_UDP, TP_UDT, TP_RUDP};

enum UDPTransmissionMode {UDP_TM_MULTICAST = 1, UDP_TM_BROADCAST = 2, UDP_TM_DIRECT = 3};

enum PacketType{UnKnownPacket = 0, /*HeartbeatPacket = 1, ConfirmationOfReceiptPacket = 2,*/ TextPacket = 3, BinPacket = 4, UserDefinedPacket = 5};




}/* namespace */


#endif /* GLOBAL_NETWORK_H_ */
