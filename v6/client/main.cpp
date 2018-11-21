#include "mainwindow.h"
#include <QApplication>
#ifdef ACTIVEX
#include <ActiveQt>
#endif
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#if 1
        MainWindow w;
        w.show();
         return a.exec();
#else
    if (QAxFactory::isServer()) {

        // MainWindow w;
        //  w.show();
        return a.exec();
    }
#endif
}
#ifdef ACTIVEX
QAXFACTORY_BEGIN("{FC08E3FC-3754-45AB-8EFE-56A53057F34E}", /* type library ID */ "{B095BA0C-224F-4933-A458-2DD7F6B85D8F}") /* application ID */
QAXCLASS(MainWindow)
QAXFACTORY_END()
#endif
