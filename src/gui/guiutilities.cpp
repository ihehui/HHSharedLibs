#include "guiutilities.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QStyleFactory>
#include <QDir>
#include <QDebug>


#ifndef LANGUAGE_FILE_DIR
    #define LANGUAGE_FILE_DIR	"translations"
#endif


#include "HHSharedCore/CoreUtilities"

#include "paletteeditor/paletteeditor.h"
#include "paletteeditor/paletteutilities.h"


namespace HEHUI {


static QPalette *sysPalette = 0;


GUIUtilities::GUIUtilities(const QString &settingsFile, QObject *parent)
    : QObject(parent)
{
    if(!sysPalette){
        sysPalette = new QPalette();
    }

    m_settingsFile = settingsFile;
    if(m_settingsFile.trimmed().isEmpty()){
        m_settingsFile = QCoreApplication::applicationDirPath() + "/settings.ini";
    }

    m_topWidget = 0;
    m_paletteEditorAction = 0;
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

void GUIUtilities::initStyle()
{
    QApplication::setStyle(getPreferedStyle());
    changePalette();
}

void GUIUtilities::setupStyleMenu(QMenu *styleMenu, QWidget *topWidget)
{
    if(!styleMenu || (!topWidget)){
        return;
    }

    m_topWidget = topWidget;
    connect(m_topWidget, SIGNAL(destroyed()), this, SLOT(topWidgetDestroyed()));

    QString preferedStyle = getPreferedStyle();
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


    bool useStylesPalette = isUsingStylesPalette();
    styleMenu->addSeparator();
    QAction *useStylesPaletteAction = styleMenu->addAction(tr("Use Style's Palette"));
    useStylesPaletteAction->setCheckable(true);
    useStylesPaletteAction->setChecked(useStylesPalette);
    useStylesPaletteAction->setData("");
    connect(useStylesPaletteAction, SIGNAL(triggered(bool)), this, SLOT(setUseStylesPalette(bool)));

    styleMenu->addSeparator();
    m_paletteEditorAction = styleMenu->addAction(tr("Palette Editor"));
    m_paletteEditorAction->setCheckable(false);
    m_paletteEditorAction->setEnabled(!useStylesPalette);
    m_paletteEditorAction->setData("PaletteEditor");
    connect(m_paletteEditorAction, SIGNAL(triggered()), this, SLOT(editPalette()));
    connect(useStylesPaletteAction, SIGNAL(triggered(bool)), this, SLOT(updatePaletteEditorAction(bool)));

}

void GUIUtilities::changeStyle(QAction *styleAction)
{
    if(!styleAction){return;}

    QString style = styleAction->data().toString();
    if(style.toLower() != styleAction->text().toLower().remove("&")){
        qApp->setStyle(style);
        setPreferedStyle("");
    }else{
        setPreferedStyle(style);
    }

}

void GUIUtilities::setPreferedStyle(const QString &style)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.setValue("UI/Style", style);

    qApp->setStyle(style);
    emit signalStyleChanged(style, isUsingStylesPalette());
    changePalette();
}

QString GUIUtilities::getPreferedStyle()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
#ifdef Q_OS_WIN
    return value("UI/Style", QString("fusion")).toString();
#endif
    return settings.value("UI/Style", "").toString();
}

void GUIUtilities::setUseStylesPalette(bool checked)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.setValue("UI/UseStylesPalette", checked);

    changePalette();

    emit signalStyleChanged(getPreferedStyle(), checked);
}

bool GUIUtilities::isUsingStylesPalette()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    return settings.value("UI/UseStylesPalette", true).toBool();
}

void GUIUtilities::changePalette()
{
    QPalette palette;
    if(isUsingStylesPalette()){
        palette = QApplication::style()->standardPalette();
    }else{
        //palette = *sysPalette;
        palette = loadSavedPalette();
    }

    QApplication::setPalette(palette);
    if(m_topWidget){
        m_topWidget->setPalette(palette);
        m_topWidget->update();
    }

}

void GUIUtilities::updatePaletteEditorAction(bool useStylesPalette)
{
    QAction *act = qobject_cast<QAction*>(sender());
    if(!act){return;}

    if(m_paletteEditorAction == act){return;}

    m_paletteEditorAction->setEnabled(!useStylesPalette);
}

void GUIUtilities::editPalette()
{
    //QPalette palette = QApplication::style()->standardPalette();
    QPalette palette = QApplication::palette();
    palette = qdesigner_internal::PaletteEditor::getPalette(m_topWidget, palette);
    QApplication::setPalette(palette);
    if(m_topWidget){
        m_topWidget->setPalette(palette);
        m_topWidget->update();
    }

    saveCurrentPalette();

    emit signalStyleChanged(getPreferedStyle(), false);
}

void GUIUtilities::saveCurrentPalette()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    QPalette palette = QApplication::palette();
    PaletteUtilities utilities;
    utilities.exportPalette(&settings, &palette);
}

QPalette GUIUtilities::loadSavedPalette()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    QPalette palette = QApplication::palette();
    PaletteUtilities utilities;
    utilities.importPalette(&settings, &palette);
    return palette;
}

void GUIUtilities::initLanguage()
{
    changeLanguage(getPreferedLanguage());
}

void GUIUtilities::setupLanguageMenu(QMenu *languageMenu)
{
    if(!languageMenu){return;}

    QStringList dirList = translationFileDirList();
    if(dirList.isEmpty()){
        dirList.append(QApplication::applicationDirPath() + QDir::separator () + QString(LANGUAGE_FILE_DIR));
    }
    dirList.removeDuplicates();


    QHash<QString /*Local Name*/, QString /*Language Name*/ > languagesHash;
    languagesHash.insert("en_US", "English");
    languagesHash.insert("zh_CN", QString::fromUtf8("\347\256\200\344\275\223\344\270\255\346\226\207"));
    QString curSystemLanguage = QLocale::languageToString(QLocale::system().language());
    QStringList translations;
    foreach (QString dir, dirList) {
        translations.append(CoreUtilities::availableTranslationLanguages(dir));
    }
    translations.removeDuplicates();

    QString language = getPreferedLanguage();
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
    setPreferedLanguage(language);
}

void GUIUtilities::changeLanguage(const QString &language)
{
    CoreUtilities::changeLangeuage(translationFileDirList(), language);
    emit signalLanguageChanged(language);
}

QStringList GUIUtilities::translationFileDirList() const
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    return settings.value("UI/TranslationFileDir").toStringList();
}

void GUIUtilities::setTranslationFileDirList(const QStringList &dirList)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.setValue("UI/TranslationFileDir", dirList);
}

void GUIUtilities::setPreferedLanguage(const QString &language)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.setValue("UI/Language", language);

    changeLanguage(language);
}

QString GUIUtilities::getPreferedLanguage() const
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    return settings.value("UI/Language", QLocale::system().name()).toString();
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

void GUIUtilities::topWidgetDestroyed()
{
    m_topWidget = 0;
}


}
