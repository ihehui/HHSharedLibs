#ifndef GUIUTILITIES_H
#define GUIUTILITIES_H

#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QStyle>
#include <QTranslator>
#include <QSystemTrayIcon>

#include "guilib.h"


namespace HEHUI {


class GUI_LIB_API GUIUtilities : public QObject
{
    Q_OBJECT
public:
    explicit GUIUtilities(QObject *parent = nullptr);

    enum WindowPosition {
        CENTER = 0,
        TOP_CENTER,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT
    };
    static void moveWindow(QWidget *widget, WindowPosition positon);

    static QStringList availableTranslationLanguages(const QString &translationFilesDir);
    //Load translation
    static bool changeLangeuage(const QString &translationFilesDir, const QString &qmLocale);





signals:
    void signalStyleChanged(const QString &style);
    void signalUsingStylesPaletteChanged(bool use);

    void signalLanguageChanged(const QString &language);


public slots:
    void setupStyleMenu(QMenu *styleMenu, const QString &preferedStyle, bool useStylesPalette);
    void setStyle(const QString &style);
    void setUseStylesPalette(bool checked);

    void setupLanguageMenu(QMenu *languageMenu, const QString &preferedLanguage, const QString &translationFilesDir = "");
    void setLanguage(const QString &language);

    void updateSystemTray(QSystemTrayIcon *tray, const QString &toolTip, const QIcon &icon, QMenu *menu = 0);
    void showSystemTrayMsg(QSystemTrayIcon *tray, const QString &title, const QString &message, QSystemTrayIcon::MessageIcon iconType = QSystemTrayIcon::Information, int secondsTimeoutHint = 3 );



private slots:
    void changeStyle(QAction *styleAction);
    void changePalette();

    void changeLanguage(QAction *languageAction);


private:
    bool m_usingStylesPalette;

    QString m_translationFilesDir;
    static QList<QTranslator *>translators;


};

} //namespace HEHUI

#endif // GUIUTILITIES_H
