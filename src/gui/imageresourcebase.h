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

    static void setBrightnessAndContrast(QImage &image, int brightness, int contrast);
    static void averageBlur(QImage &origin, QImage *dstImage, int kernelWidth = 3, int kernelHeight = 3);
    static void medianBlur(QImage &origin, QImage *newImage, int kernelWidth = 3, int kernelHeight = 3);
    static void guassianBlur(QImage &origin, QImage *dstImage, float sigma);


};

} //namespace HEHUI

#endif /* IMAGERESOURCEBASE_H_ */
