#include "seekbar.h"
#include "mpvwidget.h"

#include <QMouseEvent>
#include <QToolTip>

//#include <QDebug>

SeekBar::SeekBar(MpvWidget *mpv, QWidget *parent)
    : QFrame(parent)
    , _mpv(mpv)
{
    setStyleSheet(
"#fullbar{"
"   border-width: 0px;"
"   margin: 0px;"
"   padding: 0px;"
"   background: #ddd;"
"}"
"#currentbar{"
"   border-width: 0px;"
"   margin: 0px;"
"   padding: 0px;"
"   background: #2a4;"
"}"
"#seekablebar{"
"   border-width: 0px;"
"   margin: 0px;"
"   padding: 0px;"
"   background: #369"
"}");

    setMouseTracking(true);

    setObjectName("fullbar");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumWidth(80);

    setFixedHeight(14);

    _currentBar = new QFrame(this);
    _currentBar->setObjectName("currentbar");
    _currentBar->setFixedHeight(12);
    _currentBar->setBackgroundRole(QPalette::HighlightedText);
    _currentBar->move(1, 1);
    _width = sizeHint().width() - 2;
    connect(_mpv, &MpvWidget::demuxerStateChanged, this, &SeekBar::demuxerChanged);
}
void SeekBar::changeSeekable(double start, double end){
    Seekable seekable;
    double x = _width * start / _maxValue;
    double w = _width * (end - start) / _maxValue;
    x = qBound<double>(0, x, _width);
    w = qBound<double>(0, w, _width - x);
    if (_seekableRanges.contains(start)){
        seekable = _seekableRanges[start];
        seekable.end = end;
    }
    else {
        seekable.bar = new QFrame(this);
        seekable.bar->setObjectName("seekablebar");

        seekable.start = start;
        seekable.end = end;
        _seekableRanges[start] = seekable;
        seekable.bar->setFixedHeight(3);
        seekable.bar->move(1 + (int)x, 10);
        seekable.bar->show();
    }
    seekable.bar->setFixedWidth((int)w);
}
void SeekBar::demuxerChanged(QVariant value)
{
    auto demuxer = value.value<QMap<QString, QVariant>>();
    auto list = demuxer["seekable-ranges"].value<QVariantList>();

    QList<double> parsed;

    for (int i = 0; i < list.count(); ++i) {
        auto range = list.at(i).value<QMap<QString, QVariant>>();
        auto start = range["start"].toDouble();
        auto end = range["end"].toDouble();
        changeSeekable(start, end);
        parsed.append(start);
    }

    for (auto i = _seekableRanges.begin(); i != _seekableRanges.end();) {
        if(!parsed.contains(i.value().start)){
            i.value().bar->deleteLater();
            i = _seekableRanges.erase(i);
        }
        else {
            ++i;
        }
    }
}

void SeekBar::setMaxValue(int max){
    _maxValue = max;
    resizeBar();
}

void SeekBar::setCurrValue(int curr){
    _currValue = curr;
    resizeBar();
}

void SeekBar::resizeBar(){
    double value = _width * _currValue / _maxValue;
    _currWidth = value;
    _currentBar->setFixedWidth(_currWidth);
    if(_mpv->getProperty("demuxer-cache-idle").toBool())
        demuxerChanged(_mpv->getProperty("demuxer-cache-state"));
};

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
    if(delta > 0)
        emit valueRelative(10);
    else
        emit valueRelative(-10);
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
void SeekBar::resizeEvent(QResizeEvent *e) {
    _width = e->size().width() - 2;
    resizeBar();
};
