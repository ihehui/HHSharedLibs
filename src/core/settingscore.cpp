/*
 ****************************************************************************
 * settingscore.cpp
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




#include <QDir>
#include <QFile>
#include <QLocale>

#include "settingscore.h"
#include "cryptography/cryptography.h"



namespace HEHUI
{



SettingsCore::SettingsCore(const QString fileBaseName, const QString fileDirPath, QObject *parent )
    : QSettings( QDir::toNativeSeparators( QString( "%1/%2.ini" ).arg( fileDirPath, fileBaseName) ), QSettings::IniFormat, parent )

{


}

SettingsCore::SettingsCore(const QString fileName, Format format, QObject *parent )
    : QSettings( fileName, format, parent )
{


}

SettingsCore::~SettingsCore()
{
    //endGroup();
}

void SettingsCore::setLanguage(const QString &language)
{
    setValue("MainWindow/Language", language);
}

QString SettingsCore::getLanguage() const
{
    //return value("MainWindow/Language",QString("en_US")).toString();
    return value("MainWindow/Language", QLocale::system().name()).toString();
}

void SettingsCore::setValueWithEncryption(const QString &settingsKey, const QVariant &value, const QByteArray &encryptionKey)
{
    QByteArray destination;
    Cryptography cryptography;
    cryptography.teaCrypto(&destination, value.toByteArray(), encryptionKey, true);
    setValue(settingsKey, QVariant(destination));
}

QVariant SettingsCore::getValueWithDecryption(const QString &settingsKey, const QByteArray &encryptionKey, const QVariant &defaultValue, bool *ok) const
{
    if(!contains(settingsKey)) {
        if(ok) {
            *ok = false;
        }
        return defaultValue;
    }

    QByteArray array = value(settingsKey).toByteArray();
    QByteArray destination;
    Cryptography cryptography;
    int ret = cryptography.teaCrypto(&destination, array, encryptionKey, false);
    if(ok) {
        *ok = ret;
    }
    if(!ret) {
        return defaultValue;
    }
    return QVariant(destination);
}






} //namespace HEHUI







