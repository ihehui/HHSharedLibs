/****************************************************************************
* winutilities.h
*
* Created on: 2009-11-9
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
  * Last Modified on: 2010-08-19
  * Last Modified by: 贺辉
  ***************************************************************************
*/





#ifndef WINUTILITIES_H_
#define WINUTILITIES_H_



#include <QObject>
//#include <QPixmap>

#include <Windows.h>
#include "wmlib.h"



namespace HEHUI {


class WM_LIB_API WinUtilities {

public:
    WinUtilities();
    virtual ~WinUtilities();

    static QString WinSysErrorMsg(DWORD winErrorCode, DWORD dwLanguageId = 0);


    //Registry
    static bool parseRegKeyString(const QString &keyString, HKEY *rootKey, QString *subKeyString);
    static bool regOpen(const QString &key, HKEY *hKey, REGSAM samDesired = KEY_READ);
    static bool regRead(HKEY hKey, const QString &valueName, QString *value);
    static bool regRead(const QString &key, const QString &valueName, QString *value, bool on64BitView = false);
    static bool regEnumVal(HKEY hKey, QStringList *valueNameList);
    static bool regEnumVal(const QString &key, QStringList *valueNameList, bool on64BitView = false);
    static bool regEnumKey(HKEY hKey, QStringList *keyNameList);
    static bool regEnumKey(const QString &key, QStringList *keyNameList, bool on64BitView = false);
    static bool regCreateKey(HKEY hKey, const QString &subKeyName, HKEY *hSubKey = 0);
    static bool regCreateKey(const QString &key, const QString &subKeyName, HKEY *hSubKey = 0, bool on64BitView = false);
    static bool regSetValue(HKEY hKey, const QString &valueName, const QString &value, DWORD valueType);
    static bool regSetValue(const QString &key, const QString &valueName, const QString &value, DWORD valueType, bool on64BitView = false);
    static bool regDeleteKey(HKEY hKey, const QString &subKeyName, bool on64BitView = false);
    static bool regDeleteKey(const QString &key, bool on64BitView = false);
    static bool regDeleteValue(HKEY hKey, const QString &valueName);
    static bool regDeleteValue(const QString &key, const QString &valueName, bool on64BitView = false);
    static void regCloseKey(HKEY hKey);

    static bool is64BitApplication();
    static bool isWow64();

    //GDI+
    //Get image format Clsid
    static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
    static QByteArray ConvertHBITMAPToJpeg(HBITMAP hbitmap);
    //static QPixmap WinHBITMAPToPixmap(HBITMAP bitmap, bool noAlpha = true);
    static HBITMAP GetScreenshotBmp();



private:


};

} //namespace HEHUI

#endif /* WINUTILITIES_H_ */
