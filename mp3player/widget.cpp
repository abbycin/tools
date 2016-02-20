#include "widget.h"
#include "ui_widget.h"

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
}

Widget::~Widget()
{
    delete player;
    delete ui;
}

int Widget::load()
{
    QString path = music_path;
    if(path[path.size() - 1] != '/')
        path.append('/');
    QDir d(path);
    if(!d.exists())
        return -1;
    QStringList filters;
    filters << "*.mp3";
    QStringList items = d.entryList(filters);
    QList<QMediaContent> contents;

    for(auto &x: items)
    {
        contents.push_back(QUrl::fromLocalFile(path + x));
    }

    list.addMedia(contents);
    return 0;
}

void Widget::setup(const QString &path)
{
    music_path = path;
    if(load() < 0)
    {
        ui->song_label->setText(music_path + " not exsits!");
        everything_ok = false;
    }
    else
    {
        list.setPlaybackMode(QMediaPlaylist::Loop);
        player->setPlaylist(&list);
        player->setVolume(50);
        ui->song_label->setText(path);
        ui->volume_bar->setValue(player->volume());
        ui->volume_label->setText(QString::number(player->volume()));
    }
}

void Widget::on_start_clicked()
{
    if(!everything_ok)
        return;
    auto raw = list.currentMedia().canonicalUrl().path();
    auto song = raw.split('/');
    QString display;
    if(player->state() == QMediaPlayer::PlayingState)
    {
        display = "Paused: ";
        display += song[song.size() - 1];
        ui->song_label->setText(display);
        player->pause();
    }
    else
    {
        display = "Playing: ";
        display += song[song.size() - 1];
        ui->song_label->setText(display);
        player->play();
    }
}

void Widget::on_next_clicked()
{
    if(!everything_ok)
        return;
    on_start_clicked();
    player->playlist()->next();
    on_start_clicked();
}

void Widget::on_prev_clicked()
{
    if(!everything_ok)
        return;
    if(player->state() == QMediaPlayer::PlayingState)
        on_start_clicked();
    player->playlist()->previous();
    on_start_clicked();
}

void Widget::on_shuffle_clicked()
{
    if(!everything_ok)
        return;
    if(player->state() == QMediaPlayer::PlayingState)
        player->stop();
    player->playlist()->shuffle();
    on_start_clicked();
}

void Widget::on_volume_bar_sliderReleased()
{
    if(!everything_ok)
        return;
    player->setVolume(ui->volume_bar->value());
    ui->volume_label->setText(QString::number(ui->volume_bar->value()));
}
