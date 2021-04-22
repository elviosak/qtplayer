#include "configdialog.h"
#include "settings.h"

#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QApplication>


ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
    , _settings(Settings::instance())
{
    setModal(true);
    auto grid = new QGridLayout(this);
    auto form = new QFormLayout;
    grid->addLayout(form, 0, 0, 1, 2);
    auto seekSpin = new QSpinBox;
    seekSpin->setValue(_settings->value("seekStep", 20).toInt());
    seekSpin->setRange(1, 200);
    seekSpin->setSingleStep(10);
    seekSpin->setSuffix(" s");
    auto volumeSpin = new QSpinBox;
    volumeSpin->setRange(1,50);
    volumeSpin->setSingleStep(1);
    volumeSpin->setValue(_settings->value("volumeStep", 5).toInt());
    auto wheelCombo = new QComboBox;
    wheelCombo->setEditable(false);
    wheelCombo->addItems(QStringList() << "Seek" << "Volume" << "None");
    wheelCombo->setCurrentText(_settings->value("wheelAction", "Seek").toString());
//    auto fetchCheck = new QCheckBox;
//    fetchCheck->setChecked(_settings->value("fetchInfo", false).toBool());
    auto playlistAutoHide = new QCheckBox;
    playlistAutoHide->setChecked(_settings->value("playlistAutoHide", true).toBool());
    auto controlsAutoHide = new QCheckBox;
    controlsAutoHide->setChecked(_settings->value("controlsAutoHide", false).toBool());
    auto hideDelaySpin = new QSpinBox;
    hideDelaySpin->setValue(_settings->value("hideTimer", 4).toInt());
    hideDelaySpin->setRange(1, 20);
    hideDelaySpin->setSingleStep(1);
    hideDelaySpin->setSuffix(" second(s)");

    form->addRow("Seek Step", seekSpin);
    form->addRow("Volume Step", volumeSpin);
    form->addRow("Wheel Action", wheelCombo);
    form->addRow("Auto hide Playlist", playlistAutoHide);
    form->addRow("Auto hide Controls", controlsAutoHide);
    form->addRow("Hide delay", hideDelaySpin);
//    form->addRow("Fetch Info", fetchCheck);

    auto aboutBtn = new QPushButton("About Qt");
    auto closeBtn = new QPushButton("Close");

    auto spacer = new QWidget;
    spacer->setFixedHeight(10);
    grid->addWidget(spacer, 1, 0, 1, 2);
    grid->addWidget(aboutBtn, 2, 0, Qt::AlignLeft);
    grid->addWidget(closeBtn, 2, 1, Qt::AlignRight);

    connect(seekSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=] (int value) {
        emit settingChanged("seekStep", value);
    });
    connect(volumeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=] (int value) {
        emit settingChanged("volumeStep", value);
    });
    connect(wheelCombo, &QComboBox::currentTextChanged, this, [=] (QString text) {
        emit settingChanged("wheelAction", text);
    });
    connect(playlistAutoHide, &QCheckBox::toggled, this, [=] (bool checked) {
        emit settingChanged("playlistAutoHide", checked);
    });
    connect(controlsAutoHide, &QCheckBox::toggled, this, [=] (bool checked) {
        emit settingChanged("controlsAutoHide", checked);
    });
    connect(hideDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=] (int value) {
        emit settingChanged("hideDelay", value);
    });

//    connect(fetchCheck, &QCheckBox::toggled, this, [=] (bool checked) {
//        emit settingChanged("seekValue", checked);
//    });

    connect(aboutBtn, &QPushButton::clicked, qApp, &QApplication::aboutQt);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);

}
