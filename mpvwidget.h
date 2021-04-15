#ifndef MPVWIDGET_H
#define MPVWIDGET_H

#include <QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include "qthelper.hpp"

class MpvWidget Q_DECL_FINAL: public QOpenGLWidget
{
    Q_OBJECT
public:
    MpvWidget(QWidget *parent = 0, Qt::WindowFlags f = {});
    ~MpvWidget();
    void command(const QVariant& params);
    void setOption(QString option);
    void changeOption(QString option);
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;
    //QSize sizeHint() const override { return QSize(480, 270);};

signals:
    void durationChanged(int value);
    void positionChanged(int value);
    void speedChanged(double value);
    void pauseChanged(bool value);
    void mediaTitleChanged(QString value);
    void eofReachedChanged(bool value);
    void volumeChanged(int value);
    void playlistPosChanged(int value);
    void playlistCountChanged(int value);
    void playlistChanged(QVariant value);
    void demuxerStateChanged(QVariant value);
    void clicked();
    void doubleClicked();
    void valueStep(bool increase);

protected:
    void wheelEvent(QWheelEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void initializeGL() override;
    void paintGL() override;

private slots:
    void on_mpv_events();
    void maybeUpdate();

private:
    void handle_mpv_event(mpv_event *event);
    static void on_update(void *ctx);

    mpv_handle *mpv;
    mpv_render_context *mpv_gl;
};

#endif // MPVWIDGET_H
