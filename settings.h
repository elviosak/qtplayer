#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QSettings
{
public:
    Settings();
    static Settings *instance();
};

#endif // SETTINGS_H
