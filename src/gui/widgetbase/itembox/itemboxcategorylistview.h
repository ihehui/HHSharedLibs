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

#ifndef WIDGETBOXCATEGORYLISTVIEW_H
#define WIDGETBOXCATEGORYLISTVIEW_H

#include "abstractitemboxwidget.h"
#include "itemboxeventhandler.h"

#include <QtWidgets/QListView>
#include <QtCore/QList>

#include <QSortFilterProxyModel>
#include <QRegExp>
#include <QEvent>


namespace HEHUI
{


class ItemBoxCategoryModel;

// List view of a category, switchable between icon and list mode.
// Provides a filtered view.
class ItemBoxCategoryListView : public QListView
{
    Q_OBJECT
public:
    // Whether to access the filtered or unfiltered view
    enum AccessMode { FilteredAccess, UnfilteredAccess };

    explicit ItemBoxCategoryListView(ItemBoxEventHandler *core, QWidget *parent = 0);
    void setViewMode(ViewMode vm);

    void dropItems(const QList<ItemBoxWidgetInterface *> &item_list);

    using QListView::contentsSize;

    // These methods operate on the filtered/unfiltered model according to accessmode
    int count(AccessMode am) const;
    ItemBoxWidgetInterface::Item itemAt(AccessMode am, const QModelIndex &index) const;
    ItemBoxWidgetInterface::Item itemAt(AccessMode am, int row) const;
    ItemBoxWidgetInterface::Item item(const QString &item_id);
    int indexOfItem(const QString &item_id);


    void removeRow(AccessMode am, int row);
    void setCurrentItem(AccessMode am, int row);

    // These methods operate on the unfiltered model and are used for serialization
    void addItem(const ItemBoxWidgetInterface::Item &item, const QIcon &icon, bool editable);
    bool containsItem(const QString &item_id);
    ItemBoxWidgetInterface::Category category() const;
    bool removeCustomItems();

    bool updateItemName(const QString &item_id, const QString &name);
    bool updateItemIcon(const QString &item_id, const QIcon &icon);
//    bool updateItemIcon(const QString &item_id, const QString &iconName);


protected:
    bool event(QEvent *event);

signals:
//    void scratchPadChanged();
//    void pressed(const QString &item_id, const QPoint &globalPos);
    void itemRemoved();
    void lastItemRemoved();

public slots:
    void filter(const QRegExp &re);

private slots:
    void slotPressed(const QModelIndex &index);
    void removeCurrentItem();
    void editCurrentItem();

    void slotItemActivated(const QModelIndex &index);


private:
    int mapRowToSource(int filterRow) const;

private:
    ItemBoxEventHandler *m_core;
    QSortFilterProxyModel *m_proxyModel;
    ItemBoxCategoryModel *m_model;

    static ItemBoxCategoryListView *lastFocusedCategoryListView;


};

} //namespace HEHUI

#endif // WIDGETBOXCATEGORYLISTVIEW_H
