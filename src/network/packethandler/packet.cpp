/*
 ****************************************************************************
 * packet.cpp
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
 * Last Modified on: 2010-07-13
 * Last Modified by: 贺辉
 ***************************************************************************
 */


#include "packet.h"

#include "packetstreamoperator.h"

namespace HEHUI {

/*

 Packet::Packet(QObject *parent)
 :QObject(parent)
 {
 packetType = 0X00;
 protocolVersion = PACKET_VERSION;
 command = 0;
 packetSerialNumber = 0;

 body = '';

 packetTail = PACKET_TAIL_TAG;

 }


 Packet::~Packet() {
 // TODO Auto-generated destructor stub
 }

 */

//////////////////////////////////////////////////////////////////////////////////////////

//quint16 Packet::PacketSerialNumber = 0;

Packet::Packet() {

    //	packetHeader.resize(0);
    //	packetBody.resize(0);
    //	packetTail.resize(0);
    
    this->packetType = HEHUI::UnKnownPacket;
    //this->m_packetSerialNumber = 0;
    this->packetData = QByteArray();
    this->packetData.resize(0);
    
    this->peerHostAddress = QHostAddress::Null;;
    this->peerHostPort = 0;
    this->localHostAddress = QHostAddress::Null;;
    this->localHostPort = 0;
    
    this->transmissionProtocol = TP_UNKNOWN;
    
    this->remainingRetransmissionTimes = 10;
    this->lastTransmissionTime = QDateTime();

    this->socketID = -1;

}

Packet::Packet(int socketID){

    this->packetType = UnKnownPacket;
    //this->m_packetSerialNumber = 0;
    this->packetData = QByteArray();
    this->packetData.resize(0);

    this->peerHostAddress = QHostAddress::Null;;
    this->peerHostPort = 0;
    this->localHostAddress = QHostAddress::Null;;
    this->localHostPort = 0;

    this->transmissionProtocol = TP_UNKNOWN;

    //remainingRetransmissionTimes = int(PACKET_RETRANSMISSION_TIMES);
    this->remainingRetransmissionTimes = 10;
    this->lastTransmissionTime = QDateTime();


    this->socketID = socketID;
}


Packet::Packet(const QString &peerAddress, quint16 peerPort, const QString &localAddress, quint16 localPort)
//	:QObject(parent)
{

    //	packetHeader.resize(0);
    //	packetBody.resize(0);
    //	packetTail.resize(0);
    
    this->packetType = UnKnownPacket;
    //this->m_packetSerialNumber = 0;
    this->packetData = QByteArray();
    this->packetData.resize(0);
    
    this->peerHostAddress = QHostAddress(peerAddress);
    this->peerHostPort = peerPort;
    this->localHostAddress = QHostAddress(localAddress);
    this->localHostPort = localPort;
    
    this->transmissionProtocol = TP_UNKNOWN;
    
    //remainingRetransmissionTimes = int(PACKET_RETRANSMISSION_TIMES);
    this->remainingRetransmissionTimes = 10;
    this->lastTransmissionTime = QDateTime();

    this->socketID = -1;

}

Packet::Packet(const QHostAddress &peerAddress, quint16 peerPort, const QHostAddress &localAddress, quint16 localPort)
//	:QObject(parent)
{

    //	packetHeader.resize(0);
    //	packetBody.resize(0);
    //	packetTail.resize(0);
    
    this->packetType = UnKnownPacket;
    //this->m_packetSerialNumber = 0;
    this->packetData = QByteArray();
    this->packetData.resize(0);
    
    this->peerHostAddress = peerAddress;
    this->peerHostPort = peerPort;
    this->localHostAddress = localAddress;
    this->localHostPort = localPort;
    
    this->transmissionProtocol = TP_UNKNOWN;
    
    //remainingRetransmissionTimes = int(PACKET_RETRANSMISSION_TIMES);
    this->remainingRetransmissionTimes = 10;
    this->lastTransmissionTime = QDateTime();

    this->socketID = -1;

}

Packet::Packet(const Packet &packet){
    *this = packet;
 }

Packet & Packet::operator = (const Packet &packet){

    this->packetType = packet.getPacketType();
    //this->m_packetSerialNumber = packet.getPacketSerialNumber();
    this->packetData = packet.getPacketData();
    this->peerHostAddress = packet.getPeerHostAddress();
    this->peerHostPort = packet.getPeerHostPort();
    this->localHostAddress = packet.getLocalHostAddress();
    this->localHostPort = packet.getLocalHostPort();
    this->transmissionProtocol = packet.getTransmissionProtocol();
    this->remainingRetransmissionTimes = packet.getRemainingRetransmissionTimes();
    this->lastTransmissionTime = packet.getLastTransmissionTime();

    this->socketID = packet.getSocketID();

    return *this;
}

Packet::~Packet() {


    
}

void Packet::registerMetaTypeStreamOperators(){

    //注册自定义类型，必须重载“<<”和“>>”, 见"packetstreamoperator.h"

    int type = QMetaType::type("HEHUI::Packet");
    if(!type){
        qRegisterMetaTypeStreamOperators<HEHUI::Packet>("HEHUI::Packet");
    }else if(!QMetaType::isRegistered(type)){
        qRegisterMetaTypeStreamOperators<HEHUI::Packet>("HEHUI::Packet");
    }

}

void Packet::resetPacket(){
    this->packetType = UnKnownPacket;
    //this->m_packetSerialNumber = 0;
    this->packetData.resize(0);
    
    this->peerHostAddress = QHostAddress::Null;
    this->peerHostPort = 0;
    this->localHostAddress = QHostAddress::Null;
    this->localHostPort = 0;
    this->transmissionProtocol = TP_UNKNOWN;
    this->remainingRetransmissionTimes = 10;
    this->lastTransmissionTime = QDateTime();

    this->socketID = -1;
    
}

bool Packet::isValid() {
        if ( (peerHostAddress.isNull() || peerHostPort < 1) && (socketID == 0) ) {
		return false;
	}

	return true;

}

quint8 Packet::getPacketType() const {
	return packetType;
}

void Packet::setPacketType(quint8 packetType) {
	this->packetType = packetType;
}

//quint16 Packet::createSerialNumber() {
//	static QMutex serialNumberMutex;
//	QMutexLocker locker(&serialNumberMutex);

//        static quint16 sn = 0;
//        if (sn == quint16(0XFFFF)) {
//            sn = 1;
//        }else{
//            sn++;
//        }

//        return sn;

//}

//quint16 Packet::getPacketSerialNumber() const {
//        return m_packetSerialNumber;
//}

//void Packet::setPacketSerialNumber(quint16 packetSerialNumber) {
//        this->m_packetSerialNumber = packetSerialNumber;
//}

QByteArray Packet::getPacketData() const {
	//    QMutexLocker locker(&mutex);
        return packetData;
}

void Packet::setPacketData(const QByteArray &data) {
	//    QMutexLocker locker(&mutex);
	this->packetData = data;
}

/*

 QByteArray Packet::getPacketHeader() const
 {
 return packetHeader;
 }

 void Packet::setPacketHeader(const QByteArray &header)
 {
 this->packetHeader = header;
 }

 QByteArray Packet::getPacketBody() const
 {
 return packetBody;
 }

 void Packet::setPacketBody(const QByteArray &body)
 {
 this->packetBody = body;
 }

 quint8 Packet::getPacketTail() const
 {
 return packetTail;
 }

 void Packet::setPacketTail(quint8 packetTail)
 {
 this->packetTail = packetTail;
 }

 */

QHostAddress Packet::getPeerHostAddress() const {
	//    QMutexLocker locker(&mutex);
        return peerHostAddress;
}

void Packet::setPeerHostAddress(const QHostAddress &peerHostAddress) {
	//    QMutexLocker locker(&mutex);
        this->peerHostAddress = peerHostAddress;
}

quint16 Packet::getPeerHostPort() const {
	//    QMutexLocker locker(&mutex);
        return peerHostPort;
}

void Packet::setPeerHostPort(quint16 peerHostPort) {
	//    QMutexLocker locker(&mutex);
        this->peerHostPort = peerHostPort;
}

QHostAddress Packet::getLocalHostAddress() const {
        return localHostAddress;
}

void Packet::setLocalHostAddress(const QHostAddress &localHostAddress) {
        this->localHostAddress = localHostAddress;
}

quint16 Packet::getLocalHostPort() const {
        return localHostPort;
}

void Packet::setLocalHostPort(quint16 localHostPort) {
        this->localHostPort = localHostPort;
}

TransmissionProtocol Packet::getTransmissionProtocol() const {
	//    QMutexLocker locker(&mutex);
	return this->transmissionProtocol;
}

void Packet::setTransmissionProtocol(TransmissionProtocol transmissionProtocol) {
	//    QMutexLocker locker(&mutex);
	this->transmissionProtocol = transmissionProtocol;
}

int Packet::getRemainingRetransmissionTimes() const{
	//    QMutexLocker locker(&mutex);
	return this->remainingRetransmissionTimes;
}

void Packet::setRemainingRetransmissionTimes(int remainingRetransmissionTimes) {
	//    QMutexLocker locker(&mutex);
	this->remainingRetransmissionTimes = remainingRetransmissionTimes;
}

QDateTime Packet::getLastTransmissionTime() const {
        //    QMutexLocker locker(&mutex);
        return this->lastTransmissionTime;
}

void Packet::setLastTransmissionTime(const QDateTime &time) {
        //    QMutexLocker locker(&mutex);
        this->lastTransmissionTime = time;

}

int Packet::getSocketID() const{
    return socketID;
}

void Packet::setSocketID(int id){
    this->socketID = id;
}

void Packet::packetTransmissionFailed() {
	//    QMutexLocker locker(&mutex);
//	if ((--remainingRetransmissionTimes) < 0) {
//		//		emit signalPacketTransmissionFailed();
//	}

    remainingRetransmissionTimes--;

}

//////////////////////////////////////////////////////////////////////////////////
//TCPPacket::TCPPacket()
//    :Packet()
//{

//    setPacketSerialNumber(createSerialNumber());
    
//    setTransmissionProtocol( TP_TCP);

//}

//TCPPacket::TCPPacket(const QString &peerAddress, quint16 peerPort, const QString &localAddress, quint16 localPort)
//    :Packet(peerAddress, peerPort, localAddress, localPort)
//{

//    setPacketSerialNumber(createSerialNumber());
    
//    setTransmissionProtocol( TP_TCP);

//}

//TCPPacket::TCPPacket(const QHostAddress &peerAddress, quint16 peerPort, const QHostAddress &localAddress, quint16 localPort)
//    :Packet(peerAddress, peerPort, localAddress, localPort)
//{

//    setPacketSerialNumber(createSerialNumber());
    
//    setTransmissionProtocol( TP_TCP);

//}

//TCPPacket::~TCPPacket() {

//}

//////////////////////////////////////////////////////////////////////////////////////

//UDPPacket::UDPPacket()
//    :Packet()
//{


//    setPacketSerialNumber(createSerialNumber());
    
//    this->udpTransmissionMode = UDP_TM_DIRECT;
    
//    setTransmissionProtocol( TP_UDP);
    

//}

//UDPPacket::UDPPacket(const QString &peerAddress, quint16 peerPort, const QString &localAddress, quint16 localPort)
//    :Packet(peerAddress, peerPort, localAddress, localPort)
//{

//    setPacketSerialNumber(createSerialNumber());
    
//    this->udpTransmissionMode = UDP_TM_DIRECT;
    
//    setTransmissionProtocol( TP_UDP);
    
//}

//UDPPacket::UDPPacket(const QHostAddress &peerAddress, quint16 peerPort, const QHostAddress &localAddress, quint16 localPort)
//    :Packet(peerAddress, peerPort, localAddress, localPort)
//{

//    setPacketSerialNumber(createSerialNumber());
    
//    this->udpTransmissionMode = UDP_TM_DIRECT;
    
//    setTransmissionProtocol( TP_UDP);

    
//}

//UDPPacket::~UDPPacket() {

//}



//UDPTransmissionMode UDPPacket::getUDPTransmissionMode() {
//	return udpTransmissionMode;
//}

//void UDPPacket::setUDPTransmissionMode(UDPTransmissionMode udpTransmissionMode) {
//	this->udpTransmissionMode = udpTransmissionMode;
//}
















/*
////////////////////////////////  重载操作符  //////////////////////////////////////////////////////

QDataStream &operator<<(QDataStream &out, const HEHUI::Packet &packet);
QDataStream &operator>>(QDataStream &in, HEHUI::Packet &packet);



//重载操作符“<<”
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

//重载操作符“>>”
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














} //namespace HEHUI
