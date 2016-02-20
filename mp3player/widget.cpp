#include "widget.h"
#include "ui_widget.h"
#include <QTextStream>

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
    QString display;
    if(player->state() == QMediaPlayer::PlayingState)
    {
        display = "Paused: ";
        display += current_playing();
        ui->song_label->setText(display);
        player->pause();
    }
    else
    {
        display = "Playing: ";
        display += current_playing();
        ui->song_label->setText(display);
        player->play();
    }
}

void Widget::on_next_clicked()
{
    if(!everything_ok)
        return;
    player->stop();
    player->playlist()->next();
    player->play();
}

void Widget::on_prev_clicked()
{
    if(!everything_ok)
        return;
    player->stop();
    player->playlist()->previous();
    player->play();
}

void Widget::on_shuffle_clicked()
{
    if(!everything_ok)
        return;
    player->stop();
    player->playlist()->shuffle();
    player->play();
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
    QString s = "Playing: ";
    if(x.size() > 0)
        s += x[x.size() - 1];
    ui->song_label->setText(s);
}
