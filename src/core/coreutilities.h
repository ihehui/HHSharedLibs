/****************************************************************************
* utilities.h
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





#ifndef COREUTILITIES_H_
#define COREUTILITIES_H_




#include <QString>
#include <QTranslator>
#include <QMutex>

#include "core_lib.h"



namespace HEHUI
{


class CORE_LIB_API CoreUtilities
{

public:
    CoreUtilities();
    virtual ~CoreUtilities();

    static const QString currentUserNameOfOS();

    //// The file name should have the format "filename_language[_country].qm", where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase, two- or three-letter ISO 3166 country code. For example "myapp_zh_CN.qm".
    static QStringList availableTranslationLanguages(const QString &translationFilesDir);
    //Load translation
    //// qmLocale: a string of the form "language_country", where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase, two- or three-letter ISO 3166 country code, For example "zh_CN".
    static bool changeLangeuage(const QString &translationFilesDir, const QString &qmLocale);

    static int versionCompare(const QString &exeFile1Version, const QString &exeFile2Version);

    static void msleep(int msec);

    static const QString getFileMD5EncodedWithHex(const QString &fileName);

    static QString simplifyRichTextFilter(const QString &in, bool *isPlainTextPtr = 0);

    //Get the current time of day, returned in millisecond
    static quint64 timeGet();

private:
    static QList<QTranslator *>translators;
    static QMutex *translatorsMutex;




};

} //namespace HEHUI

#endif /* COREUTILITIES_H_ */
