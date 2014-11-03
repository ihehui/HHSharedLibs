/*
 ****************************************************************************
 * servicelib.h
 *
 * Created on: 2012-1-11
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




#ifndef SERVICELIB_H
#define SERVICELIB_H


#include <QtCore/QtGlobal>

 #if defined(SERVICE_LIBRARY_EXPORT)
 #  define SERVICE_LIB_API Q_DECL_EXPORT
 #else
 #  define SERVICE_LIB_API Q_DECL_IMPORT
 #endif


/*

#if defined(Q_WS_WIN)
#  if !defined(QT_QTSERVICE_EXPORT) && !defined(QT_QTSERVICE_IMPORT)
#    define QT_QTSERVICE_EXPORT
#  elif defined(QT_QTSERVICE_IMPORT)
#    if defined(QT_QTSERVICE_EXPORT)
#      undef QT_QTSERVICE_EXPORT
#    endif
#    define QT_QTSERVICE_EXPORT __declspec(dllimport)
#  elif defined(QT_QTSERVICE_EXPORT)
#    undef QT_QTSERVICE_EXPORT
#    define QT_QTSERVICE_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTSERVICE_EXPORT
#endif



*/


#endif // SERVICELIB_H
