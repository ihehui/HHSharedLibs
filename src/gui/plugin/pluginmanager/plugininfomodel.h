/*
 ****************************************************************************
 * plugininfomodel.h
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
 * Last Modified On: 2010-5-12
 * Last Modified By: 贺辉
 ***************************************************************************
 */



#ifndef PLUGININFOMODEL_H_
#define PLUGININFOMODEL_H_

#include <QAbstractTableModel>
#include <QList>

#include "../../../core/plugin/abstractplugininterface.h"


namespace HEHUI {

class PluginInfoModel : public QAbstractTableModel{
    Q_OBJECT

public:
    PluginInfoModel(QObject *parent = 0);
    PluginInfoModel(QHash<QString, AbstractPluginInterface *> pluginsHash, QObject *parent = 0);
    //     PluginInfoModel(QList<HEHUI::AbstractPluginInterface *> plugins, QObject *parent = 0);

    virtual ~PluginInfoModel();

    void setPluginsHash(const QHash<QString, AbstractPluginInterface *> &pluginsHash);
    void clear();
    void removePluginInfo(const QString &path);
    void removePluginInfo(AbstractPluginInterface *plugin);
    void addPluginInfo(const QString &path, AbstractPluginInterface *plugin);


    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;


private:
    QHash<QString, AbstractPluginInterface *> pluginsHash;
    //    QList<HEHUI::AbstractPluginInterface *> plugins;

};

}

#endif /* PLUGININFOMODEL_H_ */
