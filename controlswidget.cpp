#include "controlswidget.h"
#include "mpvwidget.h"
#include "seekbar.h"
#include "volumebar.h"
#include "settings.h"

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

    _currTime = 0;
    _totalTime = 0;

    _speed = _settings->value("speed", 1).toDouble();
    _volume = _settings->value("volume", 70).toInt();
    _option = _settings->value("option", "480p").toString();
    _mpv->setOption(_option);

    _seekBar = new SeekBar(_mpv);
    _seekBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _seekLabel = new QLabel(" 0:00:00 / 0:00:00");
    _seekLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    _seekLabel->setAlignment(Qt::AlignCenter);

    _playBtn = new QAction(QIcon::fromTheme("media-play"), QString());
    _prevBtn = new QAction(QIcon::fromTheme("media-skip-backward"), QString());
    _nextBtn = new QAction(QIcon::fromTheme("media-skip-forward"), QString());

    _volumeBar = new VolumeBar;
    _volumeBtn = new QAction(QIcon::fromTheme("audio-volume-medium"),
                                 QString("%1")
                              .arg(QString::number(_volume)));


    _speedSpin = new QDoubleSpinBox;
    _speedSpin->setDecimals(1);
    _speedSpin->setRange(0.1, 5);
    _speedSpin->setSuffix("x");
    _speedSpin->setValue(_speed);
    _speedSpin->setSingleStep(0.1);

    _optionCombo = new QComboBox;
    _optionCombo->addItems(QStringList() << "480p" << "720p" << "1080p");
    _optionCombo->setCurrentText(_option);

    _fullScreenAction = new QAction(QIcon::fromTheme("view-fullscreen"), QString());

    addAction(_playBtn);
    addAction(_prevBtn);
    addAction(_nextBtn);

    addSeparator();
    addWidget(_speedSpin);
    addWidget(_optionCombo);

    addSeparator();
    addWidget(_seekBar);
    addSeparator();
    addWidget(_seekLabel);

    addSeparator();
    addAction(_volumeBtn);
    addWidget(_volumeBar);

    addSeparator();
    addAction(_fullScreenAction);
    addSeparator();


    // CONTROLS
    connect(_seekBar, &SeekBar::valueChanged, this, [=] (int value) {
        _mpv->setProperty("time-pos", value);
    });
    connect(_seekBar, &SeekBar::valueRelative, this, [=] (int value) {
        int timePos = _mpv->getProperty("time-pos").toInt() + value;
        int lastPos = qMax(0, _mpv->getProperty("duration").toInt() - 1);
        int newValue = qBound(0, timePos, lastPos);
        _mpv->setProperty("time-pos", newValue);
    });
    connect(_playBtn, &QAction::triggered, this, [=] {
        bool pause = _mpv->getProperty("pause").toBool();
        _mpv->setProperty("pause", !pause);
    });
    connect(_prevBtn, &QAction::triggered, this, [=] {
        _mpv->command(QVariantList() << "playlist-prev");
    });
    connect(_nextBtn, &QAction::triggered, this, [=] {
        _mpv->command(QVariantList() << "playlist-next");
    });
    connect(_volumeBtn, &QAction::triggered, this, [=] {
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
    connect(_volumeBar, &VolumeBar::valueRelative, this, [=] (int value) {
        int newValue = qBound(0, _volume + value , 150);
        _mpv->setProperty("volume", newValue);
    });

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
    connect(_fullScreenAction, &QAction::triggered, this, [=] {
        emit toggleFullScreen();
    });
    // MPV LISTENERS
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
            _volumeBtn->setIcon(QIcon::fromTheme("audio-volume-muted"));
        else if (volume <= 40)
            _volumeBtn->setIcon(QIcon::fromTheme("audio-volume-low"));
        else if (volume <= 80)
            _volumeBtn->setIcon(QIcon::fromTheme("audio-volume-medium"));
        else
            _volumeBtn->setIcon(QIcon::fromTheme("audio-volume-high"));

        _volumeBtn->setText(QString("%1").arg(QString::number(volume)));
        _settings->setValue("volume", volume);
    });
    connect(_mpv, &MpvWidget::pauseChanged, this, [=] (bool pause) {
        if(pause)
            _playBtn->setIcon(QIcon::fromTheme("media-play"));
        else
            _playBtn->setIcon(QIcon::fromTheme("media-pause"));
    });


    _mpv->setProperty("volume", _volume);
    _mpv->setProperty("speed", _speed);

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
