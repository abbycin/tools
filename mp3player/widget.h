#ifndef WIDGET_H
#define WIDGET_H
//
#if !defined(__linux__) && !defined(__FreeBSD__)  && !defined(__APPLE__)
#error "platform not support!"
#endif

#include <QWidget>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaPlaylist>
#include <QDir>
#include <QStringList>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);

    ~Widget();
    void setup(const QString &path);

private slots:
    void on_start_clicked();

    void on_prev_clicked();

    void on_next_clicked();

    void on_shuffle_clicked();

    void on_volume_bar_valueChanged(int value);

    void on_currentMediaChanged(QMediaContent media);

private:
    Ui::Widget *ui;
    QMediaPlayer *player;
    QMediaPlaylist list;
    QString music_path;
    bool everything_ok;

    int load();

    QString current_playing();
};

#endif // WIDGET_H
