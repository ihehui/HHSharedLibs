/*
 ****************************************************************************
 * userbase.cpp
 *
 * Created on: 2009-11-10
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
  * Last Modified on: 2010-05-07
  * Last Modified by: 贺辉
  ***************************************************************************
*/




#include "userbase.h"

namespace HEHUI {

UserBase::UserBase(const QString &userID, const QString &userName, const QString &password, QObject *parent)
        :QObject(parent), userID(userID), userName(userName), password(password)
{

    authenticode = "";
    isVerified = false;

}

UserBase::~UserBase() {
	// TODO Auto-generated destructor stub
}



} //namespace HEHUI
