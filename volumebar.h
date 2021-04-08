#ifndef VOLUMEBAR_H
#define VOLUMEBAR_H

#include <QFrame>

class VolumeBar: public QFrame
{
    Q_OBJECT
public:
    VolumeBar(QWidget *parent = nullptr);

    void setMaxValue(int max);
    void setCurrValue(int curr);

private:
    bool _pressed;
    int _width;
    int _maxValue;
    int _currValue;
    int _currWidth;

    QWidget *_currentBar;

    void resizeBar();
    int pointToValue(QPoint p);
protected:
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

signals:
    void valueChanged(int value);
    void valueRelative(int value);
};

#endif // VOLUMEBAR_H
