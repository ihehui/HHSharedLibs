/*
 ****************************************************************************
 * rudppacket.h
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

#ifndef RUDPPACKET_H_
#define RUDPPACKET_H_

#include <QObject>
#include <QVariant>
#include <QMetaType>
#include <QHostAddress>
#include <QDateTime>
#include <QMutex>

#include <QTimer>


#include "../networklib.h"

#include "rudp_global.h"


namespace HEHUI {



class NETWORK_LIB_API RUDPPacket : public QObject
{
    Q_OBJECT
public:
    //RUDPPacket(QObject *parent = 0);
    RUDPPacket(QObject *parent = 0, quint8 packetType = quint8(RUDP::UnKnownPacket),
               quint16 packetSerialNumber = 0,
               int retransmissionTimes = RUDP_PACKET_RETRANSMISSION_TIMES);

    RUDPPacket(const RUDPPacket &packet);
    RUDPPacket & operator = (const RUDPPacket &packet);
    virtual ~RUDPPacket();

    void resetPacket();
    bool isNull();




    quint8 getPacketType() const;
    void setPacketType(quint8 packetType);


    quint16 getPacketSerialNumber() const;
    void setPacketSerialNumber(quint16 packetSerialNumber);

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

    int getRemainingRetransmissionTimes() const;
    void setRemainingRetransmissionTimes(int remainingRetransmissionTimes);

    QDateTime getLastTransmissionTime() const;
    void setLastTransmissionTime(const QDateTime &time);

    int getCheckTransmissionTimerInterval() const;
    void setCheckTransmissionTimerInterval(int interval);
    int getMaxCheckTransmissionTimerInterval() const;
    void setMaxCheckTransmissionTimerInterval(int interval);


    //    void packetTransmissionFailed();

    int packetDataSize();

    quint16 createSerialNumber() ;


signals:
    void transmissionTimeout();
    void packetTransmissionFailed();


private:

    quint8 packetType;
    quint16 m_packetSerialNumber;

    QByteArray packetData;

    int remainingRetransmissionTimes;

    QDateTime lastTransmissionTime;





};












} //namespace HEHUI


//自定义类型，要在使用前注册该类型：qRegisterMetaTypeStreamOperators<HEHUI::RUDPPacket>("HEHUI::RUDPPacket");
Q_DECLARE_METATYPE(HEHUI::RUDPPacket)






/*
////////////////////////////////  重载操作符  //////////////////////////////////////////////////////

QDataStream &operator<<(QDataStream &out, const HEHUI::RUDPPacket &packet);
QDataStream &operator>>(QDataStream &in, HEHUI::RUDPPacket &packet);



//重载操作符<<
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

//重载操作符>>
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





#endif /* RUDPPACKET_H_ */

