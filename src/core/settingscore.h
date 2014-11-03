/*
 ****************************************************************************
 * settingscore.h
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
  * Last Modified on: 2010-05-07
  * Last Modified by: 贺辉
  ***************************************************************************
*/



#ifndef SETTINGSCORE_H
#define SETTINGSCORE_H

#include <QSettings>
#include <QCoreApplication>

#include "core_lib.h"


class CORE_LIB_API SettingsCore : public QSettings
{
	Q_OBJECT
public:
    SettingsCore( const QString &appName, const QString &appVersion, const QString fileBaseName, const QString fileDirPath = QCoreApplication::applicationDirPath(), QObject* parent= 0 );
	~SettingsCore();

	QString programName() const;
	QString programVersion() const;

	void setLanguage(const QString &language);
	QString getLanguage() const;


private:
    QString mProgramName;
    QString mProgramVersion;


};

#endif // SETTINGSCORE_H
