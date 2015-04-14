/*
 ****************************************************************************
 * settingsbase.cpp
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






#include <QDir>
#include <QFile>

#include "settingsbase.h"


namespace HEHUI {

SettingsBase::SettingsBase(const QString fileBaseName, const QString fileDirPath, QObject* parent )
    : SettingsCore(fileBaseName, fileDirPath, parent )
{


}

SettingsBase::~SettingsBase()
{

}

void SettingsBase::setStyle(const QString &style)
{
    setValue("MainWindow/Style", style);
}

QString SettingsBase::getStyle() const
{

#ifdef Q_OS_WIN
    return value("MainWindow/Style", QString("fusion")).toString();
#endif

    return value("MainWindow/Style","").toString();

}


void SettingsBase::setPalette(bool useStylePalette)
{
    setValue("MainWindow/UseStylePalette", useStylePalette);
}

bool SettingsBase::getPalette()
{
    return value("MainWindow/UseStylePalette",false).toBool();
}

void SettingsBase::setHideOnClose(bool hideOnClose)
{
    setValue("MainWindow/HideOnClose", hideOnClose);
}

bool SettingsBase::getHideOnClose()
{
    return value("MainWindow/HideOnClose",true).toBool();
}

void SettingsBase::restoreState( QMainWindow* mw)
{
    if ( !mw )
        return;
    mw->restoreState( value( "MainWindow/State" ).toByteArray() );
    QPoint p = value( "MainWindow/Position" ).toPoint();
    QSize s = value( "MainWindow/Size" ).toSize();
    if ( !p.isNull() && !s.isNull() )
    {
        mw->resize( s );
        mw->move( p );
    }

    if ( value( "MainWindow/Maximized", false ).toBool() ){
        mw->showMaximized();
    }


    if ( value( "MainWindow/Hidden", false ).toBool() )
    {
        mw->hide();
    }

}

void SettingsBase::saveState( QMainWindow* mw)
{
    if ( !mw ){
        return;
    }

    setValue( "MainWindow/Maximized", mw->isMaximized() );
    setValue( "MainWindow/Position", mw->pos() );
    setValue( "MainWindow/Size", mw->size() );
    setValue( "MainWindow/State", mw->saveState() );
    setValue( "MainWindow/Hidden", mw->isHidden() );

}


void SettingsBase::setRestoreWindowStateOnStartup(bool restore){
    setValue( "MainWindow/RestoreWindowStateOnStartup", restore );

}

bool SettingsBase::getRestoreWindowStateOnStartup(){
    return value("MainWindow/RestoreWindowStateOnStartup", false).toBool();

}













} //namespace HEHUI


