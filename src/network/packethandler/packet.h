/*
 ****************************************************************************
 * packet.h
 *
 * Created on: 2009-11-25
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
 * Last Modified on: 2010-6-10
 * Last Modified by: 贺辉
 ***************************************************************************
 */

#ifndef PACKET_H_
#define PACKET_H_

//#include <QObject>
#include <QVariant>
#include <QMetaType>
#include <QHostAddress>
#include <QDateTime>
#include <QMutex>

#include "../global_network.h"

#include "../networklib.h"


namespace HEHUI {

/*

 class Packet {

 public:
 Packet ();
 virtual ~Packet();

 private:

 unsigned char packetType;
 unsigned char protocolVersion;
 unsigned short int command;
 unsigned short int packetSerialNumber;

 unsigned char body[];

 unsigned char packetTail;

 };

 */

//////////////////////////////////////////////////////////////////////

//struct PacketInfoStruct{
//    Packet *packet;
//
//};

class NETWORK_LIB_API Packet
{

public:
    Packet();
    Packet(int socketID);

    Packet(const QString &peerAddress, quint16 peerPort, const QString &localAddress = "0.0.0.0", quint16 localPort = 0);
    Packet(const QHostAddress &peerAddress, quint16 peerPort, const QHostAddress &localAddress = QHostAddress::Any, quint16 localPort = 0);

    Packet(const Packet &packet);
    Packet & operator = (const Packet &packet);
    virtual ~Packet();

    static void registerMetaTypeStreamOperators();


    void resetPacket();
    virtual bool isValid();

    quint8 getPacketType() const;
    void setPacketType(quint8 packetType);

    //        static quint16 createSerialNumber();

    //	quint16 getPacketSerialNumber() const;
    //	void setPacketSerialNumber(quint16 packetSerialNumber);

    QByteArray  getPacketData() const;
    void setPacketData(const QByteArray &data);

    QHostAddress getPeerHostAddress() const;
    void setPeerHostAddress(const QHostAddress &peerHostAddress);

    quint16 getPeerHostPort() const;
    void setPeerHostPort(quint16 peerHostPort);

    QHostAddress getLocalHostAddress() const;
    void setLocalHostAddress(const QHostAddress &localHostAddress);

    quint16 getLocalHostPort() const;
    void setLocalHostPort(quint16 localHostPort);

    TransmissionProtocol getTransmissionProtocol() const;
    void setTransmissionProtocol(TransmissionProtocol transmissionProtocol);

    int getRemainingRetransmissionTimes() const;
    void setRemainingRetransmissionTimes(int remainingRetransmissionTimes);

    QDateTime getLastTransmissionTime() const;
    void setLastTransmissionTime(const QDateTime &time);

    SOCKETID getSocketID() const;
    void setSocketID(SOCKETID id);


    void packetTransmissionFailed();


private:


    quint8 packetType;
    //quint16 m_packetSerialNumber;


    QByteArray packetData;

    QHostAddress peerHostAddress;
    quint16 peerHostPort;

    QHostAddress localHostAddress;
    quint16 localHostPort;

    TransmissionProtocol transmissionProtocol;

    int remainingRetransmissionTimes;

    QDateTime lastTransmissionTime;

    SOCKETID socketID;




};

//class MYSHAREDLIB_API TCPPacket : public Packet
//{

//public:
//	TCPPacket();
//        TCPPacket(const QString &peerAddress, quint16 peerPort, const QString &localAddress = "", quint16 localPort = 0);
//        TCPPacket(const QHostAddress &peerAddress, quint16 peerPort, const QHostAddress &localAddress = QHostAddress::Null, quint16 localPort = 0);

//        virtual ~TCPPacket();

//private:

//};

//class MYSHAREDLIB_API UDPPacket : public Packet
//{

//public:
//	UDPPacket();
//        UDPPacket(const QString &peerAddress, quint16 peerPort, const QString &localAddress = "", quint16 localPort = 0);
//        UDPPacket(const QHostAddress &peerAddress, quint16 peerPort, const QHostAddress &localAddress = QHostAddress::Null, quint16 localPort = 0);

//        virtual ~UDPPacket();

//	//For UDP

//	UDPTransmissionMode getUDPTransmissionMode();
//	void setUDPTransmissionMode(UDPTransmissionMode udpTransmissionMode);



//private:


//	UDPTransmissionMode udpTransmissionMode;



//};










} //namespace HEHUI


//自定义类型，要在使用前注册该类型：qRegisterMetaTypeStreamOperators<HEHUI::Packet>("HEHUI::Packet");
Q_DECLARE_METATYPE(HEHUI::Packet)






/*
////////////////////////////////  重载操作符  //////////////////////////////////////////////////////

QDataStream &operator<<(QDataStream &out, const HEHUI::Packet &packet);
QDataStream &operator>>(QDataStream &in, HEHUI::Packet &packet);



//重载操作符<<
	QDataStream &operator<<(QDataStream &out, const HEHUI::Packet &packet){
    quint8 packetType = packet.getPacketType();
    quint16 packetSerialNumber = packet.getPacketSerialNumber();
    QByteArray packetData = packet.getPacketData();

    out << packetType;
    if(packetType != quint8(HEHUI::HeartbeatPacket)){
        out << packetSerialNumber << packetData;
    }

    return out;

}

//重载操作符>>
    QDataStream &operator>>(QDataStream &in, HEHUI::Packet &packet){
    quint8 packetType;
    quint16 packetSerialNumber;
    QByteArray packetData;

    in >> packetType;
    if(packetType != quint8(HEHUI::HeartbeatPacket)){
        in >> packetSerialNumber >> packetData;
    }

    packet.setPacketType(packetType);
    packet.setPacketSerialNumber(packetSerialNumber);
    packet.setPacketData(packetData);

    return in;

}

*/





#endif /* PACKET_H_ */

