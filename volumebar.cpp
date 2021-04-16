#include "volumebar.h"

#include <QMouseEvent>
#include <QToolTip>
#include <QPainter>

VolumeBar::VolumeBar(QWidget *parent)
    : QWidget(parent)
{
    _pressed = false;
    _width = 52;
    _maxValue = 150;
    _currValue = 0;

    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setFixedWidth(_width);
    createGradient();
}

void VolumeBar::createGradient(){
    _gradient.setStart(0, 0);
    _gradient.setFinalStop(width() -2, 0);
    QGradientStops stops;
    stops << QGradientStop(0, QColor(136,255,136)) // #8f8
    << QGradientStop(0.66, QColor(0,136,0)) // # 080
    << QGradientStop(0.67, QColor(221,170,0)) // #da0
    << QGradientStop(1, QColor(255,68,68)); // #f44
    _gradient.setStops(stops);
}

void VolumeBar::paintEvent(QPaintEvent */*e*/){
    QPainter p(this);
    int barHeight = height() / 2.5;
    int barY = (height() - barHeight) / 2;

    p.fillRect(0, barY, width(), barHeight, QColor(221,221,221)); //#ddd

    double w = (width() - 2) * _currValue / _maxValue;
    QRectF r1(1, barY + 1, w, barHeight -2);
    p.fillRect(r1, _gradient);

    // 100% -> 2/3
    int lineX = (width() - 2) * 2 / 3;
    QPen pen(QColor("black"), 1, Qt::PenStyle::DotLine);
    p.setPen(pen);
    p.drawLine(lineX, barY + 1, lineX, barY + barHeight - 1);


    p.end();
}

void VolumeBar::setCurrValue(int curr){
    _currValue = curr;
    update();
    QPoint center = geometry().center();
    QToolTip::showText(parentWidget()->mapToGlobal(center), QStringLiteral("%1 %").arg(_currValue));
}

int VolumeBar::pointToValue(QPoint p){
    int x = p.x();
    int value = qBound(0, x * _maxValue / (width() - 2), _maxValue);
    return value;
}

void VolumeBar::wheelEvent(QWheelEvent *e) {
    int delta = e->angleDelta().y();
    emit valueStep(delta > 0);
}

void VolumeBar::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::MouseButton::LeftButton){
        _pressed = true;
        int value = pointToValue(e->pos());
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    QToolTip::showText(e->globalPosition().toPoint(), QStringLiteral("%1 %").arg(value));
#else
    QToolTip::showText(e->globalPos(), QStringLiteral("%1 %").arg(value));
#endif
        emit valueChanged(value);
        e->accept();
    }
    else
        e->ignore();
}

void VolumeBar::mouseMoveEvent(QMouseEvent *e) {
    int value = pointToValue(e->pos());
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    QToolTip::showText(e->globalPosition().toPoint(), QStringLiteral("%1 %").arg(value));
#else
    QToolTip::showText(e->globalPos(), QStringLiteral("%1 %").arg(value));
#endif
    if(_pressed) {
        emit valueChanged(value);
    }
    e->accept();

}

void VolumeBar::mouseReleaseEvent(QMouseEvent *e) {
    if(_pressed && e->button() == Qt::MouseButton::LeftButton){
        _pressed = false;
        int value = pointToValue(e->pos());
        emit valueChanged(value);
        e->accept();
    }
    else
        e->ignore();
};
