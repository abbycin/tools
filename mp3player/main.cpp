#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.setFixedHeight(w.height());
    w.setFixedWidth(w.width());
    w.setWindowTitle("Abby's MP3 Player");
    if(argc != 2)
        w.setup("/Data/Music");
    else
        w.setup(argv[1]);
    w.show();

    return a.exec();
}
