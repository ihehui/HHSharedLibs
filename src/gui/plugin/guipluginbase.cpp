/*
 ****************************************************************************
 * guipluginbase.cpp
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

#include "guipluginbase.h"

namespace HEHUI {


GUIPluginBase::GUIPluginBase() {

    parentWidget = 0;
    m_menu = 0;
    actionMain = 0;
    defaultAction = 0;

}

GUIPluginBase::~GUIPluginBase() {
    qDebug("----GUIPluginBase::~GUIPluginBase()");

    //unload();

    if (m_menu) {
        QAction *action = m_menu->menuAction();
        QList<QWidget *> widgetList = action->associatedWidgets();
        foreach(QWidget *widget, widgetList)
        {
            widget->removeAction(action);
        }

        QList<QGraphicsWidget *> graphicsWidgetList = action->associatedGraphicsWidgets();
        foreach(QGraphicsWidget *graphicsWidget, graphicsWidgetList)
        {
            graphicsWidget->removeAction(action);
        }
    }

    delete m_menu;
    m_menu = 0;

}

//QString GUIPluginBase::name () const{
//        return QString(tr("GUI Plug-in"));
//}
//
//QString GUIPluginBase::version() const
//{
//    return QString("1.0.0");
//}

QString GUIPluginBase::vendor() const {
    return QString(tr("He Hui"));
}

QString GUIPluginBase::author() const {
    return QString(tr("He Hui"));
}

QString GUIPluginBase::url() const {
    return QString("www.gnu.org");
}

QString GUIPluginBase::description() const {
    return QString(tr("GUI Plug-in"));
}

QString GUIPluginBase::license() const {
    QString licenseString = QString(tr("This plugin is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</br>"));
    licenseString += QString(tr("<p>See the <a href=\"http://www.gnu.org/copyleft/lesser.html\">GNU LESSER GENERAL PUBLIC LICENSE</a> for more details.</p>"));
    return licenseString;
}

//bool GUIPluginBase::isSingle()
//  {
//    return false;
//  }

bool GUIPluginBase::initialize(QWidget * parentWidget, QMenu *menu, QToolBar *toolBar, QSystemTrayIcon *systemTrayIcon, const QString& pName, const QString& pVersion) {

    qDebug("----GUIPluginBase::initialize(...)");

    setParent(parentWidget);
    this->parentWidget = parentWidget;

    //	this->systemTrayIcon = systemTrayIcon;

    //	QAction *action = new QAction(icon(), name(), parentWidget);
    //	action->setToolTip(toolTip());
    //	action->setStatusTip(toolTip());
    //	action->setWhatsThis(whatsThis());
    //	connect(action, SIGNAL(triggered()), this, SLOT(slotRun()));

    //	if(menu){
    //		menu->addAction(action);
    //	}
    //
    //	if(toolBar){
    //		toolBar->addAction(action);
    //	}

    //	if(systemTrayIcon){
    //		//TODO:
    //		//systemTrayMenu->addAction(action);
    //	}

    return true;

}

bool GUIPluginBase::unload() {
    qDebug("----GUIPluginBase::unload()");

    //    if(m_menu){
    //        QAction *action = m_menu->menuAction();
    //        QList<QWidget *> widgetList = action->associatedWidgets ();
    //        foreach(QWidget *widget, widgetList){
    //            widget->removeAction(action);
    //        }
    //
    //        QList<QGraphicsWidget *> graphicsWidgetList = action->associatedGraphicsWidgets ();
    //        foreach(QGraphicsWidget *graphicsWidget, graphicsWidgetList){
    //            graphicsWidget->removeAction(action);
    //        }
    //    }
    //

    emit signalPluginToBeUnloaded();

    return true;
}

QIcon GUIPluginBase::icon() const {
    return QIcon(":/resources/images/plugin.png");

}

QString GUIPluginBase::whatsThis() const {
    return QString(tr("GUI Plug-in"));
}

QString GUIPluginBase::toolTip() const {

    return QString(tr("GUI Plug-in"));
}

QMenu * GUIPluginBase::menu() {
    if (!m_menu) {
        m_menu = new QMenu(name(), parentWidget);
        //		m_menu->setIcon(icon());
        //		actionMain = new QAction(icon(), name(), m_menu);
        //		actionMain->setToolTip(toolTip());
        //		actionMain->setStatusTip(toolTip());
        //		actionMain->setWhatsThis(whatsThis());
        //		connect(actionMain, SIGNAL(triggered()), this, SLOT(slotRun()));
        //		m_menu->addAction(actionMain);

        //		m_menu->setDefaultAction(actionMain);
        //		m_menu->setActiveAction(actionMain);

        //		connect(m_menu, SIGNAL(triggered(QAction *)), this, SLOT(slotSetDefaultActionForMenuAction(QAction *)));

        //		defaultAction = actionMain;
        //		connect(m_menu->menuAction(), SIGNAL(triggered()), defaultAction, SLOT(trigger()));

        QAction *action = mainActionForMenu();
        if(action){
            m_menu->addAction(action);
            slotSetDefaultActionForMenuAction(action);
        }

        //		m_menu->setDefaultAction(action);
        //		m_menu->setActiveAction(action);
        //                m_menu->setIcon(icon());

        connect(m_menu, SIGNAL(triggered(QAction *)), this, SLOT(slotSetDefaultActionForMenuAction(QAction *)));

        //		defaultAction = action;
        //		connect(m_menu->menuAction(), SIGNAL(triggered()), defaultAction, SLOT(trigger()));



    }

    return m_menu;

}

void GUIPluginBase::addActionToMenu(QAction *action){

    if(!action){
        return;
    }
    
    QMenu *m = menu();
    m->addAction(action);
    
}

bool GUIPluginBase::event( QEvent * e ){

    if(e->type() == QEvent::LanguageChange){
        if(m_menu){
            m_menu->setTitle(name());
        }
        if(actionMain){
            actionMain->setText(name());    
        }
        retranslateUi();
    }
    return QObject::event(e);
}


void GUIPluginBase::slotSetDefaultActionForMenuAction(QAction * action) {
    qDebug("----GUIPluginBase::slotSetDefaultActionForMenuAction(QAction * action)");

    if ((defaultAction == action) || (!m_menu)) {
        return;
    }

    QAction *menuAction = m_menu->menuAction();
    menuAction->disconnect(defaultAction);
    this->defaultAction = action;
    connect(menuAction, SIGNAL(triggered()), defaultAction, SLOT(trigger()));

    m_menu->setDefaultAction(action);
    m_menu->setActiveAction(action);

    //menuAction->setText(action->text());
    menuAction->setToolTip(action->toolTip());
    menuAction->setStatusTip(action->statusTip());
    menuAction->setWhatsThis(action->whatsThis());

    QIcon icon = action->icon();
    if(icon.isNull()){
        menuAction->setIcon(QIcon(":/resources/images/plugin.png"));
    }else{
        menuAction->setIcon(icon);
    }


}

void GUIPluginBase::retranslateUi(){

    //    if(m_menu){
    //        m_menu->setTitle(name());
    //    }
    
    //    if(actionMain){
    //        actionMain->setText(name());
    //    }
}


QAction * GUIPluginBase::mainActionForMenu(){

    if(!actionMain){
        actionMain = new QAction(icon(), name(), m_menu);
        actionMain->setToolTip(toolTip());
        actionMain->setStatusTip(toolTip());
        actionMain->setWhatsThis(whatsThis());
        connect(actionMain, SIGNAL(triggered()), this, SLOT(slotMainActionForMenuTriggered()));
    }

    return actionMain;

}




/*
 void GUIPluginBase::slotRun(){

 if(isSingle() && SqlExplorer::isRunning()){
 //TODO: Activate the widget
 return;
 }


 User user;
 LoginBase login(&user, parentWidget);
 if (!login.isVerified()) {
 return ;
 }


 SqlExplorer *sqlExplorer = new SqlExplorer(parentWidget);
 if(parentWidget){
 if(QMdiArea *mdiArea = qobject_cast<QMdiArea *>(parentWidget)){
 mdiArea->addSubWindow(sqlExplorer, Qt::Dialog);
 qDebug()<<"Parent Widget is QMdiArea!";
 }
 }
 sqlExplorer->show();
 }
 */

//Q_EXPORT_PLUGIN2(samplePlugin, GUIPluginBase)

} //namespace HEHUI
