/*
 ****************************************************************************
 * networkutilities.cpp
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






#include <QNetworkInterface>

#include <QDebug>

#include "networkutilities.h"

namespace HEHUI {

NetworkUtilities::NetworkUtilities() {
    // TODO Auto-generated constructor stub

}

NetworkUtilities::~NetworkUtilities() {
    // TODO Auto-generated destructor stub
}



QList<QHostAddress> NetworkUtilities::broadcastAddresses(){
    QList<QHostAddress> broadcastAddresses;
    //QList<QHostAddress> ipAddresses;
    foreach (QNetworkInterface nic, QNetworkInterface::allInterfaces()) {
        foreach (QNetworkAddressEntry entry, nic.addressEntries()) {
            QHostAddress broadcastAddress = entry.broadcast();
            if (broadcastAddress != QHostAddress::Null && entry.ip() != QHostAddress::LocalHost) {
                broadcastAddresses << broadcastAddress;
                qDebug()<<"----NetworkUtilities::broadcastAddresses()~~broadcastAddress:"<<broadcastAddress.toString();
                //ipAddresses << entry.ip();
            }
        }
    }

    return broadcastAddresses;

}

QList<QHostAddress> NetworkUtilities::validIPAddresses(){
    QList<QHostAddress> ipAddresses;
    foreach (QNetworkInterface nic, QNetworkInterface::allInterfaces()) {
        foreach (QNetworkAddressEntry entry, nic.addressEntries()) {
            qDebug()<<"entry.ip():"<<entry.ip().toString();
            QHostAddress broadcastAddress = entry.broadcast();
            if (broadcastAddress != QHostAddress::Null && entry.ip() != QHostAddress::LocalHost) {
                ipAddresses << entry.ip();
            }
        }
    }

    return ipAddresses;
}


QString NetworkUtilities::hardwareAddress(const QHostAddress &ipAddress){
    if(ipAddress.isNull()){return "";}

    foreach (QNetworkInterface nic, QNetworkInterface::allInterfaces()) {
        foreach (QNetworkAddressEntry entry, nic.addressEntries()) {
            //qDebug()<<"IP:"<<entry.ip()<<" Hardware Address:"<<interface.hardwareAddress();
            if (entry.ip() == ipAddress) {
                return nic.hardwareAddress();
            }
        }
    }

    return QString("");

}

QString NetworkUtilities::hardwareAddress(const QString &ipAddress){

    return hardwareAddress(QHostAddress(ipAddress));

}

bool NetworkUtilities::isLocalAddress(const QHostAddress &address){

    QList<QHostAddress> addresses = validIPAddresses();
    foreach(QHostAddress add, addresses){
        if(add == address){
            return true;
        }
    }

    if(address == QHostAddress::LocalHost){
        return true;
    }

    return false;
}

bool NetworkUtilities::isLocalAddress(const QString &address){
    return isLocalAddress(QHostAddress(address));
}







}
