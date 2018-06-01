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


#include "packetbase.h"

#include <QUuid>


//#include "packetstreamoperator.h"

namespace HEHUI
{

//////////////////////////////////////////////////////////////////////////////////////////
//重载操作符“<<”
//Write data to stream
QDataStream &operator<<(QDataStream &out, const HEHUI::PacketBase &packet)
{
    quint8 packetType = packet.getPacketType();
    QByteArray packetData = packet.getPacketBody();

    out << packetType << packetData;

    return out;
}

//重载操作符“>>”
//Read data from stream
QDataStream &operator>>(QDataStream &in, HEHUI::PacketBase &packet)
{
    quint8 packetType = 0;
    QByteArray packetData;

    in >> packetType >> packetData;

    packet.setPacketType(packetType);
    packet.setPacketBody(packetData);

    return in;
}
//////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////
//quint16 Packet::PacketSerialNumber = 0;


QString PacketBase::m_localID = QUuid::createUuid().toString();

PacketBase::PacketBase()
{

    //	packetHeader.resize(0);
    //	packetBody.resize(0);
    //	packetTail.resize(0);

    this->m_packetType = HEHUI::UnKnownPacket;
    this->m_packetSubType = HEHUI::UserDefinedPacket;
    this->m_packetBody = QByteArray();
    this->m_packetBody.resize(0);

    this->peerHostAddress = QHostAddress::Null;;
    this->peerHostPort = 0;

    this->m_socketID = 0;
    this->m_peerID = "";


}

PacketBase::PacketBase(quint8 packetType, quint8 packetSubType)
{

    this->m_packetType = packetType;
    this->m_packetSubType = packetSubType;
    this->m_packetBody = QByteArray();
    this->m_packetBody.resize(0);

    this->peerHostAddress = QHostAddress::Null;;
    this->peerHostPort = 0;

    this->m_socketID = 0;
    this->m_peerID = "";

}


PacketBase::PacketBase(const PacketBase &packet)
{
    *this = packet;
}

PacketBase &PacketBase::operator = (const PacketBase &packet)
{

    this->m_packetType = packet.m_packetType;
    this->m_packetSubType = packet.m_packetSubType;
    this->m_packetBody = packet.m_packetBody;
    this->peerHostAddress = packet.peerHostAddress;
    this->peerHostPort = packet.peerHostPort;

    this->m_socketID = packet.m_socketID;
    this->m_peerID = packet.m_peerID;

    return *this;
}

PacketBase::~PacketBase()
{
    resetPacket();
}

void PacketBase::registerMetaTypeStreamOperators()
{

    //注册自定义类型，必须重载“<<”和“>>”, 见"packetstreamoperator.h"

    int type = QMetaType::type("HEHUI::PacketBase");
    if(!type) {
        qRegisterMetaTypeStreamOperators<HEHUI::PacketBase>("HEHUI::PacketBase");
    } else if(!QMetaType::isRegistered(type)) {
        qRegisterMetaTypeStreamOperators<HEHUI::PacketBase>("HEHUI::PacketBase");
    }

}

void PacketBase::resetPacket()
{
    this->m_packetType = UnKnownPacket;
    this->m_packetSubType = HEHUI::UserDefinedPacket;

    //this->m_packetSerialNumber = 0;
    this->m_packetBody.clear();
    this->m_packetBody.resize(0);

    this->peerHostAddress = QHostAddress::Null;
    this->peerHostPort = 0;

    this->m_socketID = 0;
    this->m_peerID = "";
}

bool PacketBase::isValid()
{
    return (UnKnownPacket != m_packetType) && (!m_packetBody.isEmpty());
}

QByteArray PacketBase::toByteArray() const{

    if(m_packetBody.isEmpty()){return QByteArray();}

    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_8);
    out << m_packetType << m_packetSubType << m_localID << m_packetBody;

    return ba;
}

bool PacketBase::fromByteArray(QByteArray *data)
{
    if(!data) {
        return false;
    };

    this->m_packetType = UnKnownPacket;
    this->m_peerID = "";
    this->m_packetBody.clear();
    this->m_packetBody.resize(0);

    QDataStream in(data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_8);
    in >> m_packetType >> m_packetSubType >> m_peerID >> m_packetBody;

    return (UnKnownPacket != m_packetType) && (!m_packetBody.isEmpty());
}


quint8 PacketBase::getPacketType() const
{
    return m_packetType;
}

void PacketBase::setPacketType(quint8 type)
{
    this->m_packetType = type;
}

quint8 PacketBase::getPacketSubType() const
{
    return m_packetSubType;
}

void PacketBase::setPacketSubType(quint8 type)
{
    m_packetSubType = type;
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

QHostAddress PacketBase::getPeerHostAddress() const
{
    //    QMutexLocker locker(&mutex);
    return peerHostAddress;
}

void PacketBase::setPeerHostAddress(const QHostAddress &peerHostAddress)
{
    //    QMutexLocker locker(&mutex);
    this->peerHostAddress = peerHostAddress;
}

quint16 PacketBase::getPeerHostPort() const
{
    //    QMutexLocker locker(&mutex);
    return peerHostPort;
}

void PacketBase::setPeerHostPort(quint16 peerHostPort)
{
    //    QMutexLocker locker(&mutex);
    this->peerHostPort = peerHostPort;
}

SOCKETID PacketBase::getSocketID() const
{
    return m_socketID;
}

void PacketBase::setSocketID(SOCKETID id)
{
    this->m_socketID = id;
}

QString PacketBase::getPeerID() const
{
    return m_peerID;
}

void PacketBase::setPeerID(const QString &id)
{
    this->m_peerID = id;
}

const QString PacketBase::getLocalID()
{
    return m_localID;
}

void PacketBase::setLocalID(const QString &id)
{
    m_localID = id;
}

QByteArray PacketBase::getPacketBody() const
{
    //    QMutexLocker locker(&mutex);
    return m_packetBody;
}

void PacketBase::setPacketBody(const QByteArray &data)
{
    //    QMutexLocker locker(&mutex);
    this->m_packetBody = data;
}

void PacketBase::clearPacketBody()
{
    m_packetBody.clear();
    m_packetBody.resize(0);
}



//////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////

Packet::Packet()
    : PacketBase()
{
    m_encrypted = false;
}

Packet::Packet(quint8 packetType, quint8 packetSubType)
    : PacketBase(packetType, packetSubType)
{
    m_encrypted = false;
}

Packet::Packet(const PacketBase &base)
    : PacketBase()
{
    convert(base);
}

Packet &Packet::operator = (const PacketBase &base)
{
    convert(base);
    return *this;
}

Packet::~Packet()
{

}

bool Packet::isEncrypted() const
{
    return m_encrypted;
}

void Packet::setEncrypted(bool encrypted)
{
    m_encrypted = encrypted;
}

QByteArray Packet::toByteArray()
{
    QByteArray body;
    if(m_encrypted){
        body = encrypt(packBodyData());
    }else{
        body = packBodyData();
    }

    if(body.isEmpty()) {
        return QByteArray();
    }

    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_8);

    out << getPacketType() << getPacketSubType() << getLocalID() << body;

    return ba;
}

//void Packet::parsePacketBody()
//{
//    QByteArray packetBody;
//    if(m_encrypted){
//        packetBody = decrypt(getPacketBody());
//    }else{
//        packetBody = getPacketBody();
//    }

//    parsePacketBody(packetBody);
//}

void Packet::fromPacket(const PacketBase &base)
{
    setPacketType(base.getPacketType());
    setPacketSubType(base.getPacketSubType());
    setPeerHostAddress(base.getPeerHostAddress());
    setPeerHostPort(base.getPeerHostPort());

    setSocketID(base.getSocketID());
    setPeerID(base.getPeerID());

    QByteArray packetBody;
    if(m_encrypted){
        packetBody = decrypt(base.getPacketBody());
    }else{
        packetBody = base.getPacketBody();
    }

    parsePacketBody(packetBody);
}

void Packet::convert(const PacketBase &base)
{

    setPacketType(base.getPacketType());
    setPacketSubType(base.getPacketSubType());
    setPeerHostAddress(base.getPeerHostAddress());
    setPeerHostPort(base.getPeerHostPort());

    setSocketID(base.getSocketID());
    setPeerID(base.getPeerID());

    setPacketBody(base.getPacketBody());

}

//////////////////////////////////////////////////////////////////////////////////////




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
