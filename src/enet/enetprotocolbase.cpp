
#include "enetprotocolbase.h"
#include "enetprotocolbase_p.h"

#include <QCoreApplication>
#include <QDebug>
#include <QtConcurrent>





namespace HEHUI {


//////////////////////////////////////////////////////////////////////////////////////
ENETProtocolBasePrivate::ENETProtocolBasePrivate(QObject *parent) :
    QObject(parent)
{

    m_listening = false;
    m_threadCount = 0;

    localServer = 0;

    if(enet_initialize())
    {
        qCritical()<<"ERROR! Failed to initialize ENET!";
    }

}

ENETProtocolBasePrivate::~ENETProtocolBasePrivate(){
    close();
}


bool ENETProtocolBasePrivate::isListening() const {
    return m_listening;
}

bool ENETProtocolBasePrivate::getPeerAddressInfo(quint32 peerID, QString *address, quint16 *port){
    if(!peersHash.contains(peerID)){
        qCritical()<<"ERROR! Failed to get peer host address! No such peer!";
        return false;
    }

    ENetPeer *peer = peersHash.value(peerID);
    ENetAddress remote = peer->address;
    char ip[256];
    if( enet_address_get_host_ip(&remote, ip, 256) < 0 ){
        qCritical()<<"ERROR! Failed to get peer host address!";
        return false;
    }

    if(address){
        *address = QString(ip);
    }
    if(port){
        *port = remote.port;
    }

    return true;

}

bool ENETProtocolBasePrivate::getLocalListeningAddressInfo(QString *address, quint16 *port){

    if(!localServer){
        return false;
    }

    ENetAddress local = localServer->address;
    char ip[256];
    if( enet_address_get_host_ip(&local, ip, 256) < 0 ){
        qCritical()<<"ERROR! Failed to get local host address!";
        return false;
    }

    if(address){
        *address = QString(ip);
    }
    if(port){
        *port = local.port;
    }

    return true;

}

void ENETProtocolBasePrivate::setPeerPrivateData(quint32 peerID, void *data){

    if(!peersHash.contains(peerID)){
        qCritical()<<"ERROR! Failed to get peer host address! No such peer!";
        return;
    }

    ENetPeer *peer = peersHash.value(peerID);
    setPeerPrivateData(peer, data);
}

void * ENETProtocolBasePrivate::getPeerPrivateData(quint32 peerID){

    if(!peersHash.contains(peerID)){
        qCritical()<<"ERROR! Failed to get peer host address! No such peer!";
        return false;
    }

    ENetPeer *peer = peersHash.value(peerID);
    return peer->data;
}

bool ENETProtocolBasePrivate::listen(quint16 port, const QHostAddress &localAddress, unsigned int maximumNumberOfPeers){

    if(localServer){
        qCritical()<<"ERROR! ENET Server already listening!";
        return false;
    }

    if(enet_initialize())
    {
        qCritical()<<"ERROR! Failed to initialize ENET!";
        return false;
    }

    if(port == 0){
        //TODO
    }

    ENetAddress localListeningAddress;

    if(enet_address_set_host(&localListeningAddress, qPrintable(localAddress.toString()))){
        qCritical()<<"ERROR! Failed to set host address!";
        return false;
    }
    //    enet_address_set_host(&localListeningAddress, localAddress.toString().toStdString().c_str());
    localListeningAddress.port = port;

    return listen(&localListeningAddress, maximumNumberOfPeers);

    //    localServer = enet_host_create(&localListeningAddress, maximumNumberOfPeers, 0, 0);
    //    if(localServer == 0)
    //    {
    //        qCritical()<<"ERROR! Failed to create ENET server!";
    //         enet_deinitialize();
    //        return false;
    //    }

    //    return true;
}


void ENETProtocolBasePrivate::close(){

    m_listening = false;
    while (m_threadCount) {
        //wait for other threads!
        QCoreApplication::processEvents();
        //qDebug()<<"Waiting for threads to quit ...";
        msleep(10);
    }

    if(localServer){
        enet_host_destroy(localServer);
        localServer = 0;
    }

    enet_deinitialize();

    peersHash.clear();

}

void ENETProtocolBasePrivate::startWaitingForIOInAnotherThread(unsigned int msecWaitForIOTimeout){
    QtConcurrent::run(this, &ENETProtocolBasePrivate::waitForIO, msecWaitForIOTimeout);
}


/////////////////////////////////
// http://lists.cubik.org/pipermail/enet-discuss/2014-November/002347.html
//Lee Salzman lsalzman at gmail.com
//Wed Nov 5 00:52:05 PST 2014
//Inside enet_host_service(), it may be accessing peers, so you would want to
//ensure that enet_peer_send() is not being used at that time and so would
//need one nasty global lock for that.
//When not inside a call to enet_host_service(), it is safe to call
//enet_peer_send() and friends on different peers at once,so long as it is
//not called on the same peer at one time.
//So stupid simplest way would just be to have the one lock guarding
//everything, host and all peers, and call it quits. More involved solution
//might be to have a reader-writer lock, with the enet_host_service() locking
//down things as a writer, and individual peer calls locking the RW lock as
//readers. Then each peer would be guarded by an individual mutex underneath
//that.

//On Tue, Nov 4, 2014 at 11:18 PM, Jason Lester <jasonrlester at yahoo.com>
//wrote:
//> I'm working on a service which dispatches new packets into a threadpool
//> including the active peer from event.peer. The system can then "respond" to
//> each packet within the threadpool based on event.peer if needed. System
//> uses a mutex to insure that only one thread can send at once (per peer) -
//> so recv'ing is synchronized as each recv is be done in main thread, and
//> send'ing is synchronized on per peer basis via per peer mutexs. System is
//> based around a Boost.Asio io_service and currently handles floods of
//> messages from 100 clients in various simulations without issue, processing
//> between 1000 - 10000 messages per second (thats total messages, not
//> messages per peer, with most messages having payloads of 16 - 128 bytes).
//>
//>
//> So far, everything is going good. However, I had to go against the grain
//> on this one with many people advising to find a "better" solution and many
//> (yet somehow always varying) suggestions that enet isn't threadsafe (even
//> though the main FAQ says that the host object is the only structure to
//> worry about). So my main question is if I'm missing something that my pop
//> up and bite me at some random point, like incorrect round trip times, lost
//> packets, lost peers, etc. If so, where would be the best place to lock
//> peers while recv'ing and send'ing within the enet lib itself?

void ENETProtocolBasePrivate::waitForIO(int msecTimeout){
    qDebug()<<"--ENETProtocolBasePrivate::waitForIO(...)";

    m_threadCount++;

    ENetEvent event;
    while(m_listening)
    {
        int eventsCount = enet_host_service(localServer, &event, msecTimeout);
        if(eventsCount < 0 ){
            qCritical()<<"ERROR! Error getting events!";
            continue;
        }
        if(eventsCount == 0){
            //qDebug()<<"No Event!";
            continue;
        }

        switch(event.type){
        case ENET_EVENT_TYPE_CONNECT:
        {
            ENetPeer *peer = event.peer;
            qDebug()<<"------CONNECT-----Peer:"<<peer<<" connectID:"<<peer->connectID;

            peersHash.insert(peer->connectID, peer);

            ENetAddress remote = peer->address;
            char ip[256];
            enet_address_get_host_ip(&remote,ip,256);

            emit connected(peer->connectID, QString(ip), remote.port);
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            QByteArray byteArray(reinterpret_cast<const char *>(event.packet->data), event.packet->dataLength);
            emit dataReceived(event.peer->connectID, byteArray);

            enet_packet_destroy(event.packet);
            break;
        }
        case  ENET_EVENT_TYPE_DISCONNECT: //失去连接
        {
            ENetPeer *peer = event.peer;
            qDebug()<<"------DISCONNECT-----Peer:"<<peer<<" connectID:"<<peer->connectID;

            ENetAddress remote = peer->address;
            char ip[256];
            enet_address_get_host_ip(&remote,ip,256);

            quint32 peerID = peersHash.key(event.peer);
            peersHash.remove(peerID);

            emit disconnected(peerID, QString(ip), remote.port);

            qDebug() <<"NO. " <<peer->data <<"Peer closed!" ;
            break;

        }
        default:
            qWarning("Unknown ENET event type received.");
            break;

        }

        m_threadCount--;

    }

    qDebug()<<"--------------Exit Function waitForIO---------------";

}

bool ENETProtocolBasePrivate::connectToHost(const QHostAddress &address, quint16 port, quint32 *peerID, unsigned int msecTimeout, quint32 channels){

    if(!localServer){
        qCritical()<<"ERROR! ENET Server is not running!";
        return false;
    }

    ENetAddress peerAddress;
    if(enet_address_set_host(&peerAddress, qPrintable(address.toString()))){
        qCritical()<<"ERROR! Failed to set host address!";
        return false;
    }
    peerAddress.port = port;

    ENetPeer *peer = enet_host_connect(localServer, &peerAddress, channels, 0);
    if(peer == NULL)
    {
        qCritical()<<"ERROR! Failed to connect to peer "<< address.toString();
        return false;
    }


    //TODO
    unsigned int timeBase = enet_time_get ();
    while (peer->state != ENET_PEER_STATE_CONNECTED) {
        //msleep(10);
        qApp->processEvents();
        if(enet_time_get() - timeBase > msecTimeout){
            qCritical()<<"ERROR! Connecting to peer timeout!";
            enet_peer_reset (peer);
            return false;
        }
    }


    //    ENetEvent event;
    //    if (enet_host_service(localServer, &event, msecTimeout) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    //    {
    //        return true;
    //    }else{
    //        qCritical()<<"ERROR! Failed to connect to peer!";
    //        enet_peer_reset (peer);
    //        return false;
    //    }

    if(peerID){
        *peerID = peer->connectID;
    }

    return true;


}

bool ENETProtocolBasePrivate::sendData(ENetPeer *peer, const QByteArray *byteArray, bool reliable, quint8 channel){

    ENetPacket *packet = enet_packet_create(byteArray->constData(), byteArray->size(), (reliable?ENET_PACKET_FLAG_RELIABLE:ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT) );
    int result = enet_peer_send(peer, channel, packet);
    if(result < 0){
        qCritical()<<"ERROR! Failed to send data!";
        enet_packet_destroy(packet);
        return false;
    }

    enet_host_flush (localServer);
//    enet_host_service(localServer, 0, 1);

    return true;
}

bool ENETProtocolBasePrivate::sendData(quint32 peerID, const QByteArray *byteArray, bool reliable, quint8 channel){
    ENetPeer *peer = peersHash.value(peerID);
    if(!peer){
        return false;
    }

    return sendData(peer, byteArray, reliable, channel);
}

void ENETProtocolBasePrivate::flush(){
    enet_host_flush (localServer);
}

void ENETProtocolBasePrivate::disconnectNow(ENetPeer *peer){

    ENetAddress remoteAddress = peer->address;
    quint32 peerID = peer->connectID;
    peersHash.remove(peerID);

    //No ENET_EVENT_DISCONNECT event will be generated.
    enet_peer_disconnect_now(peer, 0);

    char ip[256];
    enet_address_get_host_ip(&remoteAddress, ip, 256);
    emit disconnected(peerID, QString(ip), remoteAddress.port);

}



void ENETProtocolBasePrivate::disconnectNow(quint32 peerID){
    ENetPeer *peer = peersHash.value(peerID);
    if(!peer){
        return;
    }
    disconnectNow(peer);
}

void ENETProtocolBasePrivate::disconnect(ENetPeer *peer){
    //An ENET_EVENT_DISCONNECT event will be generated by enet_host_service() once the disconnection is complete.
    enet_peer_disconnect(peer, 0);
}

void ENETProtocolBasePrivate::disconnect(quint32 peerID){
    ENetPeer *peer = peersHash.value(peerID);
    if(!peer){
        return;
    }
    disconnect(peer);
}

void ENETProtocolBasePrivate::disconnectLater(ENetPeer *peer){
    //An ENET_EVENT_DISCONNECT event will be generated by enet_host_service() once the disconnection is complete.
    //All queued outgoing packets are sent.
    enet_peer_disconnect_later(peer, 0);
}

void ENETProtocolBasePrivate::disconnectLater(quint32 peerID){
    ENetPeer *peer = peersHash.value(peerID);
    if(!peer){
        return;
    }
    disconnectLater(peer);
}

bool ENETProtocolBasePrivate::listen(ENetAddress *localListeningAddress, unsigned int maximumNumberOfPeers){

    if(localServer){
        qCritical()<<"ERROR! ENET Server already listening!";
        return false;
    }

    unsigned int peers = (maximumNumberOfPeers>ENET_PROTOCOL_MAXIMUM_PEER_ID)?ENET_PROTOCOL_MAXIMUM_PEER_ID:maximumNumberOfPeers;
    localServer = enet_host_create(localListeningAddress, peers, 0, 0, 0);
    if(localServer == 0)
    {
        qCritical()<<"ERROR! Failed to create ENET server!";
        enet_deinitialize();
        return false;
    }

    m_listening = true;
    return m_listening;
}


inline void ENETProtocolBasePrivate::msleep(int msec){

#ifdef Q_OS_WIN32
    Sleep(msec);
#else
    usleep(msec*1000);
#endif

}

void ENETProtocolBasePrivate::setPeerPrivateData(ENetPeer *peer, void *data){
    if(!peer){
        return;
    }

    peer->data = data;

}





//////////////////////////////////////////////////////////////////////////////

ENETProtocolBase::ENETProtocolBase(QObject *parent) :
    QObject(parent)
{

    m_basePrivate = new ENETProtocolBasePrivate(this);

    connect(m_basePrivate, SIGNAL(connected(quint32,QString,quint16)), this, SIGNAL(connected(quint32,QString,quint16)));
    connect(m_basePrivate, SIGNAL(disconnected(quint32,QString,quint16)), this, SIGNAL(disconnected(quint32,QString,quint16)));
    connect(m_basePrivate, SIGNAL(dataReceived(quint32, const QByteArray &)), this, SLOT(processReceivedData(quint32, const QByteArray &)));


    if(enet_initialize())
    {
        qCritical()<<"ERROR! Failed to initialize ENET!";
    }

}

ENETProtocolBase::~ENETProtocolBase(){
    delete m_basePrivate;
}

bool ENETProtocolBase::isListening() const{
    return m_basePrivate->isListening();
}

bool ENETProtocolBase::getPeerAddressInfo(quint32 peerID, QString *address, quint16 *port){
    return m_basePrivate->getPeerAddressInfo(peerID, address, port);
}

bool ENETProtocolBase::getLocalListeningAddressInfo(QString *address, quint16 *port){
    return m_basePrivate->getLocalListeningAddressInfo(address, port);
}

bool ENETProtocolBase::listen(quint16 port, const QHostAddress &localAddress, unsigned int maximumNumberOfPeers){
    return m_basePrivate->listen(port, localAddress, maximumNumberOfPeers);
}

void ENETProtocolBase::close(){
    return m_basePrivate->close();
}

void ENETProtocolBase::startWaitingForIOInAnotherThread(unsigned int msecWaitForIOTimeout){
    m_basePrivate->startWaitingForIOInAnotherThread(msecWaitForIOTimeout);
}

bool ENETProtocolBase::connectToHost(const QHostAddress &address, quint16 port, quint32 *peerID, unsigned int msecTimeout, quint32 channels){
    return m_basePrivate->connectToHost(address, port, peerID, msecTimeout, channels);
}

bool ENETProtocolBase::sendData(quint32 peerID, const QByteArray *byteArray, bool reliable, quint8 channel){
    return m_basePrivate->sendData(peerID, byteArray, reliable, channel);
}

void ENETProtocolBase::flush(){
    m_basePrivate->flush();
}

void ENETProtocolBase::disconnectNow(quint32 peerID){
    m_basePrivate->disconnectNow(peerID);
}

void ENETProtocolBase::disconnect(quint32 peerID){
    m_basePrivate->disconnect(peerID);
}

void ENETProtocolBase::disconnectLater(quint32 peerID){
    m_basePrivate->disconnectLater(peerID);
}

void ENETProtocolBase::waitForIO(int msecTimeout){
    m_basePrivate->waitForIO(msecTimeout);
}
















} //namespace HEHUI
