#include "sportstracker.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SportsTracker w;
    w.show();
    return a.exec();
}
