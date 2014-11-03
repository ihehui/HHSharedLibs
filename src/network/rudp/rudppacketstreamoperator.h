/*
 ****************************************************************************
 * rudppacketstreamoperator.h
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






#ifndef RUDPPACKETSTREAMOPERATOR_H_
#define RUDPPACKETSTREAMOPERATOR_H_


#include <QDataStream>
#include "rudppacket.h"
//#include "../global_network.h"

////////////////////////////////  重载操作符  //////////////////////////////////////////////////////

QDataStream &operator<<(QDataStream &out, const HEHUI::RUDPPacket &packet);
QDataStream &operator>>(QDataStream &in, HEHUI::RUDPPacket &packet);









#endif /* RUDPPACKETSTREAMOPERATOR_H_ */
