/*
 ****************************************************************************
 * multicastlinux.h
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
 * Last Modified on: 2010-05-15
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#ifndef MULTICASTLINUX_H_
#define MULTICASTLINUX_H_

#include <QObject>
#include <QUdpSocket>
#include <QSocketNotifier>

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#include "ipmulticastsocketbase.h"


class IPMulticastLinux: public IPMulticastSocketBase {
    Q_OBJECT
public:
    IPMulticastLinux(QObject *parent);
    virtual ~IPMulticastLinux();


public slots:
    bool startIPMulticastListening(const QHostAddress &ipMulticastGroupAddress, quint16 ipMulticastGroupPort);
    void leaveGroup();
    bool slotSendUDPDatagramViaBoundSocket(const QString &ip, quint16 port, const QByteArray &data);




private slots:
    //	void readPendingDatagrams();

    void joinGroup();
    void slotDataReceived();


private:
    //	QUdpSocket * udpSocket;
    long count,Receivedpackets;



    char* ip;
    int listenPort;
    int socket_descriptor;
    struct ip_mreq command;

    int i;

};

#endif /* MULTICASTLINUX_H_ */
