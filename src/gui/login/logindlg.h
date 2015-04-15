#ifndef LOGIN_H
#define LOGIN_H

#include <QSqlError>
#include <QDialog>
#include <QMessageBox>

#include "../databaseconnecter/databaseconnecterdialog.h"
#include "../../core/singleton.h"
#include "../../core/user.h"
#include "../guilib.h"


namespace Ui {
class LoginDlgUI;
}

namespace HEHUI {

class GUI_LIB_API LoginDlg : public QDialog /*, public Singleton<LoginDlg>*/
{
    Q_OBJECT
    //friend class Singleton<LoginDlg>;

public:
    LoginDlg(User *user, const QString &windowTitle = "", QWidget *parent = 0);
    ~LoginDlg();


    void setUser(User *user);
    void setErrorMessage(const QString &message);

private:


protected:
    void closeEvent(QCloseEvent * event);
    void keyPressEvent(QKeyEvent *);
    void languageChange();

signals:
    void signalLogin();
    void signalUserButtonClicked();
    void signalKeyButtonClicked();
    void signalModifySettings();
    void signalAbort();

private slots:
    void on_toolButtonUser_clicked();
    void on_toolButtonKey_clicked();

    void on_pushButtonSettings_clicked();
    void on_pushButtonLogin_clicked();
    void on_pushButtonCancel_clicked();
    void on_pushButtonAbort_clicked();



private:
    Ui::LoginDlgUI *ui;

    User *user;

    inline QString userID() const;
    inline QString passWord() const;

};

} //namespace HEHUI

#endif
