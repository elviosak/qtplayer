#ifndef CONTROLSWIDGET_H
#define CONTROLSWIDGET_H

#include <QToolBar>
#include <QLabel>
#include <QAction>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QIcon>

#include "qthelper.hpp"

class MpvWidget;
class SeekBar;
class VolumeBar;
class Settings;

class ControlsWidget: public QToolBar
{
    Q_OBJECT
public:
    ControlsWidget(MpvWidget *mpv, QWidget *parent = nullptr);

private:
    double _speed;
    int _volume;

    QString _option;
    int _seekStep;
    int _volumeStep;
    QString _wheelAction;
//    bool _fetchInfo;

    int _mutedVolume;
    int _currTime;
    int _totalTime;

    Settings *_settings;
    MpvWidget *_mpv;
    SeekBar *_seekBar;
    QLabel *_seekLabel;
    QAction *_playAction;
    QAction *_prevAction;
    QAction *_nextAction;
    QDoubleSpinBox *_speedSpin;
    QComboBox *_optionCombo;
    VolumeBar *_volumeBar;
    QAction *_volumeAction;
    QAction * _configAction;
    QAction *_fullScreenAction;
    QIcon _playIcon;
    QIcon _pauseIcon;

    void updateTimeLabel();
    QString formatTime(int time);
    void settingChanged(QString key, QVariant val);
    void onVolumeStep(bool increase);
    void onSeekStep(bool increase);
protected:
    bool event(QEvent *e) override;

signals:
    void toggleFullScreen();
    void winIdChanged();
    void playlistVisibilityChanged(Enum::Visibility);
    void controlsVisibilityChanged(Enum::Visibility);
    void hideDelayChanged(int timeSec);
    void geometryChanged(QRect geo);
};

#endif // CONTROLSWIDGET_H
