/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "itemboxwidgetbase_p.h"

#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QSharedData>


namespace HEHUI
{


class ItemBoxItemData : public QSharedData
{
public:
    ItemBoxItemData(const QString &aid, const QString &aname,
                    const QIcon &icon,
                    ItemBoxWidgetInterface::Item::Type atype);
    QString m_id;
    QString m_name;
    QIcon m_icon;
    //QIcon::Mode m_icon_mode;
    ItemBoxWidgetInterface::Item::Type m_type;
};

ItemBoxItemData::ItemBoxItemData(const QString &aid, const QString &aname,
                                 const QIcon &icon,
                                 ItemBoxWidgetInterface::Item::Type atype) :
    m_id(aid), m_name(aname), m_icon(icon), m_type(atype)
{
}

ItemBoxWidgetInterface::Item::Item(const QString &aid, const QString &aname,
                                   const QIcon &icon, Type atype) :
    m_data(new ItemBoxItemData(aid, aname, icon, atype))
{
}

ItemBoxWidgetInterface::Item::~Item()
{
}

ItemBoxWidgetInterface::Item::Item(const Item &w) :
    m_data(w.m_data)
{
}

ItemBoxWidgetInterface::Item &ItemBoxWidgetInterface::Item::operator=(const Item &rhs)
{
    if (this != &rhs) {
        m_data = rhs.m_data;
    }
    return *this;
}

QString ItemBoxWidgetInterface::Item::id() const
{
    return m_data->m_id;
}

void ItemBoxWidgetInterface::Item::setID(const QString &aid)
{
    m_data->m_id = aid;
}

QString ItemBoxWidgetInterface::Item::name() const
{
    return m_data->m_name;
}

void ItemBoxWidgetInterface::Item::setName(const QString &aname)
{
    m_data->m_name = aname;
}

QIcon ItemBoxWidgetInterface::Item::icon() const
{
    return m_data->m_icon;
}

void ItemBoxWidgetInterface::Item::setIcon(const QIcon &icon)
{
    m_data->m_icon = icon;
}

//QIcon::Mode ItemBoxWidgetInterface::Item::iconMode() const{
//    return m_data->m_icon_mode;
//}

//void ItemBoxWidgetInterface::Item::setIconMode(QIcon::Mode icon_mode){
//    m_data->m_icon_mode = icon_mode;
//}

ItemBoxWidgetInterface::Item::Type ItemBoxWidgetInterface::Item::type() const
{
    return m_data->m_type;
}

void ItemBoxWidgetInterface::Item::setType(Type atype)
{
    m_data->m_type = atype;
}

bool ItemBoxWidgetInterface::Item::isNull() const
{
    return m_data->m_name.isEmpty();
}


ItemBoxWidgetBase::ItemBoxWidgetBase(QWidget *parent, Qt::WindowFlags flags)
    : ItemBoxWidgetInterface(parent, flags),
      m_loadMode(LoadMerge)
{

}

ItemBoxWidgetBase::LoadMode ItemBoxWidgetBase::loadMode() const
{
    return m_loadMode;
}

void ItemBoxWidgetBase::setLoadMode(LoadMode lm)
{
    m_loadMode = lm;
}

// Convenience to find a Item by class name
bool ItemBoxWidgetBase::findItem(const ItemBoxWidgetInterface *ibox,
                                 const QString &itemID,
                                 const QString &category,
                                 Item *itemData)
{
    // Note that entry names do not necessarily match the class name
    // (at least, not for the standard widgets), so,
    // look in the XML for the class name of the first Item to appear

    QString pattern = itemID;
    QRegExp regexp(pattern);
    Q_ASSERT(regexp.isValid());
    const int catCount = ibox->categoryCount();
    for (int c = 0; c < catCount; c++) {
        const Category cat = ibox->category(c);
        if (category.isEmpty() || cat.name() == category) {
            const int itemCount =  cat.itemCount();
            for (int w = 0; w < itemCount; w++) {
                const Item item = cat.item(w);
                if (regexp.exactMatch(pattern)) {
                    *itemData = item;
                    return true;
                }
            }
        }
    }
    return false;
}




} //namespace HEHUI
