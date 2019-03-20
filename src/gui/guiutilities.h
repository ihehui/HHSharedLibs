#ifndef GUIUTILITIES_H
#define GUIUTILITIES_H

#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QStyle>
#include <QTranslator>
#include <QSystemTrayIcon>
#include <QSettings>

#include "guilib.h"


namespace HEHUI {


class GUI_LIB_API GUIUtilities : public QObject
{
    Q_OBJECT
public:
    explicit GUIUtilities(const QString &settingsFile, QObject *parent = nullptr);

    enum WindowPosition {
        CENTER = 0,
        TOP_CENTER,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT
    };
    static void moveWindow(QWidget *widget, WindowPosition positon);

    static void updateSystemTray(QSystemTrayIcon *tray, const QString &toolTip, const QIcon &icon, QMenu *menu = 0);
    static void showSystemTrayMsg(QSystemTrayIcon *tray, const QString &title, const QString &message, QSystemTrayIcon::MessageIcon iconType = QSystemTrayIcon::Information, int secondsTimeoutHint = 3 );


signals:
    void signalStyleChanged(const QString &style, bool usingStylesPalette);
    void signalLanguageChanged(const QString &language);


public slots:
    void initStyle();
    void setupStyleMenu(QMenu *styleMenu, QWidget *topWidget);
    void setPreferedStyle(const QString &style);
    QString getPreferedStyle();

    void initLanguage();
    ////preferedLanguage: a string of the form "language_country", where language is a lowercase, two-letter ISO 639 language code, and country is an uppercase, two- or three-letter ISO 3166 country code.
    void setupLanguageMenu(QMenu *languageMenu);
    void setPreferedLanguage(const QString &language);
    QString getPreferedLanguage() const;
    QStringList translationFileDirList() const;
    void setTranslationFileDirList(const QStringList &dirList);


private slots:
    void setUseStylesPalette(bool checked);
    bool isUsingStylesPalette();

    void changeStyle(QAction *styleAction);
    void changePalette();
    void editPalette();
    void saveCurrentPalette();
    QPalette loadSavedPalette();
    void updatePaletteEditorAction(bool useStylesPalette);

    void changeLanguage(QAction *languageAction);
    void changeLanguage(const QString &language);

    void topWidgetDestroyed();

private:
    QString m_settingsFile;
    QWidget *m_topWidget;

    QAction *m_paletteEditorAction;


};

} //namespace HEHUI

#endif // GUIUTILITIES_H
