/*
 ****************************************************************************
 * loginbase.h
 *
 * Created on: 2008-10-17
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
 * Last Modified on: 2010-05-08
 * Last Modified by: 贺辉
 ***************************************************************************
 */

#ifndef LOGINBASE_H_
#define LOGINBASE_H_

#include <QObject>


//#include "../../core/user.h"
//#include "../../core/global_core.h"
#include "HHSharedCore/huser.h"
#include "HHSharedCore/hglobal_core.h"

#include "../guilib.h"

namespace HEHUI {

class GUI_LIB_API LoginBase: public QObject {
    Q_OBJECT

public:
    LoginBase(User *user, QObject *parent = 0);
    LoginBase(User *user, const QString &windowTitle = "", QObject *parent = 0);
    virtual ~LoginBase();

    virtual bool isVerified();

    QString getUserID() const;
    //        QString userName() const;
    QString passWord() const;

    QWidget* getParentWidget();
    //	User* getUser();

private:
    virtual bool verifyUser();
    virtual bool getUserInfo();
    virtual bool canLogin();

    void setUser(User *u);





private:

    QWidget *parentWidget;

    User *user;

    bool isSuccesseful;

    QString m_windowTitle;


};

} //namespace HEHUI

#endif /* LOGINBASE_H_ */
