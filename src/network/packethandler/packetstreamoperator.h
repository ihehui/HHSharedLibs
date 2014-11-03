/*
 ****************************************************************************
 * packetstreamoperator.h
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






#ifndef PACKETSTREAMOPERATOR_H_
#define PACKETSTREAMOPERATOR_H_


#include <QDataStream>
#include "packet.h"
#include "../global_network.h"

#include "../networklib.h"

////////////////////////////////  重载操作符  //////////////////////////////////////////////////////

NETWORK_LIB_API QDataStream &operator<<(QDataStream &out, const HEHUI::Packet &packet);
NETWORK_LIB_API QDataStream &operator>>(QDataStream &in, HEHUI::Packet &packet);









#endif /* PACKETSTREAMOPERATOR_H_ */
