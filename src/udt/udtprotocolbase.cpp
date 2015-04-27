/*
 * udtprotocolbase.cpp
 *
 *  Created on: 2010-12-15
 *      Author: hehui
 */





#include <QCoreApplication>
#include <QDataStream>
#include <QtConcurrentRun>
#include <QDebug>
#include <QFile>
#include <QDateTime>


#ifdef Q_OS_WIN32
//if compiling on VC6.0 or pre-WindowsXP systems
//use -DLEGACY_WIN32
//if compiling with MinGW, UDT only works on XP or above
//use -D_WIN32_WINNT=0x0501
    //#include <windows.h>
    #include <ws2tcpip.h>
    #include <winsock2.h>
    #ifdef LEGACY_WIN32
        #include <wspiapi.h>
    #endif
#else
    #include <unistd.h>
    #include <cstdlib>
    #include <cstring>
    #include <netdb.h>
#endif


#include "udtprotocolbase.h"

#include "./udt/src/udt.h"
#include "./udt/src/ccc.h"

#include <iostream>
using namespace std;


//#ifdef Q_CC_MSVC
//#include <windows.h>
//#define msleep(x) Sleep(x)
//#endif

//#ifdef Q_CC_GNU
//#include <unistd.h>
//#define msleep(x) usleep(x*1000)
//#endif




namespace HEHUI {


const UDTSOCKET UDTProtocolBase::INVALID_UDT_SOCK = UDT::INVALID_SOCK;

UDTProtocolBase::UDTProtocolBase(bool stream, const SocketOptions *options, QObject *parent)
    :QObject(parent), m_stream(stream)
{

    // use this function to initialize the UDT library
    UDT::startup();


    epollID = 0;
    serverSocket = UDT::INVALID_SOCK;

    m_listening = false;
    m_serverAddress = "";
    m_serverPort = 0;

    if(options){
        m_socketOptions = *options;
    }

    //IMPORTANT For Multi-thread
    if(QThreadPool::globalInstance()->maxThreadCount() < MIN_THREAD_COUNT){
        QThreadPool::globalInstance()->setMaxThreadCount(MIN_THREAD_COUNT);
    }

    m_errorMessage = "";

    m_threadCount = 0;


}

UDTProtocolBase::~UDTProtocolBase() {

    if(m_listening){
        close();
    }

    // use this function to release the UDT library
    UDT::cleanup();

}

//void AbstractUDTSocket::setSocketOption(UDT::SOCKOPT optname, const char* optval, int optlen){

//}

void UDTProtocolBase::setDefaultSocketOptions(const SocketOptions *options){

    this->m_socketOptions = *options;
}

UDTProtocolBase::SocketOptions UDTProtocolBase::getDefaultSocketOptions() const{

    return m_socketOptions;

}

UDTSOCKET UDTProtocolBase::listen(quint16 port, const QHostAddress &localAddress){
    qDebug()<<"--UDTProtocolBase::listen(...) "<<localAddress.toString()<<":"<<port;

    m_errorMessage = "";

    if(UDT::getsockstate(serverSocket) == LISTENING){
        m_listening = true;
        m_errorMessage = tr("Server is already listenning on %1:%2!").arg(m_serverAddress).arg(m_serverPort);
        qWarning()<<m_errorMessage;
        return UDT::INVALID_SOCK;
    }

    addrinfo hints;
    addrinfo* localAddressInfo;
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = m_stream?SOCK_STREAM:SOCK_DGRAM;

    if (0 != getaddrinfo(localAddress.toString().toStdString().c_str(), QString::number(port).toStdString().c_str(), &hints, &localAddressInfo))
        //if (0 != getaddrinfo(NULL, QString::number(port).toStdString().c_str(), &hints, &res))
    {
        m_errorMessage = tr("Failed to start listening! Illegal port number or port is busy!");
        qDebug()<<m_errorMessage;
        //cout << "illegal port number or port is busy.\n" << endl;
        freeaddrinfo(localAddressInfo);
        return UDT::INVALID_SOCK;
    }

    serverSocket = UDT::socket(localAddressInfo->ai_family, localAddressInfo->ai_socktype, localAddressInfo->ai_protocol);

    // UDT Options
    //TODO: setup UDT_MSS
//    UDT::setsockopt(serverSocket, 0, UDT_MSS, &(m_socketOptions.UDT_MSS), sizeof(int));
//    UDT::setsockopt(serverSocket, 0, UDT_SNDSYN, &(m_socketOptions.UDT_SNDSYN), sizeof(bool));
//    UDT::setsockopt(serverSocket, 0, UDT_RCVSYN, &(m_socketOptions.UDT_RCVSYN), sizeof(bool));
//    if(m_socketOptions.UDT_CC){
//        //UDT::setsockopt(serv, 0, UDT_CC, new CCCFactory<CUDPBlast>, sizeof(CCCFactory<CUDPBlast>));
//        //UDT::setsockopt(serv, 0, UDT_CC, &(m_socketOptions.UDT_CC), sizeof(CCC));
//    }
//    UDT::setsockopt(serverSocket, 0, UDT_FC, &(m_socketOptions.UDT_FC), sizeof(int));
//    UDT::setsockopt(serverSocket, 0, UDT_SNDBUF, &(m_socketOptions.UDT_SNDBUF), sizeof(int));
//    UDT::setsockopt(serverSocket, 0, UDT_RCVBUF, &(m_socketOptions.UDT_RCVBUF), sizeof(int));
//    UDT::setsockopt(serverSocket, 0, UDP_SNDBUF, &(m_socketOptions.UDP_SNDBUF), sizeof(int));
//    UDT::setsockopt(serverSocket, 0, UDP_RCVBUF, &(m_socketOptions.UDP_RCVBUF), sizeof(int));
//    UDT::setsockopt(serverSocket, 0, UDT_LINGER, &(m_socketOptions.UDT_LINGER), sizeof(linger));
//    UDT::setsockopt(serverSocket, 0, UDT_RENDEZVOUS, &(m_socketOptions.UDT_RENDEZVOUS), sizeof(bool));
//    UDT::setsockopt(serverSocket, 0, UDT_SNDTIMEO, &(m_socketOptions.UDT_SNDTIMEO), sizeof(int));
//    UDT::setsockopt(serverSocket, 0, UDT_RCVTIMEO, &(m_socketOptions.UDT_RCVTIMEO), sizeof(int));
//    UDT::setsockopt(serverSocket, 0, UDT_REUSEADDR, &(m_socketOptions.UDT_REUSEADDR), sizeof(bool));
//    UDT::setsockopt(serverSocket, 0, UDT_MAXBW, &(m_socketOptions.UDT_MAXBW), sizeof(int64_t));

    // Setup UDT Options
    setSocketOptions(serverSocket, &m_socketOptions);

//    bool sync = true;
//    int size = 0;
//    UDT::getsockopt(serverSocket, 0, UDT_SNDSYN, &sync, &size);
//    qDebug()<<"---------UDT_SNDSYN:"<<sync;
//    UDT::getsockopt(serverSocket, 0, UDT_RCVSYN, &sync, &size);
//    qDebug()<<"---------UDT_RCVSYN:"<<sync;
//    int time = 0;
//    UDT::getsockopt(serverSocket, 0, UDT_RCVTIMEO, &time, &size);
//        qDebug()<<"---------UDT_RCVTIMEO:"<<time;
//        UDT::getsockopt(serverSocket, 0, UDT_SNDTIMEO, &time, &size);
//        qDebug()<<"---------UDT_SNDTIMEO:"<<time;


    if (UDT::ERROR == UDT::bind(serverSocket, localAddressInfo->ai_addr, localAddressInfo->ai_addrlen))
    {
        m_errorMessage = tr("Failed to start listening! Bind error! ") + UDT::getlasterror().getErrorMessage();
        qDebug()<<m_errorMessage;
        //cout << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
        freeaddrinfo(localAddressInfo);

        //TODO:Close the socket
        UDT::close(serverSocket);
        serverSocket = UDT::INVALID_SOCK;
        return UDT::INVALID_SOCK;
    }
    freeaddrinfo(localAddressInfo);


    if (UDT::ERROR == UDT::listen(serverSocket, 10))
    {
        m_errorMessage = tr("Failed to start listening! ") + UDT::getlasterror().getErrorMessage();
        qDebug()<<m_errorMessage;
        //cout << "listen: " << UDT::getlasterror().getErrorMessage() << endl;

        //TODO:Close the socket
        UDT::close(serverSocket);
        serverSocket = UDT::INVALID_SOCK;
        return UDT::INVALID_SOCK;
    }

    epollID = UDT::epoll_create();
    if(epollID < 0){
        m_errorMessage = tr("Failed to start listening! ") + UDT::getlasterror().getErrorMessage();
        qDebug()<<m_errorMessage;
        //cout << "UDT::epoll_create: " << UDT::getlasterror().getErrorMessage() << endl;

        epollID = 0;
        //TODO:Close the socket
        UDT::close(serverSocket);
        serverSocket = UDT::INVALID_SOCK;
        return UDT::INVALID_SOCK;
    }
    //High CPU Usage
    //UDT::epoll_add_usock(epollID, serverSocket);


    m_listening = true;

    getAddressInfoFromSocket(serverSocket, &m_serverAddress, &m_serverPort, false);
    qDebug()<<QString("UDT Server is ready on %1:%2. Listening socket ID:%3").arg(m_serverAddress).arg(m_serverPort).arg(serverSocket);


    //Call startWaitingForIO() to wait for IO

    return serverSocket;

}

void UDTProtocolBase::close(){
    qDebug()<<"--UDTProtocolBase::close()";

    m_listening = false;
    while (m_threadCount) {
        //wait for other threads!
        QCoreApplication::processEvents();
        //qDebug()<<"Waiting for threads to quit ...";
        msleep(10);
    }

    m_serverAddress = "";
    m_serverPort = 0;

    UDT::close(serverSocket);
    serverSocket = UDT::INVALID_SOCK;

    UDT::epoll_release(epollID);
    epollID = 0;


    foreach (QByteArray *info, m_cachedDataInfoHash) {
        recycleCachedData(info);
    }
    m_cachedDataInfoHash.clear();

    foreach (QByteArray *info, m_unusedCached) {
        delete info;
    }
    m_unusedCached.clear();


}

void UDTProtocolBase::startWaitingForNewConnectionInOneThread(int msecWaitForNewConnectionTimeout){
    QThreadPool * pool = QThreadPool::globalInstance();
    int maxThreadCount = pool->maxThreadCount();
    if(pool->activeThreadCount() == pool->maxThreadCount()){
        pool->setMaxThreadCount(++maxThreadCount);
    }
    QtConcurrent::run(this, &UDTProtocolBase::waitingForNewConnection, msecWaitForNewConnectionTimeout);
}

void UDTProtocolBase::startWaitingForIOInSeparateThread(int msecWaitForInputTimeout, int msecWaitForOutputTimeout){
    qDebug()<<"--UDTProtocolBase::startWaitingForIOInSeparateThread(...) "<<" msecWaitForInputTimeout:"<<msecWaitForInputTimeout<<" msecWaitForOutputTimeout:"<<msecWaitForOutputTimeout;

    QThreadPool * pool = QThreadPool::globalInstance();
    int maxThreadCount = pool->maxThreadCount();
    if(pool->activeThreadCount() == pool->maxThreadCount()){
        pool->setMaxThreadCount(maxThreadCount + 2);
    }
    QtConcurrent::run(this, &UDTProtocolBase::waitForReading, msecWaitForInputTimeout);
    QtConcurrent::run(this, &UDTProtocolBase::waitForWriting, msecWaitForOutputTimeout);

}

void UDTProtocolBase::startWaitingForIOInOneThread(int msecWaitForIOTimeout){
    QThreadPool * pool = QThreadPool::globalInstance();
    int maxThreadCount = pool->maxThreadCount();
    if(pool->activeThreadCount() == pool->maxThreadCount()){
        pool->setMaxThreadCount(++maxThreadCount);
    }
    QtConcurrent::run(this, &UDTProtocolBase::waitForIO, msecWaitForIOTimeout);

}

//qint64 UDTProtocolBase::sendFile(UDTSOCKET socket, const QString &filePath, qint64 offset, qint64 size, int blockSize){

////    if(!QFile::exists(filePath)){
////        m_errorMessage = tr("File '%1' does not exist").arg(filePath);
////        qCritical()<<m_errorMessage;

////        emit fileDataSent(socket, filePath, offset, -1, m_errorMessage);
////        return -1;
////    }

//    // open the file
//    fstream ifs(filePath.toStdString().c_str(), ios::in | ios::binary);

//    //ifs.seekg(0, ios::end);
//    //int64_t size = ifs.tellg();
//    ifs.seekg(0, ios::beg);

//    UDT::TRACEINFO trace;
//    UDT::perfmon(socket, &trace);

//    // send the file
//    //int64_t offset = 0;
//    qint64 sentSize = -1;
//    if (UDT::ERROR == (sentSize = UDT::sendfile(socket, ifs, offset, size, blockSize)) )
//    {
//        m_errorMessage = tr("Failed to send file! ") + UDT::getlasterror().getErrorMessage();
//        qCritical()<<m_errorMessage;
//        //cout << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;

//        emit fileDataSent(socket, filePath, offset, sentSize, m_errorMessage);
//        return sentSize;
//    }

//    UDT::perfmon(socket, &trace);
//    cout << "speed = " << trace.mbpsSendRate/8 << "MB/sec" << endl;

//    ifs.close();

//    emit fileDataSent(socket, filePath, offset, sentSize, "");

//    return sentSize;

//}

//qint64 UDTProtocolBase::receiveFile(UDTSOCKET socket, const QString &fileSavePath, qint64 offset, qint64 size, int blockSize){


//    // receive the file
//    fstream ofs(fileSavePath.toStdString().c_str(), ios::out | ios::binary | ios::trunc);
//    int64_t recvsize = -1;
//    //int64_t offset = 0;

//    if (UDT::ERROR == (recvsize = UDT::recvfile(socket, ofs, offset, size, blockSize)))
//    {
//        m_errorMessage = tr("Failed to receive file! ") + UDT::getlasterror().getErrorMessage();
//        qCritical()<<m_errorMessage;
//        //cout << "recvfile: " << UDT::getlasterror().getErrorMessage() << endl;

//        emit fileDataReceived(socket, fileSavePath, offset, recvsize, m_errorMessage);
//        return recvsize;
//    }

//    ofs.close();

//    emit fileDataReceived(socket, fileSavePath, offset, recvsize, "");

//    return recvsize;

//}



UDTSOCKET UDTProtocolBase::connectToHost(const QHostAddress &address, quint16 port, SocketOptions *options, bool waitWhileConnecting, int msecTimeout, bool monitor){
    qDebug()<<"--UDTProtocolBase::connectToHost(...)" <<address.toString()<<":"<<port;

    m_errorMessage = "";

    if(address.isNull() || address == QHostAddress::Any){
        m_errorMessage = tr("Can not connect to host! Invalid peer address!");
        qDebug()<<m_errorMessage;

        return UDT::INVALID_SOCK;
    }
    if(!port){
        m_errorMessage = tr("Can not connect to host! Invalid peer port!");
        qDebug()<<m_errorMessage;

        return UDT::INVALID_SOCK;
    }

    if(!epollID){
        if(UDT::INVALID_SOCK == listen()){
            return UDT::INVALID_SOCK;
        }
    }
    Q_ASSERT_X(epollID, "epollID", "ERROR! EPOLL Not Initialized!");


    // use this function to initialize the UDT library
    //UDT::startup();

    struct addrinfo hints, *local, *peer;
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = m_stream?SOCK_STREAM:SOCK_DGRAM;

    if (0 != getaddrinfo(address.toString().toStdString().c_str(), QString::number(port).toStdString().c_str(), &hints, &peer))
    {
        m_errorMessage = tr("Can not connect to host! Invalid peer address or port!");
        qDebug()<<m_errorMessage;

        freeaddrinfo(peer);
        return UDT::INVALID_SOCK;
    }


    if (0 != getaddrinfo(m_serverAddress.toStdString().c_str(), QString::number(m_serverPort).toStdString().c_str(), &hints, &local))
    {
        m_errorMessage = tr("Can not connect to host! Invalid local address or port!!");
        qDebug()<<m_errorMessage;

        freeaddrinfo(local);
        freeaddrinfo(peer);

        return UDT::INVALID_SOCK;
    }


    UDTSOCKET client = UDT::socket(local->ai_family, local->ai_socktype, local->ai_protocol);

    // Setup UDT Options
    setSocketOptions(client, options);

    if (UDT::ERROR == UDT::bind(client, local->ai_addr, local->ai_addrlen))
    {
        m_errorMessage = tr("Can not connect to host! Failed to bind!") + UDT::getlasterror().getErrorMessage();
        qDebug()<<m_errorMessage;

        freeaddrinfo(local);
        freeaddrinfo(peer);

        //TODO:Close the socket
        UDT::close(client);
        return UDT::INVALID_SOCK;
    }
    freeaddrinfo(local);


    if(monitor){
        //UDT::epoll_add_usock(epollID, client);
        if(!addSocketToEpoll(client)){
            UDT::close(client);
            return UDT::INVALID_SOCK;
        }
    }


    if (UDT::ERROR == UDT::connect(client, peer->ai_addr, peer->ai_addrlen))
    {
        m_errorMessage = tr("Can not connect to host! Failed to connect!") + UDT::getlasterror().getErrorMessage();
        qDebug()<<m_errorMessage;

        freeaddrinfo(peer);

        //TODO:Close the socket
        UDT::close(client);
        return UDT::INVALID_SOCK;
    }
    freeaddrinfo(peer);

    if(waitWhileConnecting){
        QDateTime time = QDateTime::currentDateTime();
        while (UDT::getsockstate(client) == CONNECTING) {

//            UDTSTATUS state = NONEXIST;
//            int size = 0;
//            UDT::getsockopt(client, 0, UDT_STATE, &state, &size);

            if(time.addMSecs(msecTimeout) < QDateTime::currentDateTime()){
                break;
            }

            QCoreApplication::processEvents();
            msleep(10);
            //qDebug()<<QString("Connecting to %1:%2 ...").arg(address.toString()).arg(port);
        }
    }


    return client;

}

void UDTProtocolBase::closeSocket(UDTSOCKET socket){
    qDebug()<<"--UDTProtocolBase::closeSocket(...) "<<"socket:"<<socket<<" ThreadID:"<<QThread::currentThreadId();

    QMutexLocker locker(&m_epollMutex);

//    if( (UDT::INVALID_SOCK == socket) || (UDT::getsockstate(socket) == NONEXIST) ){
//        qDebug()<<"INVALID_SOCK";
//        return;
//    }

    if(UDT::close(socket) == UDT::ERROR){
        qCritical()<<QString("ERROR! UDT::close: %1 Socket: %2 ").arg(UDT::getlasterror().getErrorMessage()).arg(socket)<<"Thread: "<<(QThread::currentThreadId());
        //fprintf(stderr, "ERROR! UDT::close: %s\n", UDT::getlasterror().getErrorMessage());
    }

    if(UDT::epoll_remove_usock(epollID, socket) == UDT::ERROR){
        qCritical()<<QString("ERROR! epoll_remove_usock: %1 Socket: %2 ").arg(UDT::getlasterror().getErrorMessage()).arg(socket) <<"Thread: "<<(QThread::currentThreadId());
        //fprintf(stderr, "ERROR! epoll_remove_usock: %s\n", UDT::getlasterror().getErrorMessage());
    }

    QByteArray *data = m_cachedDataInfoHash.take(socket);
    recycleCachedData(data);

}

bool UDTProtocolBase::sendData(UDTSOCKET socket, const QByteArray *byteArray){

    if(isStreamMode()){
        return sendUDTStreamData(socket, byteArray);
    }else{
        return sendUDTMessageData(socket, byteArray);
    }

}

bool UDTProtocolBase::sendUDTStreamData(UDTSOCKET socket, const QByteArray *byteArray){
//    qDebug()<<"--UDTProtocolBase::sendUDTStreamData(...) " <<"socket:"<<socket<<" szie:"<<byteArray->size();

    m_errorMessage = "";

    if(UDT::INVALID_SOCK == socket){
        m_errorMessage = tr("Invalid UDT Socket!");
        qDebug()<<m_errorMessage;
        return false;
    }

    Q_ASSERT_X(epollID, "epollID", "ERROR! EPOLL Not Initialized!");
    if(!epollID){
        m_errorMessage = tr("EPOLL Not Initialized!");
        qDebug()<<m_errorMessage;

        return false;
    }

    if(byteArray->size() > MAX_DATA_BLOCK_SIZE){
        m_errorMessage = tr("Data is too large! Max allowed size:%1, current data size:%2").arg(MAX_DATA_BLOCK_SIZE).arg(byteArray->size());
        qDebug()<<m_errorMessage;

        return false;
    }


    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_8);
    out << *byteArray ;

    int size = block.size();
    const char* data = block.constData();

    //    int size = byteArray.size();
    //    const char* data = byteArray.constData();

    int ssize = 0;
    int ss;
    while (ssize < size)
    {
        if (UDT::ERROR == (ss = UDT::send(socket, data + ssize, size - ssize, 0)))
        {
            m_errorMessage = tr("Data sent failed! Data size:%1, Sent size:%2! %3").arg(size).arg(ssize).arg(UDT::getlasterror().getErrorMessage());
            //qDebug()<<m_errorMessage;

            int errorCode = UDT::getlasterror().getErrorCode();
            if(errorCode == UDT::ERRORINFO::EASYNCSND){
                continue;
            }
            //break;
            return false;
        }

        ssize += ss;

        //QCoreApplication::processEvents();
    }

//    if (ssize < size){
//        m_errorMessage = tr("Data sent failed! Data size:%1, Sent size:%2! %3").arg(size).arg(ssize).arg(UDT::getlasterror().getErrorMessage());
//        qDebug()<<m_errorMessage;

//        return false;
//    }

    //qDebug()<<"--UDT Stream Data Sent!"<<" Size:"<<ssize;
    return true;

}

bool UDTProtocolBase::sendUDTMessageData(UDTSOCKET socket, const QByteArray *byteArray, int ttl, bool inorder){
    qDebug()<<"--UDTProtocolBase::sendUDTMessageData(...) " <<" socket:"<<socket<<" szie:"<<byteArray->size();

    m_errorMessage = "";

    if(UDT::INVALID_SOCK == socket){
        m_errorMessage = tr("Invalid UDT Socket!");
        qDebug()<<m_errorMessage;
        return false;
    }

    Q_ASSERT_X(epollID, "epollID", "ERROR! EPOLL Not Initialized!");
    if(!epollID){
        m_errorMessage = tr("EPOLL Not Initialized!");
        qDebug()<<m_errorMessage;
        return false;
    }


    int size = byteArray->size();
    if(size > MAX_DATA_BLOCK_SIZE){
        m_errorMessage = tr("Data is too large! Max allowed size:%1, current data size:%2").arg(MAX_DATA_BLOCK_SIZE).arg(size);
        qDebug()<<m_errorMessage;
        return false;
    }
    const char* data = byteArray->constData();

    int ss = UDT::sendmsg(socket, data, size, ttl, inorder);

    if (UDT::ERROR == ss || 0 == ss)
    {
        m_errorMessage = tr("Failed to send message data! ") + UDT::getlasterror().getErrorMessage();
        qDebug()<<m_errorMessage;

        return false;
    }

    qDebug()<<"--UDT Message Data Sent!"<<" Size:"<<ss;


    return true;

}

void UDTProtocolBase::waitForIO(int msecWaitForIOTimeout){
    qDebug()<<"--UDTProtocolBase::waitForIO(...) "<<" msecWaitForIOTimeout:"<<msecWaitForIOTimeout<<" Thread Id:"<<QThread::currentThreadId();

    set<UDTSOCKET> readfds, writefds;
    int count = 0;

    bool waitForSocketReadyToRead = false;

    m_threadCount++;
    while(m_listening){
        acceptNewConnection();

        //beginTime = QDateTime::currentDateTime();

        if(waitForSocketReadyToRead){
            count = UDT::epoll_wait(epollID, &readfds, &writefds, msecWaitForIOTimeout);
            waitForSocketReadyToRead = false;
        }else{
            count = UDT::epoll_wait(epollID, &readfds, 0, msecWaitForIOTimeout);
            waitForSocketReadyToRead = true;
        }

        if(count > 0){
            //qDebug()<<QString("epoll returned %1 sockets ready to IO | %2 in read set, %3 in write set").arg(count).arg(readfds.size()).arg(writefds.size());
            //printf("epoll returned %d sockets ready to IO | %d in read set, %d in write set\n", count, readfds.size(), writefds.size());

            if(readfds.size()){
                for( std::set<UDTSOCKET>::const_iterator it = readfds.begin(); it != readfds.end(); ++it){
                    processSocketReadyToRead(*it);
                }
                readfds.clear();
            }

            if(writefds.size()){
                for( std::set<UDTSOCKET>::const_iterator it = writefds.begin(); it != writefds.end(); ++it){
                    processSocketReadyToWrite(*it);
                }
                writefds.clear();
            }

        }

        //QCoreApplication::processEvents();
    }
    m_threadCount--;

}

void UDTProtocolBase::waitForReading(int msecTimeout){
    qDebug()<<"--UDTProtocolBase::waitForReading(...)";


    set<UDTSOCKET> readfds;
    int count = 0;

    m_threadCount++;
    while(m_listening){
        acceptNewConnection();

        count = UDT::epoll_wait(epollID, &readfds, NULL, msecTimeout);
        if(count > 0){
            //qDebug()<<QString("epoll returned %1 sockets ready to IO | %2 in read set").arg(count).arg(readfds.size());
            //printf("epoll returned %d sockets ready to IO | %d in read set\n", count, readfds.size());

            for( std::set<UDTSOCKET>::const_iterator it = readfds.begin(); it != readfds.end(); ++it){
                //TODO:Process
                //QtConcurrent::run(this, &UDTProtocolBase::readDataFromSocket, *it);
                processSocketReadyToRead(*it);
            }
            readfds.clear();

        }

        //QCoreApplication::processEvents();
    }
    m_threadCount--;

}

void UDTProtocolBase::waitForWriting(int msecTimeout){
    qDebug()<<"--UDTProtocolBase::waitForWriting(...)";

    QDateTime beginTime, curTime;
    int interval = 0;
    set<UDTSOCKET> writefds;
    int count = 0;

    m_threadCount++;
    while(m_listening){

        //acceptNewConnection();
        beginTime = QDateTime::currentDateTime();

        count = UDT::epoll_wait(epollID, NULL, &writefds, msecTimeout);
        if(count > 0){
            //qDebug()<<QString("epoll returned %1 sockets ready to IO | %2 in write set").arg(count).arg(writefds.size());
            //printf("epoll returned %d sockets ready to IO | %d in write set\n", count, writefds.size());

            for( std::set<UDTSOCKET>::const_iterator it = writefds.begin(); it != writefds.end(); ++it){
                //TODO:Process
                //QtConcurrent::run(this, &UDTProtocolBase::writeDataToSocket, *it);
                processSocketReadyToWrite(*it);
            }
            writefds.clear();
        }

        curTime = QDateTime::currentDateTime();
        interval = beginTime.msecsTo(curTime);
        if(interval < msecTimeout){
            msleep(msecTimeout - interval);
        }
        //QCoreApplication::processEvents();
    }
    m_threadCount--;

}

void UDTProtocolBase::waitingForNewConnection(int msecTimeout){

    QDateTime beginTime, curTime;
    int interval = 0;


    m_threadCount++;
    while(m_listening){

        //acceptNewConnection();
        beginTime = QDateTime::currentDateTime();

        acceptNewConnection();

        curTime = QDateTime::currentDateTime();
        interval = beginTime.msecsTo(curTime);
        if(interval < msecTimeout){
            msleep(msecTimeout - interval);
        }
        //QCoreApplication::processEvents();
    }
    m_threadCount--;


}

UDTSOCKET UDTProtocolBase::acceptNewConnection(){
//    qDebug()<<"--UDTProtocolBase::acceptNewConnection()";

    sockaddr_storage clientaddr;
    int addrlen = sizeof(clientaddr);
    UDTSOCKET peer;

    if (UDT::INVALID_SOCK == (peer = UDT::accept(serverSocket, (sockaddr*)&clientaddr, &addrlen)))
    {
        //qDebug()<<"ERROR! Invalid UDTSOCKET! "<< UDT::getlasterror().getErrorMessage();
        //cout << "accept: " << UDT::getlasterror().getErrorMessage() << endl;

//        int errorCode = UDT::getlasterror().getErrorCode();
//        QString errorMessage = QString(UDT::getlasterror().getErrorMessage());
//        switch(errorCode){
//        case UDT::ERRORINFO::EINVSOCK:
//            //serverSocket is an invalid UDT socket.
//            qDebug()<<"ERROR! "<<errorMessage;
//            break;
//        case UDT::ERRORINFO::ENOLISTEN:
//            //serverSocket is not in the listening state.
//            qDebug()<<"ERROR! "<<errorMessage;
//            break;
//        case UDT::ERRORINFO::ERDVNOSERV:
//            //serverSocket is set up to support rendezvous connection.
//            qDebug()<<errorMessage;
//            break;
//        case UDT::ERRORINFO::EASYNCRCV:
//            //serverSocket is non-blocking (UDT_RCVSYN = false) but there is no connection available.
//            qDebug()<<errorMessage;
//            break;
//        default:
//            qDebug()<<errorMessage;
//            break;
//        }

        return UDT::INVALID_SOCK;

    }

    char peerAddress[NI_MAXHOST];
    char peerPort[NI_MAXSERV];

    getnameinfo((sockaddr *)&clientaddr, addrlen, peerAddress, sizeof(peerAddress), peerPort, sizeof(peerPort), NI_NUMERICHOST|NI_NUMERICSERV);
    //cout << "New Connection: " << peerAddress << ":" << peerPort << " UDTSOCKET:"<< peer << endl;
    qDebug()<<QString("New Connection:%1:%2, UDTSOCKET:%3").arg(peerAddress).arg(peerPort).arg(peer);

//    if(UDT::epoll_add_usock(epollID, peer, NULL) < 0){
//        qWarning()<<"ERROR! epoll_add_usock Failed! "<< UDT::getlasterror().getErrorMessage();
//        //fprintf(stderr, "epoll_add_usock error\n");
//        UDT::close(peer);
//        return UDT::INVALID_SOCK;
//    }
    if(!addSocketToEpoll(peer)){
        UDT::close(peer);
        return UDT::INVALID_SOCK;
    }

    emit connected(peer, QString::fromLocal8Bit(peerAddress), QString::fromLocal8Bit(peerPort).toUInt());

    return peer;

}

void UDTProtocolBase::processSocketReadyToRead(UDTSOCKET socket){
    //qDebug()<<"--UDTProtocolBase::processSocketReadyToRead(..) "<<"socket:"<<socket;

    if(socket == serverSocket){
        qDebug()<<"-------R---------0----------------";
        acceptNewConnection();
        return;
    }


    UDTSTATUS status = UDT::getsockstate(socket);
    switch(status){
    case INIT: //1
    case OPENED: //2
    case LISTENING: //3
    case CONNECTING: //4
    case CONNECTED: //5
    {
        //qDebug()<<"----R------ Status:"<<status<<" ----------";
    }
        break;
    case BROKEN: //6
    {
        qWarning()<<"-------R--------socket:"<<socket<<" BROKEN";
        closeSocket(socket);
        qWarning();

        emit disconnected(socket);
        return;
    }
        break;
    case CLOSING: //7
    {
        //qDebug()<<"CLOSING";
        return;
    }
        break;
    case CLOSED: //8
    {
        closeSocket(socket);
        emit disconnected(socket);

        qDebug()<<"-------R-------socket:"<<socket<<" CLOSED"<<" ThreadID: "<<QThread::currentThreadId();
        return;
    }
        break;
    case NONEXIST: //9
    {
        qCritical()<<"-------R-------socket:"<<socket<<" NONEXIST"<<" ThreadID: "<<QThread::currentThreadId();

        closeSocket(socket);
        qCritical();
        return;
    }
        break;
    default:
        break;

    }


    //qDebug()<<"-------------!!!--------------"<<"socket:"<<socket<<" state:"<<UDT::getsockstate(socket);

    int size = MAX_DATA_BLOCK_SIZE + 4;

    QByteArray byteArray;
    byteArray.resize(size);
    //TODO:Size
    char* data = byteArray.data();
    //data = new char[size];


    int receivedSize = 0;

    if(m_stream){
        //while (true)
        //{
        int totalReceivedSize = 0;
        while (totalReceivedSize < size)
        {
            if (UDT::ERROR == (receivedSize = UDT::recv(socket, data + totalReceivedSize, size - totalReceivedSize, 0)))
            {
                //qDebug()<<"ERROR! Failed to receive data! "<< UDT::getlasterror().getErrorMessage();
                break;
            }

            totalReceivedSize += receivedSize;
            //qDebug()<<"totalReceivedSize:"<<totalReceivedSize;
        }
        if (0 == totalReceivedSize){
            qDebug()<<"No data received! "<<" socket:"<<socket;
            return;
        }

        byteArray.resize(totalReceivedSize);
        processStreamDataAfterReceived(socket, &byteArray);

    }else{
        if (UDT::ERROR == (receivedSize = UDT::recvmsg(socket, data, size)))
        {
            qDebug()<<"ERROR! Failed to receive message! "<<UDT::getlasterror().getErrorMessage();
            //cout << "recvmsg:" << UDT::getlasterror().getErrorMessage() << endl;
            return;
        }

        byteArray.resize(receivedSize);
        messageDataReceived(socket, &byteArray);

    }

}

void UDTProtocolBase::processSocketReadyToWrite(UDTSOCKET socket){
    //qDebug()<<"--UDTProtocolBase::processSocketReadyToWrite() "<<"socket:"<<socket<<" status:"<<UDT::getsockstate(socket);



    if(socket == serverSocket){
        qDebug()<<"-------W---------0----------------";
        acceptNewConnection();
        return;
    }

    UDTSTATUS status = UDT::getsockstate(socket);
    switch(status){
    case INIT: //1
    case OPENED: //2
    case LISTENING: //3
    case CONNECTING: //4
    case CONNECTED: //5
    {
        //qDebug()<<"-----W----- Status:"<<status<<" ----------";
    }
        break;
    case BROKEN: //6
    {
        qDebug()<<"------W---------socket:"<<socket<<" BROKEN";

        closeSocket(socket);
        emit disconnected(socket);
        qWarning();
        return;
    }
        break;
    case CLOSING: //7
    {
        //qDebug()<<"CLOSING";
        return;
    }
        break;
    case CLOSED: //8
    {
        closeSocket(socket);
        emit disconnected(socket);

        qDebug()<<"-------W-------socket:"<<socket<<" CLOSED"<<" ThreadID:"<<QThread::currentThreadId();
        return;
    }
        break;
    case NONEXIST: //9
    {
        qCritical()<<"-------W-------socket:"<<socket<<" NONEXIST"<<" ThreadID:"<<QThread::currentThreadId();

        closeSocket(socket);
        qCritical();
        return;
    }
        break;
    default:
        break;

    }

    //qDebug();


}

void UDTProtocolBase::processStreamDataAfterReceived(UDTSOCKET socket, QByteArray *byteArray){
    //qDebug()<<"--UDTProtocolBase::processStreamDataAfterReceived(...) "<<"socket:"<<socket<<" size:"<<byteArray->size();


    QByteArray *cachedData = m_cachedDataInfoHash.value(socket);
    if(cachedData){
        byteArray->prepend(*cachedData);
        cachedData->clear();
    }


    QDataStream in(byteArray, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_8);
    QIODevice *dev = in.device();

    QByteArray temp;
    qint64 offset = 0;

    forever{
        in >> temp;
        if(temp.isEmpty()){
            if(!cachedData){
                cachedData = new QByteArray();
                m_cachedDataInfoHash.insert(socket, cachedData);
            }

            dev->seek(offset);
            cachedData->append(dev->readAll());
            break;

        }else{
            streamDataReceived(socket, &temp);
            offset  = dev->pos();
        }

    }


}

void UDTProtocolBase::recycleCachedData(QByteArray* data){
    if(!data){
        return;
    }
    data->clear();

}

QByteArray * UDTProtocolBase::getCachedData(){

    if(m_unusedCached.isEmpty()){
        return new QByteArray();
    }
    return m_unusedCached.takeFirst();

}

void UDTProtocolBase::setSocketOptions(UDTSOCKET socket, SocketOptions *options){
    if(socket == INVALID_UDT_SOCK){
        return;
    }

    // UDT Options
    struct SocketOptions *opts = options;
    if(!opts){
        opts = &m_socketOptions;
    }
    //TODO: setup UDT_MSS
    //UDT::setsockopt(socket, 0, UDT_MSS, &(opts->UDT_MSS), sizeof(int));
    UDT::setsockopt(socket, 0, UDT_SNDSYN, &(opts->UDT_SNDSYN), sizeof(bool));
    UDT::setsockopt(socket, 0, UDT_RCVSYN, &(opts->UDT_RCVSYN), sizeof(bool));
    if(opts->UDT_CC){
        //UDT::setsockopt(serv, 0, UDT_CC, new CCCFactory<CUDPBlast>, sizeof(CCCFactory<CUDPBlast>));
        //UDT::setsockopt(serv, 0, UDT_CC, &(opts->UDT_CC), sizeof(CCC));
    }
    UDT::setsockopt(socket, 0, UDT_FC, &(opts->UDT_FC), sizeof(int));
    UDT::setsockopt(socket, 0, UDT_SNDBUF, &(opts->UDT_SNDBUF), sizeof(int));
    UDT::setsockopt(socket, 0, UDT_RCVBUF, &(opts->UDT_RCVBUF), sizeof(int));
    UDT::setsockopt(socket, 0, UDP_SNDBUF, &(opts->UDP_SNDBUF), sizeof(int));
    UDT::setsockopt(socket, 0, UDP_RCVBUF, &(opts->UDP_RCVBUF), sizeof(int));
    //UDT::setsockopt(socket, 0, UDT_LINGER, &(opts->UDT_LINGER), sizeof(linger));
    UDT::setsockopt(socket, 0, UDT_RENDEZVOUS, &(opts->UDT_RENDEZVOUS), sizeof(bool));
    UDT::setsockopt(socket, 0, UDT_SNDTIMEO, &(opts->UDT_SNDTIMEO), sizeof(int));
    UDT::setsockopt(socket, 0, UDT_RCVTIMEO, &(opts->UDT_RCVTIMEO), sizeof(int));
    UDT::setsockopt(socket, 0, UDT_REUSEADDR, &(opts->UDT_REUSEADDR), sizeof(bool));
    UDT::setsockopt(socket, 0, UDT_MAXBW, &(opts->UDT_MAXBW), sizeof(int64_t));

//    bool block = false;
//    int size = sizeof(bool);
//    UDT::getsockopt(socket, 0, UDT_SNDSYN, &block, &size);
//    qWarning()<<"UDT_SNDSYN:"<<block;

}

inline void UDTProtocolBase::msleep(int msec){

#ifdef Q_OS_WIN32
    Sleep(msec);
#else
    usleep(msec*1000);
#endif

}


bool UDTProtocolBase::addSocketToEpoll(UDTSOCKET socket){
    qDebug()<<"--UDTProtocolBase::addSocketToEpoll(...) "<<"socket:"<<socket<<" ThreadID:"<<QThread::currentThreadId();

    QMutexLocker locker(&m_epollMutex);

    if(UDT::epoll_add_usock(epollID, socket, NULL) < 0){
        qWarning()<<"ERROR! epoll_add_usock Failed! "<< UDT::getlasterror().getErrorMessage();
        //fprintf(stderr, "epoll_add_usock error\n");
        //UDT::close(peer);
        return false;
    }

    return true;

}

void UDTProtocolBase::removeSocketFromEpoll(UDTSOCKET socket){
    qDebug()<<"--UDTProtocolBase::removeSocketFromEpoll(...) "<<"socket:"<<socket<<" ThreadID:"<<QThread::currentThreadId();

    QMutexLocker locker(&m_epollMutex);

    if(UDT::epoll_remove_usock(epollID, socket) == UDT::ERROR){
        qCritical()<<QString("ERROR! epoll_remove_usock: %1 Socket: %2 ").arg(UDT::getlasterror().getErrorMessage()).arg(socket) <<"Thread: "<<(QThread::currentThreadId());
        //fprintf(stderr, "ERROR! epoll_remove_usock: %s\n", UDT::getlasterror().getErrorMessage());
    }
    if(UDT::close(socket) == UDT::ERROR){
        qCritical()<<QString("ERROR! UDT::close: %1 Socket: %2 ").arg(UDT::getlasterror().getErrorMessage()).arg(socket)<<"Thread: "<<(QThread::currentThreadId());
        //fprintf(stderr, "ERROR! UDT::close: %s\n", UDT::getlasterror().getErrorMessage());
    }

    QByteArray *data = m_cachedDataInfoHash.take(socket);
    recycleCachedData(data);

}

bool UDTProtocolBase::getAddressInfoFromSocket(UDTSOCKET socket, QString *address, quint16 *port, bool getPeerInfo){

    m_errorMessage = "";

    sockaddr clientaddr;
    int addrlen = sizeof(clientaddr);
    int error;
    if(getPeerInfo){
        error = UDT::getpeername(socket, &clientaddr, &addrlen);
    }else{
        error = UDT::getsockname(socket, &clientaddr, &addrlen);
    }
    if(UDT::ERROR == error){
        m_errorMessage = UDT::getlasterror().getErrorMessage();
        qDebug()<<QString("ERROR! Failed to get %1 address info! %2").arg(getPeerInfo?"peer":"local").arg(m_errorMessage);
        return false;
    }

    char clienthost[NI_MAXHOST];
    char clientservice[NI_MAXSERV];
    getnameinfo(&clientaddr, addrlen, clienthost, sizeof(clienthost), clientservice, sizeof(clientservice), NI_NUMERICHOST|NI_NUMERICSERV);
    //cout << "Address Info: " << clienthost << ":" << clientservice << endl;

    if(address){
        *address = QString::fromLocal8Bit(clienthost);
    }
    if(port){
        *port = QString::fromLocal8Bit(clientservice).toUInt();
    }

    return true;

}

//UDTSocketStatus UDTProtocolBase::getUDTSocketStatus(UDTSOCKET socket){
//    return UDT::getsockstate(socket);
//}

bool UDTProtocolBase::isSocketListening(UDTSOCKET socket){
    return UDT::getsockstate(socket) == LISTENING;
}

bool UDTProtocolBase::isConnecting(UDTSOCKET socket){
    return UDT::getsockstate(socket) == CONNECTING;
}

bool UDTProtocolBase::isSocketConnected(UDTSOCKET socket){
    return UDT::getsockstate(socket) == CONNECTED;
}

bool UDTProtocolBase::isSocketBroken(UDTSOCKET socket){
    return (UDT::getsockstate(socket) == BROKEN) || (UDT::getsockstate(socket) == NONEXIST);
}

bool UDTProtocolBase::isSocketExist(UDTSOCKET socket){
    return UDT::getsockstate(socket) != NONEXIST;
}

QString UDTProtocolBase::getUDTListeningAddress(){
    return m_serverAddress;
}

quint16 UDTProtocolBase::getUDTListeningPort(){
    return m_serverPort;
}

QString UDTProtocolBase::getLastErrorMessage() const{

    return m_errorMessage;

}















}



