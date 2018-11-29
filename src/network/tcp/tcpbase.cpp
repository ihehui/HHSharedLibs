
#include <QtConcurrentRun>
#include <QDataStream>
#include <QDateTime>

#include "tcpbase.h"

//#include "../packethandler/packet.h"


namespace HEHUI
{

TCPBase::TCPBase(QObject *parent) :
    QObject(parent)
{

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, SIGNAL(newConnection ()), this, SLOT(newIncomingConnection()));

    //m_lastSocketID = (std::numeric_limits<int>::min)();
    qsrand(QDateTime::currentDateTime().toTime_t());
    m_lastSocketID = 1 + (int)((1 << 30) * (double(qrand()) / RAND_MAX));


    m_proxy = QNetworkProxy::NoProxy;


}

TCPBase::~TCPBase()
{
    qDebug() << "--TCPBase::~TCPBase()";

    if(m_tcpServer->isListening()) {
        m_tcpServer->close();
    }
    m_tcpServer->disconnect();
    m_tcpServer->deleteLater();

    QMutexLocker locker(&mutex);
    foreach (QTcpSocket *socket, m_socketsHash.values()) {
        if(!socket) {
            continue;
        }
        socket->disconnect();
        socket->close();
        socket->deleteLater();
    }
    m_socketsHash.clear();

    m_socketBlockSizeInfoHash.clear();
    m_busySockets.clear();
}

bool TCPBase::listen ( const QHostAddress &address, quint16 port )
{
    return m_tcpServer->listen(address, port);
}

bool TCPBase::isListening () const
{
    return m_tcpServer->isListening();
}

void TCPBase::closeServer ()
{
    if(m_tcpServer->isListening()) {
        m_tcpServer->close();
    }
}

QString	TCPBase::errorString () const
{
    return m_tcpServer->errorString();
}

void TCPBase::serverAddressInfo (QHostAddress *address, quint16 *port )
{

    if(address) {
        *address = m_tcpServer->serverAddress();
    }
    if(port) {
        *port = m_tcpServer->serverPort();
    }
}

QAbstractSocket::SocketError TCPBase::serverError () const
{
    return m_tcpServer->serverError();
}

bool TCPBase::waitForNewConnection ( int msec, bool *timedOut )
{
    return m_tcpServer->waitForNewConnection(msec, timedOut);
}

QNetworkProxy TCPBase::proxy () const
{
    return m_proxy;
}

void TCPBase::setProxy ( const QNetworkProxy &networkProxy )
{
    this->m_proxy = networkProxy;
}

SOCKETID TCPBase::connectToHost( const QString &hostName, quint16 port, int waitMsecs)
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setProxy(m_proxy);
    connect(socket, SIGNAL(connected()), this, SLOT(peerConnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processSocketError(QAbstractSocket::SocketError)));

    SOCKETID socketID = 0;
    {
        QMutexLocker locker(&mutex);
        socketID = (++m_lastSocketID);
        m_socketsHash.insert(socketID, socket);
    }

    socket->connectToHost(hostName, port);
    if(waitMsecs) {
        socket->waitForConnected(waitMsecs);
    }

    return socketID;

}

SOCKETID TCPBase::connectToHost( const QHostAddress &address, quint16 port, int waitMsecs)
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setProxy(m_proxy);
    connect(socket, SIGNAL(connected()), this, SLOT(peerConnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processSocketError(QAbstractSocket::SocketError)));

    SOCKETID socketID = 0;
    {
        QMutexLocker locker(&mutex);
        socketID = (++m_lastSocketID);
        m_socketsHash.insert(socketID, socket);
    }

    socket->connectToHost(address, port);
    if(waitMsecs) {
        socket->waitForConnected(waitMsecs);
    }

    return socketID;

}

void TCPBase::disconnectFromHost (SOCKETID socketID, int waitMsecs)
{

    QTcpSocket *socket = 0;
    {
        QMutexLocker locker(&mutex);
        socket = m_socketsHash.value(socketID);
    }
    if(!socket) {
        return ;
    }

    socket->disconnectFromHost();
    if(waitMsecs) {
        socket->waitForDisconnected(waitMsecs);
    }

}

void TCPBase::abort(SOCKETID socketID)
{

    QMutexLocker locker(&mutex);
    QTcpSocket *socket = m_socketsHash.take(socketID);
    if(!socket) {
        return;
    }

    socket->abort();
    delete socket;
    socket = 0;

}

bool TCPBase::isConnected(SOCKETID socketID)
{

    QMutexLocker locker(&mutex);
    QTcpSocket *socket = m_socketsHash.value(socketID);
    if(!socket) {
        return false;
    }

    return (socket->state() == QAbstractSocket::ConnectedState);

}

bool TCPBase::isConnected(const QHostAddress &address, quint16 port)
{

    QMutexLocker locker(&mutex);
    foreach (QTcpSocket *socket, m_socketsHash.values()) {
        if(socket->peerAddress() == address && socket->peerPort() == port) {
            return (socket->state() == QAbstractSocket::ConnectedState);
        }
    }

    return false;

}

QAbstractSocket::SocketState TCPBase::socketState(SOCKETID socketID)
{

    QMutexLocker locker(&mutex);
    if(!m_socketsHash.contains(socketID)) {
        return QAbstractSocket::UnconnectedState;
    }

    return m_socketsHash.value(socketID)->state();

}

QAbstractSocket::SocketState TCPBase::socketState(const QHostAddress &address, quint16 port)
{

    QMutexLocker locker(&mutex);
    foreach (QTcpSocket *socket, m_socketsHash.values()) {
        if(socket->peerAddress() == address && socket->peerPort() == port) {
            return socket->state();
        }
    }

    return QAbstractSocket::UnconnectedState;

}

//void TCPBase::socketPeerAddressInfo(int socketID, QHostAddress *peerAddress, quint16 *peerPort ){

//    QTcpSocket *socket = 0;
//    {
//        QMutexLocker locker(&mutex);
//        socket = m_socketsHash.value(socketID);
//    }
//    if(!socket){return;}

//    if(peerAddress){
//        *peerAddress = socket->peerAddress();
//    }
//    if(peerPort){
//        *peerPort = socket->peerPort();
//    }

//}

//void TCPBase::socketLocalAddressInfo(int socketID, QHostAddress *localAddress, quint16 *localPort ){

//    QTcpSocket *socket = 0;
//    {
//        QMutexLocker locker(&mutex);
//        socket = m_socketsHash.value(socketID);
//    }
//    if(!socket){return;}

//    if(localAddress){
//        *localAddress = socket->localAddress();
//    }
//    if(localPort){
//        *localPort = socket->localPort();
//    }

//}

bool TCPBase::getAddressInfoFromSocket(SOCKETID socketID, QString *address, quint16 *port, bool getPeerInfo)
{

    QTcpSocket *socket = 0;
    {
        QMutexLocker locker(&mutex);
        socket = m_socketsHash.value(socketID);
    }
    if(!socket) {
        return false;
    }


    if(getPeerInfo) {
        if(address) {
            *address = socket->peerAddress().toString();
        }
        if(port) {
            *port = socket->peerPort();
        }
    } else {
        if(address) {
            *address = socket->localAddress().toString();
        }
        if(port) {
            *port = socket->localPort();
        }
    }

    return true;

}

QString	TCPBase::socketErrorString (SOCKETID socketID)
{
    QTcpSocket *socket = 0;
    {
        QMutexLocker locker(&mutex);
        socket = m_socketsHash.value(socketID);
    }
    if(!socket) {
        QString err = tr("No such socket '%1'!").arg(socketID);
        return err;
    }

    return socket->errorString();

}

bool TCPBase::sendData(SOCKETID socketID, const QByteArray *byteArray)
{
    //qDebug()<<"--TCPBase::sendData(...) "<<"socketID:"<< "byteArray->size():"<<byteArray->size();

    QTcpSocket *socket = 0;
    {
        QMutexLocker locker(&mutex);
        socket = m_socketsHash.value(socketID);
    }
    if(!socket) {
        return false;
    }
    //qDebug()<<"peerAddress:"<<socket->peerAddress()<<":"<<socket->peerPort();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_8);
    out << quint32(0) << *byteArray;

    out.device()->seek(0);

    int blcokSize = block.size();
    out << quint32(blcokSize - sizeof(quint32));

    return (socket->write(block) == blcokSize);

}


void TCPBase::newIncomingConnection()
{

    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    {
        QMutexLocker locker(&mutex);
        int socketID = (++m_lastSocketID);
        m_socketsHash.insert(socketID, socket);
    }
    setupNewSocket(socket);

}

void TCPBase::peerConnected ()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    setupNewSocket(socket);
}

void TCPBase::peerDisconnected ()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(!socket) {
        return;
    }

    int socketID = -1;
    {
        QMutexLocker locker(&mutex);
        socketID = m_socketsHash.key(socket);
    }

    m_socketBlockSizeInfoHash.remove(socketID);
    changeSocketBusyStatus(socketID, false);


    qDebug() << QString("Peer Disconnected! %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    emit disconnected(socketID, socket->peerAddress().toString(), socket->peerPort());

}

void TCPBase::processSocketError(QAbstractSocket::SocketError error)
{

    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(!socket) {
        return;
    }

    int socketID = -1;
    {
        QMutexLocker locker(&mutex);
        socketID = m_socketsHash.key(socket);
    }
    Q_ASSERT(socket);


    qDebug() << "TCP Socket Error! " << error << " socketID:" << socketID;

    emit socketError(socketID, error);

}

void TCPBase::setupNewSocket(QTcpSocket *socket)
{

    if(!socket) {
        return;
    }
    connect(socket, SIGNAL(disconnected()), this, SLOT(peerDisconnected()));
//    connect(socket, SIGNAL(dataReceived(const QByteArray &)), this, SLOT(dataReceived(const QByteArray &)));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processSocketError(QAbstractSocket::SocketError)));

    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(slotProcessSocketReadyRead()));


    int socketID = -1;
    {
        QMutexLocker locker(&mutex);
        socketID = m_socketsHash.key(socket);
    }
    Q_ASSERT(socketID > 0);

    m_socketBlockSizeInfoHash[socketID] = 0;
    //m_socketBlockSizeInfoHash.insert(socketID, 0);



    qDebug() << QString("Peer %1:%2 connected via TCP! ").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    emit connected(socketID, socket->peerAddress().toString(), socket->peerPort());

}

void TCPBase::slotProcessSocketReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if(!socket) {
        return;
    }

    int socketID = -1;
    {
        QMutexLocker locker(&mutex);
        socketID = m_socketsHash.key(socket);
    }
    Q_ASSERT(socketID > 0);


//    if(!isSocketBusy(socketID)){
//        changeSocketBusyStatus(socketID, true);
//        QtConcurrent::run(this, &TCPBase::readSocketdData, socketID, socket);
//    }

    readSocketdData(socketID, socket);

}

void TCPBase::readSocketdData(SOCKETID socketID, QTcpSocket *socket)
{
    //qDebug()<<"--TCPBase::readSocketdData(...) "<<" socketID:"<<socketID<<" peerAddress:"<<socket->peerAddress().toString();

    Q_ASSERT(socketID > 0);
    Q_ASSERT(socket);


    quint32 nextBlockSize = m_socketBlockSizeInfoHash.value(socketID);

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_8);
    forever {
        if (nextBlockSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint32)) {
                break;
            }
            in >> nextBlockSize;
            m_socketBlockSizeInfoHash[socketID] = nextBlockSize;
        }

//        if (nextBlockSize == PACKET_TERMINATING_SYMBOL) {
//            //TODO:是否需要关闭连接？
//            socket->close();
//            break;
//        }


        if (socket->bytesAvailable() < nextBlockSize) {
            break;
        }

        QByteArray buffer;
        buffer.resize(nextBlockSize);
        in >> buffer;

        //qDebug()<<QString("Data Received From %1:%2. Size:%3").arg(socket->peerAddress().toString()).arg(socket->peerPort()).arg(buffer.size());

        processData(socketID, &buffer);

        nextBlockSize = 0;
        m_socketBlockSizeInfoHash[socketID] = 0;
    }

    changeSocketBusyStatus(socketID, false);

}

//void TCPBase::dataReceived(const QByteArray &data){
//    qDebug()<<"--TCPBase::dataReceived(...) "<<" data.size():"<<data.size();

//    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
//    if(!socket){
//        return;
//    }
//    qDebug()<<QString("Data Received From %1:%2. Size:%3").arg(socket->peerAddress().toString()).arg(socket->peerPort()).arg(data.size());


//    int socketID = -1;
//    {
//        QMutexLocker locker(&mutex);
//        socketID = m_socketsHash.key(socket);
//    }
//    Q_ASSERT(socketID > 0);

//    processData(socketID, data);

//}

//void TCPBase::processData(quint32 socketID, QByteArray *data)
//{
//    QHostAddress address;
//    quint16 port = 0;
//    socketPeerAddressInfo(socketID, &address, &port);

//    QDataStream in(&data, QIODevice::ReadOnly);
//    in.setVersion(QDataStream::Qt_4_8);
//    QVariant v;
//    in >> v;
//    if (v.canConvert<Packet>()){
//        Packet *packet = PacketHandlerBase::getPacket(socketID);
//        *packet = v.value<Packet>();
//        packet->setTransmissionProtocol(TP_TCP);
//        //packet->setSocketID(socketID);

//        packet->setPeerHostAddress(address);
//        packet->setPeerHostPort(port);
////        packet->setLocalHostAddress(m_udpSocket->localAddress());
////        packet->setLocalHostPort(m_udpSocket->localPort());

////        m_packetHandlerBase->appendIncomingPacket(packet);

////        emit packetReceived(packet);
//    }
//}

bool TCPBase::isSocketBusy(SOCKETID socketID)
{
    QMutexLocker locket(&m_busyMutex);
    return m_busySockets.contains(socketID);
}

void TCPBase::changeSocketBusyStatus(SOCKETID socketID, bool busy)
{

    QMutexLocker locker(&m_busyMutex);
    if(busy) {
        m_busySockets.append(socketID);
    } else {
        m_busySockets.removeAll(socketID);
    }

}








} //namespace HEHUI
