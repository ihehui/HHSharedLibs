/*
 ****************************************************************************
 * udpsocket.cpp
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



#include <QUdpSocket>
#include <QDataStream>
#include <QDebug>

#include "udpsocket.h"
#include "packetbase.h"

//#include "./packethandler/packetstreamoperator.h"


namespace HEHUI
{


UDPSocket::UDPSocket(QObject *parent)
    : QUdpSocket(parent)
{

    datagram = new QByteArray();
    datagram->resize(0);

    connect(this, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

//    ipMulticastSocket = 0;

    listeningState = NotListening;

    //注册自定义类型，必须重载“<<”和“>>”, 见"packetstreamoperator.h"
    //qRegisterMetaTypeStreamOperators<HEHUI::Packet>("HEHUI::Packet");
    PacketBase::registerMetaTypeStreamOperators();

}

UDPSocket::~UDPSocket()
{
    qDebug() << "UDPSocket::~UDPSocket()";

//    if(ipMulticastSocket){
//        delete ipMulticastSocket;
//        ipMulticastSocket = 0;
//    }

    datagram->clear();
    datagram->resize(0);
    delete datagram;
    datagram = 0;

    close();

}

//const IPMulticastSocket * UDPSocket::getIPMulticastSocket() const{
//    return ipMulticastSocket;
//}

UDPSocket::ListeningState UDPSocket::getListeningState() const
{
    return this->listeningState;
}

bool UDPSocket::startSimpleListening(const QHostAddress &localAddress, quint16 localPort)
{

//    if(ipMulticastSocket){
//        if((ipMulticastSocket->getUdpSocket()->localAddress() == localAddress) && (ipMulticastSocket->getPort() == localPort)){
//            qWarning("Server already started!");
//            return true;
//        }

//        if(ipMulticastSocket->isBound()){
//            ipMulticastSocket->leaveGroup();
//        }
//    }

    close();

    QHostAddress address = localAddress;
    if(localAddress == QHostAddress::Null) {
        address = QHostAddress::Any;
    }

    bool bound = bind(address, localPort);
    if(bound) {
        listeningState = SimpleListening;
    } else {
        listeningState = NotListening;
    }

    return bound;

}

bool UDPSocket::startIPMulticastListening(const QHostAddress &ipMulticastGroupAddress, quint16 ipMulticastGroupPort)
{

//    if(!ipMulticastSocket){
//        ipMulticastSocket = new IPMulticastSocket(this);

//    }else{
//        if((ipMulticastSocket->getMulticastGroupAddress() == ipMulticastGroupAddress) && (ipMulticastSocket->getPort() == ipMulticastGroupPort)){
//            qWarning("Server already started!");
//            return true;
//        }
//        ipMulticastSocket->leaveGroup();
//    }

//    close();
//    ipMulticastSocket->setUdpSocket(this);

//    bool bound = ipMulticastSocket->startIPMulticastListening(ipMulticastGroupAddress, ipMulticastGroupPort);

    close();
    listeningState = NotListening;

    //bool bound = bind(ipMulticastGroupPort, QUdpSocket::ShareAddress);
    bool bound = bind(QHostAddress::AnyIPv4, ipMulticastGroupPort, QUdpSocket::ShareAddress);
    if(!bound) {
        return bound;
    }

    bound = joinMulticastGroup(ipMulticastGroupAddress);
    if(bound) {
        listeningState = IPMulticastListening;
    }

    return bound;

}

bool UDPSocket::sendDatagram(const QByteArray &data, const QHostAddress &targetAddress, quint16 targetPort, QString *errorString)
{
    //qWarning()<<"UDPSocket::sendDatagram(...)-targetAddress:"<<targetAddress.toString()<<" targetPort:"<<targetPort;

    qint64 size = writeDatagram(data, targetAddress, targetPort);
    if(errorString) {
        *errorString = this->errorString();
    }

    if(size == -1) {
        qCritical() << QString("UDP Datagram Sent Failed! Target Address:%1, Port:%2, %3").arg(targetAddress.toString()).arg(targetPort).arg(this->errorString());
        return false;
    }

    return (size == data.size()) ? true : false;

}


bool UDPSocket::sendUDPDatagramWithAnyPort(const QString &targetAddress, quint16 targetPort, const QByteArray &data, QString *errorString)
{
    return sendUDPDatagramWithAnyPort(QHostAddress(targetAddress), targetPort, data, errorString);
}

bool UDPSocket::sendUDPDatagramWithAnyPort(const QHostAddress &targetAddress, quint16 targetPort, const QByteArray &data, QString *errorString)
{
    qDebug() << "UDPSocket::sendUDPDatagram(...)-targetAddress:" << targetAddress.toString() << " targetPort:" << targetPort;

    QUdpSocket udpSocket;
    qint64 size = udpSocket.writeDatagram(data, targetAddress, targetPort);
    if(errorString) {
        *errorString = udpSocket.errorString();
    }

    if(size == -1) {
        qCritical() << QString("UDP Datagram Sent Failed! Target Address:%1, Port:%2, %3").arg(targetAddress.toString()).arg(targetPort).arg(udpSocket.errorString());
        return false;
    }

    return (size == data.size()) ? true : false;;

}

void UDPSocket::readPendingDatagrams()
{
    //qDebug()<<"----UDPSocket::readPendingDatagrams()";

    while (hasPendingDatagrams()) {
        datagram->clear();
        qint64 datagramSize = pendingDatagramSize();
        datagram->resize(datagramSize);
        QHostAddress peerAddress;
        quint16 peerPort;

        qint64 readSize = readDatagram(datagram->data(), datagramSize, &peerAddress, &peerPort);
        if(readSize == -1) {
            qWarning() << "Can not read datagram!";
            break;
        }
        //qDebug()<<"~~datagramSize:"<<datagramSize;

//        QDataStream in(datagram, QIODevice::ReadOnly);
//        in.setVersion(QDataStream::Qt_4_8);
//        QVariant v;
//        in >> v;
//        if (v.canConvert<PacketBase>()){
//            //Packet *packet = new Packet();
//            PacketBase *packet = PacketHandlerBase::getPacket();
//            *packet = v.value<PacketBase>();
//            packet->setTransmissionProtocol(TP_UDP);
//            packet->setPeerHostAddress(peerAddress);
//            packet->setPeerHostPort(peerPort);
//            packet->setLocalHostAddress(localAddress());
//            packet->setLocalHostPort(localPort());

//            emit signalNewUDPPacketReceived(packet);

//        }else{
//            qWarning()<<"ERROR! Can not convert UDP data to Packet!";
//        }

        PacketBase packet;
        if(packet.fromByteArray(datagram)) {
            packet.setPeerHostAddress(peerAddress);
            packet.setPeerHostPort(peerPort);
            emit packetReceived(packet);
        }

    }


    datagram->clear();
    datagram->resize(0);

}
























} //namespace HEHUI

