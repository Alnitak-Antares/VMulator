#include "vmulator.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VMulator w;
    w.setFixedSize(Window_width,Window_height);
    w.show();
    return a.exec();
}
