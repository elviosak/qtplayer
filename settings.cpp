#include "settings.h"

Q_GLOBAL_STATIC(Settings, settings)

Settings::Settings()
    : QSettings("qtplayer", "qtplayer")
{}

Settings *Settings::instance()
{
    return  settings();
}
