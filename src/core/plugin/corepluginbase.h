/*
 ****************************************************************************
 * corepluginbase.h
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
  * Last Modified on: 2010-05-12
  * Last Modified by: 贺辉
  ***************************************************************************
*/




#ifndef COREPLUGINBASE_H
#define COREPLUGINBASE_H

#include <QObject>
#include <QSettings>

#include "coreinterface.h"
#include "../core_lib.h"

namespace HEHUI
{

class CORE_LIB_API CorePluginBase : public QObject, public CoreInterface
{
    Q_OBJECT
    Q_INTERFACES(HEHUI::AbstractPluginInterface)
    Q_INTERFACES(HEHUI::CoreInterface)
public:
    explicit CorePluginBase(QObject *parent = 0);


    //virtual QString name() const;
    //virtual QString version() const;
    virtual QString vendor() const;
    virtual QString author() const;
    virtual QString url() const;
    virtual QString description() const;
    virtual QString license() const;

    //virtual bool isSingle() const;

    //Initialization
    virtual bool initialize(QObject *parentObject) = 0;


signals:

private:


};

} //namespace HEHUI

#endif // COREPLUGINBASE_H
