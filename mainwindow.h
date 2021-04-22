#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>
#include <QLabel>
#include <QTimer>

class MpvWidget;
class PlaylistWidget;
class ControlsWidget;
class Settings;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

private:
    Qt::DockWidgetArea _playlistArea;
    Qt::ToolBarArea _controlsArea;

    bool _playlistAutoHide;
    bool _controlsAutoHide;
    QTimer *_playlistTimer;
    QTimer *_controlsTimer;
    QRect _playlistRect;
    QRect _controlsRect;

    int _hideDelay;

    MpvWidget *_mpv;
    Settings *_settings;
    PlaylistWidget *_playlist;
    ControlsWidget *_controls;
    QWidget *_overlay;
    QLabel *_overlayTitle;
    QTimer *_overlayTimer;
    QShortcut * _spaceShortcut;
    QShortcut * _escShortcut;

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
};

#endif // MAINWINDOW_H
