/*
 ****************************************************************************
 * systemtrayIconbase.cpp
 *
 * Created on: 2009-8-30
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



#include <QDebug>

#include "systemtrayiconbase.h"
#include "../imageresourcebase.h"


namespace HEHUI
{


//////////////////////////////////////////////////////////////////////////////////
TrayIconData::TrayIconData(QObject *parent)
    : QObject(parent)
{

    dataType = -1;
    id = "";
    owner = "";
    toolTip = "";
    menu = 0;
    data = QVariant();

    trayIconType = TRAYICON_Normal;

    iconList.clear();

}

TrayIconData::TrayIconData(int dataType, const QString &id, const QString &owner,
                           const QString &toolTip, QMenu *menu , const QVariant &data,
                           TrayIconType trayIconType, QList<QIcon> iconList,
                           QObject *parent)
    : QObject(parent)
{

    this->dataType = dataType;
    this->id = id;
    this->owner = owner;
    this->toolTip = toolTip;
    this->menu = menu;
    this->data = data;

    trayIconType = trayIconType;
    this->iconList = iconList;



}

TrayIconData::TrayIconData(const TrayIconData &trayIconData)
    : QObject()
{

    *this = trayIconData;

}

TrayIconData &TrayIconData::operator =(const TrayIconData &trayIconData)
{
    this->dataType = trayIconData.getDataType();
    this->id = trayIconData.getID();
    this->owner = trayIconData.getOwner();
    this->toolTip = trayIconData.getToolTip();
    this->menu = trayIconData.getMenu();
    this->data = trayIconData.getData();
    this->trayIconType = trayIconData.getTrayIconType();
    this->iconList = trayIconData.getIconList();
    return *this;
}


TrayIconData::~TrayIconData()
{

    this->iconList.clear();
}

bool TrayIconData::isNull() const
{
    return this->id.trimmed().isEmpty();
}




//////////////////////////////////////////////////////////////////////////////////////
SystemTrayIconBase::SystemTrayIconBase(QObject *parent) :
    QSystemTrayIcon(parent)
{

    timer = new QTimer(this);
    timer->setSingleShot(false);

    m_showSystemTrayIconDataLastAdded = true;

}

SystemTrayIconBase::SystemTrayIconBase(const TrayIconData &defaultData , QObject *parent)
    : QSystemTrayIcon(parent)
{

    resetAll(defaultData);


    timer = new QTimer(this);
    timer->setSingleShot(false);

    m_showSystemTrayIconDataLastAdded = true;


}

SystemTrayIconBase::SystemTrayIconBase(const QIcon &icon, QObject *parent) :
    QSystemTrayIcon(icon, parent)
{

    this->m_defaultData.setFirstIcon(icon);
    this->m_currentData.setFirstIcon(icon);


    timer = new QTimer(this);
    timer->setSingleShot(false);

    m_showSystemTrayIconDataLastAdded = true;


}

SystemTrayIconBase::~SystemTrayIconBase()
{
    timer->stop();
    hide();
}


void SystemTrayIconBase::showIcon(const TrayIconData &trayIconData)
{


    if(!trayIconData.isNull()) {
        return;
    }

    stop();

    this->m_currentData = trayIconData;


    setIcon(trayIconData.getFirstIcon());
    setToolTip(trayIconData.getToolTip());


    QMenu *menu = trayIconData.getMenu();
    if (menu) {
        QMenu *trayMenu = contextMenu();
        if (!trayMenu) {
            setContextMenu(menu);
        } else {
            trayMenu->addMenu(menu);
        }
    }




}

void SystemTrayIconBase::showIcon(const QIcon &icon, const QString &toolTip, QMenu *menu)
{


    this->m_currentData.setToolTip(toolTip);
    this->m_currentData.setMenu(menu);
    this->m_currentData.settrayIconType(TrayIconData::TRAYICON_Normal);
    this->m_currentData.setFirstIcon(icon);


    stop();

    setIcon(icon);
    setToolTip(toolTip);


    if (menu) {
        QMenu *trayMenu = contextMenu();
        if (!trayMenu) {
            setContextMenu(menu);
        } else {
            trayMenu->addMenu(menu);
        }
    }



}

void SystemTrayIconBase::showIcon(const QString &iconName, const QString &toolTip, QMenu *menu)
{

    QIcon ico = ImageResourceBase::createIcon(iconName);

    showIcon(ico, toolTip, menu);


}

//闪动图标
//Flash the icon
void SystemTrayIconBase::showFlash(const QIcon &icon, const QString &toolTip, QMenu *menu,
                                   int millisecondsTimeoutHint)
{

    qDebug() << "----SystemTrayIconBase::showFlash(const QIcon &icon, ...)";



    this->m_currentData.setToolTip(toolTip);
    this->m_currentData.setMenu(menu);
    this->m_currentData.settrayIconType(TrayIconData::TRAYICON_Flash);
    this->m_currentData.setFirstIcon(icon);


    disconnect(timer, SIGNAL(timeout()), 0, 0);
    timer->stop();

    iconQueue.append(icon);
    setIcon(icon);
    setToolTip(toolTip);



    connect(timer, SIGNAL(timeout()), this, SLOT(slotShowFlash()));
    timer->setInterval(millisecondsTimeoutHint);
    timer->start();

    if (menu) {
        QMenu *trayMenu = contextMenu();
        if (!trayMenu) {
            setContextMenu(menu);
        } else {
            trayMenu->addMenu(menu);
        }
    }




    qDebug() << "~~ timer->isActive():" << timer->isActive();

}

void SystemTrayIconBase::showFlash(const QString &iconName, const QString &toolTip, QMenu *menu,
                                   int millisecondsTimeoutHint)
{


    QIcon ico = ImageResourceBase::createIcon(iconName);
    showFlash(ico, toolTip, menu, millisecondsTimeoutHint);




}

void SystemTrayIconBase::showFlash(const TrayIconData &trayIconData, int millisecondsTimeoutHint)
{

    if(trayIconData.isNull()) {
        return;
    }

    stop();

    this->m_currentData = trayIconData;



    iconQueue = trayIconData.getIconList();
    setIcon(trayIconData.getFirstIcon());
    setToolTip(trayIconData.getToolTip());


    connect(timer, SIGNAL(timeout()), this, SLOT(slotShowFlash()));
    timer->setInterval(millisecondsTimeoutHint);
    timer->start();

    QMenu *menu = trayIconData.getMenu();
    if (menu) {
        QMenu *trayMenu = contextMenu();
        if (!trayMenu) {
            setContextMenu(menu);
        } else {
            trayMenu->addMenu(menu);
        }
    }




}


//连续显示一组图标
//Show a group of icon
void SystemTrayIconBase::showAnimation(QList<QIcon> iconList,
                                       const QString &toolTip, QMenu *menu, int millisecondsTimeoutHint)
{

    qDebug() << "----SystemTrayIconBase::slotShowAnimation()";

    this->m_currentData.setToolTip(toolTip);
    this->m_currentData.setMenu(menu);
    this->m_currentData.settrayIconType(TrayIconData::TRAYICON_Animation);

    disconnect(timer, SIGNAL(timeout()), 0, 0);
    timer->stop();

    iconQueue.clear();
    iconQueue = iconList;
    setIcon(iconList.at(0));
    setToolTip(toolTip);


    connect(timer, SIGNAL(timeout()), this, SLOT(slotShowAnimation()));
    timer->setInterval(millisecondsTimeoutHint);
    timer->start();

    if (menu) {
        QMenu *trayMenu = contextMenu();
        if (!trayMenu) {
            setContextMenu(menu);
        } else {
            trayMenu->addMenu(menu);
        }
    }


}

void SystemTrayIconBase::showAnimation(QStringList iconNameList,
                                       const QString &toolTip, QMenu *menu, int millisecondsTimeoutHint)
{

    disconnect(timer, SIGNAL(timeout()), 0, 0);
    timer->stop();

    iconQueue.clear();
    foreach(QString iconName, iconNameList) {
        QIcon ico = ImageResourceBase::createIcon(iconName);
        iconQueue.append(ico);
    }

    showAnimation(iconQueue, toolTip, menu, millisecondsTimeoutHint);

}

void SystemTrayIconBase::showAnimation(const TrayIconData &trayIconData, int millisecondsTimeoutHint)
{

    if(trayIconData.isNull()) {
        return;
    }

    this->m_currentData = trayIconData;

    disconnect(timer, SIGNAL(timeout()), 0, 0);
    timer->stop();

    iconQueue.clear();
    iconQueue = trayIconData.getIconList();
    setIcon(iconQueue.at(0));
    setToolTip(trayIconData.getToolTip());


    connect(timer, SIGNAL(timeout()), this, SLOT(slotShowAnimation()));
    timer->setInterval(millisecondsTimeoutHint);
    timer->start();

    QMenu *menu = trayIconData.getMenu();
    if (menu) {
        QMenu *trayMenu = contextMenu();
        if (!trayMenu) {
            setContextMenu(menu);
        } else {
            trayMenu->addMenu(menu);
        }
    }


}

void SystemTrayIconBase::stop()
{
    disconnect(timer, SIGNAL(timeout()), 0, 0);
    timer->stop();
    setIcon(m_defaultData.getFirstIcon());
    iconQueue.clear();
    iconIndex = 0;

}

void SystemTrayIconBase::resetAll(const TrayIconData &defaultData)
{

    this->m_defaultData = defaultData;
    this->m_currentData = defaultData;

    setIcon(this->m_defaultData.getFirstIcon());
    setContextMenu(this->m_defaultData.getMenu());
    setToolTip(this->m_defaultData.getToolTip());



}



void SystemTrayIconBase::resetTrayIcon(const QIcon &icon)
{


    this->m_defaultData.setFirstIcon(icon);
    this->m_currentData.setFirstIcon(icon);

    stop();
    setIcon(icon);

}

void SystemTrayIconBase::resetContexMenu(QMenu *contextMenu)
{

    this->m_defaultData.setMenu(contextMenu);
    this->m_currentData.setMenu(contextMenu);

    setContextMenu(contextMenu);



}

void SystemTrayIconBase::resetToolTip(const QString &tooltip)
{
    this->m_defaultData.setToolTip(tooltip);
    this->m_currentData.setToolTip(tooltip);

    setToolTip(tooltip);


}

//void SystemTrayIconBase::setDefault(const QIcon &defaultIcon , const TrayIconData &defaultData){
//    this->defaultIcon = defaultIcon;
//    this->m_defaultData = defaultData;
//}


TrayIconData SystemTrayIconBase::currentTrayIconData() const
{
    return m_currentData;
}

TrayIconData SystemTrayIconBase::defaultTrayIconData() const
{
    return m_defaultData;
}

void SystemTrayIconBase::appendTrayIconData(const TrayIconData &trayIconData)
{

    if(trayIconData.isNull()) {
        return;
    }

    trayIconDataHash.insert(trayIconData.getID(), trayIconData);

    updateSystemTrayIcon();

}

void SystemTrayIconBase::removeTrayIconData(const QString &trayIconDataID)
{
    //qWarning()<<"SystemTrayIconBase::removeTrayIconData(....):"<<trayIconDataID;

    stop();
    trayIconDataHash.remove(trayIconDataID);

    updateSystemTrayIcon();

}

void SystemTrayIconBase::removeTrayIconData(const QString &owner, int dataType)
{
    stop();

//    QHash<QString/*Tray Icon Data ID*/, TrayIconData> trayIconDataHash;
    QList<TrayIconData> dataList = trayIconDataHash.values();
    foreach (TrayIconData data, dataList) {
        if(data.getOwner() == owner && (dataType == data.getDataType()) ) {
            trayIconDataHash.remove(data.getID());
        }
    }

    updateSystemTrayIcon();

}

void SystemTrayIconBase::removeAllTrayIconData(const QString &owner)
{

    stop();

    if(owner.isEmpty()) {
        trayIconDataHash.clear();
    } else {
        QList<TrayIconData> dataList = trayIconDataHash.values();
        foreach (TrayIconData data, dataList) {
            if(data.getOwner() == owner) {
                trayIconDataHash.remove(data.getID());
            }
        }
    }

    updateSystemTrayIcon();

}

void SystemTrayIconBase::removeAllTrayIconData(int dataType)
{

    stop();

    QList<TrayIconData> dataList = trayIconDataHash.values();
    foreach (TrayIconData data, dataList) {
        if(data.getDataType() == dataType) {
            trayIconDataHash.remove(data.getID());
        }
    }

    updateSystemTrayIcon();

}


bool SystemTrayIconBase::setTrayIconData(const QString &trayIconDataID, const QVariant &data)
{

    TrayIconData trayIconData = trayIconDataHash.value(trayIconDataID);
    if(trayIconData.isNull()) {
        return false;
    }

    trayIconData.setData(data);
    trayIconDataHash[trayIconDataID] = trayIconData;

    return true;

}

void SystemTrayIconBase::updateSystemTrayIcon()
{

    m_currentData = TrayIconData();

    TrayIconData trayIconData;
    if(trayIconDataHash.isEmpty()) {
        trayIconData = m_defaultData;
    } else {
        QList<TrayIconData> list = trayIconDataHash.values();
        if(m_showSystemTrayIconDataLastAdded) {
            trayIconData = list.last();
        } else {
            trayIconData = list.first();
        }

    }

    TrayIconData::TrayIconType iconType = trayIconData.getTrayIconType();
    switch(iconType) {
    case TrayIconData::TRAYICON_Normal: {
        showIcon(trayIconData);
    }
    break;
    case TrayIconData::TRAYICON_Flash: {
        showFlash(trayIconData);
    }
    break;
    case TrayIconData::TRAYICON_Animation: {
        showAnimation(trayIconData);
    }
    break;
    default:
        ;

    }

    //TODO:


}

bool SystemTrayIconBase::trayIconDataExists(const QString &id)
{
    return trayIconDataHash.contains(id);
}

QVariant SystemTrayIconBase::getData(const QString &id)
{
    QVariant data;
    if(trayIconDataHash.contains(id)) {
        data = trayIconDataHash.value(id).getData();
    }

    return data;
}

void SystemTrayIconBase::setData(const QString &id, const QVariant &data)
{

    if(trayIconDataHash.contains(id)) {
        TrayIconData trayIconData = trayIconDataHash.value(id);
        trayIconData.setData(data);
        trayIconDataHash[id] = trayIconData;
    } else if(m_currentData.getID() == id) {
        m_currentData.setData(data);

    }

}






void SystemTrayIconBase::slotShowFlash()
{
    //qDebug()<<"----SystemTrayIconBase::showFlash()";

    static QIcon emptyIcon = ImageResourceBase::emptyIcon();
    static int curIndex = 1;

    if(iconIndex >= iconQueue.size()) {
        iconIndex = 0;
    }

    //qDebug()<<"~~ iconIndex:"<<iconIndex;

    if(curIndex < 0) {
        setIcon(emptyIcon);

    } else {
        setIcon(iconQueue.at(iconIndex));
        iconIndex++;
    }

    curIndex *= (-1);



}

void SystemTrayIconBase::slotShowAnimation()
{
    qDebug() << "----SystemTrayIconBase::slotShowAnimation()";

    if(iconIndex >= iconQueue.size()) {
        iconIndex = 0;
    }

    qDebug() << "~~ iconIndex:" << iconIndex;
    qDebug() << "~~ iconQueue.size():" << iconQueue.size();

    setIcon(iconQueue.at(iconIndex));
    iconIndex++;


}










} //namespace HEHUI
