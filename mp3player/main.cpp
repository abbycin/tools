#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.setFixedHeight(w.height());
    w.setFixedWidth(w.width());
    w.setWindowTitle("Abby's MP3 Player");
    w.setup("/Data/Music");
    w.show();

    return a.exec();
}
