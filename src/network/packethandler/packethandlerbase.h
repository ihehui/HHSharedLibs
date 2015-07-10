/*
 ****************************************************************************
 * packethandlerbase.h
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

#ifndef PACKETHANDLERBASE_H_
#define PACKETHANDLERBASE_H_

#include <QObject>
#include <QMutex>

#include "packetbase.h"

//#include "../networklib.h"


namespace HEHUI {

class NETWORK_LIB_API PacketHandlerBase :public QObject{
    Q_OBJECT

public:
    

    PacketHandlerBase(QObject *parent = 0);
    virtual ~PacketHandlerBase();
    
    
    
    void appendIncomingPacket(PacketBase *packet);
    PacketBase * takeIncomingPacket();
    int incomingPacketsCount();
    
    void appendOutgoingPacket(PacketBase *packet);
    PacketBase * takeOutgoingPacket();
    int outgoingPacketsCount();
    
//    void appendWaitingForReplyPacket(Packet *packet);
//    Packet * takeWaitingForReplyPacket();
//    int waitingForReplyPacketsCount();
    
//    void removeWaitingForReplyPacket(quint16 packetSerialNumber1, quint16 packetSerialNumber2);
//    bool hasWaitingForReplyPackets();
    
    static PacketBase * getPacket();

//    static UDPPacket *getUDPPacket(const QHostAddress &peerAddress, quint16 peerPort, const QHostAddress &localAddress, quint16 localPort);
    static void recylePacket(PacketBase *packet);
    static int recyledPacketsCount();
    static void clean();

private:


private:

    QList<PacketBase *> *incomingPackets;
    QMutex incomingPacketsMutex;

    QList<PacketBase *> *outgoingPackets;
    QMutex outgoingPacketsMutex;

    //QHash<quint16, Packet *> *waitingForReplyPackets;
    //QMutex *waitingForReplyPacketsMutex;



    static QList<PacketBase *> *unusedPackets;
    static QMutex *unusedPacketsMutex;



};

}

#endif /* PACKETHANDLERBASE_H_ */
