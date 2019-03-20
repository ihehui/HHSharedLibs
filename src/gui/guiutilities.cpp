#include "guiutilities.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QStyleFactory>
#include <QDir>
#include <QDebug>


#ifndef LANGUAGE_FILE_DIR
    #define LANGUAGE_FILE_DIR	"translations"
#endif



namespace HEHUI {


static QPalette *sysPalette = 0;

QList<QTranslator *> GUIUtilities::translators = QList<QTranslator *>();

GUIUtilities::GUIUtilities(QObject *parent)
    : QObject(parent)
{
    if(!sysPalette){
        sysPalette = new QPalette();
    }
}

void GUIUtilities::moveWindow(QWidget *widget, WindowPosition positon)
{
    // Get the size of screen
    QDesktopWidget *desktop = QApplication::desktop();
    QRect rect = desktop->availableGeometry(widget);
    int desktopWidth = rect.width();
    int desktopHeight = rect.height();

    int windowWidth = widget->frameGeometry().width();
    int windowHeight = widget->frameGeometry().height();

    //move the window
    switch (positon) {
    case CENTER:
        widget->move((desktopWidth - windowWidth) / 2, (desktopHeight - windowHeight) / 2);
        break;

    case TOP_CENTER:
        widget->move((desktopWidth - windowWidth) / 2, 0);
        break;

    case TOP_RIGHT:
        widget->move((desktopWidth - windowWidth), 0);
        break;

    case BOTTOM_LEFT:
        widget->move(0, (desktopHeight - windowHeight));
        break;

    case BOTTOM_CENTER:
        widget->move((desktopWidth - windowWidth) / 2, (desktopHeight - windowHeight));
        break;

    case BOTTOM_RIGHT:
        widget->move((desktopWidth - windowWidth), (desktopHeight - windowHeight));
        break;

    default:
        widget->move((desktopWidth - windowWidth) / 2, (desktopHeight - windowHeight) / 2);

    }

}

QStringList GUIUtilities::availableTranslationLanguages(const QString &translationFilesDir){

    //Search language files
    QDir dir(translationFilesDir);
    QStringList fileNames = dir.entryList(QStringList("*.qm"));
    qDebug()<<"Available language files: "<<fileNames.join(",");

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

bool GUIUtilities::changeLangeuage(const QString &translationFilesDir, const QString &qmLocale)
{
    qDebug() << "Locale System Name:" << QLocale::system().name();

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

void GUIUtilities::setupStyleMenu(QMenu *styleMenu, const QString &preferedStyle, bool useStylesPalette)
{
    if(!styleMenu){return;}

    QString defaultStyle = "";
    QStringList styles = QStyleFactory::keys();
    if(styles.contains("Oxygen", Qt::CaseInsensitive)){
        defaultStyle = "Oxygen";
    }else if(styles.contains("Breeze", Qt::CaseInsensitive)){
        defaultStyle = "Breeze";
    }else if(styles.contains("QtCurve", Qt::CaseInsensitive)){
        defaultStyle = "QtCurve";
    }else if(styles.contains("Plastique", Qt::CaseInsensitive)){
        defaultStyle = "Plastique";
    }else if(styles.contains("Fusion", Qt::CaseInsensitive)){
        defaultStyle = "Fusion";
    }else if(!styles.isEmpty()){
        defaultStyle = styles.first();
    }

    QActionGroup *actGroup = new QActionGroup(styleMenu);
    QAction *action = styleMenu->addAction(tr("Default(%1)").arg(defaultStyle));
    action->setData(defaultStyle);
    action->setCheckable(true);
    action->setChecked(true);
    actGroup->addAction(action);

    foreach (QString styleKey, styles) {
        action = styleMenu->addAction(styleKey);
        action->setCheckable(true);
        action->setData(styleKey);
        actGroup->addAction(action);

        if(preferedStyle.toLower() == styleKey.toLower()){
            action->setChecked(true);
        }
    }

    connect(actGroup, SIGNAL(triggered(QAction *)), this, SLOT(changeStyle(QAction *)));


    styleMenu->addSeparator();
    action = styleMenu->addAction(tr("Use Style's Palette"));
    action->setCheckable(true);
    action->setChecked(useStylesPalette);
    action->setData("");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(setUseStylesPalette(bool)));

    m_usingStylesPalette = useStylesPalette;
}

void GUIUtilities::changeStyle(QAction *styleAction)
{
    if(!styleAction){return;}

    QString style = styleAction->data().toString();
    if(style.toLower() != styleAction->text().toLower().remove("&")){
        qApp->setStyle(style);
        setStyle("");
    }else{
        setStyle(style);
    }
}

void GUIUtilities::setStyle(const QString &style)
{
    qApp->setStyle(style);
    emit signalStyleChanged(style);

    changePalette();
}

void GUIUtilities::setUseStylesPalette(bool checked)
{
    m_usingStylesPalette = checked;
    changePalette();

    emit signalUsingStylesPaletteChanged(checked);
}

void GUIUtilities::changePalette()
{
    QPalette palette;
    if(m_usingStylesPalette){
        palette = QApplication::style()->standardPalette();
    }else{
        palette = *sysPalette;
    }
    //QWidget::setPalette(palette);
    QApplication::setPalette(palette);
    //update();
}

void GUIUtilities::setupLanguageMenu(QMenu *languageMenu, const QString &preferedLanguage, const QString &translationFilesDir){
    if(!languageMenu){return;}

    m_translationFilesDir = translationFilesDir;
    if(m_translationFilesDir.trimmed().isEmpty()){
        m_translationFilesDir = QApplication::applicationDirPath() + QDir::separator () + QString(LANGUAGE_FILE_DIR);
    }


    QHash<QString /*Local Name*/, QString /*Language Name*/ > languagesHash;
    languagesHash.insert("en_US", "English");
    languagesHash.insert("zh_CN", QString::fromUtf8("\347\256\200\344\275\223\344\270\255\346\226\207"));
    qDebug()<<"-----"<<QLocale("fr").nativeCountryName()<<"   "<<QLocale("fr").nativeLanguageName();
    QString curSystemLanguage = QLocale::languageToString(QLocale::system().language());
    QStringList translations = availableTranslationLanguages(m_translationFilesDir);

    QString language = preferedLanguage;
    if(language.isEmpty()){
        language = curSystemLanguage;
    }

    //If the preferred language file is not found, "en_US" will be used.
    if(!translations.contains(language, Qt::CaseInsensitive)){
        language = "en_US";
    }

    QActionGroup *actGroup = new QActionGroup(languageMenu);
//    QAction *defaultLanguageAction = languageMenu->addAction(tr("Default"));
//    defaultLanguageAction->setData(defaultLanguage);
//    defaultLanguageAction->setCheckable(true);
//    actGroup->addAction(defaultLanguageAction);
//    if(preferedLanguage.trimmed().isEmpty()){
//        defaultLanguageAction->setChecked(true);
//    }

    QAction *engAction = languageMenu->addAction("English");
    engAction->setData("en_US");
    engAction->setCheckable(true);
    actGroup->addAction(engAction);
    if(language == "en_US"){
        engAction->setChecked(true);
    }


    //Make item for each language file
    for (int i = 0; i < translations.size(); i++){
        QString translationLanguage = translations[i];
        QLocale local(translationLanguage);
        QString languageName = languagesHash.value(translationLanguage);
        if(languageName.isEmpty()){
            languageName = QLocale::languageToString(local.language());
        }
        QString regionName = QLocale::countryToString(local.country());
#if QT_VERSION >= 0x048000
        languageName = local.nativeLanguageName();
        regionName = local.nativeCountryName();
#endif

        QAction *action = languageMenu->addAction(QString("%1(%2)").arg(languageName).arg(regionName));
        action->setData(translationLanguage);
        action->setCheckable(true);
        actGroup->addAction(action);

        if(language == translationLanguage){
            action->setChecked(true);
        }

    }

    connect(actGroup, SIGNAL(triggered(QAction *)), this, SLOT(changeLanguage(QAction *)));

}

void GUIUtilities::changeLanguage(QAction *languageAction)
{
    if(!languageAction){return;}

    QString language = languageAction->data().toString();
    //QString translationFilesDir = QApplication::applicationDirPath() + QDir::separator () + QString(LANGUAGE_FILE_DIR);
    setLanguage(language);
}

void GUIUtilities::setLanguage(const QString &language)
{
    changeLangeuage(m_translationFilesDir, language);
    emit signalLanguageChanged(language);
}

void GUIUtilities::updateSystemTray(QSystemTrayIcon *tray, const QString &toolTip, const QIcon &icon, QMenu *menu)
{
    if (!tray) {
        return;
    }

    tray->setIcon(icon);
    tray->setToolTip(toolTip);

    QMenu *trayMenu = tray->contextMenu();

    if (!trayMenu) {
        tray->setContextMenu(menu);
    } else {
        trayMenu->addMenu(menu);
    }

}

void GUIUtilities::showSystemTrayMsg(QSystemTrayIcon *tray, const QString &title,
                                   const QString &message, QSystemTrayIcon::MessageIcon iconType,
                                   int secondsTimeoutHint)
{
    if (!tray) {
        return;
    }

    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(iconType);
    tray->showMessage(title, message, icon, secondsTimeoutHint * 1000);
}




}
