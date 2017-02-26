/*
 ****************************************************************************
 * guipluginbase.h
 *
 * Created on: 2009-8-22
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
 * Last Modified on: 2010-05-18
 * Last Modified by: 贺辉
 ***************************************************************************
 */

#ifndef GUIPLUGINBASE_H_
#define GUIPLUGINBASE_H_

#include <QObject>
#include <QMenu>

#include "guiinterface.h"

#include "../guilib.h"

namespace HEHUI
{


class GUI_LIB_API GUIPluginBase: public QObject, public GUIInterface
{
    Q_OBJECT
    Q_INTERFACES(HEHUI::AbstractPluginInterface)
    Q_INTERFACES(HEHUI::GUIInterface)

public:
    GUIPluginBase();
    virtual ~GUIPluginBase();

    //        virtual QString name() const ;
    //        virtual QString version() const;
    virtual QString vendor() const;
    virtual QString author() const;
    virtual QString url() const;
    virtual QString description() const;
    virtual QString license() const;

    //        virtual bool isSingle() const;


    bool initialize(QWidget *parentWidget, QMenu *menu, QToolBar *toolBar, QSystemTrayIcon *systemTrayIcon, const QString &pName, const QString &pVersion);
    virtual bool unload();

    //        QWidget *parentWidgetOfPlugin();

    virtual QIcon icon() const;
    virtual QString whatsThis() const;
    virtual QString toolTip() const;

    virtual QAction *mainActionForMenu();
    virtual QMenu *menu();
    void addActionToMenu(QAction *action);

protected:
    bool event( QEvent *e );


signals:
    void signalPluginToBeUnloaded();


private slots:
    virtual void retranslateUi();
    virtual void slotMainActionForMenuTriggered() = 0;

    void slotSetDefaultActionForMenuAction(QAction *action);

private:

private:
    QWidget *parentWidget;

    QMenu *m_menu;
    QAction *actionMain;
    QAction *defaultAction;

};

} //namespace HEHUI

#endif /* GUIPLUGINBASE_H_ */
