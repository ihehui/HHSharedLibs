/*
 ****************************************************************************
 * PacketsParserBase.cpp
 *
 * Created On: 2010-6-12
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
 * Last Modified On: 2011-07-23
 * Last Modified By: 贺辉
 ***************************************************************************
 */




#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>

#include "packetparserbase.h"
#include "packet.h"
//#include "packetstreamoperator.h"


#ifdef Q_CC_MSVC
    #include <windows.h>
    #define msleep(x) Sleep(x)
#endif

#ifdef Q_CC_GNU
    #include <unistd.h>
    #define msleep(x) usleep(x*1000)
#endif


namespace HEHUI {


PacketsParserBase::PacketsParserBase(NetworkManagerBase *networkManagerBase, QObject *parent)
    :QObject(parent), m_networkManagerBase(networkManagerBase)
{

    Q_ASSERT_X(m_networkManagerBase, "PacketsParserBase::PacketsParserBase(...)", "Invalid NetworkManagerBase!");

    m_packetHandlerBase = m_networkManagerBase->getPacketHandler();
    Q_ASSERT_X(m_packetHandlerBase, "PacketsParserBase::PacketsParserBase(...)", "Invalid PacketHandlerBase!");

    //m_networkManagerBase = new NetworkManagerBase();

    m_aboutToQuit = false;


//    //注册自定义类型，必须重载“<<”和“>>”, 见"packetstreamoperator.h"
//    qRegisterMetaTypeStreamOperators<HEHUI::Packet>("HEHUI::Packet");


}

PacketsParserBase::~PacketsParserBase() {
    // TODO Auto-generated destructor stub

    qDebug()<<"PacketsParserBase::~PacketsParserBase()";

    if(!isAboutToQuit()){
        aboutToQuit();
    }



}

void PacketsParserBase::run(){

    while(!isAboutToQuit()){
        parseIncomingPackets();
        processOutgoingPackets();
//        processWaitingForReplyPackets();

    }

}

void PacketsParserBase::startparseIncomingPackets(){

    while(!isAboutToQuit()){
        QCoreApplication::processEvents();
        parseIncomingPackets();
        msleep(50);
    }

}

void PacketsParserBase::startprocessOutgoingPackets(){
    qDebug()<<"--PacketsParserBase::startprocessOutgoingPackets()";

//    QTimer processWaitingForReplyPacketsTimer;
//    processWaitingForReplyPacketsTimer.setSingleShot(false);
//    //processWaitingForReplyPacketsTimer.setInterval(UDP_PACKET_WAITING_FOR_REPLY_TIMEOUT);
//    processWaitingForReplyPacketsTimer.setInterval(UDP_PACKET_WAITING_FOR_REPLY_TIMEOUT/2);
//    connect(&processWaitingForReplyPacketsTimer, SIGNAL(timeout()), this, SLOT(processWaitingForReplyPackets()));
//    connect(this, SIGNAL(signalAboutToQuit()), &processWaitingForReplyPacketsTimer, SLOT(stop()));
//    processWaitingForReplyPacketsTimer.start();

    while(!isAboutToQuit()){
        QCoreApplication::processEvents();
        processOutgoingPackets();
        msleep(50);
    }

//    processWaitingForReplyPacketsTimer.stop();

    processOutgoingPackets();

}

bool PacketsParserBase::isAboutToQuit(){
    QMutexLocker locker(&aboutToQuitMutex);
    return m_aboutToQuit;
}


void PacketsParserBase::aboutToQuit(int msecTimeout){
    //{
        QMutexLocker locker(&aboutToQuitMutex);
        this->m_aboutToQuit = true;
        emit signalAboutToQuit();
    //}

    QCoreApplication::processEvents();
    //msleep(msecTimeout);

}


void PacketsParserBase::parseIncomingPackets(){
    //qDebug()<<"----PacketsParserBase::parseIncomingPackets()";


    //    qWarning()<<"incomingPacketsCount:"<<packetHandlerBase->incomingPacketsCount();
    for(int i = 0; i < m_packetHandlerBase->incomingPacketsCount(); i++){
        //        while(1){
        Packet *packet = m_packetHandlerBase->takeIncomingPacket();

        if (!packet) {
            break;
        }

        if (!packet->isValid()) {
            m_packetHandlerBase->recylePacket(packet);
            continue;
        }


        parseIncomingPacketData(packet);

        m_packetHandlerBase->recylePacket(packet);

        //QCoreApplication::processEvents();
    }


}

void PacketsParserBase::processOutgoingPackets() {
    //qDebug()<<"----PacketsParserBase::processOutgoingPackets()";


    for(int i = 0; i < m_packetHandlerBase->outgoingPacketsCount(); i++){
        Packet *packet = m_packetHandlerBase->takeOutgoingPacket();
        if (!packet) {
            break;
        }
        if (!packet->isValid()) {
            m_packetHandlerBase->recylePacket(packet);
            continue;
        }

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_7);
        QVariant v;
        v.setValue(*packet);
        out << v;

        TransmissionProtocol transmissionProtocol = packet->getTransmissionProtocol();

        bool result = false;
        if (transmissionProtocol == TP_TCP) {
            //result = m_networkManagerBase->slotSendNewTCPDatagram(QHostAddress(packet->getPeerHostAddress()), packet->getPeerHostPort(), block);

        } else if (transmissionProtocol == TP_UDP) {
            result = m_networkManagerBase->slotSendNewUDPDatagram(packet->getPeerHostAddress(), packet->getPeerHostPort(), &block, packet->getLocalHostPort(), false);
        }else if(transmissionProtocol == TP_RUDP){
            result = m_networkManagerBase->slotSendNewUDPDatagram(packet->getPeerHostAddress(), packet->getPeerHostPort(), &block, packet->getLocalHostPort(), true);
        }

        if (!result) {
            packet->packetTransmissionFailed();

            if((packet->getRemainingRetransmissionTimes() > 0)){
                m_packetHandlerBase->appendOutgoingPacket(packet);
            }else{
                if(transmissionProtocol == TP_RUDP){
                    m_networkManagerBase->getRUDPServer(packet->getLocalHostPort(), packet->getLocalHostAddress())->closeChannel(packet->getPeerHostAddress(), packet->getPeerHostPort());
                }
                m_packetHandlerBase->recylePacket(packet);
            }

            qCritical()<<"Packet Sent Failed! Peer Address:"<<packet->getPeerHostAddress().toString()<<":"<<packet->getPeerHostPort();
        }
        //        else if (transmissionProtocol == TP_UDP) {
        //            packet->setLastTransmissionTime(QDateTime::currentDateTime());
        //            //m_packetHandlerBase->appendWaitingForReplyPacket(packet);
        //        }
        else{
            m_packetHandlerBase->recylePacket(packet);
        }

//        if (packet->getRemainingRetransmissionTimes() > 0) {
//            if (!result) {
//                packet->packetTransmissionFailed();
//                m_packetHandlerBase->appendOutgoingPacket(packet);
//                qWarning()<<"Packet Sent Failed! Peer Address:"<<packet->getPeerHostAddress().toString()<<":"<<packet->getPeerHostPort();

//            } else {
//                if (transmissionProtocol == TP_UDP) {
//                    packet->setLastTransmissionTime(QDateTime::currentDateTime());
////                    m_packetHandlerBase->appendWaitingForReplyPacket(packet);
//                }
//            }

//        } else {
//            m_packetHandlerBase->recylePacket(packet);
//        }

        QCoreApplication::processEvents();

    }

}

//void PacketsParserBase::processWaitingForReplyPackets() {
//    qDebug()<<"----PacketsParserBase::processWaitingForReplyPackets()";

//    for(int i = 0; i < m_packetHandlerBase->waitingForReplyPacketsCount(); i++){
//        Packet *packet = m_packetHandlerBase->takeWaitingForReplyPacket();
//        if (!packet) {
//            break;
//        }
//        if (!packet->isValid()) {
//            m_packetHandlerBase->recylePacket(packet);
//            continue;
//        }

//        QDateTime t = packet->getLastTransmissionTime().addMSecs(UDP_PACKET_WAITING_FOR_REPLY_TIMEOUT);
//        if (t <= QDateTime::currentDateTime()) {
//            packet->packetTransmissionFailed();
//            m_packetHandlerBase->appendOutgoingPacket(packet);
//        }else{
//            m_packetHandlerBase->appendWaitingForReplyPacket(packet);
//        }

//        QCoreApplication::processEvents();

//    }


//}











} //namespace HEHUI
