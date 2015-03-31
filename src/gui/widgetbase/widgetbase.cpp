/*
 ****************************************************************************
 * widgetbase.cpp
 *
 * Created on: 2009-8-25
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





#include <QApplication>
#include <QDesktopWidget>
#include <QMenu>

#include "widgetbase.h"



WidgetBase::WidgetBase(QWidget *parent, Qt::WindowFlags flags)
    :QWidget(parent, flags)
{
    // TODO Auto-generated constructor stub

}

WidgetBase::~WidgetBase() {
    // TODO Auto-generated destructor stub
}


void WidgetBase::moveWindow(HEHUI::WindowPosition positon) {
    // Get the size of screen
    QDesktopWidget* desktop = QApplication::desktop();
    QRect rect = desktop->availableGeometry(this);
    int desktopWidth = rect.width();
    int desktopHeight = rect.height();

    int windowWidth = frameGeometry().width();
    int windowHeight = frameGeometry().height();

    //move the window
    switch (positon) {
    case HEHUI::Center:
        move((desktopWidth - windowWidth) / 2, (desktopHeight - windowHeight) / 2);
        break;
    case HEHUI::BottomRight:
        move((desktopWidth - windowWidth), (desktopHeight - windowHeight));
        break;
    default:
        move((desktopWidth - windowWidth) / 2, (desktopHeight - windowHeight) / 2);

    }

}


void WidgetBase::updateSystemTray(const QString &toolTip,
                                  const QIcon &icon, QMenu *menu) {
    QSystemTrayIcon *tray = SystemTrayIcon();

    if (!tray) {
        return;
    }

    tray->setIcon(icon);
    tray->setToolTip(toolTip);

    QMenu *trayMenu = tray->contextMenu();

    if (!trayMenu) {
        tray->setContextMenu(menu);
    } else {
        trayMenu->addMenu(menu);
    }

}

void WidgetBase::showSystemTrayMsg(const QString & title,
                                   const QString & message, QSystemTrayIcon::MessageIcon iconType,
                                   int secondsTimeoutHint) {
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(iconType);
    SystemTrayIcon()->showMessage(title, message, icon, secondsTimeoutHint
                                  * 1000);
}



