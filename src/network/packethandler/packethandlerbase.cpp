/*
 ****************************************************************************
 * packethandlerbase.cpp
 *
 * Created On: 2010-6-11
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
 * Last Modified On: 2010-6-11
 * Last Modified By: 贺辉
 ***************************************************************************
 */

#ifndef CACHED_UNUSED_PACKET_COUNT
#define CACHED_UNUSED_PACKET_COUNT 1000
#endif

#include <QCoreApplication>
#include <QMutexLocker>
#include <QDebug>


#include "packethandlerbase.h"

//#include "packetstreamoperator.h"



namespace HEHUI {

//PacketHandlerBase* PacketHandlerBase::m_packetHandlerBase = 0;

//QList<Packet *>* PacketHandlerBase::incomingPackets = new QList<Packet *> ();
//QMutex * PacketHandlerBase::incomingPacketsMutex = new QMutex();
//QList<Packet *>* PacketHandlerBase::outgoingPackets = new QList<Packet *> ();
//QMutex * PacketHandlerBase::outgoingPacketsMutex = new QMutex();
//QHash<quint16, Packet *>* PacketHandlerBase::waitingForReplyPackets = new QHash<quint16, Packet *> ();
//QMutex * PacketHandlerBase::waitingForReplyPacketsMutex = new QMutex();

QList<PacketBase *>* PacketHandlerBase::unusedPackets = new QList<PacketBase *> ();
QMutex * PacketHandlerBase::unusedPacketsMutex = new QMutex();



PacketHandlerBase::PacketHandlerBase(QObject *parent)
    :QObject(parent)
{

    incomingPackets = new QList<PacketBase *> ();
    //incomingPacketsMutex = new QMutex();
    outgoingPackets = new QList<PacketBase *> ();
    //outgoingPacketsMutex = new QMutex();

//    waitingForReplyPackets = new QHash<quint16, Packet *> ();
//    waitingForReplyPacketsMutex = new QMutex();

    //注册自定义类型，必须重载“<<”和“>>”, 见"packetstreamoperator.h"
    qRegisterMetaTypeStreamOperators<HEHUI::PacketBase>("HEHUI::Packet");

}

PacketHandlerBase::~PacketHandlerBase() {
    qDebug()<<"--PacketHandlerBase::~PacketHandlerBase()";


    QMutexLocker incomingPacketsLocker(&incomingPacketsMutex);
    for (int  i= 0;  i< incomingPackets->count(); i++) {
        PacketBase *p = incomingPackets->at(i);
        recylePacket(p);
    }
    incomingPackets->clear();
    delete incomingPackets;

    QMutexLocker outgoingPacketsLocker(&outgoingPacketsMutex);
    for (int  i= 0;  i< outgoingPackets->count(); i++) {
        PacketBase *p = outgoingPackets->at(i);
        recylePacket(p);
    }
    outgoingPackets->clear();
    delete outgoingPackets;

//    QMutexLocker waitingForReplyLocker(waitingForReplyPacketsMutex);
//    foreach (Packet *p, waitingForReplyPackets->values()) {
//        recylePacket(p);
//    }
//    waitingForReplyPackets->clear();


}

void PacketHandlerBase::appendIncomingPacket(PacketBase *packet){

    QMutexLocker locker(&incomingPacketsMutex);
    if(packet && packet->isValid()){
        incomingPackets->append(packet);
    }


}

PacketBase * PacketHandlerBase::takeIncomingPacket(){

    QMutexLocker locker(&incomingPacketsMutex);
    if(incomingPackets->isEmpty()){
        return 0;
    }else{
        return incomingPackets->takeFirst();
    }

}

int PacketHandlerBase::incomingPacketsCount(){
    QMutexLocker locker(&incomingPacketsMutex);
    return incomingPackets->count();
}

void PacketHandlerBase::appendOutgoingPacket(PacketBase *packet){

    QMutexLocker locker(&outgoingPacketsMutex);
    if(packet && packet->isValid()){
        outgoingPackets->append(packet);
    }

}

PacketBase * PacketHandlerBase::takeOutgoingPacket(){

    QMutexLocker locker(&outgoingPacketsMutex);
    if(outgoingPackets->isEmpty()){
        return 0;
    }else{
        return outgoingPackets->takeFirst();
    }

}

int PacketHandlerBase::outgoingPacketsCount(){

    QMutexLocker locker(&outgoingPacketsMutex);
    return outgoingPackets->count();
}



//void PacketHandlerBase::appendWaitingForReplyPacket(Packet *packet){

//    QMutexLocker locker(waitingForReplyPacketsMutex);
//    if(packet && packet->isValid()){
//        //waitingForReplyPackets.append(packet);
//        waitingForReplyPackets->insert(packet->getPacketSerialNumber(), packet);
//    }

//}

//Packet * PacketHandlerBase::takeWaitingForReplyPacket(){
//    QMutexLocker locker(waitingForReplyPacketsMutex);
//    if(waitingForReplyPackets->isEmpty()){
//        return 0;
//    }else{
//        return waitingForReplyPackets->take(waitingForReplyPackets->keys().at(0));
//    }

//}

//int PacketHandlerBase::waitingForReplyPacketsCount(){
//    QMutexLocker locker(waitingForReplyPacketsMutex);
//    return waitingForReplyPackets->count();
//}

//void PacketHandlerBase::removeWaitingForReplyPacket(quint16 packetSerialNumber1, quint16 packetSerialNumber2){
//    QMutexLocker locker(waitingForReplyPacketsMutex);
//    if(waitingForReplyPackets->isEmpty()){
//        return;
//    }else{
//        Packet *packet = waitingForReplyPackets->take(packetSerialNumber1);
//        if(packet){
//            recylePacket(packet);
//        }

//        packet = waitingForReplyPackets->take(packetSerialNumber2);
//        if(packet){
//            recylePacket(packet);
//        }

//        //        delete packet;
//        //        packet = 0;
//    }
//}


//bool PacketHandlerBase::hasWaitingForReplyPackets(){
//    QMutexLocker locker(waitingForReplyPacketsMutex);
//    return (!waitingForReplyPackets->isEmpty());
//}



PacketBase * PacketHandlerBase::getPacket(){
    QMutexLocker locker(unusedPacketsMutex);
    
    PacketBase *packet = 0;
    if(unusedPackets->isEmpty()){
        packet = new PacketBase();
    }else{
        packet = unusedPackets->takeFirst();
    }
    
    return packet;
    
}


//UDPPacket *PacketHandlerBase::getUDPPacket(const QHostAddress &peerAddress, quint16 peerPort, const QHostAddress &localAddress, quint16 localPort){
//    QMutexLocker locker(unusedPacketsMutex);
//    if(unusedPackets->isEmpty()){
//        return new UDPPacket(peerAddress, peerPort, localAddress, localPort);
//    }else{
//        //Packet *packet = unusedPackets->takeFirst();
//        UDPPacket *packet = static_cast<UDPPacket *>(unusedPackets->takeFirst());
//        packet->setTransmissionProtocol(TP_UDP);
//        packet->setPeerHostAddress(peerAddress);
//        packet->setPeerHostPort(peerPort);
//        packet->setLocalHostAddress(localAddress);
//        packet->setLocalHostPort(localPort);
//        //UDPPacket *udpPacket = static_cast<UDPPacket *>(packet);
//        return packet;
//    }
    
//}

void PacketHandlerBase::recylePacket(PacketBase *packet){
    QMutexLocker locker(unusedPacketsMutex);
    if(packet){
        packet->resetPacket();
        if(unusedPackets->count() < CACHED_UNUSED_PACKET_COUNT){
            unusedPackets->append(packet);
        }else{
            delete packet;
            packet = 0;
        }
    }

}

int PacketHandlerBase::recyledPacketsCount(){
    QMutexLocker locker(unusedPacketsMutex);
    return unusedPackets->count();
}

void PacketHandlerBase::clean(){

    QMutexLocker locker(unusedPacketsMutex);
    for (int i = 0; i < unusedPackets->count(); i++) {
        delete unusedPackets->at(i);
    }
    unusedPackets->clear();

}










}
