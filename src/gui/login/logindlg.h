#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>

//#include "../../core/singleton.h"
#include "../../core/userbase.h"
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
    LoginDlg(UserBase *user, const QString &windowTitle = "", bool hashPassword = true, QWidget *parent = 0);
    ~LoginDlg();


    //void setUser(UserBase *user);
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

    UserBase *user;
    bool hashPassword;





};

} //namespace HEHUI

#endif
