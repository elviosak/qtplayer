#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QDockWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QMap>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QToolBar>

class MpvWidget;

class PlaylistWidget: public QDockWidget
{
    Q_OBJECT
public:
    PlaylistWidget(MpvWidget *mpv, QWidget *parent = nullptr);

    QString formatTime(int time);
    void addUrl(QString url);

private:
    int _draggedIndex;
    QString _lastAction;

    QTableView *_table;
    QStandardItemModel *_model;
    QMap<int, QStandardItem*> stdItems;
    MpvWidget *_mpv;
    QWidget *_widget;
    QLineEdit *_urlLine;
    QPushButton *_urlAddBtn;
    QToolBar *_btnBar;

    void playlistChanged(QVariant playlist);

    void addFile(QString);
    void addDir(QString);

protected:

    void resizeEvent(QResizeEvent *e) override;
signals:
    void geometryChanged(QRect geo);
};

#endif // PLAYLISTWIDGET_H
