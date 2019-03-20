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

#include "HHSharedCore/GlobalCore"

#include "../plugin/guiinterface.h"

#include "../guilib.h"

#include "../guiutilities.h"


namespace HEHUI
{

class GUI_LIB_API  MainWindowBase: public QMainWindow
{
    Q_OBJECT

public:
    MainWindowBase(const QString &settingsFile, QWidget *parent = 0, GUIUtilities::WindowPosition positon = GUIUtilities::CENTER);
    virtual~ MainWindowBase();


    virtual void retranslateUi() = 0;

    virtual void loadPlugins(const QString &pluginsDirPath = QApplication::applicationDirPath() + QDir::separator () + QString(PLUGINS_MAIN_DIR) + QDir::separator () + QString(PLUGINS_MYPLUGINS_DIR));
    void unloadPlugins();

//    bool useStylePalette();
//    QString preferedStyle();
//    QString preferedLanguage();

    GUIUtilities *guiUtilities();

public slots:
    void slotResetStatusBar(bool show);

    QMenu *getLanguageMenu();
    QMenu *getStyleMenu();
    QMenu *getPluginsMenu();
    QAction *getPluginsManagementAction();


private slots:
    virtual void slotInitPlugin(AbstractPluginInterface *plugin) = 0;
    void slotManagePlugins();

private:
    virtual void initStatusBar();

    void languageChanged();


protected:
    bool event( QEvent *e );
    void closeEvent ( QCloseEvent *event ) = 0;


private:
    QWidget *m_progressWidget;
    QHBoxLayout *hlayout;
    //QLabel *label;
    QProgressBar *progressBarOnStatusBar;

    QAction *actionPluginsManagement;

    QMenu *m_languageMenu;
    QMenu *m_styleMenu;
    QMenu *m_pluginsMenu;

    GUIUtilities *m_guiUtilities;


//    QList<HEHUI::AbstractPluginInterface *> plugins;
//    QList<HEHUI::GUIInterface *> guiPlugins;

};

} //namespace HEHUI

#endif /* MAINWINDOWBASE_H_ */
