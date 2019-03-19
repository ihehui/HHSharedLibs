/*
 ****************************************************************************
 * settingsbase.h
 *
 * Created on: 2009-11-6
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
 * Last Modified on: 2010-05-14
 * Last Modified by: 贺辉
 ***************************************************************************
 */



#ifndef SETTINGSBASE_H
#define SETTINGSBASE_H

#include <QSettings>
#include <QMainWindow>

//#include "../core/settingscore.h"

#include "HHSharedCore/hsettingscore.h"
#include "guilib.h"



namespace HEHUI
{


class  GUI_LIB_API SettingsBase : public SettingsCore
{
    Q_OBJECT
public:
    SettingsBase( const QString fileBaseName, const QString fileDirPath = QCoreApplication::applicationDirPath(), QObject *parent = 0 );
    ~SettingsBase();

    void setStyle(const QString &style);
    QString getStyle() const;
    void setUseStylesPalette(bool checked);
    bool isUsingStylesPalette();

    void setHideOnClose(bool hideOnClose);
    bool getHideOnClose();

    virtual void restoreState( QMainWindow *mw);
    virtual void saveState( QMainWindow *mw);

    void setRestoreWindowStateOnStartup(bool restore);
    bool getRestoreWindowStateOnStartup();

    void setLanguage(const QString &language);
    QString getLanguage() const;

    //static QStringList availableTranslationLanguages(const QString &translationFilesDir);
    ////Load translation
    //static bool changeLangeuage(const QString &translationFilesDir, const QString &qmLocale);

public slots:
    //void loadStyleSettings();
    //void setupStyleMenu(QMenu *styleMenu);

    //void setupLanguageMenu(QMenu *languageMenu, const QString &translationFilesDir = "");



private slots:
    //void changeStyle(QAction *styleAction);
    //void changePalette();

    //void changeLanguage(QAction *languageAction);


private:
    //QString m_translationFilesDir;
    //static QList<QTranslator *>translators;

};

} //namespace HEHUI

#endif // SETTINGSBASE_H
