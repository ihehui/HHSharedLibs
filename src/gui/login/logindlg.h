#ifndef LOGIN_H
#define LOGIN_H

#include <QSqlError>
#include <QDialog>
#include <QMessageBox>

#include "ui_logindlg.h"
#include "../databaseconnecter/databaseconnecterdialog.h"
#include "../../core/singleton.h"
#include "../../core/user.h"


namespace Ui {
class LoginDlgUI;
}

namespace HEHUI {

class LoginDlg : public QDialog /*, public Singleton<LoginDlg>*/
{
    Q_OBJECT
    //friend class Singleton<LoginDlg>;

public:
    LoginDlg(User *user, const QString &windowTitle = "", QWidget *parent = 0);
    ~LoginDlg();


    void setUser(User *user);

private:


protected:
    void keyPressEvent(QKeyEvent *);
    void languageChange();

signals:
    void signalUserButtonClicked();
    void signalKeyButtonClicked();

    void signalModifySettings();

private slots:
    void on_toolButtonUser_clicked();
    void on_toolButtonKey_clicked();

    void on_pushButtonSettings_clicked();
    void on_pushButtonLogin_clicked();
    void on_pushButtonCancel_clicked();



private:
    Ui::LoginDlgUI *ui;

    User *user;

    inline QString userID() const;
    inline QString passWord() const;

};

} //namespace HEHUI

#endif
