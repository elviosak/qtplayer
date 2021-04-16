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

    bool _playlistVisible;
    bool _controlsVisible;

    MpvWidget *_mpv;
    Settings *_settings;
    PlaylistWidget *_playlist;
    ControlsWidget *_controls;
    QWidget *_hiddenControls;
    QLabel *_hiddenTitle;
    QTimer *_hideTimer;
    QShortcut * _spaceShortcut;
    QShortcut * _escShortcut;

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
};

#endif // MAINWINDOW_H
