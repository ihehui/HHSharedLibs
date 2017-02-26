/*
 ****************************************************************************
 * systemtrayIconbase.h
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




#ifndef SYSTEMTRAYICONBASE_H_
#define SYSTEMTRAYICONBASE_H_

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMenu>


#include "../guilib.h"


namespace HEHUI
{

class GUI_LIB_API TrayIconData : public QObject
{
    Q_OBJECT

public :
    enum TrayIconType {TRAYICON_Normal, TRAYICON_Flash, TRAYICON_Animation};

    TrayIconData(QObject *parent = 0);
    TrayIconData(int dataType, const QString &id, const QString &owner,
                 const QString &toolTip = "", QMenu *menu = 0, const QVariant &data = QVariant(),
                 TrayIconType trayIconType = TRAYICON_Normal, QList<QIcon> iconList = QList<QIcon>(),
                 QObject *parent = 0);
    TrayIconData(const TrayIconData &trayIconData);
    TrayIconData &operator =(const TrayIconData &trayIconData);

    ~TrayIconData();

    bool isNull() const;

    void setDataType(int dataType)
    {
        this->dataType = dataType;
    }
    int getDataType() const
    {
        return dataType;
    }

    void setID(const QString &id)
    {
        this->id = id;
    }
    QString getID() const
    {
        return id;
    }

    void setOwner(const QString &owner)
    {
        this->owner = owner;
    }
    QString getOwner() const
    {
        return owner;
    }

    void setToolTip(const QString &toolTip)
    {
        this->toolTip = toolTip;
    }
    QString getToolTip() const
    {
        return toolTip;
    }

    void setMenu(QMenu *menu)
    {
        this->menu = menu;
    }
    QMenu *getMenu() const
    {
        return menu;
    }

    void setData(const QVariant &data)
    {
        this->data = data;
    }
    QVariant getData() const
    {
        return data;
    }

    void settrayIconType(const TrayIconType trayIconType)
    {
        this->trayIconType = trayIconType;
    }
    TrayIconType getTrayIconType() const
    {
        return trayIconType;
    }

    void setIconList(const QList<QIcon> &iconList)
    {
        this->iconList = iconList;
    }
    QList<QIcon> getIconList() const
    {
        return iconList;
    }

    void appendIcon(const QIcon &icon)
    {
        this->iconList.append(icon);
    }

    void setFirstIcon(const QIcon &icon)
    {
        if(this->iconList.size()) {
            this->iconList.removeFirst();
        }
        this->iconList.insert(0, icon);
    }
    QIcon getFirstIcon() const
    {
        if(iconList.isEmpty()) {
            return QIcon();
        }
        return iconList[0];
    }

private:
    int dataType;
    QString id;
    QString owner;
    QString toolTip;
    QMenu *menu;
    QVariant data;

    TrayIconType trayIconType;
    QList<QIcon> iconList;


};


class GUI_LIB_API SystemTrayIconBase : public QSystemTrayIcon
{
    Q_OBJECT
public:
    SystemTrayIconBase(QObject *parent = 0);
    SystemTrayIconBase(const TrayIconData &defaultData, QObject *parent = 0);
    SystemTrayIconBase(const QIcon &icon, QObject *parent = 0);

    virtual ~SystemTrayIconBase();


    //Show the icon
    virtual void showIcon(const TrayIconData &trayIconData = TrayIconData());
    virtual void showIcon(const QIcon &icon, const QString &toolTip = "", QMenu *menu = 0);
    virtual void showIcon( const QString &iconName, const QString &toolTip = "", QMenu *menu = 0);



    //Flash the icon
    virtual void showFlash(const QIcon &icon, const QString &toolTip = QString(), QMenu *menu = 0, int millisecondsTimeoutHint = 500);
    virtual void showFlash(const QString &iconName, const QString &toolTip = QString(), QMenu *menu = 0, int millisecondsTimeoutHint = 500);
    virtual void showFlash(const TrayIconData &trayIconData = TrayIconData(), int millisecondsTimeoutHint = 500);



    //Show a group of icon
    virtual void showAnimation(QList<QIcon> iconList, const QString &toolTip = QString(), QMenu *menu = 0, int millisecondsTimeoutHint = 1000);
    virtual void showAnimation(QStringList iconNameList, const QString &toolTip = QString(), QMenu *menu = 0, int millisecondsTimeoutHint = 1000);
    virtual void showAnimation(const TrayIconData &trayIconData = TrayIconData(), int millisecondsTimeoutHint = 1000);



    void stop();

    void resetAll(const TrayIconData &defaultData = TrayIconData());
    void resetTrayIcon(const QIcon &icon);
    void resetContexMenu(QMenu *contextMenu);
    void resetToolTip(const QString &tooltip);
    //        void setDefault(const QIcon &defaultIcon = QIcon(), const TrayIconData &defaultData = TrayIconData());

    TrayIconData currentTrayIconData() const;
    TrayIconData defaultTrayIconData() const;

    void appendTrayIconData(const TrayIconData &trayIconData);
    void removeTrayIconData(const QString &trayIconDataID);
    void removeTrayIconData(const QString &owner, int dataType);

    void removeAllTrayIconData(const QString &owner = "");
    void removeAllTrayIconData(int dataType);

    bool setTrayIconData(const QString &trayIconDataID, const QVariant &data);

    void updateSystemTrayIcon();

    bool trayIconDataExists(const QString &id);
    QVariant getData(const QString &id);
    void setData(const QString &id, const QVariant &data);


private slots:
    void slotShowFlash();
    void slotShowAnimation();

private:

    TrayIconData m_defaultData;

    int iconIndex;

    QList<QIcon> iconQueue;

    QTimer *timer;

    TrayIconData m_currentData;

    QHash<QString/*Tray Icon Data ID*/, TrayIconData> trayIconDataHash;

    bool m_showSystemTrayIconDataLastAdded;



};

} //namespace HEHUI

#endif /* SYSTEMTRAYICONBASE_H_ */
