#ifndef ITEMBOXEVENTHANDLER_H
#define ITEMBOXEVENTHANDLER_H

#include <QString>
#include <QModelIndex>
#include <QMenu>

#include "itemboxlib.h"

namespace HEHUI
{


class ITEMBOX_LIB_API ItemBoxEventHandler
{

public:
    explicit ItemBoxEventHandler();


signals:


public /*slots*/:
    virtual void handleItemPressed(const QString &item_id, const QPoint &globalPos);
    virtual void handleItemActivated(const QModelIndex &index);
    virtual void handleItemActivated(const QString &id);
    virtual void handleItemDoubleClicked(const QModelIndex &index);
    virtual void handleItemEntered(const QModelIndex &index );

    virtual void handleContextMenuEventOnCategory(const QString &cat_id, const QPoint &global_mouse_pos, QMenu *contextMenu);
    virtual void handleContextMenuEventOnItem(const QString &item_id, const QPoint &global_mouse_pos, QMenu *contextMenu);
    virtual void handleTooltipEventOnItem(const QString &item_id, const QPoint &global_item_topLeft_pos, const QPoint &global_mouse_pos);



};

} //namespace HEHUI

#endif // ITEMBOXEVENTHANDLER_H
