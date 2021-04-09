#include "mainwindow.h"
#include "mpvwidget.h"
#include "playlistwidget.h"
#include "controlswidget.h"
#include "settings.h"

#include <QMouseEvent>
#include <QMimeData>
#include <QXmlStreamReader>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _settings(Settings::instance())
{
    setAcceptDrops(true);
    _playlistArea = _settings->value("playlistArea", Qt::DockWidgetArea::TopDockWidgetArea).value<Qt::DockWidgetArea>();
    _controlsArea = _settings->value("controlsArea", Qt::ToolBarArea::BottomToolBarArea).value<Qt::ToolBarArea>();

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

    _mpv->setFocus();
    resize(800, 640);

    connect(_mpv, &MpvWidget::doubleClicked, this, [=] {
        if (isFullScreen()){
            showNormal();
            _playlist->show();
        }
        else {
            showFullScreen();
            _playlist->hide();
        }
    });
    connect(_controls, &ControlsWidget::toggleFullScreen, this, [=] {
        if (isFullScreen()){
            showNormal();
            _playlist->show();
        }
        else {
            showFullScreen();
            _playlist->hide();
        }
    });

    connect(_escShortcut, &QShortcut::activated, this, [=]{
        if (isFullScreen()){
            showNormal();
            _playlist->show();
        }
    });
    connect(_spaceShortcut, &QShortcut::activated, this, [=]{
        bool pause = _mpv->getProperty("pause").toBool();
        _mpv->setProperty("pause", !pause);
    });

    connect(_mpv, &MpvWidget::mediaTitleChanged, this, [=](QString title){
        setWindowTitle(title);
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
