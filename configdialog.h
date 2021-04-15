#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QVariant>

class Settings;

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent = nullptr);
private:
    Settings *_settings;
signals:
    void settingChanged(QString setting, QVariant value);
};

#endif // CONFIGDIALOG_H
