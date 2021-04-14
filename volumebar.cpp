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

//    setStyleSheet(
//"#fullbar{"
//"   background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
//"                   stop: 0 #8f8,"
//"                   stop: 0.64 #080,"
//"                   stop: 0.66 #da0,"
//"                   stop: 1 #f44);"
//"   margin: 0px;"
//"   padding: 0px;"
//"border:1px solid #ddd"
//"}"
//"#currentbar{"
//"   background: #ddd;"
//"   margin: 0px;"
//"   padding: 0px;"
//"   border: 1px solid #ddd;"
//"   border-left-width: 0px;"
//"}"
//"#barline{"
//"   background: transparent;"
//"   border: 1px transparent #ddd;"
//"   margin: 0px;"
//"   padding: 0px;"
//"   border-right-width: 1px;"
//"   border-right-style: dotted;"
//"   border-right-color: black;"
//"}");

    setMouseTracking(true);
    setObjectName("fullbar");
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setFixedWidth(_width);
}
void VolumeBar::paintEvent(QPaintEvent */*e*/){
    QPainter p(this);
    int barHeight = height() / 2.5;
    int barY = (height() - barHeight) / 2;

    p.fillRect(0, barY, width(), barHeight, QColor(221,221,221)); //#ddd

    QLinearGradient g(0, 0, width()-2, 0);//(QPoint(1,1), QPoint(width() - 1, height() - 1));
    //"                   stop: 0 #8f8," // 136,255,136
    //"                   stop: 0.64 #080," // 0,136,0
    //"                   stop: 0.66 #da0,"
    //"                   stop: 1 #f44);"
//    g.setColorAt(0, QColor(136,255,136));
//    g.setColorAt(0.66, QColor(0,136,0));
//    g.setColorAt(0.67, QColor(221,170,0));
//    g.setColorAt(1, QColor(255,68,68));
    QGradientStops stops;
    stops << QGradientStop(0, QColor(136,255,136))
    << QGradientStop(0.66, QColor(0,136,0))
    << QGradientStop(0.67, QColor(221,170,0))
    << QGradientStop(1, QColor(255,68,68));
    g.setStops(stops);

    double w = (width() - 2) * _currValue / _maxValue;
    QRectF r1(1, barY + 1, w, barHeight -2);
    //p.fillRect(r1, QColor(34,170,68)); //#2a4
//    QBrush br(g);
//    br.setStyle(Qt::LinearGradientPattern);
    p.fillRect(r1, g); //#2a4
    // 100% -> 2/3
    int lineX = (width() - 2) * 2 / 3;
    QPen pen(QColor("black"), 1, Qt::PenStyle::DotLine);
    p.setPen(pen);
    p.drawLine(lineX, barY + 1, lineX, barY + barHeight - 1);


    p.end();
}
//void VolumeBar::setMaxValue(int max){
//    _maxValue = max;
//    update();
//}
void VolumeBar::setCurrValue(int curr){
    _currValue = curr;
    update();
    QPoint center = geometry().center();
    QToolTip::showText(parentWidget()->mapToGlobal(center), QStringLiteral("%1 %").arg(_currValue));
}

//void VolumeBar::resizeBar(){
//    _currWidth = _currValue / 2;
//    _currentBar->move(_currWidth, 0);
//};

int VolumeBar::pointToValue(QPoint p){
    int x = p.x();
    int value = qBound(0, x * _maxValue / (width() - 2), _maxValue);
    return value;
}

void VolumeBar::wheelEvent(QWheelEvent *e) {
    int delta = e->angleDelta().y();
    if(delta > 0)
        emit valueRelative(5);
    else
        emit valueRelative(-5);
}
void VolumeBar::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::MouseButton::LeftButton){
        _pressed = true;
        int value = pointToValue(e->pos());
        QToolTip::showText(e->globalPos(), QStringLiteral("%1 %").arg(value));
        emit valueChanged(value);
        e->accept();
    }
    else
        e->ignore();
}
void VolumeBar::mouseMoveEvent(QMouseEvent *e) {
    int value = pointToValue(e->pos());
    QToolTip::showText(e->globalPos(), QStringLiteral("%1 %").arg(value));
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
//void VolumeBar::resizeEvent(QResizeEvent *e) {
//    _width = e->size().width() - 2;
//    resizeBar();
//};

