/*
 ****************************************************************************
 * rudppacket.cpp
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

#include "rudppacket.h"

namespace HEHUI
{



//////////////////////////////////////////////////////////////////////////////////////////

//RUDPPacket::RUDPPacket(QObject *parent)
//    : QObject(parent)
//{

//    //Q_ASSERT_X(parent, "RUDPPacket::RUDPPacket(...)", "Invalid parent object!");

//    this->packetType = quint8(RUDP::UnKnownPacket);
//    this->m_packetSerialNumber = 0;
//    this->packetData = QByteArray();
//    this->packetData.resize(0);

//    this->remainingRetransmissionTimes = RUDP_PACKET_RETRANSMISSION_TIMES;
//    this->lastTransmissionTime = QDateTime();

//    checkTransmissionTimer = new QTimer();
//    connect(checkTransmissionTimer, SIGNAL(timeout()), this, SIGNAL(transmissionTimeout()));
//    checkTransmissionTimerInterval  = 400;

//}

RUDPPacket::RUDPPacket(QObject *parent, quint8 packetType, quint16 packetSerialNumber, int retransmissionTimes)
    : QObject(parent)

{

    //Q_ASSERT_X(parent, "RUDPPacket::RUDPPacket(...)", "Invalid parent object!");

    this->packetType = packetType;
    this->m_packetSerialNumber = packetSerialNumber;
    this->packetData = QByteArray();
    this->packetData.resize(0);

    this->remainingRetransmissionTimes = retransmissionTimes;

    this->lastTransmissionTime = QDateTime();


}


RUDPPacket::RUDPPacket(const RUDPPacket &packet)
{
    *this = packet;
}

RUDPPacket &RUDPPacket::operator = (const RUDPPacket &packet)
{

    this->packetType = packet.getPacketType();
    this->m_packetSerialNumber = packet.getPacketSerialNumber();
    this->packetData = packet.getPacketData();
    this->remainingRetransmissionTimes = packet.getRemainingRetransmissionTimes();
    this->lastTransmissionTime = packet.getLastTransmissionTime();

    return *this;
}

RUDPPacket::~RUDPPacket()
{



}

void RUDPPacket::resetPacket()
{
    this->packetType = RUDP::UnKnownPacket;
    this->m_packetSerialNumber = 0;
    this->packetData.clear();
    this->packetData.resize(0);

    this->remainingRetransmissionTimes = -1;
    this->lastTransmissionTime = QDateTime();


}

bool RUDPPacket::isNull()
{
    return ( RUDP::UnKnownPacket == packetType);
}


//void RUDPPacket::startCheckTransmissionTimer(int interval){
//    checkTransmissionTimerInterval = interval;
//    checkTransmissionTimer->start(checkTransmissionTimerInterval);
//}


quint8 RUDPPacket::getPacketType() const
{
    return packetType;
}

void RUDPPacket::setPacketType(quint8 packetType)
{
    this->packetType = packetType;
}



quint16 RUDPPacket::getPacketSerialNumber() const
{
    return m_packetSerialNumber;
}

void RUDPPacket::setPacketSerialNumber(quint16 packetSerialNumber)
{
    this->m_packetSerialNumber = packetSerialNumber;
}

QByteArray RUDPPacket::getPacketData() const
{
    return packetData;
}

void RUDPPacket::setPacketData(const QByteArray &data)
{
    this->packetData = data;
}

int RUDPPacket::getRemainingRetransmissionTimes() const
{
    //    QMutexLocker locker(&mutex);
    return this->remainingRetransmissionTimes;
}

void RUDPPacket::setRemainingRetransmissionTimes(int remainingRetransmissionTimes)
{
    //    QMutexLocker locker(&mutex);
    this->remainingRetransmissionTimes = remainingRetransmissionTimes;
}

QDateTime RUDPPacket::getLastTransmissionTime() const
{
    //    QMutexLocker locker(&mutex);
    return this->lastTransmissionTime;
}

void RUDPPacket::setLastTransmissionTime(const QDateTime &time)
{
    //    QMutexLocker locker(&mutex);
    this->lastTransmissionTime = time;

}

//void RUDPPacket::packetTransmissionFailed() {
//	//    QMutexLocker locker(&mutex);
////	if ((--remainingRetransmissionTimes) < 0) {
////		//		emit signalPacketTransmissionFailed();
////	}

//    remainingRetransmissionTimes--;

//}

int RUDPPacket::packetDataSize()
{
    return this->packetData.size();
}

quint16 RUDPPacket::createSerialNumber()
{

    if ((++m_packetSerialNumber) == quint16(RUDP_MAX_PACKET_SN)) {
        m_packetSerialNumber = 0;
    }
    m_packetSerialNumber = (m_packetSerialNumber % quint16(RUDP_MAX_PACKET_SN));
    return (m_packetSerialNumber == 0) ? (++m_packetSerialNumber) : m_packetSerialNumber;

}

//////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////













/*
////////////////////////////////  重载操作符  //////////////////////////////////////////////////////

QDataStream &operator<<(QDataStream &out, const HEHUI::RUDPPacket &packet);
QDataStream &operator>>(QDataStream &in, HEHUI::RUDPPacket &packet);



//重载操作符“<<”
QDataStream &operator<<(QDataStream &out, const HEHUI::RUDPPacket &packet){
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
QDataStream &operator>>(QDataStream &in, HEHUI::RUDPPacket &packet){
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
