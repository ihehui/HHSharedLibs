/*
 ****************************************************************************
 * mainwindowbase.h
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





#ifndef MAINWINDOWBASE_H_
#define MAINWINDOWBASE_H_

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QList>


#include <QSystemTrayIcon>

//#include "../../core/global_core.h"
//#include "../../core/plugin/abstractplugininterface.h"
//#include "../../core/plugin/coreinterface.h"

#include "HHSharedCore/hglobal_core.h"
//#include "HHSharedCore/hcoreinterface.h"

#include "../plugin/guiinterface.h"

#include "../guilib.h"


namespace HEHUI
{

class GUI_LIB_API  MainWindowBase: public QMainWindow
{
    Q_OBJECT

public:
    MainWindowBase(QWidget *parent = 0);
    virtual~ MainWindowBase();

    //virtual void restoreWindowState() = 0;
    //virtual void saveWindowState() = 0;

    virtual void createActions();
    //virtual void setupMenus();
    //virtual void updateMenus();


    virtual void retranslateUi() = 0;

    virtual void moveWindow(HEHUI::WindowPosition positon);

    //QProgressBar* progressBar();

//    QList<HEHUI::AbstractPluginInterface *> pluginsList() const;
//    QList<HEHUI::GUIInterface *> guiPluginsList() const;

    virtual void loadPlugins(const QString &pluginsDirPath = QApplication::applicationDirPath() + QDir::separator () + QString(PLUGINS_MAIN_DIR) + QDir::separator () + QString(PLUGINS_MYPLUGINS_DIR));
    void unloadPlugins();

    bool useStylePalette();
    QString preferedStyle();
    QString preferedLanguage();

public slots:
    void slotResetStatusBar(bool show);

    QMenu *getLanguageMenu(const QString &qmFileDirPath, const QString &local);
    QMenu *getStyleMenu(const QString &preferedStyle, bool useStylePalette = false);
    QMenu *getPluginsMenu();
    QAction *getPluginsManagementAction();


private slots:
    virtual void slotInitPlugin(AbstractPluginInterface *plugin) = 0;


    void slotChangeLanguage(QAction *action);
    void slotChangeStyle(QAction *action);
    void slotChangePalette(bool useStylePalette);
    void slotManagePlugins();


private:
    virtual void initStatusBar();

    void changeStyle(const QString &style);
    void languageChanged();

    virtual void savePreferedStyle(const QString &preferedStyle, bool useStylePalette) = 0;
    virtual void savePreferedLanguage(const QString &preferedLanguage) = 0;


protected:
    bool event( QEvent *e );
    void closeEvent ( QCloseEvent *event ) = 0;


private:
    QWidget *m_progressWidget;
    QHBoxLayout *hlayout;
    //QLabel *label;
    QProgressBar *progressBarOnStatusBar;

    QPalette originalPalette;
    bool m_useStylePalette;
    QString m_preferedStyle;
    QString m_preferedLanguage;

    QAction *actionLanguageDefaultEnglish;
    QAction *actionUseStylesPalette;
    QAction *actionPluginsManagement;

    QString qmPath;
    QString qmLocale;

    QMenu *m_languageMenu;
    QMenu *m_styleMenu;
    QMenu *m_pluginsMenu;


//    QList<HEHUI::AbstractPluginInterface *> plugins;
//    QList<HEHUI::GUIInterface *> guiPlugins;

};

} //namespace HEHUI

#endif /* MAINWINDOWBASE_H_ */
