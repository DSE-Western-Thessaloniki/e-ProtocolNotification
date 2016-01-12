#include "epn_dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EPN_Dialog w;
    w.getUpdate();
    w.show();

    return a.exec();
}
