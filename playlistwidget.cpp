#include "playlistwidget.h"
#include "mpvwidget.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QMimeData>
#include <QMimeDatabase>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QDir>

PlaylistWidget::PlaylistWidget(MpvWidget *mpv, QWidget *parent)
    : QDockWidget(parent)
    , _mpv(mpv)
{
    setWindowTitle("Playlist");
    setFeatures(QDockWidget::DockWidgetMovable
                | QDockWidget::DockWidgetVerticalTitleBar);
//    setAcceptDrops(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    _urlLine = new QLineEdit;

    _urlAddBtn = new QPushButton(QIcon(":/addplaylist"),"Add");
    auto urlBox = new QHBoxLayout;
    urlBox->addWidget(_urlLine);
    urlBox->addWidget(_urlAddBtn);


    _btnBar = new QToolBar;
    _btnBar->setOrientation(Qt::Orientation::Vertical);
    _btnBar->addAction(QIcon(":/up"), "Up");
    _btnBar->addAction(QIcon(":/down"), "Down");
    _btnBar->addSeparator();
    _btnBar->addAction(QIcon(":/trash"), "Remove");

    _btnBar->addSeparator();
    _btnBar->addAction(QIcon(":/copy"), "Copy");
    _model = new QStandardItemModel;
    _model->setColumnCount(4);
    _model->setHorizontalHeaderLabels(QStringList()
                                      << "Title"
                                      << "Duration"
                                      /*<< "Filename"*/);

    _table = new QTableView;

    _table->setModel(_model);
    // 0 title, 1 duration, 2 filename, 3 id
    _table->setColumnHidden(2, true);
    _table->setColumnHidden(3, true);
    _table->setSortingEnabled(false);
    _table->setEditTriggers(QTableView::EditTrigger::NoEditTriggers);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->verticalHeader()->hide();
    _table->setDragDropMode(QAbstractItemView::NoDragDrop);

    auto header = _table->horizontalHeader();
    header->setSectionResizeMode(0,QHeaderView::Stretch);
    header->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    header->setDefaultAlignment(Qt::AlignCenter);



    auto listBox = new QHBoxLayout;
    listBox->addWidget(_table);
    listBox->addWidget(_btnBar);

    auto mainBox = new QVBoxLayout;
    mainBox->addLayout(urlBox);
    mainBox->addLayout(listBox);

    _widget = new QWidget;
    _widget->setLayout(mainBox);
    setWidget(_widget);

    // CONTROLS
    connect(_urlLine, &QLineEdit::returnPressed, this, [=]{
        addUrl(_urlLine->text());
        _urlLine->setText(QString());
        _urlLine->clearFocus();
    });
    connect(_urlAddBtn, &QPushButton::clicked, this, [=]{
        addUrl(_urlLine->text());
        _urlLine->setText(QString());
    });
    connect(_table, &QTableView::activated, this, [=]
                (QModelIndex index) {
        _mpv->command(QStringList()
                      << "playlist-play-index"
                      << QString::number(index.row()));
        if (_mpv->getProperty("pause").toBool())
            _mpv->setProperty("pause", false);
    });
    connect(_btnBar, &QToolBar::actionTriggered, this, [=] (QAction *a){
        auto index = _table->currentIndex();
        if(!index.isValid())
            return;

        int current = index.row();
        if (a->text() == "Up"){
            if (current > 0){
                _mpv->command(QStringList()
                              << "playlist-move"
                              << QString::number(current)
                              << QString::number(current - 1));
                _lastAction = "up";
            }
        }
        else if (a->text() == "Down") {
            if (current + 1 < _model->rowCount()){
                _mpv->command(QStringList()
                              << "playlist-move"
                              << QString::number(current)
                              << QString::number(current + 2));
                _lastAction = "down";
            }
        }
        else if (a->text() == "Remove") {
            _mpv->command(QStringList()
                          << "playlist-remove"
                          << QString::number(current));
            _lastAction = "remove";
        }
        else if (a->text() == "Copy") {
            auto selectedRows = _table->selectionModel()->selectedRows(2);
            qDebug() << "selected" << selectedRows.count();
            foreach (auto item, selectedRows) {
//                qDebug() << item.column() << item.row() << item.data().toString();
                qApp->clipboard()->setText(item.data().toString());
            }
//            if (selectedRows.count() > 0) {
//                auto fileName = _model->data(selectedRows.at(2)).value<QString>();
//                qDebug() << "fileName" << fileName;
//            }

        }
    });

    // MPV LISTENERS
    connect(_mpv, &MpvWidget::playlistChanged,
            this, &PlaylistWidget::playlistChanged);

    connect(_mpv, &MpvWidget::mediaTitleChanged, this, [=] (QString title) {
//        int index = _mpv->getProperty("playlist-playing-pos").toInt();
        auto filename = _mpv->getProperty("path").toString();
//        auto titleItem = _model->item(index, 0);
        auto items = _model->findItems(filename, Qt::MatchExactly, 2);
        // 0 title, 1 duration, 2 filename, 3 id
        for (auto i = items.cbegin(); i != items.cend(); ++i) {
            auto indexTitle = (*i)->index().siblingAtColumn(0);
            _model->itemFromIndex(indexTitle)->setText(title);
        }
//        auto filenameItem = _model->item(index, 2);
//        if (titleItem && filename == filenameItem->text()){
//            titleItem->setText(title);
//        }
    });

    connect(_mpv, &MpvWidget::durationChanged, this, [=] (int time) {
        int index = _mpv->getProperty("playlist-playing-pos").toInt();
        auto filename = _mpv->getProperty("path").toString();
        auto durationItem = _model->item(index, 1);
        auto filenameItem = _model->item(index, 2);
        if (durationItem && filename == filenameItem->text()){
            QString duration = formatTime(time);
            durationItem->setText(duration);
        }
    });
    connect(_mpv, &MpvWidget::playlistPosChanged, this, [=] (int pos) {
        QFont fnormal = _table->font();
        fnormal.setBold(false);
        QFont fbold = _table->font();
        fbold.setBold(true);
        for (int i = 0; i < _model->rowCount(); ++i){
            if (i == pos){
                _model->item(i, 0)->setFont(fbold);
                _model->item(i, 1)->setFont(fbold);
            }
            else {
                _model->item(i, 0)->setFont(fnormal);
                _model->item(i, 1)->setFont(fnormal);
            }

        }
    });

}

void PlaylistWidget::addUrl(QString url){
    QString newUrl = url.trimmed();

    if (newUrl.isEmpty())
        return;
    QString localFile = QUrl(url).toLocalFile();
    QFileInfo info(localFile);
    if (info.exists()) {
        if (info.isFile())
            addFile(info.absoluteFilePath());
        else if (info.isDir())
            addDir(info.absoluteFilePath());
    }
    else {
    _mpv->command(QStringList() << "loadfile" << newUrl << "append-play");
    if (_mpv->getProperty("eof-reached").toBool()
            || _mpv->getProperty("idle-active").toBool())
        _mpv->setProperty("pause", false);
    }
};

void PlaylistWidget::addFile(QString filePath) {
    QMimeDatabase db;
    auto mime = db.mimeTypeForFile(filePath);
    auto name = mime.name();
    if (!name.startsWith(QStringLiteral("video")) && !name.startsWith(QStringLiteral("audio")))
        return;
    _mpv->command(QStringList() << "loadfile" << filePath << "append-play");
    if (_mpv->getProperty("eof-reached").toBool()
            || _mpv->getProperty("idle-active").toBool())
        _mpv->setProperty("pause", false);
}

void PlaylistWidget::addDir(QString path) {
    QDir dir(path);
    auto entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (auto i = entries.cbegin(); i != entries.cend(); ++i){
        auto info = *i;
        if (info.isFile())
            addFile(info.absoluteFilePath());
        else if (info.isDir())
            addDir(info.absoluteFilePath());
    }
}

QString PlaylistWidget::formatTime(int time){
    int sec = time % 60;
    int min = (int)(time / 60);
    int hour = (int)(min / 60);
    min = min % 60;
    return QStringLiteral("%1:%2:%3")
            .arg(hour)
            .arg(min, 2, 10, QLatin1Char('0'))
            .arg(sec, 2, 10, QLatin1Char('0'));
}



void PlaylistWidget::playlistChanged(QVariant playlist) {
    QModelIndex index = _table->currentIndex();

    auto list = playlist.value<QVariantList>();
    int i = 0;
    for (; i < list.count(); ++i) {
        auto newItem = list.at(i).value<QMap<QString, QVariant>>();
        QString filename = newItem["filename"].toString();
        QString id = QString::number(newItem["id"].toInt());
        QList<QStandardItem*> itemRow, find;

        find = _model->findItems(id, Qt::MatchExactly, 3);
        if (find.count() > 0) {
            if (find.first()->index().row() == i) {
                continue;
            }
            else {
                itemRow = _model->takeRow(find.first()->row());
            }
        }
        else {
            auto titleItem = new QStandardItem(filename);
            auto durationItem = new QStandardItem(QString());
            auto filenameItem = new QStandardItem(filename);
            auto idItem = new QStandardItem(id);
            durationItem->setTextAlignment(Qt::AlignCenter);
            itemRow.append(titleItem);
            itemRow.append(durationItem);
            itemRow.append(filenameItem);
            itemRow.append(idItem);
        }
        _model->insertRow(i, itemRow);
    }
    for (; i < _model->rowCount(); ++i) {
        auto rowItems = _model->takeRow(i);
        foreach (auto item, rowItems) {
            item->~QStandardItem();
        }
    }
    if (index.isValid()){
        if (_lastAction == "up")
            _table->setCurrentIndex(index.siblingAtRow(index.row() - 1));
        else if (_lastAction == "down")
            _table->setCurrentIndex(index.siblingAtRow(index.row() + 1));
        else if (_lastAction == "remove") {
            int newRow = qMin(index.row(), _model->rowCount()-1);
            _table->setCurrentIndex(index.siblingAtRow(newRow));
        }
        else {
            _table->setCurrentIndex(index);
        }
    }
    _lastAction = QString();

};

void PlaylistWidget::resizeEvent(QResizeEvent *e)
{
    emit geometryChanged(geometry());
    return QDockWidget::resizeEvent(e);

}
