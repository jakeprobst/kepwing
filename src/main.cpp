#include "kepwing.h"
#include <QApplication>



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Kepwing kepwing;

    kepwing.show();
    return app.exec();
}







