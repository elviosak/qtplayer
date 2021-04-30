#include "mainwindow.h"
#include "mpvwidget.h"
#include "playlistwidget.h"
#include "controlswidget.h"
#include "settings.h"

#include <QCheckBox>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMimeData>
#include <QXmlStreamReader>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _settings(Settings::instance())
{
    setAcceptDrops(true);
    setMouseTracking(true);
    _playlistArea = _settings->value("playlistArea", Qt::DockWidgetArea::TopDockWidgetArea).value<Qt::DockWidgetArea>();
    _controlsArea = _settings->value("controlsArea", Qt::ToolBarArea::BottomToolBarArea).value<Qt::ToolBarArea>();

    _playlistAutoHide = _settings->value("playlistAutoHide", true).toBool();
    _controlsAutoHide = _settings->value("controlsAutoHide", false).toBool();

//    _playlistHiding = false;
//    _playlistShowing = false;

    _hideDelay = _settings->value("hideDelay", 4).toInt() * 1000;

    setWindowIcon(QIcon(":/qtplayer"));
    _spaceShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    _spaceShortcut->setAutoRepeat(false);
    _escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    _escShortcut->setAutoRepeat(false);

    _mpv = new MpvWidget(this);
    _playlist = new PlaylistWidget(_mpv);
    _controls = new ControlsWidget(_mpv);
    setCentralWidget(_mpv);
    setDockOptions(dockOptions() & ~AllowTabbedDocks);

    addDockWidget(_playlistArea, _playlist);
    addToolBar(_controlsArea, _controls);

    _playlist->setVisible(true);
    _controls->setVisible(true);
    _controls->setAllowedAreas(Qt::BottomToolBarArea | Qt::TopToolBarArea);

    _overlayTitle = new QLabel;
    QFont f = font();
    f.setPointSize(f.pointSize() * 1.4);
    _overlayTitle->setFont(f);

    _overlay = new QWidget(_mpv);
    _overlay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto lay = new QHBoxLayout(_overlay);

    lay->addWidget(_overlayTitle);
    _overlay->move(0,0);
    _overlay->hide();


    _overlayTimer = new QTimer(this);
    _overlayTimer->setSingleShot(true);

    _playlistTimer = new QTimer(this);
    _playlistTimer->setSingleShot(true);

    _controlsTimer = new QTimer(this);
    _controlsTimer->setSingleShot(true);

    _mpv->setFocus();
    resize(800, 640);


    connect(_controls, &ControlsWidget::playlistAutoHideChanged, this, [=] (bool checked) {
        _playlistAutoHide = checked;
        if (checked)
            _playlistTimer->start(_hideDelay);
        else
            _playlistTimer->stop();

        showPlaylist();
    });
    connect(_controls, &ControlsWidget::controlsAutoHideChanged, this, [=] (bool checked) {
        _controlsAutoHide = checked;
        if (checked)
            _controlsTimer->start(_hideDelay);
        else
            _controlsTimer->stop();

        _controls->show();
    });
    connect(_controls, &ControlsWidget::hideDelayChanged, this, [=] (int timeSec) {
        _hideDelay = timeSec * 1000;
    });

    connect(_overlayTimer, &QTimer::timeout, this, [=] {
         _overlay->hide();
    });
    connect(_playlistTimer, &QTimer::timeout, this, [=] {
        if (_playlist->underMouse())
            _playlistTimer->start(_hideDelay);
        else
            hidePlaylist();
    });
    connect(_controlsTimer, &QTimer::timeout, this, [=] {
        if (_controls->underMouse())
            _controlsTimer->start(_hideDelay);
        else
            _controls->hide();
    });

    connect(_mpv, &MpvWidget::doubleClicked, this, [=] {
        if (isFullScreen()){
            showNormal();
        }
        else {
            showFullScreen();
        }
    });
    connect(_controls, &ControlsWidget::toggleFullScreen, this, [=] {
        if (isFullScreen()){
            showNormal();
        }
        else {
            showFullScreen();
        }
    });

    connect(_escShortcut, &QShortcut::activated, this, [=]{
        if (isFullScreen()){
            showNormal();
        }
    });
    connect(_spaceShortcut, &QShortcut::activated, this, [=]{
        bool pause = _mpv->getProperty("pause").toBool();
        _mpv->setProperty("pause", !pause);
    });

    connect(_mpv, &MpvWidget::mediaTitleChanged, this, [=](QString title){
        setWindowTitle(title);
        _overlayTitle->setText(title);
        _overlay->hide();
        _overlay->show();
        _overlayTimer->start(_hideDelay);
    });
    connect(_controls, &ControlsWidget::winIdChanged, this, [=]{
        auto area = toolBarArea(_controls);
        if (_controlsArea != area && area != Qt::ToolBarArea::NoToolBarArea){
            _controlsArea = area;
            _settings->setValue("controlsArea", area);
        }
    });
    connect(_playlist, &PlaylistWidget::dockLocationChanged, this, [=] (Qt::DockWidgetArea area){
        if(_playlistArea != area && area != Qt::DockWidgetArea::NoDockWidgetArea){
            _playlistArea = area;
            _settings->setValue("playlistArea", area);
        }
    });
    connect(_playlist, &PlaylistWidget::geometryChanged, this, [=] (QRect geo) {
        _playlistRect = geo;

    });
//    connect(_controls, &ControlsWidget::geometryChanged, this, [=] (QRect geo) {
//        _controlsRect = geo;
//    });
    _playlistRect = _playlist->geometry();
    //_controlsRect = _controls->geometry();


    if (_playlistAutoHide)
        _playlistTimer->start(_hideDelay);
    if (_controlsAutoHide)
        _controlsTimer->start(_hideDelay);
}
void MainWindow::showPlaylist()
{
    // TODO: animate show Playlist
    _playlist->show();
}
void MainWindow::hidePlaylist()
{
    // TODO: animate hide Playlist
    _playlist->hide();
}
void MainWindow::resizeEvent(QResizeEvent *e)
{

    if (_playlistArea == Qt::DockWidgetArea::TopDockWidgetArea)
        _playlistRect.setRight(e->size().width());
    else if (_playlistArea == Qt::DockWidgetArea::LeftDockWidgetArea)
        _playlistRect.setBottom(e->size().height());
    else if (_playlistArea == Qt::DockWidgetArea::RightDockWidgetArea) {
        _playlistRect.moveRight(e->size().width());
        _playlistRect.setBottom(e->size().height());
    }
    else if (_playlistArea == Qt::DockWidgetArea::BottomDockWidgetArea) {
        _playlistRect.moveBottom(e->size().height());
        _playlistRect.setRight(e->size().width());
    }
    QMainWindow::resizeEvent(e);
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (_controlsAutoHide && _mpv->underMouse()) {
        _controls->show();
        _controlsTimer->start(_hideDelay);
    }

    if (_playlistAutoHide
            && ((_playlist->isHidden() && _playlistRect.contains(e->pos()))
                || (_playlist->isVisible() && _playlist->underMouse()))){
        //_playlist->show();
        showPlaylist();
        _playlistTimer->start(_hideDelay);
    }
    if (_mpv->underMouse()){
        _overlay->show();
        _overlayTimer->start(_hideDelay);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasHtml()
            || e->mimeData()->hasText()
            || e->mimeData()->hasUrls()){
        e->acceptProposedAction();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *e) {
    if (e->mimeData()->hasHtml()
            || e->mimeData()->hasText()
            || e->mimeData()->hasUrls()){
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e) {
    auto data = e->mimeData();
    if (data->hasUrls()){
        for (int i = 0; i < data->urls().count(); ++i) {
            QString url = data->urls().at(i).toString();
            _playlist->addUrl(url);
        }
        e->accept();
    }
    else if (data->hasText() && !data->text().trimmed().isEmpty()) {
        _playlist->addUrl(data->text().trimmed());
        e->accept();
    }
    else
        e->ignore();
};
