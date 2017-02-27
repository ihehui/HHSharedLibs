/*
 ****************************************************************************
 * progressdlg.h
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



#ifndef PROGRESSDLG_H
#define PROGRESSDLG_H

#include <QWidget>
#include <QCloseEvent>


#include "../guilib.h"

namespace Ui
{
class ProgressDlgClass;
}

class GUI_LIB_API ProgressDlg : public QWidget
{
    Q_OBJECT

public:
    ProgressDlg(const QString &jobName = "", QWidget *parent = 0);
    ~ProgressDlg();


    void setJobName(const QString &name);
    void setProgressMessage(const QString &message);
    void setJobProgress(int progressValue);


protected:
//    void closeEvent(QCloseEvent *event);


signals:
    void widgetClosed();


public slots:
    void slotUpdateProgress(const QString &progressMessage, int progressValue = -1);
    void slotUpdateProcessOutput(const QString &message);

    void showDetails(bool show);
    void closeWidget();


private slots:
    void on_toolButtonShowDetails_toggled ( bool checked );


private:
    Ui::ProgressDlgClass *ui;


};

#endif // PROGRESSDLG_H
