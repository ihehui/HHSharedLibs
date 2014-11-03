
/*
 ****************************************************************************
 * ipmulticastsocketbase.cpp
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
 * Last Modified on: 2010-06-24
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#include "ipmulticastsocketbase.h"

IPMulticastSocketBase::IPMulticastSocketBase(QObject *parent)
    :QObject(parent)
{

    udpSocket = 0;
    bound = false;
    ipMulticastGroupAddress = QHostAddress::Null;
    port = 0;

}

IPMulticastSocketBase::~IPMulticastSocketBase() {

}

QUdpSocket * IPMulticastSocketBase::getUdpSocket() const{
    return udpSocket;
}

void IPMulticastSocketBase::setUdpSocket(QUdpSocket *udpSocket){
    this->udpSocket = udpSocket;
}

bool IPMulticastSocketBase::isBound(){
    return bound;
}

void IPMulticastSocketBase::setBound(bool bound){
    this->bound = bound;
}

QHostAddress IPMulticastSocketBase::getMulticastGroupAddress() const {
    return this->ipMulticastGroupAddress;
}

void IPMulticastSocketBase::setMulticastGroupAddress(const QHostAddress &ipMulticastGroupAddress){
    this->ipMulticastGroupAddress = ipMulticastGroupAddress;
}

//QHostAddress IPMulticastSocketBase::getLocalAddress()  const{
//    return localAddress;
//}

quint16 IPMulticastSocketBase::getPort() const {
    return this->port;
}

void IPMulticastSocketBase::setPort(quint16 port){
    this->port = port;
}

//bool IPMulticastSocketBase::sendUDPDatagram(const QHostAddress &targetAddress, quint16 targetPort, const QByteArray &data){
//    QUdpSocket udpSocket;
//    qint64 size = udpSocket.writeDatagram(data, targetAddress, targetPort);
//    if(size == -1){
//        return false;
//    }
//
//    return size;
//
//}

//bool IPMulticastSocketBase::startSimpleListening(const QHostAddress &localAddress, quint16 localPort){
//
//    if(!udpSocket){
//        qCritical("Invalid UDP Socket!");
//        return false;
//    }
//
//    if(bound){
//        if(!ipMulticastGroupAddress.isEmpty()){
//            leaveGroup();
//        }else if((this->localAddress == localAddress) && (localPort == port)){
//            qWarning("Server already started!");
//            return true;
//        }
//        close();
//    }
//
//    bound = udpSocket->bind(localAddress, localPort);
//
//    return bound;
//
//}

//void IPMulticastSocketBase::readPendingDatagrams(){
//
//    QByteArray datagram;
//
//    while (hasPendingDatagrams()) {
//            datagram.resize(pendingDatagramSize());
//            QHostAddress peerAddress;
//            quint16 peerPort;
//
//            readDatagram(datagram.data(), datagram.size(), &peerAddress, &peerPort);
//            emit signalUDPDatagramsReceived(peerAddress.toString(), peerPort, datagram);
//        }
//
//    datagram.clear();
//
//}


