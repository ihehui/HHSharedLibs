
#include <QtConcurrentRun>


#include "udtprotocolforfiletransmission.h"





namespace HEHUI {


UDTProtocolForFileTransmission::UDTProtocolForFileTransmission(QObject *parent) :
    UDTProtocolBase(true, 0, parent)
{


}


UDTSOCKET UDTProtocolForFileTransmission::connectToPeer(const QHostAddress &address, quint16 port, SocketOptions *options, int msecTimeout){
    return connectToHost(address, port, options, true, false, msecTimeout);
}

void UDTProtocolForFileTransmission::sendFileToPeer(UDTSOCKET socket, const QString &filePath, qint64 offset, qint64 size, int blockSize){

//    if(!QFile::exists(filePath)){
//        m_errorMessage = tr("File '%1' does not exist").arg(filePath);
//        qCritical()<<m_errorMessage;
//        return -1;
//    }

    QtConcurrent::run(this, &UDTProtocolForFileTransmission::sendFile, socket, filePath, offset, size, blockSize);


}

void UDTProtocolForFileTransmission::receiveFileFromPeer(UDTSOCKET socket, const QString &fileSavePath, qint64 offset, qint64 size, int blockSize){


    QtConcurrent::run(this, &UDTProtocolForFileTransmission::receiveFile, socket, fileSavePath, offset, size, blockSize);


}

qint64 UDTProtocolForFileTransmission::sendFile(UDTSOCKET socket, const QString &filePath, qint64 offset, qint64 size, int blockSize){

//    if(!QFile::exists(filePath)){
//        m_errorMessage = tr("File '%1' does not exist").arg(filePath);
//        qCritical()<<m_errorMessage;

//        emit fileDataSent(socket, filePath, offset, -1, m_errorMessage);
//        return -1;
//    }

    // open the file
    fstream ifs(filePath.toStdString().c_str(), ios::in | ios::binary);

    //ifs.seekg(0, ios::end);
    //int64_t size = ifs.tellg();
    ifs.seekg(0, ios::beg);

    UDT::TRACEINFO trace;
    UDT::perfmon(socket, &trace);

    // send the file
    //int64_t offset = 0;
    qint64 sentSize = -1;
    if (UDT::ERROR == (sentSize = UDT::sendfile(socket, ifs, offset, size, blockSize)) )
    {
        QString errorMessage = tr("Failed to send file! ") + UDT::getlasterror().getErrorMessage();
        qCritical()<<errorMessage;
        //cout << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;

        emit fileDataSent(socket, filePath, offset, sentSize, errorMessage);
        return sentSize;
    }

    UDT::perfmon(socket, &trace);
    cout << "speed = " << trace.mbpsSendRate/8 << "MB/sec" << endl;

    ifs.close();

    emit fileDataSent(socket, filePath, offset, sentSize, "");

    return sentSize;

}

qint64 UDTProtocolForFileTransmission::receiveFile(UDTSOCKET socket, const QString &fileSavePath, qint64 offset, qint64 size, int blockSize){


    // receive the file
    fstream ofs(fileSavePath.toStdString().c_str(), ios::out | ios::binary | ios::trunc);
    int64_t recvsize = -1;
    //int64_t offset = 0;

    if (UDT::ERROR == (recvsize = UDT::recvfile(socket, ofs, offset, size, blockSize)))
    {
        QString errorMessage = tr("Failed to receive file! ") + UDT::getlasterror().getErrorMessage();
        qCritical()<<errorMessage;
        //cout << "recvfile: " << UDT::getlasterror().getErrorMessage() << endl;

        emit fileDataReceived(socket, fileSavePath, offset, recvsize, errorMessage);
        return recvsize;
    }

    ofs.close();

    emit fileDataReceived(socket, fileSavePath, offset, recvsize, "");

    return recvsize;

}

void UDTProtocolForFileTransmission::streamDataReceived(UDTSOCKET socket, QByteArray *data){
    qDebug()<<"--UDTProtocol::streamDataReceived(...) "<<"socket:"<<socket;


}

void UDTProtocolForFileTransmission::messageDataReceived(UDTSOCKET socket, QByteArray *data){
    qDebug()<<"--UDTProtocol::messageDataReceived(...) "<<"socket:"<<socket;


}















}
