/*
 ****************************************************************************
 * progressdlg.cpp
 *
 * Created on: 2009-8-22
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
 * Last Modified on: 2010-05-10
 * Last Modified by: 贺辉
 ***************************************************************************
 */




#include "progressdlg.h"
#include "ui_progressdlg.h"


ProgressDlg::ProgressDlg(const QString &jobName, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ProgressDlgClass)
{
    ui->setupUi(this);

    setWindowFlags(Qt::SplashScreen);

    //ui.toolButtonShowDetails->setChecked(false);
    //ui.textBrowserDetails->hide();
    //on_toolButtonShowDetails_toggled(true);

    setJobName(jobName);

    connect(ui->pushButtonClose, SIGNAL(clicked()), this, SIGNAL(widgetClosed()));

}

ProgressDlg::~ProgressDlg()
{
    delete ui;
}

void ProgressDlg::setJobName(const QString &name)
{
    ui->groupBox->setTitle(name);
}

void ProgressDlg::setProgressMessage(const QString &message)
{
    ui->labelDescription->setText(message);
}

void ProgressDlg::setJobProgress(int progressValue)
{
    ui->progressBar->setValue(progressValue);
}

//void ProgressDlg::closeEvent(QCloseEvent *event){

//    hide();
//    emit widgetClosed();
//    event->accept();
//}

void ProgressDlg::slotUpdateProgress(const QString &progressMessage, int progressValue)
{

    ui->labelDescription->setText(progressMessage);
    ui->textBrowserDetails->append(progressMessage);

    if(progressValue) {
        ui->progressBar->setValue(progressValue);
    }

    QCoreApplication::processEvents();
}

void ProgressDlg::slotUpdateProcessOutput(const QString &message)
{
    ui->textBrowserDetails->append(message);
}

void ProgressDlg::showDetails(bool show)
{
    on_toolButtonShowDetails_toggled(show);
}

void ProgressDlg::closeWidget()
{
    ui->labelDescription->clear();
    ui->progressBar->setValue(0);
    ui->textBrowserDetails->clear();

    hide();
    //showMinimized();

    emit widgetClosed();
}

void ProgressDlg::on_toolButtonShowDetails_toggled ( bool checked )
{
    if(checked) {
        ui->textBrowserDetails->show();
    } else {
        ui->textBrowserDetails->hide();
    }
}




