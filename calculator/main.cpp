#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("Abby's Calculator");
    w.setFixedHeight(w.height());
    w.setFixedWidth(w.width());
    w.show();

    return a.exec();
}
