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
QDataStream &operator<<(QDataStream &out, const HEHUI::PacketBase &packet)
{
    quint8 packetType = packet.getPacketType();
    QByteArray packetData = packet.getPacketData();

    out << packetType << packetData;

    return out;
}

//重载操作符“>>”
//Read data from stream
QDataStream &operator>>(QDataStream &in, HEHUI::PacketBase &packet)
{
    quint8 packetType = 0;
    QByteArray packetData;

    in >> packetType >> packetData;

    packet.setPacketType(packetType);
    packet.setPacketBody(packetData);

    return in;
}
