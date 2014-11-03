#include "itemboxeventhandler.h"

#include <QDebug>

namespace HEHUI {


ItemBoxEventHandler::ItemBoxEventHandler()
{

}

void ItemBoxEventHandler::handleItemPressed(const QString &item_id, const QPoint &globalPos){
    qDebug()<<"ItemBoxCore::handleItemPressed(...)"<<" item_id:"<<item_id;
}

void ItemBoxEventHandler::handleItemActivated(const QModelIndex & index){
    qDebug()<<"ItemBoxCore::handleItemActivated(...)"<<" Data:"<<index.data().toString();
}

void ItemBoxEventHandler::handleItemActivated(const QString &id){
    qDebug()<<"ItemBoxCore::handleItemActivated(...)"<<" id:"<<id;

}

void ItemBoxEventHandler::handleItemDoubleClicked(const QModelIndex & index){
    qDebug()<<"ItemBoxCore::handleItemDoubleClicked(...)"<<" Data:"<<index.data().toString();

}

void ItemBoxEventHandler::handleItemEntered(const QModelIndex & index ){
    qDebug()<<"ItemBoxCore::handleItemEntered(...)"<<" Data:"<<index.data().toString();

}

void ItemBoxEventHandler::handleContextMenuEventOnCategory(const QString &cat_id, const QPoint &global_mouse_pos, QMenu *contextMenu){
    qDebug()<<"ItemBoxCore::handleContextMenuEventOnCategory(...)"<<" cat_id:"<<cat_id;

}

void ItemBoxEventHandler::handleContextMenuEventOnItem(const QString &item_id, const QPoint &global_mouse_pos, QMenu *contextMenu){
    qDebug()<<"ItemBoxCore::handleContextMenuEventOnItem(...)"<<" item_id:"<<item_id;

}

void ItemBoxEventHandler::handleTooltipEventOnItem(const QString &item_id, const QPoint &global_item_topLeft_pos, const QPoint &global_mouse_pos){
    qDebug()<<"ItemBoxCore::handleTooltipEventOnItem(...)"<<" item_id:"<<item_id;

}



} //namespace HEHUI
