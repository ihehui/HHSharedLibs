/*
 ****************************************************************************
 * guilib.h
 *
 * Created on: 2009-4-27
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
  * Last Modified on: 2012-01-11
  * Last Modified by: 贺辉
  ***************************************************************************
*/



#ifndef GUILIB_H
#define GUILIB_H


#include <QtCore/QtGlobal>

#if defined(GUI_LIBRARY_EXPORT)
    #define GUI_LIB_API Q_DECL_EXPORT
#else
    #define GUI_LIB_API Q_DECL_IMPORT
#endif



#endif // GUILIB_H
