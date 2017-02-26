#ifndef UDTPROTOCOLFORFILETRANSMISSION_H
#define UDTPROTOCOLFORFILETRANSMISSION_H


#include <QObject>

#include "udtlib.h"

#include "udtprotocolbase.h"



namespace HEHUI
{


class UDT_LIB_API UDTProtocolForFileTransmission : public UDTProtocolBase
{
    Q_OBJECT
public:
    explicit UDTProtocolForFileTransmission(QObject *parent = 0);

signals:

public slots:

    //Connect to peer
    UDTSOCKET connectToPeer(const QHostAddress &address, quint16 port, SocketOptions *options = 0, int msecTimeout = 5000);

    //Send & Receive File
    void sendFileToPeer(UDTSOCKET socket, const QString &filePath, qint64 offset, qint64 size, int blockSize = 8192);
    void receiveFileFromPeer(UDTSOCKET socket, const QString &fileSavePath, qint64 offset, qint64 size, int blockSize = 8192);

private slots:
    void streamDataReceived(UDTSOCKET socket, QByteArray *data);
    void messageDataReceived(UDTSOCKET socket, QByteArray *data);


private:
    //Send & Receive File
    qint64 sendFile(UDTSOCKET socket, const QString &filePath, qint64 offset, qint64 size, int blockSize = 8192);
    qint64 receiveFile(UDTSOCKET socket, const QString &fileSavePath, qint64 offset, qint64 size, int blockSize = 8192);








};

} //namespace HEHUI

#endif // UDTPROTOCOLFORFILETRANSMISSION_H
