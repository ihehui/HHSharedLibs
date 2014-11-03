/*
 ****************************************************************************
 * ipmulticastsocketbase.h
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
 * Last Modified on: 2010-06-27
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#ifndef IPMULTICASRSOCKETBASE_H_
#define IPMULTICASRSOCKETBASE_H_

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>


class IPMulticastSocketBase : public QObject{
    Q_OBJECT
public:
    IPMulticastSocketBase(QObject *parent = 0);
    virtual ~IPMulticastSocketBase();

    QUdpSocket * getUdpSocket() const;
    void setUdpSocket(QUdpSocket *udpSocket);
    bool isBound();
    void setBound(bool bound);

    QHostAddress getMulticastGroupAddress() const;
    void setMulticastGroupAddress(const QHostAddress &ipMulticastGroupAddress);
    //        QHostAddress getLocalAddress()  const;
    quint16 getPort() const;
    void setPort(quint16 port);

    //    static bool sendUDPDatagram(const QHostAddress &targetAddress, quint16 targetPort, const QByteArray &data);

signals:
    //        void signalUDPDatagramsReceived(const QString &peerIP, quint16 peerPort, const QByteArray &data);

public slots:
    //    bool startSimpleListening(const QHostAddress &localAddress, quint16 localPort);

    virtual bool startIPMulticastListening(const QHostAddress &ipMulticastGroupAddress, quint16 ipMulticastGroupPort) = 0;
    virtual void leaveGroup() = 0;
    virtual bool slotSendUDPDatagramViaBoundSocket(const QString &ip, quint16 port, const QByteArray &data) = 0;



private slots:
    //    virtual void readPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    bool bound;

    QHostAddress ipMulticastGroupAddress;
    //    QHostAddress localAddress;
    quint16 port;

};

#endif /* IPMULTICASRSOCKETBASE_H_ */
