/*
 * udtprotocolbase.h
 *
 *  Created on: 2010-12-15
 *      Author: hehui
 */

#ifndef UDTPROCOTOLBASESOCKET_H_
#define UDTPROCOTOLBASESOCKET_H_

#include <QObject>
#include <QHostAddress>
#include <QMutex>
#include <QHash>


//#ifdef Q_OS_WIN32
////if compiling on VC6.0 or pre-WindowsXP systems
////use -DLEGACY_WIN32
////if compiling with MinGW, UDT only works on XP or above
////use -D_WIN32_WINNT=0x0501
//    //#include <windows.h>
//    #include <ws2tcpip.h>
//    #include <winsock2.h>
//    #ifdef LEGACY_WIN32
//        #include <wspiapi.h>
//    #endif
//#else
//    #include <unistd.h>
//    #include <cstdlib>
//    #include <cstring>
//    #include <netdb.h>
//#endif

#ifndef MIN_THREAD_COUNT
    #define MIN_THREAD_COUNT 10
#endif

#ifndef MAX_DATA_BLOCK_SIZE
    #define MAX_DATA_BLOCK_SIZE 2048000
#endif








#include "udtlib.h"


namespace HEHUI
{

//typedef UDTSTATUS UDTSocketStatus;
typedef int UDTSOCKET;
class CCC;

class UDT_LIB_API UDTProtocolBase : public QObject
{
    Q_OBJECT
public:


    struct SocketOptions {
        SocketOptions()
        {
#ifdef WIN32
            UDT_MSS = 1052;
#else
            UDT_MSS = 1500;
#endif

            UDT_SNDSYN = false; //UDT Default:true
            UDT_RCVSYN = false; //UDT Default:true
            UDT_CC = 0;
            UDT_FC = 25600;

            UDT_SNDBUF = 10240000; //UDT Default:10M
            UDT_RCVBUF = 10240000;
            UDP_SNDBUF = 1024000; //UDT Default:1M
            UDP_RCVBUF = 1024000;

            //UDT_LINGER.l_linger = 180;
            UDT_RENDEZVOUS = false;

            UDT_SNDTIMEO = 30000; //UDT Default:-1
            UDT_RCVTIMEO = 30000; //UDT Default:-1

            UDT_REUSEADDR = true;
            UDT_MAXBW = -1;

            UDT_STATE = 0;
            UDT_EVENT = 0;

        }

        int UDT_MSS;
        bool UDT_SNDSYN;
        bool UDT_RCVSYN;
        CCC *UDT_CC;
        int UDT_FC;

        int UDT_SNDBUF;
        int UDT_RCVBUF;
        int UDP_SNDBUF;
        int UDP_RCVBUF;

        //linger UDT_LINGER;
        bool UDT_RENDEZVOUS;

        int UDT_SNDTIMEO;
        int UDT_RCVTIMEO;

        bool UDT_REUSEADDR;
        qint64 UDT_MAXBW;

        qint32 UDT_STATE; //Read only!
        qint32 UDT_EVENT; //Read only!

    };

    UDTProtocolBase(bool stream = true, const SocketOptions *options = 0, QObject *parent = 0);
    virtual ~UDTProtocolBase();

    static const UDTSOCKET INVALID_UDT_SOCK;

    //    void setSocketOption(UDT::SOCKOPT optname, const char* optval, int optlen);
    void setDefaultSocketOptions(const SocketOptions *options);
    SocketOptions getDefaultSocketOptions() const;

    bool isStreamMode()
    {
        return m_stream;
    }
    UDTSOCKET getServerSocket()
    {
        return serverSocket;
    }

    bool getAddressInfoFromSocket(UDTSOCKET socket, QString *address, quint16 *port, bool getPeerInfo = true);

    //UDTSocketStatus getUDTSocketStatus(UDTSOCKET socket);
    bool isSocketListening(UDTSOCKET socket);
    bool isConnecting(UDTSOCKET socket);
    bool isSocketConnected(UDTSOCKET socket);
    bool isSocketBroken(UDTSOCKET socket);
    bool isSocketExist(UDTSOCKET socket);

    QString getUDTListeningAddress();
    quint16 getUDTListeningPort();
    virtual QString getLastErrorMessage() const;

signals:
    void connected(int socket, const QString &address, quint16 port);
    void disconnected(int socket);

    void fileDataSent(int socket, const QString &filePath, qint64 offset, qint64 sizeSent, const QString &errorMessage);
    void fileDataReceived(int socket, const QString &fileSavePath, qint64 offset, qint64 sizeReceived, const QString &errorMessage);


public slots:
    //Start the server to listen,  implement the virtual function startWaitingForIO()
    UDTSOCKET listen(quint16 port = 0, const QHostAddress &localAddress = QHostAddress::Any);

    //Close the server
    void close();

    //Call this function after server is listening
    void startWaitingForNewConnectionInOneThread(int msecWaitForNewConnectionTimeout = 1);
    void startWaitingForIOInSeparateThread(int msecWaitForInputTimeout = 1, int msecWaitForOutputTimeout = 1);
    void startWaitingForIOInOneThread(int msecWaitForIOTimeout = 1);

    //Connect to peer
    UDTSOCKET connectToHost(const QHostAddress &address, quint16 port, SocketOptions *options = 0, bool waitWhileConnecting = true, int msecTimeout = 5000, bool monitor = true);

    //Close peer socket
    void closeSocket(UDTSOCKET socket);

    //Send data
    virtual bool sendData(UDTSOCKET socket, const QByteArray *byteArray);
    bool sendUDTStreamData(UDTSOCKET socket, const QByteArray *byteArray);
    bool sendUDTMessageData(UDTSOCKET socket, const QByteArray *byteArray, int ttl = -1, bool inorder = true);


protected:
    void waitForIO(int msecTimeout = 1);

    void waitForReading(int msecTimeout = 1);
    void waitForWriting(int msecTimeout = 1);
    void waitingForNewConnection(int msecTimeout = 1);

    UDTSOCKET acceptNewConnection();

    //Send & Receive File
//    qint64 sendFile(UDTSOCKET socket, const QString &filePath, qint64 offset, qint64 size, int blockSize = 8192);
//    qint64 receiveFile(UDTSOCKET socket, const QString &fileSavePath, qint64 offset, qint64 size, int blockSize = 8192);


private slots:
    void processSocketReadyToRead(UDTSOCKET socket);
    void processSocketReadyToWrite(UDTSOCKET socket);

    void processStreamDataAfterReceived(UDTSOCKET socket, QByteArray *byteArray);

    virtual void streamDataReceived(UDTSOCKET socket, QByteArray *data) = 0;
    virtual void messageDataReceived(UDTSOCKET socket, QByteArray *data) = 0;

private:
    //struct CachedDataInfo;
    void recycleCachedData(QByteArray *data);
    QByteArray *getCachedData();

    //UDT::ERRORINFO getLastErrorInfo(){return UDT::getlasterror();}
    void setSocketOptions(UDTSOCKET socket, SocketOptions *options = 0);

    void msleep(int msec);

    bool addSocketToEpoll(UDTSOCKET socket);
    void removeSocketFromEpoll(UDTSOCKET socket);


private:
    int epollID;
    UDTSOCKET serverSocket;

    bool m_listening;
    bool m_stream;
    QString m_serverAddress;
    quint16 m_serverPort;


    SocketOptions m_socketOptions;

    QHash<UDTSOCKET, QByteArray *> m_cachedDataInfoHash;
    QList<QByteArray *> m_unusedCached;

    QString m_errorMessage;

    int m_threadCount;

    QMutex m_epollMutex;


};

}

#endif /* ABSTRACTUDTSOCKET_H_ */
