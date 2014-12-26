/*
 ****************************************************************************
 * abstractplugininterface.h
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
  * Last Modified on: 2010-05-12
  * Last Modified by: 贺辉
  ***************************************************************************
*/




#ifndef ABSTRACTPLUGININTERFACE_H_
#define ABSTRACTPLUGININTERFACE_H_

//#include <QObject>
#include <QString>


#include "../core_lib.h"


namespace HEHUI {

class AbstractPluginInterface {
    
public:
    
    virtual ~AbstractPluginInterface(){}
    
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QString vendor() const = 0;
    virtual QString author() const = 0;
    virtual QString url() const = 0;
    virtual QString description() const = 0;
    virtual QString license() const = 0;
    
    virtual bool isSingle() = 0;

    //初始化插件
    //Initialization
    //virtual bool initialize(QObject *parentObject) = 0;
    
    //卸载插件
    //Unload the plugin
    virtual bool unload() = 0;


    };


}


#define AbstractPluginInterface_IID "org.hehui.Interface.AbstractPluginInterface/2014.12"
Q_DECLARE_INTERFACE(HEHUI::AbstractPluginInterface, AbstractPluginInterface_IID)



#endif /* ABSTRACTPLUGININTERFACE_H_ */
