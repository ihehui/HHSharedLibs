/*
 ****************************************************************************
 * multicastwin.cpp
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





#include <QtNetwork>

#include "multicastwin.h"


IPMultiCastWin::IPMultiCastWin(QObject *parent)
    : IPMulticastSocketBase(parent)
{
    //udpSocket = 0;
    count=0;
    Receivedpackets=0;

}

IPMultiCastWin::~IPMultiCastWin()
{

    if(isBound()){
        leaveGroup();
    }



    //    if(udpSocket){
    //        udpSocket->close();
    //        delete udpSocket;
    //        udpSocket = 0;
    //    }

    //    delete [] recvbuf;


}

bool IPMultiCastWin::startIPMulticastListening(const QHostAddress &ipMulticastGroupAddress, quint16 ipMulticastGroupPort) {

    QUdpSocket *udpSocket = getUdpSocket();
    if(!udpSocket){
        qCritical("Invalid UDP Socket!");
        return false;
    }


    //	int len = sizeof(struct sockaddr_in);
    //	int ret;

    //初始化WinSock2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
        printf("WSAStartup() failed\n");
        //return -1;
        return false;
    }

    //创建一个SOCK_DGRAM类型的SOCKET
    //其中,WSA_FLAG_MULTIPOINT_C_LEAF表示IP多播在控制面层上属于"无根"类型;
    //WSA_FLAG_MULTIPOINT_D_LEAF表示IP多播在数据面层上属于"无根",有关控制面层和数据面层有关概念请参阅MSDN说明。

    if ((sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0,
                          WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF
                          | WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
        printf("socket failed with:%d\n", WSAGetLastError());
        WSACleanup();
        //return -1;
        return false;
    }
    bFlag = TRUE; //file://设置套接字选项，使套接字为可重用端口地址
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &bFlag, sizeof(bFlag));

    //将sock绑定到本机某端口上。
    local.sin_family = AF_INET;
    //local.sin_port = htons(MCASTPORT);
    local.sin_port = htons(ipMulticastGroupPort);
    local.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*) &local, sizeof(local)) == SOCKET_ERROR) {
        printf("bind failed with:%d \n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        //return -1;
        return false;
    }

    //加入多播组
    remote.sin_family = AF_INET;
    //remote.sin_port = htons(MCASTPORT);
    remote.sin_addr.s_addr = inet_addr((char*)ipMulticastGroupPort);
    //remote.sin_addr.s_addr = inet_addr(MCASTADDR);
    remote.sin_addr.s_addr = inet_addr(ipMulticastGroupAddress.toString().toLocal8Bit().data());




    /* Winsock1.0 */
    /*
     mcast.imr_multiaddr.s_addr = inet_addr(MCASTADDR);
     mcast.imr_interface.s_addr = INADDR_ANY;
     if( setsockopt(sockM,IPPROTO_IP,IP_ADD_MEMBERSHIP,
     (char*)&mcast,sizeof(mcast)) == SOCKET_ERROR)
     {
     printf("setsockopt(IP_ADD_MEMBERSHIP) failed:%d\n",WSAGetLastError());
     closesocket(sockM);
     WSACleanup();
     return -1;
     }
     */

    /* Winsock2.0*/
    if ((sockM = WSAJoinLeaf(sock, (SOCKADDR*) &remote, sizeof(remote), NULL,
                             NULL, NULL, NULL, JL_BOTH)) == INVALID_SOCKET) {
        printf("WSAJoinLeaf() failed:%d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        //return -1;
        return false;
    }


    udpSocket->setSocketDescriptor(sock);
    //	udpSocket = new QUdpSocket();
    //	udpSocket->setSocketDescriptor(sock);
    //	connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

    setBound(true);
    setMulticastGroupAddress(ipMulticastGroupAddress);
    setPort(ipMulticastGroupPort);

    return true;

}

void IPMultiCastWin::leaveGroup(){
    closesocket(sock);
    WSACleanup();

    setMulticastGroupAddress(QHostAddress::Null);
    setPort(0);
    setBound(false);

    qDebug()<<"----MultiCastWin::leaveGroup()";


}


//void MultiCastWin::readPendingDatagrams() {
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
//		qDebug() <<"----MultiCastWin::readPendingDatagrams()~~Receivedpackets:"<< Receivedpackets;
//
//		qDebug() << "----MultiCastWin::readPendingDatagrams()~~data size:" << data.size();
//
//		Receivedpackets++;
//	}
//
//	emit signalUDPDatagramsReceived(udpSocket->peerAddress().toString(), udpSocket->peerPort(), datagram);
//
//
//	datagram.clear();
//}

bool IPMultiCastWin::slotSendUDPDatagramViaBoundSocket(const QString &ip, quint16 port, const QByteArray &data){


    QUdpSocket *udpSocket = getUdpSocket();
    if(!udpSocket){
        qCritical("Invalid UDP Socket!");
        return false;
    }

    //	QByteArray datagram;
    //	datagram = message.toUtf8().toBase64();

    qint64 bytesSent = udpSocket->writeDatagram(data, QHostAddress(ip), port);
    if (bytesSent != data.size() ){
        qDebug()<<"XXXXMultiCastWin::slotSendUDPDatagramsViaBoundSocket(....): Failed!!!!!!!!!";

        return false;
    }

    qDebug()<<"----MultiCastWin::slotSendUDPDatagramsViaBoundSocket(....): Successful!!!!!!!!!";


    /*

    if (sendto(sockM, datagram, datagram.size(), 0,
            (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR) {
        printf("sendto failed with: %d\n", WSAGetLastError());
        closesocket(sockM);
        closesocket(sock);
        WSACleanup();
        return false;
    }
*/

    return true;

}

