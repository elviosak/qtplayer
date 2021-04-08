#ifndef CONTROLSWIDGET_H
#define CONTROLSWIDGET_H

#include <QToolBar>
#include <QLabel>
#include <QAction>
#include <QDoubleSpinBox>
#include <QComboBox>


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

    int _mutedVolume;
    int _currTime;
    int _totalTime;

    Settings *_settings;
    MpvWidget *_mpv;
    SeekBar *_seekBar;
    QLabel *_seekLabel;
    QAction *_playBtn;
    QAction *_prevBtn;
    QAction *_nextBtn;
    QDoubleSpinBox *_speedSpin;
    QComboBox *_optionCombo;
    VolumeBar *_volumeBar;
    QAction *_volumeBtn;
    QAction *_fullScreenAction;

    void updateTimeLabel();
    QString formatTime(int time);

protected:
    bool event(QEvent *e) override;

signals:
    void toggleFullScreen();
    void winIdChanged();
};

#endif // CONTROLSWIDGET_H
