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

#ifndef ABSTRACTITEMWIDGETBOX_H
#define ABSTRACTITEMWIDGETBOX_H


#include <QtCore/QSharedDataPointer>
#include <QtCore/QMetaType>
#include <QtWidgets/QWidget>
#include <QtGui/QIcon>

#include "itemboxlib.h"



namespace HEHUI
{


class ItemBoxItemData;

class ITEMBOX_LIB_API ItemBoxWidgetInterface : public QWidget
{
    Q_OBJECT
public:
    class ITEMBOX_LIB_API Item
    {
    public:
        enum Type { Default, Custom };
        Item(const QString &aname = QString(), const QString &xml = QString(),
             const QIcon &icon = QIcon(), Type atype = Default);
        ~Item();
        Item(const Item &w);
        Item &operator=(const Item &w);

        QString id() const;
        void setID(const QString &aid);
        QString name() const;
        void setName(const QString &aname);
        QIcon icon() const;
        void setIcon(const QIcon &icon);
        //QIcon::Mode iconMode() const;
        //void setIconMode(QIcon::Mode icon_mode);

        Type type() const;
        void setType(Type atype);

        bool isNull() const;

    private:
        QSharedDataPointer<ItemBoxItemData> m_data;
    };

    typedef QList<Item> ItemList;

    class Category
    {
    public:
        enum Type { Default, Scratchpad };

        Category(const QString &aid = QString(), const QString &aname = QString(), Type atype = Default)
            : m_id(aid), m_name(aname), m_type(atype) {}

        QString id() const
        {
            return m_id;
        }
        void setID(const QString &id)
        {
            m_id = id;
        }
        QString name() const
        {
            return m_name;
        }
        void setName(const QString &aname)
        {
            m_name = aname;
        }
        int itemCount() const
        {
            return m_item_list.size();
        }
        Item item(int idx) const
        {
            return m_item_list.at(idx);
        }
        void removeItem(int idx)
        {
            m_item_list.removeAt(idx);
        }
        void addItem(const Item &aitem)
        {
            m_item_list.append(aitem);
        }
        Type type() const
        {
            return m_type;
        }
        void setType(Type atype)
        {
            m_type = atype;
        }

        bool isNull() const
        {
            return m_id.isEmpty();
        }

    private:
        QString m_id;
        QString m_name;
        Type m_type;
        QList<Item> m_item_list;
    };
    typedef QList<Category> CategoryList;

    ItemBoxWidgetInterface(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~ItemBoxWidgetInterface();

    virtual int categoryCount() const = 0;
    virtual Category category(int cat_idx) const = 0;
    virtual void addCategory(const Category &cat, int index = -1) = 0;
    virtual void removeCategory(int cat_idx) = 0;

    virtual int itemCount(int cat_idx) const = 0;
    virtual Item item(int cat_idx, int wgt_idx) const = 0;
    virtual void addItem(int cat_idx, const Item &wgt) = 0;
    virtual void removeItem(int cat_idx, int wgt_idx) = 0;

//    int findOrInsertCategory(const QString &categoryName);

//    virtual void dropItems(const QList<ItemBoxWidgetInterface*> &item_list, const QPoint &global_mouse_pos) = 0;

//    virtual void setFileName(const QString &file_name) = 0;
//    virtual QString fileName() const = 0;
//    virtual bool load() = 0;
//    virtual bool save() = 0;
};


} //namespace HEHUI

Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(HEHUI::ItemBoxWidgetInterface::Item))


#endif // ABSTRACTITEMBOX_H
