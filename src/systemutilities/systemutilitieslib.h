/*
 ****************************************************************************
 * systemutilitieslib.h
 *
 * Created on: 2009-5-1
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





#ifndef WMLIB_H
#define WMLIB_H


#include <QtCore/QtGlobal>

 #if defined(SYSUTIL_LIBRARY_EXPORT)
 #  define SYSUTIL_LIB_API Q_DECL_EXPORT
 #else
 #  define SYSUTIL_LIB_API Q_DECL_IMPORT
 #endif


#endif //systemutilitieslib.h
