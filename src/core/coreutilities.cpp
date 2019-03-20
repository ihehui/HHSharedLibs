/****************************************************************************
* utilities.cpp
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





#include <QCoreApplication>
#include <QStringList>
#include <QProcess>
#include <QDir>
#include <QMutexLocker>
#include <QDebug>
#include <QFile>
#include <QCryptographicHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>



#include "coreutilities.h"

#ifdef Q_OS_WIN32
    #include <windows.h>
    #include <time.h>
#else
    #include <unistd.h>
    #include <sys/time.h>
#endif




namespace HEHUI
{


QList<QTranslator *> CoreUtilities::translators = QList<QTranslator *>();
QMutex *CoreUtilities::translatorsMutex = new QMutex();

CoreUtilities::CoreUtilities()
{
    // TODO Auto-generated constructor stub


}

CoreUtilities::~CoreUtilities()
{
    // TODO Auto-generated destructor stub
}


const QString CoreUtilities::currentUserNameOfOS()
{
    QString username;
    QStringList envVariables;
    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*" << "HOSTNAME.*"
                 << "DOMAINNAME.*";

    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables) {
        int index = environment.indexOf(QRegExp(string));
        if (index != -1) {
            QStringList stringList = environment.at(index).split("=");
            if (stringList.size() == 2) {
                username = stringList.at(1).toUtf8();
                break;
            }
        }
    }

    if (username.isEmpty()) {
        username = "";
    }

    return username;


}

QStringList CoreUtilities::availableTranslationLanguages(const QString &translationFilesDir){

    //Search language files
    QDir dir(translationFilesDir);
    QStringList fileNames = dir.entryList(QStringList("*.qm"));
    qDebug()<<"Available Language Files: "<<fileNames.join(",");

    if (fileNames.isEmpty()) {
        return QStringList();
    }

    QString errorStr = "Invalid file name format! The file name should have the format \"filename_language[_country].qm\", where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase, two- or three-letter ISO 3166 country code. For example \"myapp_zh_CN.qm\".";

    QStringList translationLanguages;
    QString translationLanguageName;
    int lastIdx = 0;
    foreach(QString name, fileNames){
        name.truncate(name.lastIndexOf(".qm", -1, Qt::CaseInsensitive));
        lastIdx = name.lastIndexOf(QRegExp("_[a-z]{2}(_[a-zA-Z]{2,3})?$"));
        if(lastIdx < 1){
            qCritical()<<name<<": "<<errorStr;
            continue;
        }

        translationLanguageName = name.mid(lastIdx+1);
        if(translationLanguages.contains(translationLanguageName)){continue;}

        QLocale local(translationLanguageName);
        if(local.name().size() < 2){
            qCritical()<<name<<": "<<errorStr;
            continue;
        }

        translationLanguages.append(translationLanguageName);
    }

    qDebug()<<"Available translation languages: "<<translationLanguages.join(",");

    return translationLanguages;
}

bool CoreUtilities::changeLangeuage(const QString &translationFilesDir, const QString &qmLocale)
{
    qDebug() << "Locale System Name:" << QLocale::system().name();

    QMutexLocker locker(translatorsMutex);

    int lastIdx = qmLocale.lastIndexOf(QRegExp("^[a-z]{2}(_[a-zA-Z]{2,3})?$"));
    if(lastIdx < 1){
        qCritical() << "Invalid local name format! It should be a string of the form \"filename_language[_country].qm\", where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase, two- or three-letter ISO 3166 country code. For example \"zh_CN\".";
        return false;
    }

    foreach(QTranslator *translator, translators) {
        qApp->removeTranslator(translator);
        delete translator;
        translator = 0;
    }
    translators.clear();


    QStringList filters;
    filters << QString("*" + qmLocale + ".qm");
    foreach(QString file, QDir(translationFilesDir).entryList(filters, QDir::Files | QDir::System | QDir::Hidden)) {
        qDebug() << "~~Loading language file:" << file;
        QTranslator *translator = new QTranslator();
        if(translator->load(file, translationFilesDir)) {
            qApp->installTranslator(translator);
            translators.append(translator);
        } else {
            delete translator;
            translator = 0;
            qCritical() << "ERROR! Loading language file failed:" << file;
        }

    }


    return translators.size();

}

int CoreUtilities::versionCompare(const QString &exeFile1Version, const QString &exeFile2Version)
{


    QStringList exeFile1VersionInfoList = exeFile1Version.split(".");
    QStringList exeFile2VersionInfoList = exeFile2Version.split(".");
    if((exeFile1VersionInfoList.size() != 4) || (exeFile2VersionInfoList.size() != 4)) {
        qCritical() << "Invalid version string!";
        return -2;
    }

    if(exeFile1VersionInfoList[0].toUInt() > exeFile2VersionInfoList[0].toUInt()) {
        return 1;
    } else if(exeFile1VersionInfoList[0].toUInt() < exeFile2VersionInfoList[0].toUInt()) {
        return -1;
    }

    if(exeFile1VersionInfoList[1].toUInt() > exeFile2VersionInfoList[1].toUInt()) {
        return 1;
    } else if(exeFile1VersionInfoList[1].toUInt() < exeFile2VersionInfoList[1].toUInt()) {
        return -1;
    }

    if(exeFile1VersionInfoList[2].toUInt() > exeFile2VersionInfoList[2].toUInt()) {
        return 1;
    } else if(exeFile1VersionInfoList[2].toUInt() < exeFile2VersionInfoList[2].toUInt()) {
        return -1;
    }

    if(exeFile1VersionInfoList[3].toUInt() > exeFile2VersionInfoList[3].toUInt()) {
        return 1;
    } else if(exeFile1VersionInfoList[3].toUInt() < exeFile2VersionInfoList[3].toUInt()) {
        return -1;
    }


    return 0;

}

void CoreUtilities::msleep(int msec)
{

#ifdef Q_OS_WIN32
    Sleep(msec);
#else
    usleep(msec * 1000);
#endif

}

const QString CoreUtilities::getFileMD5EncodedWithHex(const QString &fileName)
{

    QString md5String = "";

    QByteArray ba;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCritical() << QString("ERROR! Failed to open file '%1'! %2").arg(fileName).arg(file.errorString());
        return md5String;
    }
    ba = file.readAll();
    file.close();

    md5String = QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();

    return md5String;

}


// Richtext simplification filter helpers: Elements to be discarded
static inline bool filterElement(const QStringRef &name)
{
    return name != QStringLiteral("meta") && name != QStringLiteral("style");
}

// Richtext simplification filter helpers: Filter attributes of elements
static inline void filterAttributes(const QStringRef &name,
                                    QXmlStreamAttributes *atts,
                                    bool *paragraphAlignmentFound)
{
    typedef QXmlStreamAttributes::iterator AttributeIt;

    if (atts->isEmpty()) {
        return;
    }

    // No style attributes for <body>
    if (name == QStringLiteral("body")) {
        atts->clear();
        return;
    }

    // Clean out everything except 'align' for 'p'
    if (name == QStringLiteral("p")) {
        for (AttributeIt it = atts->begin(); it != atts->end(); ) {
            if (it->name() == QStringLiteral("align")) {
                ++it;
                *paragraphAlignmentFound = true;
            } else {
                it = atts->erase(it);
            }
        }
        return;
    }
}

// Richtext simplification filter helpers: Check for blank QStringRef.
static inline bool isWhiteSpace(const QStringRef &in)
{
    const int count = in.size();
    for (int i = 0; i < count; i++)
        if (!in.at(i).isSpace()) {
            return false;
        }
    return true;
}

// Richtext simplification filter: Remove hard-coded font settings,
// <style> elements, <p> attributes other than 'align' and
// and unnecessary meta-information.
QString CoreUtilities::simplifyRichTextFilter(const QString &in, bool *isPlainTextPtr)
{
    unsigned elementCount = 0;
    bool paragraphAlignmentFound = false;
    QString out;
    QXmlStreamReader reader(in);
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(false);
    writer.setAutoFormattingIndent(0);

    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartElement:
            elementCount++;
            if (filterElement(reader.name())) {
                const QStringRef name = reader.name();
                QXmlStreamAttributes attributes = reader.attributes();
                filterAttributes(name, &attributes, &paragraphAlignmentFound);
                writer.writeStartElement(name.toString());
                if (!attributes.isEmpty()) {
                    writer.writeAttributes(attributes);
                }
            } else {
                reader.readElementText(); // Skip away all nested elements and characters.
            }
            break;
        case QXmlStreamReader::Characters:
            if (!isWhiteSpace(reader.text())) {
                writer.writeCharacters(reader.text().toString());
            }
            break;
        case QXmlStreamReader::EndElement:
            writer.writeEndElement();
            break;
        default:
            break;
        }
    }
    // Check for plain text (no spans, just <html><head><body><p>)
    if (isPlainTextPtr) {
        *isPlainTextPtr = !paragraphAlignmentFound && elementCount == 4u;    //
    }
    return out;
}

quint64 CoreUtilities::timeGet()
{
#ifdef Q_OS_WIN32
    return timeGetTime ();
#else
    struct timeval timeVal;
    gettimeofday (& timeVal, NULL);
    return timeVal.tv_sec * 1000 + timeVal.tv_usec / 1000;
#endif

}


























} //namespace HEHUI


