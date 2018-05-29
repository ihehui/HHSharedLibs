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

#ifndef PACKETBASE_H_
#define PACKETBASE_H_

#include <QDataStream>
#include <QByteArray>
#include <QMetaType>
#include <QHostAddress>
//#include <QDateTime>
//#include <QMutex>

#include "global_network.h"

#include "networklib.h"


namespace HEHUI
{

class PacketBase;

////////////////////////////////////////////////////////////////////////

NETWORK_LIB_API QDataStream &operator<<(QDataStream &out, const HEHUI::PacketBase &packet);
NETWORK_LIB_API QDataStream &operator>>(QDataStream &in, HEHUI::PacketBase &packet);

////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
class NETWORK_LIB_API PacketBase
{

public:
    PacketBase();
    PacketBase(quint8 packetType, quint8 packetSubType = UserDefinedPacket);

    PacketBase(const PacketBase &packet);
    PacketBase &operator = (const PacketBase &packet);
    virtual ~PacketBase();

    static void registerMetaTypeStreamOperators();


    void resetPacket();
    virtual bool isValid();

    virtual QByteArray toByteArray() const;
    bool fromByteArray(QByteArray *data);


    quint8 getPacketType() const;
    void setPacketType(quint8 type);
    quint8 getPacketSubType() const;
    void setPacketSubType(quint8 type);


    QHostAddress getPeerHostAddress() const;
    void setPeerHostAddress(const QHostAddress &peerHostAddress);

    quint16 getPeerHostPort() const;
    void setPeerHostPort(quint16 peerHostPort);


    SOCKETID getSocketID() const;
    void setSocketID(SOCKETID id);

    QString getPeerID() const;
    void setPeerID(const QString &id);

    const static QString getLocalID();
    static void setLocalID(const QString &id);


    QByteArray getPacketBody() const ;
    void setPacketBody(const QByteArray &data);


private:
    static QString m_localID;

    quint8 m_packetType;
    quint8 m_packetSubType;

    QByteArray m_packetBody;

    QHostAddress peerHostAddress;
    quint16 peerHostPort;

    SOCKETID m_socketID;
    QString m_peerID;

};

////////////////////////////////////////////////////////////////
class NETWORK_LIB_API Packet : public PacketBase
{
public:
    Packet();
    Packet(quint8 packetType, quint8 packetSubType = UnKnownPacket);
    Packet(const PacketBase &base);
    Packet &operator = (const PacketBase &base);
    ~Packet();

    bool isEncrypted() const;
    void setEncrypted(bool encrypted);

    virtual QByteArray toByteArray();
//    void parsePacketBody();

    virtual void fromPacket(const PacketBase &base);


private:
    //virtual void init() = 0;
    virtual void parsePacketBody(QByteArray &packetBody) = 0;
    virtual QByteArray packBodyData() = 0;

protected:
    virtual void convert(const PacketBase &base);
    virtual QByteArray encrypt(const QByteArray &data) = 0;
    virtual QByteArray decrypt(const QByteArray &encryptedData) = 0;

private:
    bool m_encrypted;


};

////////////////////////////////////////////////////////////////



} //namespace HEHUI


//自定义类型，要在使用前注册该类型：qRegisterMetaTypeStreamOperators<HEHUI::Packet>("HEHUI::Packet");
Q_DECLARE_METATYPE(HEHUI::PacketBase)

//////////////////////////////////////////////////////////////////////////////////////////





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





#endif /* PACKETBASE_H_ */

