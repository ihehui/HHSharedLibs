#ifndef CTRLALTDELSIMULATOR_H
#define CTRLALTDELSIMULATOR_H

#include <QObject>
#include <QThread>


namespace HEHUI {


// Simulates the "ctrl + alt + del" combination under WindowsXP.
class CtrlAltDelSimulator : public QThread
{
public:
    CtrlAltDelSimulator(QObject *parent = 0);
    ~CtrlAltDelSimulator();

protected:
    void run();

};



} //namespace HEHUI

#endif // CTRLALTDELSIMULATOR_H
