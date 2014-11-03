

//#include <QCoreApplication>

#include <QThreadPool>
#include <QtConcurrentRun>
#include <QDebug>


#include "rudpsocket.h"

#include "rudppacket.h"
#include "rudppacketstreamoperator.h"




namespace HEHUI{


RUDPSocket::RUDPSocket(PacketHandlerBase *packetHandlerBase, int keepAliveTimerInterval, QObject *parent) :
    QUdpSocket(parent), m_packetHandlerBase(packetHandlerBase), m_keepAliveTimerInterval(keepAliveTimerInterval)
{

    Q_ASSERT_X(m_packetHandlerBase, "UDPSocket::UDPSocket(PacketHandlerBase *packetHandlerBase, QObject *parent)", "Invalid PacketHandlerBase!");

    peers.clear();

    datagram = new QByteArray();
    datagram->resize(0);

    connect(this, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

    //注册自定义类型，必须重载“<<”和“>>”, 见"rudppacketstreamoperator.h"
    qRegisterMetaTypeStreamOperators<HEHUI::RUDPPacket>("HEHUI::RUDPPacket");

    //IMPORTANT For Multi-thread
    if(QThreadPool::globalInstance()->maxThreadCount() < MIN_THREAD_COUNT){
        QThreadPool::globalInstance()->setMaxThreadCount(MIN_THREAD_COUNT);
    }


    m_maxCachedUnusedChannelsCount = 10;

}

RUDPSocket::~RUDPSocket(){
    qDebug()<<"--RUDPSocket::~RUDPSocket()";



    closeAllChannels();

    foreach (RUDPChannel *channel, m_unusedRUDPChannels) {
//        channel->quit();
        delete channel;
        channel = 0;
    }
    m_unusedRUDPChannels.clear();

    datagram->clear();
    datagram->resize(0);
    delete datagram;
    datagram = 0;

    RUDPChannel::cleanAllUnusedPackets();


}

bool RUDPSocket::isConnected(const QString &peerAddressString, quint16 peerPort){

    return isConnected(QHostAddress(peerAddressString), peerPort);

}

bool RUDPSocket::isConnected(const QHostAddress &peerAddress, quint16 peerPort){

    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
    return channel->isConnected();

}

void RUDPSocket::connectToPeer(const QString &peerAddressString, quint16 peerPort, bool wait, int msecTimeout){

    connectToPeer(QHostAddress(peerAddressString), peerPort, wait, msecTimeout);
}

void RUDPSocket::connectToPeer(const QHostAddress &peerAddress, quint16 peerPort, bool wait, int msecTimeout){

    if( 0 == localPort()){
        bind();
    }

    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
    channel->connectToPeer(wait, msecTimeout);

}

void RUDPSocket::disconnectFromPeer(const QHostAddress &peerAddress, quint16 peerPort){

    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
    channel->disconnectFromPeer();
}

//bool RUDPSocket::canSendData(quint64 size){
//    return RUDPChannel::canSendData(size);
//}

//quint16 RUDPSocket::beginDataTransmission(const QHostAddress &peerAddress, quint16 peerPort){
//    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
//    return channel->beginDataTransmission();
//}

//bool RUDPSocket::sendData(const QHostAddress &peerAddress, quint16 peerPort, QByteArray &data){

//    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
//    return channel->sendData(data);

//}

//quint64 RUDPSocket::sendDatagram(const QHostAddress &peerAddress, quint16 peerPort, QByteArray *data, quint64 offset, bool fragment){

//    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
//    return channel->sendDatagram(data, offset, fragment);
//}

quint64 RUDPSocket::sendDatagram(const QHostAddress &peerAddress, quint16 peerPort, QByteArray *data, bool isReliableDataPacket){

    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
    return channel->sendDatagram(data, isReliableDataPacket);
}

//void RUDPSocket::endDataTransmission(const QHostAddress &peerAddress, quint16 peerPort, quint16 fragmentDataID){
//    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
//    return channel->endDataTransmission(fragmentDataID);
//}

void RUDPSocket::closeChannel(const QHostAddress &peerAddress, quint16 peerPort){
    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
    channel->closeChannel();
    recyleRUDPChannel(channel);

}

void RUDPSocket::closeAllChannels(){
    qDebug()<<"--RUDPSocket::closeAllChannels()"<<" ThreadId:"<<QThread::currentThreadId();

    if(peers.isEmpty()){return;}

    foreach (RUDPChannel *channel, peers.values()) {
        if(!channel){continue;}
        channel->closeChannel();
        recyleRUDPChannel(channel);
    }
    peers.clear();

}

void RUDPSocket::closeAllUnusedChannels(){
    qDebug()<<"--RUDPSocket::closeAllUnusedChannels()";

    if(m_unusedRUDPChannels.isEmpty()){return;}

    foreach (RUDPChannel *channel, m_unusedRUDPChannels) {
        if(!channel){continue;}
        delete channel;
        channel = 0;
    }
    m_unusedRUDPChannels.clear();

    RUDPChannel::cleanAllUnusedPackets();

}

void RUDPSocket::setMaxCachedUnusedChannelsCount(int count){
    this->m_maxCachedUnusedChannelsCount = count;
}

int RUDPSocket::getMaxCachedUnusedChannelsCount() const{
    return m_maxCachedUnusedChannelsCount;
}

void RUDPSocket::setMaxCachedUnusedPacketsCount(int count){
    RUDPChannel::setMaxCachedUnusedPacketsCount(count);
}

int RUDPSocket::getMaxCachedUnusedPacketsCount() const{
    return RUDPChannel::getMaxCachedUnusedPacketsCount();
}

void RUDPSocket::readPendingDatagrams() {
    //qDebug()<<"----RUDPSocket::readPendingDatagrams()";

    while (hasPendingDatagrams()) {
        datagram->clear();
        qint64 datagramSize = pendingDatagramSize();
        datagram->resize(datagramSize);
        QHostAddress peerAddress;
        quint16 peerPort;

        qint64 readSize = readDatagram(datagram->data(), datagramSize, &peerAddress, &peerPort);
        if(readSize == -1){
            qWarning()<<"Can not read datagram!";
            break;
        }

        //qWarning()<<"Datagram Received From " <<peerAddress.toString()<<" Port:"<<peerPort<<" Size:"<<datagramSize;

        //qDebug()<<"~~datagramSize:"<<datagramSize;

        //                QDataStream in(datagram, QIODevice::ReadOnly);
        //                in.setVersion(QDataStream::Qt_4_6);
        //                QVariant v;
        //                in >> v;
        //                if (v.canConvert<Packet>()){
        //                    //Packet *packet = new Packet();
        //                    RUDPSocket *packet = packetHandlerBase->getPacket();
        //                    *packet = v.value<RUDPSocket>();
        //                    packet->setTransmissionProtocol(TP_UDP);
        //                    packet->setPeerHostAddress(peerAddress);
        //                    packet->setPeerHostPort(peerPort);
        //                    packet->setLocalHostAddress(localAddress());
        //                    packet->setLocalHostPort(localPort());

        //                }


        RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
        //QtConcurrent::run(channel, &RUDPChannel::datagramReceived, *datagram);
        channel->datagramReceived(*datagram);

    }


    datagram->clear();
    datagram->resize(0);

}



void RUDPSocket::channelclosed(const QHostAddress &peerAddress, quint16 peerPort, bool normalClose){
    qDebug()<<"--RUDPSocket::channelclosed()";

    RUDPChannel *channel = getRUDPChannel(peerAddress, peerPort);
    //RUDPChannel *channel = qobject_cast<RUDPChannel *>(sender());
    if(channel){
        QHostAddress address = channel->getPeerHostAddress();
        quint16 port = channel->getPeerHostPort();
        QString channelID = address.toString() + ":" + QString::number(port);
        peers.remove(channelID);
        emit peerDisconnected(address, port, normalClose);

        recyleRUDPChannel(channel);
    }

}

RUDPChannel * RUDPSocket::getRUDPChannel(const QHostAddress &hostAddress, quint16 port){

    QString channelID = hostAddress.toString() + ":" + QString::number(port);

    RUDPChannel *channel = 0;

    if(!peers.contains(channelID)){
        if(m_unusedRUDPChannels.isEmpty()){
            qDebug()<<"Create new channel:"<<channelID;
            channel = new RUDPChannel(this, m_packetHandlerBase, hostAddress, port, m_keepAliveTimerInterval, 0);
//            connect(channel, SIGNAL(finished()), this, SLOT(channelclosed()));
//            connect(channel, SIGNAL(terminated()), this, SLOT(channelclosed()));
            connect(channel, SIGNAL(peerConnected(const QHostAddress &, quint16)), this, SIGNAL(peerConnected(const QHostAddress &, quint16)));
            connect(channel, SIGNAL(signalConnectToPeerTimeout(const QHostAddress &, quint16)), this, SIGNAL(signalConnectToPeerTimeout(const QHostAddress &, quint16)));
//            connect(channel, SIGNAL(peerDisconnected(const QHostAddress &, quint16, bool)), this, SIGNAL(peerDisconnected(const QHostAddress &, quint16, bool)));
            connect(channel, SIGNAL(peerDisconnected(const QHostAddress &, quint16, bool)), this, SLOT(channelclosed(const QHostAddress &, quint16, bool)));

            //connect(channel, SIGNAL(dataReceived(const QHostAddress &, quint16, const QByteArray &)), this, SIGNAL(dataReceived(const QHostAddress &, quint16, const QByteArray &)));

            //channel->start();
            peers.insert(channelID, channel);
        }else{
            qDebug()<<"Use idle channel:"<<channelID;
            channel = m_unusedRUDPChannels.takeFirst();
            //channel->start();
        }

    }else{
//        QList<QHostAddress> addresses = peers.keys();
//        foreach (QHostAddress address, addresses) {
//            RUDPChannel *c = peers.value(address);
//            if(c->getPeerHostPort() == port){
//                channel = c;
//                break;
//            }
//        }

        channel = peers.value(channelID);
    }

    return channel;

}

inline void RUDPSocket::recyleRUDPChannel(RUDPChannel *channel){

    //channel->quit();

    if(m_unusedRUDPChannels.size() >= m_maxCachedUnusedChannelsCount){
        delete channel;
        channel = 0;
        return;
    }

    m_unusedRUDPChannels.append(channel);

}



/////////////////////////////////////////////////////////////////////////////////////

RUDPSocketThread::RUDPSocketThread(PacketHandlerBase *packetHandlerBase, int keepAliveTimerInterval, QObject *parent)
    :QThread(parent)
{

    m_rudpSocket = new RUDPSocket(packetHandlerBase, keepAliveTimerInterval, parent);

    connect(m_rudpSocket, SIGNAL(destroyed()), this, SLOT(quit()));
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));

}

RUDPSocketThread::~RUDPSocketThread() {

}

void RUDPSocketThread::run(){
    exec();
}

RUDPSocket * RUDPSocketThread::getRUDPSocket(){

    return m_rudpSocket;
}




















} //namespace HEHUI
