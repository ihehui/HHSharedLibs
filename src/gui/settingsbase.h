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



namespace HEHUI {


class  GUI_LIB_API SettingsBase : public SettingsCore
{
    Q_OBJECT
public:
    SettingsBase( const QString fileBaseName, const QString fileDirPath = QCoreApplication::applicationDirPath(), QObject* parent= 0 );
    ~SettingsBase();


    void setStyle(const QString &style);
    QString getStyle() const;

    void setPalette(bool useStylePalette);
    bool getPalette();

    void setHideOnClose(bool hideOnClose);
    bool getHideOnClose();

    virtual void restoreState( QMainWindow* mw);
    virtual void saveState( QMainWindow* mw);

    void setRestoreWindowStateOnStartup(bool restore);
    bool getRestoreWindowStateOnStartup();

private:



};

} //namespace HEHUI

#endif // SETTINGSBASE_H
