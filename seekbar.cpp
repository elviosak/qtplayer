#include "seekbar.h"
#include "mpvwidget.h"

#include <QMouseEvent>
#include <QToolTip>
#include <QPainter>

#include <QDebug>

SeekBar::SeekBar(MpvWidget *mpv, QWidget *parent)
    : QFrame(parent)
    , _mpv(mpv)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumWidth(80);

    connect(_mpv, &MpvWidget::demuxerStateChanged, this, &SeekBar::demuxerChanged);
}
void SeekBar::paintEvent(QPaintEvent */*e*/){
    QPainter p(this);
    int barHeight = height() / 2.5;
    int barY = (height() - barHeight) / 2;

    p.fillRect(0, barY, width(), barHeight, QColor(221,221,221)); //#ddd
    double w = (width() - 2) * _currValue / _maxValue;
    QRectF r1(1, barY + 1, w, barHeight -2);
    p.fillRect(r1, QColor(34,170,68)); //#2a4

    int seekHeight = barHeight / 4;
    int seekY = barY + barHeight - seekHeight - 1;
    for (auto i = _seekableRanges.cbegin(); i != _seekableRanges.cend(); ++i) {
        double x1 = (width() - 2) * (*i).start / _maxValue;
        double x2 = (width() - 2) * (*i).end / _maxValue;
        QRectF r(x1 + 1, seekY, x2 - x1, seekHeight);
        p.fillRect(r, QColor(51,102,153));//#369
    }
    p.end();
}

void SeekBar::demuxerChanged(QVariant value)
{
    auto demuxer = value.value<QMap<QString, QVariant>>();
    auto list = demuxer["seekable-ranges"].value<QVariantList>();
    _seekableRanges.clear();
    for (int i = 0; i < list.count(); ++i) {
        auto range = list.at(i).value<QMap<QString, QVariant>>();
        auto start = qBound<double>(0, range["start"].toDouble(), _maxValue);
        auto end = qBound<double>(0, range["end"].toDouble(), _maxValue);
        Range s{start, end};
        _seekableRanges.append(s);
    }
    update();
}

void SeekBar::setMaxValue(int max){
    _maxValue = max;
    update();
}

void SeekBar::setCurrValue(int curr){
    _currValue = curr;
    update();
}

int SeekBar::pointToValue(QPoint p){
    int pos = p.x();
    int value = ((double)_maxValue * pos / width());
    int newValue = qBound<int>(0, value, _maxValue -1);
    return newValue;
}

QString SeekBar::formatTime(int time){
    int sec = time % 60;
    int min = (int)(time / 60);
    int hour = (int)(min / 60);
    min = min % 60;
    return QStringLiteral("%1:%2:%3")
            .arg(hour)
            .arg(min, 2, 10, QLatin1Char('0'))
            .arg(sec, 2, 10, QLatin1Char('0'));
}

void SeekBar::wheelEvent(QWheelEvent *e) {
    int delta = e->angleDelta().y();
    emit valueStep(delta > 0);
}
void SeekBar::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::MouseButton::LeftButton){
        _pressed = true;
        int value = pointToValue(e->pos());
        emit valueChanged(value);
        e->accept();
    }
    else
        e->ignore();
}
void SeekBar::mouseMoveEvent(QMouseEvent *e) {
    int value = pointToValue(e->pos());
    QToolTip::showText(e->globalPos(), formatTime(value));
    if(_pressed) {
        emit valueChanged(value);
        e->accept();
    }
    else {
        e->ignore();
    }

}
void SeekBar::mouseReleaseEvent(QMouseEvent *e) {
    if(_pressed && e->button() == Qt::MouseButton::LeftButton){
        _pressed = false;
        int value = pointToValue(e->pos());
        emit valueChanged(value);
        e->accept();
    }
    else
        e->ignore();
};
