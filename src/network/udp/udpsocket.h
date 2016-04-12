/*
 ****************************************************************************
 * udpsocket.h
 *
 * Created on: 2009-9-3
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



#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include <QObject>
#include <QUdpSocket>

//#include "multicast/ipmulticast.h"

#include "../networklib.h"


namespace HEHUI {
class PacketBase;

class NETWORK_LIB_API UDPSocket : public QUdpSocket {
    Q_OBJECT
public:
    enum ListeningState{NotListening = 1, SimpleListening = 2, IPMulticastListening = 3};

    UDPSocket(QObject *parent);
    virtual ~UDPSocket();

//    const IPMulticastSocket * getIPMulticastSocket() const;

    ListeningState getListeningState() const;

    bool startSimpleListening(const QHostAddress &localAddress, quint16 localPort);
    bool startIPMulticastListening(const QHostAddress &ipMulticastGroupAddress, quint16 ipMulticastGroupPort);


    bool sendDatagram(const QByteArray &data, const QHostAddress &targetAddress, quint16 targetPort, QString *errorString = 0);

    static bool sendUDPDatagramWithAnyPort(const QString &targetAddress, quint16 targetPort, const QByteArray &data, QString *errorString = 0);
    static bool sendUDPDatagramWithAnyPort(const QHostAddress &targetAddress, quint16 targetPort, const QByteArray &data, QString *errorString = 0);


signals:
//    void signalNewUDPPacketReceived(PacketBase *packet);
    void packetReceived(const PacketBase &packet);

private slots:
    virtual void readPendingDatagrams();

private:

    QByteArray *datagram;

//    IPMulticastSocket *ipMulticastSocket;

    ListeningState listeningState;

};

typedef UDPSocket UDPServer;

} //namespace HEHUI

#endif /* UDPSOCKET_H_ */
