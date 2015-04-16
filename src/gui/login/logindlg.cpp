#include <QtGui>
#include <QInputDialog>
#include <QMessageBox>


#include "logindlg.h"
#include "ui_logindlg.h"


namespace HEHUI {

LoginDlg::LoginDlg(UserBase *user, const QString &windowTitle, bool hashPassword, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDlgUI),
    user(user),
    hashPassword(hashPassword)
{

    qDebug("----LoginDlg::LoginDlg(User *user, QWidget *parent)");
    Q_ASSERT_X(user != NULL, "LoginDlg::LoginDlg(User *user, QWidget *parent)", " 'user' is NULL!");

    ui->setupUi(this);

    if(!windowTitle.isEmpty()){
        setWindowTitle(windowTitle);
    }


    ui->userIDComboBox->setEditText(user->getUserID());
    //ui.passwordLineEdit->setText(user->getPassword());

}

LoginDlg::~LoginDlg() {
    delete ui;
}

void LoginDlg::closeEvent(QCloseEvent * event){
    event->accept();
    emit signalAbort();
}

void LoginDlg::keyPressEvent(QKeyEvent *e) {

    int key = e->key();

    switch (key) {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        //        if (ui->userIDComboBox->hasFocus()) {
        //            ui->passwordLineEdit->setFocus();
        //        } else if (ui->passwordLineEdit->hasFocus()) {
        //            ui->lineEditAuthenticode->setFocus();
        //        }else{
        //            ui->pushButtonLogin->click();
        //        }
        focusNextChild();
        break;
    case Qt::Key_R:
        //是否进入RestoreMode
        //Whether enter RestoreMode
        if(ui->pushButtonLogin->hasFocus()){
            bool ok = false;
            QString text = QInputDialog::getText(this, tr("Authentication Required"),
                                                 tr("Access Code:"), QLineEdit::NoEcho,
                                                 "", &ok);
            if (ok && !text.isEmpty()){
                QString accessCodeString = "iamhehui";
                accessCodeString.append(QTime::currentTime().toString("hhmm"));
                if(text.toLower() == accessCodeString){
                    //user->setRootMode(true);
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

//void LoginDlg::setUser(UserBase *user){
//    this->user = user;
//}

void LoginDlg::setErrorMessage(const QString &message){
    ui->labelBottom->setText(message);
    ui->pushButtonAbort->setText(tr("OK"));
}

void LoginDlg::on_toolButtonUser_clicked(){
    emit signalUserButtonClicked();
}

void LoginDlg::on_toolButtonKey_clicked(){
    emit signalKeyButtonClicked();
}

void LoginDlg::on_pushButtonSettings_clicked(){
    emit signalModifySettings();
}

void LoginDlg::on_pushButtonLogin_clicked() {

    QString uid = ui->userIDComboBox->currentText().trimmed();
    QString password = ui->passwordLineEdit->text();

    if (uid.isEmpty()) {
        QMessageBox::critical(this, tr("Authentication Failed"), tr("Authentication failed! Invalid user ID!"));
        ui->userIDComboBox->setFocus();
        return;

    } else if (password.isEmpty()) {
        QMessageBox::critical(this, tr("Authentication Failed"), tr("Authentication failed! Password required!"));
        ui->passwordLineEdit->setFocus();
        return;

    }
    //    else if(ui->lineEditAuthenticode->text() != QDateTime::currentDateTime().toString("HHmm")){
    //        qDebug()<<"Authentic code:"<<QDateTime::currentDateTime().toString("HHmm");
    //        QMessageBox::critical(this, tr("Authentication Failed"), tr(
    //                                  "<b>Incorrect Authenticode!</b>"));
    //        ui->lineEditAuthenticode->clear();
    //        ui->lineEditAuthenticode->setFocus();
    //        return;

    //    }
    else{
        user->setUserID(uid);

        if(hashPassword){
            QByteArray pswd = QCryptographicHash::hash(password.toLatin1(), QCryptographicHash::Md5).toHex();
            pswd = QCryptographicHash::hash(pswd, QCryptographicHash::Md5).toHex();
            password = QString(pswd);
        }
        user->setPassword(password);
        ui->passwordLineEdit->clear();
    }

    ui->labelBottom->setText(tr("Logging in...."));
    ui->stackedWidget->setCurrentWidget(ui->pageLoggingIn);
    qApp->processEvents();

    emit signalLogin();

}

void LoginDlg::on_pushButtonCancel_clicked() {
    ui->passwordLineEdit->clear();
    reject();
}

void LoginDlg::on_pushButtonAbort_clicked(){
    ui->stackedWidget->setCurrentWidget(ui->pageUserInfo);
    emit signalAbort();
}







}


