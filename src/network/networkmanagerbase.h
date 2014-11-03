/*
 ****************************************************************************
 * networkmanagerbase.h
 *
 * Created on: 2009-9-5
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
 * Last Modified on: 2011-07-23
 * Last Modified by: 贺辉
 ***************************************************************************
 */




#ifndef NETWORKMANAGERBASE_H_
#define NETWORKMANAGERBASE_H_

#include <QObject>
#include <QHostAddress>
#include <QMutex>

#include "global_network.h"
#include "./udp/udpsocket.h"
#include "./rudp/rudpsocket.h"

#include "networklib.h"



namespace HEHUI {


class NETWORK_LIB_API NetworkManagerBase : public QObject {
    Q_OBJECT

public:
    enum NetworkType {LAN = 0, INTERNET = 1, NWTYPE_MIXED};
    enum CommunicationMode {P2P = 0, CS = 1};

    NetworkManagerBase(PacketHandlerBase *packetHandlerBase = 0, NetworkType type = LAN, CommunicationMode mode = P2P, QObject *parent = 0);
    virtual ~NetworkManagerBase();

    void closeUDPServer(quint16 port);
    void closeRUDPServer(quint16 port);
    void closeRUDPServerInstance(RUDPServer *rudpServer);
    void closeAllServers();

    void setPacketHandler(PacketHandlerBase *packetHandlerBase);
    PacketHandlerBase * getPacketHandler();

    void setNetworkType(NetworkType type);
    NetworkType getNetworkType() const;

    void setCommunicationMode(CommunicationMode mode);
    CommunicationMode getCommunicationMode() const;

    UDPServer *getUDPServer(quint16 port, const QHostAddress &localAddress);
    UDPServer * startUDPServerListening(const QHostAddress &localAddress, quint16 localPort);
    UDPServer * startIPMulticastServerListening(const QHostAddress &ipMulticastGroupAddress, quint16 ipMulticastGroupPort);

    RUDPServer * getRUDPServer(quint16 port, const QHostAddress &localAddress);
    RUDPServer * startRUDPServerListening(const QHostAddress &localAddress, quint16 localPort, int keepAliveTimerInterval = RUDP_KEEPALIVE_TIMER_INTERVAL);


    QString errorString() const;

signals:

public slots:



    bool slotSendNewUDPDatagram(const QHostAddress &targetAddress, quint16 targetPort, QByteArray *data, quint16 localPort, bool useRUDP);


    bool slotSendPacket(Packet *packet);

private slots:

    //接收到数据时执行
    //	virtual void slotUDPDatagramsReceived(const QString &ip, quint16 port, const QByteArray &data) = 0;
    //	virtual void slotTCPDatagramsReceived(const QString &ip, quint16 port, const QByteArray &data) = 0;



protected:
    void setErrorString(const QString &errorString);


private:

    PacketHandlerBase *m_packetHandlerBase;

    NetworkType networkType;
    CommunicationMode communicationMode;

    QMultiHash<quint16, UDPServer *> udpServers;
    QMultiHash<quint16, RUDPServer *> rudpServers;
    QMutex udpMutex;
    QMutex rudpMutex;

    QString m_errorString;


};

} //namespace HEHUI

//Q_DECLARE_METATYPE(QAbstractSocket::SocketError)


#endif /* NETWORKMANAGERBASE_H_ */
