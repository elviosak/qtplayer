#ifndef VOLUMEBAR_H
#define VOLUMEBAR_H

#include <QWidget>

class VolumeBar: public QWidget
{
    Q_OBJECT
public:
    VolumeBar(QWidget *parent = nullptr);

    void setCurrValue(int curr);

private:
    bool _pressed;
    int _width;
    int _maxValue;
    int _currValue;
    QLinearGradient _gradient;

    void createGradient();
    int pointToValue(QPoint p);
protected:
    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

signals:
    void valueChanged(int value);
    void valueStep(bool increase);
};

#endif // VOLUMEBAR_H
