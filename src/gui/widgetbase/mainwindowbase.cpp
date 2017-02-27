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

    actionLanguageDefaultEnglish = 0;
    actionUseStylesPalette = 0;
    actionPluginsManagement = 0;

    m_languageMenu = 0;
    m_styleMenu = 0;
    m_pluginsMenu = 0;


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


/*
 void MainWindowBase::restoreWindowState() {

 }

 void MainWindowBase::saveWindowState() {

 }
 */

void MainWindowBase::createActions()
{

}

//void MainWindowBase::setupMenus() {
//
//}
//
//void MainWindowBase::updateMenus() {
//
//}

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

bool MainWindowBase::useStylePalette()
{
    return m_useStylePalette;
}

QString MainWindowBase::preferedStyle()
{
    return m_preferedStyle;
}

QString MainWindowBase::preferedLanguage()
{
    return m_preferedLanguage;
}

void MainWindowBase::moveWindow(HEHUI::WindowPosition positon)
{
    // Get the size of screen
    QDesktopWidget *desktop = QApplication::desktop();
    QRect rect = desktop->availableGeometry(this);
    int desktopWidth = rect.width();
    int desktopHeight = rect.height();

    int windowWidth = frameGeometry().width();
    int windowHeight = frameGeometry().height();

    //move the window
    switch (positon) {
    case HEHUI::Center:
        move((desktopWidth - windowWidth) / 2, (desktopHeight - windowHeight) / 2);
        break;
    case HEHUI::BottomRight:
        move((desktopWidth - windowWidth), (desktopHeight - windowHeight));
        break;
    default:
        move((desktopWidth - windowWidth) / 2, (desktopHeight - windowHeight) / 2);

    }

}

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
        //Language
        actionLanguageDefaultEnglish = new QAction(tr("Default(English)"), this);
        actionLanguageDefaultEnglish->setData("en_US");
        actionLanguageDefaultEnglish->setCheckable(true);
        m_languageMenu->addAction(actionLanguageDefaultEnglish);

        QActionGroup *languageActionGroup = new QActionGroup(this);
        languageActionGroup->addAction(actionLanguageDefaultEnglish);
        QObject::connect(languageActionGroup, SIGNAL(triggered(QAction *)), this,
                         SLOT(slotChangeLanguage(QAction *)));

        QStringList translationLanguages = Utilities::availableTranslationLanguages(qmPath);
        //如果没有找到语言文件或者首选语言是英语,则将"en_US"置为选中
        //If no language file is found or the preferred language is English, set the "en_US" menu checked
        if (translationLanguages.isEmpty()) {
            actionLanguageDefaultEnglish->setChecked(true);
            m_preferedLanguage = "en_US";
            return m_languageMenu;
        }

        if (!translationLanguages.contains(qmLocale)) {
            qmLocale = "en_US";
        }

        if(qmLocale == "en_US") {
            actionLanguageDefaultEnglish->setChecked(true);
        }

        //为每一个语言文件生成动作为菜单
        //Make action and menu for each language file
        for (int i = 0; i < translationLanguages.size(); i++) {
            QString translationLanguage = translationLanguages[i];
            QLocale local(translationLanguage);
            QString LanguageName = QLocale::languageToString(local.language());
            QString regionName = QLocale::countryToString(local.country());

            //QAction *action = new QAction(tr("&%1 %2") .arg(i + 1).arg(translationLanguage), this);
            QAction *action = new QAction(tr("%1(%2)") .arg(LanguageName).arg(regionName), this);

            action->setCheckable(true);
            action->setData(translationLanguage);

            m_languageMenu->addAction(action);
            languageActionGroup->addAction(action);

            if (qmLocale == translationLanguage) {
                action->setChecked(true);
                action->trigger();
            }

        }

    }


    return m_languageMenu;

}

QMenu *MainWindowBase::getStyleMenu(const QString &preferedStyle, bool useStylePalette)
{

    this->m_preferedStyle = preferedStyle;
    this->m_useStylePalette = useStylePalette;
    if(!m_styleMenu) {
        m_styleMenu = new QMenu(tr("&Style"), this);

        //Style
        QActionGroup *styleActionGroup = new QActionGroup(this);
        QStringList stylesList = QStyleFactory::keys();
        for(int i = 0; i < stylesList.size(); i++) {
            QString styleName = stylesList.at(i);
            QAction *styleAction = new QAction(styleName, this);
            styleAction->setData(stylesList.at(i));
            styleAction->setCheckable(true);
            styleActionGroup->addAction(styleAction);
            m_styleMenu->addAction(styleAction);

            if(styleName.toLower() == m_preferedStyle.toLower()) {
                styleAction->setChecked(true);
                changeStyle(m_preferedStyle);
            }

        }
        m_styleMenu->addSeparator();

        actionUseStylesPalette = new QAction(tr("Use Style's Palette"), this);
        actionUseStylesPalette->setCheckable(true);
        m_styleMenu->addAction(actionUseStylesPalette);


        connect(styleActionGroup, SIGNAL(triggered(QAction *) ), this,
                SLOT(slotChangeStyle(QAction *)));
        connect(actionUseStylesPalette, SIGNAL(triggered(bool)), this,
                SLOT(slotChangePalette(bool)));

        //        //更新样式菜单
        //        //update Style Menu
        //        foreach(QAction *action, m_styleMenu->actions()){
        //                if(action->data().toString().toLower() == m_preferedStyle.toLower()){
        //                        action->setChecked(true);
        //                        changeStyle(m_preferedStyle);
        //                }
        //        }
        actionUseStylesPalette->setChecked(useStylePalette);

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

void MainWindowBase::changeStyle(const QString &style)
{

    QApplication::setStyle(style);

    slotChangePalette(m_useStylePalette);
    //保存首选样式
    //Save the preferred style
    //Settings::instance()->setStyle(style);

}

void MainWindowBase::languageChanged()
{
    qDebug() << "--MainWindowBase::languageChanged()";

    retranslateUi();

    if(m_languageMenu) {
        m_languageMenu->setTitle(tr("&Language"));
        actionLanguageDefaultEnglish->setText(tr("Default(English)"));
    }

    if(m_styleMenu) {
        m_styleMenu->setTitle(tr("&Style"));
        actionUseStylesPalette->setText(tr("Use Style's Palette"));
    }

    if(m_pluginsMenu) {
        m_pluginsMenu->setTitle(tr("&Plugins"));
        actionPluginsManagement->setText(tr("&Management"));
    }

}


void MainWindowBase::slotChangeLanguage(QAction *action)
{
    qWarning("----MainWindowBase::slotChangeLanguage(QAction *action)");

    QString lang = action->data().toString();
    //        if (!Utilities::availableTranslationLanguages(qmPath).contains(lang)){
    //		lang = "en_US";
    //	}

    bool ok = Utilities::changeLangeuage(qmPath, lang);
    if(lang != "en_US" && !ok) {
        QMessageBox::critical(this, tr("Error"), tr("Can not switch to the language '%1'!").arg(lang));
        slotChangeLanguage(actionLanguageDefaultEnglish);
        return;
    }

    //	QString langFile = QString(LANGUAGE_FILE_PREFIX) + lang + QString(".qm");
    //	appTranslator.load(langFile, qmPath);
    //        qDebug()<<"~~ Loading language file :"<<langFile ;
    //	langFile = "qt_" + lang + QString(".qm");
    //	qtTranslator.load(langFile, qmPath);
    //        qDebug()<<"~~ Loading language file :"<<langFile ;

    if (!action->isChecked()) {
        action->setChecked(true);
    }

    m_preferedLanguage = lang;

    savePreferedLanguage(m_preferedLanguage);

}

void MainWindowBase::slotChangeStyle(QAction *action)
{
    //更新样式
    //Update the app style
    m_preferedStyle = action->data().toString();
    //QApplication::setStyle(preferedStyle);
    changeStyle(m_preferedStyle);
    //slotChangePalette(ui.actionUseStylesPalette->isChecked());
    if (!action->isChecked()) {
        action->setChecked(true);
    }

    savePreferedStyle(m_preferedStyle, m_useStylePalette);

}

void MainWindowBase::slotChangePalette(bool useStylePalette)
{
    if (useStylePalette) {
        QApplication::setPalette(QApplication::style()->standardPalette());
    } else {
        QApplication::setPalette(originalPalette);
    }

    this->m_useStylePalette = useStylePalette;

    savePreferedStyle(m_preferedStyle, m_useStylePalette);


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


