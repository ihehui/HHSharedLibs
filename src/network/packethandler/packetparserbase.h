/*
 ****************************************************************************
 * packetparserbase.h
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


#ifndef PACKETPARSERBASE_H_
#define PACKETPARSERBASE_H_

#include <QObject>
//#include <QThread>

#include "packethandlerbase.h"
#include "../networkmanagerbase.h"

#include "../networklib.h"





namespace HEHUI
{


class NETWORK_LIB_API PacketsParserBase : public QObject
{
    Q_OBJECT

public:
    PacketsParserBase(NetworkManagerBase *networkManagerBase, QObject *parent = 0);
    virtual ~PacketsParserBase();

    virtual void run();
    virtual void startparseIncomingPackets();
    virtual void startprocessOutgoingPackets();

    bool isAboutToQuit();



signals:
    void signalAboutToQuit();
    //        void signalPacketTransmissionFailed();

public slots:
    void aboutToQuit(int msecTimeout = 1000);

    virtual void parseIncomingPackets();
    virtual void parseIncomingPacketData(Packet *packet) = 0;
    virtual void processOutgoingPackets();
//    virtual void processWaitingForReplyPackets();


protected:


private:
    NetworkManagerBase *m_networkManagerBase;
    PacketHandlerBase *m_packetHandlerBase;

    QMutex aboutToQuitMutex;
    bool m_aboutToQuit;



};

}

#endif /* PACKETPARSERBASE_H_ */
