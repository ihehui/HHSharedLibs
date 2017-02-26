/*
 ****************************************************************************
 * networkmanagerbase.cpp
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




#include <QStringList>
#include <QProcess>
#include <QDebug>

#include "networkmanagerbase.h"
//#include "./udpServer/udpServer.h"
//#include "tcp/tcpserver.h"


namespace HEHUI
{


//QMultiHash<quint16, UDPServer *> NetworkManagerBase::udpServers = QMultiHash<quint16, UDPServer *> ();
//QMultiHash<quint16, TcpServer *> NetworkManagerBase::tcpServers = QMultiHash<quint16, TcpServer *> ();
//QMultiHash<QHostAddress, TcpSocketConnection *> NetworkManagerBase::tcpSocketConnections = QMultiHash<QHostAddress, TcpSocketConnection *> ();

//QMutex * NetworkManagerBase::udpMutex = new QMutex();
//QMutex * NetworkManagerBase::tcpMutex = new QMutex();


NetworkManagerBase::NetworkManagerBase(PacketHandlerBase *packetHandlerBase, NetworkType type, CommunicationMode mode, QObject *parent)
    : QObject(parent), m_packetHandlerBase(packetHandlerBase), networkType(type), communicationMode(mode)
{

    //Q_ASSERT_X(m_packetHandlerBase, "NetworkManagerBase::NetworkManagerBase(...)", "Invalid PacketHandlerBase!");


//    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QHostAddress>("QHostAddress");

    m_errorString = "";


}

NetworkManagerBase::~NetworkManagerBase()
{
    qDebug() << "NetworkManagerBase::~NetworkManagerBase()";

    closeAllServers();

}

void NetworkManagerBase::closeUDPServer(quint16 port)
{
    QMutexLocker udpLocker(&udpMutex);
    if(!udpServers.isEmpty()) {
        QList<UDPServer *> udpServerList = (port == 0) ? (udpServers.values()) : (udpServers.values(port));
        foreach(UDPServer *udpServer, udpServerList) {
            if(!udpServer) {
                continue;
            }
            //if(udpServer->localPort() == port){
            udpServer->close();
            delete udpServer;
            udpServer = 0;
            //udpServers.remove(port, udpServer);
            //}
        }
        if(port == 0) {
            udpServers.clear();
        } else {
            udpServers.remove(port);
        }

    }

}

void NetworkManagerBase::closeRUDPServer(quint16 port)
{
    QMutexLocker rudpLocker(&rudpMutex);
    if(!rudpServers.isEmpty()) {
        QList<RUDPServer *> rudpServerList = (port == 0) ? (rudpServers.values()) : (rudpServers.values(port));
        foreach(RUDPServer *rudpServer, rudpServerList) {
            if(!rudpServer) {
                continue;
            }
            rudpServer->closeAllChannels();
            rudpServer->closeAllUnusedChannels();

            rudpServer->close();
            delete rudpServer;
            rudpServer = 0;
            //rudpServers.remove(port, rudpServer);
        }
        if(port == 0) {
            rudpServers.clear();
        } else {
            rudpServers.remove(port);
        }

    }

}

void NetworkManagerBase::closeRUDPServerInstance(RUDPServer *rudpServer)
{

    if(!rudpServer) {
        return;
    }
    quint16 port = rudpServer->localPort();

    rudpServer->closeAllChannels();
    rudpServer->closeAllUnusedChannels();

    rudpServer->close();
    delete rudpServer;
    rudpServer = 0;
    rudpServers.remove(port, rudpServer);

}


void NetworkManagerBase::closeAllServers()
{
    qDebug() << "NetworkManagerBase::closeAllServers()";

    QMutexLocker mcLocker(&udpMutex);
    if(!udpServers.isEmpty()) {
        QList<UDPServer *> udpServerList = udpServers.values();
        foreach(UDPServer *udpServer, udpServerList) {
            if(!udpServer) {
                continue;
            }
            udpServer->close();
            delete udpServer;
            udpServer = 0;
        }
        udpServerList.clear();
        udpServers.clear();
    }

    QMutexLocker rudpLocker(&rudpMutex);
    if(!rudpServers.isEmpty()) {
        QList<RUDPServer *> rudpServerList = rudpServers.values();
        foreach(RUDPServer *rudpServer, rudpServerList) {
            if(!rudpServer) {
                continue;
            }
            rudpServer->closeAllChannels();
            rudpServer->closeAllUnusedChannels();
            rudpServer->close();
            delete rudpServer;
            rudpServer = 0;
        }
        rudpServerList.clear();
        rudpServers.clear();
    }



}

UDPServer *NetworkManagerBase::getUDPServer(quint16 port, const QHostAddress &localAddress)
{
    //    qDebug()<<"----NetworkManagerBase::getUDPServer(...)";

    QMutexLocker locker(&udpMutex);

    QHostAddress address = localAddress;
    if(localAddress == QHostAddress::Null) {
        address = QHostAddress::Any;
    }

    QList<UDPServer *> list; // = udpServers.values(port);
    if(port == 0) {
        list = udpServers.values();
    } else {
        list = udpServers.values(port);
    }
    if(!list.isEmpty()) {
        if(address == QHostAddress::Any) {
            return list.at(0);
        }
        foreach(UDPServer *udpServer, list) {
            if((udpServer->localAddress() == address) || (udpServer->localAddress() == QHostAddress::Any)) {
                return udpServer;
            }
        }
    }

    return 0;

}

UDPServer *NetworkManagerBase::startUDPServerListening(const QHostAddress &localAddress, quint16 localPort)
{
    //    qDebug()<< "----NetworkManagerBase::startUDPServerListening(...)-"<<localAddress.toString()<<" Port:"<<localPort;

    m_errorString = "";

    UDPServer *udpServer = getUDPServer(localPort, localAddress);
    if(udpServer) {
        m_errorString = tr("UDP Server has already started!");
        qCritical() << m_errorString;
        return udpServer;
    } else {
        udpServer = new UDPServer(this);
    }

    QHostAddress address = localAddress;
    if(localAddress == QHostAddress::Null) {
        address = QHostAddress::Any;
    }

    if (udpServer->startSimpleListening(address, localPort)) {
        QMutexLocker locker(&udpMutex);
        udpServers.insert(udpServer->localPort(), udpServer);
        return udpServer;
    }

    m_errorString = udpServer->errorString();
    qCritical() << QString("ERROR! Failed to start UDP Server Listening! Local Address:%1, Port:%2. %3 ").arg(localAddress.toString()).arg(localPort).arg(errorString());

    delete udpServer;
    udpServer = 0;


    return udpServer;

}

UDPServer *NetworkManagerBase::startIPMulticastServerListening(const QHostAddress &ipMulticastGroupAddress, quint16 ipMulticastGroupPort)
{
    //    qDebug()<< "----NetworkManagerBase::startIPMulticastServerListening(const QHostAddress &localAddress, quint16 localPort)";

    m_errorString = "";

    UDPServer *udpServer = getUDPServer(ipMulticastGroupPort, QHostAddress::Any);
    if(udpServer) {
        qCritical("IP Multicast Server has already started!");
        return udpServer;
    } else {
        udpServer = new UDPServer(this);

        if (udpServer->startIPMulticastListening(ipMulticastGroupAddress, ipMulticastGroupPort)) {
            QMutexLocker locker(&udpMutex);
            udpServers.insert(ipMulticastGroupPort, udpServer);
            return udpServer;
        } else {
            m_errorString = udpServer->errorString();
            qCritical() << QString("ERROR! Failed to start IPMulticastServer Listening! Multicast Group Address:%1, Port:%2. %3").arg(ipMulticastGroupAddress.toString()).arg(ipMulticastGroupPort).arg(m_errorString);

            delete udpServer;
            udpServer = 0;

            return 0;
        }

    }


    return udpServer;

}

RUDPServer *NetworkManagerBase::getRUDPServer(quint16 port, const QHostAddress &localAddress)
{

    QMutexLocker locker(&rudpMutex);

    QHostAddress address = localAddress;
    if(localAddress == QHostAddress::Null) {
        address = QHostAddress::Any;
    }

    QList<RUDPServer *> list;
    if(port == 0) {
        list = rudpServers.values();
    } else {
        list = rudpServers.values(port);
    }
    if(!list.isEmpty()) {
        if(address == QHostAddress::Any) {
            return list.at(0);
        }
        foreach(RUDPServer *rudpServer, list) {
            if((rudpServer->localAddress() == address) || (rudpServer->localAddress() == QHostAddress::Any)) {
                return rudpServer;
            }
        }
    }

    return 0;

}

RUDPServer *NetworkManagerBase::startRUDPServerListening(const QHostAddress &localAddress, quint16 localPort, int keepAliveTimerInterval)
{

    m_errorString = "";

    RUDPServer *rudpServer = getRUDPServer(localPort, localAddress);
    if(rudpServer) {
        m_errorString = tr("RUDP Server has already started!");
        qCritical() << m_errorString;
        return rudpServer;
    } else {
        rudpServer = new RUDPServer(m_packetHandlerBase, keepAliveTimerInterval, this);
    }

    QHostAddress address = localAddress;
    if(localAddress == QHostAddress::Null) {
        address = QHostAddress::Any;
    }

    if (rudpServer->bind(address, localPort)) {
        QMutexLocker locker(&rudpMutex);
        rudpServers.insert(rudpServer->localPort(), rudpServer);
        return rudpServer;
    }

    m_errorString = rudpServer->errorString();

    qCritical() << QString("ERROR! Failed to start RUDP Server Listening! Local Address:%1, Port:%2. %3").arg(localAddress.toString()).arg(localPort).arg(m_errorString);

    delete rudpServer;
    rudpServer = 0;
    return 0;

}



bool NetworkManagerBase::slotSendNewUDPDatagram(const QHostAddress &targetAddress, quint16 targetPort, QByteArray *data, quint16 localPort, bool useRUDP)
{
    //qDebug()<< "NetworkManagerBase::slotSendNewUDPDatagram(...): Target Address:"<<targetAddress.toString()<<" Target Port:"<<targetPort << " Local Port"<<localPort;

    m_errorString = "";

    bool result = false;

//    RUDPServer *rudpServer = getRUDPServer(localPort, QHostAddress::Any);
//    if(!rudpServer){
//        m_errorString = tr("RUDP Server Not Running On Port %1!").arg(localPort);
//        qCritical()<<m_errorString;
//        return false;
//    }

//    int sentSize = rudpServer->sendDatagram(targetAddress, targetPort, data, useRUDP);
//    result = (sentSize == data->size());
//    m_errorString = rudpServer->errorString();

    if(useRUDP) {
        RUDPServer *rudpServer = getRUDPServer(localPort, QHostAddress::Any);
        if(!rudpServer) {
            m_errorString = tr("RUDP Server Not Running!");
            qCritical() << m_errorString;
            return false;
        }
        if(!rudpServer->isConnected(targetAddress, targetPort)) {
            rudpServer->connectToPeer(targetAddress, targetPort);
            m_errorString = tr("Peer Not Connected!");
            qCritical() << m_errorString;
            return false;
        }
        int sentSize = rudpServer->sendDatagram(targetAddress, targetPort, data);
        result = (sentSize == data->size());
        m_errorString = rudpServer->errorString();

    } else {
        if(localPort == 0) {
            result = UDPSocket::sendUDPDatagramWithAnyPort(targetAddress, targetPort, *data, &m_errorString);
        } else {
            UDPServer *udpServer = getUDPServer(localPort, QHostAddress::Any);
            if (udpServer) {
                qint64 size = udpServer->writeDatagram(*data, targetAddress, targetPort);
                //result = (size == data->size())?true:false;
                result = (size == data->size());
                m_errorString = udpServer->errorString();

            } else {
                result = UDPSocket::sendUDPDatagramWithAnyPort(targetAddress, targetPort, *data, &m_errorString);
            }


        }

        if (result) {
            //qDebug()<< "UDP Datagram Sent Successfully! "<<" Target:"<<targetAddress.toString()<<" Port:"<<targetPort;;
        } else {
            qCritical() << "ERROR! UDP Datagram Sent Failed! " << " Target:" << targetAddress.toString() << " Port:" << targetPort;
        }

    }


    return result;



}

bool NetworkManagerBase::slotSendPacket(Packet *packet)
{

    if (!packet) {
        qCritical() << "ERROR! NULL Packet!";
        return false;
    }

    if (!packet->isValid()) {
        qCritical() << "ERROR! Invalid Packet!";
        delete packet;
        packet = 0;
        return false;
    }


    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    QVariant v;
    v.setValue(*packet);
    out << v;
    qDebug() << "----processOutgoingPackets~~block.size():" << block.size();

    TransmissionProtocol transmissionProtocol = packet->getTransmissionProtocol();

    bool result = false;
    if (transmissionProtocol == TP_TCP) {
        //result = slotSendNewTCPDatagram(QHostAddress(packet->getPeerHostAddress()), packet->getPeerHostPort(), block);
    } else if (transmissionProtocol == TP_UDP) {
        //UDPPacket *udpPacket = static_cast<UDPPacket *> (packet);
        result = slotSendNewUDPDatagram(QHostAddress(packet->getPeerHostAddress()), packet->getPeerHostPort(), &block, packet->getLocalHostPort(), false);

    } else if(transmissionProtocol == TP_RUDP) {
        result = slotSendNewUDPDatagram(QHostAddress(packet->getPeerHostAddress()), packet->getPeerHostPort(), &block, packet->getLocalHostPort(), true);
    }

    return result;

}

void NetworkManagerBase::setPacketHandler(PacketHandlerBase *packetHandlerBase)
{
    this->m_packetHandlerBase = packetHandlerBase;
}

PacketHandlerBase *NetworkManagerBase::getPacketHandler()
{
    return this->m_packetHandlerBase;
}

void NetworkManagerBase::setNetworkType(NetworkType type)
{
    this->networkType = type;

}

NetworkManagerBase::NetworkType NetworkManagerBase::getNetworkType() const
{
    return this->networkType;

}

void NetworkManagerBase::setCommunicationMode(CommunicationMode mode)
{
    this->communicationMode = mode;

}

NetworkManagerBase::CommunicationMode NetworkManagerBase::getCommunicationMode() const
{
    return this->communicationMode;

}













QString NetworkManagerBase::errorString() const
{
    return m_errorString;
}

void NetworkManagerBase::setErrorString(const QString &errorString)
{
    this->m_errorString = errorString;
}






} //namespace HEHUI
