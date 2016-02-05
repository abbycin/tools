#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_btn_lparent_clicked();

    void on_btn_rparent_clicked();

    void on_btn_quit_clicked();

    void on_btn_clear_clicked();

    void on_btn7_clicked();

    void on_btn8_clicked();

    void on_btn9_clicked();

    void on_btn_divide_clicked();

    void on_btn4_clicked();

    void on_btn5_clicked();

    void on_btn6_clicked();

    void on_btn_multi_clicked();

    void on_btn1_clicked();

    void on_btn2_clicked();

    void on_btn3_clicked();

    void on_btn_minus_clicked();

    void on_btn_dot_clicked();

    void on_btn0_clicked();

    void on_btn_plus_clicked();

    void on_btn_equal_clicked();

    void on_btn_delete_clicked();

private:
    Ui::Widget *ui;
    QString value;
    std::vector<std::string> split(QString &s);
};

#endif // WIDGET_H
