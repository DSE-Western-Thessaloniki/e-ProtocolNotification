#include <QApplication>
#include "epn_dialog.h"
#include "logger.h"

Logger logger("epn.log");

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EPN_Dialog w;
    w.getUpdate();
    //w.exec();

    return a.exec();
}
