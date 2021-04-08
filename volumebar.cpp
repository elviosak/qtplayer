#include "volumebar.h"

#include <QMouseEvent>
#include <QToolTip>

VolumeBar::VolumeBar(QWidget *parent)
    : QFrame(parent)
{
    _pressed = false;
    _width = 75;
    _maxValue = 150;
    _currValue = 0;
    _currWidth = 0;

    setStyleSheet(
"#fullbar{"
"   background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
"                   stop: 0 #8f8,"
"                   stop: 0.64 #080,"
"                   stop: 0.66 #da0,"
"                   stop: 1 #f44);"
"border:1px solid #ddd"
"}"
"#currentbar{"
"   background: #ddd;"
"   border: 1px solid #ddd;"
"   border-left-width: 0px;"
"}"
"#barline{"
"   background: transparent;"
"   border-right-width: 1px;"
"   border-right-style: dotted;"
"   border-right-color: black;"
"}");

    setMouseTracking(true);
    setObjectName("fullbar");
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFrameShape(Shape::NoFrame);
    setFixedSize(_width + 1, 12);
    _currentBar = new QWidget(this);
    _currentBar->setMouseTracking(true);
    _currentBar->setObjectName("currentbar");

    _currentBar->setFixedSize(_width + 10, 12);
    _currentBar->move(0, 0);
    auto line = new QFrame(this);
    line->setMouseTracking(true);
    line->setFrameShape(Shape::Box);
    line->setObjectName("barline");
    line->setFixedSize(51, 12);
    line->move(0,0);
}

void VolumeBar::setMaxValue(int max){
    _maxValue = max;
    resizeBar();
}
void VolumeBar::setCurrValue(int curr){
    _currValue = curr;
    resizeBar();
    QPoint center = geometry().center();
    QToolTip::showText(parentWidget()->mapToGlobal(center), QStringLiteral("%1 %").arg(_currValue));
}

void VolumeBar::resizeBar(){
    _currWidth = _currValue / 2;
    _currentBar->move(_currWidth, 0);
};

int VolumeBar::pointToValue(QPoint p){
    int pos = p.x();
    int value = qBound(0, pos * 2, 150);
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
    if(_pressed) {
        QToolTip::showText(e->globalPos(), QStringLiteral("%1 %").arg(value));
        emit valueChanged(value);
        e->accept();
    }
    else {
        QToolTip::showText(e->globalPos(), QStringLiteral("%1 %").arg(_currValue));
        e->ignore();
    }

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
void VolumeBar::resizeEvent(QResizeEvent *e) {
    _width = e->size().width() - 2;
    resizeBar();
};

