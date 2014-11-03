
/**********************************

//Server
RUDPSocket *serverRudpSocket = new RUDPSocket(this);
serverRudpSocket->bind(3000);


//Client
RUDPSocket *rudpSocket = new RUDPSocket(this);
QHostAddress m_peerAddress = QHostAddress("127.0.0.1");
quint16 m_peerPort = 3000;
rudpSocket->connectToPeer(m_peerAddress, m_peerPort);

//Send Method 1
QByteArray data;
data.resize(1024000);

quint16 fragmentDataID = rudpSocket->beginDataTransmission(m_peerAddress, m_peerPort);
int totalSent = 0;
int sent;
while (totalSent < size)
{
    qApp->processEvents();
    if (0 == (sent = rudpSocket->sendDatagram(m_peerAddress, m_peerPort, &data, totalSent, true)) )
    {
        //msleep(10);
    }

    totalSent += sent;
}
rudpSocket->endDataTransmission(m_peerAddress, m_peerPort, fragmentDataID);

**********************************/




#ifndef RUDPSOCKET_H
#define RUDPSOCKET_H

#include <QUdpSocket>

//#include "../networklib.h"

#include "rudpchannel.h"
#include "../packethandler/packethandlerbase.h"


namespace HEHUI{

class NETWORK_LIB_API RUDPSocket : public QUdpSocket
{
    Q_OBJECT
public:
    explicit RUDPSocket(PacketHandlerBase *packetHandlerBase, int keepAliveTimerInterval = RUDP_KEEPALIVE_TIMER_INTERVAL, QObject *parent = 0);

    ~RUDPSocket();

    bool isConnected(const QString &peerAddressString, quint16 peerPort);
    bool isConnected(const QHostAddress &peerAddress, quint16 peerPort);

//    bool canSendData(quint64 size);

    void run();

signals:
    void peerConnected(const QHostAddress &peerAddress, quint16 peerPort);
    void signalConnectToPeerTimeout(const QHostAddress &peerAddress, quint16 peerPort);
    void peerDisconnected(const QHostAddress &peerAddress, quint16 peerPort, bool normalClose);

    //void dataReceived(const QHostAddress &peerAddress, quint16 peerPort, const QByteArray &data);


public slots:
    void connectToPeer(const QString &peerAddressString, quint16 peerPort, bool wait = false, int msecTimeout = RUDP_CONNECTION_TIMEOUT);
    void connectToPeer(const QHostAddress &peerAddress, quint16 peerPort, bool wait = false, int msecTimeout = RUDP_CONNECTION_TIMEOUT);

    void disconnectFromPeer(const QHostAddress &peerAddress, quint16 peerPort);


//    quint16 beginDataTransmission(const QHostAddress &peerAddress, quint16 peerPort);
//    bool sendData(const QHostAddress &peerAddress, quint16 peerPort, QByteArray &data);
//    quint64 sendDatagram(const QHostAddress &peerAddress, quint16 peerPort, QByteArray *data, quint64 offset, bool fragment);
    quint64 sendDatagram(const QHostAddress &peerAddress, quint16 peerPort, QByteArray *data, bool isReliableDataPacket = true);

//    void endDataTransmission(const QHostAddress &peerAddress, quint16 peerPort, quint16 fragmentDataID);

    void closeChannel(const QHostAddress &peerAddress, quint16 peerPort);
    void closeAllChannels();
    void closeAllUnusedChannels();
    void setMaxCachedUnusedChannelsCount(int count);
    int getMaxCachedUnusedChannelsCount() const;

    void setMaxCachedUnusedPacketsCount(int count);
    int getMaxCachedUnusedPacketsCount() const;

private slots:
    void readPendingDatagrams();

    void channelclosed(const QHostAddress &peerAddress, quint16 peerPort, bool normalClose);

private:
    RUDPChannel * getRUDPChannel(const QHostAddress &hostAddress = QHostAddress::Null, quint16 port = 0);

    void recyleRUDPChannel(RUDPChannel *channel);

private:
    PacketHandlerBase *m_packetHandlerBase;

    QHash<QString/*IP:Port*/, RUDPChannel*> peers;

    QByteArray *datagram;

    QList<RUDPChannel*> m_unusedRUDPChannels;

    int m_keepAliveTimerInterval;

    int m_maxCachedUnusedChannelsCount;



};

typedef RUDPSocket RUDPServer;



class RUDPSocketThread : public QThread
{
    Q_OBJECT
public:
    RUDPSocketThread(PacketHandlerBase *packetHandlerBase, int keepAliveTimerInterval = RUDP_KEEPALIVE_TIMER_INTERVAL, QObject *parent = 0);
    virtual ~RUDPSocketThread();

    void run();

    RUDPSocket * getRUDPSocket();

private:
    RUDPSocket *m_rudpSocket;


};



} //namespace HEHUI

#endif // RUDPSOCKET_H
