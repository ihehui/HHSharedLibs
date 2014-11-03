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

#include "itemboxcategorylistview.h"


#include <QtGui/QIcon>
#include <QtWidgets/QListView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QItemDelegate>
#include <QtCore/QSortFilterProxyModel>

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <QContextMenuEvent>
#include <QDebug>


namespace HEHUI {

enum { FilterRole = Qt::UserRole + 11 };

// Entry of the model list

struct ItemBoxCategoryEntry {
    ItemBoxCategoryEntry();
    explicit ItemBoxCategoryEntry(const ItemBoxWidgetInterface::Item &item,
                                    const QString &filter,
                                    const QIcon &icon,
                                    bool editable);

    ItemBoxWidgetInterface::Item item;
    QString toolTip;
    QString whatsThis;
    QString filter;
    QIcon icon;
    bool editable;
};


ItemBoxCategoryEntry::ItemBoxCategoryEntry() :
    editable(false)
{
}

ItemBoxCategoryEntry::ItemBoxCategoryEntry(const ItemBoxWidgetInterface::Item &w,
                                               const QString &filterIn,
                                               const QIcon &i, bool e) :
    item(w),
    filter(filterIn),
    icon(i),
    editable(e)
{
}

/* ItemBoxCategoryModel, representing a list of category entries. Uses a
 * QAbstractListModel since the behaviour depends on the view mode of the list
 * view, it does not return text in the case of IconMode. */

class ItemBoxCategoryModel : public QAbstractListModel {
public:
    explicit ItemBoxCategoryModel(ItemBoxEventHandler *core, QObject *parent = 0);

    // QAbstractListModel
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags (const QModelIndex & index ) const;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    // The model returns no text in icon mode, so, it also needs to know it
    QListView::ViewMode viewMode() const;
    void setViewMode(QListView::ViewMode vm);

    void addItem(const ItemBoxWidgetInterface::Item &item, const QIcon &icon, bool editable);

    ItemBoxWidgetInterface::Item itemAt(const QModelIndex & index) const;
    ItemBoxWidgetInterface::Item itemAt(int row) const;
    ItemBoxWidgetInterface::Item item(const QString &item_id) const;
    int indexOfItem(const QString &item_id) const;


    int indexOfWidget(const QString &name);

    ItemBoxWidgetInterface::Category category() const;
    bool removeCustomWidgets();

    bool updateItemName(const QString &item_id, const QString &name);
    bool updateItemIcon(const QString &item_id, const QIcon &icon);
//    bool updateItemIcon(const QString &item_id, const QString &iconName);


private:
    typedef QList<ItemBoxCategoryEntry> ItemBoxCategoryEntrys;

    QRegExp m_classNameRegExp;
    ItemBoxEventHandler *m_core;
    ItemBoxCategoryEntrys m_categoryentrys;
    QListView::ViewMode m_viewMode;
};

ItemBoxCategoryModel::ItemBoxCategoryModel(ItemBoxEventHandler *core, QObject *parent) :
    QAbstractListModel(parent),
    m_classNameRegExp(QStringLiteral("<widget +class *= *\"([^\"]+)\"")),
    m_core(core),
    m_viewMode(QListView::ListMode)
{
    Q_ASSERT(m_classNameRegExp.isValid());
}

QListView::ViewMode ItemBoxCategoryModel::viewMode() const
{
    return m_viewMode;
}

void ItemBoxCategoryModel::setViewMode(QListView::ViewMode vm)
{
    if (m_viewMode == vm)
        return;
    const bool empty = m_categoryentrys.isEmpty();
    if (!empty)
        beginResetModel();
    m_viewMode = vm;
    if (!empty)
        endResetModel();
}

int ItemBoxCategoryModel::indexOfWidget(const QString &name)
{
    const int count = m_categoryentrys.size();
    for (int  i = 0; i < count; i++)
        if (m_categoryentrys.at(i).item.name() == name)
            return i;
    return -1;
}

ItemBoxWidgetInterface::Category ItemBoxCategoryModel::category() const
{
    ItemBoxWidgetInterface::Category rc;
    const ItemBoxCategoryEntrys::const_iterator cend = m_categoryentrys.constEnd();
    for (ItemBoxCategoryEntrys::const_iterator it = m_categoryentrys.constBegin(); it != cend; ++it)
        rc.addItem(it->item);
    return rc;
}

bool ItemBoxCategoryModel::removeCustomWidgets()
{
    // Typically, we are a whole category of custom widgets, so, remove all
    // and do reset.
    bool changed = false;
    for (ItemBoxCategoryEntrys::iterator it = m_categoryentrys.begin(); it != m_categoryentrys.end(); )
        if (it->item.type() == ItemBoxWidgetInterface::Item::Custom) {
            if (!changed)
                beginResetModel();
            it = m_categoryentrys.erase(it);
            changed = true;
        } else {
            ++it;
        }
    if (changed)
        endResetModel();
    return changed;
}

bool ItemBoxCategoryModel::updateItemName(const QString &item_id, const QString &name){

    QModelIndex idx = index(indexOfItem(item_id));
    if(!idx.isValid()){
        return false;
    }

    setData(idx, name, Qt::DisplayRole);
    return true;

}

bool ItemBoxCategoryModel::updateItemIcon(const QString &item_id, const QIcon &icon){

    QModelIndex idx = index(indexOfItem(item_id));
    if(!idx.isValid()){
        return false;
    }

    setData(idx, icon, Qt::DecorationRole);

    return true;

}

//bool ItemBoxCategoryModel::updateItemIcon(const QString &item_id, const QString &iconName){

//    QModelIndex idx = index(indexOfItem(item_id));
//    if(!idx.isValid()){
//        return false;
//    }

//    setData(idx, iconName, Qt::DecorationRole);

//    return true;

//}

void ItemBoxCategoryModel::addItem(const ItemBoxWidgetInterface::Item &item, const QIcon &icon,bool editable)
{
    // build item. Filter on name + class name if it is different and not a layout.
    QString filter = item.id();
    //TODO:Unique Name?
//    if (!filter.contains(QStringLiteral("Layout")) && m_classNameRegExp.indexIn(item.domXml()) != -1) {
//        const QString className = m_classNameRegExp.cap(1);
//        if (!filter.contains(className))
//            filter += className;
//    }
    ItemBoxCategoryEntry categoryEntry(item, filter, icon, editable);
//    const QDesignerWidgetDataBaseInterface *db = m_core->widgetDataBase();
//    const int dbIndex = db->indexOfClassName(item.name());
//    if (dbIndex != -1) {
//        const QDesignerWidgetDataBaseItemInterface *dbItem = db->item(dbIndex);
//        const QString toolTip = dbItem->toolTip();
//        if (!toolTip.isEmpty())
//            categoryEntry.toolTip = toolTip;
//        const QString whatsThis = dbItem->whatsThis();
//        if (!whatsThis.isEmpty())
//            categoryEntry.whatsThis = whatsThis;
//    }


    const QString toolTip = item.id();
    if (!toolTip.isEmpty())
        categoryEntry.toolTip = toolTip;
    const QString whatsThis = item.id();
    if (!whatsThis.isEmpty())
        categoryEntry.whatsThis = whatsThis;



    // insert
    const int row = m_categoryentrys.size();
    beginInsertRows(QModelIndex(), row, row);
    m_categoryentrys.push_back(categoryEntry);
    endInsertRows();
}

QVariant ItemBoxCategoryModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (row < 0 || row >=  m_categoryentrys.size())
        return QVariant();

    const ItemBoxCategoryEntry &categoryEntry = m_categoryentrys.at(row);
    switch (role) {
    case Qt::DisplayRole:
        // No text in icon mode
        return QVariant(m_viewMode == QListView::ListMode ? categoryEntry.item.name() : QString());
    case Qt::DecorationRole:
        return QVariant(categoryEntry.icon);
    case Qt::EditRole:
        return QVariant(categoryEntry.item.name());
//    case Qt::ToolTipRole: {
//        if (m_viewMode == QListView::ListMode)
//            return QVariant(categoryEntry.toolTip);
//        // Icon mode tooltip should contain the  class name
//        QString tt =  categoryEntry.item.name();
//        if (!categoryEntry.toolTip.isEmpty()) {
//            tt += QLatin1Char('\n');
//            tt += categoryEntry.toolTip;
//        }
//        return QVariant(tt);

//    }
    case Qt::WhatsThisRole:
        return QVariant(categoryEntry.whatsThis);
    case FilterRole:
        return categoryEntry.filter;
    }
    return QVariant();
}

bool ItemBoxCategoryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    const int row = index.row();
    if ( row < 0 || row >=  m_categoryentrys.size() ){
        return false;
    }
    //if (role != Qt::EditRole || row < 0 || row >=  m_categoryEntries.size() || value.type() != QVariant::String)
    //   return false;

    ItemBoxCategoryEntry &categoryEntry = m_categoryentrys[row];

    if(role == Qt::DisplayRole && value.type() == QVariant::String){
        const QString newName = value.toString();
        categoryEntry.item.setName(newName);

//        const QDomDocument doc = stringToDom(CategoryListView::objectItemDomXml(categoryEntry.objectItem));
//        QDomElement widget_elt = doc.firstChildElement(QLatin1String(widgetElementC));
//        if (!widget_elt.isNull()) {
//            widget_elt.setAttribute(QLatin1String(nameAttributeC), newName);
//            categoryEntry.objectItem.setDomXml(domToString(widget_elt));
//        }
    }

    if(role == Qt::DecorationRole){
        if( value.type() == QVariant::Icon){
            categoryEntry.icon = value.value<QIcon>();
        }//else if(value.type() == QVariant::String){
            //const QString newIconName = value.toString();
            //categoryEntry.item.setIcon(newIconName);
            //categoryEntry.icon = QIcon(newIconName);
        //}
    }

    emit dataChanged(index, index);
    return true;


//    const int row = index.row();
//    if (role != Qt::EditRole || row < 0 || row >=  m_categoryentrys.size() || value.type() != QVariant::String)
//        return false;
//    // Set name and adapt Xml
//    ItemBoxCategoryEntry &item = m_categoryentrys[row];
//    const QString newName = value.toString();
//    item.widget.setName(newName);

//    const QDomDocument doc = stringToDom(ItemBoxCategoryListView::widgetDomXml(item.widget));
//    QDomElement widget_elt = doc.firstChildElement(QLatin1String(widgetElementC));
//    if (!widget_elt.isNull()) {
//        widget_elt.setAttribute(QLatin1String(nameAttributeC), newName);
//        item.widget.setDomXml(domToString(widget_elt));
//    }
//    emit dataChanged(index, index);
//    return true;
}

Qt::ItemFlags ItemBoxCategoryModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = Qt::ItemIsEnabled;
    const int row = index.row();
    if (row >= 0 && row <  m_categoryentrys.size())
        if (m_categoryentrys.at(row).editable) {
            rc |= Qt::ItemIsSelectable;
            // Can change name in list mode only
            if (m_viewMode == QListView::ListMode)
                rc |= Qt::ItemIsEditable;
        }
    return rc;
}

int ItemBoxCategoryModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_categoryentrys.size();
}

bool ItemBoxCategoryModel::removeRows(int row, int count, const QModelIndex & parent)
{
    if (row < 0 || count < 1)
        return false;
    const int size = m_categoryentrys.size();
    const int last =  row + count - 1;
    if (row >= size || last >= size)
        return false;
    beginRemoveRows(parent, row, last);
    for (int r = last; r >= row; r--)
         m_categoryentrys.removeAt(r);
    endRemoveRows();
    return true;
}

ItemBoxWidgetInterface::Item ItemBoxCategoryModel::itemAt(const QModelIndex & index) const
{
    return itemAt(index.row());
}

ItemBoxWidgetInterface::Item ItemBoxCategoryModel::itemAt(int row) const
{
    if (row < 0 || row >=  m_categoryentrys.size())
        return ItemBoxWidgetInterface::Item();
    return m_categoryentrys.at(row).item;
}

ItemBoxWidgetInterface::Item ItemBoxCategoryModel::item(const QString &item_id) const{
    foreach(ItemBoxCategoryEntry entry, m_categoryentrys){
        if(entry.item.id() == item_id){
            return entry.item;
        }
    }

    return ItemBoxWidgetInterface::Item();

}

int ItemBoxCategoryModel::indexOfItem(const QString &item_id) const{
    const int count = m_categoryentrys.size();
    for (int  i = 0; i < count; i++)
        if (m_categoryentrys.at(i).item.id() == item_id)
            return i;
    return -1;
}

/* ItemBoxCategoryEntryDelegate, ensures a valid name using a regexp validator */

class ItemBoxCategoryEntryDelegate : public QItemDelegate
{
public:
    explicit ItemBoxCategoryEntryDelegate(QWidget *parent = 0) : QItemDelegate(parent) {}
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
};

QWidget *ItemBoxCategoryEntryDelegate::createEditor(QWidget *parent,
                                                const QStyleOptionViewItem &option,
                                                const QModelIndex &index) const
{
    QWidget *result = QItemDelegate::createEditor(parent, option, index);
    if (QLineEdit *line_edit = qobject_cast<QLineEdit*>(result)) {
        QRegExp re = QRegExp(QStringLiteral("[_a-zA-Z][_a-zA-Z0-9]*"));
        Q_ASSERT(re.isValid());
        line_edit->setValidator(new QRegExpValidator(re, line_edit));
    }
    return result;
}

// ----------------------  ItemBoxCategoryListView　----------------------

ItemBoxCategoryListView *ItemBoxCategoryListView::lastFocusedCategoryListView = 0;

ItemBoxCategoryListView::ItemBoxCategoryListView(ItemBoxEventHandler *core, QWidget *parent) :
    QListView(parent),
    m_core(core),
    m_proxyModel(new QSortFilterProxyModel(this)),
    m_model(new ItemBoxCategoryModel(core, this))
{
    //setFocusPolicy(Qt::StrongFocus);
    setFrameShape(QFrame::NoFrame);
    setIconSize(QSize(22, 22));
    setSpacing(1);
    setTextElideMode(Qt::ElideMiddle);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setResizeMode(QListView::Adjust);
    setUniformItemSizes(true);

    setItemDelegate(new ItemBoxCategoryEntryDelegate(this));

    connect(this, SIGNAL(pressed(QModelIndex)), this, SLOT(slotPressed(QModelIndex)));
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(slotItemActivated(QModelIndex)));

//    setEditTriggers(QAbstractItemView::AnyKeyPressed);

    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterRole(FilterRole);
    setModel(m_proxyModel);
//    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(scratchPadChanged()));


}

void ItemBoxCategoryListView::setViewMode(ViewMode vm)
{
    QListView::setViewMode(vm);
    m_model->setViewMode(vm);
}

void ItemBoxCategoryListView::setCurrentItem(AccessMode am, int row)
{
    const QModelIndex index = am == FilteredAccess ?
        m_proxyModel->index(row, 0) :
        m_proxyModel->mapFromSource(m_model->index(row, 0));

    if (index.isValid())
        setCurrentIndex(index);
}

void ItemBoxCategoryListView::slotPressed(const QModelIndex &index)
{
    //TODO
    m_core->handleTooltipEventOnItem("", QPoint(0,0), QPoint(0,0));

    const ItemBoxWidgetInterface::Item itm = m_model->itemAt(m_proxyModel->mapToSource(index));
    if (itm.isNull())
        return;

    m_core->handleItemPressed(itm.id(), QCursor::pos());
//    emit pressed(itm.id(), QCursor::pos());

}

void ItemBoxCategoryListView::removeCurrentItem()
{
    const QModelIndex index = currentIndex();
    if (!index.isValid() || !m_proxyModel->removeRow(index.row()))
        return;

    // We check the unfiltered item count here, we don't want to get removed if the
    // filtered view is empty
    if (m_model->rowCount()) {
        emit itemRemoved();
    } else {
        emit lastItemRemoved();
    }
}

void ItemBoxCategoryListView::editCurrentItem()
{
    const QModelIndex index = currentIndex();
    if (index.isValid())
        edit(index);
}

void ItemBoxCategoryListView::slotItemActivated(const QModelIndex &index)
{
    /*const*/ ItemBoxWidgetInterface::Item itm = m_model->itemAt(m_proxyModel->mapToSource(index));
    if (itm.isNull())
        return;

    QString id = itm.id();
    m_core->handleItemActivated(id);
}


int ItemBoxCategoryListView::count(AccessMode am) const
{
    return am == FilteredAccess ? m_proxyModel->rowCount() : m_model->rowCount();
}

int ItemBoxCategoryListView::mapRowToSource(int filterRow) const
{
    const QModelIndex filterIndex = m_proxyModel->index(filterRow, 0);
    return m_proxyModel->mapToSource(filterIndex).row();
}

ItemBoxWidgetInterface::Item ItemBoxCategoryListView::itemAt(AccessMode am, const QModelIndex & index) const
{
    const QModelIndex unfilteredIndex = am == FilteredAccess ? m_proxyModel->mapToSource(index) : index;
    return m_model->itemAt(unfilteredIndex);
}

ItemBoxWidgetInterface::Item ItemBoxCategoryListView::itemAt(AccessMode am, int row) const
{
    return m_model->itemAt(am == UnfilteredAccess ? row : mapRowToSource(row));
}

ItemBoxWidgetInterface::Item ItemBoxCategoryListView::item(const QString &item_id){
    return m_model->item(item_id);
}

int ItemBoxCategoryListView::indexOfItem(const QString &item_id){
    return m_model->indexOfItem(item_id);
}

void ItemBoxCategoryListView::removeRow(AccessMode am, int row)
{
    m_model->removeRow(am == UnfilteredAccess ? row : mapRowToSource(row));
    viewport()->update();
}

bool ItemBoxCategoryListView::containsItem(const QString &item_id)
{
    return m_model->indexOfWidget(item_id) != -1;
}

void ItemBoxCategoryListView::addItem(const ItemBoxWidgetInterface::Item &item, const QIcon &icon, bool editable)
{
    m_model->addItem(item, icon, editable);
    viewport()->update();
}

void ItemBoxCategoryListView::filter(const QRegExp &re)
{
    m_proxyModel->setFilterRegExp(re);
}

ItemBoxWidgetInterface::Category ItemBoxCategoryListView::category() const
{
    return m_model->category();
}

bool ItemBoxCategoryListView::removeCustomItems()
{
    return m_model->removeCustomWidgets();
}

bool ItemBoxCategoryListView::updateItemName(const QString &item_id, const QString &name){
    //qDebug()<<"----CategoryListView::updateObjectItemName(const QString &item_id, const QIcon &icon)";

    bool ok = m_model->updateItemName(item_id, name);
    viewport()->update();
    return ok;

}

bool ItemBoxCategoryListView::updateItemIcon(const QString &item_id, const QIcon &icon){
    //qDebug()<<"----CategoryListView::updateObjectItemIcon(const QString &item_id, const QIcon &icon)";

    bool ok =  m_model->updateItemIcon(item_id, icon);
    viewport()->update();
    return ok;

}

//bool ItemBoxCategoryListView::updateItemIcon(const QString &item_id, const QString &iconName){
//    //qDebug()<<"----CategoryListView::updateObjectItemIcon(const QString &item_id, const QString &iconName)";

//    bool ok = m_model->updateItemIcon(item_id, iconName);
//    viewport()->update();
//    return ok;

//}

bool ItemBoxCategoryListView::event(QEvent *event){

    switch(event->type()){
    case QEvent::FocusIn:
    {
        if(lastFocusedCategoryListView != this){
            if(lastFocusedCategoryListView){
                lastFocusedCategoryListView->clearSelection();
            }
            lastFocusedCategoryListView = this;
        }

    }
        break;
        //    case QEvent::FocusOut:
        //    {
        //        QPoint cursorpos = QCursor::pos();
        //        QPoint topleft = mapToGlobal(pos());
        //        if(cursorpos.x() < topleft.x() || cursorpos.x() > (topleft.x() + width()) || cursorpos.y() < topleft.y() || cursorpos.y() > (topleft.y() + height()) ){
        //            clearSelection();
        //        }

        //        qDebug()<<"--QEvent::FocusOut";
        //        //return true;
        //    }
        //    break;
    case QEvent::ContextMenu:
    {
        QContextMenuEvent *e = static_cast<QContextMenuEvent *> (event);
        if(!e){return false;}

        QString itemID = "";
        QModelIndex index = indexAt(e->pos());
        if(!index.isValid()){return false;}
        ItemBoxWidgetInterface::Item item = itemAt(UnfilteredAccess, index);
        if(!item.isNull()){
            itemID = item.id();
        }

        m_core->handleContextMenuEventOnItem(itemID, mapToGlobal(e->pos()), 0);

        return true;
    }
        break;
    case QEvent::ToolTip:
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        if(!helpEvent){return false;}

        QString itemID = "";
        QModelIndex index = indexAt(helpEvent->pos());
        if(!index.isValid()){return false;}
        ItemBoxWidgetInterface::Item item = itemAt(UnfilteredAccess, index);
        if(item.isNull()){
            return false;
        }
        itemID = item.id();

        QRect rect = visualRect(index);
        QPoint globalvisualRectTopLeft = mapToGlobal(rect.topLeft());
        QPoint globalMousePos = helpEvent->globalPos();

        QSize iconsize = iconSize();
        if(globalMousePos.x() < globalvisualRectTopLeft.x() || globalMousePos.x() > (globalvisualRectTopLeft.x() + iconsize.width()) || globalMousePos.y() < globalvisualRectTopLeft.y() || globalMousePos.y() > (globalvisualRectTopLeft.y() + iconsize.height()) ){
            //m_core->slotTooltipEventOnObjectItemOccurs("", globalvisualRectTopLeft, globalMousePos);
            return false;
        }

        //m_core->slotTooltipEventOnObjectItemOccurs(itemID, helpEvent->globalPos());
        m_core->handleTooltipEventOnItem(itemID, globalvisualRectTopLeft, globalMousePos);

        return true;
    }
        break;
    case QEvent::Resize:
    {
        QResizeEvent *e = static_cast<QResizeEvent *>(event);
        QListView::resizeEvent(e);
        updateGeometries();
    }

        break;
    default:
        break;

    }


    return QListView::event(event);

}





} //namespace HEHUI
