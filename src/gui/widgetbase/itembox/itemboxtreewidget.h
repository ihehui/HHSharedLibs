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

#ifndef WIDGETBOXTREEWIDGET_H
#define WIDGETBOXTREEWIDGET_H

#include "itemboxwidgetbase_p.h"

#include <QtWidgets/QTreeWidget>
#include <QtGui/QIcon>
#include <QtCore/QList>
#include <QtCore/QHash>
//#include <QTimer>



namespace HEHUI {

class ItemBoxEventHandler;

class ItemBoxCategoryListView;

// ItemBoxTreeWidget: A tree of categories

class ItemBoxTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    typedef ItemBoxWidgetInterface::Item Item;
    typedef ItemBoxWidgetInterface::Category Category;
    typedef ItemBoxWidgetInterface::CategoryList CategoryList;

    explicit ItemBoxTreeWidget(ItemBoxEventHandler *core, QWidget *parent = 0);
    ~ItemBoxTreeWidget();


    int categoryCount() const;
    Category category(int cat_idx) const;
    Category category(const QString &cat_id) const;
    /*static*/ int findCategory(const QString &cat_id, const ItemBoxTreeWidget::CategoryList &list);

    void addCategory(const Category &cat, int index = -1);
    void removeCategory(int cat_idx);
    void removeCategory(const QString &cat_id);
    void hideCategory(const QString &cat_id);
    void setCategoryHidden(int cat_idx, bool hide);
    void setCategoryHidden(const QString &cat_id, bool hide);
    void setCategoryExpanded(int cat_idx, bool expand);
    void setCategoryExpanded(const QString &cat_id, bool expand);

    void updateCategoryName(const QString &cat_id, const QString &new_cat_name);

    int itemCount(int cat_idx) const;
    Item item(int cat_idx, int item_idx) const;
    Item item(const QString &cat_id, const QString &item_id) const;

    void addItem(int cat_idx, const Item &item);
    void addItem(const QString &cat_id, const Item &item);

    void removeItem(int cat_idx, int wgt_idx);
    void removeItem(const QString &cat_id, const QString &item_id);

    void moveItem(int old_cat_idx, int new_cat_idx, const QString &item_id);
    void moveItem(const QString &old_cat_id, const QString &new_cat_id, const QString &item_id);


//    void dropItems(const QList<ItemBoxWidgetInterface *> &item_list);

//    void setFileName(const QString &file_name);
//    QString fileName() const;
//    bool load(ItemBoxWidgetBase::LoadMode loadMode);
//    bool loadContents(const QString &contents);
//    bool save();



//    void setLoadMode(ItemBoxWidgetBase::LoadMode loadMode);

    bool updateItemName(const QString &cat_id, const QString &item_id, const QString &itemName);
    bool updateItemIcon(const QString &cat_id, const QString &item_id, const QIcon &icon);
//    bool updateItemIcon(const QString &cat_id, const QString &item_id, const QString &iconName);


signals:
//    void pressed(const QString name, const QString dom_xml, const QPoint &global_mouse_pos);

public slots:
    void filter(const QString &);

protected:
    void contextMenuEvent(QContextMenuEvent *e);
    void resizeEvent(QResizeEvent *e);

private slots:
//    void slotSave();
    void slotScratchPadItemDeleted();
    void slotLastScratchPadItemDeleted();

    void handleMousePress(QTreeWidgetItem *item);
    void deleteScratchpad();
    void slotListMode();
    void slotIconMode();

private:
    ItemBoxCategoryListView *addCategoryView(QTreeWidgetItem *parent, bool iconMode);
    ItemBoxCategoryListView *categoryViewAt(int idx) const;
    ItemBoxCategoryListView *categoryView(const QString &cat_id) const;

    void adjustSubListSize(QTreeWidgetItem *cat_item);

//    static bool readCategories(const QString &fileName, const QString &xml, CategoryList *cats, QString *errorMessage);
//    static bool readWidget(Item *w, const QString &xml, QXmlStreamReader &r);

//    CategoryList loadCustomCategoryList() const;
//    void writeCategories(QXmlStreamWriter &writer, const CategoryList &cat_list) const;

    int indexOfCategory(const QString &cat_id) const;
    int indexOfScratchpad() const;
    int ensureScratchpad();
//    void addCustomCategories(bool replace);

//    void saveExpandedState() const;
//    void restoreExpandedState();
    void updateViewMode();

    ItemBoxEventHandler *m_core;
//    QString m_file_name;
    typedef QHash<QString, QIcon> IconCache;
    mutable IconCache m_pluginIcons;
    bool m_iconMode;
    QTimer *m_scratchPadDeleteTimer;

//    ItemBoxWidgetBase::LoadMode m_loadMode;


};

} //namespace HEHUI

#endif // WIDGETBOXTREEWIDGET_H
