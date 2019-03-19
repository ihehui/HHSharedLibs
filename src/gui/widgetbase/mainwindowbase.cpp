/*
 ****************************************************************************
 * mainwindowbase.cpp
 *
 * Created on: 2009-4-27
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
 * Last Modified on: 2010-05-20
 * Last Modified by: 贺辉
 ***************************************************************************
 */

#include <QApplication>
#include <QStatusBar>
#include <QMenu>
#include <QDesktopWidget>
#include <QMessageBox>

#include "mainwindowbase.h"

#include "../../core/plugin/pluginmanager.h"
#include "../../core/utilities.h"

#include "../plugin/pluginmanager/pluginmanagerwindow.h"



namespace HEHUI
{


MainWindowBase::MainWindowBase(QWidget *parent)
    : QMainWindow(parent)
{

    initStatusBar();

    //loadPlugins();

    originalPalette = QApplication::palette();
    m_useStylePalette = false;

    //actionLanguageDefaultEnglish = 0;
    //actionUseStylesPalette = 0;
    actionPluginsManagement = 0;

    m_languageMenu = 0;
    m_styleMenu = 0;
    m_pluginsMenu = 0;

    m_guiUtilities = new GUIUtilities(this);
    connect(m_guiUtilities, SIGNAL(signalStyleChanged(const QString &)), this, SLOT(savePreferedStyle(const QString &)));
    connect(m_guiUtilities, SIGNAL(signalUsingStylesPaletteChanged(bool)), this, SLOT(saveUsingStylePalette(bool)));
    connect(m_guiUtilities, SIGNAL(signalLanguageChanged(const QString &)), this, SLOT(savePreferedLanguage(const QString &)));
}

MainWindowBase::~MainWindowBase()
{

    if(m_languageMenu) {
        m_languageMenu->deleteLater();
    }

    if(m_styleMenu) {
        m_styleMenu->deleteLater();
    }

    if(m_pluginsMenu) {
        m_pluginsMenu->deleteLater();
    }

    //unloadPlugins();

}

bool MainWindowBase::event( QEvent *e )
{
    if(e->type() == QEvent::LanguageChange) {
        languageChanged();
    }
    return QMainWindow::event(e);
}

void MainWindowBase::loadPlugins(const QString &pluginsDirPath)
{
    qDebug() << "----MainWindowBase::loadPlugins(...)~~Plugins Path:" << pluginsDirPath;

    PluginManager *pluginManager = PluginManager::instance();
    connect(pluginManager, SIGNAL(signalPluginLoaded(AbstractPluginInterface *)), this, SLOT(slotInitPlugin(AbstractPluginInterface *)));
    pluginManager->loadPlugins(pluginsDirPath);

    //        plugins = PluginManager::instance()->pluginsList();
    //        for (int i = 0; i < plugins.size(); i++) {
    //                AbstractPluginInterface *plugin = plugins.value(i);
    //		HEHUI::GUIInterface *guiInterface =
    //				static_cast<HEHUI::GUIInterface *> (plugin);
    //		if (guiInterface) {
    //			guiPlugins.append(guiInterface);
    //		}
    //	}
    //
    //	return;

    /*

    QDir pluginsDir(pluginsPath);
    foreach(QString fileName, pluginsDir.entryList(QDir::Files))
    {
    qDebug() << QString("~~ Testing library %1").arg(
    pluginsDir.absoluteFilePath(fileName));
    QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
    QObject *plugin = pluginLoader.instance();
    if (plugin) {
         HEHUI::AbstractInterface *coreInterface = qobject_cast<HEHUI::AbstractInterface *> (plugin);
    if (coreInterface) {
    corePlugins.append(coreInterface);
    qDebug() << QString("~~ Loading Core Plugin %1").arg(fileName);

    HEHUI::GUIInterface *guiInterface = qobject_cast<HEHUI::GUIInterface *> (plugin);
    if (guiInterface) {
    guiPlugins.append(guiInterface);
    qDebug() << QString("~~ Loading GUI Plugin %1").arg(fileName);
    }

    } else {
    qCritical() << QString("XX Unknown Plugin! ");
    break;
    }

    } else {
    qDebug() << QString("XX An error occurred while loading plugin : %1").arg(
    pluginLoader.errorString());
    }

    }

    */

}

void MainWindowBase::unloadPlugins()
{
    qDebug() << "--MainWindowBase::unloadPlugins()";

    PluginManager *pluginManager = PluginManager::instance();
    pluginManager->unloadPlugins();

    //delete pluginManager();

}

//bool MainWindowBase::useStylePalette()
//{
//    return m_useStylePalette;
//}

//QString MainWindowBase::preferedStyle()
//{
//    return m_preferedStyle;
//}

//QString MainWindowBase::preferedLanguage()
//{
//    return m_preferedLanguage;
//}


void MainWindowBase::initStatusBar()
{
    m_progressWidget = new QWidget();
    hlayout = new QHBoxLayout(m_progressWidget);

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    //label = new QLabel(tr("Updating search index"));
    //label->setSizePolicy(sizePolicy);
    //hlayout->addWidget(label);

    progressBarOnStatusBar = new QProgressBar(m_progressWidget);
    progressBarOnStatusBar->setRange(0, 0);
    progressBarOnStatusBar->setTextVisible(false);
    progressBarOnStatusBar->setSizePolicy(sizePolicy);

    //hlayout->setSpacing(6);
    hlayout->setMargin(0);
    hlayout->addWidget(progressBarOnStatusBar);

    statusBar()->addPermanentWidget(m_progressWidget);
    m_progressWidget->hide();

}

void MainWindowBase::slotResetStatusBar(bool show)
{
    //statusBar()->removeWidget(m_progressWidget);
    //delete m_progressWidget;
    //m_progressWidget = 0;

    if (show) {
        m_progressWidget->show();
    } else {
        progressBarOnStatusBar->reset();
        m_progressWidget->hide();
    }

}

QMenu *MainWindowBase::getLanguageMenu(const QString &qmFileDirPath, const QString &local)
{
    qDebug() << "--setupLanguageMenu(...) " << " qmFileDirPath:" << qmFileDirPath << " local:" << local;

    qmPath = qmFileDirPath;
    qmLocale = local;
    if(!m_languageMenu) {
        m_languageMenu = new QMenu(tr("&Language"), this);
        m_guiUtilities->setupLanguageMenu(m_languageMenu, local, qmFileDirPath);
    }

    return m_languageMenu;
}

QMenu *MainWindowBase::getStyleMenu(const QString &preferedStyle, bool useStylePalette)
{

    this->m_preferedStyle = preferedStyle;
    this->m_useStylePalette = useStylePalette;
    if(!m_styleMenu) {
        m_styleMenu = new QMenu(tr("&Style"), this);
        m_guiUtilities->setupStyleMenu(m_styleMenu, preferedStyle, useStylePalette);
    }

    return m_styleMenu;
}

QMenu *MainWindowBase::getPluginsMenu()
{

    if(!m_pluginsMenu) {
        m_pluginsMenu = new QMenu(tr("&Plugins"), this);

        //        actionPluginsManagement = new QAction(QIcon(":/resources/images/plugin.png"), tr("&Management"), m_pluginsMenu);
        //        connect(actionPluginsManagement, SIGNAL(triggered()), this, SLOT(slotManagePlugins()));
        //        m_pluginsMenu->addAction(actionPluginsManagement);

        m_pluginsMenu->addAction(getPluginsManagementAction());
        m_pluginsMenu->addSeparator();

    }

    return m_pluginsMenu;
}

QAction *MainWindowBase::getPluginsManagementAction()
{
    if(!actionPluginsManagement) {
        actionPluginsManagement = new QAction(QIcon(":/resources/images/plugin.png"), tr("&Management"), m_pluginsMenu);
        connect(actionPluginsManagement, SIGNAL(triggered()), this, SLOT(slotManagePlugins()));
    }

    return actionPluginsManagement;
}

void MainWindowBase::languageChanged()
{
    qDebug() << "--MainWindowBase::languageChanged()";

    retranslateUi();

    if(m_languageMenu) {
        m_languageMenu->setTitle(tr("&Language"));
        //actionLanguageDefaultEnglish->setText(tr("Default(English)"));
    }

    if(m_styleMenu) {
        m_styleMenu->setTitle(tr("&Style"));
        //actionUseStylesPalette->setText(tr("Use Style's Palette"));
    }

    if(m_pluginsMenu) {
        m_pluginsMenu->setTitle(tr("&Plugins"));
        actionPluginsManagement->setText(tr("&Management"));
    }

}

void MainWindowBase::slotChangeLanguage(const QString &preferedLanguage)
{
    m_preferedLanguage = preferedLanguage;
    savePreferedLanguage(preferedLanguage);
}

void MainWindowBase::slotChangeStyle(const QString &preferedStyle)
{
    m_preferedStyle = preferedStyle;
    savePreferedStyle(m_preferedStyle);
}

void MainWindowBase::slotChangePalette(bool useStylePalette)
{
    this->m_useStylePalette = useStylePalette;
    saveUsingStylePalette(m_useStylePalette);
}

void MainWindowBase::slotManagePlugins()
{
    QDialog dlg(this);
    PluginManagerWindow pluginManagerWindow(&dlg);
    QHBoxLayout layout(&dlg);
    layout.addWidget(&pluginManagerWindow);
    dlg.setLayout(&layout);

    dlg.resize(640, 480);
    dlg.setWindowTitle(tr("Plugins"));
    dlg.exec();

}

//QProgressBar* MainWindowBase::progressBar() {
//	return progressBarOnStatusBar;
//}

//QList<HEHUI::AbstractPluginInterface *> MainWindowBase::pluginsList() const {
//        return plugins;
//}
//
//QList<HEHUI::GUIInterface *> MainWindowBase::guiPluginsList() const {
//	return guiPlugins;
//}







} //namespace HEHUI


