/*
 ****************************************************************************
 * imageresourcebase.h
 *
 * Created on: 2009-11-6
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
 * Last Modified on: 2010-05-14
 * Last Modified by: 贺辉
 ***************************************************************************
 */





#ifndef IMAGERESOURCEBASE_H_
#define IMAGERESOURCEBASE_H_

#include <QObject>
#include <QIcon>
#include <QString>

#include "guilib.h"


namespace HEHUI {

class GUI_LIB_API ImageResourceBase :public QObject{
    Q_OBJECT
public:
    ImageResourceBase(QObject *parent = 0);
    virtual ~ImageResourceBase();

    static QIcon createIcon(const QString &iconFileName, const QString &defaultIconName = QString(), QIcon::Mode mode = QIcon::Normal);
    static QIcon emptyIcon();


#ifdef Q_OS_WIN32

    static QImage WinHBITMAPToImage(HBITMAP bitmap, bool noAlpha = true);
//    static HBITMAP PixmapToWinHBITMAP(const QImage &image, bool noAlpha = true);

    static HBITMAP GetScreenshotBmp();
    static QImage screenshot();

#endif







};

} //namespace HEHUI

#endif /* IMAGERESOURCEBASE_H_ */
