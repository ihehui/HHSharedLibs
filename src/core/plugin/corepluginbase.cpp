/*
 ****************************************************************************
 * coreinterface.cpp
 *
 * Created on: 2010-4-10
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



#include "corepluginbase.h"


namespace HEHUI {


CorePluginBase::CorePluginBase(QObject *parent)
    :QObject(parent)
{

}


// QString CorePluginBase::name() const
// {
//    return QString(tr("Core Plug-in"));
// }
//
// QString CorePluginBase::version() const
// {
//    return QString(tr("1.0.0"));
// }

QString CorePluginBase::vendor() const
{
    return QString(tr("He Hui"));
}

QString CorePluginBase::author() const
{
    return QString(tr("He Hui"));
}

QString CorePluginBase::url() const
{
    return QString("www.gnu.org");
}

QString CorePluginBase::description() const
{
    return QString(tr("Core Plug-in"));
}

QString CorePluginBase::license() const
{
    return QString(tr("LGPL"));
}


// bool CorePluginBase::isSingle() const
//{
//    return false;
//}






} //namespace HEHUI
