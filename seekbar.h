#ifndef SEEKBAR_H
#define SEEKBAR_H

#include <QFrame>
#include <QMap>

struct Range {
    double start;
    double end;
};

class MpvWidget;
class SeekBar: public QFrame
{
    Q_OBJECT
public:
    SeekBar(MpvWidget *mpv, QWidget *parent = nullptr);

    void setMaxValue(int max);
    void setCurrValue(int curr);
    void demuxerChanged(QVariant value);
private:
    MpvWidget *_mpv;
    bool _pressed = false;

    int _maxValue = 1;
    int _currValue = 0;

    QList<Range> _seekableRanges;

    int pointToValue(QPoint p);
    QString formatTime(int time);

protected:
    void paintEvent(QPaintEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

signals:
    void valueChanged(int value);
    void valueRelative(int value);
};

#endif // SEEKBAR_H
