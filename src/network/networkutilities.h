/*
 ****************************************************************************
 * networkutilities.h
 *
 * Created on: 2009-12-11
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
 * Last Modified on: 2010-05-15
 * Last Modified by: 贺辉
 ***************************************************************************
 */




#ifndef NETWORKUTILITIES_H_
#define NETWORKUTILITIES_H_

#include <QHostAddress>

#include "networklib.h"

namespace HEHUI
{

class NETWORK_LIB_API NetworkUtilities
{
public:
    NetworkUtilities();
    virtual ~NetworkUtilities();


    static QList<QHostAddress> broadcastAddresses();
    static QList<QHostAddress> validIPAddresses();


    static QString hardwareAddress(const QHostAddress &ipAddress);
    static QString hardwareAddress(const QString &ipAddress);

    static bool isLocalAddress(const QHostAddress &address);
    static bool isLocalAddress(const QString &address);




};

}

#endif /* NETWORKUTILITIES_H_ */
