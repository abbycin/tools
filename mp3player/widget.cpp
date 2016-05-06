#include "widget.h"
#include "ui_widget.h"
#include <QAction>

namespace nm {
    namespace {
        class Travel
        {
        public:
            void travel(QString path)
            {
                QFileInfo info(path);
                if(info.isDir())
                {
                    QDir dir(path);
                    QStringList items = dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
                    for(const auto &x: items)
                    {
                        travel(dir.canonicalPath().append("/") + x);
                    }
                }
                else
                {
                    info.setFile(path);
                    if(info.suffix() == "mp3" || info.suffix() == "flac" || info.suffix() == "ape")
                    {
                        l.push_back(QUrl::fromLocalFile(path));
                    }
                }
            }

            QList<QUrl> get()
            {
                return l;
            }
        private:
            QList<QUrl> l;
        };
    }

    QList<QUrl> LoadMedia(const QString dir)
    {
        QDir d(dir);
        Travel t;
        t.travel(d.canonicalPath());
        return t.get();
    }
}

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    if(parent)
        player = new QMediaPlayer(parent);
    else
        player = new QMediaPlayer(this);
    ui->setupUi(this);
    everything_ok = true;
    connect(player, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(on_currentMediaChanged(QMediaContent)));
    setWindowIcon(QIcon(":/tray_icon.png"));

    next = new QAction("Next", this);
    prev = new QAction("Previous", this);
    control = new QAction("Play", this);
    shuffle = new QAction("Shuffle", this);
    ac_restore = new QAction("Restore", this);
    ac_quit = new QAction("Exit", this);

    tray_menu = new QMenu(this);
    tray_menu->addAction(prev);
    tray_menu->addAction(next);
    tray_menu->addAction(control);
    tray_menu->addAction(shuffle);
    tray_menu->addAction(ac_restore);
    tray_menu->addAction(ac_quit);

    connect(next, &QAction::triggered, this, &Widget::on_next_clicked);
    connect(prev, &QAction::triggered, this, &Widget::on_prev_clicked);
    connect(control, &QAction::triggered, this, &Widget::on_start_clicked);
    connect(shuffle, &QAction::triggered, this, &Widget::on_shuffle_clicked);
    connect(ac_restore, &QAction::triggered, this, &Widget::showNormal);
    connect(ac_quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    tray_icon = new QSystemTrayIcon(this);
    tray_icon->setContextMenu(tray_menu);
    tray_icon->setIcon(QIcon(":/tray_icon.png"));
    tray_icon->show();

    connect(tray_icon, &QSystemTrayIcon::activated, this, &Widget::on_tray_icon_clicked);
}

Widget::~Widget()
{
    delete next;
    delete prev;
    delete control;
    delete shuffle;
    delete ac_restore;
    delete ac_quit;
    delete tray_menu;
    delete tray_icon;
    delete player;
    delete ui;
}

int Widget::load()
{
    QDir d(music_path);
    music_path = d.absolutePath();
    if(!d.exists())
    {
        ui->song_label->setText(music_path + " not exsits!");
        return -1;
    }

    QList<QUrl> items = nm::LoadMedia(music_path);
    if(items.size() == 0)
    {
        ui->song_label->setText("no meida file in " + music_path);
        return -1;
    }
    QList<QMediaContent> contents;

    for(auto &x: items)
    {
        contents.push_back(x);
    }

    list.addMedia(contents);
    return 0;
}

QString Widget::current_playing()
{
    auto raw = list.currentMedia().canonicalUrl().path().split('/');
    auto res = raw[raw.size() - 1];
    return res;
}

void Widget::setup(const QString &path)
{
    music_path = path;
    if(load() < 0)
    {
        everything_ok = false;
    }
    else
    {
        list.setPlaybackMode(QMediaPlaylist::Loop);
        player->setPlaylist(&list);
        player->setVolume(50);
        ui->start->setText("play");
        ui->song_label->setText(music_path);
        ui->volume_bar->setValue(player->volume());
        ui->volume_label->setText(QString::number(player->volume()));
    }
}

void Widget::on_start_clicked()
{
    if(!everything_ok)
        return;
    if(player->state() == QMediaPlayer::PlayingState)
    {
        control->setText("Play");
        ui->start->setText("play");
        ui->song_label->setText(current_playing());
        player->pause();
    }
    else
    {
        control->setText("Pause");
        ui->start->setText("pause");
        ui->song_label->setText(current_playing());
        player->play();
    }
}

void Widget::on_next_clicked()
{
    if(!everything_ok)
        return;
    player->stop();
    player->playlist()->next();
    on_start_clicked();
}

void Widget::on_prev_clicked()
{
    if(!everything_ok)
        return;
    player->stop();
    player->playlist()->previous();
    on_start_clicked();
}

void Widget::on_shuffle_clicked()
{
    if(!everything_ok)
        return;
    player->stop();
    player->playlist()->shuffle();
    on_start_clicked();
}

void Widget::on_volume_bar_valueChanged(int value)
{
    if(!everything_ok)
        return;
    player->setVolume(value);
    ui->volume_label->setText(QString::number(value));
}

void Widget::on_currentMediaChanged(QMediaContent media)
{
    auto x = media.canonicalUrl().path().split('/');
    QString s;
    if(x.size() > 0)
        s = x[x.size() - 1];
    ui->song_label->setText(s);
    ui->song_label->setToolTip(s);
}


void Widget::on_tray_icon_clicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        if(this->isVisible())
            this->hide();
        else
            this->showNormal();
    }
}
