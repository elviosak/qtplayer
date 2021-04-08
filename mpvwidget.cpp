#include "mpvwidget.h"
#include <stdexcept>
#include <QOpenGLContext>
#include <QMetaObject>
#include <QMouseEvent>

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}
MpvWidget::MpvWidget(QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{
    QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mpv = mpv_create();
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

    //mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "keep-open", "yes");
    //mpv_set_option_string(mpv, "ytdl-format", "bestvideo[height<=?480]+bestaudio/best");
    mpv_set_option_string(mpv, "msg-level", "all=fatal");
    mpv_set_option_string(mpv, "demuxer-readahead-secs", "20");

    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Request hw decoding, just for testing.
    mpv::qt::set_option_variant(mpv, "hwdec", "auto");
    mpv_observe_property(mpv, 0, "playlist", MPV_FORMAT_NODE);
    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "speed", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "media-title", MPV_FORMAT_STRING);
    mpv_observe_property(mpv, 0, "eof-reached", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "volume", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "playlist-pos", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "playlist-count", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "demuxer-cache-state", MPV_FORMAT_NODE);

    mpv_set_wakeup_callback(mpv, wakeup, this);
}

MpvWidget::~MpvWidget()
{
    makeCurrent();
    if (mpv_gl)
        mpv_render_context_free(mpv_gl);
    mpv_terminate_destroy(mpv);
}
void MpvWidget::setOption(QString option)
{
    if (option =="480p")
        mpv_set_option_string(mpv, "ytdl-format", "bestvideo[height<=?480]+bestaudio/best");
    if (option =="720p")
        mpv_set_option_string(mpv, "ytdl-format", "bestvideo[height<=?720]+bestaudio/best");
    if (option =="1080p")
        mpv_set_option_string(mpv, "ytdl-format", "bestvideo[height<=?1080]+bestaudio/best");

    if (getProperty("playlist-count").toInt() > 0) {     
        auto playlist = getProperty("playlist").value<QVariantList>();
        auto current = playlist.at(getProperty("current-pos").toInt()).value<QMap<QString, QVariant>>();
        command(QStringList() << "playlist-play-index" << "current");
    }
}
void MpvWidget::mousePressEvent(QMouseEvent *e)
{
    setFocus();
    if (e->button() == Qt::MouseButton::LeftButton){
        setProperty("pause", !getProperty("pause").toBool());
        QWidget::mousePressEvent(e);
    }
    else {
        QWidget::mousePressEvent(e);
    }
}
void MpvWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    setFocus();
    if (e->button() == Qt::MouseButton::LeftButton){
        emit doubleClicked();
        e->accept();
    }
    else {
        QWidget::mouseDoubleClickEvent(e);
    }
}
void MpvWidget::command(const QVariant& params)
{
    mpv::qt::command_variant(mpv, params);
}

void MpvWidget::setProperty(const QString& name, const QVariant& value)
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant MpvWidget::getProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}

void MpvWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void MpvWidget::handle_mpv_event(mpv_event *event)
{
    if (event->event_id != MPV_EVENT_PROPERTY_CHANGE) {
        return;
    }

    mpv_event_property *prop = (mpv_event_property *)event->data;
    if (strcmp(prop->name, "playlist") == 0) {
        if (prop->format == MPV_FORMAT_NODE) {
            auto playlist = mpv::qt::node_to_variant((mpv_node*)prop->data);
            emit playlistChanged(playlist);
        }
    }
    else if (strcmp(prop->name, "time-pos") == 0) {
        if (prop->format == MPV_FORMAT_INT64) {
            int time = *(int *)prop->data;
            emit positionChanged(time);
        }
    }
    else if (strcmp(prop->name, "duration") == 0) {
        if (prop->format == MPV_FORMAT_INT64) {
            int duration = *(int *)prop->data;
            emit durationChanged(duration);
        }
    }
    else if (strcmp(prop->name, "speed") == 0) {
        if (prop->format == MPV_FORMAT_DOUBLE) {
            double speed = *(double *)prop->data;
            emit speedChanged(speed);
        }
    }
    else if (strcmp(prop->name, "pause") == 0) {
        if (prop->format == MPV_FORMAT_FLAG) {
            bool pause = *(bool *)prop->data;
            emit pauseChanged(pause);
        }
    }
    else if (strcmp(prop->name, "media-title") == 0) {
        if (prop->format == MPV_FORMAT_STRING) {
            char* title = *(char **)prop->data;
            emit mediaTitleChanged(QLatin1String(title));
        }
    }
    else if (strcmp(prop->name, "eof-reached") == 0) {
        if (prop->format == MPV_FORMAT_FLAG) {
            bool eofReached = *(bool *)prop->data;
            emit eofReachedChanged(eofReached);
        }
    }
    else if (strcmp(prop->name, "volume") == 0) {
        if (prop->format == MPV_FORMAT_INT64) {
            int volume = *(int *)prop->data;
            emit volumeChanged(volume);
        }
    }
    else if (strcmp(prop->name, "playlist-pos") == 0) {
        if (prop->format == MPV_FORMAT_INT64) {
            int playlistPos = *(int *)prop->data;
            emit playlistPosChanged(playlistPos);
        }
    }
    else if (strcmp(prop->name, "playlist-count") == 0) {
        if (prop->format == MPV_FORMAT_INT64) {
            int playlistCount = *(int *)prop->data;
            emit playlistCountChanged(playlistCount);
        }
    }
    else if (strcmp(prop->name, "demuxer-cache-state") == 0) {
        if (prop->format == MPV_FORMAT_NODE) {
            auto demuxerState = mpv::qt::node_to_variant((mpv_node*)prop->data);
            emit demuxerStateChanged(demuxerState);
        }
    }

}

void MpvWidget::initializeGL()
{
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr, nullptr};
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    if (mpv_render_context_create(&mpv_gl, mpv, params) < 0)
        throw std::runtime_error("failed to initialize mpv GL context");
    mpv_render_context_set_update_callback(mpv_gl, MpvWidget::on_update, reinterpret_cast<void *>(this));
}

void MpvWidget::paintGL()
{
    mpv_opengl_fbo mpfbo{static_cast<int>(defaultFramebufferObject()), width(), height(), 0};
    int flip_y{1};

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    // See render_gl.h on what OpenGL environment mpv expects, and
    // other API details.
    mpv_render_context_render(mpv_gl, params);
}

// Make Qt invoke mpv_render_context_render() to draw a new/updated video frame.
void MpvWidget::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's render API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        doneCurrent();
    } else {
        update();
    }
}

void MpvWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "maybeUpdate");
}
