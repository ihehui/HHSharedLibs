#ifndef TCPBASE_H
#define TCPBASE_H

#include <QObject>
#include <QHostAddress>
#include <QNetworkProxy>
#include <QMutex>
#include <QTcpServer>
#include <QTcpSocket>


#include "../networklib.h"
#include "../global_network.h"

namespace HEHUI {


class NETWORK_LIB_API TCPBase : public QObject
{
    Q_OBJECT
public:
    explicit TCPBase(QObject *parent = 0);
    ~TCPBase();

    /////////////////////////////
    bool listen ( const QHostAddress & address = QHostAddress::Any, quint16 port = 0 );
    bool isListening () const;
    void closeServer ();
    QString	errorString () const;
    void serverAddressInfo(QHostAddress *address, quint16 *port );
    QAbstractSocket::SocketError serverError () const;
    bool waitForNewConnection ( int msec = 0, bool * timedOut = 0 );

    QNetworkProxy proxy () const;
    void setProxy ( const QNetworkProxy & networkProxy );

    //////////////////////////////
    SOCKETID connectToHost ( const QString & hostName, quint16 port, int waitMsecs = 0);
    SOCKETID connectToHost ( const QHostAddress & address, quint16 port, int waitMsecs = 0);
    void disconnectFromHost (SOCKETID socketID, int waitMsecs = 0);
    void abort(SOCKETID socketID);
    bool isConnected(SOCKETID socketID);
    bool isConnected(const QHostAddress & address, quint16 port);
    QAbstractSocket::SocketState socketState(SOCKETID socketID);
    QAbstractSocket::SocketState socketState(const QHostAddress & address, quint16 port);
//    void socketPeerAddressInfo(int socketID, QHostAddress *peerAddress, quint16 *peerPort );
//    void socketLocalAddressInfo(int socketID, QHostAddress *localAddress, quint16 *localPort );
    bool getAddressInfoFromSocket(SOCKETID socketID, QString *address, quint16 *port, bool getPeerInfo = true);

    QString	socketErrorString (SOCKETID socketID);

    bool sendData(SOCKETID socketID, const QByteArray *byteArray);



signals:
    void connected (SOCKETID socketID, const QString &peerAddress, quint16 peerPort);
    void disconnected (SOCKETID socketID, const QString &peerAddress = "", quint16 peerPort = 0);
    void socketError(SOCKETID socketID, QAbstractSocket::SocketError error);

protected:

private slots:
    void newIncomingConnection();
    void peerConnected ();
    void peerDisconnected ();
    void processSocketError(QAbstractSocket::SocketError error);

    void setupNewSocket(QTcpSocket *socket);

    void slotProcessSocketReadyRead();
    void readSocketdData(SOCKETID socketID, QTcpSocket *socket);


//    void dataReceived(const QByteArray &data);

private:
    virtual void processData(quint32 socketID, QByteArray *data) = 0;

    bool isSocketBusy(SOCKETID socketID);
    void changeSocketBusyStatus(SOCKETID socketID, bool busy);

private:
    QTcpServer *m_tcpServer;
    SOCKETID m_lastSocketID;

    QHash<SOCKETID/*Socket ID*/, QTcpSocket*> m_socketsHash;
    QHash<SOCKETID/*Socket ID*/, quint32 /*Block Size*/> m_socketBlockSizeInfoHash;
    QList<SOCKETID/*Socket ID*/> m_busySockets;
    QMutex m_busyMutex;


    QNetworkProxy m_proxy;

    QMutex mutex;

    
};

} //namespace HEHUI

#endif // TCPBASE_H
