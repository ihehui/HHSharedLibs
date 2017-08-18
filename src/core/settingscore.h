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


namespace HEHUI
{


class CORE_LIB_API SettingsCore : public QSettings
{
    Q_OBJECT
public:
    SettingsCore(const QString fileBaseName, const QString fileDirPath = QCoreApplication::applicationDirPath(), QObject *parent = 0 );
    SettingsCore(const QString fileName, Format format, QObject *parent = 0 );

    ~SettingsCore();


    void setLanguage(const QString &language);
    QString getLanguage() const;

    void setValueWithEncryption(const QString &key, const QVariant &value, const QByteArray &encryptionKey);
    QVariant getValueWithDecryption(const QString &key, const QByteArray &encryptionKey, const QVariant &defaultValue = QVariant(), bool *ok = 0) const;

    void enableLog(bool enable, const QString &logFileBaaseName = "log", bool logToConsole = false);
    void initLogger(quint8 targets, const QString &logFileBaaseName = "log");
    void closeLogger();
    bool isLogEnabled();


private:
    QString mProgramName;
    QString mProgramVersion;

    bool m_enableLog;

};


} //namespace HEHUI

#endif // SETTINGSCORE_H
