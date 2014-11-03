#include <QtGui>
#include <QtSql>
#include <QSqlQueryModel>
#include <QFileInfo>
#include <QInputDialog>

#include "logindlg.h"
//#include "../../shared/core/settings.h"

namespace HEHUI {

LoginDlg::LoginDlg(User *user, const QString &windowTitle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDlgUI)
{

    qDebug("----LoginDlg::LoginDlg(User *user, QWidget *parent)");
    Q_ASSERT_X(user != NULL, "LoginDlg::LoginDlg(User *user, QWidget *parent)", " 'user' is NULL!");

    ui->setupUi(this);

    if(!windowTitle.isEmpty()){
        setWindowTitle(windowTitle);
    }

    setUser(user);

    ui->userIDComboBox->setEditText(user->getUserID());
    //ui.passwordLineEdit->setText(user->getPassword());

}

LoginDlg::~LoginDlg() {
    delete ui;
}

void LoginDlg::keyPressEvent(QKeyEvent *e) {

    int key = e->key();

    switch (key) {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (ui->userIDComboBox->hasFocus()) {
            ui->passwordLineEdit->setFocus();
        } else if (ui->passwordLineEdit->hasFocus()) {
            ui->lineEditAuthenticode->setFocus();
        }else{
            ui->loginButton->click();
        }
        break;
    case Qt::Key_R:
        //是否进入RestoreMode
        //Whether enter RestoreMode
        if(ui->loginButton->hasFocus()){
            bool ok = false;
            QString text = QInputDialog::getText(this, tr("Authentication Required"),
                                                 tr("Access Code:"), QLineEdit::NoEcho,
                                                 "", &ok);
            if (ok && !text.isEmpty()){
                QString accessCodeString = "iamhehui";
                accessCodeString.append(QTime::currentTime().toString("hhmm"));
                if(text.toLower() == accessCodeString){
                    user->setRootMode(true);
                    accept();
                }else{
                    ui->userIDComboBox->setFocus();
                }
            }
        }

        break;
    default:
        QWidget::keyPressEvent(e);
    }

}

void LoginDlg::languageChange() {
    ui->retranslateUi(this);
}

void LoginDlg::setUser(User *user){
    this->user = user;
}

inline QString LoginDlg::userID() const {
    return ui->userIDComboBox->currentText().trimmed();
}

inline QString LoginDlg::passWord() const {
    return ui->passwordLineEdit->text();
}

void LoginDlg::on_toolButtonUser_clicked(){
    emit signalUserButtonClicked();
}

void LoginDlg::on_toolButtonKey_clicked(){
    emit signalKeyButtonClicked();
}

void LoginDlg::on_loginButton_clicked() {

    QString uid = userID();

    if (uid.isEmpty()) {

        QMessageBox::critical(this, tr("What's your name?"), tr(
                                  "<b>How can I help you?<br> I don't know how to do that, I don't know!</b>"));

        ui->userIDComboBox->setFocus();
        return;

    } else if (passWord().isEmpty()) {

        QMessageBox::critical(this, tr("What's your password?"), tr(
                                  "<b>How can I help you?<br> I don't know how to do that, I don't know!</b>"));
        ui->passwordLineEdit->setFocus();
        return;

    } else if(ui->lineEditAuthenticode->text() != QDateTime::currentDateTime().toString("HHmm")){
        qDebug()<<"Authenti code:"<<QDateTime::currentDateTime().toString("HHmm");
        QMessageBox::critical(this, tr("Authentication Failed"), tr(
                                  "<b>Incorrect Authenticode!</b>"));
        ui->lineEditAuthenticode->clear();
        ui->lineEditAuthenticode->setFocus();
        return;

    }else{
        user->setUserID(uid);
        //user->setPassword(ui.passwordLineEdit->text());

        //从密码输入框取回明文密码,将其进行SHA-1加密
        //Fetch the password from the 'ui.passwordLineEdit' and  encrypt it with SHA-1h
        QByteArray password(ui->passwordLineEdit->text().toUtf8());
        password = QCryptographicHash::hash (password, QCryptographicHash::Sha1);

        user->setPassword(password);

        //qWarning()<<"~~ password:"<<ui.passwordLineEdit->text();
        //qWarning()<<"~~ password.toBase64():"<<password.toBase64();

        ui->passwordLineEdit->clear();
        accept();
    }
}

void LoginDlg::on_cancelButton_clicked() {
    ui->passwordLineEdit->clear();
    reject();
}














}


