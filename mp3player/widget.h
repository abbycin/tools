#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaPlaylist>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QCloseEvent>

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

protected:
    void contextMenuEvent(QContextMenuEvent *e)
    {
        tray_menu->exec(QCursor::pos());
        e->accept();
    }

    void closeEvent(QCloseEvent *e)
    {
        tray_icon->showMessage(this->windowTitle(), "Player will run in background");
        this->hide();
        e->ignore();
    }

private slots:
    void on_start_clicked();

    void on_prev_clicked();

    void on_next_clicked();

    void on_shuffle_clicked();

    void on_volume_bar_valueChanged(int value);

    void on_currentMediaChanged(QMediaContent media);

    void on_tray_icon_clicked(QSystemTrayIcon::ActivationReason reason);

private:
    Ui::Widget *ui;
    QMediaPlayer *player;
    QMediaPlaylist list;
    QString music_path;
    bool everything_ok;

    QSystemTrayIcon *tray_icon;
    QMenu *tray_menu;
    QAction *ac_quit, *ac_restore, *next, *prev, *control, *shuffle;

    int load();

    QString current_playing();
};

#endif // WIDGET_H
