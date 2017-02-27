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





#ifndef UTILITIES_H_
#define UTILITIES_H_




#include <QString>
#include <QTranslator>
#include <QMutex>

#include "core_lib.h"



namespace HEHUI
{


class CORE_LIB_API Utilities
{

public:
    Utilities();
    virtual ~Utilities();

    static const QString currentUserNameOfOS();

    static QStringList availableTranslationLanguages(const QString &translationFilesDir);

    //载入语言文件
    //Load translation
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

#endif /* UTILITIES_H_ */
