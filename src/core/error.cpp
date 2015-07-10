/*
 ****************************************************************************
 * error.cpp
 *
 * Created on: 2010-7-12
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
 * Last Modified on: 2010-7-12
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#include "error.h"


namespace HEHUI {

Error::Error(unsigned int errorCode, const QString &errorString){
    this->m_errorCode = errorCode;
    this->m_errorString = errorString;

}

Error::Error(const Error &error){
    *this = error;

}

Error & Error::operator = (const Error &error){
    this->m_errorCode = error.code();
    this->m_errorString = error.errorString();
    return *this;
}

unsigned int Error::code() const{
    return this->m_errorCode;
}

void Error::setErrorCode(unsigned int errorCode){
    this->m_errorCode = errorCode;
}

QString Error::errorString() const{
    return this->m_errorString;
}

void Error::setErrorString(const QString errorString){
    this->m_errorString = errorString;
}
















} //namespace HEHUI
