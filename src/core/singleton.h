/********************************************************************************************************
 * PROGRAM      : fresh
 * DATE - TIME  : 2007/06/17
 * AUTHOR       : Nox PasNox ( pasnox@gmail.com ), klorydryk ( benoit.piedallu@free.fr ), lud42fr, IrmatDen
 * FILENAME     : QSingleton.h
 * LICENSE      : GPL
 * COMMENTARY   : This template class allow to singletonize QObject herited classes
 ********************************************************************************************************/
/*
	How to use :

	// single sur un QPushButton
	class MyPushButon : public QPushButton, public Singleton<MyPushButon>
	{
  		friend class Singleton<MyPushButon>;

	protected:
		MyPushButon( QWidget* p )
		: QPushButton( p ), Singleton<MyPushButon>()
		{};
	};
	// in cpp
	MyPushButton* pb = MyPushButton::instance( parentWidget ); // initialise object
	// then u can retreive it with MyPushButton::instance() or using pb pointer
*/

#ifndef SINGLETON_H
#define SINGLETON_H

#include <QObject>
#include <QHash>
#include <QMetaObject>


#include "core_lib.h"


class CORE_LIB_API SingletonExpose
{
protected:
	static QHash<const QMetaObject*, QObject*> mInstances;
};

template <class T>
class Singleton : public SingletonExpose
{
protected:
	Singleton() {};
	virtual ~Singleton()
	{ mInstances.remove( &T::staticMetaObject ); }

public:
	template <typename P>
	static T* instance( P* );
	static T* instance();
	static bool instanceAvailable()
	{ return mInstances.contains( &T::staticMetaObject ); }
	static void cleanInstance()
	{ if ( instanceAvailable() ) delete mInstances[ &T::staticMetaObject ]; }

};

template <class T>
template <typename P>
T* Singleton<T>::instance( P* p )
{
	T* t = qobject_cast<T*>( mInstances.value( &T::staticMetaObject ) );
	if ( !t )
		mInstances[&T::staticMetaObject] = ( t = new T( p ) );
	return t;
}

template <class T>
T* Singleton<T>::instance()
{
	T* t = qobject_cast<T*>( mInstances.value( &T::staticMetaObject ) );
	if ( !t )
#ifdef Q_CC_GNU
		mInstances[&T::staticMetaObject] = ( t = new T );
#else
		mInstances[&T::staticMetaObject] = ( t = new T( 0 ) );
#endif
	return t;
}

#endif // QSINGLETON_H
