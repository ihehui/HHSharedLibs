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
#include <QStyleFactory>
#include <QStyle>
#include <QPalette>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QTranslator>
#include <QDebug>

#include "settingsbase.h"


#ifndef LANGUAGE_FILE_DIR
    #define LANGUAGE_FILE_DIR	"translations"
#endif


namespace HEHUI
{

//static QPalette *sysPalette = 0;
//QList<QTranslator *> SettingsBase::translators = QList<QTranslator *>();


SettingsBase::SettingsBase(const QString fileBaseName, const QString fileDirPath, QObject *parent )
    : SettingsCore(fileBaseName, fileDirPath, parent )
{

//    if(!sysPalette){
//        sysPalette = new QPalette();
//    }
}

SettingsBase::~SettingsBase()
{
    sync();
}

//void SettingsBase::loadStyleSettings()
//{
//    QApplication::setStyle(getStyle());
//    changePalette();
//}


//void SettingsBase::setupStyleMenu(QMenu *styleMenu)
//{
//    if(!styleMenu){return;}

//    QString preferedStyle = getStyle();
//    QString defaultStyle = "";
//    QStringList styles = QStyleFactory::keys();
//    if(styles.contains("Oxygen", Qt::CaseInsensitive)){
//        defaultStyle = "Oxygen";
//    }else if(styles.contains("Breeze", Qt::CaseInsensitive)){
//        defaultStyle = "Breeze";
//    }else if(styles.contains("QtCurve", Qt::CaseInsensitive)){
//        defaultStyle = "QtCurve";
//    }else if(styles.contains("Plastique", Qt::CaseInsensitive)){
//        defaultStyle = "Plastique";
//    }else if(styles.contains("Fusion", Qt::CaseInsensitive)){
//        defaultStyle = "Fusion";
//    }else if(!styles.isEmpty()){
//        defaultStyle = styles.first();
//    }

//    QActionGroup *actGroup = new QActionGroup(styleMenu);
//    QAction *action = styleMenu->addAction(tr("Default(%1)").arg(defaultStyle));
//    action->setData(defaultStyle);
//    action->setCheckable(true);
//    action->setChecked(true);
//    actGroup->addAction(action);

//    foreach (QString styleKey, styles) {
//        action = styleMenu->addAction(styleKey);
//        action->setCheckable(true);
//        action->setData(styleKey);
//        actGroup->addAction(action);

//        if(preferedStyle.toLower() == styleKey.toLower()){
//            action->setChecked(true);
//        }
//    }

//    connect(actGroup, SIGNAL(triggered(QAction *)), this, SLOT(changeStyle(QAction *)));


//    bool useStylesPalette = isUsingStylesPalette();
//    styleMenu->addSeparator();
//    action = styleMenu->addAction(tr("Use Style's Palette"));
//    action->setCheckable(true);
//    action->setChecked(useStylesPalette);
//    action->setData("");
//    connect(action, SIGNAL(triggered(bool)), this, SLOT(setUseStylesPalette(bool)));

//}


//void SettingsBase::setStyle(const QString &style)
//{
//    setValue("UI/Style", style);
//}

//QString SettingsBase::getStyle() const
//{

//#ifdef Q_OS_WIN
//    return value("UI/Style", QString("fusion")).toString();
//#endif

//    return value("UI/Style", "").toString();

//}

//void SettingsBase::setUseStylesPalette(bool checked)
//{
//    setValue("UI/UseStylesPalette", checked);
//    sync();

//    //changePalette();
//}

//bool SettingsBase::isUsingStylesPalette()
//{
//    return value("UI/UseStylesPalette", true).toBool();
//}

//void SettingsBase::changePalette()
//{
//    QPalette palette;
//    if(isUsingStylesPalette()){
//        palette = QApplication::style()->standardPalette();
//    }else{
//        palette = *sysPalette;
//    }
//    //QWidget::setPalette(palette);
//    QApplication::setPalette(palette);
//    //update();
//}

//void SettingsBase::changeStyle(QAction *styleAction)
//{
//    if(!styleAction){return;}

//    QString style = styleAction->data().toString();
//    qApp->setStyle(style);
//    if(style.toLower() != styleAction->text().toLower().remove("&")){
//        setStyle("");
//    }else{
//        setStyle(style);
//    }

//    changePalette();
//}

//void SettingsBase::setupLanguageMenu(QMenu *languageMenu, const QString &translationFilesDir){
//    if(!languageMenu){return;}

//    m_translationFilesDir = translationFilesDir;
//    if(m_translationFilesDir.trimmed().isEmpty()){
//        m_translationFilesDir = QApplication::applicationDirPath() + QDir::separator () + QString(LANGUAGE_FILE_DIR);
//    }

//    QHash<QString /*Local Name*/, QString /*Language Name*/ > languagesHash;
//    languagesHash.insert("en_US", "English");
//    languagesHash.insert("zh_CN", QString::fromUtf8("\347\256\200\344\275\223\344\270\255\346\226\207"));
//    QString curSystemLanguage = QLocale::languageToString(QLocale::system().language());
//    QString preferedLanguage = getLanguage();

//    QStringList translations = availableTranslationLanguages(m_translationFilesDir);
//    if(preferedLanguage.isEmpty()){
//        preferedLanguage = curSystemLanguage;
//    }

//    //If the preferred language file is not found, "en_US" will be used.
//    if(!translations.contains(preferedLanguage, Qt::CaseInsensitive)){
//        preferedLanguage = "en_US";
//    }

//    QActionGroup *actGroup = new QActionGroup(languageMenu);
////    QAction *defaultLanguageAction = languageMenu->addAction(tr("Default"));
////    defaultLanguageAction->setData(defaultLanguage);
////    defaultLanguageAction->setCheckable(true);
////    actGroup->addAction(defaultLanguageAction);
////    if(preferedLanguage.trimmed().isEmpty()){
////        defaultLanguageAction->setChecked(true);
////    }

//    QAction *engAction = languageMenu->addAction("English");
//    engAction->setData("en_US");
//    engAction->setCheckable(true);
//    actGroup->addAction(engAction);
//    if(preferedLanguage == "en_US"){
//        engAction->setChecked(true);
//    }


//    //Make item for each language file
//    for (int i = 0; i < translations.size(); i++){
//        QString translationLanguage = translations[i];
//        QLocale local(translationLanguage);
//        QString LanguageName = languagesHash.value(translationLanguage);
//        if(LanguageName.isEmpty()){
//            LanguageName = QLocale::languageToString(local.language());
//        }

//        QString regionName = QLocale::countryToString(local.country());
//        QAction *action = languageMenu->addAction(QString("%1(%2)").arg(LanguageName).arg(regionName));
//        action->setData(translationLanguage);
//        action->setCheckable(true);
//        actGroup->addAction(action);

//        if(preferedLanguage == translationLanguage){
//            action->setChecked(true);
//        }

//    }

//    connect(actGroup, SIGNAL(triggered(QAction *)), this, SLOT(changeLanguage(QAction *)));

//}

//void SettingsBase::changeLanguage(QAction *languageAction)
//{
//    if(!languageAction){return;}

//    QString language = languageAction->data().toString();
//    changeLangeuage(m_translationFilesDir, language);
//    setLanguage(language);
//}

//void SettingsBase::setLanguage(const QString &language)
//{
//    setValue("UI/Language", language);
//}

//QString SettingsBase::getLanguage() const
//{
//    //return value("UI/Language",QString("en_US")).toString();
//    return value("UI/Language", QLocale::system().name()).toString();
//}

//QStringList SettingsBase::availableTranslationLanguages(const QString &translationFilesDir){

//    //Search language files
//    QDir dir(translationFilesDir);
//    QStringList fileNames = dir.entryList(QStringList("*.qm"));
//    qDebug()<<"Available Language Files: "<<fileNames.join(",");

//    if (fileNames.isEmpty()) {
//        return QStringList();
//    }

//    QStringList translationLanguages;
//    foreach(QString file, fileNames){
//        file.truncate(file.lastIndexOf(".qm", -1, Qt::CaseInsensitive));
//        QString translationLanguageName = file.right(5);
//        //qDebug()<<"~~translationLanguageName:"<<translationLanguageName;
//        if((translationLanguageName.size() == 5) && (!translationLanguages.contains(translationLanguageName))){
//            translationLanguages.append(translationLanguageName);
//        }
//    }

//    return translationLanguages;
//}

//bool SettingsBase::changeLangeuage(const QString &translationFilesDir, const QString &qmLocale){

//    qDebug()<<"Locale System Name:"<< QLocale::system().name();

//    if(qmLocale.size() != 5){
//        qCritical()<<"Invalid local name! It should be a string of the form 'language_country', where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase, two-letter ISO 3166 country code.";
//        return false;
//    }

//    foreach(QTranslator *translator, translators){
//        qApp->removeTranslator(translator);
//        delete translator;
//        translator = 0;
//    }
//    translators.clear();

//    QStringList filters;
//    filters << QString("*" + qmLocale + ".qm");
//    foreach(QString file, QDir(translationFilesDir).entryList(filters, QDir::Files|QDir::System|QDir::Hidden))
//    {
//        qDebug()<<"Loading language file:"<<file;
//        QTranslator *translator = new QTranslator();
//        if(translator->load(file, translationFilesDir)){
//            qApp->installTranslator(translator);
//            translators.append(translator);
//        }else{
//            delete translator;
//            translator = 0;
//            qCritical()<<"ERROR! Loading language file failed:"<<file;
//        }

//    }

//    return translators.size();
//}

void SettingsBase::setHideOnClose(bool hideOnClose)
{
    setValue("MainWindow/HideOnClose", hideOnClose);
}

bool SettingsBase::getHideOnClose()
{
    return value("MainWindow/HideOnClose", true).toBool();
}

void SettingsBase::restoreState( QMainWindow *mw)
{
    if ( !mw ) {
        return;
    }
    mw->restoreState( value( "MainWindow/State" ).toByteArray() );
    QPoint p = value( "MainWindow/Position" ).toPoint();
    QSize s = value( "MainWindow/Size" ).toSize();
    if ( !p.isNull() && !s.isNull() ) {
        mw->resize( s );
        mw->move( p );
    }

    if ( value( "MainWindow/Maximized", false ).toBool() ) {
        mw->showMaximized();
    }


    if ( value( "MainWindow/Hidden", false ).toBool() ) {
        mw->hide();
    }

}

void SettingsBase::saveState( QMainWindow *mw)
{
    if ( !mw ) {
        return;
    }

    setValue( "MainWindow/Maximized", mw->isMaximized() );
    setValue( "MainWindow/Position", mw->pos() );
    setValue( "MainWindow/Size", mw->size() );
    setValue( "MainWindow/State", mw->saveState() );
    setValue( "MainWindow/Hidden", mw->isHidden() );

}

void SettingsBase::setRestoreWindowStateOnStartup(bool restore)
{
    setValue( "MainWindow/RestoreWindowStateOnStartup", restore );

}

bool SettingsBase::getRestoreWindowStateOnStartup()
{
    return value("MainWindow/RestoreWindowStateOnStartup", false).toBool();

}













} //namespace HEHUI


