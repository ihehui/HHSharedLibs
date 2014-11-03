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




#include <QtGui/QDropEvent>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QApplication>
#include <QtWidgets/QToolBar>
#include <QtGui/QIcon>


#include "itemboxwidget.h"
#include "itemboxtreewidget.h"


#include "iconloader_p.h"
#include "filterwidget_p.h"


namespace HEHUI {



ItemBoxWidget::ItemBoxWidget(ItemBoxEventHandler *core, QWidget *parent, Qt::WindowFlags flags)
    : ItemBoxWidgetBase(parent, flags),
      m_core(core),
      m_treeWidget(new ItemBoxTreeWidget(m_core))
{

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    // Prevent the filter from grabbing focus since Our view has Qt::NoFocus
    /*FilterWidget **/filterWidget = new FilterWidget(0, FilterWidget::LayoutAlignNone);
    filterWidget->setRefuseFocus(true);
    connect(filterWidget, SIGNAL(filterChanged(QString)), m_treeWidget, SLOT(filter(QString)));

    l->addWidget(filterWidget);
    filterWidget->hide();

//    QToolBar *toolBar = new QToolBar(this);
//    toolBar->addWidget(filterWidget);
//    l->addWidget(toolBar);
//    toolBar->hide();

    // View
//    connect(m_treeWidget, SIGNAL(pressed(QString,QString,QPoint)), this, SLOT(handleMousePress(QString,QString,QPoint)));
    l->addWidget(m_treeWidget);

    setAcceptDrops (true);

    installEventFilter(this);

    m_treeWidget->setFocusPolicy(Qt::StrongFocus);
    m_treeWidget->setFocus();

}

ItemBoxWidget::~ItemBoxWidget()
{

}

bool ItemBoxWidget::eventFilter(QObject *o, QEvent *e)
{

    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if ((ke->modifiers() & Qt::ControlModifier) && ke->key() == Qt::Key_F) {
            setFilterWidgetVisible(!filterWidget->isVisible());
            //autoHideTimer->start();
            //keyPressEvent(ke);
            return true;
        }else if(ke->key() == Qt::Key_Escape){
            setFilterWidgetVisible(false);
            return true;
        }

    }

    return QWidget::eventFilter(o, e);
}

ItemBoxEventHandler *ItemBoxWidget::core() const
{
    return m_core;
}

//void ItemBoxWidget::handleMousePress(const QString &name, const QString &xml, const QPoint &global_mouse_pos)
//{
//    if (QApplication::mouseButtons() != Qt::LeftButton)
//        return;

//    //m_core->formWindowManager()->dragItems(item_list);
//}

int ItemBoxWidget::categoryCount() const
{
    return m_treeWidget->categoryCount();
}

ItemBoxWidgetInterface::Category ItemBoxWidget::category(int cat_idx) const
{
    return m_treeWidget->category(cat_idx);
}

void ItemBoxWidget::addCategory(const Category &cat, int index)
{
    m_treeWidget->addCategory(cat, index);
}

void ItemBoxWidget::removeCategory(int cat_idx)
{
    m_treeWidget->removeCategory(cat_idx);
}

void ItemBoxWidget::removeCategory(const QString &cat_id){
     m_treeWidget->removeCategory(cat_id);
}

void ItemBoxWidget::clearAllCategories(){
    m_treeWidget->clear();
}

void ItemBoxWidget::setCategoryHidden(int cat_idx, bool hide){
    m_treeWidget->setCategoryHidden(cat_idx, hide);
}

void ItemBoxWidget::setCategoryHidden(const QString &cat_id, bool hide){
    m_treeWidget->setCategoryHidden(cat_id, hide);
}

void ItemBoxWidget::setCategoryExpanded(int cat_idx, bool expand){
    m_treeWidget->setCategoryExpanded(cat_idx, expand);
}

void ItemBoxWidget::setCategoryExpanded(const QString &cat_id, bool expand){
    m_treeWidget->setCategoryExpanded(cat_id, expand);
}

void ItemBoxWidget::updateCategoryName(const QString &cat_id, const QString &new_cat_name){
    m_treeWidget->updateCategoryName(cat_id, new_cat_name);
}


int ItemBoxWidget::itemCount(int cat_idx) const
{
    return m_treeWidget->itemCount(cat_idx);
}

ItemBoxWidgetInterface::Item ItemBoxWidget::item(int cat_idx, int wgt_idx) const
{
    return m_treeWidget->item(cat_idx, wgt_idx);
}

ItemBoxWidgetInterface::Item ItemBoxWidget::item(const QString &cat_id, const QString &item_id) const{
    return m_treeWidget->item(cat_id, item_id);
}

void ItemBoxWidget::addItem(int cat_idx, const HEHUI::ItemBoxWidgetInterface::Item &itm)
{
    m_treeWidget->addItem(cat_idx, itm);
}

void ItemBoxWidget::addItem(const QString &cat_id, const Item &item){
    m_treeWidget->addItem(cat_id, item);
}

void ItemBoxWidget::removeItem(int cat_idx, int wgt_idx)
{
    m_treeWidget->removeItem(cat_idx, wgt_idx);
}

void ItemBoxWidget::removeItem(const QString &cat_id, const QString &item_id){
    m_treeWidget->removeItem(cat_id, item_id);
}

void ItemBoxWidget::moveItem(int old_cat_idx, int new_cat_idx, const QString &item_id){
    m_treeWidget->moveItem(old_cat_idx, new_cat_idx, item_id);
}

void ItemBoxWidget::moveItem(const QString &old_cat_id, const QString &new_cat_id, const QString &item_id){
    m_treeWidget->moveItem(old_cat_id, new_cat_id, item_id);
}

//void ItemBoxWidget::dropItems(const QList<ItemBoxWidgetInterface*> &item_list, const QPoint&)
//{
//    m_treeWidget->dropItems(item_list);
//}

//void ItemBoxWidget::setFileName(const QString &file_name)
//{
//    m_treeWidget->setFileName(file_name);
//}

//QString ItemBoxWidget::fileName() const
//{
//    return m_treeWidget->fileName();
//}

//bool ItemBoxWidget::load()
//{
//    return m_view->load(loadMode());
//}

//bool ItemBoxWidget::loadContents(const QString &contents)
//{
//    return m_view->loadContents(contents);
//}

//bool ItemBoxWidget::save()
//{
//    return m_view->save();
//}

//static const void *checkDragEvent(QDropEvent * event,
//                                               bool acceptEventsFromWidgetBox)
//{


//}

void ItemBoxWidget::dragEnterEvent (QDragEnterEvent * event)
{
    // We accept event originating from the widget box also here,
    // because otherwise Windows will not show the DnD pixmap.
    //checkDragEvent(event, true);
}

void ItemBoxWidget::dragMoveEvent(QDragMoveEvent * event)
{
    //checkDragEvent(event, true);
}

void ItemBoxWidget::dropEvent(QDropEvent * event)
{


}


bool ItemBoxWidget::updateObjectItemName(const QString &cat_id, const QString &item_id, const QString &itemName){
    return m_treeWidget->updateItemName(cat_id, item_id, itemName);
}

bool ItemBoxWidget::updateObjectItemIcon(const QString &cat_id, const QString &item_id, const QIcon &icon){
    return m_treeWidget->updateItemIcon(cat_id, item_id, icon);
}

//bool ItemBoxWidget::updateObjectItemIcon(const QString &cat_id, const QString &item_id, const QString &iconName){
//    return m_treeWidget->updateItemIcon(cat_id, item_id, iconName);
//}

void ItemBoxWidget::setFilterWidgetVisible(bool visible){
    filterWidget->setVisible(visible);
    if(visible){
        filterWidget->setFocusToEditor();
    }else{
        filterWidget->reset();
        filterWidget->clearFocus();
    }

}

void ItemBoxWidget::clearFilterWidget(){
    filterWidget->reset();
}

void ItemBoxWidget::expandAllCategories(){
    m_treeWidget->expandAll();
}

void ItemBoxWidget::collapseAllCategories(){
    m_treeWidget->collapseAll();
}





} //namespace HEHUI
