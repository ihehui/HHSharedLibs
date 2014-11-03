
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>

#include "rudpchannel.h"

//#include "cryptography/cryptography.h"

#include "packethandler/packet.h"


#ifdef Q_CC_MSVC
#include <windows.h>
#endif

#ifdef Q_CC_GNU
#include <unistd.h>
#endif


namespace HEHUI {

qint64 RUDPChannel::m_freeSendBufferSize = 102400;
QMutex * RUDPChannel::m_freeSendBufferSizeMutex = new QMutex();
//quint64 RUDPChannel::m_globalFreeSendBufferSize = 1024000;
QList<RUDPPacket *> * RUDPChannel::m_unusedPackets = new QList<RUDPPacket *>();
QMutex * RUDPChannel::unusedPacketsMutex = new QMutex();
int RUDPChannel::m_maxUnusedPacketsCount = 100;

RUDPChannel::RUDPChannel(QUdpSocket *udpSocket, PacketHandlerBase *packetHandlerBase, int keepAliveTimerInterval, QObject *parent)
    :QObject(parent), m_udpSocket(udpSocket), m_packetHandlerBase(packetHandlerBase), m_keepAliveTimerInterval(keepAliveTimerInterval)
{

    Q_ASSERT_X(m_udpSocket, "RUDPChannel::RUDPChannel(...)", "Invalid UDP Socket!");
    Q_ASSERT_X(m_packetHandlerBase, "UDPSocket::UDPSocket(PacketHandlerBase *packetHandlerBase, QObject *parent)", "Invalid PacketHandlerBase!");


    init();



    sendACKTimer = new QTimer(this);
    connect(sendACKTimer, SIGNAL(timeout()), this, SLOT(sendACKTimerTimeout()));
    //    sendACKTimer = 0;
    //    startSendACKTimer();

    sendNACKTimer = new QTimer(this);
    connect(sendNACKTimer, SIGNAL(timeout()), this, SLOT(sendNACKTimerTimeout()));
    //    sendNACKTimer = 0;
    //    startSendNACKTimer();

    retransmissionTimer = new QTimer(this);
    connect(retransmissionTimer, SIGNAL(timeout()), this, SLOT(retransmissionTimerTimeout()));
    //    retransmissionTimer = 0;
    //    startRetransmissionTimer();

    m_keepAliveTimer = new QTimer(this);
    connect(m_keepAliveTimer, SIGNAL(timeout()), this, SLOT(keepAliveTimerTimeout()));
    //    m_keepAliveTimer = 0;
    //    startKeepAliveTimer();

    m_checkPeerAliveTimer = new QTimer(this);
    connect(m_checkPeerAliveTimer, SIGNAL(timeout()), this, SLOT(checkPeerAliveTimerTimeout()));
    //    m_checkPeerAliveTimer = 0;
    //    startCheckPeerAliveTimer();


    m_connectToPeerTimer = new QTimer(this);
    connect(m_connectToPeerTimer, SIGNAL(timeout()), this, SLOT(connectToPeerTimeout()));


    m_peerAddress = QHostAddress::Null;
    m_peerPort = 0;




}

RUDPChannel::RUDPChannel(QUdpSocket *udpSocket, PacketHandlerBase *packetHandlerBase, const QHostAddress &peerAddress, quint16 peerPort, int keepAliveTimerInterval, QObject *parent)
    :QObject(parent), m_udpSocket(udpSocket), m_packetHandlerBase(packetHandlerBase), m_keepAliveTimerInterval(keepAliveTimerInterval)
{

    Q_ASSERT_X(m_udpSocket, "RUDPChannel::RUDPChannel(...)", "Invalid UDP Socket!");
    Q_ASSERT_X(m_packetHandlerBase, "UDPSocket::UDPSocket(PacketHandlerBase *packetHandlerBase, QObject *parent)", "Invalid PacketHandlerBase!");


    init();


    sendACKTimer = new QTimer(this);
    connect(sendACKTimer, SIGNAL(timeout()), this, SLOT(sendACKTimerTimeout()));
    //    sendACKTimer = 0;
    //    startSendACKTimer();

    sendNACKTimer = new QTimer(this);
    connect(sendNACKTimer, SIGNAL(timeout()), this, SLOT(sendNACKTimerTimeout()));
    //    sendNACKTimer = 0;
    //    startSendNACKTimer();

    retransmissionTimer = new QTimer(this);
    connect(retransmissionTimer, SIGNAL(timeout()), this, SLOT(retransmissionTimerTimeout()));
    //    retransmissionTimer = 0;
    //    startRetransmissionTimer();

    m_keepAliveTimer = new QTimer(this);
    connect(m_keepAliveTimer, SIGNAL(timeout()), this, SLOT(keepAliveTimerTimeout()));
    //    m_keepAliveTimer = 0;
    //    startKeepAliveTimer();

    m_checkPeerAliveTimer = new QTimer(this);
    connect(m_checkPeerAliveTimer, SIGNAL(timeout()), this, SLOT(checkPeerAliveTimerTimeout()));
    //    m_checkPeerAliveTimer = 0;
    //    startCheckPeerAliveTimer();


    m_connectToPeerTimer = new QTimer(this);
    connect(m_connectToPeerTimer, SIGNAL(timeout()), this, SLOT(connectToPeerTimeout()));


    m_peerAddress = peerAddress;
    m_peerPort = peerPort;



}

RUDPChannel::~RUDPChannel(){
    qDebug()<<"--RUDPChannel::~RUDPChannel()"<<" ThreadId:"<<QThread::currentThreadId();

    //    if(m_ChannelState != UnconnectedState){
    //        reset();
    //    }


    if(m_connectToPeerTimer){
        qDebug()<<"--------01----";
        m_connectToPeerTimer->stop();
        delete m_connectToPeerTimer;
        m_connectToPeerTimer = 0;
        qDebug()<<"--------01----";
    }

    //if(sendPacketTimer){
    //    sendPacketTimer->stop();
    //delete sendPacketTimer;
    //sendPacketTimer = 0;
    //}

    if(sendACKTimer){
        qDebug()<<"--------02----";
        sendACKTimer->stop();
        delete sendACKTimer;
        sendACKTimer = 0;
        qDebug()<<"--------02----";
    }

    if(sendNACKTimer){
        qDebug()<<"--------03----";
        sendNACKTimer->stop();
        delete sendNACKTimer;
        sendNACKTimer = 0;
        qDebug()<<"--------03----";
    }

    if(retransmissionTimer){
        qDebug()<<"--------04----";
        retransmissionTimer->stop();
        delete retransmissionTimer;
        retransmissionTimer = 0;
        qDebug()<<"--------04----";
    }

    if(m_keepAliveTimer){
        qDebug()<<"--------05----";
        m_keepAliveTimer->stop();
        delete m_keepAliveTimer;
        m_keepAliveTimer = 0;
        qDebug()<<"--------05----";
    }

    if(m_checkPeerAliveTimer){
        qDebug()<<"--------06----";
        m_checkPeerAliveTimer->stop();
        delete m_checkPeerAliveTimer;
        m_checkPeerAliveTimer = 0;
        qDebug()<<"--------06----";
    }

    //cleanAllUnusedPackets();

}

//void RUDPChannel::run(){
//    exec();
//}

bool RUDPChannel::isConnected(){
    QMutexLocker locker(&m_ChannelStateMutex);
    return (m_ChannelState == ConnectedState);

}

void RUDPChannel::connectToPeer(bool wait, int msecTimeout){
    //qDebug()<<"--RUDPChannel::connectToPeer(...)";

    if(m_peerAddress.isNull() || m_peerPort ==0){
        qCritical()<<"ERROR! Invalid Host Address Or Port!";
        return;
    }

    connectToPeer(m_peerAddress, m_peerPort, wait, msecTimeout);

}

void RUDPChannel::connectToPeer(const QString &peerAddressString, quint16 peerPort, bool wait, int msecTimeout){
    //qDebug()<<"--RUDPChannel::connectToPeer(...)";

    QHostAddress address = QHostAddress(peerAddressString);
    if(address.isNull() || m_peerPort ==0){
        qCritical()<<"ERROR! Invalid Host Address Or Port!";
        return;
    }

    connectToPeer(address, peerPort, wait, msecTimeout);

}

void RUDPChannel::connectToPeer(const QHostAddress &peerAddress, quint16 peerPort, bool wait, int msecTimeout){
    qDebug()<<"--RUDPChannel::connectToPeer(...)";


    if(getChannelState() == ConnectedState){
        return;
    }else{
        updateChannelState(ConnectingState);
    }

    this->m_peerAddress = peerAddress;
    this->m_peerPort = peerPort;

    m_msecConnectToPeerTimeout = msecTimeout;
    //    if(!m_connectToPeerTimer){
    //        m_connectToPeerTimer = new QTimer();
    //        connect(m_connectToPeerTimer, SIGNAL(timeout()), this, SLOT(connectToPeerTimeout()));

    //        //sendResetPacket();
    //    }

    if(!m_connectToPeerTimer->isActive()){
        m_connectToPeerTimer->setInterval(5000);
        QMetaObject::invokeMethod(m_connectToPeerTimer, "start");
        QCoreApplication::processEvents();
        //sendResetPacket();
        sendHandshakePacket(m_myHandshakeID);

    }
    if(wait){
        waitForConnected(msecTimeout);
    }

    //    //if(m_ChannelState == UnconnectedState){
    //        m_ChannelState = ConnectingState;
    //        //sendResetPacket();
    //        sendHandshakePacket(m_myHandshakeID);
    //    //}



    //QTimer::singleShot(msecTimeout, this, SLOT(connectToPeerTimeout()));


    //    ba.clear();
    //    out.device()->seek(0);
    //    QVariant v;
    //    v.setValue(*packet);
    //    out << v;

    //    qint64 size = m_udpSocket->writeDatagram(ba, peerAddress, peerPort);
    //    if(size != ba.size()){
    //        qCritical()<<"ERROR! Failed to send UDP datagram!";
    //    }


}

bool RUDPChannel::waitForConnected(int msecTimeout){

    QDateTime startTime = QDateTime::currentDateTime();

    while (getChannelState() != ConnectedState) {
        QCoreApplication::processEvents();
        msleep(50);
        if(startTime.addMSecs(msecTimeout) < QDateTime::currentDateTime()){
            return false;
        }
    }

    return true;

}

//void RUDPChannel::peerConnected(){
//    qDebug()<<"--RUDPChannel::peerConnected()";

//    if(m_connectToPeerTimer){
//        m_connectToPeerTimer->stop();
//        delete m_connectToPeerTimer;
//        m_connectToPeerTimer = 0;
//    }

//    emit peerConnected(m_peerAddress, m_peerPort);

//}


void RUDPChannel::disconnectFromPeer(){
    //qDebug()<<"--RUDPChannel::disconnectFromPeer()"<<" m_peerPort:"<<m_peerPort<<" localPort:"<<m_udpSocket->localPort();

    if(getChannelState() == UnconnectedState){
        return;
    }

    RUDPPacket *packet = getUnusedPacket();

    packet->setPacketType(quint8(RUDP::Goodbye));
    packet->setPacketSerialNumber(createSerialNumberForControlPacket());
    packet->setRemainingRetransmissionTimes(int(RUDP_PACKET_RETRANSMISSION_TIMES));
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << QString("Goodbye");
    packet->setPacketData(ba);

    tryingToSendPacket(packet);

    //m_ChannelState = DisconnectingState;
    updateChannelState(DisconnectingState);


}

bool RUDPChannel::waitForDisconnected(int msecTimeout){
    //qDebug()<<"--RUDPChannel::waitForDisconnected(...)";

    QDateTime startTime = QDateTime::currentDateTime();

    //NOTICE: Deadlock
    while ( true ) {

        ChannelState state = getChannelState();
        if(state != UnconnectedState){
            QCoreApplication::processEvents();
            msleep(10);
            if(startTime.addMSecs(msecTimeout) < QDateTime::currentDateTime()){
                return false;
            }
        }else{
            return true;
        }

    }

    return true;

}


void RUDPChannel::closeChannel(){
    qDebug()<<"--RUDPChannel::closeChannel()"<<" ThreadId:"<<QThread::currentThreadId()<<" Peer Address:"<<m_peerAddress.toString()<<":"<<m_peerPort;

    //NOTICE: Deadlock
    ChannelState state = getChannelState();
    if(state == ConnectedState){
        disconnectFromPeer();
        //waitForDisconnected();
    }

    QMetaObject::invokeMethod(this, "reset");

    QCoreApplication::sendPostedEvents ();
    //QCoreApplication::processEvents();

    //quit();

}

//bool RUDPChannel::sendData( QByteArray &data){
//    qDebug()<<"--RUDPChannel::sendData(...)";

//    //QCoreApplication::processEvents();

//    int dataSize = data.size();

//    if(dataSize > getGlobalFreeSendBufferSize()){
//        qCritical()<<"ERROR! Can not send data! There is not enough buffer to cache the data! Free buffer size:"<<getGlobalFreeSendBufferSize();
//        return false;
//    }


//    static quint16 fragmentDataID = 0;

//    if(dataSize > m_MSS){
//        if(fragmentDataID == 65535){
//            fragmentDataID = 0;
//        }
//        fragmentDataID++;

//        int fragmentSize = m_MSS-3;
//        int fragmentCount = dataSize/fragmentSize;
//        fragmentCount = (dataSize%fragmentSize)?(fragmentCount+1):fragmentCount;

//        QDataStream in(&data, QIODevice::ReadOnly);
//        in.setVersion(QDataStream::Qt_4_7);

//        for(int i=0; i<fragmentCount; i++){
//            //QCoreApplication::processEvents();

//            QByteArray fragmentData;
//            fragmentData.resize(fragmentSize);

//            //in.readRawData(fragmentData.data(), fragmentSize);
//            qint64 size = in.device()->read(fragmentData.data(), fragmentSize);
//            quint8 isLastFragment = in.atEnd()?quint8(1):quint8(0);
//            if(isLastFragment){
//                fragmentData.resize(size);
//            }
////qDebug()<<"---------------------------------fragmentDataID:"<<fragmentDataID<<" size:"<<size<<" fragmentSize:"<<fragmentSize<<" Checksum:"<<qChecksum(fragmentData.data(), size);

//            RUDPPacket *packet = getUnusedPacket();
//            packet->setPacketType(quint8(RUDP::FragmentDataPacket));
//            packet->setPacketSerialNumber(createSerialNumber());
//            //packet->setRemainingRetransmissionTimes(int(RUDP_PACKET_RETRANSMISSION_TIMES));

//            QByteArray ba;
//            QDataStream out(&ba, QIODevice::WriteOnly);
//            out.setVersion(QDataStream::Qt_4_7);
//            out << fragmentDataID << isLastFragment << fragmentData ;
//            packet->setPacketData(ba);

//            tryingToSendPacket(packet);


//        }



//    }else{

//        RUDPPacket *packet = getUnusedPacket();

//        packet->setPacketType(quint8(RUDP::CompleteDataPacket));
//        packet->setPacketSerialNumber(createSerialNumber());
//        //packet->setRemainingRetransmissionTimes(int(RUDP_PACKET_RETRANSMISSION_TIMES));
//        QByteArray ba;
//        QDataStream out(&ba, QIODevice::WriteOnly);
//        out.setVersion(QDataStream::Qt_4_7);
//        out << data;
//        packet->setPacketData(ba);

//        tryingToSendPacket(packet);

//    }

//    return true;

//}

quint64 RUDPChannel::sendDatagram(QByteArray *data, bool isReliableDataPacket){
    //qDebug()<<"--RUDPChannel::sendDatagram(QByteArray *data) "<<" data->size():"<<data->size();

    Q_ASSERT(data);

    if(!data || (getChannelState() == DisconnectingState) ){
        return 0;
    }

    if(!isReliableDataPacket){
        sendUnreliableDataPacket(data);
        return data->size();
    }

    if(getChannelState() != ConnectedState){
        if(m_ChannelState == DisconnectingState){return 0;}
        connectToPeer();
        waitForConnected();
    }

    int size = data->size();
    int totalSent = 0;

    if(size > m_MSS){
        quint16 fragmentDataID = beginDataTransmission();
        int sent;
        while (totalSent < size)
        {
            QCoreApplication::processEvents();
            if (0 == (sent = sendDatagram(data, totalSent, true)) )
            {
                msleep(1);
            }
            totalSent += sent;
        }

        endDataTransmission(fragmentDataID);
    }else{
        while (totalSent == 0)
        {
            QCoreApplication::processEvents();
            if (0 == (totalSent = sendDatagram(data, 0, false)) )
            {
                msleep(1);
            }

        }

    }
    //qDebug()<<"-----------------------totalSent:"<<totalSent;

    return totalSent;
}

quint64 RUDPChannel::sendDatagram(QByteArray *data, quint64 offset, bool fragment){

    Q_ASSERT(data);

    if(!lostPacketsInSenderSide.isEmpty()) {
        retransmitLostPacket();
        return 0;
    }

    //    if(waitingForACKPackets.size() > 512){
    //        return 0;
    //    }

    //    if(EXPCOUNT > 1){
    //        retransmitLostPacket();
    //    }


    if(!m_ToBeSentPacketsHash.isEmpty()){
        sendToBeSentPackets();
    }

    quint64 freeSendBufferSize = getGlobalFreeSendBufferSize();
    if(freeSendBufferSize < m_MSS){
        return 0;
    }

    quint64 sizeToBeSent = data->size() - offset;
    quint64 sizeSent = 0;

    //qDebug()<<"-----------------------------freeSendBufferSize:"<<freeSendBufferSize<<" offset:"<<offset <<" sizeToBeSent:"<<sizeToBeSent;

    if(fragment){

        int fragmentSize = m_MSS;
        quint64 sizeCanBeSent = (sizeToBeSent>freeSendBufferSize)?freeSendBufferSize:sizeToBeSent;
        int fragmentCount = sizeCanBeSent/fragmentSize;
        fragmentCount = (sizeCanBeSent%fragmentSize)?(fragmentCount+1):fragmentCount;

        QDataStream in(data, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_4_7);
        QIODevice *dev = in.device();
        dev->seek(offset);

        for(int i=0; i<fragmentCount; i++){

            QByteArray fragmentData;
            fragmentData.resize(fragmentSize);

            qint64 size = dev->read(fragmentData.data(), fragmentSize);
            sizeSent += size;

            //quint8 isLastFragment = in.atEnd()?quint8(1):quint8(0);
            quint8 isLastFragment = dev->atEnd()?quint8(1):quint8(0);
            if(isLastFragment){
                fragmentData.resize(size);
            }

            RUDPPacket *packet = getUnusedPacket();
            packet->setPacketType(quint8(RUDP::FragmentDataPacket));
            packet->setPacketSerialNumber(createSerialNumber());

            QByteArray ba;
            QDataStream out(&ba, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_7);
            out << fragmentData ;
            packet->setPacketData(ba);

            tryingToSendPacket(packet);

            m_fragmentCount++;
            //qDebug()<<"----------Fragment Data! SN:"<<packet->getPacketSerialNumber()<<" size:"<<packet->packetDataSize();

        }



    }else{

        RUDPPacket *packet = getUnusedPacket();

        packet->setPacketType(quint8(RUDP::CompleteDataPacket));
        packet->setPacketSerialNumber(createSerialNumber());
        QByteArray ba;
        QDataStream out(&ba, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_7);
        out << *data;
        packet->setPacketData(ba);

        tryingToSendPacket(packet);

        sizeSent = sizeToBeSent;

        //qDebug()<<"----------Full Data! SN:"<<packet->getPacketSerialNumber()<<" size:"<<packet->packetDataSize();

    }

    return sizeSent;

}


void RUDPChannel::datagramReceived(QByteArray &block){
    //qDebug()<<"--RUDPChannel::datagramReceived(...)";

    //QCoreApplication::processEvents();


    RUDPPacket *packet = 0;

    QDataStream inBlock(&block, QIODevice::ReadOnly);
    inBlock.setVersion(QDataStream::Qt_4_7);
    QVariant v;
    inBlock >> v;
    if (v.canConvert<RUDPPacket>()){
        packet = getUnusedPacket();
        *packet = v.value<RUDPPacket>();
    }else{
        qWarning()<<"ERROR! Invalid RUDPPacket!";
        return;
    }

    m_freeReceiveBufferSize -= packet->packetDataSize();
    m_peerLastLiveTime = QDateTime::currentDateTime();


    quint8 packetType = packet->getPacketType();
    quint16 packetSerialNumber = packet->getPacketSerialNumber();


    if(isUnreliablePacket(packetType)){
        processPacket(packet);
        return;
    }else{
        qDebug()<<"--datagramReceived "<<" packetType:"<<packetType<<" packetSerialNumber:"<<packetSerialNumber<<" LRSN:"<<LRSN<<" m_firstReceivedPacketIDInReceiveWindow:"<<m_firstReceivedPacketIDInReceiveWindow<<" m_peerAddress:"<<m_peerAddress.toString()<<":"<<m_peerPort;

        if(getChannelState() != ConnectedState){
            sendResetPacket();
        }
    }

    if(!sendNACKTimer->isActive()){
        sendACKTimerInterval = sendNACKTimerInterval = RUDP_MIN_SEND_ACK_TIMER_INTERVAL;
        QMetaObject::invokeMethod(this, "startSendACKTimer");
        QMetaObject::invokeMethod(this, "startSendNACKTimer");
        QCoreApplication::processEvents();
    }


    if(waitingForACKPackets.isEmpty()){
        EXPCOUNT = 0;
        timeEXPCOUNTReset = m_peerLastLiveTime;
    }else if(packetType == quint8(RUDP::ACK) || packetType == quint8(RUDP::NACK)){
        //retransmissionTimer->start(retransmissionTimerInterval);
    }


    if(packetSerialNumber%16 == 1){
        //ReceivedDataPacketInfo info = receivedDataPacketsHistory.last();
        //probingPacketsInterval.append(m_peerLastLiveTime.msecsTo(info.arrivalTime));

        QDateTime arrivalTime = receivedDataPacketsHistory.last();
        probingPacketsInterval.append(abs(m_peerLastLiveTime.msecsTo(arrivalTime)));
    }

    receivedDataPacketsHistory.removeFirst();
    receivedDataPacketsHistory.append(m_peerLastLiveTime);

    //Check packet whether in window
    //quint32 temp = m_firstReceivedPacketIDInReceiveWindow + m_receiveWindowSize;
    //quint32 temp = LRSN + RUDP_MAX_SEND_WINDOW_SIZE;
    quint32 temp = m_firstReceivedPacketIDInReceiveWindow + RUDP_MAX_SEND_WINDOW_SIZE;
    if( temp > quint16(RUDP_MAX_PACKET_SN)){

        //if(packetSerialNumber < m_firstReceivedPacketIDInReceiveWindow){
        //quint16 rightSideID = m_receiveWindowSize - ((quint16(RUDP_MAX_PACKET_SN) - packetSerialNumber));
        quint16 rightSideID = RUDP_MAX_SEND_WINDOW_SIZE - ((quint16(RUDP_MAX_PACKET_SN) - m_firstReceivedPacketIDInReceiveWindow));

        qDebug()<<"----m_receiveWindowSize:"<<m_receiveWindowSize<<" rightSideID:"<<rightSideID<<" packetSerialNumber:"<<packetSerialNumber;
        if(packetSerialNumber > rightSideID && (packetSerialNumber < m_firstReceivedPacketIDInReceiveWindow) ){
            qDebug()<<"Out of window! m_firstReceivedPacketIDInQueue:"<<m_firstReceivedPacketIDInReceiveWindow<<" m_receiveWindowSize:"<<m_receiveWindowSize<<" SN:"<<packetSerialNumber;
            recylePacket(packet);
            sendPacketDroppedInfo(packetSerialNumber);
            return;
        }else{
            if( (LRSN + 1) > m_firstReceivedPacketIDInReceiveWindow){
                if(packetSerialNumber > 0 && packetSerialNumber <= rightSideID){
                    addLostPacketsInReceiverSide(LRSN, RUDP_MAX_PACKET_SN);
                    addLostPacketsInReceiverSide(0, packetSerialNumber-1);
                    LRSN = packetSerialNumber;
                }else if(LRSN < packetSerialNumber){
                    addLostPacketsInReceiverSide(LRSN, packetSerialNumber-1);
                    LRSN = packetSerialNumber;
                }
            }else if((LRSN + 1) < m_firstReceivedPacketIDInReceiveWindow){
                if(packetSerialNumber > 0 && packetSerialNumber <= rightSideID && LRSN < packetSerialNumber){
                    addLostPacketsInReceiverSide(LRSN, packetSerialNumber-1);
                    LRSN = packetSerialNumber;
                }
            }else{
                LRSN = packetSerialNumber;
            }

        }

        //}


    }else{
        //if(packetSerialNumber < m_firstReceivedPacketIDInReceiveWindow || (packetSerialNumber > temp)){
        if(packetSerialNumber < m_firstReceivedPacketIDInReceiveWindow || packetSerialNumber > temp){
            qDebug()<<"Out of window! m_firstReceivedPacketIDInQueue:"<<m_firstReceivedPacketIDInReceiveWindow<<" m_receiveWindowSize:"<<m_receiveWindowSize<<" SN:"<<packetSerialNumber;
            recylePacket(packet);
            sendPacketDroppedInfo(packetSerialNumber);
            return;
        }else{
            if(LRSN == RUDP_MAX_PACKET_SN){LRSN = 0;}
            if(LRSN < packetSerialNumber){
                if(LRSN >= 1 && ( (LRSN + 1) < packetSerialNumber) ){
                    addLostPacketsInReceiverSide(LRSN, packetSerialNumber-1);
                }
                LRSN = packetSerialNumber;
            }
        }


    }


    removeLostPacketInReceiverSide(packetSerialNumber);

    //qDebug()<<"-----------------------------------lostPacketsSNInReceiverSide:"<<lostPacketsSNInReceiverSide;


    //Cache received packet
    if(m_firstReceivedPacketIDInReceiveWindow != packetSerialNumber){
        m_cachedReceivedPacketsHash.insert(packetSerialNumber, packet);
        qDebug()<<"Received Packet Cached! SN:"<<packetSerialNumber<<" m_firstReceivedPacketIDInQueue:"<<m_firstReceivedPacketIDInReceiveWindow;
        return;
    }


    //Process Packet
    while(m_firstReceivedPacketIDInReceiveWindow == packetSerialNumber){

        processPacket(packet);

        if(m_firstReceivedPacketIDInReceiveWindow == quint16(RUDP_MAX_PACKET_SN)){
            m_firstReceivedPacketIDInReceiveWindow = 1;
        }else{
            m_firstReceivedPacketIDInReceiveWindow++;
        }

        if(m_cachedReceivedPacketsHash.contains(m_firstReceivedPacketIDInReceiveWindow)){
            packetSerialNumber = m_firstReceivedPacketIDInReceiveWindow;
            packet = m_cachedReceivedPacketsHash.take(m_firstReceivedPacketIDInReceiveWindow);
        }else{
            break;
        }

    }



}


void RUDPChannel::connectToPeerTimeout(){
    //qDebug()<<"--RUDPChannel::connectToPeerTimeout()";

    m_msecConnectToPeerTimeout -= m_connectToPeerTimer->interval();

    if(getChannelState() == ConnectedState){
        QMetaObject::invokeMethod(m_connectToPeerTimer, "stop");
        QCoreApplication::processEvents();
        //m_connectToPeerTimer->stop();
        //delete m_connectToPeerTimer;
        //m_connectToPeerTimer = 0;
        m_msecConnectToPeerTimeout = 0;

    }else{
        if(m_msecConnectToPeerTimeout > 0){
            sendHandshakePacket(m_myHandshakeID);
        }else{
            qCritical()<<QString("ERROR! Connection Timeout! Peer Address:%1:%2").arg(m_peerAddress.toString()).arg(m_peerPort);
            QMetaObject::invokeMethod(this, "reset");
            QCoreApplication::processEvents();
            //reset();
            emit signalConnectToPeerTimeout(m_peerAddress, m_peerPort);
        }
    }

}

void RUDPChannel::sendHandshakePacket(uint handshakeID){
    //qDebug()<<"--RUDPChannel::sendHandshakePacket()"<<" handshakeID:"<<handshakeID;


    RUDPPacket *packet = getUnusedPacket();

    packet->setPacketType(quint8(RUDP::Handshake));
    packet->setPacketSerialNumber(createSerialNumberForControlPacket());
    //packet->setRemainingRetransmissionTimes(-1);
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << quint8(RUDP_VERSION) << m_MSS << handshakeID;
    packet->setPacketData(ba);

    //tryingToSendPacket(packet);
    sendPacket(packet);


}

void RUDPChannel::sendResetPacket(){
    qDebug()<<"--RUDPChannel::sendResetPacket()";


    RUDPPacket *packet = getUnusedPacket();

    packet->setPacketType(quint8(RUDP::Reset));
    packet->setPacketSerialNumber(createSerialNumberForControlPacket());
    //packet->setRemainingRetransmissionTimes(-1);
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << quint8(RUDP_VERSION);
    packet->setPacketData(ba);

    //tryingToSendPacket(packet);
    sendPacket(packet);


}

void RUDPChannel::sendPacketDroppedInfo(quint16 packetID){
    //qDebug()<<"--RUDPChannel::sendPacketDroppedInfo(...)"<<"---packetID:"<<packetID;

    RUDPPacket *packet = getUnusedPacket();

    packet->setPacketType(quint8(RUDP::PacketDropped));
    packet->setPacketSerialNumber(packetID);
    //packet->setRemainingRetransmissionTimes(int(0));
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << m_firstReceivedPacketIDInReceiveWindow <<m_receiveWindowSize;
    packet->setPacketData(ba);


    //tryingToSendPacket(packet);

    sendPacket(packet);


    //    ba.clear();
    //    out.device()->seek(0);
    //    QVariant v;
    //    v.setValue(*packet);
    //    out << v;

    //    qint64 size = m_udpSocket->writeDatagram(ba, peerAddress, peerPort);
    //    if(size != ba.size()){
    //        qCritical()<<"ERROR! Failed to send UDP datagram!";
    //    }


}

void RUDPChannel::sendKeepAlivePacket(){
    //qDebug()<<"--RUDPChannel::sendKeepAlivePacket()"<<" Peer:"<<m_peerAddress.toString();

    RUDPPacket *packet = getUnusedPacket();

    packet->setPacketType(quint8(RUDP::KeepAlive));
    packet->setPacketSerialNumber(createSerialNumberForControlPacket());
    packet->setRemainingRetransmissionTimes(int(0));
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    packet->setPacketData(ba);

    //tryingToSendPacket(packet);
    sendPacket(packet);

}

quint16 RUDPChannel::beginDataTransmission(){
    qDebug()<<"--RUDPChannel::beginDataTransmission()";


    static quint16 fragmentDataID = 0;
    if(fragmentDataID == 65535){
        fragmentDataID = 0;
    }
    fragmentDataID++;

    RUDPPacket *packet = getUnusedPacket();

    packet->setPacketType(quint8(RUDP::BeginOrEndDataTransmission));
    packet->setPacketSerialNumber(createSerialNumber());
    packet->setRemainingRetransmissionTimes(int(0));
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << fragmentDataID << quint8(1);
    packet->setPacketData(ba);

    tryingToSendPacket(packet);

    qDebug()<<"------------------------------------DataTransmission Begin--"<<" fragmentDataID:"<<fragmentDataID<<" SN:"<<packet->getPacketSerialNumber();


    return fragmentDataID;

}

void RUDPChannel::endDataTransmission(quint16 fragmentDataID){
    qDebug()<<"--RUDPChannel::endDataTransmission()";

    RUDPPacket *packet = getUnusedPacket();

    packet->setPacketType(quint8(RUDP::BeginOrEndDataTransmission));
    packet->setPacketSerialNumber(createSerialNumber());
    packet->setRemainingRetransmissionTimes(int(0));
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << fragmentDataID << quint8(0) << m_fragmentCount;
    packet->setPacketData(ba);

    tryingToSendPacket(packet);

    qDebug()<<"------------------------------------DataTransmission End--"<<" fragmentDataID:"<<fragmentDataID<<" m_fragmentCount"<<m_fragmentCount<<" SN:"<<packet->getPacketSerialNumber();

    m_fragmentCount = 0;

}

void RUDPChannel::sendUnreliableDataPacket(QByteArray *data){

    Q_ASSERT(data);
    if(!data){return;}

    RUDPPacket *packet = getUnusedPacket();

    packet->setPacketType(quint8(RUDP::UnreliableDataPacket));
    packet->setPacketSerialNumber(createSerialNumberForControlPacket());
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << *data;
    packet->setPacketData(ba);

    tryingToSendPacket(packet);

}

bool RUDPChannel::isUnreliablePacket(quint8 packetType){
    //RUDPPacketType type = RUDPPacketType(packetType);
    switch(packetType){
    case quint8(RUDP::BeginOrEndDataTransmission):
    case quint8(RUDP::CompleteDataPacket):
    case quint8(RUDP::FragmentDataPacket):
    case quint8(RUDP::DataStreamPacket):
        return false;
        break;

    default:
        return true;

    }

    return true;

}

void RUDPChannel::sendToBeSentPackets(){

    //qDebug()<<"--RUDPChannel::sendToBeSentPackets()  m_ToBeSentPacketsHash.size():"<<m_ToBeSentPacketsHash.size();

    if(m_ToBeSentPacketsHash.isEmpty()){
        return;
    }

    //////////////////////////////////////////////////////


    int temp = m_firstWaitingForACKPacketIDInSendWindow + m_sendWindowSize;
    //qDebug()<<"-------------------m_firstWaitingForACKPacketIDInSendWindow:"<<m_firstWaitingForACKPacketIDInSendWindow<<" m_sendWindowSize:"<<m_sendWindowSize;
    if(temp > RUDP_MAX_PACKET_SN){
        for(int i=m_firstWaitingForACKPacketIDInSendWindow; i<=RUDP_MAX_PACKET_SN; i++){

            if(m_ToBeSentPacketsHash.contains(i)){
                //qDebug()<<"-----------1-------------------m_ToBeSentPacketsHash.contains:"<<i;

                RUDPPacket *packet = takeToBeSentPacket(i);
                sendPacket(packet);

            }

        }

        quint16 rightSideID = m_sendWindowSize - (quint16(RUDP_MAX_PACKET_SN) - m_firstWaitingForACKPacketIDInSendWindow);
        for(int i=1; i<=rightSideID; i++){

            if(m_ToBeSentPacketsHash.contains(i)){
                //qDebug()<<"-----------1-------------------m_ToBeSentPacketsHash.contains:"<<i;

                RUDPPacket *packet = takeToBeSentPacket(i);
                sendPacket(packet);

            }

        }


    }else{
        for(int i=m_firstWaitingForACKPacketIDInSendWindow; i<=temp; i++){

            if(m_ToBeSentPacketsHash.contains(i)){
                //qDebug()<<"-----------1-------------------m_ToBeSentPacketsHash.contains:"<<i;

                RUDPPacket *packet = takeToBeSentPacket(i);
                sendPacket(packet);

            }

        }

    }



    ////////////////////////////////////




}

bool RUDPChannel::tryingToSendPacket(RUDPPacket *packet){
    //qDebug()<<"--RUDPChannel::tryingToSendPacket(...)";

    //retransmitLostPacket();


    Q_ASSERT(packet);

    static int packetsSent = 0;


    if(!packet){
        qCritical()<<"ERROR! Invalid RUDPPacket!";
        return false;
    }
    if(packet->isNull()){
        qCritical()<<"ERROR! Invalid RUDPPacket!";
        recylePacket(packet);
        return false;
    }

    updateGlobalFreeSendBufferSize(packet->packetDataSize());

    quint8 packetType = packet->getPacketType();
    quint16 sn = packet->getPacketSerialNumber();





    ///////////////////////////////////////////////////


    quint32 temp = m_firstWaitingForACKPacketIDInSendWindow + m_sendWindowSize;
    if( temp > quint16(RUDP_MAX_PACKET_SN)){
        quint16 rightSideID = m_sendWindowSize - (quint16(RUDP_MAX_PACKET_SN) - m_firstWaitingForACKPacketIDInSendWindow);
        if(sn>rightSideID &&(sn<m_firstWaitingForACKPacketIDInSendWindow)){
            addToBeSentPacket(packet);
            qDebug()<<"Can not send packet! Out of window! Packet cached! sn:"<<sn<<" m_sendWindowSize:"<<m_sendWindowSize<<" m_firstWaitingForACKPacketIDInSendWindow:"<<m_firstWaitingForACKPacketIDInSendWindow;
            return false;
        }


    }else{
        if(sn < m_firstWaitingForACKPacketIDInSendWindow || (sn > temp)){
            addToBeSentPacket(packet);
            qDebug()<<"Can not send packet! Out of window! Packet cached! sn:"<<sn<<" m_sendWindowSize:"<<m_sendWindowSize<<" m_firstWaitingForACKPacketIDInSendWindow:"<<m_firstWaitingForACKPacketIDInSendWindow;
            return false;
        }

    }


    ///////////////////////////////////////////////////


    sendPacket(packet);

    packetsSent++;

    if(!isUnreliablePacket(packetType)){
        qDebug()<<"OK! RUDPPacket Sent! sn:"<<sn<<" packetType:"<<packetType<<" packetDataSize:"<<packet->packetDataSize()<<" m_sendWindowSize:"<<m_sendWindowSize<<" m_firstWaitingForACKPacketIDInQueue:"<<m_firstWaitingForACKPacketIDInSendWindow<<" m_peerPort:"<<m_peerPort;
        //    qWarning()<<"--------------Total Packets Sent:"<<packetsSent;

    }




    return true;


}

void RUDPChannel::startKeepAliveTimer(){
    qDebug()<<"--RUDPChannel::startKeepAliveTimer()";

    if(!m_keepAliveTimer){
        m_keepAliveTimer = new QTimer();
        connect(m_keepAliveTimer, SIGNAL(timeout()), this, SLOT(keepAliveTimerTimeout()));
    }
    m_keepAliveTimer->start(m_keepAliveTimerInterval - 2 * RTT);

    //qDebug()<<"------------------------keepAliveTimerInterval:"<<m_keepAliveTimer->interval();

}

void RUDPChannel::keepAliveTimerTimeout(){
    //qDebug()<<"--RUDPChannel::keepAliveTimerTimeout()";

    //if(m_lastPacketSentTime.addMSecs(m_keepAliveTimerInterval+RTT) <= QDateTime::currentDateTime()){
    sendKeepAlivePacket();
    //}

}

void RUDPChannel::startCheckPeerAliveTimer(){
    qDebug()<<"--RUDPChannel::startCheckPeerAliveTimer()";

    if(!m_checkPeerAliveTimer){
        m_checkPeerAliveTimer = new QTimer();
        connect(m_checkPeerAliveTimer, SIGNAL(timeout()), this, SLOT(checkPeerAliveTimerTimeout()));
    }
    m_checkPeerAliveTimer->start(m_keepAliveTimerInterval + 2 * RTT);
    //qDebug()<<"------------------------checkPeerAliveTimerInterval:"<<m_checkPeerAliveTimer->interval();

}

void RUDPChannel::checkPeerAliveTimerTimeout(){
    //qDebug()<<"--RUDPChannel::checkPeerAliveTimerTimeout()";

    if(m_peerLastLiveTime.addMSecs(m_keepAliveTimerInterval) <= QDateTime::currentDateTime()){
        m_checkPeerAliveTimes--;
    }else{
        m_checkPeerAliveTimes = RUDP_MAX_CHECK_ALIVE_TIMES;
    }

    if(m_checkPeerAliveTimes < 1){
        closeChannel();
        //QMetaObject::invokeMethod(this, "reset");
        //reset();
        qDebug()<<"Error! Peer Offline!";
        emit peerDisconnected(m_peerAddress, m_peerPort, false);
    }

    //qDebug()<<"------m_peerLastLiveTime:"<<m_peerLastLiveTime.toString("hh:mm:ss:zzz")<<" m_keepAliveTimerInterval:"<<m_keepAliveTimerInterval;


}

void RUDPChannel::sendACK2(quint16 packetSN){
    //qDebug()<<"--RUDPChannel::sendACK2()";


    RUDPPacket *packet = getUnusedPacket();

    packet->setPacketType(quint8(RUDP::ACK2));
    packet->setPacketSerialNumber(packetSN);
    //packet->setRemainingRetransmissionTimes(int(0));
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    //out << RTT << RTTVar << freeReceiveBufferSize;

    packet->setPacketData(ba);

    //tryingToSendPacket(packet);
    sendPacket(packet);


    //    ba.clear();
    //    out.device()->seek(0);
    //    QVariant v;
    //    v.setValue(*packet);
    //    out << v;

    //    qint64 size = m_udpSocket->writeDatagram(ba, peerAddress, peerPort);
    //    if(size != ba.size()){
    //        qCritical()<<"ERROR! Failed to send UDP datagram!";
    //    }


}

bool RUDPChannel::retransmitLostPacket(){
    //qDebug()<<"--RUDPChannel::retransmitLostPacket(...)--SN:"<<packet->getPacketSerialNumber();


    //TODO:Disconnect

    static int count = 0;

    if(lostPacketsInSenderSide.isEmpty()){
        return false;
    }

    if(m_sendWindowSize > RUDP_SLOWSTART_THRESHOLD){
        m_sendWindowSize = m_sendWindowSize/2;
        //m_sendWindowSize -= (m_sendWindowSize/10);
    }
    if(m_sendWindowSize > RUDP_MIN_SEND_WINDOW_SIZE){
        //m_sendWindowSize -= (m_sendWindowSize/10);
        m_sendWindowSize --;
    }
    if(m_sendWindowSize < RUDP_MIN_SEND_WINDOW_SIZE){
        m_sendWindowSize = RUDP_MIN_SEND_WINDOW_SIZE;
    }

    quint16 sn = lostPacketsInSenderSide.takeFirst();
    qDebug()<<"---------------1------------sn:"<<sn;
    lostPacketsInSenderSide.removeAll(sn);

    RUDPPacket *packet = sentPackets.value(sn);
    if(!packet){return false;}
 qDebug()<<"---------------2------------sn:"<<packet->getPacketSerialNumber();

    QDateTime curTime = QDateTime::currentDateTime();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    QVariant v;
    v.setValue(*packet);
    out << v;

    qint64 size = m_udpSocket->writeDatagram(block, m_peerAddress, m_peerPort);
    packet->setLastTransmissionTime(curTime);

    if(size != block.size()){
        qCritical()<<"ERROR! Failed to send RUDPPacket!";
        lostPacketsInSenderSide.prepend(sn);
        return false;
    }


    qDebug()<<"---------------3------------sn:"<<packet->getPacketSerialNumber();

    count++;
    qDebug()<<"--------------Resend lost packet: sn:"<<packet->getPacketSerialNumber()<<" packetType:"<<packet->getPacketType()<<" m_sendWindowSize:"<<m_sendWindowSize<<" m_firstWaitingForACKPacketIDInQueue:"<<m_firstWaitingForACKPacketIDInSendWindow;
    qWarning()<<"--------------Total Resent Packets:"<<count;


    qint64 interval = abs(QDateTime::currentDateTime().msecsTo(curTime));
    qint64 timeWait = sendPacketInterval - interval;
    if(timeWait > 0){
        msleep(timeWait);
    }


    return true;

}

bool RUDPChannel::sendPacket(RUDPPacket *packet){
    //qDebug()<<"--RUDPChannel::sendPacket(...)--Type:"<<packet->getPacketType()<<" SN:"<<packet->getPacketSerialNumber();

    Q_ASSERT_X(packet, "RUDPChannel::sendPacket(...)", "Invalid Packet!");

    QDateTime curTime = QDateTime::currentDateTime();


    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    QVariant v;
    v.setValue(*packet);
    out << v;

    qint64 size = m_udpSocket->writeDatagram(block, m_peerAddress, m_peerPort);
    packet->setLastTransmissionTime(curTime);

    if(size != block.size()){
        qCritical()<<"ERROR! Failed to send RUDPPacket!";
        return false;
    }


    if(!isUnreliablePacket(packet->getPacketType())){
        addWaitingForACKPacket(packet);
        LSSN = packet->getPacketSerialNumber();
    }


    //    quint16 sn = packet->getPacketSerialNumber();
    //    sentPackets.insert(sn, packet);
    //    waitingForACKPackets.append(sn);

    qint64 interval = abs(QDateTime::currentDateTime().msecsTo(curTime));
    qint64 timeWait = sendPacketInterval - interval;
    if(timeWait > 0){
        msleep(timeWait);
    }

    return true;

}

void RUDPChannel::startSendACKTimer(){
    qDebug()<<"--RUDPChannel::startSendACKTimer()"<<" sendACKTimerInterval:"<<sendACKTimerInterval<<" "<<m_peerAddress.toString();

    if(!sendACKTimer){
        sendACKTimer = new QTimer();
        //sendACKTimer->setSingleShot(true);
        connect(sendACKTimer, SIGNAL(timeout()), this, SLOT(sendACKTimerTimeout()));
    }
    if(sendACKTimerInterval < RUDP_MIN_SEND_ACK_TIMER_INTERVAL){
        sendACKTimerInterval = RUDP_MIN_SEND_ACK_TIMER_INTERVAL;
    }
    sendACKTimer->start(sendACKTimerInterval);

}

void RUDPChannel::stopSendACKTimer(){
    qDebug()<<"--RUDPChannel::stopSendACKTimer()"<<" "<<m_peerAddress.toString();
    if(!sendACKTimer){return;}
    sendACKTimer->stop();

}

void RUDPChannel::sendACKTimerTimeout(){

    //qDebug()<<"--RUDPChannel::sendACKTimerTimeout()"<<" sendACKTimerInterval:"<<sendACKTimerInterval;

    //    quint16 sn = 0;
    //    if(lostPacketsInReceiverSide.isEmpty()){
    //        sn = LRSN + 1;
    //    }else{
    //        sn = lostPacketsSNInReceiverSide.first();
    //    }

    //    if(sn == largestACK2SN){
    //        return;
    //    }
    //    if(ackPacketsSNHistory.size() && sn == ackPacketsSNHistory.last()){
    //        ACKPacketInfo * info = ackPacketsHistory.value(sn);
    //        QDateTime sentTime = info->sentTime;
    //        if(QDateTime::currentDateTime().msecsTo(sentTime) < 2 * RTT){
    //            return;
    //        }
    //    }

    static int count = 0;
    static quint16 lastLRSN = 0;

    if(!ackPacketsSNHistory.isEmpty() ){
        //if(!ackPacketsSNHistory.isEmpty() && largestACK2SN == ackPacketsSNHistory.last() ){
        //ACKPacketInfo * info = ackPacketsHistory.value(largestACK2SN);
        ACKPacketInfo * info = ackPacketsHistory.value(ackPacketsSNHistory.last());
        quint16 sn = info->firstReceivedPacketIDInReceiveWindow;
        qDebug()<<"------------------"<<" ackPacketsSNHistory.last():"<<ackPacketsSNHistory.last()<<" largestACK2SN:"<<largestACK2SN<<" sn:"<<sn <<" m_firstReceivedPacketIDInReceiveWindow:"<<m_firstReceivedPacketIDInReceiveWindow<<" LRSN:"<<LRSN <<" m_peerAddress:"<<m_peerAddress.toString()<<":"<<m_peerPort;
        if(sn == m_firstReceivedPacketIDInReceiveWindow && ( (LRSN + 1) == m_firstReceivedPacketIDInReceiveWindow || (LRSN == RUDP_MAX_PACKET_SN && m_firstReceivedPacketIDInReceiveWindow == 1) || lastLRSN == LRSN ) ){
            count++;
            if(count >= 10){
                //sendACKTimerInterval = sendNACKTimerInterval = RUDP_MAX_SEND_ACK_TIMER_INTERVAL;
                //QMetaObject::invokeMethod(this, "startSendACKTimer");
                //QMetaObject::invokeMethod(this, "startSendNACKTimer");
                QMetaObject::invokeMethod(this, "stopSendACKTimer");
                QMetaObject::invokeMethod(this, "stopSendNACKTimer");
                QCoreApplication::processEvents();
            }
            qDebug()<<"-----------1---------- No ACK Sent!";
            return;
        }
    }

    QDateTime time = receivedDataPacketsHistory.last();
    if(time.isNull()){
        count++;
        if(count >= 10){
            sendACKTimerInterval = sendNACKTimerInterval = RUDP_MAX_SEND_ACK_TIMER_INTERVAL;
            //QMetaObject::invokeMethod(this, "startSendACKTimer");
            //QMetaObject::invokeMethod(this, "startSendNACKTimer");
            QMetaObject::invokeMethod(this, "stopSendACKTimer");
            QMetaObject::invokeMethod(this, "stopSendNACKTimer");
            QCoreApplication::processEvents();
        }
        qDebug()<<"-----------2---------- No ACK Sent!";
        return;
    }
    //        if((abs(QDateTime::currentDateTime().msecsTo(time)) > 2 * sendACKTimerInterval && ( (LRSN + 1) == m_firstReceivedPacketIDInReceiveWindow)) || (LRSN == RUDP_MAX_PACKET_SN && m_firstReceivedPacketIDInReceiveWindow == 1) ){
    //            return;
    //        }

    //qDebug()<<"-------------receivedDataPacketsHistory.last():"<<receivedDataPacketsHistory.last().toString("mm:ss:zzz")<<" LRSN:"<<LRSN<<" m_firstReceivedPacketIDInReceiveWindow:"<<m_firstReceivedPacketIDInReceiveWindow;



    count = 0;

    //Calculate the packet arrival speed
    int AI = 0;
    int packetArrivalSpeed = 0; //number of packets per second

    QDateTime time1, time2;
    QList<qint64> intervals;
    for(int i=1; i<17; i++){
        time1 = receivedDataPacketsHistory.at(i - 1);
        time2 = receivedDataPacketsHistory.at(i);
        if(time2 < time1){break;}
        intervals.append(abs(time2.msecsTo(time1)));
    }
    int intervalsSize = intervals.size();
    if(intervalsSize % 2 == 0){
        AI = (intervals.at(intervalsSize/2)+ intervals.at(intervalsSize/2+1))/2;
    }else{
        AI = intervals.at(intervalsSize/2+1);
    }

    QList<qint64> intervals2 = intervals;
    for(int i=0; i<16; i++){
        qint64 interval = intervals.at(i);
        if(interval > AI*8 || interval < AI/8){
            intervals2.removeAt(i);
        }
    }
    int intervals2Size = intervals2.size();
    if(intervals2Size >= 8){
        qint64 total = 0;
        foreach (qint64 interval, intervals2) {
            total += interval;
        }
        AI = total/intervals2Size;
        if(AI > 0){
            packetArrivalSpeed = 1/AI;
        }


    }

    //Calculate the estimated link capacity
    int PI = (probingPacketsInterval.at(8) + probingPacketsInterval.at(9))/2;
    int linkCapacity = 0; //number of packets per second
    if(PI > 0){
        linkCapacity = 1/PI;
    }


    //Send ACK
    RUDPPacket *packet = getUnusedPacket();
    quint16 packetSerialNumber = createSerialNumberForControlPacket();
    //quint16 packetSerialNumber = createSerialNumber();


    packet->setPacketType(quint8(RUDP::ACK));
    packet->setPacketSerialNumber(packetSerialNumber);
    packet->setRemainingRetransmissionTimes(int(0));
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << m_firstReceivedPacketIDInReceiveWindow << RTT << RTTVar << m_freeReceiveBufferSize << packetArrivalSpeed << linkCapacity;

    packet->setPacketData(ba);

    sendPacket(packet);

    qDebug()<<"------------ ACK Sent! -------------m_firstReceivedPacketIDInReceiveWindow:"<<m_firstReceivedPacketIDInReceiveWindow<<" m_peerAddress:"<<m_peerAddress.toString();

    //Record the ACK sequence number and the departure time
    ACKPacketInfo * info = new ACKPacketInfo();
    info->firstReceivedPacketIDInReceiveWindow = m_firstReceivedPacketIDInReceiveWindow;
    info->sentTime = QDateTime::currentDateTime();
    ackPacketsHistory.insert(packetSerialNumber, info);
    ackPacketsSNHistory.append(packetSerialNumber);

    lastLRSN = LRSN;

    if(sendACKTimer->interval() != sendACKTimerInterval){
        //sendACKTimer->stop();
        //startSendACKTimer();
        QMetaObject::invokeMethod(this, "startSendACKTimer");
        QCoreApplication::processEvents();
        qDebug()<<"--Restart SendACK Timer! sendACKTimerInterval:"<< sendACKTimerInterval;
    }


}

void RUDPChannel::startSendNACKTimer(){

    qDebug()<<"--RUDPChannel::startSendNACKTimer()"<<" "<<m_peerAddress.toString();

    if(!sendNACKTimer){
        sendNACKTimer = new QTimer();
        //sendNACKTimer->setSingleShot(true);
        connect(sendNACKTimer, SIGNAL(timeout()), this, SLOT(sendNACKTimerTimeout()));
    }
    sendNACKTimer->start(sendNACKTimerInterval);

}

void RUDPChannel::stopSendNACKTimer(){
    qDebug()<<"--RUDPChannel::stopSendNACKTimer()"<<" "<<m_peerAddress.toString();
    if(!sendNACKTimer){return;}
    sendNACKTimer->stop();

}

void RUDPChannel::sendNACKTimerTimeout(){
    //qDebug()<<"--RUDPChannel::sendNACKTimerTimeout()";

    if(lostPacketsInReceiverSide.isEmpty()){
        return;
    }

    QDateTime curTime = QDateTime::currentDateTime();
    QList<quint16> packetsSN;

    foreach (quint16 sn, lostPacketsSNInReceiverSide) {
        LostPacketInfo *lostPacketInfo = lostPacketsInReceiverSide.value(sn);
        Q_ASSERT_X(lostPacketInfo, "RUDPChannel::sendNACKTimerTimeout()", "Invalid LostPacketInfo!");
        int feedbacktimes = lostPacketInfo->feedbacktimes;
        QDateTime latestFeedbackTime = lostPacketInfo->latestFeedbackTime;
        if(latestFeedbackTime.addMSecs(feedbacktimes * RTT) < curTime){
            packetsSN.append(lostPacketInfo->packetSN);
        }

    }


    //qDebug()<<"----------------------m_firstReceivedPacketIDInReceiveWindow:"<<m_firstReceivedPacketIDInReceiveWindow;
    qDebug()<<"----------------------packetsSN:"<<packetsSN<<" "<<m_peerAddress.toString() ;


    //Send NACK
    RUDPPacket *packet = getUnusedPacket();
    quint16 packetSerialNumber = createSerialNumberForControlPacket();

    packet->setPacketType(quint8(RUDP::NACK));
    packet->setPacketSerialNumber(packetSerialNumber);
    packet->setRemainingRetransmissionTimes(int(0));
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    while (!packetsSN.isEmpty() && ba.size() < (m_MSS -3) ) {
        packLostPacket(&packetsSN, &out);
        QCoreApplication::processEvents();
    }

    packet->setPacketData(ba);

    sendPacket(packet);

    qDebug()<<"-------------- NACK Sent! -------------"<<m_peerAddress.toString();


    if(sendNACKTimer->interval() != sendNACKTimerInterval){
        //sendNACKTimer->stop();
        //startSendNACKTimer();
        QMetaObject::invokeMethod(this, "startSendNACKTimer");
        QCoreApplication::processEvents();
    }


}

void RUDPChannel::packLostPacket(QList<quint16> *packets, QDataStream *out){
    //qDebug()<<"--RUDPChannel::packLostPacket(...)";

    Q_ASSERT(packets);
    Q_ASSERT(out);

    if(packets->isEmpty()){return;}

    quint16 sn0 = packets->takeFirst();
    packets->removeAll(sn0);

    quint16 sn = sn0;
    *out << sn;
    while (sn < RUDP_MAX_PACKET_SN && packets->contains(sn+1)) {
        sn++;
        packets->removeAll(sn);
        QCoreApplication::processEvents();
    }
    if(sn > sn0 ){
        *out << QChar('-') << sn;
    }else{
        *out << QChar(',');
    }


}

void RUDPChannel::addLostPacketsInReceiverSide(quint16 start, quint16 end){
    qDebug()<<"--RUDPChannel::addLostPacketsInReceiverSide(...)--"<<" start"<<start<<" end"<<end;

    if(end - start < 1){
        return;
    }

    for(int i=start+1; i<=end; i++) {
        QCoreApplication::processEvents();

        LostPacketInfo *lostPacketInfo = lostPacketsInReceiverSide.value(i);
        if(!lostPacketInfo){
            lostPacketInfo = new LostPacketInfo();
            lostPacketInfo->feedbacktimes = 2;
            lostPacketsSNInReceiverSide.append(i);
            lostPacketsInReceiverSide.insert(i, lostPacketInfo);
        }
        lostPacketInfo->packetSN = i;
        lostPacketInfo->feedbacktimes++;
        lostPacketInfo->latestFeedbackTime = m_peerLastLiveTime;

    }

}

void RUDPChannel::removeLostPacketInReceiverSide(quint16 packetSN){
    //qDebug()<<"--RUDPChannel::removeLostPacketInReceiverSide(...)--"<<" packetSN:"<<packetSN;

    lostPacketsSNInReceiverSide.removeAll(packetSN);

    LostPacketInfo *lostPacketInfo = lostPacketsInReceiverSide.take(packetSN);
    delete lostPacketInfo;
    lostPacketInfo = 0;

}

void RUDPChannel::startRetransmissionTimer(){
    //qDebug()<<"--RUDPChannel::startRetransmissionTimer()";

    if(!retransmissionTimer){
        retransmissionTimer = new QTimer();
        //retransmissionTimer->setSingleShot(true);
        connect(retransmissionTimer, SIGNAL(timeout()), this, SLOT(retransmissionTimerTimeout()));
    }
    retransmissionTimer->start(retransmissionTimerInterval);

}

void RUDPChannel::retransmissionTimerTimeout(){
    //qDebug()<<"--RUDPChannel::retransmissionTimerTimeout()";

    //    if(retransmissionTimer->interval() != retransmissionTimerInterval){
    //        startRetransmissionTimer();
    //    }

    QDateTime curTime = QDateTime::currentDateTime();


    for(int i=waitingForACKPackets.size()-1; i>=0; i--){
        quint16 sn = waitingForACKPackets.at(i);
        lostPacketsInSenderSide.removeAll(sn);
        lostPacketsInSenderSide.prepend(sn);
    }

    //    if(!lostPacketsInSenderSide.isEmpty()){
    //        qDebug()<<"------------------------lostPacketsInSenderSide:"<<lostPacketsInSenderSide.size()<<" "<<lostPacketsInSenderSide.first()<<"~"<<lostPacketsInSenderSide.last();
    //    }
    //    if(!waitingForACKPackets.isEmpty()){
    //        qDebug()<<"------------------------waitingForACKPackets:"<<waitingForACKPackets.size()<<" "<<waitingForACKPackets.first()<<"~"<<waitingForACKPackets.last();
    //    }
    if(lostPacketsInSenderSide.isEmpty()){
        EXPCOUNT = 0;
        timeEXPCOUNTReset = curTime;

        ////////////////////////////////
        if(!m_ToBeSentPacketsHash.isEmpty()){
            QTimer::singleShot(1, this, SLOT(sendToBeSentPackets()));
        }
        ////////////////////////////////

        return;
    }

    //TODO
    //    if( (EXPCOUNT > 16 && timeEXPCOUNTReset.addSecs(3) < curTime) || timeEXPCOUNTReset.addSecs(180) < curTime){
    //        qDebug()<<"------------------------------------0--"<<"EXPCOUNT:"<<EXPCOUNT<<" timeEXPCOUNTReset:"<<timeEXPCOUNTReset<<" curTime:"<<curTime;
    //        disconnectFromPeer();
    //        return;
    //    }

    //    if(lostPacketsInSenderSide.isEmpty()){
    //        sendKeepAlivePacket();
    //    }

    EXPCOUNT++;

    if(EXPCOUNT > 4){
        retransmitLostPacket();
    }


}


void RUDPChannel::init(){

    QDateTime curTime = QDateTime::currentDateTime();;

    m_MSS = RUDP_ETHERNET_MSS;

    SYN = 10;
    RTT = 100;
    RTTVar = 100;



    //------发送端------
    //sendPacketTimer = new QTimer();
    //sendPacketTimer = 0;
    sendPacketInterval = 2;
    LSSN = 0;

    m_ToBeSentPacketsHash.clear();
    sentPackets.clear();
    lostPacketsInSenderSide.clear();
    waitingForACKPackets.clear();

    //-----------------


    //------接收端------
    maxReceiveBufferSize = 10240000;
    m_freeReceiveBufferSize = 10240000;
    m_receiveWindowSize = m_freeReceiveBufferSize / m_MSS;
    LRSN = 0;

    //sendACKTimer = new QTimer(this);
    //connect(sendACKTimer, SIGNAL(timeout()), this, SLOT(sendACKTimerTimeout()));
    sendACKTimerInterval = RUDP_MIN_SEND_ACK_TIMER_INTERVAL;
    //sendACKTimer = 0;
    //startSendACKTimer();

    //sendNACKTimer = new QTimer(this);
    //connect(sendNACKTimer, SIGNAL(timeout()), this, SLOT(sendNACKTimerTimeout()));
    sendNACKTimerInterval = 3 * RTT;
    //sendNACKTimer = 0;
    //startSendNACKTimer();

    m_packetArrivalSpeed = 0;
    m_linkCapacity = 0;

    //retransmissionTimer = new QTimer(this);
    //connect(retransmissionTimer, SIGNAL(timeout()), this, SLOT(retransmissionTimerTimeout()));
    retransmissionTimerInterval = 3 * RTT + SYN;
    //retransmissionTimer = 0;
    //startRetransmissionTimer();

    EXPCOUNT = 0;
    timeEXPCOUNTReset = curTime;

    lostPacketsInReceiverSide.clear();
    lostPacketsSNInReceiverSide.clear();

    ackPacketsHistory.clear();
    ackPacketsSNHistory.clear();
    largestACK2SN = 0;

    receivedDataPacketsHistory.clear();
    for(int i=0; i<17; i++){
        receivedDataPacketsHistory.append(QDateTime());
    }

    probingPacketsInterval.clear();
    for(int i=0; i<16; i++){
        probingPacketsInterval.append(0);
    }

    //-----------------




    m_myHandshakeID = curTime.toString("hhmmsszzz").toUInt();
    qsrand(curTime.time().msec());
    m_myHandshakeID += qrand();
    m_peerHandshakeID = 0;

    //m_connectToPeerTimer = 0;
    m_msecConnectToPeerTimeout = 0;

    m_firstReceivedPacketIDInReceiveWindow = 1;
    m_cachedReceivedPacketsHash.clear();

    m_sendWindowSize = 16;


    m_firstWaitingForACKPacketIDInSendWindow = 1;


    m_receivedFragmentDataPacketID = 0;
    m_receivedFragmentDataPackets.clear();





    m_keepAliveTimer = 0;
    //m_keepAliveTimerInterval = RUDP_KEEPALIVE_TIMER_INTERVAL;
    m_peerLastLiveTime = QDateTime();

    m_checkPeerAliveTimer = 0;
    m_checkPeerAliveTimes = RUDP_MAX_CHECK_ALIVE_TIMES;

    //m_ChannelState = UnconnectedState;
    updateChannelState(UnconnectedState);

    m_packetSerialNumber = 0;

    m_fragmentCount = 0;
    activeFragmentID = 0;
    firstFragmentDataPacketSN = 0;

}

void RUDPChannel::reset(){
    qDebug()<<"--RUDPChannel::reset()"<<" ThreadId:"<<QThread::currentThreadId();


    if(m_connectToPeerTimer){
        m_connectToPeerTimer->stop();
        //delete m_connectToPeerTimer;
        //m_connectToPeerTimer = 0;
    }

    //if(sendPacketTimer){
    //    sendPacketTimer->stop();
    //delete sendPacketTimer;
    //sendPacketTimer = 0;
    //}

    if(sendACKTimer){
        sendACKTimer->stop();
        //delete sendACKTimer;
        //sendACKTimer = 0;
    }

    if(sendNACKTimer){
        sendNACKTimer->stop();
        //delete sendNACKTimer;
        //sendNACKTimer = 0;
    }

    if(retransmissionTimer){
        retransmissionTimer->stop();
        //delete retransmissionTimer;
        //retransmissionTimer = 0;
    }

    if(m_keepAliveTimer){
        m_keepAliveTimer->stop();
        //delete m_keepAliveTimer;
        //m_keepAliveTimer = 0;
    }

    if(m_checkPeerAliveTimer){
        m_checkPeerAliveTimer->stop();
        //delete m_checkPeerAliveTimer;
        //m_checkPeerAliveTimer = 0;
    }


    foreach (RUDPPacket *packet, m_ToBeSentPacketsHash.values()) {
        recylePacket(packet);
        //delete packet;
        //packet = 0;
    }

    foreach (RUDPPacket *packet, sentPackets.values()) {
        recylePacket(packet);
        //delete packet;
        //packet = 0;
    }

    foreach (LostPacketInfo *info, lostPacketsInReceiverSide.values()) {
        delete info;
        info = 0;
    }
    foreach (ACKPacketInfo *info, ackPacketsHistory.values()) {
        delete info;
        info = 0;
    }

    foreach (RUDPPacket *packet, m_cachedReceivedPacketsHash.values()) {
        recylePacket(packet);
        //delete packet;
        //packet = 0;
    }
    //m_cachedReceivedPacketsList.clear();

    foreach (RUDPPacket *packet, m_receivedFragmentDataPackets.values()) {
        recylePacket(packet);
        //delete packet;
        //packet = 0;
    }
    //m_receivedFragmentDataPackets.clear();

    //    foreach (RUDPPacket *packet, m_unusedPackets) {
    //        delete packet;
    //        packet = 0;
    //    }
    //    m_unusedPackets.clear();


    init();





}

quint16 RUDPChannel::createSerialNumber() {
    //qDebug()<<"--RUDPChannel::createSerialNumber()";

    if (m_packetSerialNumber == quint16(RUDP_MAX_PACKET_SN)) {
        m_packetSerialNumber = 1;
    }else{
        m_packetSerialNumber++;
    }

    return m_packetSerialNumber;

}

quint16 RUDPChannel::createSerialNumberForControlPacket() {
    //qDebug()<<"--RUDPChannel::createSerialNumberForControlPacket()";

    static quint16 sn = 0;

    if (sn == quint16(RUDP_MAX_PACKET_SN)) {
        sn = 1;
    }else{
        sn++;
    }

    return sn;

}


void RUDPChannel::processPacket(RUDPPacket *packet){
    //qDebug()<<"--RUDPChannel::processPacket(...)";

    //QCoreApplication::processEvents();

    Q_ASSERT(packet);

    //TODO:Process Packet
    QByteArray packetData = packet->getPacketData();
    QDataStream in(&packetData, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_7);

    quint8 packetType = packet->getPacketType();
    quint16 packetSerialNumber = packet->getPacketSerialNumber();

    switch(packetType){
    case quint8(RUDP::Handshake):
    {
        quint8 peerVersion =0;
        quint16 peerMSS = m_MSS;
        uint handshakeID = 0;
        in >> peerVersion >> peerMSS >> handshakeID;

        qDebug()<<"--------------m_myHandshakeID:"<<m_myHandshakeID<<" m_peerHandshakeID:"<<m_peerHandshakeID<<" handshakeID"<<handshakeID;

        if(m_peerHandshakeID == 0){
            m_peerHandshakeID = handshakeID;
            if(getChannelState() == ConnectingState){
                sendHandshakePacket(m_myHandshakeID + 1);
            }else{
                sendHandshakePacket(m_myHandshakeID);
            }
            return;
        }else{

            if(m_peerHandshakeID != handshakeID){
                if((m_peerHandshakeID + 1) == handshakeID){
                    if(getChannelState() == UnconnectedState){
                        sendHandshakePacket(m_myHandshakeID + 1);
                    }
                    if(getChannelState() == ConnectingState){
                        QMetaObject::invokeMethod(this, "startKeepAliveTimer");
                        QCoreApplication::processEvents();
                        //startKeepAliveTimer();
                    }
                }else{
                    uint myHandshakeID = m_myHandshakeID;
                    QMetaObject::invokeMethod(this, "reset");
                    QCoreApplication::processEvents();
                    m_myHandshakeID = myHandshakeID;

                    m_peerHandshakeID = handshakeID;
                    sendHandshakePacket(m_myHandshakeID);
                    return;
                }
            }else{
                sendHandshakePacket(m_myHandshakeID + 1);
                return;
            }

        }


        if(getChannelState() != ConnectedState){

            if(peerMSS < m_MSS){
                m_MSS = peerMSS;
            }

            //m_ChannelState = ConnectedState;
            updateChannelState(ConnectedState);

            QMetaObject::invokeMethod(this, "startSendACKTimer");
            QMetaObject::invokeMethod(this, "startSendNACKTimer");
            QMetaObject::invokeMethod(this, "startRetransmissionTimer");
            QMetaObject::invokeMethod(this, "startCheckPeerAliveTimer");
            //            startSendACKTimer();
            //            startSendNACKTimer();
            //            startRetransmissionTimer();
            //            startCheckPeerAliveTimer();

            emit peerConnected(m_peerAddress, m_peerPort);

            QCoreApplication::processEvents();

            //qDebug()<<"--Peer Connected! IP:"<<m_peerAddress;

        }

        //qDebug()<<"~~Handshake--"<<" peerVersion:"<<peerVersion<<" peerMSS:"<<peerMSS;
    }
        break;
    case quint8(RUDP::Reset):
    {
        quint8 peerVersion =0;
        in >> peerVersion;

        QMetaObject::invokeMethod(this, "reset");
        QCoreApplication::processEvents();
        //reset();

        qDebug()<<"~~Reset--"<<" peerVersion:"<<peerVersion;
    }
        break;
    case quint8(RUDP::Goodbye):
    {
        QString msg = "";
        in >> msg;

        if(getChannelState() != DisconnectingState){
            disconnectFromPeer();
        }

        QMetaObject::invokeMethod(this, "reset");
        QCoreApplication::processEvents();
        //reset();
        emit peerDisconnected(m_peerAddress, m_peerPort, true);

        //qDebug()<<"~~Goodbye--"<<"msg:"<<msg;
    }
        break;

    case quint8(RUDP::ACK):
    {

        quint16 peerFirstReceivedPacketIDInReceiveWindow = 0;
        int rtt = 0, rttvar = 0, freeReceiveBufferSize = 0, packetArrivalSpeed = 0, linkCapacity = 0;
        in >> peerFirstReceivedPacketIDInReceiveWindow >> rtt >> rttvar >> freeReceiveBufferSize >> packetArrivalSpeed >> linkCapacity;

        sendACK2(packetSerialNumber);

        RTT = ((rtt == 0)?100:rtt);
        RTTVar = (rttvar == 0)?100:rttvar;
        sendACKTimerInterval = sendNACKTimerInterval = 4 * RTT + RTTVar + SYN;
        if(sendACKTimerInterval > RUDP_MAX_SEND_ACK_TIMER_INTERVAL){
            sendACKTimerInterval = sendNACKTimerInterval = RUDP_MAX_SEND_ACK_TIMER_INTERVAL;
        }
        //retransmissionTimerInterval = 3 * RTT + SYN;

        //TODO
        if(m_sendWindowSize < RUDP_MAX_SEND_WINDOW_SIZE){
            if(m_sendWindowSize <= RUDP_SLOWSTART_THRESHOLD){
                m_sendWindowSize *= 2;

                //int rate = (m_sendWindowSize/5);
                //m_sendWindowSize += (rate?rate:1);
            }else{
                m_sendWindowSize++;
            }
        }
        quint16 peerReceiveWindowSize = freeReceiveBufferSize / m_MSS;
        //qDebug()<<"------------peerReceiveWindowSize:"<<peerReceiveWindowSize<<" m_sendWindowSize:"<<m_sendWindowSize;
        if(m_sendWindowSize > peerReceiveWindowSize && peerReceiveWindowSize > 0){
            m_sendWindowSize = peerReceiveWindowSize;
        }
        if(m_sendWindowSize < RUDP_MIN_SEND_WINDOW_SIZE){
            m_sendWindowSize = RUDP_MIN_SEND_WINDOW_SIZE;
        }

        //if(!freeReceiveBufferSize){return;}

        m_packetArrivalSpeed = (m_packetArrivalSpeed * 7 + packetArrivalSpeed) / 8;
        m_linkCapacity = (m_linkCapacity * 7 + linkCapacity) / 8;

        //Update sender's buffer
        if(!waitingForACKPackets.isEmpty()){
            updateFirstWaitingForACKPacketIDInSendWindow(peerFirstReceivedPacketIDInReceiveWindow);
        }

        qDebug()<<"~~ACK--"<<" peerFirstReceivedPacketIDInReceiveWindow:"<<peerFirstReceivedPacketIDInReceiveWindow<<" rtt:"<<rtt<<" rttvar:"<<rttvar<<" m_firstWaitingForACKPacketIDInSendWindow:"<<m_firstWaitingForACKPacketIDInSendWindow;
    }
        break;
    case quint8(RUDP::ACK2):
    {
        largestACK2SN = packetSerialNumber;
        ACKPacketInfo *info = ackPacketsHistory.value(largestACK2SN);
        if(!info){break;}

        int rtt= abs(QDateTime::currentDateTime().msecsTo(info->sentTime));
        RTT = (RTT * 7 + rtt) / 8;
        RTTVar = (RTTVar * 3 + abs(RTT - rtt)) / 4;
        sendACKTimerInterval = sendNACKTimerInterval = 4 * RTT + RTTVar + SYN;
        if(sendACKTimerInterval > RUDP_MAX_SEND_ACK_TIMER_INTERVAL){
            sendACKTimerInterval = sendNACKTimerInterval = RUDP_MAX_SEND_ACK_TIMER_INTERVAL;
        }
        //retransmissionTimerInterval = 3 * RTT + SYN;


        //qDebug()<<"------------------- ACK2 ---------------------"<<"rtt:"<<rtt<<" RTT:"<<RTT<<" RTTVar:"<<RTTVar<<" sendACKTimerInterval:"<<sendACKTimerInterval;

        //TODO: 删除 ACKPacketInfo
        // delete info;

        //qDebug()<<"~~ACK2--";

    }
        break;
    case quint8(RUDP::NACK):
    {
        QList<quint16> lostPackets;

        quint16 firstSN = 0;
        in >> firstSN;
        lostPackets.append(firstSN);

        while (!in.atEnd()) {
            getLostPacketsFromNACK(&lostPackets, &in);
            QCoreApplication::processEvents();
        }

        qSort(lostPackets.end(), lostPackets.begin());

        //qDebug()<<"------------------ NACK ----------------------Lost Packets From NACK:"<<lostPackets;

        lostPackets.removeAll(0);
        for(int i=lostPackets.size()-1; i>=0; i--){
            quint16 sn = lostPackets.at(i);
            lostPacketsInSenderSide.removeAll(sn);
            lostPacketsInSenderSide.prepend(sn);
        }

        //qDebug()<<"------------------ NACK ----------------------lostPacketsInSenderSide:"<<lostPacketsInSenderSide;

        //TODO
        //Update the SND period by rate control

        //Reset the EXP time variable.


        retransmitLostPacket();

        qDebug()<<"~~NACK--";

    }
        break;

    case quint8(RUDP::PacketDropped):
    {
        quint16 peerFirstReceivedPacketIDInQueue = 0, peerReceiveWindowSize = 0;
        in >> peerFirstReceivedPacketIDInQueue >> peerReceiveWindowSize;

        updateFirstWaitingForACKPacketIDInSendWindow(peerFirstReceivedPacketIDInQueue);

        if(m_sendWindowSize > peerReceiveWindowSize){
            m_sendWindowSize = peerReceiveWindowSize;
        }
        if(m_sendWindowSize < RUDP_MIN_SEND_WINDOW_SIZE){
            m_sendWindowSize = RUDP_MIN_SEND_WINDOW_SIZE;
        }

        qDebug()<<"~~PacketDropped--"<<"packetSerialNumber:"<<packetSerialNumber<<" m_firstWaitingForACKPacketIDInSendWindow:"<<m_firstWaitingForACKPacketIDInSendWindow<<" peerFirstReceivedPacketIDInQueue:"<<peerFirstReceivedPacketIDInQueue<<" peerReceiveWindowSize:"<<peerReceiveWindowSize;
    }
        break;
    case quint8(RUDP::KeepAlive):
    {

        m_peerLastLiveTime = QDateTime::currentDateTime();

        if(!m_keepAliveTimer || !m_keepAliveTimer->isActive()){
            sendKeepAlivePacket();
        }

        //qDebug()<<"~~KeepAlive--"<<" m_peerLastLiveTime:"<<m_peerLastLiveTime.toString("hh:mm:dd:zzz");
    }
        break;
    case quint8(RUDP::CompleteDataPacket):
    {
        QByteArray data;
        in >> data;

        //emit dataReceived(m_peerAddress, m_peerPort, data);
        cacheData(&data);

        //qDebug()<<"~~CompleteDataPacket--"<<" SN:"<<packetSerialNumber<<" data.size():"<<data.size();
    }
        break;
    case quint8(RUDP::BeginOrEndDataTransmission):
    {
        //static quint16 activeFragmentID = 0;
        //static quint32 firstFragmentDataPacketSN = 0;


        quint16 fragmentDataID = 0;
        quint8 start = 0;
        in >> fragmentDataID >> start;

        qDebug()<<"~~BeginOrEndDataTransmission--"<<" fragmentDataID:"<<fragmentDataID<<" start:"<<start;


        if(start){
            qDebug()<<"----------------activeFragmentID:"<<activeFragmentID<<" fragmentDataID:"<<fragmentDataID<<" "<<m_peerAddress.toString();
//            if(activeFragmentID != 0){
//                //Fragment ID Conflict!
//                QMetaObject::invokeMethod(this, "reset");
//                QCoreApplication::processEvents();
//                return;
//            }
            Q_ASSERT_X(activeFragmentID == 0, "RUDPChannel::processPacket(...)", "Fragment ID Conflict!");
            activeFragmentID = fragmentDataID;
            if(packetSerialNumber == RUDP_MAX_PACKET_SN){
                firstFragmentDataPacketSN = 1;
            }else{
                firstFragmentDataPacketSN = packetSerialNumber + 1;
            }

            qDebug()<<"~~BeginOrEndDataTransmission--Start-- "<<"activeFragmentID:"<<activeFragmentID<<" firstFragmentDataPacketSN:"<<firstFragmentDataPacketSN<<" m_receivedFragmentDataPackets.size():"<<m_receivedFragmentDataPackets.size();
        }else{
            quint32 fragmentCount;
            in >> fragmentCount;

            QByteArray fullData;
            QDataStream out(&fullData, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_7);
            QIODevice *dev = out.device();

            //            quint32 fragmentCount = 0;
            quint16 lastFragmentDataPacketSN = 0;
            if(packetSerialNumber == 1){
                lastFragmentDataPacketSN = RUDP_MAX_PACKET_SN;
            }else{
                lastFragmentDataPacketSN = packetSerialNumber - 1;
            }

            //            if(lastFragmentDataPacketSN > firstFragmentDataPacketSN){
            //                fragmentCount = lastFragmentDataPacketSN - firstFragmentDataPacketSN + 1;
            //            }else{
            //                fragmentCount = (RUDP_MAX_PACKET_SN - firstFragmentDataPacketSN) + 1 + lastFragmentDataPacketSN;
            //            }
            qDebug()<<"----m_receivedFragmentDataPackets.size():"<<m_receivedFragmentDataPackets.size()<<" fragmentCount:"<<fragmentCount;
            //qDebug()<<"----m_receivedFragmentDataPackets:"<<m_receivedFragmentDataPackets.keys();

            Q_ASSERT_X(m_receivedFragmentDataPackets.size() == fragmentCount, "RUDPChannel::processPacket(...)", "Fragment Data Lost!");


            //            int steps = 0;
            //            if(firstFragmentDataPacketSN > lastFragmentDataPacketSN){
            //                steps = RUDP_MAX_PACKET_SN - firstFragmentDataPacketSN + lastFragmentDataPacketSN;
            //            }else{
            //                steps = lastFragmentDataPacketSN - firstFragmentDataPacketSN;
            //            }
            int steps = fragmentCount;
            for(int i=0; i<steps; i++){
                quint16 sn = firstFragmentDataPacketSN + i;
                if(sn == RUDP_MAX_PACKET_SN){
                    i = -1;
                    //fragmentCount -= (RUDP_MAX_PACKET_SN - firstFragmentDataPacketSN);
                    steps = lastFragmentDataPacketSN;
                    firstFragmentDataPacketSN = 1;
                }
                RUDPPacket *p = m_receivedFragmentDataPackets.value(sn);
                if(!p){continue;}

                qint64 sizeWrite = dev->write(p->getPacketData().data(), p->packetDataSize());
                Q_ASSERT_X(sizeWrite == p->packetDataSize(), "RUDPChannel::processPacket(...)", QString("ERROR! Write data error! %1").arg(dev->errorString()).toLocal8Bit());
                if(sizeWrite == -1){
                    qCritical()<<QString("ERROR! Write data error! %1").arg(dev->errorString());
                }

                //qDebug()<<"----------sizeWrite:"<<sizeWrite<<" fullData.size():"<<fullData.size();
                qDebug()<<"------------------fragmentDataID:"<<fragmentDataID<<" SN:"<<sn<< " size:"<<p->packetDataSize()<<" Checksum:"<<qChecksum(p->getPacketData().data(), p->packetDataSize());

                m_freeReceiveBufferSize += p->packetDataSize();
                recylePacket(p);
            }

            qDebug()<<"~~BeginOrEndDataTransmission--End-- "<<"activeFragmentID:"<<activeFragmentID<<" firstFragmentDataPacketSN:"<<firstFragmentDataPacketSN<<" m_receivedFragmentDataPackets.size():"<<m_receivedFragmentDataPackets.size()<<" fullData.size():"<<fullData.size();

            activeFragmentID = 0;
            firstFragmentDataPacketSN = 0;
            m_receivedFragmentDataPackets.clear();
            //emit dataReceived(m_peerAddress, m_peerPort, fullData);
            cacheData(&fullData);

        }


        //qDebug()<<"~~BeginOrEndDataTransmission--"<<" fragmentDataID:"<<fragmentDataID<<" start:"<<start<<" fragmentCount:"<<fragmentCount;


    }
        break;
    case quint8(RUDP::FragmentDataPacket):
    {
        QByteArray fragmentData;
        in >> fragmentData ;
        packet->setPacketData(fragmentData);

        m_receivedFragmentDataPackets.insert(packetSerialNumber, packet);
        packet = 0;


        //qDebug()<<"~~FragmentDataPacket---fragmentData.size():"<<fragmentData.size()<<" m_receivedFragmentDataPackets.size():"<<m_receivedFragmentDataPackets.size();
    }
        break;
    case quint8(RUDP::UnreliableDataPacket):
    {
        QByteArray data;
        in >> data;

        cacheData(&data);

        qDebug()<<"~~UnreliableDataPacket";
    }
        break;

    default:
        qWarning()<<"Unknown Packet Type:"<<packetType
                 <<" Serial Number:"<<packetSerialNumber
                <<" From:"<<m_peerAddress.toString()
               <<" Port:"<<m_peerPort;
        break;

    }

    if(packet){
        recylePacket(packet);

        m_freeReceiveBufferSize += packet->packetDataSize();
    }



}

void RUDPChannel::getLostPacketsFromNACK(QList<quint16> *lostPackets, QDataStream *in){
    //qDebug()<<"--RUDPChannel::getLostPacketsFromNACK(...)";

    Q_ASSERT(lostPackets);
    Q_ASSERT(in);


    quint16 sn = 0;
    QChar sepChar = ',';
    *in >> sepChar >> sn;
    if(!sn){return;}

    if(sepChar == ','){
        lostPackets->append(sn);
    }else if(sepChar == '-'){
        quint16 lastSn = lostPackets->last();
        Q_ASSERT_X(lastSn < sn, "RUDPChannel::getLostPacketsFromNACK(...)", QString("Range Error! lastSn:%1 sn:%2").arg(lastSn).arg(sn).toLocal8Bit() );

        for(int i=lastSn+1; i<=sn; i++){
            lostPackets->append(i);
        }

    }


}

void RUDPChannel::cacheData(QByteArray *data){
    qDebug()<<"--RUDPChannel::cacheData(QByteArray *data) "<<" Size:"<<data->size();

    Q_ASSERT(data);

    QDataStream in(data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_7);
    QVariant v;
    in >> v;
    if (v.canConvert<Packet>()){
        Packet *packet = m_packetHandlerBase->getPacket();
        *packet = v.value<Packet>();
        packet->setTransmissionProtocol(TP_RUDP);
        packet->setPeerHostAddress(m_peerAddress);
        packet->setPeerHostPort(m_peerPort);
        packet->setLocalHostAddress(m_udpSocket->localAddress());
        packet->setLocalHostPort(m_udpSocket->localPort());

        m_packetHandlerBase->appendIncomingPacket(packet);
        qDebug()<<"------------m_packetHandlerBase->incomingPacketsCount():"<<m_packetHandlerBase->incomingPacketsCount();
    }

}

inline void RUDPChannel::addWaitingForACKPacket(RUDPPacket *packet){

    Q_ASSERT(packet);

    quint16 packetSerialNumber = packet->getPacketSerialNumber();
    sentPackets.insert(packetSerialNumber, packet);
    waitingForACKPackets.append(packetSerialNumber);
}

void RUDPChannel::removeWaitingForACKPackets(quint16 start, quint16 end){
    //qDebug()<<"--removeWaitingForACKPackets(...)--start:"<<start<<" end:"<<end;

    while (start <= end) {
        if( (end - start) > RUDP_MAX_SEND_WINDOW_SIZE ){break;}
        QCoreApplication::processEvents();

        waitingForACKPackets.removeAll(start);
        lostPacketsInSenderSide.removeAll(start);
        RUDPPacket *packet = sentPackets.take(start);
        if(packet){
            updateGlobalFreeSendBufferSize(packet->packetDataSize(), false);
            recylePacket(packet);
        }

        if(!waitingForACKPackets.isEmpty()){
            start = waitingForACKPackets.first();
        }else{
            break;
        }
        QCoreApplication::processEvents();
    }


    //qDebug()<<"--lostPacketsInSenderSide:"<<lostPacketsInSenderSide;
    //qDebug()<<"--waitingForACKPackets:"<<waitingForACKPackets;


}

void RUDPChannel::updateFirstWaitingForACKPacketIDInSendWindow(quint16 peerFirstReceivedPacketIDInQueue){
    //qDebug()<<"--RUDPChannel::updateFirstWaitingForACKPacketIDInSendWindow(...)--"<<" peerFirstReceivedPacketIDInQueue:"<<peerFirstReceivedPacketIDInQueue<<" m_firstWaitingForACKPacketIDInSendWindow:"<<m_firstWaitingForACKPacketIDInSendWindow<<" LSSN:"<<LSSN ;

    if(m_firstWaitingForACKPacketIDInSendWindow <= peerFirstReceivedPacketIDInQueue){
        removeWaitingForACKPackets(m_firstWaitingForACKPacketIDInSendWindow, peerFirstReceivedPacketIDInQueue-1);
    }else{
        if((peerFirstReceivedPacketIDInQueue - 1) <= LSSN){
            removeWaitingForACKPackets(m_firstWaitingForACKPacketIDInSendWindow, RUDP_MAX_PACKET_SN);
            removeWaitingForACKPackets(1, peerFirstReceivedPacketIDInQueue-1);
        }
    }
    //qDebug()<<"----"<<" peerFirstReceivedPacketIDInQueue:"<<peerFirstReceivedPacketIDInQueue<<" m_firstWaitingForACKPacketIDInSendWindow:"<<m_firstWaitingForACKPacketIDInSendWindow<<" LSSN:"<<LSSN ;

    if(m_firstWaitingForACKPacketIDInSendWindow != peerFirstReceivedPacketIDInQueue){
        m_firstWaitingForACKPacketIDInSendWindow = peerFirstReceivedPacketIDInQueue;
    }


}

inline RUDPPacket * RUDPChannel::takeToBeSentPacket(quint16 packetID){

    return m_ToBeSentPacketsHash.take(packetID);

}

void RUDPChannel::addToBeSentPacket(RUDPPacket *packet ){

    Q_ASSERT(packet);

    quint16 sn = packet->getPacketSerialNumber();
    m_ToBeSentPacketsHash.insert(sn, packet);

    updateGlobalFreeSendBufferSize(packet->packetDataSize());

}

RUDPPacket * RUDPChannel::getUnusedPacket(){
    //qDebug()<<"--RUDPChannel::getUnusedPacket()";

    QMutexLocker locker(unusedPacketsMutex);
    RUDPPacket *packet = 0;
    if(m_unusedPackets->isEmpty()){
        packet = new RUDPPacket(0, quint8(RUDP::UnKnownPacket), 0, RUDP_PACKET_RETRANSMISSION_TIMES);
    }else{
        packet = m_unusedPackets->takeFirst();
    }

    return packet;

}

void RUDPChannel::recylePacket(RUDPPacket *packet){
    //qDebug()<<"--RUDPChannel::recylePacket(...)";

    //Q_ASSERT_X(packet, "RUDPChannel::recylePacket(RUDPPacket *packet)", "Invalid Packet!");

    if(!packet){return;}

    QMutexLocker locker(unusedPacketsMutex);

    m_receiveWindowSize = m_freeReceiveBufferSize / m_MSS;


    if(m_unusedPackets->size() >= m_maxUnusedPacketsCount){
        delete packet;
        packet = 0;
        return;
    }

    if(packet){
        //m_freeSendBufferSize += packet->packetDataSize();
        packet->resetPacket();
        m_unusedPackets->append(packet);

    }


}

void RUDPChannel::cleanAllUnusedPackets(){
    QMutexLocker locker(unusedPacketsMutex);

    for(int i=0; i<m_unusedPackets->size(); i++){
        RUDPPacket *packet = m_unusedPackets->at(i);
        delete packet;
        packet = 0;
    }

    m_unusedPackets->clear();

}

void RUDPChannel::setMaxCachedUnusedPacketsCount(int count){
    QMutexLocker locker(unusedPacketsMutex);
    m_maxUnusedPacketsCount = count;
}

int RUDPChannel::getMaxCachedUnusedPacketsCount(){
    QMutexLocker locker(unusedPacketsMutex);
    return m_maxUnusedPacketsCount;

}

//bool RUDPChannel::canSendData(qint64 size){

//    if(size > getGlobalFreeSendBufferSize()){
//        qCritical()<<"ERROR! Can not send data! There is not enough buffer to cache the data! Free buffer size:"<<getGlobalFreeSendBufferSize();
//        return false;
//    }

//    return true;
//}

void RUDPChannel::setGlobalSendBufferSize(qint64 size){

    QMutexLocker locket(m_freeSendBufferSizeMutex);

    m_freeSendBufferSize = size;

}

qint64 RUDPChannel::getGlobalFreeSendBufferSize(){

    QMutexLocker locket(m_freeSendBufferSizeMutex);
    return m_freeSendBufferSize ;

}

void RUDPChannel::updateGlobalFreeSendBufferSize(qint64 size, bool reduce){

    QMutexLocker locket(m_freeSendBufferSizeMutex);

    if(reduce){
        m_freeSendBufferSize -= size;
        if(m_freeSendBufferSize < 0){
            m_freeSendBufferSize = 0;
        }
    }else{
        m_freeSendBufferSize += size;
    }

}

void RUDPChannel::updateChannelState(ChannelState state){
    qDebug()<<"--RUDPChannel::updateChannelState(...)"<<" state:"<<state;

    {
        //QMutexLocker locker(&m_ChannelStateMutex);
        this->m_ChannelState = state;
    }

}

RUDPChannel::ChannelState RUDPChannel::getChannelState(){

    {
        //QMutexLocker locker(&m_ChannelStateMutex);
        return m_ChannelState;
    }

}

inline void RUDPChannel::msleep(int msec){

#ifdef Q_CC_MSVC
    Sleep(msec);
#else
    usleep(msec*1000);
#endif

}





















} //namespace HEHUI
