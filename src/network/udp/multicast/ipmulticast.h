/*
 ****************************************************************************
 * ipmulticast.h
 *
 * Created on: 2009-9-4
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
 * Last Modified on: 2010-06-24
 * Last Modified by: 贺辉
 ***************************************************************************
 */






#ifndef IPMULTICAST_H_
#define IPMULTICAST_H_



#ifdef Q_OS_WIN32
#include "multicastwin.h"
typedef IPMultiCastWin IPMulticastSocket;

#else//  Q_OS_LINUX
#include "multicastlinux.h"
typedef IPMulticastLinux IPMulticastSocket;

#endif








#endif /* IPMULTICAST_H_ */
