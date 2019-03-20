#ifndef PALETTEUTILITIES_H
#define PALETTEUTILITIES_H

#include <QObject>
#include <QSettings>
#include <QPalette>

class PaletteUtilities
{
public:
    PaletteUtilities();


    bool importPalette(QSettings *settings, QPalette *palette, QString *errorMessage = 0);
    bool exportPalette(QSettings *settings, QPalette *palette, QString *errorMessage = 0);

private:
    void readSettings(QSettings *settings, int colorRole, QPalette *palette);
    void writeSettings(QSettings *settings, int colorRole, QPalette *palette);


private:
    QList<int> m_colorRoleList;


};

#endif // PALETTEUTILITIES_H
