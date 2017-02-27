/*
 ****************************************************************************
 * multicastlinux.cpp
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





#include "multicastlinux.h"
#include <QDebug>

IPMulticastLinux::IPMulticastLinux(QObject *parent)
    : IPMulticastSocketBase(parent)
{
    //	udpSocket = 0;
    count = 0;
    Receivedpackets = 0;

    ip = 0;

    i = 1;

    //joinGroup();

    qDebug() << "----MulticastLinux::MulticastLinux(...)~~MulticastLinux::MulticastLinux(const QString &ip, uint port, QObject *parent)";

}

IPMulticastLinux::~IPMulticastLinux()
{

    if(isBound()) {
        leaveGroup();
    }

    //    if(udpSocket){
    //        udpSocket->close();
    //        delete udpSocket;
    //        udpSocket = 0;
    //    }

    delete ip;
    ip = 0;

}


bool IPMulticastLinux::startIPMulticastListening(const QHostAddress &ipMulticastGroupAddress, quint16 ipMulticastGroupPort)
{

    QUdpSocket *udpSocket = getUdpSocket();
    if(!udpSocket) {
        qCritical("Invalid UDP Socket!");
        return false;
    }

    ip = ipMulticastGroupAddress.toString().toLocal8Bit().data();

    int loop = 1;

    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    memset(&command, 0, sizeof(command));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(ipMulticastGroupPort);

    if (i == 1) {
        i++;
        if ((socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("Socket Error");
            return false;
            //exit( EXIT_FAILURE);
        }
        loop = 1;
        if (setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &loop,
                       sizeof(loop)) < 0) {
            perror("setsockopt:SO_REUSEADDR");
            return false;
            //exit( EXIT_FAILURE);
        }
        if (bind(socket_descriptor, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
            perror("Bind Error");
            return false;
            //exit( EXIT_FAILURE);
        }

        if (setsockopt(socket_descriptor, IPPROTO_IP, IP_MULTICAST_IF, &sin,
                       sizeof(sin)) < 0) {
            perror("setsockopt:IP_MULTICAST_LOOP");
            return false;
            //exit( EXIT_FAILURE);
        }

        //command.imr_multiaddr.s_addr = inet_addr(ipEdit->text());
        command.imr_multiaddr.s_addr = inet_addr(ip);
        command.imr_interface.s_addr = htonl(INADDR_ANY);
        //if (command.imr_multiaddr.s_addr == -1) {
        if (!command.imr_multiaddr.s_addr > 0) {
            //perror("224.0.0.1 not a legal multicast address");
            return false;
            //exit( EXIT_FAILURE);
        }
        if (setsockopt(socket_descriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       &command, sizeof(command)) < 0) {
            perror("setsockopt:IP_ADD_MEMBERSHIP");
        }

        //		QSocketNotifier *socketNotifier = new QSocketNotifier(
        //				socket_descriptor, QSocketNotifier::Read, this);
        //		QObject::connect(socketNotifier, SIGNAL(activated(int)), this,
        //				SLOT(slotDataReceived()));

    } else {
        //command.imr_multiaddr.s_addr = inet_addr(ipEdit->text());
        command.imr_multiaddr.s_addr = inet_addr(ip);
        command.imr_interface.s_addr = htonl(INADDR_ANY);
        //if (command.imr_multiaddr.s_addr == -1) {
        if (!command.imr_multiaddr.s_addr > 0) {
            //perror("224.0.0.1 not a legal multicast address");
            return false;
            //exit( EXIT_FAILURE);
        }
        if (setsockopt(socket_descriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       &command, sizeof(command)) < 0) {
            perror("setsockopt:IP_ADD_MEMBERSHIP");
        }

        //		QSocketNotifier *socketNotifier = new QSocketNotifier(
        //				socket_descriptor, QSocketNotifier::Read, this);
        //		QObject::connect(socketNotifier, SIGNAL(activated(int)), this,
        //				SLOT(slotDataReceived()));

    }


    udpSocket->setSocketDescriptor(socket_descriptor);
    //	udpSocket = new QUdpSocket();
    //        udpSocket->setSocketDescriptor(socket_descriptor);
    //        connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

    setBound(true);
    setMulticastGroupAddress(ipMulticastGroupAddress);
    setPort(ipMulticastGroupPort);

    return true;

}


//void MulticastLinux::readPendingDatagrams(){
//	QByteArray datagram;
//	while (udpSocket->hasPendingDatagrams()) {
//		//QByteArray datagram;
//		datagram.resize(udpSocket->pendingDatagramSize());
//		QHostAddress sender;
//		quint16 senderPort;
//
//		udpSocket->readDatagram(datagram.data(), datagram.size(), &sender,
//				&senderPort);
//		QString data = QString::fromUtf8(datagram);
//		memcpy(&count, datagram.data(), sizeof(long));
//		qDebug() <<"----MulticastLinux::readPendingDatagrams()~~Receivedpackets:"<< Receivedpackets;
//		qDebug()<<"----MulticastLinux::readPendingDatagrams()~~count:"<< count;
//
//		qDebug() << "----MulticastLinux::readPendingDatagrams()~~data size:" << data.size();
//		qDebug() << "----MulticastLinux::readPendingDatagrams()~~localAddress ():" << udpSocket->localAddress ();
//
//		Receivedpackets++;
//	}
//
//	emit signalUDPDatagramsReceived(udpSocket->peerAddress().toString(), udpSocket->peerPort(), datagram);
//
//	datagram.clear();
//
//}



void IPMulticastLinux::joinGroup()
{
    //struct ip_mreq command;
    //int listenPort = atoi(portEdit->text());

    int loop = 1;

    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    memset(&command, 0, sizeof(command));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(listenPort);

    if (i == 1) {
        i++;
        if ((socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("socket");
            exit( EXIT_FAILURE);
        }
        loop = 1;
        if (setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &loop,
                       sizeof(loop)) < 0) {
            perror("setsockopt:SO_REUSEADDR");
            exit( EXIT_FAILURE);
        }
        if (bind(socket_descriptor, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
            perror("bind");
            exit( EXIT_FAILURE);
        }

        if (setsockopt(socket_descriptor, IPPROTO_IP, IP_MULTICAST_IF, &sin,
                       sizeof(sin)) < 0) {
            perror("setsockopt:IP_MULTICAST_LOOP");
            exit( EXIT_FAILURE);
        }

        //command.imr_multiaddr.s_addr = inet_addr(ipEdit->text());
        command.imr_multiaddr.s_addr = inet_addr(ip);
        command.imr_interface.s_addr = htonl(INADDR_ANY);
        //if (command.imr_multiaddr.s_addr == -1) {
        if (!command.imr_multiaddr.s_addr > 0) {
            //perror("224.0.0.1 not a legal multicast address");
            exit( EXIT_FAILURE);
        }
        if (setsockopt(socket_descriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       &command, sizeof(command)) < 0) {
            perror("setsockopt:IP_ADD_MEMBERSHIP");
        }

        QSocketNotifier *socketNotifier = new QSocketNotifier(
            socket_descriptor, QSocketNotifier::Read, this);

        QObject::connect(socketNotifier, SIGNAL(activated(int)), this,
                         SLOT(slotDataReceived()));
    } else {
        //command.imr_multiaddr.s_addr = inet_addr(ipEdit->text());
        command.imr_multiaddr.s_addr = inet_addr(ip);
        command.imr_interface.s_addr = htonl(INADDR_ANY);
        //if (command.imr_multiaddr.s_addr == -1) {
        if (!command.imr_multiaddr.s_addr > 0) {
            //perror("224.0.0.1 not a legal multicast address");
            exit( EXIT_FAILURE);
        }
        if (setsockopt(socket_descriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       &command, sizeof(command)) < 0) {
            perror("setsockopt:IP_ADD_MEMBERSHIP");
        }

        QSocketNotifier *socketNotifier = new QSocketNotifier(
            socket_descriptor, QSocketNotifier::Read, this);

        QObject::connect(socketNotifier, SIGNAL(activated(int)), this,
                         SLOT(slotDataReceived()));
    }

    setBound(true);

    /*if(setsockopt(socket_descriptor, IPPROTO_IP, IP_DROP_MEMBERSHIP,
     &command, sizeof(command)) < 0)
     {
     perror("setsockopt:IP_DROP_MEMBERSHIP");
     }*/
}

void IPMulticastLinux::leaveGroup()
{
    if (setsockopt(socket_descriptor, IPPROTO_IP, IP_DROP_MEMBERSHIP, &command, sizeof(command)) < 0) {
        perror("setsockopt:IP_DROP_MEMBERSHIP");
    }

    memset(&socket_descriptor, 0, socket_descriptor);


    close(socket_descriptor);

    setMulticastGroupAddress(QHostAddress::Null);
    setPort(0);
    setBound(false);

    ip = 0;

}

void IPMulticastLinux::slotDataReceived()
{
    int sin_len;
    char message[256];
    //int listenPort = atoi(portEdit->text());

    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    memset(message, 0, 256);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(listenPort);

    sin_len = sizeof(sin);

    if (recvfrom(socket_descriptor, message, 256, 0, (struct sockaddr *) &sin,
                 (socklen_t *) &sin_len) == -1) {
        perror("recvfrom");
    }
    qDebug() << QString("-----MulticastLinux::slotDataReceived()~~Data received from server: %1").arg(message);
    //emit signalUDPDatagramsReceived(message);

    /*if(setsockopt(socket_descriptor, IPPROTO_IP, IP_DROP_MEMBERSHIP,
     &command, sizeof(command)) < 0)
     {
     perror("setsockopt:IP_DROP_MEMBERSHIP");
     }*/

}



/*

bool IPMulticastLinux::slotSendData(const QString &ip, quint16 port, const QString &message) {
    qDebug() << "MulticastLinux::slotSendData()";

    //char message[256];

    //int targetMCGroupPort = atoi(portEdit->text());
    int targetMCGroupPort = port;
    int socket_descriptor;
    struct sockaddr_in targetMCGroupAddress;
     首先建立套接口
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_descriptor == -1) {
        perror("Opening socket");
        return false;
        //exit( EXIT_FAILURE);
    }
     初始化IP多播地址
    memset(&targetMCGroupAddress, 0, sizeof(targetMCGroupAddress));
    targetMCGroupAddress.sin_family = AF_INET;
    targetMCGroupAddress.sin_addr.s_addr = inet_addr(ip.toLocal8Bit().data());
    targetMCGroupAddress.sin_port = htons(targetMCGroupPort);

     开始进行IP多播
    // while(1)
    // {
    QString temp = message;

    if (sendto(socket_descriptor, temp.toLocal8Bit().data(), temp.length(), 0,(struct sockaddr *) &targetMCGroupAddress, sizeof(targetMCGroupAddress)) < 0) {
        perror("sendto");
        return false;
        //exit( EXIT_FAILURE);
    }
    //sendText->clear();
    //temp = "";
    //sleep(2);
    //}
    // exit(1);

    return true;

}

*/



bool IPMulticastLinux::slotSendUDPDatagramViaBoundSocket(const QString &ip, quint16 port, const QByteArray &data)
{

    QUdpSocket *udpSocket = getUdpSocket();
    if(!udpSocket) {
        qCritical("Invalid UDP Socket!");
        return false;
    }
    //QByteArray datagram = data;
    //datagram = message.toUtf8().toBase64();

    qint64 bytesSent = udpSocket->writeDatagram(data, QHostAddress(ip), port);

    if (bytesSent != data.size() ) {
        qDebug() << "XXXXMulticastLinux::slotSendUDPDatagramsViaBoundSocket(...)~~Data Size:" << data.size() << "---Failed!!!!!!!!!";

        return false;
    }


    qDebug() << "----MulticastLinux::slotSendUDPDatagramsViaBoundSocket(...)~~Data Size:" << data.size() << "---Successful!!!!!!!!!";


    return true;

}


