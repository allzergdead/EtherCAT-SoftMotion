#include "ethercatmaster.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EtherCatMaster w;
    //w.show();

    //return a.exec();
    return a.exec();
}
