/*
 ****************************************************************************
 * guiinterface.h
 *
 * Created on: 2009-8-20
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
 * Last Modified on: 2010-05-10
 * Last Modified by: 贺辉
 ***************************************************************************
 */

#ifndef GUIINTERFACE_H_
#define GUIINTERFACE_H_

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "../../core/plugin/abstractplugininterface.h"
#include "../guilib.h"

namespace HEHUI
{

class GUI_LIB_API GUIInterface: public AbstractPluginInterface
{

public:

    virtual ~GUIInterface() {}

    //        virtual QString name() const = 0;
    //        virtual QString version() const = 0;
    //        virtual QString vendor() const = 0;
    //        virtual QString author() const = 0;
    //        virtual QString url() const = 0;
    //        virtual QString description() const = 0;
    //        virtual QString license() const = 0;
    //
    //        virtual bool isSingle() = 0;

    //Initialization
    // virtual bool initialize( QWidget *parent) = 0;
    virtual bool initialize(QWidget *parentWidget, QMenu *menu, QToolBar *toolBar, QSystemTrayIcon *systemTrayIcon, const QString &appName, const QString &appVersion) = 0;

    //Unload the plugin
    //virtual bool unload() = 0;

    //        virtual QWidget *parentWidgetOfPlugin() = 0;

    virtual QIcon icon() const = 0;
    virtual QString whatsThis() const = 0;
    virtual QString toolTip() const = 0;

    virtual QMenu *menu() = 0;

};

} //namespace HEHUI


#define GUIInterface_IID "org.hehui.Interface.GUIInterface/2014.11"
Q_DECLARE_INTERFACE(HEHUI::GUIInterface, GUIInterface_IID)


#endif /* GUIINTERFACE_H_ */

