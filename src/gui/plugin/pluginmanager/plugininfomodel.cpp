/*
 ****************************************************************************
 * plugininfomodel.cpp
 *
 * Created On: 2010-5-10
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
 * Last Modified On: 2010-5-13
 * Last Modified By: 贺辉
 ***************************************************************************
 */

//#include<QUrl>

#include "plugininfomodel.h"
//#include "../../../shared/core/plugin/pluginmanager.h"


namespace HEHUI {

PluginInfoModel::PluginInfoModel(QObject *parent) :
    QAbstractTableModel(parent) {
    // pluginsHash = PluginManager::instance()->getPluginsHash();
    pluginsHash = QHash<QString, AbstractPluginInterface *> ();

}

PluginInfoModel::PluginInfoModel(
        QHash<QString, AbstractPluginInterface *> pluginsHash, QObject *parent) :
    QAbstractTableModel(parent), pluginsHash(pluginsHash) {
    // TODO Auto-generated constructor stub

}

//PluginInfoModel::PluginInfoModel(QList<HEHUI::AbstractPluginInterface *> plugins, QObject *parent)
//        :QAbstractTableModel(parent), plugins(plugins)
//{
//	// TODO Auto-generated constructor stub
//
//}

PluginInfoModel::~PluginInfoModel() {
    // TODO Auto-generated destructor stub
}

void PluginInfoModel::setPluginsHash(const QHash<QString, AbstractPluginInterface *> &pluginsHash) {
    beginResetModel();
    this->pluginsHash = pluginsHash;
    endResetModel();
}

void PluginInfoModel::clear(){

    beginResetModel();
    this->pluginsHash.clear();
    endResetModel();

}

void PluginInfoModel::removePluginInfo(const QString &path){

    beginResetModel();
    this->pluginsHash.remove(path);
    endResetModel();
}

void PluginInfoModel::removePluginInfo(AbstractPluginInterface *plugin){

    beginResetModel();
    pluginsHash.remove(pluginsHash.key(plugin));
    endResetModel();

}

void PluginInfoModel::addPluginInfo(const QString &path, AbstractPluginInterface *plugin){

    beginResetModel();
    pluginsHash.insert(path, plugin);
    endResetModel();

}



int PluginInfoModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return pluginsHash.count();

}

int PluginInfoModel::columnCount(const QModelIndex & parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return 5;

}

QVariant PluginInfoModel::data(const QModelIndex &index, int role) const {
    //qDebug("----PluginInfoModel::data(const QModelIndex &index, int role)");

    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= pluginsHash.count() || index.row() < 0) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        HEHUI::AbstractPluginInterface *plugin = pluginsHash.values().at(index.row());
        switch (index.column()) {
        case 0:
            return "Enabled";
            break;
        case 1:
            return plugin->name();
        case 2:
            return plugin->version();
            break;
        case 3:
            return plugin->vendor();
            break;
        case 4:
            return pluginsHash.key(plugin);
            break;
            //        case 5:
            //                    return plugin->author();
            //                    break;
            //        case 6:
            //                    return QString("<a href = \"%1\">%1</a>").arg(QUrl::fromUserInput(plugin->url()).toString());
            //                    break;
            //        case 7:
            //                    return plugin->description();
            //                    break;
            //        case 8:
            //                    return plugin->license();
            //                    break;
        default:
            return QVariant();
            break;
        }
    } else {
        return QVariant();
    }

}

QVariant PluginInfoModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QString(tr("State"));
            break;
        case 1:
            return QString(tr("Name"));
            break;
        case 2:
            return QString(tr("Version"));
            break;
        case 3:
            return QString(tr("Vendor"));
            break;
        case 4:
            return QString(tr("Location"));
            break;
        default:
            return QVariant();
            break;
        }
    } else {
        return QVariant();
    }

}















}
