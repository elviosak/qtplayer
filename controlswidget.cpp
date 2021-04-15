#include "controlswidget.h"
#include "mpvwidget.h"
#include "seekbar.h"
#include "volumebar.h"
#include "settings.h"
#include "configdialog.h"

#include <QEvent>

ControlsWidget::ControlsWidget(MpvWidget *mpv, QWidget *parent)
    : QToolBar(parent)
    , _settings(Settings::instance())
    , _mpv(mpv)
{
    setWindowTitle("Controls");
    setFloatable(true);
    setMovable(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    _seekStep = _settings->value("seekStep", 20).toInt();
    _volumeStep = _settings->value("volumeStep", 5).toInt();
    _wheelAction = _settings->value("wheelAction", "Seek").toString();
    //_fetchInfo = _settings->value( "fetchInfo", false).toBool();

    _currTime = 0;
    _totalTime = 0;

    _playIcon = QIcon(":/play");
    _pauseIcon = QIcon(":/pause");


    _speed = _settings->value("speed", 1).toDouble();
    _volume = _settings->value("volume", 70).toInt();
    _option = _settings->value("option", "480p").toString();
    _mpv->setOption(_option);

    _seekBar = new SeekBar(_mpv);
    _seekBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _seekLabel = new QLabel(" 0:00:00 / 0:00:00");
    _seekLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    _seekLabel->setAlignment(Qt::AlignCenter);

    _playAction = new QAction(QString());
    _playAction->setIcon(_playIcon);
    _prevAction = new QAction(QIcon(":/backward"), QString());
    _nextAction = new QAction(QIcon(":/forward"), QString());

    _volumeBar = new VolumeBar;
    _volumeAction = new QAction(QIcon(":/volume-3"),
                                 QString("%1")
                              .arg(QString::number(_volume)));


    _speedSpin = new QDoubleSpinBox;
    _speedSpin->setDecimals(1);
    _speedSpin->setRange(0.1, 5);
    _speedSpin->setSuffix("x");
    _speedSpin->setValue(_speed);
    _speedSpin->setSingleStep(0.1);
    _speedSpin->setFocusPolicy(Qt::NoFocus);

    _optionCombo = new QComboBox;
    _optionCombo->addItems(QStringList() << "480p" << "720p" << "1080p");
    _optionCombo->setCurrentText(_option);

    _configAction = new QAction(QIcon(":/settings"), QString());
    _fullScreenAction = new QAction(QIcon(":/fullscreen"), QString());

    addAction(_playAction);
    addAction(_prevAction);
    addAction(_nextAction);

    addSeparator();
    addWidget(_speedSpin);
    addWidget(_optionCombo);

    addSeparator();
    addWidget(_seekBar);
    addSeparator();
    addWidget(_seekLabel);

    addSeparator();
    addAction(_volumeAction);
    addWidget(_volumeBar);

    addSeparator();
    addAction(_configAction);
    addSeparator();
    addAction(_fullScreenAction);


    // CONTROLS
    connect(_seekBar, &SeekBar::valueChanged, this, [=] (int value) {
        _mpv->setProperty("time-pos", value);
    });
    connect(_seekBar, &SeekBar::valueStep, this, &ControlsWidget::onSeekStep);
    connect(_playAction, &QAction::triggered, this, [=] {
        bool pause = _mpv->getProperty("pause").toBool();
        _mpv->setProperty("pause", !pause);
    });
    connect(_prevAction, &QAction::triggered, this, [=] {
        _mpv->command(QVariantList() << "playlist-prev");
    });
    connect(_nextAction, &QAction::triggered, this, [=] {
        _mpv->command(QVariantList() << "playlist-next");
    });
    connect(_volumeAction, &QAction::triggered, this, [=] {
        int vol = _mpv->getProperty("volume").toInt();
        if (vol == 0){
            int newValue = (_mutedVolume < 10) ? 10 : _mutedVolume;
            _mpv->setProperty("volume", newValue);
        }
        else {
            _mutedVolume = _volume;
            _mpv->setProperty("volume", 0);
        }
    });
    connect(_volumeBar, &VolumeBar::valueChanged, this, [=] (int value) {
        _mpv->setProperty("volume", value);
    });
    connect(_volumeBar, &VolumeBar::valueStep, this, &ControlsWidget::onVolumeStep);

    connect(_speedSpin,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [=] (double value) {
        _mpv->setProperty("speed", value);
    });
    connect(_optionCombo, &QComboBox::currentTextChanged, this, [=] (QString text) {
        _option = text;
        _mpv->setOption(text);
        _settings->setValue("option", _option);
    });
    connect(_configAction, &QAction::triggered, this, [=] {
        auto config = new ConfigDialog(this);
        connect(config, &ConfigDialog::settingChanged, this, &ControlsWidget::settingChanged);
        config->show();
    });
    connect(_fullScreenAction, &QAction::triggered, this, [=] {
        emit toggleFullScreen();
    });


    // MPV LISTENERS
    connect(_mpv, &MpvWidget::valueStep, this, [=] (bool increase){
        if (_wheelAction == "Seek")
            onSeekStep(increase);
        else if (_wheelAction == "Volume")
            onVolumeStep(increase);
    });
    connect(_mpv, &MpvWidget::positionChanged, this, [=] (int value) {
        _seekBar->setCurrValue(value);
        _currTime = value;
        updateTimeLabel();
    });
    connect(_mpv, &MpvWidget::durationChanged, this, [=] (int value) {
        _seekBar->setMaxValue(value);
        _totalTime = value;
        updateTimeLabel();
    });
    connect(_mpv, &MpvWidget::speedChanged, this, [=] (double speed) {
        _speedSpin->setValue(speed);
        _settings->setValue("speed", speed);

    });
    connect(_mpv, &MpvWidget::volumeChanged, this, [=] (int volume) {
        _volume = volume;
        _volumeBar->setCurrValue(volume);
        if (volume == 0)
            _volumeAction->setIcon(QIcon(":/volume-0"));
        else if (volume <= 40)
            _volumeAction->setIcon(QIcon(":/volume-1"));
        else if (volume <= 80)
            _volumeAction->setIcon(QIcon(":/volume-2"));
        else
            _volumeAction->setIcon(QIcon(":/volume-3"));

        _volumeAction->setText(QString("%1").arg(QString::number(volume)));
        _volumeAction->setToolTip(QString("%1%").arg(QString::number(volume)));

        _settings->setValue("volume", volume);
    });
    connect(_mpv, &MpvWidget::pauseChanged, this, [=] (bool pause) {
        if(pause)
            _playAction->setIcon(_playIcon);
        else
            _playAction->setIcon(_pauseIcon);
    });


    _mpv->setProperty("volume", _volume);
    _mpv->setProperty("speed", _speed);

}
void ControlsWidget::onSeekStep(bool increase)
{
    int step = increase ? _seekStep : -_seekStep;
    int timePos = _mpv->getProperty("time-pos").toInt() + step;
    int lastPos = qMax(0, _mpv->getProperty("duration").toInt());
    int newValue = qBound(0, timePos, lastPos - 1);
    _mpv->setProperty("time-pos", newValue);
}

void ControlsWidget::onVolumeStep(bool increase)
{
    int step = increase ? _volumeStep : -_volumeStep;
    int newValue = qBound(0, _volume + step , 150);
    _mpv->setProperty("volume", newValue);
}

void ControlsWidget::settingChanged(QString key, QVariant val){
    if (key == "seekStep") {
        _seekStep = val.toInt();
        _settings->setValue("seekStep", val);
    }
    else if (key == "volumeStep") {
        _volumeStep = val.toInt();
        _settings->setValue("volumeStep", val);
    }
    else if (key == "wheelAction") {
        _wheelAction = val.toString();
        _settings->setValue("wheelAction", val);
    }
//    else if (key == "fetchInfo") {
//        _fetchInfo = val.toBool();
//        _settings->setValue("fetchInfo", val);
//    }
}


QString ControlsWidget::formatTime(int time)
{
    int sec = time % 60;
    int min = (int)(time / 60);
    int hour = (int)(min / 60);
    min = min % 60;
    return QStringLiteral("%1:%2:%3")
            .arg(hour)
            .arg(min, 2, 10, QLatin1Char('0'))
            .arg(sec, 2, 10, QLatin1Char('0'));
}

void ControlsWidget::updateTimeLabel()
{
    QString formattedText = QStringLiteral(" %1 / %2")
            .arg(formatTime(_currTime), formatTime(_totalTime));
    _seekLabel->setText(formattedText);
}

bool ControlsWidget::event(QEvent *e)
{
    if (e->type() == QEvent::WinIdChange){
        emit winIdChanged();
    }
    return QToolBar::event(e);
}
