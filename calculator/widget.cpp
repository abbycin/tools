#include "widget.h"
#include "ui_widget.h"
#include "rpn.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->label_expression->setText("");
    ui->label_output->setText("");
}

Widget::~Widget()
{
    delete ui;
}

std::vector<std::string> Widget::split(QString &s)
{
    QString number, punct;
    std::vector<std::string> res;
    std::string ss = s.toStdString();
    std::string token{"()+-*/."};

    for(auto x: ss)
    {
        if(x <= '9' && x >= '0')
        {
            number.append(x);
        }
        else if(token.find(x) != token.npos)
        {
            if(x == '.')
                number.append(x);
            else
            {
                if(!number.isEmpty())
                    res.push_back(number.toStdString());
                number.clear();
                punct.append(x);
                res.push_back(punct.toStdString());
                punct.clear();
            }
        }
    }
    if(!number.isEmpty())
        res.push_back(number.toStdString());
    return res;
}

void Widget::on_btn_lparent_clicked()
{
    value.append('(');
    ui->label_expression->setText(value);
}

void Widget::on_btn_rparent_clicked()
{
    value.append(')');
    ui->label_expression->setText(value);
}

void Widget::on_btn_quit_clicked()
{
    qApp->quit();
}

void Widget::on_btn_clear_clicked()
{
    ui->label_expression->setText("");
    ui->label_output->setText("");
    value.clear();
}

void Widget::on_btn7_clicked()
{
    value.append('7');
    ui->label_expression->setText(value);
}

void Widget::on_btn8_clicked()
{
    value.append('8');
    ui->label_expression->setText(value);
}

void Widget::on_btn9_clicked()
{
    value.append('9');
    ui->label_expression->setText(value);
}

void Widget::on_btn_divide_clicked()
{
    value.append('/');
    ui->label_expression->setText(value);
}

void Widget::on_btn4_clicked()
{
    value.append('4');
    ui->label_expression->setText(value);
}

void Widget::on_btn5_clicked()
{
    value.append('5');
    ui->label_expression->setText(value);
}

void Widget::on_btn6_clicked()
{
    value.append('6');
    ui->label_expression->setText(value);
}

void Widget::on_btn_multi_clicked()
{
    value.append('*');
    ui->label_expression->setText(value);
}

void Widget::on_btn1_clicked()
{
    value.append('1');
    ui->label_expression->setText(value);
}

void Widget::on_btn2_clicked()
{
    value.append('2');
    ui->label_expression->setText(value);
}

void Widget::on_btn3_clicked()
{
    value.append('3');
    ui->label_expression->setText(value);
}

void Widget::on_btn_minus_clicked()
{
    value.append('-');
    ui->label_expression->setText(value);
}

void Widget::on_btn_dot_clicked()
{
    value.append('.');
    ui->label_expression->setText(value);
}

void Widget::on_btn0_clicked()
{
    value.append('0');
    ui->label_expression->setText(value);
}

void Widget::on_btn_plus_clicked()
{
    value.append('+');
    ui->label_expression->setText(value);
}

void Widget::on_btn_equal_clicked()
{
    std::vector<std::string> vs = split(value);

    double res = .0;
    try
    {
        res = rpn::get_answer(vs);
    }
    catch(std::runtime_error &e)
    {
        ui->label_output->setText(e.what());
        return;
    }

    ui->label_output->setText(QString::number(res));
}

void Widget::on_btn_delete_clicked()
{
    if(value.size() > 0)
        value.remove(value.size() - 1, 1);
    ui->label_expression->setText(value);
}
