/*
 ****************************************************************************
 * pluginmanager.h
 *
 * Created On: 2010-5-11
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
 * Last Modified On: 2010-5-20
 * Last Modified By: 贺辉
 ***************************************************************************
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include <QObject>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QList>
#include <QHash>
#include <QPluginLoader>

#include "../global_core.h"
#include "abstractplugininterface.h"
#include "../core_lib.h"

namespace HEHUI {


class CORE_LIB_API PluginManager: public QObject {
    Q_OBJECT

public:

    virtual ~PluginManager();

    static PluginManager* instance();

    //Read only
    const QHash<QString, AbstractPluginInterface *> & getPluginsHash() const;

    //Plugins list
    QList<AbstractPluginInterface *> pluginsList() const;

    QString pluginFilePath(AbstractPluginInterface *plugin) const;

    //Load plugins
    virtual void loadPlugins(const QString &pluginsDirPath = QCoreApplication::applicationDirPath() + QDir::separator()
            + QString(PLUGINS_MAIN_DIR) + QDir::separator() + QString(PLUGINS_MYPLUGINS_DIR));
    virtual AbstractPluginInterface * loadPlugin(const QString &pluginFilePath, QString *errorString = 0);
    virtual bool reloadPlugin(const QString &pluginFilePath);
    virtual bool unloadPlugins();

private:
    PluginManager(QObject *parent = 0);

signals:
    void signalPluginLoaded(const QString &pluginFilePath);
    void signalPluginLoaded(AbstractPluginInterface *plugin);
    //void signalPluginUnloaded(const QString &pluginFilePath);

public slots:
    //Unload plugin
    bool unloadPlugin(AbstractPluginInterface *plugin);
    bool unloadPlugin(const QString &pluginFilePath);

private:
    static PluginManager *pluginManager;

    QHash<QString, AbstractPluginInterface *> pluginsHash;
    QHash<QString, QPluginLoader *> pluginLoadersHash;

    //TODO:Enhance the security!
    //QHash<QString, AbstractPluginInterface **> pluginsHash;


};

} //namespace HEHUI

#endif /* PLUGINMANAGER_H_ */
