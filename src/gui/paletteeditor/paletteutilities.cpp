#include "paletteutilities.h"


PaletteUtilities::PaletteUtilities()
{

    m_colorRoleList.append(QPalette::Window);
    m_colorRoleList.append(QPalette::WindowText);
    m_colorRoleList.append(QPalette::Base);
    m_colorRoleList.append(QPalette::AlternateBase);
    m_colorRoleList.append(QPalette::ToolTipBase);
    m_colorRoleList.append(QPalette::ToolTipText);
    m_colorRoleList.append(QPalette::Text);
    m_colorRoleList.append(QPalette::Button);
    m_colorRoleList.append(QPalette::ButtonText);
    m_colorRoleList.append(QPalette::BrightText);

    m_colorRoleList.append(QPalette::Light);
    m_colorRoleList.append(QPalette::Midlight);
    m_colorRoleList.append(QPalette::Mid);
    m_colorRoleList.append(QPalette::Dark);
    m_colorRoleList.append(QPalette::Shadow);
    m_colorRoleList.append(QPalette::Highlight);
    m_colorRoleList.append(QPalette::HighlightedText);

    m_colorRoleList.append(QPalette::Link);
    m_colorRoleList.append(QPalette::LinkVisited);

}

bool PaletteUtilities::importPalette(QSettings *settings, QPalette *palette, QString *errorMessage)
{
    if(!settings || (!palette)){
        if(errorMessage){
            *errorMessage = QObject::tr("Invalid parameter!");
        }
        return false;
    }

    settings->beginGroup("Palette");
    if(settings->allKeys().isEmpty()){
        if(errorMessage){
            *errorMessage = QObject::tr("Invalid palette info!");
        }
        return false;
    }
    foreach (int colorRole, m_colorRoleList) {
        readSettings(settings, colorRole, palette);
    }
    settings->endGroup();

    return true;
}

void PaletteUtilities::readSettings(QSettings *settings, int colorRole, QPalette *palette)
{
    Q_ASSERT(settings && palette);

    QStringList colors = settings->value(QString::number(colorRole), "").toStringList();
    QColor color;
    if(colors.size() > 0){
        color = QColor(colors[0]);
        if(color.isValid()){
            palette->setColor(QPalette::All, QPalette::ColorRole(colorRole), QColor(color));
        }
    }
    if(colors.size() > 1){
        color = QColor(colors[1]);
        if(color.isValid()){
            palette->setColor(QPalette::Disabled, QPalette::ColorRole(colorRole), QColor(color));
        }
    }

    if(colors.size() > 2){
        color = QColor(colors[2]);
        if(color.isValid()){
            palette->setColor(QPalette::Inactive, QPalette::ColorRole(colorRole), QColor(color));
        }
    }
}

bool PaletteUtilities::exportPalette(QSettings *settings, QPalette *palette, QString *errorMessage)
{
    if(!settings || (!palette)){
        if(errorMessage){
            *errorMessage = QObject::tr("Invalid parameter!");
        }
        return false;
    }

    settings->beginGroup("Palette");

    foreach (int colorRole, m_colorRoleList) {
        writeSettings(settings, colorRole, palette);
    }

    settings->endGroup();
    settings->sync();

    return true;
}

void PaletteUtilities::writeSettings(QSettings *settings, int colorRole, QPalette *palette)
{
    Q_ASSERT(settings && palette);

    QStringList colors;
    colors.append(palette->color(QPalette::All, QPalette::ColorRole(colorRole)).name());
    colors.append(palette->color(QPalette::Disabled, QPalette::ColorRole(colorRole)).name());
    colors.append(palette->color(QPalette::Inactive, QPalette::ColorRole(colorRole)).name());
    settings->setValue(QString::number(colorRole), colors);
}
