/*
 ****************************************************************************
 * error.h
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





#ifndef ERROR_H
#define ERROR_H

#ifndef NO_ERROR
    #define NO_ERROR 0
#endif


#include <QString>


namespace HEHUI
{

class Error
{

public:

    Error(unsigned int code = NO_ERROR, const QString &errorString = "");
    Error(const Error &error);
    Error &operator = (const Error &error);

    unsigned int code() const;
    void setErrorCode(unsigned int code);

    QString errorString() const;
    void setErrorString(const QString errorString);



private:
    unsigned int m_errorCode;
    QString m_errorString;



};

} //namespace HEHUI

#endif // ERROR_H
