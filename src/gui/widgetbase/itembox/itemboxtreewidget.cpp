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

#include "itemboxtreewidget.h"
#include "itemboxcategorylistview.h"

// shared
#include "iconloader_p.h"
#include "sheet_delegate_p.h"


#include <QtWidgets/QHeaderView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidgetItem>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QMenu>

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include <QHash>

namespace HEHUI
{


enum TopLevelRole  { NORMAL_ITEM, SCRATCHPAD_ITEM, CUSTOM_ITEM };
QHash<const QTreeWidgetItem *, TopLevelRole>  topLevelRoleHash;


static void setTopLevelRole(TopLevelRole tlr, QTreeWidgetItem *item)
{
//    item->setData(0, Qt::UserRole, QVariant(tlr));
    topLevelRoleHash.insert(item, tlr);
}

static TopLevelRole topLevelRole(const  QTreeWidgetItem *item)
{
//    return static_cast<TopLevelRole>(item->data(0, Qt::UserRole).toInt());
    return topLevelRoleHash.value(item);
}



ItemBoxTreeWidget::ItemBoxTreeWidget(ItemBoxEventHandler *core, QWidget *parent) :
    QTreeWidget(parent),
    m_core(core),
    m_iconMode(false),
    m_scratchPadDeleteTimer(0)
{
    setFocusPolicy(Qt::NoFocus);
    setIndentation(0);
    setRootIsDecorated(false);
    setColumnCount(1);
    header()->hide();
    header()->setSectionResizeMode(QHeaderView::Stretch);
    setTextElideMode(Qt::ElideMiddle);
    setVerticalScrollMode(ScrollPerPixel);

    setItemDelegate(new SheetDelegate(this, this));

    connect(this, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(handleMousePress(QTreeWidgetItem *)));
}

//void ItemBoxTreeWidget::setLoadMode(ItemBoxWidgetBase::LoadMode loadMode){
//    m_loadMode = loadMode;
//}

bool ItemBoxTreeWidget::updateItemName(const QString &cat_id, const QString &item_id, const QString &itemName)
{
    ItemBoxCategoryListView *cat = categoryView(cat_id);
    if(!cat) {
        return false;
    }
    return cat->updateItemName(item_id, itemName);
}

bool ItemBoxTreeWidget::updateItemIcon(const QString &cat_id, const QString &item_id, const QIcon &icon)
{
    ItemBoxCategoryListView *cat = categoryView(cat_id);
    return cat->updateItemIcon(item_id, icon);
}

//bool ItemBoxTreeWidget::updateItemIcon(const QString &cat_id, const QString &item_id, const QString &iconName){
//    ItemBoxCategoryListView *cat = categoryView(cat_id);
//    return cat->updateItemIcon(item_id, iconName);
//}

ItemBoxCategoryListView *ItemBoxTreeWidget::categoryViewAt(int idx) const
{
    ItemBoxCategoryListView *rc = 0;
    if (QTreeWidgetItem *cat_item = topLevelItem(idx))
        if (QTreeWidgetItem *embedItem = cat_item->child(0)) {
            rc = qobject_cast<ItemBoxCategoryListView *>(itemWidget(embedItem, 0));
        }
    Q_ASSERT(rc);
    return rc;
}

ItemBoxCategoryListView *ItemBoxTreeWidget::categoryView(const QString &cat_id) const
{

    ItemBoxCategoryListView *rc = 0;
    if(cat_id.isEmpty()) {
        qCritical() << "Empty cat_name!";
        return rc;
    }

    int topItemCount = topLevelItemCount();
    for (int i = 0;  i < topItemCount; i++) {
        QTreeWidgetItem *cat_item = topLevelItem(i);
        //if(cat_item && cat_item->text(0) == cat_id){
        if(cat_item && cat_item->data(0, Qt::UserRole).toString() == cat_id) {
            if(QTreeWidgetItem *embedItem = cat_item->child(0)) {
                rc = qobject_cast<ItemBoxCategoryListView *>(itemWidget(embedItem, 0));
                //qDebug()<<"----ItemBoxTreeWidget::categoryView(...)~~cat_item->text(0):"<<cat_item->text(0);
                break;
            }

        }
    }

    Q_ASSERT(rc);

    return rc;

}

//void ItemBoxTreeWidget::saveExpandedState() const
//{
//    QStringList closedCategories;
//    if (const int numCategories = categoryCount()) {
//        for (int i = 0; i < numCategories; ++i) {
//            const QTreeWidgetItem *cat_item = topLevelItem(i);
//            if (!isItemExpanded(cat_item))
//                closedCategories.append(cat_item->text(0));
//        }
//    }
//    QDesignerSettingsInterface *settings = m_core->settingsManager();
//    settings->beginGroup(QLatin1String(widgetBoxRootElementC));
//    settings->setValue(QStringLiteral("Closed categories"), closedCategories);
//    settings->setValue(QStringLiteral("View mode"), m_iconMode);
//    settings->endGroup();
//}

//void  ItemBoxTreeWidget::restoreExpandedState()
//{
//    typedef QSet<QString> StringSet;
//    QDesignerSettingsInterface *settings = m_core->settingsManager();
//    m_iconMode = settings->value(QStringLiteral("WidgetBox/View mode")).toBool();
//    updateViewMode();
//    const StringSet closedCategories = settings->value(QStringLiteral("WidgetBox/Closed categories"), QStringList()).toStringList().toSet();
//    expandAll();
//    if (closedCategories.empty())
//        return;

//    if (const int numCategories = categoryCount()) {
//        for (int i = 0; i < numCategories; ++i) {
//            QTreeWidgetItem *item = topLevelItem(i);
//            if (closedCategories.contains(item->text(0)))
//                item->setExpanded(false);
//            }
//    }
//}

ItemBoxTreeWidget::~ItemBoxTreeWidget()
{
    //saveExpandedState();
}

//void ItemBoxTreeWidget::setFileName(const QString &file_name)
//{
//    m_file_name = file_name;
//}

//QString ItemBoxTreeWidget::fileName() const
//{
//    return m_file_name;
//}

//bool ItemBoxTreeWidget::save()
//{
//    if (fileName().isEmpty())
//        return false;

//    QFile file(fileName());
//    if (!file.open(QIODevice::WriteOnly))
//        return false;

//    CategoryList cat_list;
//    const int count = categoryCount();
//    for (int i = 0; i < count; ++i)
//        cat_list.append(category(i));

//    QXmlStreamWriter writer(&file);
//    writer.setAutoFormatting(true);
//    writer.setAutoFormattingIndent(1);
//    writer.writeStartDocument();
//    writeCategories(writer, cat_list);
//    writer.writeEndDocument();

//    return true;
//}

//void ItemBoxTreeWidget::slotSave()
//{
//    //save();
//}

void ItemBoxTreeWidget::handleMousePress(QTreeWidgetItem *item)
{
    if (item == 0) {
        return;
    }

    if (QApplication::mouseButtons() != Qt::LeftButton) {
        return;
    }

    if (item->parent() == 0) {
        setItemExpanded(item, !isItemExpanded(item));
        return;
    }
}

int ItemBoxTreeWidget::ensureScratchpad()
{
    const int existingIndex = indexOfScratchpad();
    if (existingIndex != -1) {
        return existingIndex;
    }

    QTreeWidgetItem *scratch_item = new QTreeWidgetItem(this);
    scratch_item->setText(0, tr("Scratchpad"));
    setTopLevelRole(SCRATCHPAD_ITEM, scratch_item);
    addCategoryView(scratch_item, false); // Scratchpad in list mode.
    return categoryCount() - 1;
}

ItemBoxCategoryListView *ItemBoxTreeWidget::addCategoryView(QTreeWidgetItem *parent, bool iconMode)
{
    QTreeWidgetItem *embed_item = new QTreeWidgetItem(parent);
    embed_item->setFlags(Qt::ItemIsEnabled);
    ItemBoxCategoryListView *categoryView = new ItemBoxCategoryListView(m_core, this);
    categoryView->setViewMode(iconMode ? QListView::IconMode : QListView::ListMode);
    categoryView->setFlow(iconMode ? QListView::LeftToRight : QListView::TopToBottom);

    //categoryView->setFixedWidth(header()->width());
    //const int height = qMax(categoryView->contentsSize().height(), 1);
    //categoryView->setFixedHeight(1);

//    connect(categoryView, SIGNAL(scratchPadChanged()), this, SLOT(slotSave()));
//    connect(categoryView, SIGNAL(pressed(QString,QString,QPoint)), this, SIGNAL(pressed(QString,QString,QPoint)));
    connect(categoryView, SIGNAL(itemRemoved()), this, SLOT(slotScratchPadItemDeleted()));
//    connect(categoryView, SIGNAL(lastItemRemoved()), this, SLOT(slotLastScratchPadItemDeleted()));

    setItemWidget(embed_item, 0, categoryView);
    return categoryView;

}

int ItemBoxTreeWidget::indexOfScratchpad() const
{
    if (const int numTopLevels =  topLevelItemCount()) {
        for (int i = numTopLevels - 1; i >= 0; --i) {
            if (topLevelRole(topLevelItem(i)) == SCRATCHPAD_ITEM) {
                return i;
            }
        }
    }
    return -1;
}

int ItemBoxTreeWidget::indexOfCategory(const QString &cat_id) const
{
    const int topLevelCount = topLevelItemCount();
    for (int i = 0; i < topLevelCount; ++i) {
        //if (topLevelItem(i)->text(0) == name)
        if (topLevelItem(i)->data(0, Qt::UserRole).toString() == cat_id) {
            return i;
        }
    }
    return -1;
}

//bool ItemBoxTreeWidget::load(ItemBoxWidgetBase::LoadMode loadMode)
//{
//    switch (loadMode) {
//    case ItemBoxWidgetBase::LoadReplace:
//        clear();
//        break;
//    case ItemBoxWidgetBase::LoadCustomWidgetsOnly:
//        addCustomCategories(true);
//        updateGeometries();
//        return true;
//    default:
//        break;
//    }

//    const QString name = fileName();

//    QFile f(name);
//    if (!f.open(QIODevice::ReadOnly)) // Might not exist at first startup
//        return false;

//    const QString contents = QString::fromUtf8(f.readAll());
//    return loadContents(contents);
//}

//bool ItemBoxTreeWidget::loadContents(const QString &contents)
//{
//    QString errorMessage;
//    CategoryList cat_list;
//    if (!readCategories(m_file_name, contents, &cat_list, &errorMessage)) {
//        qdesigner_internal::designerWarning(errorMessage);
//        return false;
//    }

//    foreach(const Category &cat, cat_list)
//        addCategory(cat);

//    addCustomCategories(false);
//    // Restore which items are expanded
//    restoreExpandedState();
//    return true;
//}

//void ItemBoxTreeWidget::addCustomCategories(bool replace)
//{
//    if (replace) {
//        // clear out all existing custom widgets
//        if (const int numTopLevels =  topLevelItemCount()) {
//            for (int t = 0; t < numTopLevels ; ++t)
//                categoryViewAt(t)->removeCustomItems();
//        }
//    }
//    // re-add
//    const CategoryList customList = loadCustomCategoryList();
//    const CategoryList::const_iterator cend = customList.constEnd();
//    for (CategoryList::const_iterator it = customList.constBegin(); it != cend; ++it)
//        addCategory(*it);
//}

//static inline QString msgXmlError(const QString &fileName, const QXmlStreamReader &r)
//{
//    return ItemBoxWidgetBase::tr("An error has been encountered at line %1 of %2: %3")
//            .arg(r.lineNumber()).arg(fileName, r.errorString());
//}

//bool ItemBoxTreeWidget::readCategories(const QString &fileName, const QString &contents,
//                                       CategoryList *cats, QString *errorMessage)
//{
//    // Read widget box XML:
//    //
//    //<widgetbox version="4.5">
//    // <category name="Layouts">
//    //  <categoryentry name="Vertical Layout" icon="win/editvlayout.png" type="default">
//    //   <widget class="QListWidget" ...>
//    // ...

//    QXmlStreamReader reader(contents);


//    // Entries of category with name="invisible" should be ignored
//    bool ignoreEntries = false;

//    while (!reader.atEnd()) {
//        switch (reader.readNext()) {
//        case QXmlStreamReader::StartElement: {
//            const QStringRef tag = reader.name();
//            if (tag == QLatin1String(widgetBoxRootElementC)) {
//                //<widgetbox version="4.5">
//                continue;
//            }
//            if (tag == QLatin1String(categoryElementC)) {
//                // <category name="Layouts">
//                const QXmlStreamAttributes attributes = reader.attributes();
//                const QString categoryName = attributes.value(QLatin1String(nameAttributeC)).toString();
//                if (categoryName == QLatin1String(invisibleNameC)) {
//                    ignoreEntries = true;
//                } else {
//                    Category category(categoryName);
//                    if (attributes.value(QLatin1String(typeAttributeC)) == QLatin1String(scratchPadValueC))
//                        category.setType(Category::Scratchpad);
//                    cats->push_back(category);
//                }
//                continue;
//            }
//            if (tag == QLatin1String(categoryEntryElementC)) {
//                //  <categoryentry name="Vertical Layout" icon="win/editvlayout.png" type="default">
//                if (!ignoreEntries) {
//                    QXmlStreamAttributes attr = reader.attributes();
//                    const QString widgetName = attr.value(QLatin1String(nameAttributeC)).toString();
//                    const QString widgetIcon = attr.value(QLatin1String(iconAttributeC)).toString();
//                    const ItemBoxTreeWidget::Item::Type widgetType =
//                        attr.value(QLatin1String(typeAttributeC)).toString()
//                            == QLatin1String(customValueC) ?
//                        ItemBoxTreeWidget::Item::Custom :
//                        ItemBoxTreeWidget::Item::Default;

//                    Item w;
//                    w.setName(widgetName);
//                    w.setIconName(widgetIcon);
//                    w.setType(widgetType);
//                    if (!readWidget(&w, contents, reader))
//                        continue;

//                    cats->back().addWidget(w);
//                } // ignoreEntries
//                continue;
//            }
//            break;
//        }
//        case QXmlStreamReader::EndElement: {
//           const QStringRef tag = reader.name();
//           if (tag == QLatin1String(widgetBoxRootElementC)) {
//               continue;
//           }
//           if (tag == QLatin1String(categoryElementC)) {
//               ignoreEntries = false;
//               continue;
//           }
//           if (tag == QLatin1String(categoryEntryElementC)) {
//               continue;
//           }
//           break;
//        }
//        default: break;
//        }
//    }

//    if (reader.hasError()) {
//        *errorMessage = msgXmlError(fileName, reader);
//        return false;
//    }

//    return true;
//}



/*!
 * Read out a widget within a category. This can either be
 * enclosed in a <ui> element or a (legacy) <widget> element which may
 * contain nested <widget> elements.
 *
 * Examples:
 *
 * <ui language="c++">
 *  <widget class="MultiPageWidget" name="multipagewidget"> ... </widget>
 *  <customwidgets>...</customwidgets>
 * <ui>
 *
 * or
 *
 * <widget>
 *   <widget> ... </widget>
 *   ...
 * <widget>
 *
 * Returns true on success, false if end was reached or an error has been encountered
 * in which case the reader has its error flag set. If successful, the current item
 * of the reader will be the closing element (</ui> or </widget>)
 */
//bool ItemBoxTreeWidget::readWidget(Item *w, const QString &xml, QXmlStreamReader &r)
//{
//    qint64 startTagPosition =0, endTagPosition = 0;

//    int nesting = 0;
//    bool endEncountered = false;
//    bool parsedWidgetTag = false;
//    QString outmostElement;
//    while (!endEncountered) {
//        const qint64 currentPosition = r.characterOffset();
//        switch(r.readNext()) {
//        case QXmlStreamReader::StartElement:
//            if (nesting++ == 0) {
//                // First element must be <ui> or (legacy) <widget>
//                const QStringRef name = r.name();
//                if (name == QLatin1String(uiElementC)) {
//                    startTagPosition = currentPosition;
//                } else {
//                    if (name == QLatin1String(widgetElementC)) {
//                        startTagPosition = currentPosition;
//                        parsedWidgetTag = true;
//                    } else {
//                        r.raiseError(ItemBoxWidgetBase::tr("Unexpected element <%1> encountered when parsing for <widget> or <ui>").arg(name.toString()));
//                        return false;
//                    }
//                }
//            } else {
//                // We are within <ui> looking for the first <widget> tag
//                if (!parsedWidgetTag && r.name() == QLatin1String(widgetElementC)) {
//                    parsedWidgetTag = true;
//                }
//            }
//            break;
//        case QXmlStreamReader::EndElement:
//            // Reached end of widget?
//            if (--nesting == 0)  {
//                endTagPosition = r.characterOffset();
//                endEncountered = true;
//            }
//            break;
//        case QXmlStreamReader::EndDocument:
//            r.raiseError(ItemBoxWidgetBase::tr("Unexpected end of file encountered when parsing widgets."));
//            return false;
//        case QXmlStreamReader::Invalid:
//            return false;
//        default:
//            break;
//        }
//    }
//    if (!parsedWidgetTag) {
//        r.raiseError(ItemBoxWidgetBase::tr("A widget element could not be found."));
//        return false;
//    }
//    // Oddity: Startposition is 1 off
//    QString widgetXml = xml.mid(startTagPosition, endTagPosition - startTagPosition);
//    const QChar lessThan = QLatin1Char('<');
//    if (!widgetXml.startsWith(lessThan))
//        widgetXml.prepend(lessThan);
//    w->setDomXml(widgetXml);
//    return true;
//}

//void ItemBoxTreeWidget::writeCategories(QXmlStreamWriter &writer, const CategoryList &cat_list) const
//{
//    const QString widgetbox = QLatin1String(widgetBoxRootElementC);
//    const QString name = QLatin1String(nameAttributeC);
//    const QString type = QLatin1String(typeAttributeC);
//    const QString icon = QLatin1String(iconAttributeC);
//    const QString defaultType = QLatin1String(defaultTypeValueC);
//    const QString category = QLatin1String(categoryElementC);
//    const QString categoryEntry = QLatin1String(categoryEntryElementC);
//    const QString iconPrefix = QLatin1String(iconPrefixC);
//    const QString widgetTag = QLatin1String(widgetElementC);

//    //
//    // <widgetbox>
//    //   <category name="Layouts">
//    //     <categoryEntry name="Vertical Layout" type="default" icon="win/editvlayout.png">
//    //       <ui>
//    //        ...
//    //       </ui>
//    //     </categoryEntry>
//    //     ...
//    //   </category>
//    //   ...
//    // </widgetbox>
//    //

//    writer.writeStartElement(widgetbox);

//    foreach (const Category &cat, cat_list) {
//        writer.writeStartElement(category);
//        writer.writeAttribute(name, cat.name());
//        if (cat.type() == Category::Scratchpad)
//            writer.writeAttribute(type, QLatin1String(scratchPadValueC));

//        const int widgetCount = cat.itemCount();
//        for (int i = 0; i < widgetCount; ++i) {
//           const  Item wgt = cat.item(i);
//            if (wgt.type() == Item::Custom)
//                continue;

//            writer.writeStartElement(categoryEntry);
//            writer.writeAttribute(name, wgt.name());
//            if (!wgt.iconName().startsWith(iconPrefix))
//                writer.writeAttribute(icon, wgt.iconName());
//            writer.writeAttribute(type, defaultType);

//            const DomUI *domUI = ItemBoxWidgetBase::xmlToUi(wgt.name(), ItemBoxCategoryListView::widgetDomXml(wgt), false);
//            if (domUI) {
//                domUI->write(writer);
//                delete domUI;
//            }

//            writer.writeEndElement(); // categoryEntry
//        }
//        writer.writeEndElement(); // categoryEntry
//    }

//    writer.writeEndElement(); // widgetBox
//}

static int findCategory(const QString &name, const ItemBoxTreeWidget::CategoryList &list)
{
    int idx = 0;
    foreach (const ItemBoxTreeWidget::Category &cat, list) {
        if (cat.name() == name) {
            return idx;
        }
        ++idx;
    }
    return -1;
}

static inline bool isValidIcon(const QIcon &icon)
{
    if (!icon.isNull()) {
        const QList<QSize> availableSizes = icon.availableSizes();
        if (!availableSizes.empty()) {
            return !availableSizes.front().isEmpty();
        }
    }
    return false;
}

//ItemBoxTreeWidget::CategoryList ItemBoxTreeWidget::loadCustomCategoryList() const
//{
//    CategoryList result;

//    const QDesignerPluginManager *pm = m_core->pluginManager();
//    const QDesignerPluginManager::CustomWidgetList customWidgets = pm->registeredCustomWidgets();
//    if (customWidgets.empty())
//        return result;

//    static const QString customCatName = tr("Custom Widgets");

//    const QString invisible = QLatin1String(invisibleNameC);
//    const QString iconPrefix = QLatin1String(iconPrefixC);

//    foreach(QDesignerCustomWidgetInterface *c, customWidgets) {
//        const QString dom_xml = c->domXml();
//        if (dom_xml.isEmpty())
//            continue;

//        const QString pluginName = c->name();
//        const QDesignerCustomWidgetData data = pm->customWidgetData(c);
//        QString displayName = data.xmlDisplayName();
//        if (displayName.isEmpty())
//            displayName = pluginName;

//        QString cat_name = c->group();
//        if (cat_name.isEmpty())
//            cat_name = customCatName;
//        else if (cat_name == invisible)
//            continue;

//        int idx = findCategory(cat_name, result);
//        if (idx == -1) {
//            result.append(Category(cat_name));
//            idx = result.size() - 1;
//        }
//        Category &cat = result[idx];

//        const QIcon icon = c->icon();

//        QString icon_name;
//        if (isValidIcon(icon)) {
//            icon_name = iconPrefix;
//            icon_name += pluginName;
//            m_pluginIcons.insert(icon_name, icon);
//        } else {
//            icon_name = QLatin1String(qtLogoC);
//        }

//        cat.addWidget(Item(displayName, dom_xml, icon_name, Item::Custom));
//    }

//    return result;
//}

void ItemBoxTreeWidget::adjustSubListSize(QTreeWidgetItem *cat_item)
{

    QTreeWidgetItem *embedItem = cat_item->child(0);
    if (embedItem == 0) {
        return;
    }

    ItemBoxCategoryListView *list_widget = static_cast<ItemBoxCategoryListView *>(itemWidget(embedItem, 0));
    list_widget->setFixedWidth(header()->width());
    list_widget->doItemsLayout();
    const int height = qMax(list_widget->contentsSize().height(), 1);
    list_widget->setFixedHeight(height);
    //list_widget->setFlow(QListView::TopToBottom);
    list_widget->setFlow((list_widget->viewMode() == QListView::ListMode) ? QListView::TopToBottom : QListView::LeftToRight);


    embedItem->setSizeHint(0, QSize(-1, height - 1));
    viewport()->update();

}

int ItemBoxTreeWidget::categoryCount() const
{
    return topLevelItemCount();
}

ItemBoxTreeWidget::Category ItemBoxTreeWidget::category(int cat_idx) const
{
    if (cat_idx >= topLevelItemCount()) {
        return Category();
    }

    QTreeWidgetItem *cat_item = topLevelItem(cat_idx);

    QTreeWidgetItem *embedItem = cat_item->child(0);
    ItemBoxCategoryListView *categoryView = static_cast<ItemBoxCategoryListView *>(itemWidget(embedItem, 0));

    Category result = categoryView->category();
    result.setName(cat_item->text(0));

    switch (topLevelRole(cat_item)) {
    case SCRATCHPAD_ITEM:
        result.setType(Category::Scratchpad);
        break;
    default:
        result.setType(Category::Default);
        break;
    }
    return result;
}

ItemBoxTreeWidget::Category ItemBoxTreeWidget::category(const QString &cat_id) const
{
    return category(indexOfCategory(cat_id));
}

int ItemBoxTreeWidget::findCategory(const QString &cat_id, const ItemBoxTreeWidget::CategoryList &list)
{
    int idx = 0;
    foreach (const ItemBoxTreeWidget::Category &cat, list) {
        if (cat.id() == cat_id) {
            return idx;
        }
        ++idx;
    }
    return -1;
}

void ItemBoxTreeWidget::addCategory(const Category &cat, int index)
{
//    if (cat.itemCount() == 0)
//        return;

    const bool isScratchPad = cat.type() == Category::Scratchpad;
    ItemBoxCategoryListView *categoryView = 0;
    QTreeWidgetItem *cat_item = 0;

    if (isScratchPad) {
        const int idx = ensureScratchpad();
        categoryView = categoryViewAt(idx);
        cat_item = topLevelItem(idx);
    } else {
        const int existingIndex = indexOfCategory(cat.id());
        if (existingIndex == -1) {
            cat_item = new QTreeWidgetItem();
            cat_item->setData(0, Qt::UserRole, QVariant(cat.id()));
            cat_item->setText(0, cat.name());
            setTopLevelRole(NORMAL_ITEM, cat_item);
            // insert before scratchpad
            const int scratchPadIndex = indexOfScratchpad();
            if (scratchPadIndex == -1) {
                if(index == -1) {
                    addTopLevelItem(cat_item);
                } else {
                    insertTopLevelItem(index, cat_item);
                }

            } else {
                insertTopLevelItem(scratchPadIndex, cat_item);
            }
            setItemExpanded(cat_item, true);
            categoryView = addCategoryView(cat_item, m_iconMode);
        } else {
            categoryView = categoryViewAt(existingIndex);
            cat_item = topLevelItem(existingIndex);
        }
    }
    // The same categories are read from the file $HOME, avoid duplicates
    const int widgetCount = cat.itemCount();
    for (int i = 0; i < widgetCount; ++i) {
        const Item w = cat.item(i);
        if (!categoryView->containsItem(w.name())) {
            categoryView->addItem(w, w.icon(), isScratchPad);
        }
    }
    adjustSubListSize(cat_item);

}

void ItemBoxTreeWidget::removeCategory(int cat_idx)
{
    if (cat_idx >= topLevelItemCount()) {
        return;
    }
    delete takeTopLevelItem(cat_idx);
}

void ItemBoxTreeWidget::removeCategory(const QString &cat_id)
{

    int cat_idx = indexOfCategory(cat_id);
    if (cat_idx >= topLevelItemCount() || cat_idx < 0) {
        return;
    }

    removeCategory(cat_idx);

}

void ItemBoxTreeWidget::setCategoryHidden(int cat_idx, bool hide)
{
    setRowHidden(cat_idx, QModelIndex(), hide);
}

void ItemBoxTreeWidget::setCategoryHidden(const QString &cat_id, bool hide)
{
    int cat_idx = indexOfCategory(cat_id);
    if (cat_idx >= topLevelItemCount() || cat_idx < 0) {
        return;
    }

    setCategoryHidden(cat_idx, hide);

}

void ItemBoxTreeWidget::setCategoryExpanded(int cat_idx, bool expand)
{

    QTreeWidgetItem *cat_item = topLevelItem(cat_idx);
    if(expand) {
        expandItem(cat_item);
    } else {
        collapseItem(cat_item);
    }

}

void ItemBoxTreeWidget::setCategoryExpanded(const QString &cat_id, bool expand)
{

    int cat_idx = indexOfCategory(cat_id);
    if (cat_idx >= topLevelItemCount() || cat_idx < 0) {
        return;
    }

    setCategoryExpanded(cat_idx, expand);

}

void ItemBoxTreeWidget::updateCategoryName(const QString &cat_id, const QString &new_cat_name)
{

    int cat_idx = indexOfCategory(cat_id);
    if (cat_idx >= topLevelItemCount() || cat_idx < 0) {
        return;
    }

    QTreeWidgetItem *cat_item = topLevelItem(cat_idx);
    cat_item->setText(0, new_cat_name);

    //    QTreeWidgetItem *embedItem = cat_item->child(0);
    //    embedItem->setText(0, new_cat_name);

}

int ItemBoxTreeWidget::itemCount(int cat_idx) const
{
    if (cat_idx >= topLevelItemCount()) {
        return 0;
    }
    // SDK functions want unfiltered access
    return categoryViewAt(cat_idx)->count(ItemBoxCategoryListView::UnfilteredAccess);
}

ItemBoxTreeWidget::Item ItemBoxTreeWidget::item(int cat_idx, int item_idx) const
{
    if (cat_idx >= topLevelItemCount()) {
        return Item();
    }
    // SDK functions want unfiltered access
    ItemBoxCategoryListView *categoryView = categoryViewAt(cat_idx);
    return categoryView->itemAt(ItemBoxCategoryListView::UnfilteredAccess, item_idx);
}

ItemBoxTreeWidget::Item ItemBoxTreeWidget::item(const QString &cat_id, const QString &item_id) const
{
    if (cat_id.trimmed().isEmpty()) {
        return Item();
    }
    // SDK functions want unfiltered access
    ItemBoxCategoryListView *cv = categoryView(cat_id);
    return cv->item(item_id);
}

void ItemBoxTreeWidget::addItem(int cat_idx, const Item &item)
{
    qDebug() << "--ItemBoxTreeWidget::addItem(...) cat_idx:" << cat_idx << " item.id():" << item.id();

    if (cat_idx >= topLevelItemCount() || cat_idx < 0) {
        qCritical() << "ERROR! Invalid category index!";
        return;
    }

    QTreeWidgetItem *cat_item = topLevelItem(cat_idx);
    ItemBoxCategoryListView *categoryView = categoryViewAt(cat_idx);

    const bool scratch = topLevelRole(cat_item) == SCRATCHPAD_ITEM;
    //const bool scratch = topLevelRole(cat_item) == NORMAL_ITEM;

    // The same categories are read from the file $HOME, avoid duplicates
    if (!categoryView->containsItem(item.id())) {
        categoryView->addItem(item, item.icon(), scratch);
        categoryView->updateGeometry();
    } else {
        qWarning() << "ERROR! Item " << item.id() << " already exists!";
    }

    adjustSubListSize(cat_item);

    updateGeometries();

}

void ItemBoxTreeWidget::addItem(const QString &cat_id, const Item &item)
{
    qDebug() << "--ItemBoxTreeWidget::addItem(...) cat_id:" << cat_id << " item.id():" << item.id();

    if(cat_id.trimmed().isEmpty()) {
        qDebug() << "ERROR! Empty cat_id!";
        return;
    }

    addItem(indexOfCategory(cat_id), item);

}

void ItemBoxTreeWidget::removeItem(int cat_idx, int wgt_idx)
{
    if (cat_idx >= topLevelItemCount()) {
        return;
    }

    ItemBoxCategoryListView *categoryView = categoryViewAt(cat_idx);

    // SDK functions want unfiltered access
    const ItemBoxCategoryListView::AccessMode am = ItemBoxCategoryListView::UnfilteredAccess;
    if (wgt_idx >= categoryView->count(am)) {
        return;
    }

    categoryView->removeRow(am, wgt_idx);
    categoryView->updateGeometry();

}

void ItemBoxTreeWidget::removeItem(const QString &cat_id, const QString &item_id)
{

    qDebug() << "--ItemBoxTreeWidget::removeObjectItem()";

    int cat_Idx = indexOfCategory(cat_id);
    if (cat_Idx >= topLevelItemCount() || cat_Idx < 0) {
        return ;
    }

    // unfiltered access
    const ItemBoxCategoryListView::AccessMode am = ItemBoxCategoryListView::UnfilteredAccess;

    ItemBoxCategoryListView *categoryview = categoryViewAt(cat_Idx);
    int item_idx = categoryview->indexOfItem(item_id);
    //TreeWidget::ObjectItem objItem = old_categoryView->objectItemAt(am, item_idx);
    //if(objItem.isNull()){return;}

    if (item_idx >= categoryview->count(am)) {
        return;
    }
    categoryview->removeRow(am, item_idx);
    categoryview->updateGeometry();
    adjustSubListSize(topLevelItem(cat_Idx));

    updateGeometries();
    //viewport()->update();

}

void ItemBoxTreeWidget::moveItem(int old_cat_idx, int new_cat_idx, const QString &item_id)
{

    qDebug() << "--ItemBoxTreeWidget::moveObjectItem()";

    if (old_cat_idx >= topLevelItemCount() || new_cat_idx >= topLevelItemCount()) {
        qCritical() << "ERROR! Invalid old_cat_idx or new_cat_idx!";
        return ;
    }

    // unfiltered access
    const ItemBoxCategoryListView::AccessMode am = ItemBoxCategoryListView::UnfilteredAccess;

    ItemBoxCategoryListView *old_categoryView = categoryViewAt(old_cat_idx);
    int item_idx = old_categoryView->indexOfItem(item_id);
    ItemBoxTreeWidget::Item objItem = old_categoryView->itemAt(am, item_idx);
    if(objItem.isNull()) {
        return;
    }

    //if (item_idx >= old_categoryView->count(am)){return;}
    old_categoryView->removeRow(am, item_idx);
    adjustSubListSize(topLevelItem(old_cat_idx));

    addItem(new_cat_idx, objItem);


    //    QTreeWidgetItem *cat_item = topLevelItem(new_cat_idx);
    //    CategoryListView *categoryView = categoryViewAt(new_cat_idx);

    //    const bool scratch = topLevelRole(cat_item) == SCRATCHPAD_ITEM;
    //    //const bool scratch = topLevelRole(cat_item) == NORMAL_ITEM;

    //    // The same categories are read from the file $HOME, avoid duplicates
    //    if (!categoryView->containsObjectItem(objItem.id())){
    //        categoryView->addObjectItem(objItem, iconForObjectItem(objItem.iconName(), objItem.iconMode()), scratch);
    //    }else{
    //        qWarning()<<"ERROR! Item "<<objItem.id()<<" already exists!";
    //    }

    //    adjustSubListSize(cat_item);

    updateGeometries();



}

void ItemBoxTreeWidget::moveItem(const QString &old_cat_id, const QString &new_cat_id, const QString &item_id)
{

    int old_cat_idx = indexOfCategory(old_cat_id);
    int new_cat_idx = indexOfCategory(new_cat_id);

    moveItem(old_cat_idx, new_cat_idx, item_id);

}


void ItemBoxTreeWidget::slotScratchPadItemDeleted()
{
    const int scratch_idx = indexOfScratchpad();
    QTreeWidgetItem *scratch_item = topLevelItem(scratch_idx);
    adjustSubListSize(scratch_item);
//    save();
}

void ItemBoxTreeWidget::slotLastScratchPadItemDeleted()
{
    // Remove the scratchpad in the next idle loop
    if (!m_scratchPadDeleteTimer) {
        m_scratchPadDeleteTimer = new QTimer(this);
        m_scratchPadDeleteTimer->setSingleShot(true);
        m_scratchPadDeleteTimer->setInterval(0);
        connect(m_scratchPadDeleteTimer, SIGNAL(timeout()), this, SLOT(deleteScratchpad()));
    }
    if (!m_scratchPadDeleteTimer->isActive()) {
        m_scratchPadDeleteTimer->start();
    }
}

void ItemBoxTreeWidget::deleteScratchpad()
{
    const int idx = indexOfScratchpad();
    if (idx == -1) {
        return;
    }
    delete takeTopLevelItem(idx);
    //save();
}


void ItemBoxTreeWidget::slotListMode()
{
    m_iconMode = false;
    updateViewMode();
}

void ItemBoxTreeWidget::slotIconMode()
{
    m_iconMode = true;
    updateViewMode();
}

void ItemBoxTreeWidget::updateViewMode()
{
    if (const int numTopLevels = topLevelItemCount()) {
        for (int i = numTopLevels - 1; i >= 0; --i) {
            QTreeWidgetItem *topLevel = topLevelItem(i);
            // Scratch pad stays in list mode.
            const QListView::ViewMode viewMode  = m_iconMode && (topLevelRole(topLevel) != SCRATCHPAD_ITEM) ? QListView::IconMode : QListView::ListMode;
            ItemBoxCategoryListView *categoryView = categoryViewAt(i);
            if (viewMode != categoryView->viewMode()) {
                categoryView->setViewMode(viewMode);
                adjustSubListSize(topLevelItem(i));
            }
        }
    }

    updateGeometries();
}

void ItemBoxTreeWidget::resizeEvent(QResizeEvent *e)
{
    QTreeWidget::resizeEvent(e);
    if (const int numTopLevels = topLevelItemCount()) {
        //        for (int i = numTopLevels - 1; i >= 0; --i)
        //            adjustSubListSize(topLevelItem(i));
        for (int i = 0; i < numTopLevels; i++) {
            adjustSubListSize(topLevelItem(i));
        }
    }

}

void ItemBoxTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QTreeWidgetItem *item = itemAt(e->pos());
    if(!item) {
        return;
    }

    const bool scratchpad_menu = item != 0
                                 && item->parent() != 0
                                 && topLevelRole(item->parent()) ==  SCRATCHPAD_ITEM;

    QMenu menu;
    menu.addAction(tr("Expand all"), this, SLOT(expandAll()));
    menu.addAction(tr("Collapse all"), this, SLOT(collapseAll()));
    menu.addSeparator();

    QAction *listModeAction = menu.addAction(tr("List View"));
    QAction *iconModeAction = menu.addAction(tr("Icon View"));
    listModeAction->setCheckable(true);
    iconModeAction->setCheckable(true);
    QActionGroup *viewModeGroup = new QActionGroup(&menu);
    viewModeGroup->addAction(listModeAction);
    viewModeGroup->addAction(iconModeAction);
    if (m_iconMode) {
        iconModeAction->setChecked(true);
    } else {
        listModeAction->setChecked(true);
    }
    connect(listModeAction, SIGNAL(triggered()), SLOT(slotListMode()));
    connect(iconModeAction, SIGNAL(triggered()), SLOT(slotIconMode()));

    if (scratchpad_menu) {
        menu.addSeparator();
        menu.addAction(tr("Remove"), itemWidget(item, 0), SLOT(removeCurrentItem()));
        if (!m_iconMode) {
            menu.addAction(tr("Edit name"), itemWidget(item, 0), SLOT(editCurrentItem()));
        }
    }
    e->accept();
//    menu.exec(mapToGlobal(e->pos()));
//    m_core->handleContextMenuEventOnCategory(item->text(0), mapToGlobal(e->pos()), &menu);
    m_core->handleContextMenuEventOnCategory(item->data(0, Qt::UserRole).toString(), mapToGlobal(e->pos()), &menu);

}

//void ItemBoxTreeWidget::dropItems(const QList<ItemBoxWidgetInterface *> &item_list)
//{
//    QTreeWidgetItem *scratch_item = 0;
//    ItemBoxCategoryListView *categoryView = 0;
//    bool added = false;

//    foreach (ItemBoxWidgetInterface *item, item_list) {
//        QWidget *w = item->widget();
//        if (w == 0)
//            continue;


//        const Item wgt = Item(w->objectName(), xml);
//        categoryView->addItem(wgt, iconForWidget(wgt.iconName()), true);
//        setItemExpanded(scratch_item, true);
//        added = true;
//    }

//    if (added) {
//        save();
//        QApplication::setActiveWindow(this);
//        // Is the new item visible in filtered mode?
//        const ItemBoxCategoryListView::AccessMode am = ItemBoxCategoryListView::FilteredAccess;
//        if (const int count = categoryView->count(am))
//            categoryView->setCurrentItem(am, count - 1);
//        categoryView->adjustSize(); // XXX
//        adjustSubListSize(scratch_item);
//    }
//}

void ItemBoxTreeWidget::filter(const QString &f)
{
    const bool empty = f.isEmpty();
    QRegExp re = empty ? QRegExp() : QRegExp(f, Qt::CaseInsensitive, QRegExp::FixedString);
    const int numTopLevels = topLevelItemCount();
    bool changed = false;
    for (int i = 0; i < numTopLevels; i++) {
        QTreeWidgetItem *tl = topLevelItem(i);
        ItemBoxCategoryListView *categoryView = categoryViewAt(i);
        // Anything changed? -> Enable the category
        const int oldCount = categoryView->count(ItemBoxCategoryListView::FilteredAccess);
        categoryView->filter(re);
        const int newCount = categoryView->count(ItemBoxCategoryListView::FilteredAccess);
        if (oldCount != newCount) {
            changed = true;
            const bool categoryEnabled = newCount > 0 || empty;
            if (categoryEnabled) {
                categoryView->adjustSize();
                adjustSubListSize(tl);
            }
            setRowHidden (i, QModelIndex(), !categoryEnabled);
        }
    }
    if (changed) {
        updateGeometries();
    }
}



} //namespace HEHUI
