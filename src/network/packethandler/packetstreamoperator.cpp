/*
 ****************************************************************************
 * packetstreamoperator.cpp
 *
 * Created on: 2010-07-22
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
 * Last Modified on: 2010-07-22
 * Last Modified by: 贺辉
 ***************************************************************************
 */






#include "packetstreamoperator.h"

//重载操作符“<<”
//Write data to stream
QDataStream &operator<<(QDataStream &out, const HEHUI::Packet &packet){
    quint8 packetType = packet.getPacketType();
    QByteArray packetData = packet.getPacketData();

    out << packetType;
    //    if(packetType == quint8(HEHUI::HeartbeatPacket)){
    out << packetData;
    //    }else{
    //        quint16 packetSerialNumber = packet.getPacketSerialNumber();
    //        if(packetSerialNumber == 0){
    //            packetSerialNumber = packet.createSerialNumber();
    //        }
    //        out << packetSerialNumber << packetData;
    //    }

    //qDebug()<<"operator<<-packetType:"<<packetType<<"   packetSerialNumber:"<<packetSerialNumber;
    //qDebug()<<"~~Out---Size:"<<" quint8:"<<sizeof(quint8)<<" quint16"<<sizeof(quint16)<<" packetData.size()"<<packetData.size();


    return out;

}

//重载操作符“>>”
//Read data from stream
QDataStream &operator>>(QDataStream &in, HEHUI::Packet &packet){
    quint8 packetType = 0;
    //    quint16 packetSerialNumber = 0;
    QByteArray packetData;

    in >> packetType;
    //    if(packetType == quint8(HEHUI::HeartbeatPacket)){
    in >> packetData;
    //    }else{
    //        in >> packetSerialNumber >> packetData;
    //    }

    packet.setPacketType(packetType);
    //    packet.setPacketSerialNumber(packetSerialNumber);
    packet.setPacketData(packetData);

    //    qDebug()<<"operator>>-packetType:"<<packetType<<"   packetSerialNumber:"<<packetSerialNumber;
    //    qDebug()<<"~~In---Size:"<<" quint8:"<<sizeof(quint8)<<" quint16"<<sizeof(quint16)<<" packetData.size()"<<packetData.size();

    return in;

}
