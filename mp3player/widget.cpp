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
    connect(player, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(on_currentMediaChanged(QMediaContent)));
}

Widget::~Widget()
{
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
    QStringList filters;
    filters << "*.mp3";
    QStringList items = d.entryList(filters);
    if(items.size() == 0)
    {
        ui->song_label->setText("no mp3 file in " + music_path);
        return -1;
    }
    QList<QMediaContent> contents;

    if(music_path[music_path.size() - 1] != '/')
        music_path.append('/');

    for(auto &x: items)
    {
        contents.push_back(QUrl::fromLocalFile(music_path + x));
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
        ui->start->setText("play");
        ui->song_label->setText(current_playing());
        player->pause();
    }
    else
    {
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
